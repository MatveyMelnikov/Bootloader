#ifndef BOOTLOADER_IO_H
#define BOOTLOADER_IO_H

#include <stdint.h>
#include "bootloader_defs.h"

bootloader_status bootloader_io_read(
    const uint8_t *const data,
    const uint16_t size
);

bootloader_status bootloader_io_write(
    uint8_t *const data,
    const uint16_t size
);

uint32_t bootloader_io_get_dev_id(void);

#endif
