#ifndef __WIFI_H
#define __WIFI_H


typedef enum {
    AT_MODE_NOWIFI,
    AT_MODE_STATION,
    AT_MODE_SOFTAP,
    AT_MODE_SOFTAP_STATION,
} at_mode;


#define AT_CMD_MODE             _T("AT+CWMOD")
#define AT_CMD_SCAN             _T("AT+CWLAP")
#define AT_CMD_CONNECT          _T("AT+CWJAP")
#define AT_CMD_STATUS           _T("AT+CIPSTATUS")

// typedef int AT_COMMAND {
// } at_t;

static int wifi_send(char* command);

// just like uart init
void wifi_init();
void wifi_mode(at_mode mode);
void wifi_auto_connect(int enable);

int wifi_status();
int wifi_scan(char* id);
int wifi_connect(char* id, char* pwd);

int wifi_post();
int wifi_get();

#endif