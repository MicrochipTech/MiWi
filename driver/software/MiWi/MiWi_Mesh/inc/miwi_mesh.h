/*******************************************************************************
  MiWi Mesh Header File

  Company:
    Microchip Technology Inc.

  File Name:
    miwi_mesh.h

  Summary:
    This file contains the System Sleep functions for the project.

  Description:
    This file contains the System Sleep functions for the project.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END
#ifndef MIWI_MESH_H
#define MIWI_MESH_H

/************************ HEADERS **********************************/
#include "config/default/definitions.h"
// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#if defined(ENABLE_FREQUENCY_AGILITY)
#define CHANNEL_UPDATE_TIME_IN_SEC        15U
#endif

#define MAC_BROADCAST_ADDR               (0xFFFFU)

#define SECURITY_FAILURE                 (0xFFU)

#define IS_CORRECT_BROADCAST_ADDR(A) \
((MESH_BROADCAST_TO_ALL == (A))||((MESH_BROADCAST_TO_COORDINATORS <= LE16_TO_CPU(A))&&(LE16_TO_CPU(A) <= MESH_BROADCAST_TO_FFDS)))

#define COORD_MASK          0xFF00U
#define ENDDEVICE_MASK      0x00FFU

#define SW_TIMER_INTERVAL   1000U

#define QUEUE_CAPACITY 30U

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
/* State of the Mesh stack */
typedef enum meshState_
{
    /* State during boot condition */
    INITIAL_STATE,
    /* State where Mesh protocol Init is completed */
    INIT_STATE,
    /* State where Network Start operation is in progress */
    STARTING_NETWORK,
    /* State where device is searching for network to join */
    SEARCHING_NETWORK,
    /* State where device is connecting with a device/parent */
    ESTABLISHING_NETWORK,
    /* State where search operation is complete */
    SEARCH_COMPLETE,
    /* Device successfully started or joined into a network */
    IN_NETWORK_STATE,
    /* Device successfully joined into a network, but it is not in expected role */
    IN_NETWORK_PARTIAL_STATE,
	/* Device is disconnected from network - applicable only for End devices*/
	DISCONNECTED
}meshState_t;

/*********************** External Definitions **************************************/
/* State of the Mesh stack */
extern meshState_t meshCurrentState;
extern uint8_t  meshSequenceNumber;
extern uint8_t newChannelToUpdate;
#if defined(ENABLE_FREQUENCY_AGILITY)
extern uint16_t backupParentNwkAddress;
#endif
extern queue_t frameTxQueue;
extern queue_t frameRxQueue;
/* If a module is busy, it can increment this busylock so that device wont enter into sleep */
extern uint8_t busyLock;
extern queue_t sendFrameQueue;
extern queue_t ackWaitFrameQueue;
extern queue_t nonAckFrameQueue;
#ifndef ENDDEVICE
extern queue_t indirectFrameQueue;
#endif
//#if defined(ENABLE_NETWORK_FREEZER)
#ifdef ENDDEVICE
void connectionConfirm(miwi_status_t status);
#endif
//#endif
extern API_UINT16_UNION myPANID;
extern uint8_t currentChannel;
extern miwi_status_t initStatus;
extern bool pdsRestoreFlag; 
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
/************************ Prototype Definitions **************************************/
bool isSameAddress(uint8_t *Address1, uint8_t *Address2);
void ackReqDataConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
void nonAckDataCallback(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
uint8_t calculateAckWaitTimeout(uint8_t hops, uint16_t destAddr);

#endif