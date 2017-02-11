/**
 * @file iM880A_RadioInterface.c
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

#include "bsp_defs.h"
#include "tock.h"
#include "ComSlip.h"
#include "CRC16.h"
#include "RadioDefs.h"
#include "iM880A_RadioInterface.h"

#define DEST_ADDR_SIZE          3

TWiMODLR_HCIMessage TxMessage;
TWiMODLR_HCIMessage RxMessage;


TRadioInterfaceCbRxIndication cbRxIndication;
TRadioInterfaceCbTxIndication cbTxIndication;


static uint8_t configBuffer[26];


//------------------------------------------------------------------------------
//
//  iM880A_SendHCIMessage
//
//  @brief  send generic HCI message to iM880A
//
//------------------------------------------------------------------------------

static TWiMDLRResultcodes
iM880A_SendHCIMessage(uint8_t sapID, uint8_t msgID, uint8_t* payload, uint16_t length)
{
    // 1. check parameter
    //
    // 1.1 check length
    //
    if(length > WIMODLR_HCI_MSG_PAYLOAD_SIZE)
    {
        return WiMODLR_RESULT_PAYLOAD_LENGTH_ERROR;
    }

    // 2.  init TxMessage
    //
    // 2.1 init SAP ID
    //
    TxMessage.SapID = sapID;

    // 2.2 init Msg ID
    //
    TxMessage.MsgID = msgID;

    // 2.3 copy payload, if present
    //
    if(payload && length)
    {
        uint8_t*  dstPtr  = TxMessage.Payload;
        int     n       = (int)length;

        // copy bytes
        while(n--)
            *dstPtr++ = *payload++;
    }

    // 3. Calculate CRC16 over header and optional payload
    //
    uint16_t crc16 = CRC16_Calc(&TxMessage.SapID, length + WIMODLR_HCI_MSG_HEADER_SIZE, CRC16_INIT_VALUE);

    // 3.1 get 1's complement
    //
    crc16 = ~crc16;

    // 3.2 attach CRC16 and correct length, lobyte first
    //
    TxMessage.Payload[length++] = LOBYTE(crc16);
    TxMessage.Payload[length++] = HIBYTE(crc16);

    // 4. forward message to SLIP layer
    //    - start transmission with SAP ID
    //    - correct length by header size

    if (ComSlip_SendMessage(&TxMessage.SapID, length + WIMODLR_HCI_MSG_HEADER_SIZE))
    {
        // ok !
        return WiMODLR_RESULT_OK;
    }

    // SLIP layer wasn't able to sent
    return WiMODLR_RESULT_TRANMIT_ERROR;
}



//------------------------------------------------------------------------------
//
//  iM880A_SendRadioTelegram
//
//  @brief  send radio telegram to default address
//
//------------------------------------------------------------------------------

TWiMDLRResultcodes
iM880A_SendRadioTelegram(uint8_t* payload, uint16_t length)
{
    TxMessage.Payload[0] = COMRADIO_CFG_DEFAULT_TXGROUPADDRESS;
    TxMessage.Payload[1] = LOBYTE(COMRADIO_CFG_DEFAULT_TXADDRESS);
    TxMessage.Payload[2] = HIBYTE(COMRADIO_CFG_DEFAULT_TXADDRESS);

    if(payload && length)
    {
        uint8_t*  dstPtr  = TxMessage.Payload + DEST_ADDR_SIZE;
        int     n       = (int)length;

        // copy bytes
        while(n--)
            *dstPtr++ = *payload++;
    }

    return iM880A_SendHCIMessage(RADIOLINK_SAP_ID, RADIOLINK_MSG_SEND_URADIO_MSG_REQ, NULL, length + DEST_ADDR_SIZE);
}



//------------------------------------------------------------------------------
//
//  iM880A_PingRequest
//
//  @brief  send ping to check communication link
//
//------------------------------------------------------------------------------

TWiMODLRResult
iM880A_PingRequest(void)
{
    return iM880A_SendHCIMessage(DEVMGMT_SAP_ID, DEVMGMT_MSG_PING_REQ, NULL, 0);
}



//------------------------------------------------------------------------------
//
//  iM880A_CbProcessRxMessage
//
//  @brief: handle incoming HCI message
//
//------------------------------------------------------------------------------

static uint8_t*
iM880A_CbProcessRxMessage(uint8_t* rxBuffer, uint16_t length)
{
    // 1. check CRC
    if (CRC16_Check(rxBuffer, length, CRC16_INIT_VALUE))
    {
        // 2. check min length, 2 bytes for SapID + MsgID + 2 bytes CRC16
        if(length >= (WIMODLR_HCI_MSG_HEADER_SIZE + WIMODLR_HCI_MSG_FCS_SIZE))
        {
            // 3. Hack: since only one RxMessage buffer is used,
            //          rxBuffer must point to RxMessage.SapId, thus
            //          memcpy to RxMessage structure is not needed here

            // add length
            RxMessage.Length = length - (WIMODLR_HCI_MSG_HEADER_SIZE + WIMODLR_HCI_MSG_FCS_SIZE);

            // dispatch completed RxMessage
            // 1. forward message according to SapID
            switch(RxMessage.SapID)
            {
                case    DEVMGMT_SAP_ID:

                        // TODO: handle messaged for DEVMGMT_SAP_ID, e.g. ping response

                        break;

                case    RADIOLINK_SAP_ID:
                        if(RxMessage.MsgID == RADIOLINK_MSG_SEND_URADIO_MSG_RSP) {
                            (*cbTxIndication)(NULL, RxMessage.Payload[0]);
                        }
                        if(RxMessage.MsgID == RADIOLINK_MSG_URADIO_MSG_TX_IND)
                        {
                            (*cbTxIndication)(NULL, RxMessage.Payload[0]);
                        }
                        else if(RxMessage.MsgID == RADIOLINK_MSG_RECV_URADIO_MSG_IND)
                        {
                            (*cbRxIndication)(RxMessage.Payload, length, trx_RXDone);
                        }
                        break;
                default:
                        // handle unsupported SapIDs here
                        break;
            }
        }
    }
    else
    {
        // handle CRC error
    }

    // return same buffer again, keep receiver enabled
    return &RxMessage.SapID;
}


//------------------------------------------------------------------------------
//
//  iM880A_Init
//
//  @brief: initialize radio interface
//
//------------------------------------------------------------------------------

void
iM880A_Init(void)
{
    // Init Slip Layer
    ComSlip_Init();
    ComSlip_RegisterClient(iM880A_CbProcessRxMessage);

    // pass first RxBuffer and enable receiver/decoder
    ComSlip_SetRxBuffer(&RxMessage.SapID, (uint16_t)WIMODLR_HCI_RX_MESSAGE_SIZE);
}


//------------------------------------------------------------------------------
//
//  iM880A_RegisterRadioCallbacks
//
//  @brief: set callback functions for Rx/Tx
//
//------------------------------------------------------------------------------

void
iM880A_RegisterRadioCallbacks(TRadioInterfaceCbRxIndication cbRxInd,
                              TRadioInterfaceCbTxIndication cbTxInd)
{
    cbRxIndication = cbRxInd;
    cbTxIndication = cbTxInd;
}



//------------------------------------------------------------------------------
//
//  iM880A_Configure
//
//  @brief: Configure iM880A
//
//------------------------------------------------------------------------------

TWiMODLRResult
iM880A_Configure(void)
{
    uint8_t offset=0;

    configBuffer[offset++]   = 0x00;                                        // NVM Flag - change config only temporary
    configBuffer[offset++]   = COMRADIO_CFG_DEFAULT_RFRADIOMODE;            // 0
    configBuffer[offset++]   = COMRADIO_CFG_DEFAULT_RXGROUPADDRESS;         // 1
    configBuffer[offset++]   = COMRADIO_CFG_DEFAULT_TXGROUPADDRESS;         // 2
    HTON16(&configBuffer[offset], COMRADIO_CFG_DEFAULT_RFDEVICEADDRESS);    // 3
    offset += 2;
    HTON16(&configBuffer[offset], COMRADIO_CFG_DEFAULT_TXADDRESS);          // 5
    offset += 2;
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFRADIOMODULATION;       // 7
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFCHANNEL_LSB;           // 8
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFCHANNEL_MID;           // 9
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFCHANNEL_MSB;           // 10
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFCHANNELBW;             // 11
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFSPREADINGFACTOR;       // 12
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFERRORCODING;           // 13
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFPOWERLEVEL;            // 14
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFTXOPTIONS;             // 15
    configBuffer[offset++]  = COMRADIO_CFG_DEFAULT_RFRXOPTIONS;             // 16
    HTON16(&configBuffer[offset], COMRADIO_CFG_DEFAULT_RFRXWINDOW);         // 17
    offset += 2;
    configBuffer[offset++] = COMRADIO_CFG_DEFAULT_LEDCONTROL;               // 19
    configBuffer[offset++] = COMRADIO_CFG_DEFAULT_MISCOPTIONS;              // 20

    configBuffer[offset++] = COMRADIO_CFG_DEFAULT_FSK_DATARATE;             // 21
    configBuffer[offset++] = COMRADIO_CFG_DEFAULT_POWERSAVINGMODE;          // 22
    HTON16(&configBuffer[offset], COMRADIO_CFG_DEFAULT_LBTTHRESHOLD);       // 23
    offset += 2;

    // Set Configuration
    return iM880A_SendHCIMessage(DEVMGMT_SAP_ID, DEVMGMT_MSG_SET_RADIO_CONFIG_REQ, (unsigned char*)&configBuffer, offset);
}


//------------------------------------------------------------------------------
//
//  iM880A_PowerDown
//
//  @brief: Set iM880A to low power mode
//
//------------------------------------------------------------------------------
TWiMODLRResult
iM880A_PowerDown(void)
{
    uint8_t payload[1] = {0x00};
    return iM880A_SendHCIMessage(DEVMGMT_SAP_ID, DEVMGMT_MSG_ENTER_LPM_REQ, payload, 1);
}


//------------------------------------------------------------------------------
//
//  iM880A_WakeUp
//
//  @brief: Wake up iM880A from power down mode
//
//------------------------------------------------------------------------------
TWiMODLRResult
iM880A_WakeUp(void)
{
    return iM880A_SendHCIMessage(DEVMGMT_SAP_ID, DEVMGMT_MSG_PING_REQ, NULL, 0);
}


//------------------------------------------------------------------------------
//
//  iM880A_ResetRadioConfig
//
//  @brief: Restore factore settings
//
//------------------------------------------------------------------------------

TWiMODLRResult
iM880A_ResetRadioConfig(void)
{
    return iM880A_SendHCIMessage(DEVMGMT_SAP_ID, DEVMGMT_MSG_RESET_RADIO_CONFIG_REQ, NULL, 0);
}



//------------------------------------------------------------------------------
//
//  iM880A_ResetRequest
//
//  @brief: iM880A software initiated reset
//
//------------------------------------------------------------------------------

TWiMODLRResult
iM880A_ResetRequest(void)
{
    return iM880A_SendHCIMessage(DEVMGMT_SAP_ID, DEVMGMT_MSG_RESET_REQ, NULL, 0);
}

