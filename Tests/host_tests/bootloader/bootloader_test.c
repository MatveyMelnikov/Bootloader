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
    mock_bootloader_io_create(10);
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

    mock_bootloader_io_expect_write(start_message, strlen(start_message) + 1);
    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1); 

    bootloader_status status = bootloader_start_output();

    TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, output_help_success)
{
    static char *commands_list_message = "\r\nCommands:\r\n"
        "Get id of chip - '1';\r\n"
        "Get bootloader version - '2';\r\n"
        "Write to memory (2 bytes) - '3'.";
    static char *input_data = "0";
    char *input_prompt = "\r\n>>";

    mock_bootloader_io_expect_read_then_return(input_data, 1);
    mock_bootloader_io_expect_write(input_data, 1);
    mock_bootloader_io_expect_write(
        commands_list_message,
        strlen(commands_list_message) + 1
    );
    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1);

    bootloader_status status = bootloader_proccess_input();

    TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, output_get_id)
{
    static char *input_data = "1";
    static char *id_message = "\r\nChip ID: ";
    static char *id_num_message = "1034";
    static char *input_prompt = "\r\n>>";

    mock_bootloader_io_expect_read_then_return(input_data, 1);
    mock_bootloader_io_expect_write(input_data, 1);
    mock_bootloader_io_expect_write(id_message, strlen(id_message) + 1);
    mock_bootloader_io_expect_get_id_then_return();
    mock_bootloader_io_expect_write(id_num_message, strlen(id_num_message) + 1);
    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1);

    bootloader_status status = bootloader_proccess_input();

    TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, output_bootloader_id)
{
    static char *input_data = "2";
    static char *bootloader_version_message = "\r\nBootloader version: ";
    static char *version_message = "0.1";
    static char *input_prompt = "\r\n>>";

    mock_bootloader_io_expect_read_then_return(input_data, 1);
    mock_bootloader_io_expect_write(input_data, 1);
    mock_bootloader_io_expect_write(
        bootloader_version_message,
        strlen(bootloader_version_message) + 1
    );
    mock_bootloader_io_expect_write(version_message, 3);

    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1);

    bootloader_status status = bootloader_proccess_input();

    TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, write_success)
{
    static char *input_cmd = "3";
    static uint32_t input_addr = APP_START_ADDRESS;
    static uint32_t input_data = 0xf5533aa;
    static char *input_prompt = "\r\n>>";
    static uint8_t ack_byte = ACK_BYTE;

    mock_bootloader_io_expect_read_then_return(input_cmd, 1);
    mock_bootloader_io_expect_write(input_cmd, 1);
    mock_bootloader_io_expect_read_then_return(
        (uint8_t*)&input_addr,
        sizeof(input_addr)
    );
    mock_bootloader_io_expect_write(&ack_byte, 1);
    mock_bootloader_io_expect_read_then_return(
        (uint8_t*)&input_data,
        sizeof(input_data)
    );
    mock_bootloader_io_expect_write(&ack_byte, 1);
    mock_bootloader_io_expect_program(&input_data);
    mock_bootloader_io_expect_write(&ack_byte, 1);

    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1);

    bootloader_status status = bootloader_proccess_input();

    TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}

TEST(bootloader, write_bound_error)
{
    static char *input_cmd = "3";
    static uint32_t input_addr = 0x08000000 + 0x400 * 128;
    static uint32_t input_data = 0xf5533aa;
    static char *input_prompt = "\r\n>>";
    static uint8_t ack_byte = ACK_BYTE;
    static uint8_t nack_byte = NACK_BYTE;

    mock_bootloader_io_expect_read_then_return(input_cmd, 1);
    mock_bootloader_io_expect_write(input_cmd, 1);
    mock_bootloader_io_expect_read_then_return(
        (uint8_t*)&input_addr,
        sizeof(input_addr)
    );
    mock_bootloader_io_expect_write(&ack_byte, 1);
    mock_bootloader_io_expect_read_then_return(
        (uint8_t*)&input_data,
        sizeof(input_data)
    );
    mock_bootloader_io_expect_write(&ack_byte, 1);
    mock_bootloader_io_expect_program(&input_data);
    mock_bootloader_io_expect_write(&nack_byte, 1);

    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1);

    bootloader_status status = bootloader_proccess_input();

    TEST_ASSERT_EQUAL(BOOTLOADER_BOUNDS_ERROR, status);
}

TEST(bootloader, write_middle_success)
{
    static char *input_cmd = "3";
    static uint32_t input_addr = 0x08000000 + 0x400 * 63;
    static uint32_t input_data = 0xf5533aa;
    static char *input_prompt = "\r\n>>";
    static uint8_t ack_byte = ACK_BYTE;

    mock_bootloader_io_expect_read_then_return(input_cmd, 1);
    mock_bootloader_io_expect_write(input_cmd, 1);
    mock_bootloader_io_expect_read_then_return(
        (uint8_t*)&input_addr,
        sizeof(input_addr)
    );
    mock_bootloader_io_expect_write(&ack_byte, 1);
    mock_bootloader_io_expect_read_then_return(
        (uint8_t*)&input_data,
        sizeof(input_data)
    );
    mock_bootloader_io_expect_write(&ack_byte, 1);
    mock_bootloader_io_expect_program(&input_data);
    mock_bootloader_io_expect_write(&ack_byte, 1);

    mock_bootloader_io_expect_write(input_prompt, strlen(input_prompt) + 1);

    bootloader_status status = bootloader_proccess_input();

    TEST_ASSERT_EQUAL(BOOTLOADER_OK, status);
}
