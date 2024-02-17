#include "bootloader_io.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal_uart.h"

extern UART_HandleTypeDef *bootloader_uart;

bootloader_status bootloader_io_read(
    const uint8_t *const data,
    const uint16_t size
)
{
    return (bootloader_status)HAL_UART_Receive(
        bootloader_uart,
        (uint8_t*)data,
        size,
        UART_DELAY
    );
}

bootloader_status bootloader_io_write(
    uint8_t *const data,
    const uint16_t size
)
{
    return (bootloader_status)HAL_UART_Transmit(
        bootloader_uart,
        (uint8_t*)data,
        size,
        UART_DELAY
    );
}

uint32_t bootloader_io_get_dev_id()
{
    return HAL_GetDEVID();
}
