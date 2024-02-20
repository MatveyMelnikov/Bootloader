#include "unity_fixture.h"
#include "bootloader_cmd.h"
#include "mock_bootloader_io.h"
#include <string.h>

// Defines -------------------------------------------------------------------

#define ADD_ADDR_IN_BYTES(arr, addr) \
  arr[0] = (uint8_t)((addr) >> 8); \
  arr[1] = (uint8_t)(addr)

// Static functions ----------------------------------------------------------

static void check_data(
  const uint8_t *const expected,
  const uint8_t *const actual,
  const uint16_t data_size
)
{
  for (uint16_t i = 0; i < data_size; i++)
    TEST_ASSERT_BYTES_EQUAL(expected[i], actual[i]);
}

// Tests ---------------------------------------------------------------------

TEST_GROUP(bootloader);

TEST_SETUP(bootloader)
{
  mock_bootloader_io_create(30);
}

TEST_TEAR_DOWN(bootloader)
{
  mock_bootloader_io_verify_complete();
  mock_bootloader_io_destroy();
}

TEST(bootloader, start_output_success)
{
  static char *start_message = "-FlexyPixel Bootloader."
    " Type '0' for commands list";
  char *input_prompt = "\r\n>>";

  mock_bootloader_io_expect_write(
    (uint8_t*)start_message,
    strlen(start_message) + 1
  );
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  ); 

  bootloader_status status = bootloader_start_output();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, output_help_success)
{
  static char *commands_list_message = "\r\nCommands:\r\n"
  "Get id of chip - '1';\r\n"
  "Get bootloader version - '2';\r\n"
  "Write to memory (2 bytes) - '3';\r\n"
  "Erase.";
  static char *input_data = "0";
  char *input_prompt = "\r\n>>";

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_data, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)commands_list_message,
    strlen(commands_list_message) + 1
  );
  mock_bootloader_io_expect_write((uint8_t*)input_data, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, output_get_id)
{
  static char *input_cmd = "1";
  static char *id_message = "\r\nChip ID: ";
  static char *id_num_message = "1034";
  static char *input_prompt = "\r\n>>";

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)id_message,
    strlen(id_message) + 1
  );
  mock_bootloader_io_expect_get_id_then_return();
  mock_bootloader_io_expect_write(
    (uint8_t*)id_num_message,
    strlen(id_num_message) + 1
  );
  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, output_bootloader_id)
{
  static char *input_cmd = "2";
  static char *bootloader_version_message = "\r\nBootloader version: ";
  static char *version_message = "0.1";
  static char *input_prompt = "\r\n>>";

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)bootloader_version_message,
    strlen(bootloader_version_message) + 1
  );
  mock_bootloader_io_expect_write((uint8_t*)version_message, 3);

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, write_success)
{
  static char *input_cmd = "3";
  static uint32_t input_addr[4] = {
    APP_START_ADDRESS,
    APP_START_ADDRESS + 2U,
    APP_START_ADDRESS + 4U,
    APP_START_ADDRESS + 6U
  };
  static uint16_t input_data[4] = { 0xf553, 0x3344, 0x55aa, 0xff00 };
  static char *input_prompt = "\r\n>>";
  static uint8_t ack_byte = ACK_BYTE;
  static uint32_t end_seq = END_SUBSEQUENCE;

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);

  mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  for (uint8_t i = 0; i < 4; i++)
  {
    mock_bootloader_io_expect_read_then_return(
      (uint8_t*)&input_addr[i],
      sizeof(uint32_t)
    );
    mock_bootloader_io_expect_read_then_return(
      (uint8_t*)&input_data[i],
      sizeof(uint16_t)
    );
    mock_bootloader_io_expect_program((uint8_t*)&input_data[i]);
    mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  }
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&end_seq,
    sizeof(end_seq)
  );

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, write_bound_error)
{
  static char *input_cmd = "3";
  static uint32_t input_addr = 0x08000000 + 0x400 * 128;
  static uint16_t input_data = 0x33aa;
  static char *input_prompt = "\r\n>>";
  static uint8_t ack_byte = ACK_BYTE;
  static uint8_t nack_byte = NACK_BYTE;

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);

  mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_addr,
    sizeof(uint32_t)
  );
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_data,
    sizeof(uint16_t)
  );
  mock_bootloader_io_expect_program((uint8_t*)&input_data);
  mock_bootloader_io_expect_write(&nack_byte, sizeof(nack_byte));

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_BOUNDS_ERROR, status);
}

TEST(bootloader, write_middle_success)
{
  static char *input_cmd = "3";
  static const uint32_t start_address = 0x08000000 + 0x400 * 63;
  static uint32_t input_addr[4] = {
    start_address,
    start_address + 2U,
    start_address + 4U,
    start_address + 6U
  };
  static uint16_t input_data[4] = { 0x5555, 0xaaaa, 0x34, 0xcc00 };
  static char *input_prompt = "\r\n>>";
  static uint8_t ack_byte = ACK_BYTE;
  static uint32_t end_seq = END_SUBSEQUENCE;

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);

  mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  for (uint8_t i = 0; i < 4; i++)
  {
    mock_bootloader_io_expect_read_then_return(
      (uint8_t*)&input_addr[i],
      sizeof(uint32_t)
    );
    mock_bootloader_io_expect_read_then_return(
      (uint8_t*)&input_data[i],
      sizeof(uint16_t)
    );
    mock_bootloader_io_expect_program((uint8_t*)&input_data[i]);
    mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  }
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&end_seq,
    sizeof(end_seq)
  );

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, erase_success)
{
  static char *input_cmd = "4";
  static uint32_t input_addr = 0x08000000 + 0x400 * 63;
  static uint8_t input_page_num = 32;
  static char *input_prompt = "\r\n>>";
  static uint8_t ack_byte = ACK_BYTE;

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);

  mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_addr,
    sizeof(input_addr)
  );
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_page_num,
    sizeof(input_page_num)
  );
  mock_bootloader_io_expect_erase((uint8_t*)&input_addr, input_page_num);
  mock_bootloader_io_expect_write(&ack_byte, 1);

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, erase_start_bound_error)
{
  static char *input_cmd = "4";
  static uint32_t input_addr = APP_START_ADDRESS - 1;
  static uint8_t input_page_num = 32;
  static char *input_prompt = "\r\n>>";
  static uint8_t nack_byte = NACK_BYTE;
  static uint8_t ack_byte = ACK_BYTE;

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);

  mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_addr,
    sizeof(input_addr)
  );
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_page_num,
    sizeof(input_page_num)
  );
  mock_bootloader_io_expect_erase((uint8_t*)&input_addr, input_page_num);
  mock_bootloader_io_expect_write(&nack_byte, 1);

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_BOUNDS_ERROR, status);
}

TEST(bootloader, erase_end_bound_error)
{
  static char *input_cmd = "4";
  static uint32_t input_addr = 0x08000000 + 0x400 * 128;
  static uint8_t input_page_num = 32;
  static char *input_prompt = "\r\n>>";
  static uint8_t nack_byte = NACK_BYTE;
  static uint8_t ack_byte = ACK_BYTE;

  mock_bootloader_io_expect_read_then_return((uint8_t*)input_cmd, 1);

  mock_bootloader_io_expect_write(&ack_byte, sizeof(ack_byte));
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_addr,
    sizeof(input_addr)
  );
  mock_bootloader_io_expect_read_then_return(
    (uint8_t*)&input_page_num,
    sizeof(input_page_num)
  );
  mock_bootloader_io_expect_erase((uint8_t*)&input_addr, input_page_num);
  mock_bootloader_io_expect_write(&nack_byte, 1);

  mock_bootloader_io_expect_write((uint8_t*)input_cmd, 1);
  mock_bootloader_io_expect_write(
    (uint8_t*)input_prompt,
    strlen(input_prompt) + 1
  );

  bootloader_status status = bootloader_proccess_input();

  TEST_ASSERT_EQUAL(BOOTLOADER_BOUNDS_ERROR, status);
}
