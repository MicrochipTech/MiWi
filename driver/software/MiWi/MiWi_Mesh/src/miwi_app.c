/*******************************************************************************
  MiWi Demo Source File

  Company:
    Microchip Technology Inc.

  File Name:
    wsndemo.c

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
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "config/default/definitions.h"
#include "config/default/driver/IEEE_802154_PHY/pal/inc/pal.h"
// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define APP_SCAN_DURATION 10U

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************

/*- Variables --------------------------------------------------------------*/
AppState_t appStates;
static SYS_Timer_t appDataSendingTimer;
static SYS_TIME_HANDLE appDataSendingTimerHandle;
//SYS_TIME_HANDLE testtimehandle;
#if defined(COORDINATOR) || defined (ENDDEVICE)
static SYS_Timer_t appNetworkStatusTimer;
static SYS_TIME_HANDLE appNetworkStatusTimerHandle;
static bool appNetworkStatus;
#endif
AppMessage_t appMsg;
#ifndef PAN_COORDINATOR
static uint8_t wsnmsghandle;
#endif
static bool deviceCanSleep = false;
static STACK_API_Request apiReq;
uint8_t channelCount = 0U;
uint8_t maxRSSI;
uint8_t bloomFilterValue[BLOOM_FILTER_SIZE];
uint8_t maxNumDevices = MAX_NUMBER_OF_DEVICES_IN_NETWORK;
uint8_t acceptedDeviceCount = 0;
// Increase the number of devices that can join the network by changing the parameter "numDevices".  Also add the device address, which can form a part of the network, in the array "acceptedDeviceAddress".
//This Address filtering feature is activated only by enabling the macor "ADDRESS_CHECK"
uint8_t const numDevices = 1;
uint64_t acceptedDeviceAddress [1] = {0x22665544332211AB};//{0x33665544332211AB, 0x22665544332211AB,0x55665544332211AB,0x66665544332211AB,0x77665544332211AB,0x88665544332211AB, 0x99665544332211AB, 0x44665544332211AB, 0x11555544332211AB,0x22555544332211AB};
uint8_t deviceCountRem = numDevices;
static uint8_t gOptimalChannel = 0xFFU;
static uint8_t minRSSI = 0xFFU;
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
static inline uint32_t miwi_app_scan_duration_ticks(uint8_t scan_duration);
static void Connection_Confirm(miwi_status_t status);
#ifdef ENABLE_SLEEP_FEATURE
static void MAC_ReadyToDeepSleep(void);
static void app_initiate_polling(void *parameter);
#endif
#if defined(ENABLE_NETWORK_FREEZER)
static void ReconnectionIndication (miwi_status_t status);
#endif
static void appDataInd(RECEIVED_MESH_MESSAGE *ind);
/*- Implementations --------------------------------------------------------*/

/*****************************************************************************
*****************************************************************************/
static void appDataInd(RECEIVED_MESH_MESSAGE *ind)
{
    AppMessage_t *msg = (AppMessage_t *)ind->payload;
 
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Toggle();
#else
    LED_Toggle(1,LED_DATA);
#endif
#endif
#endif
#endif
 
    msg->lqi = ind->packetLQI;
    msg->rssi = (int8_t)ind->packetRSSI;
    SYS_CONSOLE_PRINT("\r\nPayload received\r\n");
#if defined(PAN_COORDINATOR)
    for(uint8_t i=0U; i<ind->payloadSize; i++)
    {
        SYS_CONSOLE_PRINT("%x",ind->payload[i]);
    }
        SYS_CONSOLE_PRINT("\n");
#else
//    appCmdDataInd(ind);
    for(uint8_t i=0U; i<ind->payloadSize; i++)
    {
        SYS_CONSOLE_PRINT("%x",ind->payload[i]);
    }
    SYS_CONSOLE_PRINT("\n");
#endif
#if defined(ENABLE_SLEEP_FEATURE) && defined(ENDDEVICE)
#if (CAPABILITY_INFO == CAPABILITY_INFO_ED)
    deviceCanSleep = true;
    APP_Msg_T sleepReq;
    sleepReq.msgId = (uint8_t)APP_STATE_DATA_RECEIVE_IND;  
    appStates = APP_STATE_DATA_RECEIVE_IND;
    OSAL_QUEUE_Send(&appData.appQueue, &sleepReq, 0);
#endif
#endif
}

/*****************************************************************************
*****************************************************************************/
static void appDataSendingTimerHandler(uintptr_t context) 
{
    APP_Msg_T *p_appModes;
    APP_Msg_T appModes;
    p_appModes = &appModes;
	if ((APP_STATE_WAIT_SEND_TIMER == appStates) || (APP_STATE_PREPARE_TO_SLEEP == appStates)) 
	{
		appStates = APP_STATE_SEND;
        p_appModes->msgId = (uint8_t)APP_STATE_SEND;
        OSAL_QUEUE_Send(&appData.appQueue, p_appModes, 0);
	}
	else
	{
        appStates = APP_STATE_WAIT_SEND_TIMER;
        uint8_t myData = 0U;
        appDataSendingTimerHandle = SYS_TIME_CallbackRegisterMS(&appDataSendingTimerHandler, (uintptr_t)&myData,APP_SENDING_INTERVAL, SYS_TIME_SINGLE);
        if(appDataSendingTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
	}

    (void)context;
}

#if defined(COORDINATOR) || defined (ENDDEVICE)

/*****************************************************************************
*****************************************************************************/
static void appNetworkStatusTimerHandler(uintptr_t context)
{
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Toggle();
#else
	LED_Toggle(1,LED_NETWORK);
#endif
#endif
#endif
#endif
	(void)context;
}
#endif

/*****************************************************************************
*****************************************************************************/
#if defined(COORDINATOR) || defined (ENDDEVICE)
static void appDataConf(uint8_t msgConfHandle, miwi_status_t status, uint8_t* msgPointer)
{
    APP_Msg_T *p_appModes;
    APP_Msg_T appModes;
    p_appModes = &appModes;
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Off();
#else
	LED_Off(1,LED_DATA);
#endif
#endif
#endif
#endif

	if (SUCCESS == status) {
		if (!appNetworkStatus) {
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_On();
#else
	LED_On(1, LED_NETWORK);
#endif
#endif
#endif
#endif

            SYS_TIME_TimerStop(appNetworkStatusTimerHandle);   
			appNetworkStatus = true;
		}
	} else {
		if (appNetworkStatus) {
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Off();
#else
	LED_Off(1,LED_NETWORK);
#endif
#endif
#endif
#endif
            uint8_t myData  = 0;
        appNetworkStatusTimerHandle = SYS_TIME_CallbackRegisterMS(&appNetworkStatusTimerHandler, (uintptr_t)&myData,APP_NWKSTATUS_INTERVAL, SYS_TIME_PERIODIC);
        if(appNetworkStatusTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
			appNetworkStatus = false;
		}
	}
	if ((APP_STATE_WAIT_CONF == appStates) || (APP_STATE_PREPARE_TO_SLEEP == appStates))
	{
		appStates = APP_STATE_SENDING_DONE;
        p_appModes->msgId = (uint8_t)APP_STATE_SENDING_DONE;
        OSAL_QUEUE_Send(&appData.appQueue, p_appModes, 0);
	}
}

#endif

/*****************************************************************************
*****************************************************************************/
void appSendData(AppMessage_t *appMsg)
{
    uint8_t myData = 0U;
    APP_Msg_T  appMode;
    APP_Msg_T *appState;
    appState = &appMode;
#ifndef PAN_COORDINATOR
    uint16_t dstAddr = 0U; /* PAN Coordinator Address */
#endif
#if defined(OTAU_ENABLED)
	otau_log(LOG_INFO, MIWI_APP, ENTRY, 8, (uint8_t *)"App Data");
	otau_trace(TRACE_ENTRY);
#endif

#ifndef PAN_COORDINATOR
    /* Get Next Hop Short address to reach PAN Coordinator*/
	appMsg->nextHopAddr = MiApp_MeshGetNextHopAddr(PAN_COORDINATOR_SHORT_ADDRESS);
#endif

#if defined(PAN_COORDINATOR)
    appDataSendingTimerHandle = SYS_TIME_CallbackRegisterMS(&appDataSendingTimerHandler, (uintptr_t)&myData,APP_SENDING_INTERVAL, SYS_TIME_SINGLE);
    if(appDataSendingTimerHandle == SYS_TIME_HANDLE_INVALID)
    {
        return;
    }
    appStates = APP_STATE_WAIT_SEND_TIMER;
	appState->msgId = APP_STATE_WAIT_SEND_TIMER;
    OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
#else
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_On();
#else
	LED_On(1, LED_DATA);
#endif
#endif
#endif
#endif

#ifdef ENABLE_SECURITY
	if (MiApp_SendData(2, (uint8_t *)&dstAddr, (uint8_t)sizeof(AppMessage_t), (uint8_t *)appMsg, wsnmsghandle, true, appMsg->ConfCallback))
#else
    if (MiApp_SendData(2, (uint8_t *)&dstAddr, (uint8_t)sizeof(AppMessage_t), (uint8_t *)appMsg, wsnmsghandle, true, appMsg->ConfCallback))
#endif
	{
		++wsnmsghandle;
        appStates = APP_STATE_WAIT_CONF;
		appState->msgId = (uint8_t)APP_STATE_WAIT_CONF;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
	}
	else
	{
		appState->msgId = (uint8_t)APP_STATE_SENDING_DONE;
        appStates = APP_STATE_SENDING_DONE;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
	}
#endif
}

/*************************************************************************//**
*****************************************************************************/
void MiApp_Init(void)
{
    Wsndemo_Init();
    Demomsg_Init();   
    MiAppTimer_Init();
	MiApp_SubscribeDataIndicationCallback(appDataInd);
#ifndef PAN_COORDINATOR
	MiApp_SubscribeLinkFailureCallback(appLinkFailureCallback);
#endif
    Rx_On(false);
    appInitialized = true;
#ifdef USER_BUTTON_ENABLED
#if defined(CHIMERA_SOC)
    EIC_CallbackRegister(EIC_PIN_0, eic_custom_cb, dummyVal);
#else
    EIC_CallbackRegister(EIC_PIN_2, eic_custom_cb, dummyVal);
#endif
#endif
}

#if defined(ENABLE_NETWORK_FREEZER)
static void ReconnectionIndication (miwi_status_t status)
{
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg;
	if(SUCCESS == status)
	{
        appStates = APP_STATE_RECONNECT_SUCCESS;
//		appState->msgId = APP_STATE_RECONNECT_SUCCESS;
//        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
	}
	else
	{
        appStates = APP_STATE_RECONNECT_FAILURE;
//		appState->msgId = APP_STATE_RECONNECT_FAILURE;
//        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
	}
}
#endif

void MiApp_StateInit(void)
{
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg;  
#if defined(ENABLE_NETWORK_FREEZER)
         if (appStates == APP_STATE_RECONNECT_SUCCESS) 
         {
         appStates = APP_STATE_RECONNECT_SUCCESS;
         appState->msgId = (uint8_t)APP_STATE_RECONNECT_SUCCESS;
         OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
         }
         else if(appStates == APP_STATE_WAIT_FOR_RECONNECT_CALLBACK)
         {
         appStates = APP_STATE_WAIT_FOR_RECONNECT_CALLBACK;
         appState->msgId = (uint8_t)APP_STATE_WAIT_FOR_RECONNECT_CALLBACK;
         OSAL_QUEUE_Send(&appData.appQueue, appState, 0);             
         }
         else
#endif
         {
#if defined(PAN_COORDINATOR)
        appStates = APP_STATE_START_NETWORK;
        appState->msgId = (uint8_t)APP_STATE_START_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
#else
        appStates = APP_STATE_CONNECT_NETWORK;
        appState->msgId = (uint8_t)APP_STATE_CONNECT_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
#endif 
         }
}

/*************************************************************************//**
*****************************************************************************/
void MiAPP_TaskHandler(APP_Msg_T *appState) 
{
	switch (appState->msgId) 
    {
#if defined(ENABLE_NETWORK_FREEZER)
	case (uint8_t)APP_STATE_RECONNECT_SUCCESS:
	{
        appStates = APP_STATE_SEND;
        appState->msgId = (uint8_t)APP_STATE_SEND;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
	break;
    }
    case (uint8_t)APP_STATE_WAIT_FOR_RECONNECT_CALLBACK:
    {
        if(appStates == APP_STATE_RECONNECT_SUCCESS)
        {
        appStates = APP_STATE_RECONNECT_SUCCESS;
        appState->msgId = (uint8_t)APP_STATE_RECONNECT_SUCCESS;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);            
        }
        else if(appStates == APP_STATE_RECONNECT_FAILURE)
        {
        appStates = APP_STATE_RECONNECT_FAILURE;
        appState->msgId = (uint8_t)APP_STATE_RECONNECT_FAILURE;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);              
        }
        else
        {
        appStates = APP_STATE_WAIT_FOR_RECONNECT_CALLBACK;
        appState->msgId = (uint8_t)APP_STATE_WAIT_FOR_RECONNECT_CALLBACK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);                
        }
            
        break;
    }
    case (uint8_t)APP_STATE_RECONNECT_FAILURE:
    {
#if defined(PAN_COORDINATOR)
        appStates = APP_STATE_START_NETWORK;
        appState->msgId = (uint8_t)APP_STATE_START_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
#else
        appStates = APP_STATE_CONNECT_NETWORK;
        appState->msgId = (uint8_t)APP_STATE_CONNECT_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
#endif 
        break;
    }
#endif    

#if defined(PAN_COORDINATOR)
	case (uint8_t)APP_STATE_START_NETWORK:
	{
        startNetworkReq_t startNwReq;
        startNwReq.ChannelMap = (uint32_t)CHANNEL_MAP;
        startNwReq.Mode = START_CONN_DIRECT;
        startNwReq.ScanDuration = APP_SCAN_DURATION;
        startNwReq.ConfCallback = Connection_Confirm;
        apiReq.parameters = (void*)&startNwReq;
        apiReq.paramSize = (uint8_t)sizeof(startNetworkReq_t);
        apiReq.uApiID = (uint8_t)MIWI_START_NW;
        MIWI_API_CALL((STACK_API_Request*)&apiReq);
        APP_Msg_T    appMsg;
        APP_Msg_T *appState;
        appState = &appMsg;
        appStates = APP_ADD_ACCEPTED_ADDRESS;
        appState->msgId = (uint8_t)APP_ADD_ACCEPTED_ADDRESS;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
	break;
	}

    case APP_ADD_ACCEPTED_ADDRESS:
    {
        if (deviceCountRem !=0)
        {
            if (!bloomFilterAutoJoin)
                {
                    if (acceptedDeviceAddress[acceptedDeviceCount]!= 0)
                        {
                            bool addNewDevice = MiApp_Commissioning_AddNewDevice(acceptedDeviceAddress[acceptedDeviceCount], true);
                            if (addNewDevice == true);
                            {
                                deviceCountRem--;
                                acceptedDeviceCount++;
                            }
                        }
                }
        }  
    break;
    }

#else
	case (uint8_t)APP_STATE_CONNECT_NETWORK:
	{
        searchNetworkReq_t searchNwReq;
        searchNwReq.ScanDuration = APP_SCAN_DURATION;
        searchNwReq.ChannelMap = (uint32_t)CHANNEL_MAP;
        searchNwReq.ConfCallback = searchConfim;
        apiReq.uApiID = (uint8_t)MIWI_CONNECT_NW;
        apiReq.paramSize = (uint8_t)sizeof(searchNetworkReq_t);
        apiReq.parameters = (void*)&searchNwReq;
        MIWI_API_CALL(&apiReq);
    	break;
	}

#endif
#if defined(COORDINATOR) || defined (ENDDEVICE)
	case (uint8_t)APP_STATE_SEND:
	{
        AppMessage_t *p_appMsg = NULL;
        appMsg.sensors.battery     = rand() & 0xffff;
	    appMsg.sensors.temperature = rand() & 0x7f; //sensor data or any user data
	    appMsg.sensors.light       = rand() & 0xff;
        /* Get Short address */
	    MiApp_Get(SHORT_ADDRESS, (uint8_t *)&appMsg.shortAddr);
	    appMsg.extAddr   = appMsg.shortAddr;
	    MiApp_Get(CHANNEL, (uint8_t *)&appMsg.workingChannel);
	    MiApp_Get(PANID, (uint8_t *)&appMsg.panId);
        p_appMsg = &appMsg;
        appMsg.caption.type         = 32;
        #if defined(COORDINATOR)
	    if (appMsg.shortAddr & RXON_ENDEVICE_ADDRESS_MASK)
	    {
		appMsg.caption.size         = APP_CAPTION_ED_SIZE;
//	    memcpy(appMsg.caption.text, APP_CAPTION_ED, APP_CAPTION_ED_SIZE);
//		sprintf(&(appMsg.caption.text[APP_CAPTION_ED_SIZE - SHORT_ADDRESS_CAPTION_SIZE]), "-0x%04X", shortAddressLocal);
	   }
	   else
       #endif
	   {
	    appMsg.caption.size         = (uint8_t)APP_CAPTION_SIZE;
//	    memcpy(appMsg.caption.text, APP_CAPTION, APP_CAPTION_SIZE);
//		sprintf(&(appMsg.caption.text[APP_CAPTION_SIZE - SHORT_ADDRESS_CAPTION_SIZE]), "-0x%04X", shortAddressLocal);
	    }
        appMsg.ConfCallback = appDataConf;
        apiReq.parameters = p_appMsg;
        apiReq.paramSize = (uint8_t)sizeof(appMsg);
        apiReq.uApiID = (uint8_t)MIWI_SEND_DATA;
        MIWI_API_CALL((STACK_API_Request*)&apiReq);
		break;
    }
#endif
    case (uint8_t)APP_STATE_DATA_RECEIVE_IND:
	case (uint8_t)APP_STATE_SENDING_DONE:
	{
        uint8_t myData = 0U;
#if defined(ENABLE_SLEEP_FEATURE) && defined(ENDDEVICE) && (CAPABILITY_INFO == CAPABILITY_INFO_ED)
        deviceCanSleep = true;
        appDataSendingTimerHandle = SYS_TIME_CallbackRegisterMS(&appDataSendingTimerHandler, (uintptr_t)&myData,APP_SENDING_INTERVAL, SYS_TIME_SINGLE);
//        testtimehandle = appDataSendingTimerHandle;
        if(appDataSendingTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
        appStates = APP_STATE_PREPARE_TO_SLEEP;
		appState->msgId = (uint8_t)APP_STATE_PREPARE_TO_SLEEP;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
#else
        appDataSendingTimerHandle = SYS_TIME_CallbackRegisterMS(&appDataSendingTimerHandler, (uintptr_t)&myData,APP_SENDING_INTERVAL, SYS_TIME_SINGLE);
        if(appDataSendingTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
        appStates = APP_STATE_WAIT_SEND_TIMER;
		appState->msgId = (uint8_t)APP_STATE_WAIT_SEND_TIMER;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
#endif
		break;
    }

#if defined(COORDINATOR) || defined(ENDDEVICE)
    case (uint8_t)APP_STATE_WAIT_SEND_TIMER:
    case (uint8_t)APP_STATE_WAIT_CONF:
    {
        //do nothing just wait
            break;
    }
#endif
#if defined(ENABLE_SLEEP_FEATURE) && defined(ENDDEVICE)
	case (uint8_t)APP_STATE_PREPARE_TO_SLEEP:
	{
    uint32_t sleepDuration;
    if (APP_ReadyToSleep(&sleepDuration))
    {    
        /* Enter system sleep mode */
        DEVICE_EnterDeepSleep(true, sleepDuration);
    }
    else
    {
        APP_Msg_T sleepReq;
        appStates = APP_STATE_PREPARE_TO_SLEEP;
        sleepReq.msgId = (uint8_t)APP_STATE_PREPARE_TO_SLEEP;      
        OSAL_QUEUE_Send(&appData.appQueue, &sleepReq, 0); 
    } 
		break;
    }
#endif
#if defined(ENABLE_FREQUENCY_AGILITY)
    case (uint8_t)APP_STATE_FREQUENCY_AGILITY:
    {
//        MiApp_InitChannelHopping(FULL_CHANNEL_MAP);
        if( appState->msgData[0] > maxRSSI )
        {
            maxRSSI = appState->msgData[0];
        }
        // if scan time exceed scan duration, prepare to scan the next channel
            if( maxRSSI < minRSSI )
            {
                minRSSI = maxRSSI;
                gOptimalChannel = channelCount-1;
            }    
        appStates = APP_STATE_NOISE_DETECTION;
            appState->msgId = (uint8_t)APP_STATE_NOISE_DETECTION;
            OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
        break;
    }
    
    case (uint8_t)APP_STATE_NOISE_DETECTION:
    {
        uint8_t ScanDuration = 3;
        uint32_t channelMask = 0x00000001;
	    /* Check the given channel is within the range and callback is not NULL*/
	    if (!(MiMAC_GetPHYChannelInfo() & FULL_CHANNEL_MAP))
        {
            return;
        }
        if(channelCount < 32)
        {
            if(FULL_CHANNEL_MAP & MiMAC_GetPHYChannelInfo() & (channelMask << channelCount))
            {
                        maxRSSI = 0;
                /* choose appropriate channel */
                MiApp_Set(CHANNEL, &channelCount);
                PHY_EdStart(ScanDuration);
            }
            else
            {
                appStates = APP_STATE_NOISE_DETECTION;
                appState->msgId = (uint8_t)APP_STATE_NOISE_DETECTION;
                OSAL_QUEUE_Send(&appData.appQueue, appState, 0);
            }
            channelCount++;
        }
        else
        {
            MiApp_InitChannelHopping(FULL_CHANNEL_MAP, gOptimalChannel);
        }
       break;     
    }
#endif
    
	default:
        //Handle exceptions if any
		break;
	}
}

static inline uint32_t miwi_app_scan_duration_ticks(uint8_t scan_duration)
{
    uint32_t scan_symbols;

    scan_symbols =  ABASESUPERFRAMEDURATION *((1<<scan_duration) + 1);
    return MiMAC_SymbolToTicks(scan_symbols);
}
#ifdef ENABLE_SLEEP_FEATURE
bool APP_ReadyToSleep(uint32_t *sleepDuration)
{
    bool sleep = false;
//    deviceCanSleep = true;
    if(deviceCanSleep)
    {
        if(MAC_ReadyToSleep())
        {           
            *sleepDuration = APP_POLL_PERIOD_MS;
            sleep = true;                         
        }
    }    
    return sleep;
}

uint32_t MAC_ReadyToSleep(void)
{
	uint32_t sleepTime = 0;
    PHY_TrxStatus_t trxStatus = PHY_GetTrxStatus();
	if ((busyLock != 0U) || (frameTxQueue.size != 0U) || (frameRxQueue.size != 0U) || (trxStatus != PHY_TRX_SLEEP))
    {
        SYS_CONSOLE_PRINT("\r\n no sleep\r\n");
		sleepTime = 0;
	} else 
    {
        
		sleepTime = READY_TO_SLEEP;
#ifdef ENABLE_SLEEP_FEATURE
        MAC_ReadyToDeepSleep();
#endif
	}
	return sleepTime;
}

typedef __PACKED_STRUCT mac_ds_param
{
    uint64_t mac_CoordExtAddr;  
    uint64_t mac_ieee_addr; 
    uint32_t mac_CoordShtAddr; 
    uint32_t mac_short_addr;
    uint32_t panid;
    uint32_t mac_max_frame_total_wait_time;
    uint32_t mac_response_wait_time; 
    uint32_t mesh_state;  
    uint32_t mac_radio_sleep_state;
    uint32_t app_state;
    uint32_t mac_poll_state; 
    uint32_t mac_associated_PAN_coord;   
    uint32_t mac_auto_request; 
    uint32_t mac_batt_life_ext_periods; 
    uint32_t mac_dsn; 
    uint32_t phy_current_channel;
}MAC_Ds_Param_t;

static uint16_t    macshortaddr; 
static uint64_t    macieeeaddr;
static uint16_t    panid;
PHY_TrxStatus_t macRadioSleepState;

#if ((defined MAC_SECURITY_ZIP)  || (defined MAC_SECURITY_2006))
MAC_SecPib_t __attribute__ ((persistent)) macSecPibBackup;
#endif

MAC_Ds_Param_t __attribute__((persistent)) mdsParam;

static void memcpy4ByteAligned(void* outbuf, void* inbuf, uint16_t length)
{
  static uint16_t mod_size;
  static uint16_t size;
  static uint16_t k;
  uint32_t* src = (uint32_t* )inbuf;
  uint32_t* dst = (uint32_t* )outbuf;

  mod_size = (length % 4U);
  // total_length is in multiple of 4
  if (mod_size !=0U)
    size  = length + 4U - mod_size; 
  else 
    size  = length;

  size  = size >> 2U;
  for (k = 0U; k < size; k++)
  {
      *dst = *src;
      src++;
      dst++;
  }
}

static void MAC_ReadyToDeepSleep(void)
{  
    MAC_Ds_Param_t param; 
    uint16_t mac_CoordShtAddr = 0;    
//    param.mac_CoordExtAddr = macPib.mac_CoordExtendedAddress;
    MiApp_Get(PARENT_SHORT_ADDRESS, (uint8_t*)&mac_CoordShtAddr);
    param.mac_CoordShtAddr = (uint32_t)mac_CoordShtAddr;
//    param.mac_max_frame_total_wait_time = macPib.mac_MaxFrameTotalWaitTime;
//    param.mac_response_wait_time = macPib.mac_ResponseWaitTime;    
    param.mesh_state = (uint32_t)meshCurrentState;    
    param.app_state = (uint32_t)appStates;
//    param.mac_poll_state = macPollState;
    macRadioSleepState = PHY_GetTrxStatus();
    param.mac_radio_sleep_state = (uint32_t)macRadioSleepState;    
//    param.mac_associated_PAN_coord = macPib.mac_AssociatedPANCoord;
//    param.mac_auto_request = macPib.mac_AutoRequest;      
//    param.mac_batt_life_ext_periods = macPib.mac_BattLifeExtPeriods;
    param.mac_dsn = (uint32_t)meshSequenceNumber;
            
    PHY_PibGet(macPANId, (uint8_t *)&panid);
    param.panid = (uint32_t)panid;
    
    PHY_PibGet(macShortAddress, (uint8_t *)&macshortaddr);
    param.mac_short_addr = (uint32_t)macshortaddr;
    
    PHY_PibGet(macIeeeAddress, (uint8_t *)&macieeeaddr);
    memcpy(&param.mac_ieee_addr, &macieeeaddr, sizeof(macieeeaddr));
//    param.mac_ieee_addr = macieeeaddr;
    
    uint8_t channelBeforeSleep;
    PHY_PibGet(phyCurrentChannel, &channelBeforeSleep);
    param.phy_current_channel = (uint32_t)channelBeforeSleep; 
        
    memcpy4ByteAligned(&mdsParam,&param,sizeof(mdsParam)); 
#if ((defined MAC_SECURITY_ZIP)  || (defined MAC_SECURITY_2006))   
    memcpy4ByteAligned(&macSecPibBackup, &macSecPib, sizeof(macSecPib) );
#endif
} 

static void MAC_WakeUpFromDeepSleep(void)
{   
    MAC_Ds_Param_t param1;
    uint8_t channelAfterSleep;
        
    memset (&param1, 0, sizeof(param1)); 
    memcpy4ByteAligned(&param1,&mdsParam,sizeof(mdsParam));
    
//    macPib.mac_CoordExtendedAddress = param1.mac_CoordExtAddr;
//    macieeeaddr = param1.mac_ieee_addr;   
    memcpy(&macieeeaddr, &param1.mac_ieee_addr, sizeof(param1.mac_ieee_addr));
    myParentShortAddress = (uint16_t)param1.mac_CoordShtAddr;
    macshortaddr = (uint16_t)param1.mac_short_addr;
//    macPib.mac_MaxFrameTotalWaitTime = param1.mac_max_frame_total_wait_time;
//    macPib.mac_ResponseWaitTime = param1.mac_response_wait_time;   
    meshCurrentState = (meshState_t)param1.mesh_state;      
    macRadioSleepState = (PHY_TrxStatus_t)param1.mac_radio_sleep_state;   
//    macPib.mac_AssociatedPANCoord = param1.mac_associated_PAN_coord;
//    macPib.mac_AutoRequest = param1.mac_auto_request;       
//    macPib.mac_BattLifeExtPeriods = param1.mac_batt_life_ext_periods;   
    channelAfterSleep = (uint8_t)param1.phy_current_channel;
    panid = (uint16_t)param1.panid;
    appStates = (AppState_t)param1.app_state;
    meshSequenceNumber = (uint8_t)param1.mac_dsn;
    
    MiApp_Set(CHANNEL, &channelAfterSleep);
    MiApp_Set(PANID, (uint8_t*)&panid);
    MiApp_Set(SHORT_ADDRESS, (uint8_t*)&macshortaddr);
//    PibValue_t pibValue;    
//    pibValue.pib_value_64bit = macieeeaddr;
//    PHY_PibSet(macIeeeAddress, &pibValue);
#if ((defined MAC_SECURITY_ZIP)  || (defined MAC_SECURITY_2006))   
    memcpy4ByteAligned(&macSecPib, &macSecPibBackup, sizeof(macSecPibBackup) );
#endif

}

/*
 * @brief MAC Wakeup Callback Function from application
 *
 */
void MAC_Wakeup(void)
{
    /* Retrieve MAC Parameters from Retention RAM after Deepsleep wakeup*/
    MAC_WakeUpFromDeepSleep();
}

#endif
/*****************************************************************************
*****************************************************************************/

/**
 * Init function of the WSNDemo application
 */
void Wsndemo_Init(void)
{
    trx_cca_mode_t trxCcaMode;
	bool invalidIEEEAddrFlag = false;
	uint64_t invalidIEEEAddr;
	PHY_Retval_t retVal = PHY_FAILURE;
    PibValue_t pibValue;

#if defined(ENABLE_NETWORK_FREEZER)
    MiApp_SubscribeReConnectionCallback((ReconnectionCallback_t)ReconnectionIndication );
#endif

	/* Initialize the Protocol */
	if (MiApp_ProtocolInit() == RECONNECTION_IN_PROGRESS)
	{
		appStates = APP_STATE_WAIT_FOR_RECONNECT_CALLBACK;
	}

	/* Check if a valid IEEE address is available.0x0000000000000000 and 0xFFFFFFFFFFFFFFFF is persumed to be invalid */
	/* Check if IEEE address is 0x0000000000000000 */
	memset((uint8_t *)&invalidIEEEAddr, 0x00, LONG_ADDR_LEN);
	if (0 == memcmp((uint8_t *)&invalidIEEEAddr, (uint8_t *)&myLongAddress, LONG_ADDR_LEN))
	{
		invalidIEEEAddrFlag = true;
	}
//memset(&pibValue.pib_value_64bit, 0, MY_ADDRESS_LENGTH);
	/* Check if IEEE address is 0xFFFFFFFFFFFFFFFF */
	memset((uint8_t *)&invalidIEEEAddr, 0xFF, LONG_ADDR_LEN);
	if (0 == memcmp((uint8_t *)&invalidIEEEAddr, (uint8_t *)&myLongAddress, LONG_ADDR_LEN))
	{
		invalidIEEEAddrFlag = true;
	}
	
	if (invalidIEEEAddrFlag)
	{
		/*
			* In case no valid IEEE address is available, a random
			* IEEE address will be generated to be able to run the
			* applications for demonstration purposes.
			* In production code this can be omitted.
			*/
	uint64_t randomNumber;    
    if (PAL_SUCCESS != PAL_GetRandomNumber((uint8_t*)&randomNumber, sizeof(randomNumber))) {
		return;
	}    
    PHY_PibSet(macIeeeAddress,(PibValue_t *) &randomNumber);
	} 
    else
    {
    memcpy(&pibValue.pib_value_64bit, &myLongAddress, MY_ADDRESS_LENGTH);
    retVal = PHY_PibSet(macIeeeAddress, &pibValue);
    if(retVal != PHY_SUCCESS)
    {
        return;
    }        
    }
    trxCcaMode = TRX_CCA_MODE2;
    PHY_PibGet(macIeeeAddress, (uint8_t*)&pibValue);
    pibValue.pib_value_8bit = (uint8_t)trxCcaMode;
    retVal = PHY_PibSet(phyCCAMode, &pibValue);
    PHY_PibGet(phyCCAMode, (uint8_t*)&pibValue);

}

void Demomsg_Init(void)
{
    appMsg.commandId            = APP_COMMAND_ID_NETWORK_INFO;
	appMsg.nodeType             = APP_NODE_TYPE;
	appMsg.extAddr              = 0;
	appMsg.shortAddr            = 0;
	appMsg.softVersion          = 0x01100000;
	appMsg.channelMask          = CHANNEL_MAP;
	appMsg.nextHopAddr          = 0;
	appMsg.lqi                  = 0;
	appMsg.rssi                 = 0;

	appMsg.sensors.type        = 1U;
	appMsg.sensors.size        = (uint8_t)sizeof(int32_t) * 3U;
	appMsg.sensors.battery     = 0;
	appMsg.sensors.temperature = 0;
	appMsg.sensors.light       = 0;

	appMsg.caption.type         = 32U;
	appMsg.caption.size         = (uint8_t)APP_CAPTION_SIZE;
	memcpy(&appMsg.caption.text, APP_CAPTION, APP_CAPTION_SIZE);
}

void MiAppTimer_Init(void)
{
#if defined(COORDINATOR) || defined (ENDDEVICE)
    appDataSendingTimer.interval = APP_SENDING_INTERVAL;
	appDataSendingTimer.mode = SYS_TIME_SINGLE;
	appDataSendingTimer.handler = appDataSendingTimerHandler;
	appNetworkStatus = false;
	appNetworkStatusTimer.interval = APP_NWKSTATUS_INTERVAL;
	appNetworkStatusTimer.mode = SYS_TIME_PERIODIC;
	appNetworkStatusTimer.handler = appNetworkStatusTimerHandler;

#else
#if !defined(ENABLE_SLEEP_FEATURE)
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_On();
#else
	LED_On(1, LED_NETWORK);
#endif
#endif
#endif
#endif
#endif
}
#ifndef PAN_COORDINATOR
/**
 * Search confirmation
 */
void searchConfim(uint8_t foundScanResults, void* ScanResults)
{
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg;
	searchConf_t* searchConfRes = (searchConf_t *)ScanResults;
	uint8_t selectedParentIndex = 0xFFU;
	if (foundScanResults != 0U)
	{
		for (uint8_t loopindex = 0U; loopindex < foundScanResults; loopindex++)
		{
			if (searchConfRes->beaconList[loopindex].connectionPermit)
			{
#if defined(ENDDEVICE)
                /* Select the parent which has the high end device capacity (holding less number of end devices) */
				if (loopindex == 0U)
				{
					selectedParentIndex = 0U;
				}
#if (CAPABILITY_INFO == CAPABILITY_INFO_ED)
				else if (searchConfRes->beaconList[loopindex].sleepEnddeviceCapacity > searchConfRes->beaconList[selectedParentIndex].sleepEnddeviceCapacity){}
#elif (CAPABILITY_INFO == CAPABILITY_INFO_ED_RXON)
				else if (searchConfRes->beaconList[loopindex].enddeviceCapacity > searchConfRes->beaconList[selectedParentIndex].enddeviceCapacity)
                {
				    selectedParentIndex = loopindex;
				}
#endif
#else
				{
				    selectedParentIndex = loopindex;
				}
#endif
			}
		}
		
		if (selectedParentIndex != 0xFFU)
		{
			MiApp_EstablishConnection(searchConfRes->beaconList[selectedParentIndex].logicalChannel,
			SHORT_ADDR_LEN, (uint8_t*)&searchConfRes->beaconList[selectedParentIndex].shortAddress, CAPABILITY_INFO, Connection_Confirm);
			return;
		}
		/* Initiate the search again since no connection permit found to join */
        appStates = APP_STATE_CONNECT_NETWORK;
		appState->msgId = (uint8_t)APP_STATE_CONNECT_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
	}
	else
	{
		/* Initiate the search again since no beacon */
        appStates = APP_STATE_CONNECT_NETWORK;
		appState->msgId = (uint8_t)APP_STATE_CONNECT_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
	}
}
#endif

/**
 * Connection confirmation
 */
static void Connection_Confirm(miwi_status_t status)
{
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg;
	if (SUCCESS == status)
	{
        appStates = APP_STATE_SEND;
        appState->msgId = (uint8_t)APP_STATE_SEND;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
	}
	else
	{
#if defined(PAN_COORDINATOR)
        appStates = APP_STATE_START_NETWORK;
		appState->msgId = (uint8_t)APP_STATE_START_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
#else
        appStates = APP_STATE_CONNECT_NETWORK;
        appState->msgId = (uint8_t)APP_STATE_CONNECT_NETWORK;
        OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
#endif
	}

}

/**
 * Task of the WSNDemo application
 * This task should be called in a while(1)
 */
void wsndemo_task(void)
{
	MeshTasks();
}

#ifndef PAN_COORDINATOR
void appLinkFailureCallback(void)
{
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg;
    SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
    SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
	SYS_TIME_TimerStop(appDataSendingTimerHandle);
    SYS_TIME_TimerDestroy(appDataSendingTimerHandle);
    SYS_TIME_TimerStop(appNetworkStatusTimerHandle);
    SYS_TIME_TimerDestroy(appNetworkStatusTimerHandle);
    	/* On link failure initiate search to establish connection */
    appStates = APP_STATE_CONNECT_NETWORK;
	appState->msgId = (uint8_t)APP_STATE_CONNECT_NETWORK;
    OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
}
#endif

void MiMesh_TaskHandler(void)
{
    wsndemo_task();
}

void MiMac_TaskHandler(void)
{
#ifdef PROTOCOL_MESH
MiMesh_TaskHandler();
#endif
#ifdef PROTOCOL_STAR
Star_TaskHandler();
#endif
#ifdef PROTOCOL_P2P
P2P_TaskHandler();
#endif    
}

miwi_status_t PhyToMiwi_Status(PHY_Retval_t status)
{
    miwi_status_t dataStat = FAILURE;
    switch(status)
    {
        case PHY_SUCCESS:
        {
            dataStat = SUCCESS;
            break;
        }
        case PHY_FAILURE:
        case PHY_BUSY:
        {
            dataStat = FAILURE;
            break;
        }
        case PHY_CHANNEL_BUSY:
        case PHY_CHANNEL_ACCESS_FAILURE:
        {
            dataStat = CHANNEL_ACCESS_FAILURE;
            break;
        }
#ifdef CHIMERA_SOC
        case PHY_RF_REQ_ABORTED:
        case PHY_RF_UNAVAILABLE:
        {
            dataStat = ERR_TRX_FAIL;
            break;
        }
#endif
        case PHY_NO_ACK:
        {
            dataStat = NO_ACK;
            break;
        }
        case PHY_INVALID_PARAMETER:
        case PHY_UNSUPPORTED_ATTRIBUTE:
        {
            dataStat = ERR_INVALID_INPUT;
            break;
        }
        case PHY_TRX_ASLEEP:
        case PHY_TRX_AWAKE:
        case PHY_FRAME_PENDING:
        case PHY_CHANNEL_IDLE:
        {
            dataStat = FAILURE;
            break;
        }
        default:
        {
            //Handle exceptions if any
            break;
        }
    }
    return dataStat;
}

void MiMAC_FFDDemoInit(void)
{    
    /* If Restored properly, valid short address will be available */
    if (myShortAddress != 0xFFFFU)
    {
#ifndef ENDDEVICE
        /* Change state since restoration is complete and all required network information available */
        meshCurrentState = IN_NETWORK_STATE;
#ifndef PAN_COORDINATOR
        /* Update Parent short address */
        if (myShortAddress & RXON_ENDEVICE_ADDRESS_MASK)
        {
            myParentShortAddress = myShortAddress & COORD_MASK;

            /* Change state to partial since it is working as RX ON End device */
            meshCurrentState = IN_NETWORK_PARTIAL_STATE;

            /* Keep alive timer - load with rxon end device since it got address as rxonED*/
            timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout = generateJitterTimeout(miwiDefaultRomOrRamParams->keepAliveRxOnEdSendInterval * 1000, 5);
        }
        else
        {
            myParentShortAddress = PAN_COORDINATOR_ADDRESS;
            /* load Keep alive timer */
            timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout = generateJitterTimeout(miwiDefaultRomOrRamParams->keepAliveCoordSendInterval * 1000, 5);
        }

        /* Start the keep alive timer upon successful reconnection */
        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval = timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout;
        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.handler = sendKeepAlive;
        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.mode = SYS_TIME_PERIODIC;
        uint8_t myData = 0U;
        timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle = SYS_TIME_CallbackRegisterMS(timerHandles.keepAliveTimerSendKeepAliveRxOnEd.handler, (uintptr_t)&myData, timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval, SYS_TIME_PERIODIC);
        if(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;	
        }
#endif
#if defined(ENABLE_NETWORK_FREEZER)
       /* Inform application reconnection success */
       if (NULL != reconnectionCallback)
         reconnectionCallback(SUCCESS);
#endif

#if defined(ENABLE_FREQUENCY_AGILITY)
        if (newChannelToUpdate != 0xFFU)
        {
            /* Start the timer for channel update time */
            timerHandles.joinTimerchannelUpdate.handler = channelUpdateTimerExpired;
            timerHandles.joinTimerchannelUpdate.timeout = (CHANNEL_UPDATE_TIME_IN_SEC * 1000U);
            timerHandles.joinTimerchannelUpdate.interval = (CHANNEL_UPDATE_TIME_IN_SEC * 1000U); //michp
            timerHandles.joinTimerchannelUpdate.mode = SYS_TIME_SINGLE;            
            uint8_t myData = 0U;
            timerHandles.joinTimerchannelUpdateHandle = SYS_TIME_CallbackRegisterMS(timerHandles.joinTimerchannelUpdate.handler, (uintptr_t)&myData, timerHandles.joinTimerchannelUpdate.interval, SYS_TIME_SINGLE);
            if(timerHandles.joinTimerchannelUpdateHandle == SYS_TIME_HANDLE_INVALID)
            {
             return;	
            }
        
        }
#endif
#endif
    }
}
#ifdef ENDDEVICE
void MiMAC_RFDDemoInit(void)
{ 
    
#ifdef ENABLE_SLEEP_FEATURE  
    DEVICE_DeepSleepWakeSrc_T wakeupSrc;
    DEVICE_GetDeepSleepWakeUpSrc(&wakeupSrc);
   
    if(wakeupSrc == DEVICE_DEEP_SLEEP_WAKE_NONE  ||  wakeupSrc == DEVICE_DEEP_SLEEP_WAKE_MCLR)
#endif
    {

#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Toggle();
    RGB_LED_BLUE_Off();
    RGB_LED_RED_Off();
#else
    LED_On(1,LED_IDENTIFY);     /* indicating application is started */
    LED_Off(1,LED_IDENTIFY); /* indicating network is started */
    LED_Off(1,LED_DATA);     /* indicating data transmission */
#endif
#endif
#endif
//        SYS_CONSOLE_PRINT("\nMAC RFD Demo Application\r\n\n");    
    /* If Restored properly, valid short address will be available */
    if (myShortAddress != 0xFFFFU)
    {
        /* Incase of End device, try to rejoin using establish connection procedure */
        uint16_t parentNetworkAddress = 0xFFFF;
        parentNetworkAddress = myShortAddress & COORD_MASK;
#if defined(ENABLE_FREQUENCY_AGILITY)
		backupParentNwkAddress =  parentNetworkAddress;
#endif
		initStatus = RECONNECTION_IN_PROGRESS;
        MiApp_EstablishConnection(currentChannel, SHORT_ADDR_LEN, (uint8_t *)&parentNetworkAddress, gCapabilityInfo, connectionConfirm);
    }

    }
#ifdef ENABLE_SLEEP_FEATURE 
    else
    {
        MAC_Wakeup();
        app_initiate_polling(NULL);
    }
#endif
}


#ifdef ENABLE_SLEEP_FEATURE 
static void app_initiate_polling(void *parameter)
{
	/*
     * initiate data polling
	 */
    uint8_t myData = 0U;
    APP_Msg_T *p_appModes;
    APP_Msg_T appModes;
    p_appModes = &appModes;
	/*Keep compiler happy*/
	parameter = parameter;
    deviceCanSleep = false;
/* Poll immediately after join- small timer for context break */
timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
timerHandles.keepAliveTimerRxOffEd.timeout = 50;
timerHandles.keepAliveTimerRxOffEd.interval = 50;
timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
{
    return;
}
#if defined(ENABLE_NETWORK_FREEZER)
    /* Indicate application with status of reconnection */
    if (NULL != reconnectionCallback)
    {
        reconnectionCallback(SUCCESS);
    }
#endif
}
#endif
#endif

void Rx_On(bool isProm)
{
PHY_TrxStatus_t trxStatus;
PHY_TrxState_t trxState = PHY_STATE_RX_ON;
//bool promCtrl = true;
trxStatus = PHY_RxEnable(trxState);
if(PHY_RX_ON == trxStatus)
{
    //TRX is in receive state
    
}


// Enable Promiscuous mode
PHY_ConfigRxPromiscuousMode(isProm);

// To get the PDT level configured
PHY_GetTrxConfig(AACK_PROMSCS_MODE, (uint8_t*)&isProm);
}

uint8_t MiApp_TransceiverPowerState(uint8_t Mode)
{
    if(MiMAC_PowerState(Mode))
    {
        return 1U;
    }
    else
    {
        return 0U;
    }
}