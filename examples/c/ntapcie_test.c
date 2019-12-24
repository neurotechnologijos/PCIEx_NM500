/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ntapcie_func.h"

#include "ntia_api.h"
#include "ntia_api_ll.h"

static struct nta_dev_handle_t nta_dev_handle;

static void do_main_menu(struct nta_dev_handle_t* const dev_handle);
static void print_error(uint32_t _error);

int main(int argc, const char* const argv[])
{
  enum ntpcie_nn_error_t nn_result;
  struct nta_pcidev_list_t devs_list;

  puts("\n");
  puts("-- NT Adaptive PCIe X NM500 card test/demo program --");
  puts("-- v20191224 --- MIT license ---                   --");
  puts("-- Copyright (c) 2017-2019 NeuroTechnologijos UAB  --");
  puts("-- http://www.neurotechnologijos.com/              --");
  puts("-- http://neurotech.lt/                            --");
  puts("-- https://github.com/neurotechnologijos/          --");
  puts("\n");

  // startup sequnce
  nn_result = ntpcie_sys_init(&nta_dev_handle, &devs_list);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    puts("error: ntpcie_sys_init failed");
    print_error(nn_result);
    return EXIT_FAILURE;
  } else
  {
    if (devs_list.devs_count == 0)
    {
      puts("error: no NT Adaptive PCIe X NM500 card found");
      ntpcie_sys_deinit(&nta_dev_handle);
      return EXIT_FAILURE;
    }
  }

  size_t device_select = 0;

  printf("found %" PRIu64 " 'NT Adaptive PCIe X NM500' card(s) (%04" PRIx16 ":%04" PRIx16 ")\n",
         devs_list.devs_count, devs_list.pci_id_vendor, devs_list.pci_id_device);
  if (devs_list.devs_count > 1)
  {
    do
    {
      puts(" list of available devices, please select:");
      for (size_t ix = 0; ix < devs_list.devs_count; ++ix)
      {
        printf("   %" PRIu64 ". card - bus:0x%02" PRIx16 "; slot:0x%02" PRIx16 "; func:0x%1" PRIx16 "\n",
               ix, devs_list.devices[ix].bus, devs_list.devices[ix].slot, devs_list.devices[ix].func);
      }
      fputs(" input number from list: ", stdout);
      scanf("%" PRIu64, &device_select);
    } while (device_select > devs_list.devs_count-1);
  }

  nn_result = ntpcie_device_open(&nta_dev_handle,
                                 devs_list.devices[device_select].bus,
                                 devs_list.devices[device_select].slot);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    puts("error: ntpcie_device_open failed");
    print_error(nn_result);
    return EXIT_FAILURE;
  }

  puts(" PCIe card and Neuron net have initialized\n");

  card_view_info(&nta_dev_handle);

  if (argc == 1)
  {
    // --- main loop with user interactive interface -------------------
    do_main_menu(&nta_dev_handle);
    // -----------------------------------------------------------------
  }

  // cleanup before exit
  nn_result = ntpcie_sys_deinit(&nta_dev_handle);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    puts("error: ntpcie_sys_deinit failed");
    print_error(nn_result);
  }
  else
  {
    puts("ntpcie_sys_deinit - OK");
  }

  puts("------ PROGRAM HAS STOPPED ---------");
  return EXIT_SUCCESS;
}

static void print_error(uint32_t _error)
{
  printf("error code - 0x%08" PRIx32 "\n", _error);
  puts(ntpcie_error_text(_error));
}

typedef void (*action_t)(struct nta_dev_handle_t* const);

struct menu_item_t
{
  const char* const item_text;
  const action_t item_action;
};

// clang-format off
// *INDENT-OFF*

static const struct menu_item_t main_menu[] = {
  { "Exit",                          NULL },
  { "card: reset (hard)",            &card_reset },
  { "card: reset stress test",       &card_reset_stress_test },
  { "card: view info",               &card_view_info },
  { "NN:   simple test",             &nntest_simple_test },
  { "NN:   full random test",        &nntest_full_test },
  { "NN:   register read",           &nntest__register_read },
  { "NN:   register write",          &nntest_register_write },
  { "NN:   neuron dump",             &nntest_neuron_dump },
  { "NN:   forget ALL (soft reset)", &nntest_forget_all },
  { "KB:   KB store",                &nntest_kb_store },
  { "KB:   KB load",                 &nntest_kb_load },
  { "KB:   KB's compare",            &nntest_kbs_compare },
  { NULL, NULL },
};

// *INDENT-ON*
// clang-format on

static void do_main_menu(struct nta_dev_handle_t* const dev_handle)
{
  unsigned int sel_item = 0;

  size_t main_menu_items_count;

  for (main_menu_items_count = 0;
       main_menu[main_menu_items_count].item_text != NULL;
       ++main_menu_items_count)
    ;

  do
  {
    puts("--- Main Menu -----------------\n");
    for (size_t ix = 0; ix < main_menu_items_count; ix++)
    {
      printf("%2" PRIu64 ". %s\n", ix, main_menu[ix].item_text);
    }
    puts("-------------------------------\n");
    fputs(" Enter menu index: ", stdout);
    scanf("%u", &sel_item);

    if (sel_item + 1 > main_menu_items_count)
    {
      puts(" BAD menu index");
      continue;
    }

    if (main_menu[sel_item].item_action != NULL)
    {
      main_menu[sel_item].item_action(dev_handle);
    }

    puts("");
  } while (sel_item > 0);
}
