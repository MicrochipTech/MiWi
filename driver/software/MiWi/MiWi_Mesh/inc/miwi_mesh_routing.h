/**
* \file  miwi_mesh_routing.h
*
* \brief MiWi Mesh Protocol Route Handling interface
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

#ifndef MIWI_MESH_ROUTING_H
#define MIWI_MESH_ROUTING_H

/************************ HEADERS **********************************/
#include "config/default/definitions.h"

/*********************** Macro Definitions **************************************/
#define NO_NEXT_HOP   0xFFU
#define NO_HOP_COUNT  0x00U
#define NO_LQI        0x00U
#define NO_SCORE      0x00U

#define MAX_HOP       0xFFU

#define MAX_HOP_COUNT 0x0FU

#define DEFAULT_SCORE 0x03U

#define DEFAULT_NEXT_HOP_ADDR (0xFFFFU)


#define CURRENT_HOP_COUNT(coordCount) (((coordCount % 2U) == 0U)? \
(coordCount / 2U) : \
((coordCount / 2U) + 1U))

/*********************** External Definitions **************************************/
extern queue_t routeFrameQueue;
#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
extern bool routeTestMode;
#endif
/************************ Type Definitions **************************************/

/************************ Prototype Definitions **************************************/
void initRouteTable(void);
bool handleRouteMessage(MeshFrameHeader_t *meshHeader, uint8_t macSrcAddrLen,
						uint8_t* macSrcAddr, uint8_t* payload, uint8_t lqiValue);
bool addRoute(uint16_t routerAddr, uint16_t nextHopAddr, uint8_t noOfHops, uint8_t rcvdLqi);
bool removeRoute(uint16_t routerAddr);
bool initiateRouteReq(uint16_t devAddr);
void setNextHopAddr(uint16_t coordAddr, uint16_t nextHopAddr);
uint16_t getNextHopAddr(uint16_t coordAddr);
uint8_t getNumberOfCoordinators(void);
void setHopCount(uint8_t index, uint8_t value);
uint8_t getHopCount(uint8_t index);
void routeTimerHandler(void);
#endif