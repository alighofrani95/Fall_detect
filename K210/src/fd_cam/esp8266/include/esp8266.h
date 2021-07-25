#ifndef _ESP8266_H_
#define _ESP8266_H_

#define UART_RECV_MAX_LEN   16348

#include <uart.h>
#include <stdint.h>

/* ESP8266 模式 */
typedef enum
{
    ESP_MODE_STATION            = 0x00,
    ESP_MODE_AP                 = 0x01,
    ESP_MODE_AP_STATION         = 0x02,
} esp_mode;

/* UART接收模式 */
typedef enum
{
    ESP_UART_RECV_MODE_CHECK    = 0x00,
    ESP_UART_RECV_MODE_DATA     = 0x01,
    ESP_UART_RECV_MODE_MAX,
} esp_uart_recv_mode;

extern uint8_t esp_uart_recv_buffer[UART_RECV_MAX_LEN];
extern uart_device_number_t uart_device;

int esp_init(esp_mode mode, uint8_t uart, uint8_t tx_pin, uint8_t rx_pin);
int esp_connect_wifi(char *ssid, char *password);
int esp_tcp_connect(char *ip, char *port);
int esp_tcp_start_trans();
int esp_tcp_quit_trans();
int esp_send_cmd(uint8_t *cmd, uint8_t *ack, uint64_t wait_time);
void esp_uart_set_recv_mode(esp_uart_recv_mode mode);
uint8_t* esp_check_cmd(uint8_t *str);
int esp_send_data_tcp(uint8_t *data, uint64_t wait_time);

#endif // !_ESP8266_H_
