#ifndef BOOTLOADER_DEFS_H
#define BOOTLOADER_DEFS_H

#define APP_START_ADDRESS 0x08002800 /* page 10 */
#define SRAM_SIZE 20 * 1024
#define SRAM_END (SRAM_BASE + SRAM_SIZE)

enum
{
  BOOTLOADER_VER_MAJOR = 0U + '0',
  BOOTLOADER_VER_MINOR = 1U + '0',
  CMD_HELP = 0U + '0',
  CMD_GET_ID = 1U + '0',
  CMD_GET_BOOTLOADER_VER = 2U + '0',
  CMD_WRITE = 3U + '0',
  CMD_ERASE = 4U + '0',
  UART_POLLING_DELAY = 50U,
  UART_DELAY = 500U,
  LED_DELAY = 500U,
  LED_ERROR_DELAY = 150U,
  UART_BUFFER_SIZE = 120U,
  ACK_BYTE = 0x55,
  NACK_BYTE = 0xaa,
  END_SUBSEQUENCE = 0xCC33U
};

typedef enum 
{
  BOOTLOADER_OK = 0x00U,
  BOOTLOADER_ERROR = 0x01U,
  BOOTLOADER_BUSY = 0x02U,
  BOOTLOADER_TIMEOUT = 0x03U,
  BOOTLOADER_BOUNDS_ERROR = 0x04U,
  BOOTLOADER_FLASH_PAGE_ERROR = 0x05U,
} bootloader_status;

#endif
