/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTIA_SHARED_DEFS_H_
#define ONCE_INC_NTIA_SHARED_DEFS_H_

#if defined _WIN32 || defined __CYGWIN__
  #define NTIA_API_HELPER_DLL_IMPORT __declspec(dllimport)
  #define NTIA_API_HELPER_DLL_EXPORT __declspec(dllexport)
  #define NTIA_API_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define NTIA_API_HELPER_DLL_IMPORT __attribute__((__visibility__("default")))
    #define NTIA_API_HELPER_DLL_EXPORT __attribute__((__visibility__("default")))
    #define NTIA_API_HELPER_DLL_LOCAL  __attribute__((__visibility__("hidden")))
  #else
    #define NTIA_API_HELPER_DLL_IMPORT
    #define NTIA_API_HELPER_DLL_EXPORT
    #define NTIA_API_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define NTIA_API and NTIA_API_LOCAL.
// NTIA_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// NTIA_LOCAL is used for non-api symbols.

#ifdef NTIA_API_DLL // defined if NTIA_API is compiled as a DLL
  #ifdef NTIA_API_DLL_EXPORTS // defined if we are building the NTIA_API DLL (instead of using it)
    #define NTIA_API NTIA_API_HELPER_DLL_EXPORT
  #else
    #define NTIA_API NTIA_API_HELPER_DLL_IMPORT
  #endif // NTIA_API_DLL_EXPORTS
  #define NTIA_LOCAL NTIA_API_HELPER_DLL_LOCAL
#else // NTIA_API_DLL is not defined: this means NTIA_API is a static lib.
  #define NTIA_API
  #define NTIA_LOCAL
#endif // NTIA_API_DLL

#endif  // ONCE_INC_NTIA_SHARED_DEFS_H_
