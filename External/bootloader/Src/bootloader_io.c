#include "bootloader_io.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal_uart.h"
#include <stdbool.h>

extern UART_HandleTypeDef *bootloader_uart;

// Static functions ----------------------------------------------------------

static bool is_address_in_bounds(const uint32_t address)
{
  return !(
    address < APP_START_ADDRESS || 
    address + sizeof(uint16_t) >= FLASH_BANK1_END
  );
}

static bool is_page_address(const uint32_t address)
{
  // 128 pages
  return (
    address % 1024 == 0|| 
    address + 1024 < FLASH_BANK1_END // flash bank1 end
  );
}

// Implementations -----------------------------------------------------------

bootloader_status bootloader_io_read(
  uint8_t *const data,
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
  const uint8_t *const data,
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
  const uint32_t address,
  const uint16_t data
)
{
  // 128 pages
  if (!is_address_in_bounds(address))
    return BOOTLOADER_BOUNDS_ERROR;

  HAL_StatusTypeDef status = HAL_FLASH_Unlock();
  if (status)
    return (bootloader_status)status;

  status |= HAL_FLASH_Program(
    FLASH_TYPEPROGRAM_HALFWORD, // uint16_t
    address,
    data
  );

  status |= HAL_FLASH_Lock();

  return (bootloader_status)status;
}

bootloader_status bootloader_io_erase(
  const uint32_t address,
  const uint8_t pages_num
)
{
  if (!is_address_in_bounds(address) || !is_page_address(address))
    return BOOTLOADER_BOUNDS_ERROR;

  HAL_StatusTypeDef status = HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase_init_struct = (FLASH_EraseInitTypeDef){
    .TypeErase = FLASH_TYPEERASE_PAGES,
    .Banks = FLASH_BANK_1,
    .PageAddress = address,
    .NbPages = pages_num
  };
  uint32_t page_error = 0x0U;

  status |= HAL_FLASHEx_Erase(&erase_init_struct, &page_error);

  if (page_error != ~0x0)
    return status |= BOOTLOADER_FLASH_PAGE_ERROR; 

  status |= HAL_FLASH_Lock();

  return status;
}

bootloader_status bootloader_io_read_flash(
  const uint32_t address,
  uint8_t *const value
)
{
  if (address < FLASH_BASE || address + sizeof(uint16_t) >= FLASH_BANK1_END)
    return BOOTLOADER_BOUNDS_ERROR;

  *value = (uint8_t)(*((volatile uint32_t*)address));

  return BOOTLOADER_OK;
}
