#include "bootloader_cmd.h"
#include "bootloader_defs.h"
#include <string.h>
#include <stdbool.h>

static char *start_message = "-FlexyPixel Bootloader."
  " Type '0' for commands list";
static char *input_prompt = "\r\n>>";
static char *commands_list_message = "\r\nCommands:\r\n"
  "Get id of chip - '1';\r\n"
  "Get bootloader version - '2';\r\n"
  "Write to memory (2 bytes) - '3'.";
static char *id_message = "\r\nChip ID: ";
static char *bootloader_version_message = "\r\nBootloader version: ";

static uint8_t uart_buffer[UART_BUFFER_SIZE];

// Static functions ----------------------------------------------------------

__attribute__((always_inline))
inline static void swap(uint8_t *a, uint8_t *b)
{
  *a = *a ^ *b;
  *b = *a ^ *b;
  *a = *b ^ *a;
}

__attribute__((always_inline))
inline static uint8_t int_to_string(uint8_t *const buffer, uint32_t num)
{
  int i = 0;
    
  do {
    buffer[i++] = num % 10 + '0';
    num /= 10;
  } while (num > 0);
  buffer[i] = '\0';

  // Reverse string
  int j = 0;
  while (j < i / 2) {
    swap(buffer + j, buffer + (i - j - 1));
    j++;
  }

  return i;
}

static void send_response(bootloader_status status)
{
  if (status)
    uart_buffer[0] = NACK_BYTE;
  else
    uart_buffer[0] = ACK_BYTE;
  (void)bootloader_io_write(uart_buffer, 1);
}

static void cmd_help()
{
  strncpy(
    (char*)uart_buffer,
    commands_list_message,
    strlen(commands_list_message)
  );
  bootloader_io_write(uart_buffer, strlen(commands_list_message));
}

static void cmd_get_id()
{
  strncpy((char*)uart_buffer, id_message, strlen(id_message) + 1);
  bootloader_io_write(uart_buffer, strlen(id_message) + 1);

  uint8_t size = int_to_string(uart_buffer, bootloader_io_get_dev_id());
  bootloader_io_write(uart_buffer, size);
}

static void cmd_get_bootloader_version()
{
  strncpy(
    (char*)uart_buffer,
    bootloader_version_message,
    strlen(bootloader_version_message) + 1
  );
  bootloader_io_write(uart_buffer, strlen(bootloader_version_message) + 1);

  uart_buffer[0] = BOOTLOADER_VER_MAJOR;
  uart_buffer[1] = '.';
  uart_buffer[2] = BOOTLOADER_VER_MINOR;

  bootloader_io_write(uart_buffer, 3);
}

// cmd_0: address (4 bytes)
// cmd_0: data (2 bytes)
static bootloader_status cmd_write()
{
  uint32_t address = 0;
  uint16_t data = 0;

  bootloader_status status = BOOTLOADER_OK;
  while (true) {
    status |= bootloader_io_read(
      (uint8_t*)&address,
      sizeof(uint32_t)
    );
    if (address == END_SUBSEQUENCE)
      break;

    status |= bootloader_io_read((uint8_t*)&data, sizeof(uint16_t));

    if (status == BOOTLOADER_OK)
      status |= bootloader_io_program(address, data);

    send_response(status);
    if (status)
      break;
  }

  return status;
}

// cmd_0: address (4 bytes)
// cmd_0: pages to erase (1 byte)
static bootloader_status cmd_erase()
{
  uint32_t address = 0;
  uint8_t page_num = 0;

  bootloader_status status = bootloader_io_read(
    (uint8_t*)&address,
    sizeof(uint32_t)
  );

  status |= bootloader_io_read(&page_num, sizeof(uint8_t));

  if (status == BOOTLOADER_OK)
    status |= bootloader_io_erase(address, page_num);
  send_response(status);

  return status;
}

// Implementations -----------------------------------------------------------

bootloader_status bootloader_start_output()
{
  bootloader_status status = bootloader_io_write(
    (uint8_t*)start_message,
    strlen(start_message) + 1
  );
  status |= bootloader_io_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  return status;
}

bootloader_status bootloader_proccess_input()
{
  bootloader_status status = bootloader_io_read(uart_buffer, 1);

  if (status == BOOTLOADER_TIMEOUT)
    return status;

  status = bootloader_io_write(uart_buffer, 1);

  switch(uart_buffer[0])
  {
    case CMD_HELP:
      cmd_help();
      break;
    case CMD_GET_ID:
      cmd_get_id();
      break;
    case CMD_GET_BOOTLOADER_VER:
      cmd_get_bootloader_version();
      break;
    case CMD_WRITE:
      status |= cmd_write();
      break;
    case CMD_ERASE:
      status |= cmd_erase();
      break;
  }

  status |= bootloader_io_write((uint8_t*)input_prompt, 4);
  return status;
}
