#include "mock_bootloader_io.h"
#include "unity_fixture.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
    int kind;
    const uint8_t * data;
    uint8_t data_size;
} expectation;

enum
{
    IO_READ,
    IO_WRITE,
    IO_DEV_ID,
    IO_PROGRAM,
    NO_EXPECTED_VALUE = -1,
    BOOTLOADER_ID = 1034
};

static char *report_not_init = "MockIO not initialized";
static char *report_no_room_for_exp = "No room for expectations in MockIO";
static char *report_verify_error = "Verify error in MockIO. Expected %u "
    "operations, but got %u";
static char *report_kind_error = "Error kind in MockIO."
    " Num of expectation %u ";
static char *report_addr_error = "Error addr in MockIO."
    " Num of expectation %u ";
static char *report_data_error = "Error data in MockIO."
    " Expected %u, but got %u";

static expectation *expectations = NULL;
static int set_expectation_count;
static int get_expectation_count;
static int max_expectation_count;

// Static functions ----------------------------------------------------------

static void fail_when_no_init()
{
    if (expectations == NULL)
        FAIL(report_not_init);
}

static void fail_when_no_room_for_expectations()
{
    fail_when_no_init();
    if (set_expectation_count >= max_expectation_count)
        FAIL(report_no_room_for_exp);
}


static void record_expectation(
    const int kind,
    const uint8_t *const data,
    const uint8_t data_size
)
{
    expectations[set_expectation_count].kind = kind;
    expectations[set_expectation_count].data = data;
    expectations[set_expectation_count].data_size = data_size;
    set_expectation_count++;
}

static void check_kind(const expectation *const current_expectation, int kind)
{
    char *message[sizeof(report_kind_error) + 10];

    if (current_expectation->kind == kind)
        return;

    sprintf((char*)message, report_kind_error, get_expectation_count);
    FAIL((char*)message);
}

static void check_data(
    const expectation *const current_expectation,
    const uint8_t *const data
)
{
    char *message[sizeof(report_data_error) + 10];

    bool fail = false;
    for (uint8_t i = 0; i < current_expectation->data_size; i++)
    {
        if (current_expectation->data[i] != data[i])
        {
            fail = true;
            break;
        }
    }

    if (!fail)
        return;

    sprintf((char*)message, report_data_error, get_expectation_count);
    FAIL((char*)message);
}


// Implementations -----------------------------------------------------------

void mock_bootloader_io_create(const uint8_t max_expectations)
{
    if (expectations != NULL)
        free(expectations);
    expectations = calloc(max_expectations, sizeof(expectation));
    max_expectation_count = max_expectations;
}

void mock_bootloader_io_destroy(void)
{
    if (expectations)
        free(expectations);
    expectations = NULL;

    set_expectation_count = 0;
    get_expectation_count = 0;
    max_expectation_count = 0;
}

void mock_bootloader_io_expect_write(
    const uint8_t *const  data,
    const uint8_t data_size
)
{
    fail_when_no_room_for_expectations();
    record_expectation(IO_WRITE, data, data_size);
}

void mock_bootloader_io_expect_program(
    const uint8_t *const data
)
{
    fail_when_no_room_for_expectations();
    record_expectation(IO_PROGRAM, data, sizeof(uint32_t));
}

void mock_bootloader_io_expect_read_then_return(
    const uint8_t *const  data,
    const uint8_t data_size
)
{
    fail_when_no_room_for_expectations();
    record_expectation(IO_READ, data, data_size);
}

void mock_bootloader_io_expect_get_id_then_return(void)
{
    fail_when_no_room_for_expectations();
    record_expectation(IO_DEV_ID, NULL, 0);
}

void mock_bootloader_io_verify_complete(void)
{
    char *message[sizeof(report_verify_error) + 10];

    if (set_expectation_count == get_expectation_count)
        return;
    
    sprintf(
        (char*)message,
        report_verify_error,
        set_expectation_count,
        get_expectation_count
    );
    FAIL((char*)message);
}

// Implementations from bootloaderm_io --------------------------------------------

bootloader_status bootloader_io_read(
    const uint8_t *const data,
    const uint16_t size
) 
{
    expectation current_expectation = expectations[get_expectation_count];

    fail_when_no_init();
    check_kind(&current_expectation, IO_READ);

    memcpy((uint8_t*)data, current_expectation.data, size);
    
    get_expectation_count++;
    return BOOTLOADER_OK;
}

bootloader_status bootloader_io_write(
    uint8_t *const data,
    const uint16_t size
)
{
    expectation current_expectation = expectations[get_expectation_count];

    fail_when_no_init();
    check_kind(&current_expectation, IO_WRITE);
    check_data(&current_expectation, data);

    get_expectation_count++;

    return BOOTLOADER_OK;
}

uint32_t bootloader_io_get_dev_id()
{
    expectation current_expectation = expectations[get_expectation_count];

    fail_when_no_init();
    check_kind(&current_expectation, IO_DEV_ID);
    
    get_expectation_count++;
    return BOOTLOADER_ID;
}

bootloader_status bootloader_io_program(
    uint32_t address,
    const uint8_t *const data
)
{
    bootloader_status status = BOOTLOADER_OK;

    // 128 pages
    if (
        address < APP_START_ADDRESS || 
        address + sizeof(uint32_t) >= 0x0801FFFFUL
    )
        status = BOOTLOADER_BOUNDS_ERROR;

    expectation current_expectation = expectations[get_expectation_count];

    fail_when_no_init();
    check_kind(&current_expectation, IO_PROGRAM);

    memcpy(
        (uint8_t*)data,
        current_expectation.data,
        current_expectation.data_size
    );
    
    get_expectation_count++;
    return status;
}
