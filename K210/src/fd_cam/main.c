#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sleep.h"
#include "unistd.h"
#include "sysctl.h"
#include "global_config.h"

#include "dvp.h"
#include "fpioa.h"
#include "iomem.h"
#include "plic.h"
#include "camera.h"
#include "esp8266.h"
#include "rtc.h"
#include "syslog.h"
#include "fall_detect_api.h"
#include "rgb565_to_bmp.h"

#if CONFIG_ENABLE_LCD
#include "nt35310.h"
#include "lcd.h"
#endif



static const char *TAG = "MAIN";

static uint32_t *g_lcd_gram0;
static uint32_t *g_lcd_gram1;
volatile uint8_t g_dvp_finish_flag;


volatile uint8_t g_ram_mux;
#if CONFIG_ENABLE_LCD

void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
{
    size_t chn_size = width * height;
    for (size_t i = 0; i < width * height; i++)
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
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        /* switch gram */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)g_lcd_gram0 : (uint32_t)g_lcd_gram1);
        
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        if (g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }

    return 0;
}

void sensors_init() {
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
    g_lcd_gram0 = (uint32_t *)iomem_malloc(320 * 240 * 2);
    g_lcd_gram1 = (uint32_t *)iomem_malloc(320 * 240 * 2);


    dvp_set_ai_addr((uint32_t)0x40600000, (uint32_t)0x40612C00, (uint32_t)0x40625800);
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

    /* ESP8266 init */
    LOGI(TAG, "ESP8266 init");
    int ret = 0;
    #ifdef CONFIG_KD233
        ret += esp_init(ESP_MODE_STATION, UART_DEVICE_1, 28, 27);
        msleep(100);
    #endif
    #ifdef CONFIG_MAIX_NANO
        ret += esp_init(ESP_MODE_STATION, UART_DEVICE_1, 6, 7);
        msleep(100);
    #endif
    ret += esp_connect_wifi(WIFI_SSID, WIFI_PASSWORD);
    msleep(100);
    // 启用单连接模式
    ret += esp_send_cmd("AT+CIPMUX=0", "OK", 50);
    if(ret) {
        LOGE(TAG, "ESP8266 Init Error!");
    } else {
        LOGI(TAG, "ESP8266 Init Successed!");
    }
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

    /* system start */
    LOGI(TAG, "System start");
    g_ram_mux = 0;
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    uint8_t *img = (uint8_t *)malloc(320*240);
    // uint8_t img[80*60] = { 0 };
    // for(int i = 0; i < 80*60; i++) {
    //     img[i] = i + 48;
    // }
    
    // /* API test */
    // 登录
    int ret = 0;
    ret = device_login(COMPANY, USER_NAME, PASSWORD);
    if(ret) {
        LOGE(TAG, "Login Error %d!", ret);
    }
    ret = fall_event_register(DEVICE_SN, "true", img, NULL);
    if(ret) {
        LOGE(TAG, "Upload Error %d!", ret);
    }
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

    while (1)
    {
        /* ai cal finish*/
        while (g_dvp_finish_flag == 0)
            ;
        g_dvp_finish_flag = 0;
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
    return 0;
}
