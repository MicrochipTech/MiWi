/**
* \file  p2p_demo.c
*
* \brief Demo Application for MiWi P2P Implementation
*
* Copyright (c) 2023 - 2024 Microchip Technology Inc. and its subsidiaries.
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

/************************ HEADERS ****************************************/
#include "config/default/definitions.h"
#include "miwi_api.h"
#include "miwi_p2p_star.h"
#include "p2p_demo.h"
#if defined(USER_BUTTON_ENABLED)
#include "config/default/peripheral/eic/plib_eic.h"
#endif
#if defined(ENABLE_SLEEP_FEATURE)
#include "config/default/device_deep_sleep.h"
#endif

#if defined(ENABLE_NETWORK_FREEZER)
#include "MiWi/MiWi_P2P/Services/inc/wlPdsMemIds.h"
#endif

#if defined(PROTOCOL_P2P)
/************************ LOCAL VARIABLES ****************************************/
uint8_t i;
uint8_t TxSynCount = 0;
uint8_t TxSynCount2 = 0;
uint8_t TxNum = 0;
uint8_t RxNum = 0;
bool chk_sel_status = true;
uint8_t NumOfActiveScanResponse;
bool update_ed;
uint8_t select_ed;
uint8_t msghandledemo = 0;
/* Connection Table Memory */
extern CONNECTION_ENTRY connectionTable[CONNECTION_SIZE];
#if defined(ENABLE_FREQUENCY_AGILITY)
uint8_t maxRSSI;
uint8_t channelCount = 0U;
static uint8_t minRSSI = 0xFFU;
static uint8_t gOptimalChannel = 0xFFU;
#endif

/************************ FUNCTION DEFINITIONS ****************************************/
/*********************************************************************
* Function: static void dataConfcb(uint8_t handle, miwi_status_t status)
*
* Overview: Confirmation Callback for MiApp_SendData
*
* Parameters:  handle - message handle, miwi_status_t status of data send
****************************************************************************/
static void dataConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
    if (SUCCESS == status)
    {
        /* Update the TX NUM and Display it on the LCD */
        DemoOutput_UpdateTxRx(++TxNum, RxNum);
        /* Delay for Display */
//        delay_ms(100);
    }
    /* After Displaying TX and RX Counts , Switch back to showing Demo Instructions */
    DemoOutput_Instruction ();
#if defined(ENABLE_SLEEP_FEATURE)
    deviceCanSleep = true;
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg; 
    appState->msgId = APP_STATE_PREPARE_TO_SLEEP;
    OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
#endif
}

/*********************************************************************
* Function: void run_p2p_demo(void)
*
* Overview: runs the demo based on input
*
* Parameters: None
*********************************************************************/
void MiAPP_TaskHandler(APP_Msg_T *appState)
{
        /*******************************************************************/
        // If no packet received, now we can check if we want to send out
        // any information.
        // Function ButtonPressed will return if any of the two buttons
        // has been pushed.
        /*******************************************************************/

        switch( appState->msgId )
        {
            case APP_STATE_BROADCAST_DATA:
            {
                /*******************************************************************/
                // Button 1 pressed. We need to send out the bitmap of word "MiWi".
                /*******************************************************************/
                uint16_t broadcastAddress = 0xFFFFU;
                bool mac_ack_status;

                /* Function MiApp_SendData is used to broadcast a message with address as 0xFFFF */
#ifdef ENABLE_SECURITY
                mac_ack_status = MiApp_SendData(SHORT_ADDR_LEN, (uint8_t *)&broadcastAddress, MIWI_TEXT_LEN, (uint8_t *)&MiWi[(TxSynCount%6)][0], msghandledemo++, true, true, dataConfcb);
#else
				mac_ack_status = MiApp_SendData(SHORT_ADDR_LEN, (uint8_t *)&broadcastAddress, MIWI_TEXT_LEN, (uint8_t *)&MiWi[(TxSynCount%6)][0], msghandledemo++, true, false, dataConfcb);
#endif                  
                if (mac_ack_status)
                {
                    /* Update the bitmap count */
                    TxSynCount++;
                }
            }
            break;

#if !defined (CONF_BOARD_JOYSTICK)
            case APP_STATE_UNICAST_DATA:
            {
                select_ed = 0U;

#ifdef ENABLE_SECURITY
                            if( MiApp_SendData(LONG_ADDR_LEN, connectionTable[select_ed].Address, DE_LEN, (uint8_t*)&DE[(TxSynCount2%6)][i], msghandledemo++, 1, true, dataConfcb) == false)
#else
							if( MiApp_SendData(LONG_ADDR_LEN, connectionTable[select_ed].Address, DE_LEN, (uint8_t*)&DE[(TxSynCount2%6)][i], msghandledemo++, 1, false, dataConfcb) == false)
#endif 
                {
                    DemoOutput_UnicastFail();
                }
                else
                {
                    // Successful Transmission
                    TxSynCount2++;
                }

            } // End of Display

                break;
#endif

            case APP_STATE_NAVIGATE_INDEX_CONNTABLE:
            {
                if (select_ed > conn_size-2U)  /* Reset Peer Device Info */
                {
                        /* If end of Peer Device Info reset the count */
                    select_ed = 0U;
                }
                else
                {
                    /* New device Information */
                    select_ed = select_ed+1U;
                }
            }
            break;
        

#if defined(ENABLE_SLEEP_FEATURE)
	    case (uint8_t)APP_STATE_PREPARE_TO_SLEEP:
	    {
//             SYS_CONSOLE_PRINT("\r\n sleep0\r\n");
            uint32_t sleepDuration = 0;
        if (Total_Connections() > 0U)
        {
            if(MiApp_ReadyToSleep(&sleepDuration))
//            if (APP_ReadyToSleep(&sleepDuration))
            {    
//                PAL_TimerDelay(1000);
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
        }
        }
        	break;
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
        // appStates = APP_STATE_NOISE_DETECTION;
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
            {
                //handle exceptions if any
            }
            break;
        }
}

/*********************************************************************
* Function: void ReceivedDataIndication (RECEIVED_MESSAGE *ind)
*
* Overview: Process a Received Message
*
* PreCondition: MiApp_ProtocolInit
*
* Input:  RECEIVED_MESSAGE *ind - Indication structure
********************************************************************/
void ReceivedDataIndication (RECEIVED_MESSAGE *ind)
{
#if defined(ENABLE_CONSOLE)
    /* Print the received information via Console */
    DemoOutput_HandleMessage();
#endif

    /* Update the TX AND RX Counts on the display */
    DemoOutput_UpdateTxRx(TxNum, ++RxNum);

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

    /* Display the Instructions message */
    DemoOutput_Instruction();
#if defined(ENABLE_SLEEP_FEATURE)
    deviceCanSleep = true;
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg; 
    appState->msgId = APP_STATE_PREPARE_TO_SLEEP;
    OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
#endif
}

/*******************************************************************************
  Function:
    void InitializeApp()

******************************************************************************/
void MiApp_Init(void)
{
    bool freezer_enable = true;
    // Demo Start Message
    DemoOutput_Greeting();

    demo_output_freezer_options();
    // User Selection to commission a network or use Freezer
    //freezer_enable = freezer_feature();

    // Commission the network
    Initialize_Demo(freezer_enable);
    Rx_On(false);
    appInitialized = true;
#ifdef USER_BUTTON_ENABLED
    dummyVal = 0U;
#if defined(CHIMERA_SOC)
    EIC_CallbackRegister(EIC_PIN_0, eic_custom_cb, dummyVal);
#else
    EIC_CallbackRegister(EIC_PIN_2, eic_custom_cb, dummyVal);
#endif
#endif
}

#endif
#if defined(ENABLE_FREQUENCY_AGILITY)
void performFreqAgility(void)
{
    APP_Msg_T    appMsg;
    APP_Msg_T    *p_appmsg;
    p_appmsg = &appMsg;
    channelCount = 0U;
    maxRSSI = 0U;
    MiApp_Get(CHANNEL, &backupChannel);
    appStates = APP_STATE_NOISE_DETECTION;
    p_appmsg->msgId = APP_STATE_NOISE_DETECTION;
    OSAL_QUEUE_Send(&appData.appQueue, p_appmsg, 0UL);
}
#endif