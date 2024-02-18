#include "unity_fixture.h"

TEST_GROUP_RUNNER(bootloader)
{
  RUN_TEST_CASE(bootloader, start_output_success);
  RUN_TEST_CASE(bootloader, output_help_success);
  RUN_TEST_CASE(bootloader, output_get_id);
  RUN_TEST_CASE(bootloader, output_bootloader_id);
  RUN_TEST_CASE(bootloader, write_success);
  RUN_TEST_CASE(bootloader, write_bound_error);
  RUN_TEST_CASE(bootloader, write_middle_success);
  RUN_TEST_CASE(bootloader, erase_success);
  RUN_TEST_CASE(bootloader, erase_start_bound_error);
  RUN_TEST_CASE(bootloader, erase_end_bound_error);
}