#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <stdint.h>

typedef struct
{
    uint32_t state;
    uint32_t data_length;
    uint8_t data[16348]; 
} respon_t;

int http_client_post(char *url, char *host, char *token, char *json_data, respon_t *respon);
int http_client_get(char *url, char *host, char *token, respon_t *respon);

#endif // !_HTTP_CLIENT_H_