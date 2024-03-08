/*******************************************************************************
  MiWi Init Header File

  Company:
    Microchip Technology Inc.

  File Name:
    miwi_init.h

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

#ifndef MIWI_INIT_H
#define MIWI_INIT_H

#include <stdbool.h>
#include <stdint.h>
#include "config/default/definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define QUEUE_LENGTH (8)
#define QUEUE_ITEM_SIZE (sizeof(void *))

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
typedef enum
{
  MIWI_NONE,
} eMODULE_ID;

typedef struct
{    
   uint8_t uApiID;       // API/Function ID in the respctive module
   uint8_t paramSize;
   void *parameters;      // Function Parameters 
}__attribute__((packed, aligned(1)))STACK_API_Request;

extern OSAL_QUEUE_HANDLE_TYPE miwiRequestQueueHandle;
extern defaultParametersRomOrRam_t *miwiDefaultRomOrRamParams;
extern defaultParametersRamOnly_t *miwiDefaultRamOnlyParams;
extern bool appInitialized;
extern OSAL_API_LIST_TYPE *mimac;
extern uint8_t dummyVal;
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
void MiWi_Init(OSAL_API_LIST_TYPE *osalAPIList,OSAL_QUEUE_HANDLE_TYPE *miwiRequestQueueHandle);
void MiWi_ApplicationInit(void);
void MiMAC_Tasks(void);
void MiMac_PostTask(bool isISRContext);
void MIWI_API_CALL(STACK_API_Request *request);
#ifdef USER_BUTTON_ENABLED
void eic_custom_cb(uintptr_t context);
void eicCbTimerHandler(uintptr_t context);
#endif
#endif//MIWI_INIT_H