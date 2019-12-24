/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_TRANSPORT_SYS_H_
#define ONCE_INC_TRANSPORT_SYS_H_

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "sorry, tested only for LITTLE_ENDIAN"
#endif // __BYTE_ORDER__

#ifndef __linux__
#error "sorry, this module for Linux ONLY"
#endif // __linux__

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_UIO_MAPS (6)

struct uxio_map_t
{
  void*  iomem;
  size_t size;
};

struct uxio_dev_handle_t
{
  size_t maps_total;
  size_t maps_active;
  struct uxio_map_t maps[MAX_UIO_MAPS];
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ONCE_INC_TRANSPORT_SYS_H_
