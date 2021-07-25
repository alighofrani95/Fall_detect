#include "fall_detect_api.h"
#include "esp8266.h"
#include "iomem.h"
#include "http_client.h"
#include "syslog.h"
#include "global_config.h"
#include <stdio.h>
#include <sleep.h>
#include <string.h>

static const char *TAG = "fall_detect_api";
static char token[256] = { 0 };
static char *host = "rest.fall-test.com";

int device_login(char *domain, char *user_id, char *password)
{
    respon_t respon;
    char raw[1024] = { 0 };

    //json data
    sprintf(raw, "{\"domain\":\"%s\",\"userId\":\"%s\",\"password\":\"%s\"}", domain, user_id, password);
    http_client_post("/api/login", host, NULL, raw, &respon);    
    LOGD(TAG, "%s", (char *)respon.data);
    memcpy(token, respon.data, respon.data_length);
    printf("%s",respon.data);
    if(respon.state == 200) {
        return 0;
    } else {
        return respon.state;
    }
}

/*
 * 定期发送
 */

/*
 * POST등록
 */
int device_register(char *device_sn, char *is_active, uint8_t *data)
{
    respon_t respon;
    char raw[1024] = { 0 };

    sprintf(raw, "{\"deviceSN\":\"%s\",\"isActive\":%s}", device_sn, is_active);
    http_client_post("/api/fall-cameras", host, token, raw, &respon);
    LOGD(TAG, "%s", respon.data);
    if(data) {
        memcpy(data, respon.data, respon.data_length);
    }
    if(respon.state == 201) {
        return 0;
    } else {
        return respon.state;
    }
}

/*
 * GET목록
 */
int device_list(int page, int size, uint8_t *data)
{
    respon_t respon;
    char url[256] = { 0 };

    sprintf(url, "/api/fall-cameras?page=%d&size=%d&sort=id,desc", page, size);
    http_client_get(url, host, token, &respon);
    LOGD(TAG, "%s", respon.data);
    if(data) {
        memcpy(data, respon.data, respon.data_length);
    }
    if(respon.state == 200) {
        return 0;
    } else {
        return respon.state;
    }
}

/*
 * GET목록(관리대상자별)
 */
int device_admin(int page, int size, uint8_t *data)
{
    respon_t respon;
    char url[256] = { 0 };

    sprintf(url, "/api/fall-cameras/gwallidaesangs/16?page=%d&size=%d&sort=id,desc", page, size);
    http_client_get(url, host, token, &respon);
    LOGD(TAG, "%s", respon.data);
    if(data) {
        memcpy(data, respon.data, respon.data_length);
    }
    if(respon.state == 200) {
        return 0;
    } else {
        return respon.state;
    }
}
/*
 * 事件发生时发送
 */

/*
 * POST등록 form
 */
int fall_event_register(char *device_sn, char *is_active, uint8_t *img_buf, uint8_t *data)
{
    char head[1024] = { 0 };
    char head_host[1024] = { 0 };
    char head_token[2048] = { 0 };
    char head_length[512] = { 0 };
    int wait_time = 100;
    respon_t respon;

    //confirm wifi status
    if(esp_send_cmd("AT+CWJAP?", "OK",50)) {
        int ret = 0;
        #ifdef CONFIG_KD233
            ret += esp_init(ESP_MODE_STATION, UART_DEVICE_1, 28, 27);
            msleep(100);
        #endif
        ret += esp_connect_wifi(WIFI_SSID, WIFI_PASSWORD);
        msleep(100);
        if(ret) {
            return -1;
        }
    }
    sprintf(head, "POST %s HTTP/1.1\r\n", "/cgi-bin/post.py");
    sprintf(head_host, "Host: %s\r\n", HOST_VIDEO);
    sprintf(head_length, "Content-Length: %ld\r\n", 80*60+478+strlen(device_sn)+strlen(is_active)+strlen(token));
    // tcp connection
    if(esp_tcp_connect(HOST_VIDEO, HOST_VIDEO_PORT)) {
        return -1;
    }
    //开启透传模式
    esp_tcp_start_trans();
    esp_uart_set_recv_mode(ESP_UART_RECV_MODE_DATA);
    //发送数据
    uart_send_data(uart_device, head, strlen(head));
    uart_send_data(uart_device, head_host, strlen(head_host));
    uart_send_data(uart_device, head_length, strlen(head_length));
    uart_send_data(uart_device, "Connection: keep-alive\r\n", 24);
    uart_send_data(uart_device, "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 83);
    uart_send_data(uart_device, "\r\n", 2);
    // POST DATA
    //token
    uart_send_data(uart_device, "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 41);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"token\"\r\n\r\n", 48);
    uart_send_data(uart_device, token, strlen(token));
    uart_send_data(uart_device, "\r\n", 2);
    //img data
    uart_send_data(uart_device, "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 41);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"img_data\"; filename=\"img.bytes\"\r\n", 71);
    uart_send_data(uart_device, "Content-Type: application/octet-stream\r\n\r\n", 42);
    //send img data
    // uart_send_data(uart_device, "(data)", 6);
    for(int i = 0; i < 80*60; i++) {
        uart_send_data(uart_device, img_buf[i], 1);
    }
    uart_send_data(uart_device, "\r\n", 2);
    //device sn
    uart_send_data(uart_device, "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 41);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"deviceSN\"\r\n\r\n", 51);
    uart_send_data(uart_device, device_sn, strlen(device_sn));
    uart_send_data(uart_device, "\r\n", 2);
    //isactive
    uart_send_data(uart_device, "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 41);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"isActive\"\r\n\r\n", 51);
    uart_send_data(uart_device, is_active, strlen(is_active));
    uart_send_data(uart_device, "\r\n", 2);
    uart_send_data(uart_device, "------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n", 43);
    msleep(100);    //wait respon
    //解析
    while(wait_time--) {
        char *ret = strstr(esp_uart_recv_buffer, "HTTP/1.1 ");
        if(ret) {
            ret += 9;
            sscanf(ret, "%d", &respon.state);
            msleep(100);
            break;
        }
        msleep(10);
    }
    if(wait_time <= 0) {
        esp_tcp_quit_trans();
        return -1;
    }
    respon.data_length = strlen(esp_uart_recv_buffer);
    memcpy(respon.data, esp_uart_recv_buffer, respon.data_length);
    respon.data[respon.data_length] = 0;
    //退出透传模式
    esp_tcp_quit_trans();
    // 断开连接
    esp_send_cmd("AT+CIPCLOSE", 0, 20);
    msleep(10);
    esp_uart_set_recv_mode(ESP_UART_RECV_MODE_MAX);
    LOGD(TAG, "%s", respon.data);
    if(data) {
        memcpy(data, respon.data, respon.data_length);
    }
    if(respon.state == 200) {
        return 0;
    }

    return respon.state;
}

/*
 * GET목록
 */
int fall_event_list(int page, int size, uint8_t *data)
{
    respon_t respon;
    char url[256] = { 0 };

    sprintf(url, "/api/fall-events?page=%d&size=%d&sort=id,desc", page, size);
    http_client_get(url, host, token, &respon);
    LOGD(TAG, "%s", respon.data);
    if(data) {
        memcpy(data, respon.data, respon.data_length);
    }
    if(respon.state == 200) {
        return 0;
    } else {
        return respon.state;
    }
}

/*
 * GET목록(관리대상자별)
 */
int fall_event_admin(int page, int size, uint8_t *data)
{
    respon_t respon;
    char url[256] = { 0 };

    sprintf(url, "/api/fall-events/gwallidaesangs/16?page=%d&size=%d&sort=id,desc", page, size);
    http_client_get(url, host, token, &respon);
    LOGD(TAG, "%s", respon.data);
    if(data) {
        memcpy(data, respon.data, respon.data_length);
    }
    if(respon.state == 200) {
        return 0;
    } else {
        return respon.state;
    }
}