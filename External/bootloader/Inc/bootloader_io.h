#ifndef BOOTLOADER_IO_H
#define BOOTLOADER_IO_H

#include <stdint.h>
#include "bootloader_defs.h"

bootloader_status bootloader_io_read(
  uint8_t *const data,
  const uint16_t size
);
bootloader_status bootloader_io_write(
  const uint8_t *const data,
  const uint16_t size
);
uint32_t bootloader_io_get_dev_id(void);
bootloader_status bootloader_io_program(
  const uint32_t address,
  const uint16_t data
);
bootloader_status bootloader_io_erase(
  const uint32_t address,
  const uint8_t pages_num
);
bootloader_status bootloader_io_read_flash(
  const uint32_t address,
  uint8_t *const value
);

#endif
