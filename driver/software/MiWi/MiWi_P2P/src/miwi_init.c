/*******************************************************************************
  MiWi Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "config/default/definitions.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// Queue Pointer received from Application
extern OSAL_QUEUE_HANDLE_TYPE apiRequestQueueHandle; 
defaultParametersRomOrRam_t *miwiDefaultRomOrRamParams = NULL;
defaultParametersRamOnly_t *miwiDefaultRamOnlyParams = NULL;
uint8_t dummyVal = 0U;
#ifdef USER_BUTTON_ENABLED
uint8_t buttonCnt = 0U;
SYS_TIME_HANDLE eicCbTimerHandle;
SYS_Timer_t eicCbTimer;
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void MiWi_Init(OSAL_API_LIST_TYPE *osalAPIList,OSAL_QUEUE_HANDLE_TYPE *miwiRequestQueueHandle)

  Remarks:
    See prototype in miwi_task.h.
*/
void MiWi_Init(OSAL_API_LIST_TYPE *osalAPIList,OSAL_QUEUE_HANDLE_TYPE *miwiRequestQueueHandle)
{
  mimac = osalAPIList;   // Store the OSAL functions to local pointer
  apiRequestQueueHandle = *miwiRequestQueueHandle;    // API Request queue from application task  
   /* Store the ROmRAM parameter references */
  miwiDefaultRomOrRamParams = &defaultParamsRomOrRam;
  miwiDefaultRamOnlyParams = &defaultParamsRamOnly;
}

/*******************************************************************************
  Function:
    void MiWi_ApplicationInit()

  Remarks:
    See prototype in app_miwi.h.
******************************************************************************/
void MiWi_ApplicationInit(void)
{
    MiApp_Init();
}


#ifdef USER_BUTTON_ENABLED
void eic_custom_cb(uintptr_t context)
{ 
    if(buttonCnt == 0U)
    {
        eicCbTimer.handler = eicCbTimerHandler;
        eicCbTimer.interval = 1000uL;
        eicCbTimer.timeout = 1000uL;
        eicCbTimer.mode = SYS_TIME_SINGLE;
        eicCbTimerHandle = SYS_TIME_CallbackRegisterMS(&eicCbTimerHandler, (uintptr_t)&dummyVal, eicCbTimer.interval, eicCbTimer.mode); 
        if(eicCbTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
    }
    buttonCnt += 1U;
    if (buttonCnt >= 3U)
    {
        buttonCnt = 0U;
    }     
}
 
void eicCbTimerHandler(uintptr_t context)
{
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg;
 
    if(buttonCnt == 2U) //short press
    {
        buttonCnt = 0U;
        appState->msgId = APP_STATE_BROADCAST_DATA;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
    }
    else if(buttonCnt == 1U)//long press
    {
        buttonCnt = 3U;
        appState->msgId = APP_STATE_UNICAST_DATA;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
    }
    else
    {
        //Do nothing
    }

    (void)context;
}
#endif
