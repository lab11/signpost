/**
 * @file CRC16.h
 * <!------------------------------------------------------------------------>
 * @brief @ref REF "CRC16 Implementation"
 *
 * @par Project:
 * <!------------------------------------------------------------------------>
 * <!--
 * @par Description:
 *
 * Implementation of 16-BIT CRC CCITT
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
 * @author Kai vorm Walde (KvW), IMST
 * <!------------------------------------------------------------------------
 * Target OS:    independent
 * Target CPU:   independent
 * Compiler:     IAR C/C++ Compiler
 * -------------------------------------------------------------------------->
 * @internal
 * @par Revision History:
 * <PRE>
 * ---------------------------------------------------------------------------
 * Version | Date       | Author | Comment
 * ---------------------------------------------------------------------------
 * 0.1     | 28.08.1997 | KvW    | Created
 * 0.2     | 14.09.2011 | KvW    | Cleanup
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

#ifndef    __CRC16_H__
#define    __CRC16_H__

//------------------------------------------------------------------------------
//
//  Section Include Files
//
//------------------------------------------------------------------------------
#include <stdint.h>

//------------------------------------------------------------------------------
//
//  Section Defines
//
//------------------------------------------------------------------------------

#define CRC16_INIT_VALUE    0xFFFF    //!< initial value for CRC algorithem
#define CRC16_GOOD_VALUE    0x0F47    //!< constant compare value for check
#define CRC16_POLYNOM       0x8408    //!< 16-BIT CRC CCITT POLYNOM

//------------------------------------------------------------------------------
//
//  Section Prototypes
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//! Calc CRC16
uint16_t
CRC16_Calc  (uint8_t*     data,
             uint16_t     length,
             uint16_t     initVal);
//------------------------------------------------------------------------------
//! Calc & Check CRC16
bool
CRC16_Check (uint8_t*     data,
             uint16_t     length,
             uint16_t     initVal);

//------------------------------------------------------------------------------

#endif // __CRC16_H__
//------------------------------------------------------------------------------
// end of file
//------------------------------------------------------------------------------
