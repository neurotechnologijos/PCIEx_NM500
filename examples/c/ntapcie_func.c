/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ntia_api.h"
#include "ntia_api_data_types.h"
#include "ntia_api_data_types_ll.h"
#include "ntia_api_ll.h"

#include "ntapcie_func.h"

#include "rand_simple.h"
#include "getCPUtime.h"

// ------------------------- global data ----------------------------------

// working area for massive learn/classify test
static struct test_data_t test_data[MAX_NEURONS_COUNT];

// KB storage in main memory (RAM) - for store/load functions
static struct nn_sample_kb_t test_kbs_array[MAX_KBS_SLOTS];

// ------------------------------------------------------------------------

void ntpcie_error_viewer(uint32_t const _error_code)
{
  printf("ec = %" PRIx32 ": %s", _error_code, ntpcie_error_text(_error_code));
}

void nntest__register_read(struct nta_dev_handle_t* const dev_handle)
{
  uint16_t address = 0;
  uint16_t value   = 0;

  enum ntpcie_nn_error_t nn_result;

  printf("\n-- READ REGISTER --\n");
  printf(" input number of register: 0x");
  scanf("%hx", &address);
  nn_result = ntpcie_nn_register_read(dev_handle, (enum nn_int_register_t)(address), &value);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    ntpcie_error_viewer(nn_result);
    return;
  }
  else
  {
    printf(" reg address 0x%04" PRIx16 " -> 0x%04" PRIx16 "\n", address, value);
  }
}

void nntest_register_write(struct nta_dev_handle_t* const dev_handle)
{
  uint16_t address = 0;
  uint16_t value   = 0;
  enum ntpcie_nn_error_t nn_result;

  puts("\n-- WRITE REGISTER --");
  fputs(" input number of register: 0x", stdout);
  scanf("%hx", &address);
  fputs(" input data to write:      0x", stdout);
  scanf("%hx", &value);
  nn_result = ntpcie_nn_register_write(dev_handle, (enum nn_int_register_t)(address), value);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    ntpcie_error_viewer(nn_result);
  }
}

void nntest_neuron_dump(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  uint16_t neuron_index = 0;
  size_t neurons_commited;
  struct nn_neuron_t neuron;

  neurons_commited = dev_handle->nn_state.neurons_committed;

  printf("\n-- READ SELECTED NEURON --\n");
  if (neurons_commited < 1)
  {
    puts(" ERROR: committed neurons is ZERO");
#ifdef NTIAPCIE_DEBUG
    puts(" *** NTIAPCIE_DEBUG active");
#else
    return;
#endif // NTIAPCIE_DEBUG
  }
  else
  {
    printf("\n There are %zu committed neurons\n", neurons_commited);
  }
  printf(" input number of neuron [0..%" PRIu64 "]: ", (neurons_commited > 0) ? (neurons_commited-1) : 0);
  scanf("%" PRIu16, &neuron_index);
  if (neuron_index + 1 > neurons_commited)
  {
#ifdef NTIAPCIE_DEBUG
    puts(" *** NTIAPCIE_DEBUG active");
    puts(" WARNING: number of neuron is out from range [0..neurons_commited-1]");
#else
    puts(" ERROR: number of neuron is out from range [0..neurons_commited-1]");
    return;
#endif // NTIAPCIE_DEBUG

  }

  nn_result = ntpcie_nn_neuron_read(dev_handle, neuron_index, &neuron);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    printf(" ERROR - ");
    ntpcie_error_viewer(nn_result);
  }
  else
  {
    fprintf(stderr, "context  %5d\n", neuron.ncr);
    fprintf(stderr, "category %5d\n", neuron.category);
    fprintf(stderr, "AIF      %5d\n", neuron.aif);
    fprintf(stderr, "MINIF    %5d\n", neuron.minif);
    fputs("components:\n", stderr);
    for (size_t i = 0; i < NN_NEURON_COMPONENTS; i++)
    {
      fprintf(stderr, " 0x%02x", neuron.comp[i]);
      if ((i & 15) == 15)
      {
        fputs("\n", stderr);
      }
    }
    fputs("\n", stderr);
  }
}

void card_reset(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  nn_result = ntpcie_device_reset(dev_handle);
  if (nn_result == NTPCIE_ERROR_SUCCESS)
  {
    puts(" card RESET - OK");
  }
  else
  {
    puts(" card RESET - failed\n");
    ntpcie_error_viewer(nn_result);
  }
}

void nntest_forget_all(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result;

  nn_result = ntpcie_nn_reset(dev_handle);
  if (nn_result == NTPCIE_ERROR_SUCCESS)
  {
    puts(" NN FORGET (soft reset) - OK");
  }
  else
  {
    puts(" NN FORGET (soft reset) - failed");
    ntpcie_error_viewer(nn_result);
  }
}

void nntest_full_test(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t result = NTPCIE_ERROR_SUCCESS;
  size_t cycles = 0, cnt = 0, cntl = 0, cntr = 0, cnti = 0, cntn = 0, cnt_dist_not_0 = 0;

  size_t neurons_count = 0;
#ifdef NTIAPCIE_DEBUG
  puts(" *** NTIAPCIE_DEBUG active");
  fputs(" Input amount of neurons (force) [1...16384]: ", stdout);
  scanf("%" PRIu64, &neurons_count);
#else
  neurons_count = (dev_handle->nn_state.neurons_overall > MAX_NEURONS_COUNT) ? MAX_NEURONS_COUNT : dev_handle->nn_state.neurons_overall;
#endif // NTIAPCIE_DEBUG

  fputs(" Input amount of cycles [1...65535]: ", stdout);
  scanf("%" PRIu64, &cycles);

  rand_simple_set_seed_auto();

  while (cnt < cycles)
  {
    printf("--- Step --- #%zu\n", cnt);

    cnt++;

    cntl           = 0;
    cntr           = 0;
    cnti           = 0;
    cntn           = 0;
    cnt_dist_not_0 = 0;

    const uint16_t comps_to_test = 256;

    // forget ALL
    result = ntpcie_nn_reset(dev_handle);
    if (result != NTPCIE_ERROR_SUCCESS)
    {
      puts("... FORGET before LEARN failed ...");
      ntpcie_error_viewer(result);
      return;
    }

    // prepare dataset
    for (size_t i = 0; i < neurons_count; ++i)
    {
      test_data[i].context  = 2;
      test_data[i].category = (rand_simple() & 0x00007FFEu);
      if (test_data[i].category == 0)
      {
        test_data[i].category = 23;
      }

      for (size_t ix = 0; ix < comps_to_test; ++ix)
      {
        test_data[i].comps[ix] = (rand_simple() & 0x000000FFu);
      }
    }

    printf("test data has been created (neurons = %" PRIu64 ")\n", neurons_count);

    double time_start, time_stop;
    uint64_t usec_duration;

    // learn

    time_start = getCPUTime();
    for (size_t i = 0; i < neurons_count; ++i)
    {
      cntl++;

      result = ntpcie_nn_vector_learn(dev_handle, NN_DIST_EVAL_L1, test_data[i].context, test_data[i].category, NN_DEF_MAXIF, NN_DEF_MINIF, comps_to_test,
                                      &test_data[i].comps[0]);
      if (result != NTPCIE_ERROR_SUCCESS)
      {
        puts("... Learn operation is fault ...\n");
        ntpcie_error_viewer(result);
        return;
      }
    }
    time_stop = getCPUTime();

    usec_duration = (uint64_t)((time_stop - time_start) * 1000000.0);

    // TODO: prevent divide to ZERO

    printf(" Loaded %" PRIu64 " for learn vectors, committed neurons = %" PRIu64 "\n", cntl, dev_handle->nn_state.neurons_committed);
    printf(" It took %" PRIu64 " us, per vector %" PRIu64 " us on average\n", usec_duration, usec_duration / cntl);

    // classify
    time_start = getCPUTime();
    for (size_t i = 0; i < neurons_count; ++i)
    {
      struct response_neuron_state_t resp[4];
      size_t resp_count;

      cntr++;
      resp_count = 4;
      result     = ntpcie_nn_vector_classify(dev_handle, NN_DIST_EVAL_L1, test_data[i].context, NN_CLASSIFIER_KNN, comps_to_test, &test_data[i].comps[0],
                                         &resp_count, resp);
      if (result != NTPCIE_ERROR_SUCCESS)
      {
        puts("... classify operation is fault ...");
        ntpcie_error_viewer(result);
        return;
      }
      // show results of recognize
      if ((resp_count > 0) && (resp[0].category == test_data[i].category))
      {
        cnti++;
        if (resp[0].distance != 0)
        {
          cnt_dist_not_0++;
        }
      }
      else
      {
        cntn++;
      }
    }

    time_stop = getCPUTime();

    usec_duration = (uint64_t)((time_stop - time_start) * 1000000.0);

    // TODO: prevent divide to ZERO

    printf(" Loaded %" PRIu64 " for classify vectors\n", cntr);
    printf(" It took %" PRIu64 " us, per vector %" PRIu64 " us on average\n", usec_duration, usec_duration / cntr);
    printf(" Total: identified/dist_non_zero/unknown: %" PRIu64 "/%" PRIu64 "/%" PRIu64 "\n", cnti, cnt_dist_not_0, cntn);
  }
}

void card_reset_stress_test(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  size_t neurons_count;

  printf(" input neurons desired amount [0..n] (HW specific, 0 EQ %" PRIu64 "): ", dev_handle->nn_state.neurons_overall);
  scanf("%" PRIu64, &neurons_count);

  if (neurons_count == 0)
  {
    neurons_count = dev_handle->nn_state.neurons_overall;
  }

  size_t cycles, cnt_reset_fault = 0, cnt_neurons_count_missmath = 0;

  fputs("input cycles count: ", stdout);
  scanf("%" PRIu64, &cycles);

  for (size_t ix = 0; ix < cycles; ix++)
  {
    nn_result = ntpcie_device_reset(dev_handle);
    if (nn_result != NTPCIE_ERROR_SUCCESS)
    {
      cnt_reset_fault++;
      continue;
    }
    else
    {
      if (dev_handle->nn_state.neurons_overall != neurons_count)
      {
        cnt_neurons_count_missmath++;
        printf(" neurons_count = %" PRIu64, dev_handle->nn_state.neurons_overall);
      }
    }
  }

  printf(" total/reset_fault/neurons_miss: %" PRIu64 "/%" PRIu64 "/%" PRIu64 "\n", cycles, cnt_reset_fault, cnt_neurons_count_missmath);
}

void nntest_kb_store(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  if (dev_handle->nn_state.neurons_committed < 1)
  {
    puts(" error: committed neurons is ZERO");
#ifdef NTIAPCIE_DEBUG
    puts(" *** NTIAPCIE_DEBUG active");
#else
    return;
#endif // NTIAPCIE_DEBUG
  }

  size_t kb_slot_ix = 0;

  fputs(" input KB slot number [0..1]: ", stdout);
  scanf("%" PRIu64, &kb_slot_ix);
  if (kb_slot_ix > 1)
  {
    puts(" error: slot number not in range [0..1]");
  }

  memset(&test_kbs_array[kb_slot_ix], 0, sizeof(test_kbs_array[0]));

  uint16_t _ncount = 0;
// ------ ATTENTION: NN register NCOUNT must be read before storing KB (for implicit set NR mode in NN)
  nn_result = ntpcie_nn_register_read(dev_handle, (enum nn_int_register_t)(CM_NCOUNT), &_ncount);
// -----------------------------------------------------------------

  size_t neurons_count = 0;
#ifdef NTIAPCIE_DEBUG
  puts(" *** NTIAPCIE_DEBUG active");
  fputs(" inputs neurons count: ", stdout);
  scanf("%" PRIu64, &neurons_count);
#else
  neurons_count = dev_handle->nn_state.neurons_committed;
#endif // NTIAPCIE_DEBUG

  test_kbs_array[kb_slot_ix].id            = 26382265743;
  test_kbs_array[kb_slot_ix].neurons_count = neurons_count;

  for (size_t ix = 0; ix < test_kbs_array[kb_slot_ix].neurons_count; ++ix)
  {
    nn_result = ntpcie_kbase_store(dev_handle, &test_kbs_array[kb_slot_ix].neurons_state[ix]);
    if (nn_result != NTPCIE_ERROR_SUCCESS)
    {
      puts(" error: store KB failed");
      ntpcie_error_viewer(nn_result);
      return;
    }
  }
  nn_sample_kb_crc32_update(&test_kbs_array[kb_slot_ix]);

  // TODO: serialize RAM to external storage (json/csv/DB/binary) on disk

  puts(" KB store to RAM - OK");
}

void nntest_kb_load(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result;

  // TODO: deserialize KB from external storage to RAM

  size_t kb_slot_ix = 0;

  fputs(" input KB slot number [0..1]: ", stdout);
  scanf("%" PRIu64, &kb_slot_ix);

  if (nn_sample_kb_is_valid(&test_kbs_array[kb_slot_ix]) == true)
  {
    if (test_kbs_array[kb_slot_ix].neurons_count > dev_handle->nn_state.neurons_overall)
    {
      puts(" error: KB is too BIG");
#ifdef NTIAPCIE_DEBUG
      puts(" *** NTIAPCIE_DEBUG active");
#else
      return;
#endif // NTIAPCIE_DEBUG
    }

    uint16_t _ncount = 0;
// ------ ATTENTION: NN register NCOUNT must be read before loading KB (for implicit set NR mode in NN)
    nn_result = ntpcie_nn_register_read(dev_handle, (enum nn_int_register_t)(CM_NCOUNT), &_ncount);
// -----------------------------------------------------------------
    nn_result = ntpcie_nn_reset(dev_handle);
    if (nn_result != NTPCIE_ERROR_SUCCESS)
    {
      puts(" error: NN reset failed");
      ntpcie_error_viewer(nn_result);
      return;
    }

    for (size_t ix = 0; ix < test_kbs_array[kb_slot_ix].neurons_count; ++ix)
    {
      nn_result = ntpcie_kbase_load(dev_handle, NN_NEURON_COMPONENTS, &test_kbs_array[kb_slot_ix].neurons_state[ix]);
      if (nn_result != NTPCIE_ERROR_SUCCESS)
      {
        puts(" error: load KB failed");
        ntpcie_error_viewer(nn_result);
        return;
      }
    }
    puts(" KB load to NN - OK");
    return;
  }
  else
  {
    puts(" error: KB is NOT valid");
    return;
  }
}

void nntest_simple_test(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t result = NTPCIE_ERROR_SUCCESS;

  if (result != NTPCIE_ERROR_SUCCESS)
  {
    puts("... FORGET before LEARN failed ...");
    ntpcie_error_viewer(result);
    return;
  }

  size_t errors_count = 0;
  size_t global_cycles_count;

  fputs(" Input amount of cycles [1...65535]: ", stdout);
  scanf("%" PRIu64, &global_cycles_count);

  nn_vector_comp_t v[256];

  for (size_t ix = 0; ix < global_cycles_count; ++ix)
  {
    result = ntpcie_nn_reset(dev_handle);

    v[0] = 1;
    v[1] = 1;
    v[2] = 1;
    v[3] = 1;
    v[4] = 1;

    result = ntpcie_nn_vector_learn(dev_handle, NN_DIST_EVAL_LSUP, 1, 1, 6, NN_DEF_MINIF, 5, v);
    if (result != NTPCIE_ERROR_SUCCESS)
    {
      puts("... Learn operation is fault ...\n");
      ntpcie_error_viewer(result);
      return;
    }

    v[0] = 6;
    v[1] = 6;
    v[2] = 6;
    v[3] = 6;
    v[4] = 6;

    result = ntpcie_nn_vector_learn(dev_handle, NN_DIST_EVAL_LSUP, 1, 2, 6, NN_DEF_MINIF, 5, v);
    if (result != NTPCIE_ERROR_SUCCESS)
    {
      puts("... Learn operation is fault ...\n");
      ntpcie_error_viewer(result);
      return;
    }

    struct response_neuron_state_t resp[4];
    size_t resp_count = 4;

    v[0] = 5;
    v[1] = 5;
    v[2] = 5;
    v[3] = 5;
    v[4] = 5;

    result = ntpcie_nn_vector_classify(dev_handle, NN_DIST_EVAL_LSUP, 1, NN_CLASSIFIER_KNN, 5, v, &resp_count, resp);
    if (result != NTPCIE_ERROR_SUCCESS)
    {
      puts("... classify operation is fault ...");
      ntpcie_error_viewer(result);
      return;
    }

    nntest_kb_store(dev_handle);
    ntpcie_nn_reset(dev_handle);
    nntest_kb_load(dev_handle);

    v[0] = 5;
    v[1] = 5;
    v[2] = 5;
    v[3] = 5;
    v[4] = 5;

    result = ntpcie_nn_vector_classify(dev_handle, NN_DIST_EVAL_LSUP, 1, NN_CLASSIFIER_KNN, 5, v, &resp_count, resp);
    if (result != NTPCIE_ERROR_SUCCESS)
    {
      puts("... classify operation is fault ...");
      ntpcie_error_viewer(result);
      return;
    }

    if (resp_count != 2)
    {
      ++errors_count;
      printf(" resp count = %" PRIu64 "\n", resp_count);
      for (size_t ix = 0; ix < resp_count; ++ix)
      {
        printf(" dist = %u category = %u id = %u degenerated = %u\n", resp[ix].distance, resp[ix].category, resp[ix].id, resp[ix].degenerated);
      }
    }
  }
  printf(" errors_cout = %" PRIu64 "\n", errors_count);
}

void card_view_info(struct nta_dev_handle_t* const dev_handle)
{
  puts("");
  puts("Active PCIe card info:");
  printf(" DEVICE_HANDLE     = 0x%08" PRIx64 "\n", (uint64_t)(dev_handle->_iox_handle));
  printf(" neurons total     = %" PRIu64 "\n", dev_handle->nn_state.neurons_overall);
  printf(" neurons committed = %" PRIu64 "\n", dev_handle->nn_state.neurons_committed);
  puts("last operation counters:");
  printf(" loops_wait_ready  = %" PRIu64 "\n", dev_handle->nn_state.count_loop_wait_ready);
  printf(" cpu_ticks         = %" PRIu64 "\n", dev_handle->nn_state.cpu_ticks_last_oper);
  puts("total operation counters (from last reset):");
  printf(" learn_cpu_ticks   = %" PRIu64 "\n", dev_handle->nn_state.cpu_ticks_total_learn);
  printf(" class_cpu_ticks   = %" PRIu64 "\n", dev_handle->nn_state.cpu_ticks_total_class);
  printf(" vectors_learn     = %" PRIu64 "\n", dev_handle->nn_state.vecs_count_total_learn);
  printf(" vectors_class     = %" PRIu64 "\n", dev_handle->nn_state.vecs_count_total_class);
}

void nntest_kbs_compare(struct nta_dev_handle_t * const dev_handle)
{
  (void)dev_handle;

  size_t kb_slot1_ix = 0;
  size_t kb_slot2_ix = 0;

  fputs(" input KB slot #1 number [0..1]: ", stdout);
  scanf("%" PRIu64, &kb_slot1_ix);
  fputs(" input KB slot #2 number [0..1]: ", stdout);
  scanf("%" PRIu64, &kb_slot2_ix);

  if (kb_slot1_ix > MAX_KBS_SLOTS || kb_slot2_ix > MAX_KBS_SLOTS)
  {
    puts(" error: slots X not in range [0..1]");
    return;
  }

  int compare_result = memcmp(&test_kbs_array[kb_slot1_ix], &test_kbs_array[kb_slot2_ix], sizeof(test_kbs_array[0]));

  if (compare_result == 0)
  {
    puts(" KB's is EQ");
  }
  else
  {
    puts(" KB's is NOT EQ");
  }
}
