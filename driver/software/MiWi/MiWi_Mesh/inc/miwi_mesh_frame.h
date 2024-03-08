/**
* \file  miwi_mesh_frame.h
*
* \brief MiWi Mesh Protocol Frame Handling interface
*
* Copyright (c) 2024 Microchip Technology Inc. and its subsidiaries. 
*
* \asf_license_start
*
* \page License
*
* Subject to your compliance with these terms, you may use Microchip
* software and any derivatives exclusively with Microchip products. 
* It is your responsibility to comply with third party license terms applicable 
* to your use of third party software (including open source software) that 
* may accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, 
* WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, 
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, 
* AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE 
* LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL 
* LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE 
* SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE 
* POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT 
* ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY 
* RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, 
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
* \asf_license_stop
*
*/
/*
* Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
*/

#ifndef MIWI_MESH_FRAME_H
#define MIWI_MESH_FRAME_H
#include "config/default/definitions.h"
/************************ HEADERS **********************************/

/*********************** Macro Definitions **************************************/

#define BIT_FIELDS_1(f1) f1;
#define BIT_FIELDS_2(f1, f2) f1; f2;
#define BIT_FIELDS_3(f1, f2, f3)  f1; f2; f3;
#define BIT_FIELDS_4(f1, f2, f3, f4)  f1; f2; f3; f4;
#define BIT_FIELDS_5(f1, f2, f3, f4, f5)  f1; f2; f3; f4; f5;
#define BIT_FIELDS_6(f1, f2, f3, f4, f5, f6)  f1; f2; f3; f4; f5; f6;
#define BIT_FIELDS_7(f1, f2, f3, f4, f5, f6, f7)  f1; f2; f3; f4; f5; f6; f7;
#define BIT_FIELDS_8(f1, f2, f3, f4, f5, f6, f7, f8) f1; f2; f3; f4; f5; f6; f7; f8;

#define LITTLE_ENDIAN_OCTET(amount, fields) BIT_FIELDS_ ## amount fields

#define MESH_MAX_PAYLOAD_SIZE      80U

#define NWK_FRAMETYPE_DATA       0U
#define NWK_FRAMETYPE_COMMAND    1U
#define NWK_FRAMETYPE_MANU_SPEC  2U

#define SHORT_ADDR_MODE          2U
#define LONG_ADDR_MODE           3U

#define SECURITY_LEVEL_LEN       1U
#define FRAME_COUNTER_LEN        4U

#define SINGLE_HOP               1U
#define MAXIMUM_HOP              miwiDefaultRomOrRamParams->numOfCoordinators + 1U

/*********************** External Definitions **************************************/


/************************ Type Definitions **************************************/

typedef struct _MeshFrameControl_t
{
  LITTLE_ENDIAN_OCTET(7, (
    /** The frame type sub-field is 2 bits in length and shall be set to one of
     * the nonreserved values: NWK_FRAMETYPE_DATA or NWK_FRAMETYPE_COMMAND. */
    uint8_t frameType                : 2,
    /** The securityEnabled sub-field shall be set if security to be enabled. */
    uint8_t securityEnabled          : 1,
     /** The interPan sub-field species whether source PANID present or not */
    uint8_t interPan             : 1,
    /** The ackRequest sub-field shall be set if frame requires acknowledgment. */
    uint8_t ackRequest               : 1,
    /** The addressSameAsMAC sub-field shall be set NWK header uses the same addressing fields of MAC */
    uint8_t addressSameAsMAC         : 1,
    /** The dataPending sub-field species more data pending for enddevice or not */
    uint8_t dataPending              : 1,
    uint8_t reserved                 : 1
  ))
} MeshFrameControl_t;

typedef struct _MeshAuxSecFrameHeader_t
{
    /** The securityLevel sub-field species the security level used for the security processing */
    uint8_t securityLevel;
    /** The frameCounter sub-field species the outgoing frame counter of the device */
    API_UINT32_UNION frameCounter;
	/* Source Extended Address of the frame initiator */
	uint64_t extendedAddress;
} MeshAuxSecFrameHeader_t;

typedef struct _MeshFrameHeader_t
{
    /** The hops sub-field specifies the number of hops the frame to be transmitted */
    uint8_t  hops;
    /** The frame control sub-field specifies the bit fields of control field */
    MeshFrameControl_t frameControl;
    /** The dstPanId sub-field specifies the Destination PANID */
    uint16_t dstPanId;
    /** The dstAddr sub-field specifies the address of destination device */
    uint16_t dstAddr;
    /** The dstAddr sub-field specifies the address of source device */
    uint16_t srcAddr;
    /** The sequenceNumber sub-field specifies the sequence number of the packet */
    uint8_t  sequenceNumber;
#ifdef MESH_SECURITY
    /** The meshAuxSecHeader sub-field specifies the auxiliary security information */
    MeshAuxSecFrameHeader_t meshAuxSecHeader;
#endif
} MeshFrameHeader_t;

typedef struct _MeshFrame_t
{
    /** The header sub-field specifies the mesh frame header */
    MeshFrameHeader_t header;
    /** The header sub-field specifies the mesh frame payload length*/
    uint8_t  payloadLen;
    /** The header sub-field specifies the mesh frame payload*/
#ifdef MESH_SECURITY
    uint8_t  payload[MESH_MAX_PAYLOAD_SIZE + 16]; // To support max security level...
#else
    uint8_t  payload[MESH_MAX_PAYLOAD_SIZE];
#endif
} MeshFrame_t;

typedef struct _NwkDataFrame_t
{
	DataConf_callback_t confCallback;
	uint32_t timeout;
	uint16_t nextHopAddr;
	uint8_t frameHeaderLen;
	uint8_t framePayloadLen;
	uint8_t retry;
	uint8_t msghandle;
	bool routedOrRetransmitFrame;
	MeshFrame_t meshFrame;
} NwkDataFrame_t;

typedef struct _NwkFrame_t
{
	NwkDataFrame_t *nextFrame;
	NwkDataFrame_t nwkDataFrame;
}NwkFrame_t;

typedef struct _TxFrameEntry_t
{
	API_UINT64_UNION frameDstAddr;
	DataConf_callback_t frameConfCallback;
	uint8_t *frame;
	MAC_TRANS_PARAM frameParam;
	uint8_t frameLength;
	uint8_t frameHandle;
} TxFrameEntry_t;

typedef struct _TxFrame_t
{
	TxFrameEntry_t *nextFrame;
	TxFrameEntry_t txFrameEntry;
    buffer_t* pMemClr;
} __attribute__((packed, aligned(1)))TxFrame_t;

/** The header sub-field specifies the mesh frame header */
typedef enum _MeshCmdIdentifier_t
{
	/* Join related commands */
    CMD_MESH_BEACON_REQUEST             = 0x01,
    CMD_MESH_BEACON_RESPONSE            = 0x02,
    CMD_MESH_CONNECTION_REQUEST         = 0x03,
    CMD_MESH_CONNECTION_RESPONSE        = 0x04,
    CMD_MESH_ROLE_UPGRADE_REQUEST       = 0x05,
    CMD_MESH_ROLE_UPGRADE_RESPONSE      = 0x06,
	CMD_MESH_KEEP_ALIVE                 = 0x07,
	CMD_MESH_LEAVE                      = 0x08,
	/* Routing related commands */
    CMD_MESH_ROUTE_REQUEST              = 0x11,
    CMD_MESH_ROUTE_REPLY                = 0x12,
    CMD_MESH_ROUTE_TABLE_UPDATE         = 0x13,
	/* Commissioning related commands */
    CMD_MESH_DEVICEAUTH_BLOOMUPDATE     = 0x21,
	/* Data frame related commands */
	CMD_MESH_DATA_REQUEST               = 0x31,
	CMD_MESH_ACK                        = 0x32,
	/* Frequency agility related commands */
	CMD_MESH_CHANNEL_UPDATE             = 0x41,

} MeshCmdIdentifier_t;

#define CMD_MASK              0xF0U
#define CMD_JOIN              0x00U
#define CMD_ROUTE             0x10U
#define CMD_COMM              0x20U
#define CMD_DATA              0x30U
#define CMD_FREQ_AGILITY      0x40U

#define CMD_OFFSET    3U
#define CMD_OFFSET_WITH_ADDRESS    9U

/************************ Prototype Definitions **************************************/
bool checkDuplicateRejectionTableEntry(uint16_t srcAddr, uint8_t seqNo);
void prepareGenericHeader(uint8_t hops, uint16_t srcAddr, uint16_t destAddr, MeshFrameHeader_t* meshHeader);
uint8_t generalFrameConstruct(MeshFrameHeader_t* meshHeader, uint8_t* meshframe);
bool frameTransmit(MeshFrameHeader_t* meshHeader, uint8_t meshFrameHeaderLen, uint8_t meshFramePayloadLen, uint8_t* meshframe,
                    uint8_t macDstAddrLen, uint8_t* macDstAddress, uint8_t handle, DataConf_callback_t ConfCallback, buffer_t* memClrPtr);
void frameParse(MAC_RECEIVED_PACKET *macRxPacket);

bool sendDataFrame(NwkFrame_t *nwkFrame, uint16_t nextHopAddr, DataConf_callback_t confCallback);

void rebroadcastTimerHandler(void);
void initRebroadcastTable(void);
bool addRebroadcastTableEntry(uint16_t srcAddr, uint8_t seqNum);

void duplicateRejectionTimerHandler(void);
void initDuplicateRejectionTable(void);
bool addDuplicateRejectionTableEntry(uint16_t srcAddr, uint8_t seqNum);

void indirectDataTimerHandler(void);
void handleDataMessage(MeshFrameHeader_t *meshHeader, uint8_t* nwkPayload);
#if defined(ENABLE_FREQUENCY_AGILITY)
void handleFreqAgilityMessage(MeshFrameHeader_t *meshHeader, uint8_t* payload);
void channelUpdateTimerExpired(uintptr_t context);
#endif
#endif