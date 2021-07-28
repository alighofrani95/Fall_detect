#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global_config.h"
#include "sleep.h"
#include "sysctl.h"
#include "unistd.h"
#include "bsp.h"
#include "timer.h"

#include "ai_engine.h"
#include "camera.h"
#include "dvp.h"
#include "esp8266.h"
#include "fall_detect_api.h"
#include "fpioa.h"
#include "image_process.h"
#include "iomem.h"
#include "plic.h"
#include "rgb565_to_bmp.h"
#include "rtc.h"
#include "syslog.h"
#include "w25qxx.h"
#include "image_process.h"

#if CONFIG_ENABLE_LCD
#include "lcd.h"
#include "nt35310.h"
#endif

static const char *TAG = "MAIN";

static uint32_t *g_lcd_gram0;
static uint32_t *g_lcd_gram1;
static uint8_t send_fram_count = 0;
volatile uint8_t g_dvp_finish_flag;
volatile static uint8_t timer_flag_send_status = 0;
volatile static uint8_t timer_flag_frame = 0;

// added by noahzhy
volatile uint8_t flag_image_pos = 0; // a flag which image.addr is previous or next frame -> img_ai_buf
static uint8_t *model_data;
static image_t g_ai_buf, img_ai_buf;
volatile int res_fall_down = 0;
kpu_model_context_t fall_detect_task;

// a images array for storing 5 frames to send
volatile image_t imgs_80x60x5;

volatile uint8_t g_ram_mux;
#if CONFIG_ENABLE_LCD

void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
{
    size_t chn_size = width * height;
    for(size_t i = 0; i < width * height; i++)
    {
        uint8_t r = src[i];
        uint8_t g = src[chn_size + i];
        uint8_t b = src[chn_size * 2 + i];

        uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        dest[d_i] = rgb;
    }
}
#endif

static int on_irq_dvp(void *ctx)
{
    if(dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        /* switch gram */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)g_lcd_gram0 : (uint32_t)g_lcd_gram1);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    } else
    {
        if(g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }

    return 0;
}

static int timer_callback(void *ctx)
{
    static int count = 0;

    if(0 == timer_flag_send_status) {
        count++;
        if(120 == count) {
            timer_flag_send_status = 1;
            count = 0;
        }
    }
    if(0 == timer_flag_frame) {
        timer_flag_frame = 1;
    }

    return 0;
}

void sensors_init()
{
    /* DVP init */
    LOGI(TAG, "DVP init");
    camera_init();
#if CONFIG_ENABLE_LCD
    /* LCD init */
    LOGI(TAG, "LCD init");
    lcd_init();
#if CONFIG_MAIX_DOCK
    lcd_set_direction(DIR_YX_RLDU);

#else
    lcd_set_direction(DIR_YX_RLUD);
#endif

    lcd_clear(BLACK);
#endif
    g_lcd_gram0 = (uint32_t *)iomem_malloc(320 * 240 * 3);
    g_lcd_gram1 = (uint32_t *)iomem_malloc(320 * 240 * 3);

    g_ai_buf.depth = 3;
    g_ai_buf.width = 320;
    g_ai_buf.height = 240;
    image_init(&g_ai_buf);
    img_ai_buf.depth = 1;
    img_ai_buf.width = 320;
    img_ai_buf.height = 480;
    image_init(&img_ai_buf);

    imgs_80x60x5.depth = 1;
    imgs_80x60x5.width = FRAME_WIDTH;
    imgs_80x60x5.height = FRAME_HEIGHT*FRAMES_NUM;
    image_init(&imgs_80x60x5);

    dvp_set_ai_addr((uint32_t)g_ai_buf.addr, (uint32_t)(g_ai_buf.addr + 320 * 240), (uint32_t)(g_ai_buf.addr + 320 * 240 * 2));
    dvp_set_display_addr((uint32_t)g_lcd_gram0);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    /* DVP interrupt config */
    LOGD(TAG, "DVP interrupt config");
    plic_set_priority(IRQN_DVP_INTERRUPT, 2);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* enable global interrupt */
    sysctl_enable_irq();

    /* timer init */
    timer_init(TIMER_DEVICE_0);
    // Set timer interval to 5ms
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 500000000);
    // Set timer callback function with single shot method
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1, 1, timer_callback, NULL);
    // Enable timer
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);

    /* ESP8266 init */
    LOGI(TAG, "ESP8266 init");
    int ret = 0;
#if CONFIG_KD233
    ret += esp_init(ESP_MODE_STATION, UART_DEVICE_1, 28, 27);
    msleep(100);
#endif
#if CONFIG_MAIX_NANO
    ret += esp_init(ESP_MODE_STATION, UART_DEVICE_1, 6, 7);
    msleep(100);
#endif
    ret += esp_connect_wifi(WIFI_SSID, WIFI_PASSWORD);
    msleep(100);
    // 启用单连接模式
    ret += esp_send_cmd("AT+CIPMUX=0", "OK", 50);
    if(ret)
    {
        LOGE(TAG, "ESP8266 Init Error!");
    } else
    {
        LOGI(TAG, "ESP8266 Init Successed!");
    }
}

void init_model()
{
    LOGI(TAG, "Load AI model");
    model_data = (uint8_t *)iomem_malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t *)(((uintptr_t)model_data + 255) & (~255));
    w25qxx_read_data(KMODEL_ADDR, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);

    if((kpu_load_kmodel(&fall_detect_task, model_data_align) != 0))
    {
        while(1)
        {
            printf("[ERRO] Model Init Error\n");
        }
    }

    return;
}

int fall_detect(kpu_model_context_t *task, image_t *image)
{
    float *output_features;
    size_t output_size;

    ai_infer(task, image->addr, &output_features, &output_size);
    // printf("[INFO] fall detect result: %f\n", output_features[0]);
    return sigmoid(output_features);
}

int core1_tasks(void *ctx) {
    while (1) {
        // Parameters:
        // g_ai_buf(320x240x3): the original frame
        // img_ai_buf(320x480x1): two frames only green channel, thus the height is 480
        // imgs_80x60x5(80x60x1): the five frames that ready to send if someone was falling down
        // flag_image_pos: a flag which current frame is previous or next frame
        // res_fall_down: a flag of fall-down event
        // TODO:
        // get original frame that size is 320x240 each half-second
        // img_ai_buf consists of two frames(only green channel) that every two frames intervals of half-second
        // there is a function which named 'image_resize' to resize the single channel image to 80x60
        // if someone was falling down, put resized frame in the image array(imgs_80x60x5) each half-second
        // send it
        /* send active status(
            {
                "deviceSN" : "fall_0001",
                "isActive" : true
            }
            ) via wifi every minute
        */
        /* send active status */
        if(timer_flag_send_status) {
            device_register(DEVICE_SN, "true", NULL);
            timer_flag_send_status = 0;
        }
        /* if some one falling down */
        if(res_fall_down) {
            fall_event_register(DEVICE_SN, "true", imgs_80x60x5.addr, NULL);
        }
    }
return 0;
}

int main(void)
{
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);

    plic_init();
    rtc_init();

    /* sensor init */
    sensors_init();

#if AI_TEST_MODE
    // load AI model from flash
    init_model();
#endif
    /* system start */
    LOGI(TAG, "System start");
    g_ram_mux = 0;

    // g_dvp_finish_flag = 0;
    // dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    // dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    // uint8_t *img = (uint8_t *)malloc(320*240);
    // for(int i = 0; i < 80*60; i++) {
    //     img[i] = i + 48;
    // }

    // /* API test */
    // 登录
    int ret = 0;
    ret = device_login(COMPANY, USER_NAME, PASSWORD);
    if(ret)
    {
        LOGE(TAG, "Login Error %d!", ret);
    }
    // ret = fall_event_register(DEVICE_SN, "true", img, NULL);
    // if(ret)
    // {
    //     LOGE(TAG, "Upload Error %d!", ret);
    // }
    // ret = device_register(DEVICE_SN, "true", NULL);
    // if(ret) {
    //     LOGE(TAG, "device_register Error %d!", ret);
    // }
    // ret = device_list(0, 100, NULL);
    // if(ret) {
    //     LOGE(TAG, "device_list Error %d!", ret);
    // }
    // ret = fall_event_list(0, 100, NULL);
    // if(ret) {
    //     LOGE(TAG, "fall_event_list Error %d!", ret);
    // }
    // ret = fall_event_admin(0, 100, NULL);
    // if(ret) {
    //     LOGE(TAG, "fall_event_admin Error %d!", ret);
    // }

    // dvp_set_ai_addr((uint32_t)g_ai_buf, (uint32_t)(g_ai_buf + 320 * 240), (uint32_t)(g_ai_buf + 320 * 240 * 2));
    // dvp_set_display_addr((uint32_t)g_lcd_gram0);
    // dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    // dvp_disable_auto();

    register_core1(core1_tasks, NULL);
    /* enable global interrupt */
    sysctl_enable_irq();
    while(1)
    {
        /* ai cal finish*/
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while(g_dvp_finish_flag == 0)
            ;
        if(0 == flag_image_pos) {
            memcpy(img_ai_buf.addr, g_ai_buf.addr+320*240, 320*240);
            timer_flag_frame = 0;
            flag_image_pos = 1;
        } else if(timer_flag_frame){
            memcpy(img_ai_buf.addr + 320*240, g_ai_buf.addr+320*240, 320*240);
            image_resize(g_ai_buf.addr+320*240, 320, 240, imgs_80x60x5.addr+80*60*send_fram_count, 80, 60);
            send_fram_count++;
            if(send_fram_count == 5) {
                send_fram_count = 0;
            }
        }
#if AI_TEST_MODE
        res_fall_down = fall_detect(&fall_detect_task, &img_ai_buf);
#endif
        // uint16_t* d=g_ram_mux?(uint16_t*)g_lcd_gram0:(uint16_t*)g_lcd_gram1;
        // for(int i=0;i<240;i++)
        // {
        //     for(int j=0;j<320;j++)
        //     {
        //         printf("%d,",d[i*320+j]);
        //     }
        //     printf("\n");
        // }
        // while(1);
#if CONFIG_ENABLE_LCD
        /* display pic*/
        g_ram_mux ^= 0x01;

        lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? g_lcd_gram0 : g_lcd_gram1);
        /* up load img test */
        // ret++;
        // if(ret == 300) {
        //     LOGI(TAG, "UPLOAD start");
        //     uint16_t* d=g_ram_mux?(uint16_t*)g_lcd_gram0:(uint16_t*)g_lcd_gram1;
        //     // for(int i = 0; i < 320*240; i++) {
        //     //     img[i] = ((d[i] & (uint16_t)0x07E0) >> 5);
        //     // }
        //     // for(int i = 0; i < 320*240; i++) {
        //     //     printf("%d,", img[i]);
        //     // }
        //     for(int i = 0, k = 0; i < 320; i+=4, k++) {
        //         for(int j = 0, l = 0; j < 240; j+=4, l++) {
        //             img[k*80+l] = ((d[i*320+j] & (uint16_t)0x07E0) >> 5);
        //         }
        //     }
        //     fall_event_register(DEVICE_SN, "true", img, NULL);
        //     ret = 0;
        // }
        // msleep(1);
#endif
    }
    iomem_free(g_lcd_gram0);
    iomem_free(g_lcd_gram1);

    image_deinit(&g_ai_buf);
    image_deinit(&img_ai_buf);
    return 0;
}
