/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_TRANSPORT_UMDFV2_H_
#define ONCE_INC_TRANSPORT_UMDFV2_H_

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "sorry, tested only for LITTLE_ENDIAN"
#endif // __BYTE_ORDER__

#ifndef _WIN32
#error "sorry, this module for win10 ONLY"
#endif // _WIN32

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NTIA_MAX_DEVPATH_LENGTH (256)

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ONCE_INC_TRANSPORT_UMDFV2_H_
