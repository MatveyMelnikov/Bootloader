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
  "Write to memory (2 bytes) - '3';\r\n"
  "Erase - '4';\r\n"
  "Read pages from flash - '5'.\r\n";
static char *id_message = "\r\nChip ID: ";
static char *bootloader_version_message = "\r\nBootloader version: ";
static char *read_message = "\r\nEnter flash page number(000 - 127): ";

static uint8_t uart_buffer[UART_BUFFER_SIZE];
static char* hex_symbols = "0123456789ABCDEF";

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

__attribute__((always_inline))
inline static void get_hex_string(char *const buffer, uint8_t num)
{
  buffer[0] = hex_symbols[(num & 0xf0) >> 4];
  buffer[1] = hex_symbols[num & 0x0f];
}

static void send_response(bootloader_status status)
{
  if (status)
    uart_buffer[0] = NACK_BYTE;
  else
    uart_buffer[0] = ACK_BYTE;
  (void)bootloader_io_write(uart_buffer, 1);
}

static bool is_not_num(const char input)
{
  return input < '0' || input > '9';
}

static uint8_t read_string()
{
  bootloader_status status = BOOTLOADER_OK;
  int8_t index = 0;
  uint8_t page = 0;

  memset(uart_buffer, 0, 3);

  while (true)
  {
    status = bootloader_io_read(uart_buffer + index, sizeof(char));

    if ((status == BOOTLOADER_TIMEOUT) | is_not_num(uart_buffer[index]))
      continue;

    (void)bootloader_io_write(uart_buffer + index, 1);

    index++;
    if (index > 2)
      break;
  }

  int8_t rank = 1;
  for (int8_t i = index - 1; i >= 0; i--)
  {
    page += (uart_buffer[i] - '0') * rank;
    rank *= 10;
  }

  return page;
}

static void cmd_help()
{
  uint16_t message_size = strlen(commands_list_message) + 1;

  strncpy((char*)uart_buffer, commands_list_message, message_size);
  bootloader_io_write(uart_buffer, message_size);
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
  uint16_t message_size = strlen(bootloader_version_message) + 1;

  strncpy((char*)uart_buffer, bootloader_version_message, message_size);
  bootloader_io_write(uart_buffer, message_size);

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
  uint32_t num = 0;
  bootloader_status status = BOOTLOADER_OK;

  send_response(status);
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
    num++;
  }

  return status;
}

// cmd_0: address (4 bytes)
// cmd_0: pages to erase (1 byte)
static bootloader_status cmd_erase()
{
  uint32_t address = 0;
  uint8_t page_num = 0;
  bootloader_status status = BOOTLOADER_OK;

  send_response(status);
  status = bootloader_io_read(
    (uint8_t*)&address,
    sizeof(uint32_t)
  );

  status |= bootloader_io_read(&page_num, sizeof(uint8_t));

  if (status == BOOTLOADER_OK)
    status |= bootloader_io_erase(address, page_num);
  send_response(status);

  return status;
}

static bootloader_status cmd_read()
{
  uint8_t page_num = 0;
  uint8_t value = 0;
  bootloader_status status = BOOTLOADER_OK;

  strncpy((char*)uart_buffer, read_message, strlen(read_message) + 1);
  bootloader_io_write(uart_buffer, strlen(read_message) + 1);

  page_num = read_string();

  if (page_num < 0 || page_num >= 128)
    return BOOTLOADER_BOUNDS_ERROR;

  strncpy((char*)uart_buffer, "\r\n", 3);
  bootloader_io_write(uart_buffer, 2);

  strncpy((char*)uart_buffer + 2, " ", 2);
  for (uint8_t i = 0; i < 16; i++)
  {
    get_hex_string((char*)uart_buffer, i);
    bootloader_io_write(uart_buffer, 4);
  }

  strncpy((char*)uart_buffer, "\r\n", 3);
  bootloader_io_write(uart_buffer, 2);
  bootloader_io_write(uart_buffer, 2);

  uart_buffer[2] = ' ';
  uint32_t address = 0x08000000 + (0x400 * page_num);
  for (uint8_t i = 0; i < 64; i++)
  {
    for (uint8_t j = 0; j < 16; j++)
    {
      status |= bootloader_io_read_flash(address++, &value);
      if (status)
        return status;

      get_hex_string((char*)uart_buffer, value);
      bootloader_io_write(uart_buffer, 3);
    }
    strncpy((char*)uart_buffer, "\r\n", 3);
    bootloader_io_write(uart_buffer, 2);
  }

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
    case CMD_READ:
      status |= cmd_read();
      break;
  }

  status |= bootloader_io_write((uint8_t*)input_prompt, 4);
  return status;
}
