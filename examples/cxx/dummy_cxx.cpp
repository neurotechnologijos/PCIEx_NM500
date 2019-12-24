/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <cstddef>
#include <cstdlib>

#include "ntia_api.h"
#include "ntia_api_data_types.h"
#include "ntia_api_data_types_ll.h"
#include "ntia_api_ll.h"

// fake project for testing *api*.h in CXX compile mode

int main(int, const char* const*)
{
  constexpr bool run_enable = false;
  static struct nta_dev_handle_t nta_dev_handle;
  struct nta_pcidev_list_t devs_list;

  if (run_enable)
  {
    ntpcie_sys_init(&nta_dev_handle, &devs_list);
    ntpcie_device_open(&nta_dev_handle, 0, 0);
    ntpcie_device_close(&nta_dev_handle);
    ntpcie_sys_deinit(&nta_dev_handle);
  }
  return EXIT_SUCCESS;
}
