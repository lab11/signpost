/**
 * @file bsp_defs.h
 * <!------------------------------------------------------------------------>
 * @brief @ref REF "Board Support Package (BSP)"
 *
 * @par Project:
 * <!------------------------------------------------------------------------>
 * <!--
 * @par Description:
 *
 * Basic Macro Definitions.
 *
 * <!------------------------------------------------------------------------>
 * <!--
 * @remarks
 * - [...]
 * - [...]
 * -->
 * <!------------------------------------------------------------------------
 * Copyright (c) 2009
 * IMST GmbH
 * Carl-Friedrich Gauss Str. 2
 * 47475 Kamp-Lintfort
 * -------------------------------------------------------------------------->
 * @author Mathias Hater (MH), IMST
 * <!------------------------------------------------------------------------
 * Target OS:    independent
 * Target CPU:   independent
 * Compiler:     
 * -------------------------------------------------------------------------->
 * @internal
 * @par Revision History:
 * <PRE>
 * ---------------------------------------------------------------------------
 * Version | Date       | Author | Comment
 * ---------------------------------------------------------------------------
 * 0.1     | 2009-05-12 | TPa    | initial version
 * 0.2     | 2010-09-13 | KvW    | cleanup
 *
 * </PRE>

 Copyright (c) 2009 IMST GmbH.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are NOT permitted without prior written permission
 of the IMST GmbH.

 THIS SOFTWARE IS PROVIDED BY THE IMST GMBH AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE IMST GMBH OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 THIS SOFTWARE IS CLASSIFIED AS: CONFIDENTIAL

 *******************************************************************************/

#ifndef __BSP_DEFS_H__
#define __BSP_DEFS_H__

#include <stdint.h>

//------------------------------------------------------------------------------
//
// Standard Types
//
//------------------------------------------------------------------------------

#ifndef __TYPES__
#define __TYPES__

#ifdef  GCC_COMPILER
#define INLINE __attribute__((always_inline)) static

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned long       UINT32;
typedef unsigned char       BOOL;
//typedef unsigned char       bool;

typedef char                INT8;
typedef short               INT16;
typedef long                INT32;

#else

#include <stdint.h>
#include <stdbool.h>

#define INLINE              static inline

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned long       UINT32;
typedef unsigned char       BOOL;
//typedef unsigned char       bool;

typedef signed char         INT8;
typedef signed short        INT16;
typedef signed long         INT32;

#endif // GCC

// IAR compiler
#if defined ( __ICCARM__ )
#endif
// IAR compiler

#endif // Types


#define EOF                 -1
#define BV(x)               (1<<(x))
#define st(x)               do{x}while(0)

//------------------------------------------------------------------------------
// Boolean Constants
//------------------------------------------------------------------------------

#define false   0
#define true    1

#ifndef FALSE
   #define FALSE 0
#endif

#ifndef TRUE
   #define TRUE 1
#endif

#ifndef NULL
   #define NULL 0
#endif

//------------------------------------------------------------------------------
// Integer Conversion Macros
//------------------------------------------------------------------------------

#define HINIBBLE(x)         ((uint8_t) (((uint8_t) (x)) >> 4))
#define LONIBBLE(x)         ((uint8_t) (((uint8_t) (x)) & 0xF))
#define HIBYTE(x)           ((uint8_t) (((uint16_t)(x)) >> 8))
#define LOBYTE(x)           ((uint8_t) (((uint16_t)(x)) & 0xFF))
#define HIWORD(x)           ((uint16_t)(((uint32_t)(x)) >> 16))
#define LOWORD(x)           ((uint16_t)(((uint32_t)(x)) & 0xFFFF))

#define MAKEBYTE(lo, hi)    ((uint8_t) (((uint8_t) (lo)) | (((uint8_t) ((uint8_t) (hi))) << 4)))
#define MAKEWORD(lo, hi)    ((uint16_t)(((uint8_t) (lo)) | (((uint16_t)((uint8_t) (hi))) << 8)))
#define MAKEDWORD(lo, hi)   ((uint32_t)(((uint16_t)(lo)) | (((uint32_t)((uint16_t)(hi))) << 16)))

#define SWAPBYTE(x)         MAKEWORD(HIBYTE(x), LOBYTE(x))

//------------------------------------------------------------------------------
// Byte-Order Conversion Macros
//------------------------------------------------------------------------------

#define NTOH16(srcPtr)  MAKEWORD((srcPtr)[0], (srcPtr)[1])

#define HTON16(dstPtr, value)           \
        (dstPtr)[0] = LOBYTE(value);    \
        (dstPtr)[1] = HIBYTE(value);

#define NTOH32(srcPtr)  MAKEDWORD(MAKEWORD((srcPtr)[0], (srcPtr)[1]),MAKEWORD((srcPtr)[2], (srcPtr)[3]))

#define HTON32(dstPtr, value)                   \
        (dstPtr)[0] = LOBYTE(LOWORD(value));    \
        (dstPtr)[1] = HIBYTE(LOWORD(value));    \
        (dstPtr)[2] = LOBYTE(HIWORD(value));    \
        (dstPtr)[3] = HIBYTE(HIWORD(value));

#endif  //  __BSP_DEFS_H__
        
//------------------------------------------------------------------------------
// end of file
//------------------------------------------------------------------------------


