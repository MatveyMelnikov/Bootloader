#ifndef MOCK_BOOTLOADER_IO_H
#define MOCK_BOOTLOADER_IO_H

#include "bootloader_io.h"

void mock_bootloader_io_create(const uint8_t max_expectations);
void mock_bootloader_io_destroy(void);
void mock_bootloader_io_expect_write(
    const uint8_t *const data,
    const uint8_t data_size
);
void mock_bootloader_io_expect_program(
    const uint8_t *const data
);
void mock_bootloader_io_expect_read_then_return(
    const uint8_t *const data,
    const uint8_t data_size
);
void mock_bootloader_io_expect_get_id_then_return(void);
void mock_bootloader_io_verify_complete(void);

#endif
