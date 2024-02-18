#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#include <stdint.h>

extern void unity_config_put_c(uint8_t a);

#define UNITY_OUTPUT_CHAR(a) \
  unity_config_put_c(a)
#define UNITY_PRINT_EOL() \
  unity_config_put_c('\r'); \
  unity_config_put_c('\n')

#include "unity.h"

#endif
