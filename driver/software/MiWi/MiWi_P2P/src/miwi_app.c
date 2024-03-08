/**
* \file  task.c
*
* \brief Implementation of Tasks for Demo Application on MiWi P2P
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

/***********************Headers***************************************/
#include <string.h>
#include <stdio.h>
#include "config/default/definitions.h"
#include "miwi_app.h"
#include "miwi_api.h"
#include "p2p_demo.h"
#include "config/default/driver/IEEE_802154_PHY/pal/inc/pal.h"

#if defined(ENABLE_SLEEP_FEATURE)
#include "config/default/device_deep_sleep.h"
#endif
/************************** VARIABLES ************************************/
#define LIGHT   0x01
#define SWITCH  0x02

/*************************************************************************/
// AdditionalNodeID variable array defines the additional
// information to identify a device on a PAN. This array
// will be transmitted when initiate the connection between
// the two devices. This  variable array will be stored in
// the Connection Entry structure of the partner device. The
// size of this array is ADDITIONAL_NODE_ID_SIZE, defined in
// miwi_config.h.
// In this demo, this variable array is set to be empty.
/*************************************************************************/
#if ADDITIONAL_NODE_ID_SIZE > 0
    uint8_t AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE] = {LIGHT};
#endif
bool deviceCanSleep = false;
AppState_t appStates;
/* Connection Table Memory */
CONNECTION_ENTRY connectionTable[CONNECTION_SIZE];
#ifdef ENABLE_ACTIVE_SCAN
/* Active Scan Results Table Memory */
ACTIVE_SCAN_RESULT activeScanResults[ACTIVE_SCAN_RESULT_SIZE];
#endif

defaultParametersRomOrRam_t defaultParamsRomOrRam = {
    .ConnectionTable = &connectionTable[0],
#ifdef ENABLE_ACTIVE_SCAN
    .ActiveScanResults = &activeScanResults[0],
#endif
#if ADDITIONAL_NODE_ID_SIZE > 0
    .AdditionalNodeID = &AdditionalNodeID[0],
#endif
    .networkFreezerRestore = 0,
};

defaultParametersRamOnly_t defaultParamsRamOnly = {
    .dummy = 0,
};
extern API_UINT16_UNION  myPANID;
#ifdef ENABLE_SLEEP_FEATURE
static void MiWi_ReadyToDeepSleep(void);
static void app_initiate_polling(void *parameter);
static void MiWi_WakeUpFromDeepSleep(void);
#endif
/*************************************************************************/
// The variable myChannel defines the channel that the device
// is operate on. This variable will be only effective if energy scan
// (ENABLE_ED_SCAN) is not turned on. Once the energy scan is turned
// on, the operating channel will be one of the channels available with
// least amount of energy (or noise).
/*************************************************************************/
#if defined(CHIMERA_SOC)
uint8_t myChannel = 26U;
/* Range: 11 to 26 */
#elif defined(PHY_AT86RF212B)
uint8_t myChannel = 8U;
#elif defined(PHY_AT86RF233)
uint8_t myChannel = 26U;
/* Range for default configuration: 1 to 10
 Note: TX Power and PHY Mode Setting needs to be modified as per the
 recommendation from Data Sheet for European band (ie.,Channel 0)*/
#endif

/*********************************************************************
* Function: bool freezer_feature(void)
*
* Overview: Allows user to select network freezer restore
*
* Return:  true if network freezer to be used for restoring
********************************************************************/
bool freezer_feature(void)
{
//    uint8_t switch_val;
//        switch_val = ButtonPressed ();
        uint8_t switch_val = 1U;
        if(switch_val == 1U)
        {
#if defined (ENABLE_CONSOLE)
            printf("\r\nRestoring Network !!\r\n");
            PAL_TimerDelay(1000U);
#endif
            return true;
        }
        else
        {
            return false;
        }

    return false;
}

/*********************************************************************
* Function: static void longAddressValidationAndUpdation(void)
*
* Overview: validates the long address and assigns new address
            by random allocation if invalid address found
********************************************************************/
static void longAddressValidationAndUpdation(void)
{
    bool invalidIEEEAddrFlag = false;
    uint64_t invalidIEEEAddr;
	PHY_Retval_t retVal = PHY_FAILURE;
    PibValue_t pibValue;
//    srand(PHY_RandomReq());

    /* Check if a valid IEEE address is available.
    0x0000000000000000 and 0xFFFFFFFFFFFFFFFF is persumed to be invalid */
    /* Check if IEEE address is 0x0000000000000000 */
    memset((uint8_t *)&invalidIEEEAddr, 0x00, LONG_ADDR_LEN);
    if (0 == memcmp((uint8_t *)&invalidIEEEAddr, (uint8_t *)&myLongAddress, LONG_ADDR_LEN))
    {
        invalidIEEEAddrFlag = true;
    }

    /* Check if IEEE address is 0xFFFFFFFFFFFFFFFF */
    memset((uint8_t *)&invalidIEEEAddr, 0xFF, LONG_ADDR_LEN);
    if (0 == memcmp((uint8_t *)&invalidIEEEAddr, (uint8_t *)&myLongAddress, LONG_ADDR_LEN))
    {
        invalidIEEEAddrFlag = true;
    }

    if (invalidIEEEAddrFlag)
    {
         /* In case no valid IEEE address is available, a random
          * IEEE address will be generated to be able to run the
          * applications for demonstration purposes.
          * In production code this can be omitted.
         */
        uint8_t* peui64 = (uint8_t *)&myLongAddress;
        for(uint8_t i = 0; i < MY_ADDRESS_LENGTH; i++)
        {
            *peui64++ = (uint8_t)rand();
        }
    }
    /* Set the address in transceiver */
    memcpy((uint8_t*)&pibValue.pib_value_64bit, &myLongAddress, MY_ADDRESS_LENGTH);//misra 21.15
    retVal = PHY_PibSet(macIeeeAddress, &pibValue);
    if(retVal != PHY_SUCCESS)
    {
        return;
    }  
}

bool startNetwork = false;
/*********************************************************************
* Function: static void Connection_Confirm(miwi_status_t status)
*
* Overview: callback function called upon MiAPP_StarConnection
*           or MiApp_EstablishConnection procedure completes
* Parameter: status of the completed operation
********************************************************************/
static void Connection_Confirm(miwi_status_t status)
{
    /* If success or already exists status, update the LED,CONSOLE,LCD
       and show the demo instruction for the user to proceed */
    if ((SUCCESS == status) || (ALREADY_EXISTS == status))
    {

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

        if (!startNetwork)
        {
            DemoOutput_Channel(myChannel, 1);
        }
#if defined(ENABLE_CONSOLE)
        else
        {
            printf("\r\nStarted Wireless Communication on Channel ");
            printf("%u",currentChannel);
            printf("\r\n");
        }
        DemoOutput_Instruction();
        DumpConnection(0xFF);
#endif
    }
    else
    {
        /* Upon EstablishConnection failure, initiate the startConnection to form a network */
        startNetwork = true;
        MiApp_StartConnection(START_CONN_DIRECT, 10, (1L << myChannel), Connection_Confirm);
    }
}

/*********************************************************************
* Function: bool Initialize_Demo(bool freezer_enable)
*
* Overview: Initializes the demo by initializing protocol, required
            components and initiates connection
********************************************************************/
bool Initialize_Demo(bool freezer_enable)
{
    uint16_t broadcastAddr = 0xFFFFU;
    /* Subscribe for data indication */
    MiApp_SubscribeDataIndicationCallback(ReceivedDataIndication);

#ifdef ENABLE_SLEEP_FEATURE
    /* Sleep manager initialization */
    sleepMgr_init();
#endif

    /* Update NetworkFreezerRestore parameter whether to restore from network freezer or not */
    defaultParamsRomOrRam.networkFreezerRestore = freezer_enable;

    /* Initialize the P2P and Star Protocol */
    if (MiApp_ProtocolInit(&defaultParamsRomOrRam, &defaultParamsRamOnly) == RECONNECTED)
    {
        printf("\r\nPANID:");
        printf("%x",myPANID.v[1]);
        printf("%x",myPANID.v[0]);
        printf(" Channel:");
        printf("%d",currentChannel);
#if defined(PROTOCOL_P2P)
        DemoOutput_Instruction();
#else
        STAR_DEMO_OPTIONS_MESSAGE (role);
#endif
        return true;
    }
    /* Unable to boot from the Network Freezer parameters, so initiate connection */
    /* Check Valid address is found , else update with random */
    longAddressValidationAndUpdation();

    /* Enable all kinds of connection */
    MiApp_ConnectionMode(ENABLE_ALL_CONN);

    // Set default channel
    if( MiApp_Set(CHANNEL, &myChannel) == false )
    {
        DemoOutput_ChannelError(myChannel);
        return false;
    }

    DemoOutput_Channel(myChannel, 0U);

    startNetwork =  false;

    /* Try to establish a new connection with peer device by broadcast Connection Request */
    return MiApp_EstablishConnection(myChannel, 2U, (uint8_t*)&broadcastAddr, 0U, Connection_Confirm);
}

/*********************************************************************
* Function: void Run_Demo(void)
*
* Overview: runs the demo based on user input
********************************************************************/
void Run_Demo(void)
{
    P2PTasks();
    run_p2p_demo();
}
#ifdef PROTOCOL_P2P
void MiP2P_TaskHandler(void)
{
    P2PTasks();
}
#endif
void MiMac_TaskHandler(void)
{
#ifdef PROTOCOL_MESH
MiMesh_TaskHandler();
#endif
#ifdef PROTOCOL_STAR
Star_TaskHandler();
#endif
#ifdef PROTOCOL_P2P
MiP2P_TaskHandler();
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

#ifdef ENABLE_CONSOLE
#ifdef ENABLE_DUMP
/*********************************************************************
    * void DumpConnection(uint8_t index)
    *
    * Overview:        This function prints out the content of the connection 
    *                  with the input index of the P2P Connection Entry
    *
    * PreCondition:    
    *
    * Input:  
    *          index   - The index of the P2P Connection Entry to be printed out
    *                  
    * Output:  None
    *
    * Side Effects:    The content of the connection pointed by the index 
    *                  of the P2P Connection Entry will be printed out
    *
    ********************************************************************/
void DumpConnection(uint8_t index)
{
    uint8_t i, j;
        
    if( index > CONNECTION_SIZE )
    {
        printf("\r\n\r\nMy Address: 0x");
        for(i = 0; i < MY_ADDRESS_LENGTH; i++)
        {
            printf("%02x",myLongAddress[MY_ADDRESS_LENGTH-1-i]);
        }
        #if defined(IEEE_802_15_4)
            printf("  PANID: 0x");
            printf("%x",myPANID.v[1]);
            printf("%x",myPANID.v[0]);
        #endif
        printf("  Channel: ");
        printf("%d",currentChannel);
    }
            
    if( index < CONNECTION_SIZE )
    {
        printf("\r\nConnection \tPeerLongAddress \tPeerInfo\r\n");  
        if( connectionTable[index].status.bits.isValid )
        {
            printf("%02x",index);
            printf("\t\t\t");
            for(i = 0; i < 8; i++)
            {
                if(i < MY_ADDRESS_LENGTH)
                {
                    printf("%02x", connectionTable[index].Address[MY_ADDRESS_LENGTH-1-i] );
                }
                else
                {
                    printf("\t");
                }
            }
            printf("/t");
            #if ADDITIONAL_NODE_ID_SIZE > 0
                for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++)
                {
                    printf("%02x", connectionTable[index].PeerInfo[i] );
                }
            #endif
            printf("\r\n");
        }
    }
    else
    {
        printf("\r\n\r\nConnection     PeerLongAddress     PeerInfo\r\n");  
        for(i = 0; i < CONNECTION_SIZE; i++)
        {
                
            if( connectionTable[i].status.bits.isValid )
            {
                printf("%02x",i);
                printf("             ");
                for(j = 0; j < 8; j++)
                {
                    if( j < MY_ADDRESS_LENGTH )
                    {
                        printf("%02x", connectionTable[i].Address[MY_ADDRESS_LENGTH-1-j] );
                    }
                    else
                    {
                        printf("  ");
                    }
                }
                printf("    ");
#if ADDITIONAL_NODE_ID_SIZE > 0
                    for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++)
                    {
                        printf("%02x", connectionTable[i].PeerInfo[j] );
                    }
#endif
                printf("\r\n");
            }  
        }
    }
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
}

#ifdef ENABLE_SLEEP_FEATURE
bool APP_ReadyToSleep(uint32_t *sleepDuration)
{
    bool sleep = false;
    if(deviceCanSleep)
    {
        if(MiWi_ReadyToSleep())
        {           
            *sleepDuration = APP_POLL_PERIOD_MS;
            sleep = true;                         
        }
    }    
    return sleep;
}

uint32_t MiWi_ReadyToSleep(void)
{
	uint32_t sleepTime = 0uL;
    MiMAC_PowerState(POWER_STATE_DEEP_SLEEP);
    PHY_TrxStatus_t trxStatus = PHY_GetTrxStatus();
	if ((busyLock != 0U) || (frameTxQueue.size != 0U) || (frameRxQueue.size != 0U) || (trxStatus != PHY_TRX_SLEEP))
    {
//        SYS_CONSOLE_PRINT("\r\n no sleep\r\n");
		sleepTime = 0uL;
	} 
    else 
    {
        
		sleepTime = READY_TO_SLEEP;
#ifdef ENABLE_SLEEP_FEATURE
        MiWi_ReadyToDeepSleep();
#endif
	}
	return sleepTime;
}

typedef __PACKED_STRUCT miwip2pstar_ds_param
{
    uint64_t miwi_parentExtAddr;  
    uint64_t miwi_ieee_addr; 
    uint32_t miwi_parentShtAddr; 
    uint32_t miwi_short_addr;
    uint32_t panid;
    uint32_t connMode;
    uint32_t conn_size;
    uint32_t role;
    uint32_t myConnectionIndex_in_PanCo;
    uint32_t miwi_max_frame_total_wait_time;
    uint32_t miwi_response_wait_time; 
    uint32_t p2pStarCurrentState;  
    uint32_t miwi_radio_sleep_state;
    uint32_t app_state;
    uint32_t miwi_poll_state; 
    uint32_t miwi_associated_PAN_coord;   
    uint32_t miwi_auto_request; 
    uint32_t miwi_batt_life_ext_periods; 
    uint32_t miwi_dsn; 
    uint32_t phy_current_channel;
}MIWIP2PStar_Ds_Param_t;



#if ((defined MAC_SECURITY_ZIP)  || (defined MAC_SECURITY_2006))
MIWI_SecPib_t __attribute__ ((persistent)) miwiSecPibBackup;
#endif

MIWIP2PStar_Ds_Param_t __attribute__((persistent)) mdsParam;

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

static void MiWi_ReadyToDeepSleep(void)
{  
//    SYS_CONSOLE_PRINT("\r\n sleep rdy\r\n");
    MIWIP2PStar_Ds_Param_t param; 
    uint16_t panid;
    uint16_t    miwishortaddr; 
    PHY_TrxStatus_t miwiRadioSleepState;
    uint64_t    miwiieeeaddr;
    memcpy(&param.miwi_parentExtAddr, &miwiDefaultRomOrRamParams->ConnectionTable[0].Address, sizeof(miwiDefaultRomOrRamParams->ConnectionTable[0].Address));  
    param.p2pStarCurrentState = (uint32_t)p2pStarCurrentState;    
    param.app_state = (uint32_t)appStates;
    miwiRadioSleepState = PHY_GetTrxStatus();
    param.miwi_radio_sleep_state = (uint32_t)miwiRadioSleepState;    
    param.miwi_dsn = (uint32_t)p2pStarSeqNum;
    param.connMode = (uint32_t)ConnMode;
    param.conn_size = (uint32_t)conn_size;
    param.role = (uint32_t)role;
            
    PHY_PibGet(macPANId, (uint8_t *)&panid);
    param.panid = (uint32_t)panid;
    
    PHY_PibGet(macShortAddress, (uint8_t *)&miwishortaddr);
    param.miwi_short_addr = (uint32_t)miwishortaddr;
    
    PHY_PibGet(macIeeeAddress, (uint8_t *)&miwiieeeaddr);
    memcpy(&param.miwi_ieee_addr, &myLongAddress, sizeof(myLongAddress));
    
    uint8_t channelBeforeSleep;
    PHY_PibGet(phyCurrentChannel, &channelBeforeSleep);
    param.phy_current_channel = (uint32_t)channelBeforeSleep; 
        
    memcpy4ByteAligned(&mdsParam,&param,(uint16_t)sizeof(mdsParam)); 
#if ((defined MIWI_SECURITY_ZIP)  || (defined MIWI_SECURITY_2006))   
    memcpy4ByteAligned(&miwiSecPibBackup, &miwiSecPib, sizeof(miwiSecPib) );
#endif
} 

static void MiWi_WakeUpFromDeepSleep(void)
{   
    MIWIP2PStar_Ds_Param_t param1;
    uint8_t channelAfterSleep = 0U;
    uint16_t panid = 0x0000U;
    uint64_t    miwiieeeaddr;
    PHY_TrxStatus_t miwiRadioSleepState;
    uint16_t    miwishortaddr; 
    memset (&param1, 0, sizeof(param1)); 
    memcpy4ByteAligned(&param1,&mdsParam,(uint16_t)sizeof(mdsParam)); 
    memcpy(&miwiieeeaddr, &param1.miwi_ieee_addr, sizeof(param1.miwi_ieee_addr));
//    myParentExtAddress = (uint16_t)param1.miwi_parentExtAddr;
    memcpy(&miwiDefaultRomOrRamParams->ConnectionTable[0].Address, &param1.miwi_parentExtAddr, sizeof(param1.miwi_parentExtAddr));  
    p2pStarCurrentState = (p2pStarState_t)param1.p2pStarCurrentState;      
    miwiRadioSleepState = (PHY_TrxStatus_t)param1.miwi_radio_sleep_state;   
    panid = (uint16_t)param1.panid;
    appStates = (AppState_t)param1.app_state;
    p2pStarSeqNum = (uint8_t)param1.miwi_dsn;
    channelAfterSleep = (uint8_t)param1.phy_current_channel;
    miwishortaddr = (uint16_t)param1.miwi_short_addr;
    MiApp_Set(CHANNEL, &channelAfterSleep);
    MiMAC_SetPanId((uint8_t*)&panid);
    MiMAC_SetAltAddress((uint8_t*)&miwishortaddr);
    ConnMode = (uint8_t)param1.connMode;
    conn_size = (uint8_t)param1.conn_size;
    role = (DeviceRole_t)param1.role;
#if ((defined MAC_SECURITY_ZIP)  || (defined MAC_SECURITY_2006))   
    memcpy4ByteAligned(&macSecPib, &miwiSecPibBackup, sizeof(miwiSecPibBackup) );
#endif

}

/*
 * @brief MAC Wakeup Callback Function from application
 *
 */
static void MiWi_Wakeup(void)
{
    /* Retrieve MAC Parameters from Retention RAM after Deepsleep wakeup*/
    MiWi_WakeUpFromDeepSleep();
}
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
//    if (myShortAddress != 0xFFFFU)
//    {
//        /* Incase of End device, try to rejoin using establish connection procedure */
//        uint16_t parentNetworkAddress = 0xFFFF;
//        parentNetworkAddress = myShortAddress & COORD_MASK;
//#if defined(ENABLE_FREQUENCY_AGILITY)
//		backupParentNwkAddress =  parentNetworkAddress;
//#endif
//		initStatus = RECONNECTION_IN_PROGRESS;
//        MiApp_EstablishConnection(currentChannel, SHORT_ADDR_LEN, (uint8_t *)&parentNetworkAddress, gCapabilityInfo, connectionConfirm);
//    }

    }
#ifdef ENABLE_SLEEP_FEATURE 
    else
    {
        MiWi_Wakeup();
        app_initiate_polling(NULL);
    }
#endif
}

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
//timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
//timerHandles.keepAliveTimerRxOffEd.timeout = 50;
//timerHandles.keepAliveTimerRxOffEd.interval = 50;
//timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
//timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
//if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
//{
//    return;
//}
//    /* Indicate application with status of reconnection */
//    if (NULL != reconnectionCallback)
//    {
//        reconnectionCallback(SUCCESS);
//    }
}
#endif