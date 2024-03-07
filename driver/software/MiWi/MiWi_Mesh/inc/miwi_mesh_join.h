/**
* \file  miwi_mesh_join.h
*
* \brief MiWi Mesh Protocol Join Handling interface
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

#ifndef MIWI_MESH_JOIN_H
#define MIWI_MESH_JOIN_H

#if defined(PROTOCOL_MESH)
/************************ HEADERS **********************************/
#include "config/default/definitions.h"
/*********************** Macro Definitions **************************************/
#define JOINWISH_ED_MASK            0x01U
#define JOINWISH_COORD_ALONE_MASK   0x02U
#define JOINWISH_ANY_MASK           0x03U

#define DEVICE_TYPE_PANCOORDINATOR  0x01U
#define DEVICE_TYPE_COORDINATOR     0x02U
#define DEVICE_TYPE_ENDDEVICE       0x03U

#define PAN_COORDINATOR_ADDRESS     0x0000U

#define MAX_SCAN_DURATION           14U

#define ESTABLISHMENT_TIMER_EXPIRED   4

#define RXONWHENIDLEMASK               2U
#define RXONWHENIDLE_ED_ADDRESS_MASK   0x80U

/*********************** External Definitions **************************************/
extern volatile uint16_t myShortAddress;
extern uint16_t myParentShortAddress;
extern uint8_t gCapabilityInfo;
extern uint32_t gChannelMap;
#if defined(ENABLE_NETWORK_FREEZER)
extern ReconnectionCallback_t reconnectionCallback;
#endif
#ifndef PAN_COORDINATOR
extern uint8_t edLinkFailureAttempts;
extern bool edInPollingState;
#endif
extern TimerHandles_t timerHandles;
extern uint8_t bloomFilterValue[BLOOM_FILTER_SIZE];
extern uint8_t backupChannel;
/************************ Type Definitions **************************************/

/************************ Prototype Definitions **************************************/
#ifdef COORDINATOR
void roleUpgradeTimerExpired(uintptr_t context);
#endif
uint32_t generateJitterTimeout(uint32_t inputTimeout, uint8_t jitterPercent);
void handleJoinMessage(MeshFrameHeader_t *meshHeader, uint8_t macSrcAddrLen, uint8_t* macSrcAddr, uint8_t* payload, uint8_t lqiValue);
void coordinatorTableInit(void);
void deviceTableInit(void);
void keepAliveTimerHandler(void);
void checkLinkFailureAtNoAck(miwi_status_t status);
bool isCorrectIeeeAddr(const uint8_t *ieeeAddr);
#ifndef ENDDEVICE
void sendForceLeaveNetwork(uint16_t orphAddress);
#else
void startDataWaitIntervalTimer(void);
#endif
#ifndef PAN_COORDINATOR
void rxOffEdKeepAliveTimerHandler(uintptr_t context);
void sendKeepAlive(uintptr_t context);
void sendPollRequest(void);
void dataWaitIntervalTimerHandler(uintptr_t context);
#endif
#endif
#if defined(ENABLE_FREQUENCY_AGILITY)
void performFreqAgility(void);
#endif
#endif