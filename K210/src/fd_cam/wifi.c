#include <uart.h>
#include <fpioa.h>

#define UART_NUM UART_DEVICE_3

static void io_init(void) {
    // uart
    fpioa_set_function(6, FUNC_UART1_RX + UART_NUM * 2);
    fpioa_set_function(7, FUNC_UART1_TX + UART_NUM * 2);
}

void wifi_init() {
    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);
}
