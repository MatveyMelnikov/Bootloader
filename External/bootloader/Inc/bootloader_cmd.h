#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "bootloader_cmd.h"
#include "bootloader_defs.h"
#include "bootloader_io.h"
#include <stdint.h>

bootloader_status bootloader_start_output(void);
bootloader_status bootloader_proccess_input(void);

#endif
