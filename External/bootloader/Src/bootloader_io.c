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

bootloader_status bootloader_io_program(
    uint32_t address,
    const uint8_t *const data
)
{
  // 128 pages
  if (
    address < APP_START_ADDRESS || 
    address + sizeof(uint16_t) >= FLASH_BANK1_END
  )
    return BOOTLOADER_BOUNDS_ERROR;

  HAL_StatusTypeDef status = HAL_FLASH_OB_Unlock();

  status = HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase_init_struct = (FLASH_EraseInitTypeDef){
    .TypeErase = FLASH_TYPEERASE_PAGES,
    .Banks = FLASH_BANK_1,
    .PageAddress = address,
    .NbPages = 1
  };
  uint32_t page_error = 0x0U;

  status |= HAL_FLASHEx_Erase(&erase_init_struct, &page_error);

  if (status != ~0x0)
    return BOOTLOADER_FLASH_PAGE_ERROR; 
  if (status)
    return (bootloader_status)status;

  for (uint8_t i = 0; i < 16; i++)
  {
     status |= HAL_FLASH_Program(
      FLASH_TYPEPROGRAM_WORD,
      address,
      data
    );
  }

  status |= HAL_FLASH_Lock();

  return (bootloader_status)status;
}
