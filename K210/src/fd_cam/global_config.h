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
// #define WIFI_SSID                           "GUET-NB"
// #define WIFI_PASSWORD                       "15177564904"
#define WIFI_SSID                           "roha-ap"
#define WIFI_PASSWORD                       "ARSaKThne1"

#define CAM_WIDTH                           160
#define CAM_HEIGHT                          120
#define FRAMES_NUM                          5
#define FRAME_WIDTH                         80
#define FRAME_HEIGHT                        60
#define AI_TEST_MODE                        1
#define KMODEL_SIZE                         (616 * 1024)
#define KMODEL_ADDR                         0xA00000

// Board
#define CONFIG_MAIX_DOCK                    0
#define CONFIG_MAIX_NANO                    1
#define CONFIG_KD233                        0

// Camera
#define CONFIG_CAMERA_GC0328                1
#define CONFIG_CAMERA_OV5640                0
#define CONFIG_CAMERA_OV2640                0
#define CONFIG_CAMERA_RESOLUTION_WIDTH      160
#define CONFIG_CAMERA_RESOLUTION_HEIGHT     120

// LCD
#define CONFIG_ENABLE_LCD                   0
#define CONFIG_LCD_DCX_GPIONUM              2
#define CONFIG_LCD_RST_GPIONUM              3
#define CONFIG_LCD_SPI_CHANNEL              0
#define CONFIG_LCD_SPI_SLAVE_SELECT         3
#define CONFIG_LCD_WIDTH                    160
#define CONFIG_LCD_HEIGHT                   120

#endif // !_GLOBAL_CONFIG_H_