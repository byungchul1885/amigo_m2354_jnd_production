/*
 * Copyright (C) 2016  Electronics and Telecommunications Research Institute (ETRI)
 *
 * This file is subject to the terms and conditions of the GNU General Public License V3
 * See the file LICENSE in the top level directory for more details.
 *
 * This file is part of the NanoQplus3 operating system.
 */

/**
 * @file typedef.h
 * @brief Basic type definitions
 * @author Haeyong Kim (ETRI)
 * @date 2016. 1. 29.
 * @ingroup noslib_stm32f4xx
 * @copyright GNU General Public License v3
 */

#ifndef TYPEDEF_H
#define TYPEDEF_H

#if 1
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t   UINT8;
typedef uint16_t  UINT16;
#if defined (__ICCARM__)
typedef unsigned long   UINT32;
#else
typedef uint32_t  UINT32;
#endif
typedef uint64_t  UINT64;
typedef int8_t    INT8;
typedef int16_t   INT16;
#if defined (__ICCARM__)
typedef signed   long   INT32;
#else
typedef int32_t   INT32;
#endif
typedef int64_t   INT64;
typedef bool      BOOL;
typedef uint32_t    MEMADDR_T;
typedef uint32_t    MEMENTRY_T;
#endif

#endif // TYPEDEF_H
