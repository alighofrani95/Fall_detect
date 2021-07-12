#ifndef _ESP8266_H_
#define _ESP8266_H_

#include <uart.h>
#include <stdint.h>

typedef enum
{
    ESP_MODE_STATION    = 0x00,
    ESP_MODE_AP         = 0x01,
    ESP_MODE_AP_STATION = 0x02,
} esp_mode;

int esp_init(esp_mode mode, uint8_t uart, uint8_t tx_pin, uint8_t rx_pin);
int esp_connect_wifi(char *ssid, char *password);
int esp_tcp_connect(char *ip, char *port);
int esp_send_cmd(uint8_t *cmd, uint8_t *ack, uint64_t wait_time);
uint8_t* esp_check_cmd(uint8_t *str);
int esp_send_data_tcp(uint8_t *data, uint64_t wait_time);

#endif // !_ESP8266_H_
