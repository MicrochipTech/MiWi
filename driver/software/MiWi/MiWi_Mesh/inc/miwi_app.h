/*******************************************************************************
  WSN Demo Header File

  Company:
    Microchip Technology Inc.

  File Name:
    miwi_app.h

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

#ifndef MIWIAPP_H
#define MIWIAPP_H

#include <stdbool.h>
#include <stdint.h>
#include "config/default/definitions.h"
#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
typedef enum AppState_t {
	APP_STATE_INITIAL,
	APP_STATE_START_NETWORK,
	APP_STATE_CONNECT_NETWORK,
	APP_STATE_CONNECTING_NETWORK,
	APP_STATE_IN_NETWORK,
	APP_STATE_WAIT_FOR_RECONNECT_CALLBACK,
	APP_STATE_RECONNECT_SUCCESS,
	APP_STATE_RECONNECT_FAILURE,
	APP_STATE_SEND,
	APP_STATE_WAIT_CONF,
	APP_STATE_SENDING_DONE,
    APP_STATE_DATA_RECEIVE_IND,
	APP_STATE_WAIT_SEND_TIMER,
	APP_STATE_WAIT_COMMAND_TIMER,
	APP_STATE_PREPARE_TO_SLEEP,
	APP_STATE_SLEEP,
    APP_STATE_FREQUENCY_AGILITY,
    APP_STATE_NOISE_DETECTION,
} AppState_t;

extern AppState_t appStates;
extern uint8_t channelCount;
extern uint8_t maxRSSI;
//extern bool deviceCanSleep;
extern SYS_TIME_HANDLE testtimehandle;
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
void Wsndemo_Init(void);
void Demomsg_Init(void);
void MiAppTimer_Init(void);
void MiApp_Init(void);
void wsndemo_task(void);
void MiAPP_TaskHandler(APP_Msg_T *appState);
void MiMac_TaskHandler(void);
void MiMesh_TaskHandler(void);
void MiApp_StateInit(void);
void MiMAC_RFDDemoInit(void);
void MiMAC_FFDDemoInit(void);
void appSendData(AppMessage_t *appMsg);
#ifndef PAN_COORDINATOR
void searchConfim(uint8_t foundScanResults, void* ScanResults);
void appLinkFailureCallback(void);
#endif
#ifdef ENABLE_SLEEP_FEATURE
uint32_t MAC_ReadyToSleep(void);
bool APP_ReadyToSleep(uint32_t *sleepDuration);
#endif
void Rx_On(bool isProm);
#endif /* MIWIAPP_H */
