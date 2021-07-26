#ifndef _GLOBAL_CONFIG_H_
#define _GLOBAL_CONFIG_H_

// Device
#define DEVICE_SN                           "fall_0001"
#define USER_NAME                           "admin"
#define COMPANY                             "Company1"
#define PASSWORD                            "test1234"
#define HOST_FALL                           "rest.fall-test.com"
#define HOST_FALL_PORT                      "80"
#define HOST_VIDEO                          "3.35.0.186"
#define HOST_VIDEO_PORT                     "80"
#define WIFI_SSID                           "GUET-NB"
#define WIFI_PASSWORD                       "15177564904"

// Board
#define CONFIG_MAIX_DOCK                    0
#define CONFIG_MAIX_NANO                    0
#define CONFIG_KD233                        1

// Camera
#define CONFIG_CAMERA_GC0328                1
#define CONFIG_CAMERA_OV5640                0
#define CONFIG_CAMERA_OV2640                0
#define CONFIG_CAMERA_RESOLUTION_WIDTH      320
#define CONFIG_CAMERA_RESOLUTION_HEIGHT     240

// LCD
#define CONFIG_ENABLE_LCD                   0
#define CONFIG_LCD_DCX_GPIONUM              2
#define CONFIG_LCD_RST_GPIONUM              3
#define CONFIG_LCD_SPI_CHANNEL              0
#define CONFIG_LCD_SPI_SLAVE_SELECT         3
#define CONFIG_LCD_WIDTH                    320
#define CONFIG_LCD_HEIGHT                   240

#endif // !_GLOBAL_CONFIG_H_