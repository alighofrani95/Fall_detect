#include "esp8266.h"
#include <fpioa.h>
#include <uart.h>
#include <string.h>
#include <sleep.h>
#include <syslog.h>
#include <assert.h>

#define UART_RECV_MAX_LEN   256

static const char *TAG = "ESP8266";
static uart_device_number_t uart_device;
static uint16_t uart_recv_sta;
static uint8_t uart_recv_buffer[UART_RECV_MAX_LEN];

static int on_esp_uart_recv(void *ctx) 
{
    char v_buf;
    int ret = uart_receive_data(uart_device, &v_buf, 1);
    if((uart_recv_sta & 0x8000) == 0) {
        //接收未完成
        if(uart_recv_sta & 0x4000) {
            //接收到了0x0d
            if(v_buf != 0x0a) {
                //接收错误,重新开始
                uart_recv_sta = 0;
            } else {
                 uart_recv_sta |= 0x8000;	//接收完成了 
                 uart_recv_buffer[uart_recv_sta&0X3FFF] = 0;
            }
        } else {
            //还没收到0X0D	
            if(v_buf == 0x0d) {
                uart_recv_sta|=0x4000;
            } else {
                uart_recv_buffer[uart_recv_sta&0X3FFF] = v_buf ;
                uart_recv_sta++;
                if(uart_recv_sta > (UART_RECV_MAX_LEN-1)) {
                    uart_recv_sta=0;//接收数据错误,重新开始接收	
                }  
            }		 
        }
    }

    return ret;
}



int esp_init(esp_mode mode, uint8_t uart, uint8_t tx_pin, uint8_t rx_pin)
{
    int ret = 0;

    uart_device = uart;
    /* init uart*/
    switch (uart_device)
    {
    case UART_DEVICE_1:
        ret += fpioa_set_function(tx_pin, FUNC_UART1_TX);
        ret += fpioa_set_function(rx_pin, FUNC_UART1_RX);
        break;

    case UART_DEVICE_2:
        ret += fpioa_set_function(tx_pin, FUNC_UART2_TX);
        ret += fpioa_set_function(rx_pin, FUNC_UART2_RX);
        break;

    case UART_DEVICE_3:
        ret += fpioa_set_function(tx_pin, FUNC_UART3_TX);
        ret += fpioa_set_function(rx_pin, FUNC_UART3_RX);
        break;

    default:
        break;
    }
    uart_init(uart_device);
    uart_configure(uart_device, 115200, 8, UART_STOP_1, UART_PARITY_NONE);
    uart_set_receive_trigger(uart_device, UART_RECEIVE_FIFO_1);
    uart_irq_register(uart_device, UART_RECEIVE, on_esp_uart_recv, NULL, 1);
    uart_recv_sta = 0;
    /* init esp 8266*/
    switch (mode)
    {
    case ESP_MODE_STATION:
        // set esp8266 to station mode
        ret += esp_send_cmd("AT+CWMODE=1", "OK", 500);
        ret += esp_send_cmd("AT+RST", "ready", 500);
        break;

    case ESP_MODE_AP:
        ret += esp_send_cmd("AT+CWMODE=2", "OK", 50);
        ret += esp_send_cmd("AT+RST", "OK", 200);
        break;

    case ESP_MODE_AP_STATION:
        ret += esp_send_cmd("AT+CWMODE=3", "OK", 50);
        ret += esp_send_cmd("AT+RST", "OK", 200);
        break;
        
    default:
        break;
    }

    return ret;
}

int esp_connect_wifi(char *ssid, char *password)
{
    int ret = 0;
    char *cmd[256] = {0};

    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    ret += esp_send_cmd(cmd, "OK", 1000);

    return ret;
}

int esp_tcp_connect(char *ip, char *port)
{
    int ret = 0;
    char *cmd[256] = {0};

    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s", ip, port);
    ret += esp_send_cmd(cmd, "OK", 200);

    return ret;
}

int esp_send_cmd(uint8_t *cmd, uint8_t *ack, uint64_t wait_time)
{
    uint8_t res = 0;
    
    uart_recv_sta = 0;
    uart_send_data(uart_device, cmd, strlen(cmd));
    uart_send_data(uart_device, "\r\n", 2);
    uart_send_data(UART_DEVICE_3, cmd, strlen(cmd));
    uart_send_data(UART_DEVICE_3, "\r\n", 2);
    if(ack && wait_time) {
        while(--wait_time) {
            msleep(10);
            if(uart_recv_sta & 0x8000) {
                if(esp_check_cmd(ack)) {
                    uart_recv_sta = 0;
                    break;
                }
                uart_recv_sta = 0;
            }
        }
        if(0 == wait_time) {
            res = 1;
        }
    }

    return res;
}

uint8_t* esp_check_cmd(uint8_t *str)
{
    char *strx = NULL;

    if(uart_recv_sta & 0x8000) {
        strx = strstr((const char*)uart_recv_buffer, (const char*)str);
        LOGD(TAG, "%s", uart_recv_buffer);
    }

    return (uint8_t *)strx;
}

uint8_t* esp_send_data(uint8_t *cmd, uint64_t wait_time)
{
    char temp[5];
    char *ack = temp;
}

int esp_send_data_tcp(uint8_t *data, uint64_t wait_time)
{
    char cmd[48] = {0};

	sprintf(cmd, "AT+CIPSEND=%ld", (strlen(data) + 2));
	if(esp_send_cmd(cmd,0,wait_time) == 0) {
        msleep(10);
	    uart_send_data(uart_device, data, strlen(data));
        uart_send_data(uart_device, "\r\n", 2);
        uart_send_data(UART_DEVICE_3, data, strlen(data));
        uart_send_data(UART_DEVICE_3, "\r\n", 2);
        uart_recv_sta = 0;
	}
    // printf("%d\n", esp_send_cmd(cmd,"OK",wait_time));
    // uart_send_data(uart_device, data, strlen(data));
    // uart_send_data(uart_device, "\n", 1);

    return 0;
}