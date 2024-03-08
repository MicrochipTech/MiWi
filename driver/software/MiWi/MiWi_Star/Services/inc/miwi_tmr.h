/*******************************************************************************
  MiWi Timer Header File

  Company:
    Microchip Technology Inc.

  File Name:
    miwi_tmr.h

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

#ifndef SYS_TIMER_H
#define SYS_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include "config/default/definitions.h"
// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define SYS_TIMER_INTERVAL      10UL /* ms */
#define MS 1000

#define ONE_SECOND              ((uint32_t)1000000)

#define ONE_MICRO_SECOND    (ONE_SECOND/1000000)
#define ONE_MILI_SECOND     (ONE_SECOND/1000)
#define HUNDRED_MILI_SECOND (ONE_SECOND/10)
#define FORTY_MILI_SECOND   (ONE_SECOND/25)
#define FIFTY_MILI_SECOND   (ONE_SECOND/20)
#define TWENTY_MILI_SECOND  (ONE_SECOND/50)
#define TEN_MILI_SECOND     (ONE_SECOND/100)
#define FIVE_MILI_SECOND    (ONE_SECOND/200)
#define TWO_MILI_SECOND     (ONE_SECOND/500)
#define ONE_MINUTE          (ONE_SECOND*60)
#define ONE_HOUR            (ONE_MINUTE*60)

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
typedef struct SYS_Timer_t {
	/* Internal data */
	struct SYS_Timer_t *next;
	uint32_t timeout;
	/* Timer parameters */
	uint32_t interval;
    SYS_TIME_CALLBACK_TYPE mode;
	void (*handler)(uintptr_t context);
} SYS_Timer_t;

typedef struct TimerHandles {
    SYS_Timer_t joinTimerEstcommandConfcb;//single shot
    SYS_TIME_HANDLE joinTimerHandleEstcommandConfcbHandle;//timer for receiving connection response
    SYS_Timer_t joinTimerRoleUpgrade;//periodic
    SYS_TIME_HANDLE joinTimerRoleUpgradeHandle;//@CMD_CONNECTION_RESPONSE if coordinator
    SYS_Timer_t keepAliveTimerSendKeepAliveRxOnEd;//periodic//reload applicable if rx on Ed gets role upgrade response
    SYS_TIME_HANDLE keepAliveTimerSendKeepAliveRxOnEdHandle;//@CMD_CONNECTION_RESPONSE if coordinator
    SYS_Timer_t joinTimerBeaconReqConfcb;//single shot in beacon cb
    SYS_TIME_HANDLE joinTimerBeaconReqConfcbHandle;
    SYS_Timer_t joinTimerEdScan; //MiApp noise detection single shot
    SYS_TIME_HANDLE joinTimerEdScanHandle;
    SYS_Timer_t joinTimerchannelUpdate;//single shot for channel frequency agility
    SYS_TIME_HANDLE joinTimerchannelUpdateHandle;
    SYS_Timer_t keepAliveTimerRxOffEd;//single shot timer for sending data request from ED
    SYS_TIME_HANDLE keepAliveTimerRxOffEdHandle;
    SYS_Timer_t dataWaitIntervalTimerEd;//single shot timer for waiting for data after sending data request 
    SYS_TIME_HANDLE dataWaitIntervalTimerEdHandle;
}TimerHandles_t;

typedef union _MIWI_TICK
{
    uint32_t Val;
    struct _MIWI_TICK_bytes
    {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    } byte;
    uint8_t v[4];
    struct _MIWI_TICK_words
    {
        uint16_t w0;
        uint16_t w1;
    } word;
} MIWI_TICK;

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************



#endif /* _SYS_TIMER_H_ */
