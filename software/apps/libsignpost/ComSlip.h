/**
 * @file ComSlip.h
 * <!------------------------------------------------------------------------>
 * @brief @ref REF "SLIP Driver Implementation"
 *
 * @par Project:
 * <!------------------------------------------------------------------------>
 * <!--
 * @par Description:
 *
 * Implementation of SLIP Driver
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
 * 0.1     | 12.05.2009 | KvW    | Created
 * 0.2     | 14.09.2011 | KvW    | cleanup
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

#ifndef COMSLIP_H
#define COMSLIP_H

//------------------------------------------------------------------------------
//
// Include Files
//
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t* (*TComSlipCbByteIndication)(uint8_t* rxBuffer, uint16_t length);


//------------------------------------------------------------------------------
//
//  Section Prototypes
//
//------------------------------------------------------------------------------

void
ComSlip_Init(void);

void
ComSlip_RegisterClient(TComSlipCbByteIndication cbRxIndication);

bool
ComSlip_SetRxBuffer(uint8_t*  rxBuffer, uint16_t rxbufferSize);

bool
ComSlip_SendMessage(uint8_t* msg, uint16_t msgLength);

void
ComSlip_ProcessRxByte(uint8_t rxData);


#ifdef __cplusplus
}
#endif

#endif // COMSLIP_H

//------------------------------------------------------------------------------
// end of file
//------------------------------------------------------------------------------
