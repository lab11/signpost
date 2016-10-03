/**
 * @file RadioDefs.h
 * <!------------------------------------------------------------------------>
 * @brief @ref REF "Radio Definitions"
 *
 * @par Project:
 * <!------------------------------------------------------------------------>
 * <!--
 * @par Description:
 *
 * This file contains typedefs and defines for every parameter and parameter set
 * that is used within the whole Radio driver section.
 *
 * <!------------------------------------------------------------------------>
 * <!--
 * @remarks
 * - [...]
 * - [...]
 * -->
 * <!------------------------------------------------------------------------
 * Copyright (c) 2013
 * IMST GmbH
 * Carl-Friedrich Gauss Str. 2
 * 47475 Kamp-Lintfort
 * -------------------------------------------------------------------------->
 * @author Tobias Parketny (TPa), IMST
 * <!------------------------------------------------------------------------
 * Target OS:    none
 * Target CPU:   independent
 * Compiler:     IAR C/C++ Compiler
 * -------------------------------------------------------------------------->
 * @internal
 * @par Revision History:
 * <PRE>
 * -----+---------+------+--------+--------------------------------------------
 * VER  |    DATE | TIME | AUTHOR | CHANGES
 * -----+---------+------+--------+--------------------------------------------
 * 0.1  | 27.05.2013     | TPa    | Created
 *
 * </PRE>

 Copyright (c) 2013 IMST GmbH.
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

#ifndef __RadioDefs_H__
#define __RadioDefs_H__

#include <stdint.h>

#define RF_LORA_CHANNEL_BW_125K     0
#define RF_LORA_CHANNEL_BW_250K     1
#define RF_LORA_CHANNEL_BW_500K     2

#define RF_LORA_FEC_4_5         1
#define RF_LORA_FEC_4_6         2
#define RF_LORA_FEC_4_7         3
#define RF_LORA_FEC_4_8         4

#define RF_LORA_SF_6            6
#define RF_LORA_SF_7            7
#define RF_LORA_SF_8            8
#define RF_LORA_SF_9            9
#define RF_LORA_SF_10           10
#define RF_LORA_SF_11           11
#define RF_LORA_SF_12           12

#define RF_FSK_DATARATE_100_KBPS 0
#define RF_FSK_DATARATE_250_KBPS 1
#define RF_FSK_DATARATE_MAX      RF_FSK_DATARATE_250_KBPS


//------------------------------------------------------------------------------
//  RF Deviceaddress
//------------------------------------------------------------------------------
//! typedef for radio device address
typedef uint16_t  TRFDeviceAddress;
//! typedef for radio tx group address
typedef uint8_t   TRFTXGroupAddress;
//! typedef for radio rx group address
typedef uint8_t   TRFRXGroupAddress;


//! radio broadcast address
#define RADIO_BROADCAST_ADDRESS         0xFFFF
#define RADIO_BROADCAST_GROUP_ADDRESS   0xFF


//------------------------------------------------------------------------------
//  Common Transceicer States & Flags
//------------------------------------------------------------------------------

typedef enum
{
    trx_RXDone,
    trx_RXTimeout,
    trx_RXCRCError,
    trx_RXLengthError,
    trx_RXValidHeader,
    trx_TXDone,
    trx_TXTimeout,
    trx_CADDone,
    trx_CADDetected,
    trx_ACKTimeout,
    trx_ACKDone,
}TRadioFlags;


// radio operating modes
#define RADIO_MODE_NORMAL           0x00
#define RADIO_MODE_SNIFFER          0x02
#define RADIO_MODE_REPEATER         0x01

// RF RX Options
#define RADIO_RXOPTION_RCVR_OFF     0x00
#define RADIO_RXOPTION_RCVR_ON      0x01
#define RADIO_RXOPTION_RCVR_WINDOW  0x02


//------------------------------------------------------------------------------
//  RF Frame Size Definitions
//------------------------------------------------------------------------------


//! maximum radio frame size - FIFO buffer Size, shared for RX and TX
#define RADIO_FRAME_PHY_PAYLOAD_SIZE    (255)

//! radio frame header size with network (group) address byte
//! 1 byte mac control field
//! 1 byte tx group address
//! 2 bytes destination address
//! 1 byte rx group address
//! 2 bytes source address
#define RADIO_FRAME_MAC_HEADER_SIZE     (7)

//! Size of CRC
#define RADIO_FRAME_MAC_CRC_SIZE        (2)

//! radio frame payload size
#define RADIO_FRAME_MAC_PAYLOAD_SIZE    (RADIO_FRAME_PHY_PAYLOAD_SIZE - RADIO_FRAME_MAC_HEADER_SIZE)

//! radio frame minimum size
#define RADIO_FRAME_MIN_SIZE            RADIO_FRAME_MAC_HEADER_SIZE

//! radio frame payload size available on application layer
#define RADIO_FRAME_APP_PAYLOAD_SIZE    RADIO_FRAME_MAC_PAYLOAD_SIZE


// Radio Message
typedef struct RadioMsg_T
{
    //! Length Field
    uint8_t   Length;

    //! Control field
    uint8_t   Ctrl;

    //! Destination Group Address
    uint8_t   DstGroupAddr;

    //! Destination Address
    uint8_t   DstAddr[2];

    //! Source Group Address
    uint8_t   SrcGroupAddr;

    //! Source Address
    uint8_t   SrcAddr[2];

    //! Payload Field with Message dependent content
    uint8_t   Payload[RADIO_FRAME_MAC_PAYLOAD_SIZE];

    //! timestamp
    uint32_t  TimeStamp;

    //! RSSI value
    int16_t   Rssi;

    //! SNR value
    int8_t    Snr;

}TRadioMsg;





//------------------------------------------------------------------------------
//  Configuration Defaults
//------------------------------------------------------------------------------
// Defualt RF Radio Mode: Normal Device
#define COMRADIO_CFG_DEFAULT_RFRADIOMODE        RADIO_MODE_NORMAL
// Default to high power band with 869.525 MHz
#define COMRADIO_CFG_DEFAULT_RFCHANNEL_MSB      0xD9
#define COMRADIO_CFG_DEFAULT_RFCHANNEL_MID      0x61
#define COMRADIO_CFG_DEFAULT_RFCHANNEL_LSB      0x99
// Channel zero within the high power band
#define COMRADIO_CFG_DEFAULT_RFCHANNEL          0x00
// bandwidth default is 125 kHz
#define COMRADIO_CFG_DEFAULT_RFCHANNELBW        RF_LORA_CHANNEL_BW_125K
// Default device address
#define COMRADIO_CFG_DEFAULT_RFDEVICEADDRESS    0x1234
// error coding default is 4/6
#define COMRADIO_CFG_DEFAULT_RFERRORCODING      RF_LORA_FEC_4_6
// default FSK datarate
#define COMRADIO_CFG_DEFAULT_RFFSKDATARATE      RF_FSK_DATARATE_250_KBPS
// default miscellaneous options:use extended output format, use RTC
//#define COMRADIO_CFG_DEFAULT_MISCOPTIONS        0x03
// default RF tx options: filter not used
#define COMRADIO_CFG_DEFAULT_RFTXOPTIONS        0x00
// default RF Rx options: receiver always on
#define COMRADIO_CFG_DEFAULT_RFRXOPTIONS        0x01
// Power level 17 dBm
#define COMRADIO_CFG_DEFAULT_RFPOWERLEVEL       17
// Radio Modulation Mode: LoRa
#define COMRADIO_CFG_DEFAULT_RFRADIOMODULATION  0x00
// RX group address default
#define COMRADIO_CFG_DEFAULT_RXGROUPADDRESS     0x10
// rx window default, no window
#define COMRADIO_CFG_DEFAULT_RFRXWINDOW         3000 // 3s
// default spreading factor = 7
#define COMRADIO_CFG_DEFAULT_RFSPREADINGFACTOR  RF_LORA_SF_11
// default tx group address
#define COMRADIO_CFG_DEFAULT_TXGROUPADDRESS     0x10
// default tx address: broadcast
#define COMRADIO_CFG_DEFAULT_TXADDRESS          0x1234
// default LED configuration: LEDs toggleing enabled
#define COMRADIO_CFG_DEFAULT_LEDCONTROL          0x0F
                                             /* 0000 1111
                                                 |||| ||||
                                                 |||| |||- toggled LED D3 as 'Rx Indicator'
                                                 |||| ||-- toggled LED D2 as 'Tx Indicator'
                                                 |||| |--- toggled LED D4 as 'Alive Indicator'
                                                 |||| ---- toggled LED D1 as 'Button Pressed Indicator'
                                             */
// default misc options
#define COMRADIO_CFG_DEFAULT_MISCOPTIONS         0x07
                                              /* 0000 0111
                                                 |||| ||||
                                                 |||| |||- extended RF packet output format
                                                 |||| ||-- RTC enabled
                                                 |||| |--- HCI Tx Indication enabled
                                                 |||| ---- HCI Power Up Indication disabled
                                                 |||------ HCI Button Pressed Indication disabeld
                                                 ||------- AES Encryption/Decryption off
                                             */
// default FSK Data Rate: 250000 pbs
#define COMRADIO_CFG_DEFAULT_FSK_DATARATE        0x02
// default Power Saving Mode: off
#define COMRADIO_CFG_DEFAULT_POWERSAVINGMODE     0x00
// Default Listen Before Talk Treshold: 0dBm
#define COMRADIO_CFG_DEFAULT_LBTTHRESHOLD        0x0000


//------------------------------------------------------------------------------
// eof
//------------------------------------------------------------------------------
#endif
