#include "fall_detect_api.h"
#include "esp8266.h"
#include "iomem.h"
#include "http_client.h"
#include "syslog.h"
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
int fall_event_register(char *device_sn, char *is_active, FALL_BMP_File *img_file, uint8_t *data)
{
    char head[1024] = { 0 };
    char head_host[1024] = { 0 };
    char head_token[2048] = { 0 };
    char head_length[512] = { 0 };
    int wait_time = 100;
    respon_t respon;

    // //confirm wifi status
    // if(esp_send_cmd("AT+CWSTATE?", "+CWSTATE:1",50)) {
    //     return -1;
    // }
    // //confirm tcp connection
    // if(esp_send_cmd("AT+CIPSTATUS", "STATUS:3",50)) {
    //     return -1;
    // }
    sprintf(head, "POST %s HTTP/1.1\r\n", "/api/fall-events/upload");
    sprintf(head_host, "Host: %s\r\n", host);
    sprintf(head_token, "Authorization: %s\r\n", token);
    // if(json_data) {
        sprintf(head_length, "Content-Length: %d\r\n", 154038);
    // }
    //开启透传模式
    esp_send_cmd("AT+CIPMODE=1", "OK", 200);
    esp_send_cmd("AT+CIPSEND", ">", 200);
    esp_uart_set_recv_mode(ESP_UART_RECV_MODE_DATA);
    //发送数据
    uart_send_data(uart_device, head, strlen(head));
    uart_send_data(uart_device, head_host, strlen(head_host));
    uart_send_data(uart_device, head_token, strlen(head_token));
    uart_send_data(uart_device, "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 83);
    uart_send_data(uart_device, head_length, strlen(head_length));
    uart_send_data(uart_device, "\r\n", 2);
    uart_send_data(uart_device, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"file\"; filename=\"Sample.bmp\"\r\n", 68);
    uart_send_data(uart_device, "Content-Type: image/bmp\r\n\r\n", 27);
    uart_send_data(uart_device, &img_file->BMPHead, sizeof(img_file->BMPHead));
    uart_send_data(uart_device, &img_file->BMIHead, sizeof(img_file->BMIHead));
    uart_send_data(uart_device, img_file->RgbQuadClr, 3*sizeof(RgbQuad));
    uart_send_data(uart_device, img_file->frame_buf, 320*240*sizeof(uint16_t));
    uart_send_data(uart_device, "\r\n", 2);
    uart_send_data(uart_device, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"deviceSN\"\r\n\r\n", 51);
    uart_send_data(uart_device, "fall_0001\r\n", 11);
    uart_send_data(uart_device, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
    uart_send_data(uart_device, "Content-Disposition: form-data; name=\"isActive\"\r\n\r\n", 51);
    uart_send_data(uart_device, "true\r\n", 6);
    uart_send_data(uart_device, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);

    // uart_send_data(UART_DEVICE_3, head, strlen(head));
    // uart_send_data(UART_DEVICE_3, head_host, strlen(head_host));
    // uart_send_data(UART_DEVICE_3, head_token, strlen(head_token));
    // uart_send_data(UART_DEVICE_3, "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 83);
    // uart_send_data(UART_DEVICE_3, head_length, strlen(head_length));
    // uart_send_data(UART_DEVICE_3, "\r\n", 2);
    // uart_send_data(UART_DEVICE_3, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
    // uart_send_data(UART_DEVICE_3, "Content-Disposition: form-data; name=\"file\"; filename=\"Sample.bmp\"\r\n", 68);
    // uart_send_data(UART_DEVICE_3, "Content-Type: image/bmp\r\n\r\n", 27);
    // uart_send_data(UART_DEVICE_3, &img_file->BMPHead, sizeof(img_file->BMPHead));
    // uart_send_data(UART_DEVICE_3, &img_file->BMIHead, sizeof(img_file->BMIHead));
    // uart_send_data(UART_DEVICE_3, img_file->RgbQuadClr, 3*sizeof(RgbQuad));
    // uart_send_data(UART_DEVICE_3, img_file->frame_buf, 320*240*sizeof(uint16_t));
    // uart_send_data(UART_DEVICE_3, "\r\n", 2);
    // uart_send_data(UART_DEVICE_3, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
    // uart_send_data(UART_DEVICE_3, "Content-Disposition: form-data; name=\"deviceSN\"\r\n\r\n", 51);
    // uart_send_data(UART_DEVICE_3, "fall_0001\r\n", 11);
    // uart_send_data(UART_DEVICE_3, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
    // uart_send_data(UART_DEVICE_3, "Content-Disposition: form-data; name=\"isActive\"\r\n\r\n", 51);
    // uart_send_data(UART_DEVICE_3, "true\r\n", 6);
    // uart_send_data(UART_DEVICE_3, "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n", 39);
// 	fwrite(&bmfHdr, 1, sizeof(BitMapFileHeader), fp); 
// 	fwrite(&bmiHdr, 1, sizeof(BitMapInfoHeader), fp); 
// 	fwrite(&bmiClr, 1, 3*sizeof(RgbQuad), fp);
 
// //	fwrite(buf, 1, bmiHdr.biSizeImage, fp);	//mirror
// 	for(int i=0; i<height; i++){
// 		fwrite(buf+(width*(height-i-1)*2), 2, width, fp);
// 	}
    msleep(200);    //wait respon
    //解析
    while(wait_time--) {
        char *ret = strstr(esp_uart_recv_buffer, "HTTP/1.1 ");
        if(ret) {
            ret += 9;
            sscanf(ret, "%d", &respon.state);
            msleep(100);
            while (wait_time--) {
                ret = strstr(esp_uart_recv_buffer, "Content-Length: ");
                if(ret) {
                    ret += 16;
                    sscanf(ret, "%d", &respon.data_length);
                    msleep(100);
                    while (wait_time--) {
                        ret = strstr(esp_uart_recv_buffer, "\r\n\r\n");
                        if(ret) {
                            ret += 4;
                            msleep(100);
                            while (wait_time--) {
                                if(strlen(ret) >= respon.data_length) {
                                    memcpy(respon.data, ret, strlen(ret));
                                    respon.data[respon.data_length] = 0;
                                }
                                msleep(10);
                            }
                            break;
                        }
                        msleep(10);
                    }
                    break;
                }
                msleep(10);
            }
            break;
        }
        msleep(10);
    }
    //退出透传模式
    uart_send_data(uart_device, "+++", 3);
    msleep(10);
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