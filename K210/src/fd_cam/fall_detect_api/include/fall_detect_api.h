#ifndef _FALL_DETECT_API_H_
#define _FALL_DETECT_API_H_

#include <stdint.h>
#include "rgb565_to_bmp.h"

int device_login(char *domain, char *user_id, char *password);
/*
 * 定期发送
 */
int device_register(char *device_sn, char *is_active, uint8_t *data);
int device_list(int page, int size, uint8_t *data);
int device_admin(int page, int size, uint8_t *data);
/*
 * 事件发生时发送
 */
int fall_event_register(char *device_sn, char *is_active, uint8_t *img_buf, uint8_t *data);
int fall_event_list(int page, int size, uint8_t *data);
int fall_event_admin(int page, int size, uint8_t *data);

#endif // !_FALL_DETECT_API_H_