/**
 * @file iM880A_RadioInterface.h
 * <!------------------------------------------------------------------------>
 * @brief @ref REF "WiMODLR HCI Example"
 *
 * @par Project:
 * <!------------------------------------------------------------------------>
 * <!--
 * @par Description:
 *
 *  [Description]
 * -->
 * <!--
 *  @ref [extdocname] "more..."
 *  -->
 * <!------------------------------------------------------------------------>
 * <!--
 * @remarks
 * - [...]
 * - [...]
 * -->
 * <!------------------------------------------------------------------------
 * Copyright (c) 2015
 * IMST GmbH
 * Carl-Friedrich Gauss Str. 2
 * 47475 Kamp-Lintfort
 * -------------------------------------------------------------------------->
 * @author Mathias Hater (MH), IMST
 * <!------------------------------------------------------------------------
 * Target OS:    none
 * Target CPU:   EFM32
 * Compiler:     IAR C/C++ Compiler
 * -------------------------------------------------------------------------->
 * @internal
 * @par Revision History:
 * <PRE>
 * ---------------------------------------------------------------------------
 * Version | Date       | Author | Comment
 * ---------------------------------------------------------------------------
 * 0.1     | 22.01.2015 | MH     | Created
 *
 * </PRE>

 Copyright (c) 2015 IMST GmbH.
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


#ifndef iM880A_RADIOINTERFACE_H_
#define iM880A_RADIOINTERFACE_H_


//------------------------------------------------------------------------------
//
// Include Files
//
//------------------------------------------------------------------------------
#include <stdint.h>
#include "RadioDefs.h"

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
//
// General Declaration
//
//------------------------------------------------------------------------------

typedef uint16_t        TWiMODLRResult;

//------------------------------------------------------------------------------
//
// Definition of Result/Error Codes
//
//------------------------------------------------------------------------------

typedef enum
{
    WiMODLR_RESULT_OK = 0,
    WiMODLR_RESULT_PAYLOAD_LENGTH_ERROR,
    WiMODLR_RESULT_PAYLOAD_PTR_ERROR,
    WiMODLR_RESULT_TRANMIT_ERROR
}TWiMDLRResultcodes;


//------------------------------------------------------------------------------
//
// Service Access Point Identifier
//
//------------------------------------------------------------------------------

#define DEVMGMT_SAP_ID                      0x01
#define RADIOLINK_SAP_ID                    0x03
#define HWTEST_SAP_ID                       0xA1

//------------------------------------------------------------------------------
//
// Device Management Message Identifier
//
//------------------------------------------------------------------------------

// Status Codes
#define	DEVMGMT_STATUS_OK					0x00
#define	DEVMGMT_STATUS_ERROR				0x01
#define	DEVMGMT_STATUS_CMD_NOT_SUPPORTED	0x02
#define	DEVMGMT_STATUS_WRONG_PARAMETER		0x03
#define DEVMGMT_STATUS_WRONG_DEVICE_MODE    0x04

// Message IDs
#define DEVMGMT_MSG_PING_REQ                0x01
#define DEVMGMT_MSG_PING_RSP                0x02

#define DEVMGMT_MSG_GET_DEVICEINFO_REQ      0x03
#define DEVMGMT_MSG_GET_DEVICEINFO_RSP      0x04

#define DEVMGMT_MSG_GET_FW_VERSION_REQ      0x05
#define DEVMGMT_MSG_GET_FW_VERSION_RSP      0x06

#define	DEVMGMT_MSG_RESET_REQ				0x07
#define	DEVMGMT_MSG_RESET_RSP				0x08

#define DEVMGMT_MSG_SET_OPMODE_REQ          0x09
#define DEVMGMT_MSG_SET_OPMODE_RSP          0x0A


#define	DEVMGMT_MSG_SET_RTC_REQ				0x0D
#define	DEVMGMT_MSG_SET_RTC_RSP				0x0E
#define	DEVMGMT_MSG_GET_RTC_REQ				0x0F
#define	DEVMGMT_MSG_GET_RTC_RSP				0x10

#define DEVMGMT_MSG_SET_RADIO_CONFIG_REQ    0x11
#define DEVMGMT_MSG_SET_RADIO_CONFIG_RSP    0x12
#define DEVMGMT_MSG_GET_RADIO_CONFIG_REQ    0x13
#define DEVMGMT_MSG_GET_RADIO_CONFIG_RSP    0x14

#define	DEVMGMT_MSG_RESET_RADIO_CONFIG_REQ  0x15
#define	DEVMGMT_MSG_RESET_RADIO_CONFIG_RSP  0x16

#define	DEVMGMT_MSG_GET_SYSTEM_STATUS_REQ	0x17
#define	DEVMGMT_MSG_GET_SYSTEM_STATUS_RSP	0x18

#define DEVMGMT_MSG_ENTER_LPM_REQ           0x1B
#define DEVMGMT_MSG_ENTER_LPM_RSP           0x1C


#define RADIOLINK_MSG_SEND_URADIO_MSG_REQ   0x01
#define RADIOLINK_MSG_SEND_URADIO_MSG_RSP   0x02
#define RADIOLINK_MSG_RECV_URADIO_MSG_IND   0x04
#define RADIOLINK_MSG_URADIO_MSG_TX_IND     0x06
#define RADIOLINK_MSG_RECV_RAWRADIO_MSG_IND 0x08


#define HWTEST_MSG_RADIO_TEST_REQ           0x01
#define HWTEST_MSG_RADIO_TEST_RSP           0x02


//------------------------------------------------------------------------------
//
// HCI Message Declaration
//
//------------------------------------------------------------------------------

// message header size: 2 bytes for SapID + MsgID
#define WIMODLR_HCI_MSG_HEADER_SIZE     2

// message payload size
#define WIMODLR_HCI_MSG_PAYLOAD_SIZE    280

// frame check sequence field size: 2 bytes for CRC16
#define WIMODLR_HCI_MSG_FCS_SIZE        2

// visible max. buffer size for lower SLIP layer
#define WIMODLR_HCI_RX_MESSAGE_SIZE (WIMODLR_HCI_MSG_HEADER_SIZE\
                                    +WIMODLR_HCI_MSG_PAYLOAD_SIZE\
                                    +WIMODLR_HCI_MSG_FCS_SIZE)


//------------------------------------------------------------------------------
//
// public available Data Types
//
//------------------------------------------------------------------------------


typedef struct
{
    // Payload Length Information, not transmitted over UART interface !
    uint16_t  Length;

    // Service Access Point Identifier
    uint8_t   SapID;

    // Message Identifier
    uint8_t   MsgID;

    // Payload Field
    uint8_t   Payload[WIMODLR_HCI_MSG_PAYLOAD_SIZE];

    // Frame Check Sequence Field
    uint8_t   CRC16[WIMODLR_HCI_MSG_FCS_SIZE];

}TWiMODLR_HCIMessage;




typedef void (*TRadioInterfaceCbRxIndication)(uint8_t* rxMsg, uint8_t length, TRadioFlags rxFlags);
typedef void (*TRadioInterfaceCbTxIndication)(TRadioMsg* txMsg, uint8_t status);



//------------------------------------------------------------------------------
//
//  Section Prototypes
//
//------------------------------------------------------------------------------

void
iM880A_Init(void);

void
iM880A_RegisterRadioCallbacks(TRadioInterfaceCbRxIndication cbRxInd,
                              TRadioInterfaceCbTxIndication cbTxInd);

TWiMODLRResult
iM880A_Configure(void);

TWiMDLRResultcodes
iM880A_SendRadioTelegram(uint8_t* payload, uint16_t length);

TWiMODLRResult
iM880A_PingRequest(void);

TWiMODLRResult
iM880A_PowerDown(void);

TWiMODLRResult
iM880A_WakeUp(void);

TWiMODLRResult
iM880A_ResetRadioConfig(void);

TWiMODLRResult
iM880A_ResetRequest(void);

#ifdef __cplusplus
}
#endif

#endif
