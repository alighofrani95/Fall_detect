#include "http_client.h"
#include "esp8266.h"
#include "iomem.h"
#include <string.h>
#include <sleep.h>

int http_client_post(char *url, char *host, char *token, char *json_data, respon_t *respon)
{
    char head[1024] = { 0 };
    char head_host[1024] = { 0 };
    char head_token[2048] = { 0 };
    char head_length[512] = { 0 };
    int wait_time = 100;

    // //confirm wifi status
    // if(esp_send_cmd("AT+CWSTATE?", "+CWSTATE:1",50)) {
    //     return -1;
    // }
    // //confirm tcp connection
    // if(esp_send_cmd("AT+CIPSTATUS", "STATUS:3",50)) {
    //     return -1;
    // }
    sprintf(head, "POST %s HTTP/1.1\r\n", url);
    sprintf(head_host, "Host: %s\r\n", host);
    if(token) {
        sprintf(head_token, "Authorization: %s\r\n", token);
    }
    if(json_data) {
        sprintf(head_length, "Content-Length: %ld\r\n", strlen(json_data));
    }
    //开启透传模式
    esp_send_cmd("AT+CIPMODE=1", "OK", 200);
    esp_send_cmd("AT+CIPSEND", ">", 200);
    esp_uart_set_recv_mode(ESP_UART_RECV_MODE_DATA);
    //发送数据
    uart_send_data(uart_device, head, strlen(head));
    uart_send_data(uart_device, head_host, strlen(head_host));
    if(token) {
        uart_send_data(uart_device, head_token, strlen(head_token));
    }
    uart_send_data(uart_device, "Content-Type: application/json\r\n", 32);
    uart_send_data(uart_device, head_length, strlen(head_length));
    uart_send_data(uart_device, "\r\n", 2);
    uart_send_data(uart_device, json_data, strlen(json_data));
    msleep(200);    //wait respon
    //解析
    while(wait_time--) {
        char *ret = strstr(esp_uart_recv_buffer, "HTTP/1.1 ");
        if(ret) {
            ret += 9;
            sscanf(ret, "%d", &respon->state);
            msleep(100);
            while (wait_time--) {
                ret = strstr(esp_uart_recv_buffer, "Content-Length: ");
                if(ret) {
                    ret += 16;
                    sscanf(ret, "%d", &respon->data_length);
                    msleep(100);
                    while (wait_time--) {
                        ret = strstr(esp_uart_recv_buffer, "\r\n\r\n");
                        if(ret) {
                            ret += 4;
                            msleep(100);
                            while (wait_time--) {
                                if(strlen(ret) >= respon->data_length) {
                                    memcpy(respon->data, ret, strlen(ret));
                                    respon->data[respon->data_length] = 0;
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

    return 0;
}

int http_client_get(char *url, char *host, char *token, respon_t *respon)
{
    char head[1024] = { 0 };
    char head_host[1024] = { 0 };
    char head_token[2048] = { 0 };
    int wait_time = 100;

    // //confirm wifi status
    // if(esp_send_cmd("AT+CWSTATE?", "+CWSTATE:1",50)) {
    //     return -1;
    // }
    // //confirm tcp connection
    // if(esp_send_cmd("AT+CIPSTATUS", "STATUS:3",50)) {
    //     return -1;
    // }
    sprintf(head, "GET %s HTTP/1.1\r\n", url);
    sprintf(head_host, "Host: %s\r\n", host);
    if(token) {
        sprintf(head_token, "Authorization: %s\r\n", token);
    }
    //开启透传模式
    esp_send_cmd("AT+CIPMODE=1", "OK", 200);
    esp_send_cmd("AT+CIPSEND", ">", 200);
    esp_uart_set_recv_mode(ESP_UART_RECV_MODE_DATA);
    //发送数据
    uart_send_data(uart_device, head, strlen(head));
    uart_send_data(uart_device, head_host, strlen(head_host));
    if(token) {
        uart_send_data(uart_device, head_token, strlen(head_token));
    }
    uart_send_data(uart_device, "Content-Type: application/json\r\n", 32);
    uart_send_data(uart_device, "\r\n", 2);
    msleep(200);    //wait respon
    //解析
    while(wait_time--) {
        char *ret = strstr(esp_uart_recv_buffer, "HTTP/1.1 ");
        if(ret) {
            ret += 9;
            sscanf(ret, "%d", &respon->state);
            msleep(100);
            while (wait_time--) {
                ret = strstr(esp_uart_recv_buffer, "Content-Length: ");
                if(ret) {
                    ret += 16;
                    sscanf(ret, "%d", &respon->data_length);
                    msleep(100);
                    while (wait_time--) {
                        ret = strstr(esp_uart_recv_buffer, "\r\n\r\n");
                        if(ret) {
                            ret += 4;
                            msleep(100);
                            while (wait_time--) {
                                if(strlen(ret) >= respon->data_length) {
                                    memcpy(respon->data, ret, strlen(ret));
                                    respon->data[respon->data_length] = 0;
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

    return 0;
}
