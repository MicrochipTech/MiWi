/**
* \file  miwi_mesh.c
*
* \brief MiWi Mesh Protocol implementation
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

/************************ HEADERS **********************************/
#include "config/default/definitions.h"
/************************ Macro Definitions **********************************/
///* Number of octets added by MAC- 2 FrameCoontrol, 1-SeqNo,2- SrcPanId,
//  2 -DstPanId 8-src address, 8- dst address, */
#define MAC_OVERHEAD                    (23U)

/* Processing Time of a packet in milliseconds */
#define PROCESSING_TIME_OF_PACKET       (7U)

/* Encryption or Decryption time using Transceiver */
#define ENCRYPTION_TIME                 (36U)

/* Total Static One hop ACK Wait Timeout Calculation */
#define ONE_HOP_ACK_WAIT_TIMEOUT        (MAX_FRAME_TX_TIME + PROCESSING_TIME_OF_PACKET + ENCRYPTION_TIME)

/************************ Static Prototypes **********************************/
static void dataTimerHandler(uintptr_t context);
static void ackTimerHandler(void);
static void frameTxCallback(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
static bool sendData(uint8_t dataType, uint8_t addr_len, uint8_t *addr, uint8_t msglen,uint8_t *msgpointer,uint8_t msghandle, bool ackReq, DataConf_callback_t ConfCallback);
queue_t frameTxQueue;
queue_t frameRxQueue;
API_UINT16_UNION myPANID;
// #if defined(ENABLE_NETWORK_FREEZER)
// #ifdef ENDDEVICE
// void connectionConfirm(miwi_status_t status);
// #endif
// #endif

/************************ Static Declarations **********************************/
/* current operating channel for the device */
uint8_t  currentChannel;
uint8_t  meshSequenceNumber;
#if defined(ENABLE_FREQUENCY_AGILITY)
uint8_t newChannelToUpdate = 0xFFU;
uint16_t backupParentNwkAddress;
#endif

/* If a module is busy, it can increment this busylock 
so that device wont enter into sleep */
uint8_t busyLock = 0;
/* State of the Mesh stack */
meshState_t meshCurrentState = INITIAL_STATE;

queue_t ackWaitFrameQueue;
queue_t nonAckFrameQueue;
#ifndef ENDDEVICE
queue_t indirectFrameQueue;
#endif
static SYS_Timer_t meshDataTimer;
static SYS_TIME_HANDLE meshDataTimerHandle;
static DataConf_callback_t sentFrameCb;
bool txCallbackReceived = true;
miwi_status_t initStatus = SUCCESS;
bool pdsRestoreFlag = false;
/************************ Function Definitions **********************************/
/************************************************************************************
* Function:
*      miwi_status_t MiApp_ProtocolInit(void)
*
* Summary:
*      This function initialize the Microchip proprietary wireless protocol
*
* Description:
*      This is the primary user interface function to initialize the Microchip
*      proprietary wireless protocol, which is chosen by the application layer.
*      Usually, this function must be called after the hardware initialization,
*      before any other MiApp interface can be called.
*
* PreCondition:
*      Hardware initialization has been done.
*
* Parameters:
*      None
*
* Returns:
*      miwi_status_t status of Initialization
       In Mesh, when Network Freezer is enabled, stack try to restores the network freezer
.......data and tries to reconnect to the network, this is indicated with reconnection progress status
*
* Example:
*      <code>
*      HardwareInit();
*      MiApp_ProtocolInit();
*      </code>
*
* Remarks:
*      None
*
*********************************************************************************/
miwi_status_t MiApp_ProtocolInit(void)
{
    /* Initialize MiMAC and PHy layers */
    MiMAC_Init();

#if defined(IEEE_802_15_4)
    {
        uint16_t tmp = 0xFFFF;
        MiMAC_SetAltAddress((uint8_t *)&tmp);
        MiMAC_SetPanId((uint8_t *)&myPANID.Val);
    }
#endif

#ifdef PAN_COORDINATOR
    /* Initialize Coordinator Table*/
    coordinatorTableInit();
#endif

#ifndef ENDDEVICE
	/* Initialize Routing Table*/
	initRouteTable();

    /* Initialize Device Table*/
    deviceTableInit();

	initRebroadcastTable();
#endif
	
	initDuplicateRejectionTable();

    /* Mesh stack is initialized, change state to INIT_STATE */
    meshCurrentState = INIT_STATE;

    /*Short Address at Boot */
    myShortAddress = 0xFFFF;

    qmm_queue_init(&ackWaitFrameQueue, QUEUE_CAPACITY);
    qmm_queue_init(&nonAckFrameQueue, QUEUE_CAPACITY);

#ifndef ENDDEVICE
    qmm_queue_init(&indirectFrameQueue, QUEUE_CAPACITY);
#endif

    qmm_queue_init(&frameTxQueue, QUEUE_CAPACITY);
    qmm_queue_init(&frameRxQueue, QUEUE_CAPACITY);
	meshDataTimer.interval = SW_TIMER_INTERVAL;
	meshDataTimer.mode = SYS_TIME_PERIODIC;
	meshDataTimer.handler = dataTimerHandler;
    uint8_t myData = 0U;
    meshDataTimerHandle = SYS_TIME_CallbackRegisterMS(&dataTimerHandler, (uintptr_t)&myData, (uint32_t)SW_TIMER_INTERVAL, SYS_TIME_PERIODIC);
    if(meshDataTimerHandle == SYS_TIME_HANDLE_INVALID)
    {
        return FAILURE;
    }

	meshSequenceNumber = (uint8_t)rand();

#if defined(ENABLE_NETWORK_FREEZER)
    /* Restore Network Information Base from Non Volatile Memory 
       - If it fails, initialize the PDS Items */
    if (!PDS_IsAbleToRestore(MIWI_ALL_MEMORY_MEM_ID))
    {
        if(!PDS_Restore(MIWI_ALL_MEMORY_MEM_ID))
        {
//        PDS_InitItems(PDS_NULL_ID, PDS_MAX_ID);
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_MAX_ID);
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_BLOOM_VALUE_ID);
#if defined(ENABLE_FREQUENCY_AGILITY)
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_CHANNEL_UPDATE_ID);
#endif
#if defined(PAN_COORDINATOR)
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_COMM_DEVICE_TABLE_ID);
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_COORDINATOR_TABLE_ID);
#endif
#if defined(PAN_COORDINATOR) || defined (COORDINATOR)
		PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_ROUTE_TABLE_HOP_ID);
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_DEVICE_TABLE_NONSLEEP_ID);
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_DEVICE_TABLE_SLEEP_ID);
        PDS_InitItems(PDS_GENERAL_INFO_ID, PDS_ROUTE_TABLE_COORD_ID);
#endif
        }
    }
    else
    {
        pdsRestoreFlag = true; 
        if(PDS_Restore(MIWI_ALL_MEMORY_MEM_ID))
        {
            PDS_UpdateMemory_t pdsItemInfo;           
            pdsItemInfo.id = PDS_GENERAL_INFO_ID;
            pdsItemInfo.size = sizeof(MeshGeneralInfotMem_t);
            pdsItemInfo.data = &genInfoMem;
            pdsUpdateMemoryCallback(&pdsItemInfo);
        }
    }

#if defined(ENABLE_SLEEP_FEATURE) && defined(ENDDEVICE) 
            MiMAC_RFDDemoInit();
#else
            MiMAC_FFDDemoInit();
#endif

#endif
    return initStatus;
}

// #if defined(ENABLE_NETWORK_FREEZER)
#ifdef ENDDEVICE
void connectionConfirm(miwi_status_t status)
{
#if defined(ENABLE_NETWORK_FREEZER)
    /* Indicate application with status of reconnection */
    if (NULL != reconnectionCallback)
    {
        reconnectionCallback(status);
    }
#endif
#if defined(ENABLE_FREQUENCY_AGILITY)
    if (newChannelToUpdate != 0xFFU)
    {
        /* Start the timer for channel update time */
            timerHandles.joinTimerchannelUpdate.handler = channelUpdateTimerExpired;
            timerHandles.joinTimerchannelUpdate.timeout = (CHANNEL_UPDATE_TIME_IN_SEC * 1000);
            timerHandles.joinTimerchannelUpdate.interval = (CHANNEL_UPDATE_TIME_IN_SEC * 1000);
            timerHandles.joinTimerchannelUpdate.mode = SYS_TIME_SINGLE;
        
        uint8_t myData = 0U;
        timerHandles.joinTimerchannelUpdateHandle = SYS_TIME_CallbackRegisterMS(timerHandles.joinTimerchannelUpdate.handler, (uintptr_t)&myData, timerHandles.joinTimerchannelUpdate.interval, SYS_TIME_SINGLE);
        if(timerHandles.joinTimerchannelUpdateHandle == SYS_TIME_HANDLE_INVALID)
        {
           return;	
        }
    }
#endif
}
#endif
// #endif

/*********************************************************************
 * BOOL    isSameAddress(uint8_t *Address1, uint8_t *Address2)
 *
 * Overview:        This function compares two long addresses and returns
 *                  the boolean to indicate if they are the same
 *
 * PreCondition:
 *
 * Input:
 *          Address1    - Pointer to the first long address to be compared
 *          Address2    - Pointer to the second long address to be compared
 *
 * Output:
 *          If the two address are the same
 *
 * Side Effects:
 *
 ********************************************************************/
bool  isSameAddress(uint8_t *Address1, uint8_t *Address2)
{
    uint8_t i;

    for(i = 0U; i < MY_ADDRESS_LENGTH; i++)
    {
        if( Address1[i] != Address2[i] )
        {
            return false;
        }
    }
    return true;
}

/************************************************************************************
    * Function:
    *      bool MiApp_Set(set_params id, uint8_t *value )
    *
    * Summary:
    *      This function sets the information base values
    *
    * Description:
    *      This is the primary user interface function to set the different values in the MiWi
    *      stack
    *
    * PreCondition:
    *      Protocol initialization has been done.
    *
    * Parameters:
    *      set_params id -  The identifier of the value to be set
    *      value - value to be set
    *
    * Returns:
    *      a boolean to indicate if set operation has been performed successfully
    *
    * Example:
    *      <code>
    *      if( true == MiApp_Set(CHANNEL, 15) )
    *      {
    *          // channel changes successfully
    *      }
    *      </code>
    *
    * Remarks:
    *      None
    *
    *********************************************************************************/
bool MiApp_Set(miwi_params_t id, uint8_t *value)
{
    switch(id)
    {
        case CHANNEL:
        {
            if(MiMAC_Set(MAC_CHANNEL, value))
            {
                currentChannel = *value;
                return true;
            }
        }
        break;
        case SHORT_ADDRESS:
        {
            API_UINT16_UNION shortAddress;
            shortAddress.v[0] = value[0];
            shortAddress.v[1] = value[1];
            if(MiMAC_SetAltAddress((uint8_t *)&shortAddress.Val))
            {
                myShortAddress = shortAddress.Val;
                return true;
            }
        }
        break;

        case PANID:
        {
	        API_UINT16_UNION panID;
	        panID.v[0] = value[0];
	        panID.v[1] = value[1];
	        if(MiMAC_SetPanId((uint8_t *)&panID.Val))
	        {
		        myPANID.Val = panID.Val;
		        return true;
	        }
        }
		break;

        case BLOOM_AUTO_JOIN:
        {
            bloomFilterAutoJoin = *value;
            return true;
        }
        break;
        case CHANNELMAP:
        {
            memcpy(&gChannelMap, value, sizeof(gChannelMap));
            return true;
        }
        break;
        case CAPABILITYINFO:
        {
            gCapabilityInfo = *value;
            return true;
        }
        break;

#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
        case ROUTE_TEST_MODE:
        {
#if defined(PAN_COORDINATOR) || defined (COORDINATOR)
	        routeTestMode = *value;
	        return true;
#endif
        }
        break;
#endif
        default:
            //Handle exceptions if any
            break;
    }
    return false;
}

bool MiTrxParam_Get(miwi_params_t id, uint8_t *value)
{
    switch(id)
    {
        case CHANNEL:
        {
            PHY_Retval_t retVal = PHY_FAILURE;
            uint8_t phyChannel = 0;
            retVal = PHY_PibGet(phyCurrentChannel, &phyChannel);
            if(PHY_SUCCESS == retVal)
            {
                currentChannel = phyChannel;
                return true;
            }
        }
        break;
        
        case SHORT_ADDRESS:
        {
            PHY_Retval_t retVal = PHY_FAILURE;
            uint16_t shortAddr = 0;
            retVal = PHY_PibGet(macShortAddress, (uint8_t*)&shortAddr);
            if(PHY_SUCCESS == retVal)
            {
                myShortAddress = shortAddr;
                return true;
            }
        }
        break;

        case PANID:
        {
            PHY_Retval_t retVal = PHY_FAILURE;
            uint16_t panId = 0;
            retVal = PHY_PibGet(macPANId, (uint8_t*)&panId);
            if(PHY_SUCCESS == retVal)
            {
                myPANID.Val = panId;
                return true;
            }
        }
		break;
        
        default:
		{
            //Handle exceptions if any
            break;
	    }
    }
    return false;
}


void MeshTasks(void)
{
	if ((frameTxQueue.size > 0U) && (txCallbackReceived))
    {
#ifdef ENDDEVICE
		if (!edInPollingState)
#endif
		{
			TxFrame_t *txFramePtr = NULL;
            buffer_t *buffer_header = NULL;
            buffer_header =  qmm_queue_remove(&frameTxQueue, NULL);
            txFramePtr = (TxFrame_t *)BMM_BUFFER_POINTER(buffer_header);
			if (NULL != txFramePtr)
			{
                sentFrameCb = txFramePtr->txFrameEntry.frameConfCallback;
				busyLock++; 
				MiMAC_SendPacket(txFramePtr->txFrameEntry.frameParam, txFramePtr->txFrameEntry.frame,
					txFramePtr->txFrameEntry.frameLength, txFramePtr->txFrameEntry.frameHandle,
					frameTxCallback,txFramePtr->pMemClr);
				txCallbackReceived = false;
                bmm_buffer_free(buffer_header);
			}
		}
	}
    
    if (frameRxQueue.size != 0U) 
    {
    if(MiMAC_ReceivedPacket())
	{
		frameParse(&MACRxPacket);
	}
    }
#ifdef ENDDEVICE
	if ((IN_NETWORK_STATE == meshCurrentState) && ((RXONWHENIDLE_ED_ADDRESS_MASK & myShortAddress) == 0U) && (0U == busyLock))
	{   
      MiMAC_PowerState(POWER_STATE_DEEP_SLEEP);
	}
#endif
}


bool MiApp_SendData(uint8_t addr_len, uint8_t *addr, uint8_t msglen, uint8_t *msgpointer, uint8_t msghandle,bool ackReq, DataConf_callback_t ConfCallback)
{
	return sendData(NWK_FRAMETYPE_DATA, addr_len, addr, msglen, msgpointer, msghandle, ackReq, ConfCallback);
}

bool MiApp_ManuSpecSendData(uint8_t addr_len, uint8_t *addr, uint8_t msglen, uint8_t *msgpointer, uint8_t msghandle,
		bool ackReq, DataConf_callback_t ConfCallback)
{
	return sendData(NWK_FRAMETYPE_MANU_SPEC, addr_len, addr, msglen, msgpointer, msghandle, ackReq, ConfCallback);
}

static bool sendData(uint8_t dataType, uint8_t addr_len, uint8_t *addr, uint8_t msglen, uint8_t *msgpointer,
		uint8_t msghandle, bool ackReq, DataConf_callback_t ConfCallback)
{
    API_UINT16_UNION shortDstAddr;
//    API_UINT64_UNION longDstAddr;
    buffer_t *buffer_header = NULL;
#ifndef ENDDEVICE
	bool sendWithAckCheck = false;
#endif
	shortDstAddr.v[0] = addr[0];
	shortDstAddr.v[1] = addr[1];
	if (MESH_MAX_PAYLOAD_SIZE < msglen || myShortAddress == shortDstAddr.Val)
	{
		return false;
	}

	if (IN_NETWORK_STATE == meshCurrentState || IN_NETWORK_PARTIAL_STATE == meshCurrentState)
	{
		if (SHORT_ADDR_LEN == addr_len)
		{
            buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
			NwkFrame_t *nwkFramePtr = NULL;
	       if (NULL == buffer_header) 
           {
		   /* Buffer is not available */
		        return false;
	        }
            nwkFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);

			if (NULL == nwkFramePtr)
			{
				return false;
			}
			if (NWK_FRAMETYPE_MANU_SPEC == dataType)
			{
				nwkFramePtr->nwkDataFrame.meshFrame.header.frameControl.frameType = NWK_FRAMETYPE_MANU_SPEC;
			}
			else
			{
				nwkFramePtr->nwkDataFrame.meshFrame.header.frameControl.frameType = NWK_FRAMETYPE_DATA;
			}
			nwkFramePtr->nwkDataFrame.meshFrame.header.dstAddr = shortDstAddr.Val;
			nwkFramePtr->nwkDataFrame.meshFrame.header.srcAddr = myShortAddress;
			nwkFramePtr->nwkDataFrame.meshFrame.header.frameControl.ackRequest = (uint8_t)ackReq;
			nwkFramePtr->nwkDataFrame.meshFrame.payloadLen = msglen;
			nwkFramePtr->nwkDataFrame.msghandle = msghandle;
			nwkFramePtr->nwkDataFrame.routedOrRetransmitFrame = false;
			nwkFramePtr->nwkDataFrame.confCallback = ConfCallback;
			memcpy(nwkFramePtr->nwkDataFrame.meshFrame.payload, msgpointer, msglen);


			if (MESH_BROADCAST_TO_COORDINATORS <= shortDstAddr.Val)
			{
				nwkFramePtr->nwkDataFrame.meshFrame.header.frameControl.ackRequest = 0;
				ackReq = 0;
#ifndef ENDDEVICE
				if (MESH_BROADCAST_TO_ALL == shortDstAddr.Val)
				{
					for (uint8_t loopIndex = 1; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
					{
						if (true == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr))
						{
//							if (50 < MiMem_PercentageOfFreeBuffers())//this feature not implemented in bmm_buffer in Standalone 15.4 PHY-will be updated in future
                                if (true)
							{
                                buffer_t *buffer_header_indirectfptr = NULL;
                                NwkFrame_t *indirectBroadcastFramePtr = NULL;
                                 buffer_header_indirectfptr = bmm_buffer_alloc(LARGE_BUFFER_SIZE);                                
                                if (NULL == buffer_header_indirectfptr) 
                                {
		                        /* Buffer is not available */
		                             return false;
	                            }
                                indirectBroadcastFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header_indirectfptr);
								if (NULL != indirectBroadcastFramePtr)
								{
									memcpy(indirectBroadcastFramePtr, nwkFramePtr, sizeof(NwkFrame_t));
									indirectBroadcastFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->indirectDataWaitInterval + 1;
									indirectBroadcastFramePtr->nwkDataFrame.retry = 0;
									/* End device address as next hop Address */
									indirectBroadcastFramePtr->nwkDataFrame.nextHopAddr = myShortAddress + loopIndex;
									indirectBroadcastFramePtr->nwkDataFrame.confCallback = NULL;
                                    qmm_queue_append(&indirectFrameQueue, buffer_header_indirectfptr);
								}
							}
						}
					}
				}
#endif
				if(!sendDataFrame(nwkFramePtr, MAC_BROADCAST_ADDR, nonAckDataCallback))
				{
					if (NULL != ConfCallback)
					{
						ConfCallback(nwkFramePtr->nwkDataFrame.msghandle, FAILURE, (uint8_t *)nwkFramePtr);
					}
                    bmm_buffer_free(buffer_header);
					return false;
				}
				else
				{
                    qmm_queue_append(&nonAckFrameQueue, buffer_header);
				}
			}
			else
			{
#ifndef ENDDEVICE
				if ((ENDDEVICE_MASK & shortDstAddr.Val) && (myShortAddress == (shortDstAddr.Val & COORD_MASK)))
				{
					nwkFramePtr->nwkDataFrame.nextHopAddr = shortDstAddr.Val;
					//data for my end device
					if (RXONWHENIDLE_ED_ADDRESS_MASK & shortDstAddr.Val)
					{
						// data for RX ON device
						sendWithAckCheck = true;
					}
					else
					{
						// data for RX OFF device
						nwkFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->indirectDataWaitInterval + 1;
						nwkFramePtr->nwkDataFrame.retry = miwiDefaultRomOrRamParams->frameRetry + 1;
                        qmm_queue_append(&indirectFrameQueue, buffer_header);
					}
				}
				else
				{
					if (IN_NETWORK_PARTIAL_STATE == meshCurrentState)
					{
						nwkFramePtr->nwkDataFrame.nextHopAddr = myShortAddress & COORD_MASK;
					}
					else
					{
						nwkFramePtr->nwkDataFrame.nextHopAddr = getNextHopAddr(shortDstAddr.Val & COORD_MASK);
					}
					if (DEFAULT_NEXT_HOP_ADDR == nwkFramePtr->nwkDataFrame.nextHopAddr)
					{
						if (initiateRouteReq(shortDstAddr.Val))
						{
							nwkFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->routeReqWaitInterval + 1;
                            qmm_queue_append(&routeFrameQueue, buffer_header);
							return true;
						}
						else
						{
							if (NULL != ConfCallback)
							{
								ConfCallback(nwkFramePtr->nwkDataFrame.msghandle, FAILURE, (uint8_t *)nwkFramePtr);
							}
                            bmm_buffer_free(buffer_header);
							return false;
						}
					}
					else
					{
						sendWithAckCheck = true;
					}
				}
				if (sendWithAckCheck)
#else
				nwkFramePtr->nwkDataFrame.nextHopAddr = myShortAddress & COORD_MASK;
#endif
				{
					if (ackReq)
					{
					    if (nwkFramePtr->nwkDataFrame.nextHopAddr == shortDstAddr.Val)
						{
							nwkFramePtr->nwkDataFrame.timeout = calculateAckWaitTimeout(1, nwkFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
						}
						else
						{
							nwkFramePtr->nwkDataFrame.timeout = calculateAckWaitTimeout(miwiDefaultRomOrRamParams->numOfCoordinators + 1, nwkFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
						}
						nwkFramePtr->nwkDataFrame.retry = miwiDefaultRomOrRamParams->frameRetry + 1U;
						if(!sendDataFrame(nwkFramePtr, nwkFramePtr->nwkDataFrame.nextHopAddr, ackReqDataConfcb))
						{
							nwkFramePtr->nwkDataFrame.retry -= 1U;
						}
                        qmm_queue_append(&ackWaitFrameQueue, buffer_header);                     
                        
					}
					else
					{
						nwkFramePtr->nwkDataFrame.retry = 0U;
						if(!sendDataFrame(nwkFramePtr, nwkFramePtr->nwkDataFrame.nextHopAddr, nonAckDataCallback))
						{
							if (NULL != ConfCallback)
							{
								ConfCallback(nwkFramePtr->nwkDataFrame.msghandle, FAILURE, (uint8_t *)nwkFramePtr);
							}
                            bmm_buffer_free(buffer_header);
							return false;
						}
                        qmm_queue_append(&nonAckFrameQueue, buffer_header);
					}
				}

			}

			return true;
		}
		else if (LONG_ADDR_LEN == addr_len)
		{
			return false;
		}
		else
		{
			//Invalid address length
		}
        
    }
	return false;
    
}

bool MiApp_Get(miwi_params_t id, uint8_t *value)
{
    switch((miwi_params_t)id)
    {
        case CHANNEL:
        {
            *value = currentChannel;
            return true;
        }
        break;
        case SHORT_ADDRESS:
        {
            value[0] = myShortAddress;
            value[1] = myShortAddress >> 8;
            return true;
        }
		break;
#ifndef PAN_COORDINATOR
        case PARENT_SHORT_ADDRESS:
        {
	        value[0] = myParentShortAddress;
	        value[1] = myParentShortAddress >> 8;
	        return true;
        }
        break;
#endif
        case PANID:
        {
	        value[0] = myPANID.Val;
	        value[1] = myPANID.Val >> 8;
	        return true;
        }
        break;
        case CHANNELMAP:
        {
            memcpy(value, &gChannelMap, sizeof(gChannelMap));
            return true;
        }
        break;
        case CAPABILITYINFO:
        {
            *value = gCapabilityInfo;
            return true;
        }
        break;
        default:
        break;
    }
    return false;
}


/******************************************************************************
* Function:
*      static void dataTimerHandler(SYS_Timer_t *timer)
*
* Summary:
*      This function handles overall data timers at 1sec periodicity
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
static void dataTimerHandler(uintptr_t context)
{
    uint8_t channel = 0U;
    uint16_t panId = 0U, shortAddress = 0U;
	ackTimerHandler();
	duplicateRejectionTimerHandler();
#ifndef ENDDEVICE
	if (IN_NETWORK_STATE == meshCurrentState)
	{
		routeTimerHandler();
		rebroadcastTimerHandler();
		indirectDataTimerHandler();
	}
    keepAliveTimerHandler();
#endif
//    MiTrxParam_Get(SHORT_ADDRESS, (uint8_t*)&shortAddress);
//    MiTrxParam_Get(CHANNEL, &channel);
//    MiTrxParam_Get(PANID, (uint8_t*)&panId);
    (void)context;
}

/******************************************************************************
* Function:
*      void routeTimerHandler(void)
*
* Summary:
*      This function handles Ack frame Timers at 1sec periodicity
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
static void ackTimerHandler(void)
{
	NwkFrame_t *dataFramePtr = NULL;
    buffer_t *buffer_header = NULL;
	uint8_t loopIndex;

	for (loopIndex = 0U; loopIndex < ackWaitFrameQueue.size; loopIndex++)
	{
        buffer_header =  qmm_queue_remove(&ackWaitFrameQueue, NULL);
		if (NULL == buffer_header)
		{
			return;
		}
        dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataFramePtr)
		{
			return;
		}

		if ((0U != dataFramePtr->nwkDataFrame.timeout) && (--dataFramePtr->nwkDataFrame.timeout) == 0U)
		{
			if (0U != dataFramePtr->nwkDataFrame.retry && (--dataFramePtr->nwkDataFrame.retry) == 0U)
			{
				DataConf_callback_t callback = dataFramePtr->nwkDataFrame.confCallback;
				if (NULL != callback)
				{
					callback(dataFramePtr->nwkDataFrame.msghandle, NO_ACK, (uint8_t *)dataFramePtr);
				}
                bmm_buffer_free(buffer_header);
#ifndef ENDDEVICE
				{
					uint16_t dstAddr = dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr;
					if ((!(dstAddr & ENDDEVICE_MASK)) && ((dstAddr & COORD_MASK) != myShortAddress))
					{
						uint8_t coordIndex = dstAddr >> 8U;
						if (0U != miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score &&
						(--miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score == 0U))
						{
							removeRoute(dstAddr);
						}
					}
				}
#endif
			}
			else
			{
				dataFramePtr->nwkDataFrame.routedOrRetransmitFrame = true;
				if(!sendDataFrame(dataFramePtr, dataFramePtr->nwkDataFrame.nextHopAddr, ackReqDataConfcb))
				{
					dataFramePtr->nwkDataFrame.retry -= 1U;
				}
				dataFramePtr->nwkDataFrame.timeout = (uint32_t)calculateAckWaitTimeout(miwiDefaultRomOrRamParams->numOfCoordinators + 1U, dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
                qmm_queue_append(&ackWaitFrameQueue, buffer_header);
			}
		}
		else
		{
            qmm_queue_append(&ackWaitFrameQueue, buffer_header);
		}
	}
}

/************************************************************************************
* Function:
*      void ackReqDataConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function check status of ack requested frame and re-intiates tx
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
void ackReqDataConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
	NwkFrame_t *dataFramePtr = NULL;
	uint8_t loopIndex;    
	uint8_t seq_no = msgPointer[12];
    buffer_t *buffer_header;
#ifndef PAN_COORDINATOR
	if (myShortAddress & ENDDEVICE_MASK)
	{
		checkLinkFailureAtNoAck(status);
	}
#endif
	for (loopIndex = 0U; loopIndex < ackWaitFrameQueue.size; loopIndex++)
	{
        buffer_header =  qmm_queue_remove(&ackWaitFrameQueue, NULL);
		if (NULL == buffer_header)
		{
			return;
		}
        dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataFramePtr)
		{
			return;
		}

		if (seq_no == dataFramePtr->nwkDataFrame.meshFrame.header.sequenceNumber && handle == dataFramePtr->nwkDataFrame.msghandle)
		{
			uint16_t dstAddr = dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr;
			if (SUCCESS != status)
			{
#ifndef ENDDEVICE
				if (NO_ACK == status)
				{
					if ((!(dstAddr & ENDDEVICE_MASK)) && ((dstAddr & COORD_MASK) != myShortAddress))
					{
						uint8_t coordIndex = dstAddr >> 8U;
						if (0U != miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score &&
							(--miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score == 0U))
						{
							removeRoute(dstAddr);
						}
					}
				}
#endif
				if ((0U != dataFramePtr->nwkDataFrame.retry && (0U == (--dataFramePtr->nwkDataFrame.retry))) || (DISCONNECTED == meshCurrentState))
				{
					DataConf_callback_t callback = dataFramePtr->nwkDataFrame.confCallback;
					if (NULL != callback)
					{
						callback(handle, status, msgPointer);
					}
                    bmm_buffer_free(buffer_header);
                    
				}
				else
				{
					dataFramePtr->nwkDataFrame.routedOrRetransmitFrame = true;
					if(!sendDataFrame(dataFramePtr, dataFramePtr->nwkDataFrame.nextHopAddr, ackReqDataConfcb))
					{
						dataFramePtr->nwkDataFrame.retry -= 1U;
					}
                    qmm_queue_append(&ackWaitFrameQueue, buffer_header);
				}
			}
			else
			{
#ifndef ENDDEVICE
				if ((!(dstAddr & ENDDEVICE_MASK)) && ((dstAddr & COORD_MASK) != myShortAddress))
				{
					uint8_t coordIndex = dstAddr >> 8U;
					miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score = DEFAULT_SCORE;
				}
#endif
				dataFramePtr->nwkDataFrame.timeout = (uint32_t)calculateAckWaitTimeout(miwiDefaultRomOrRamParams->numOfCoordinators + 1U, dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
                qmm_queue_append(&ackWaitFrameQueue, buffer_header);
			}
		}
		else
		{
            qmm_queue_append(&ackWaitFrameQueue, buffer_header);
		}
	}
}


static void frameTxCallback(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
	busyLock--;
	txCallbackReceived = true;
	DataConf_callback_t callback = sentFrameCb;
	if (NULL != callback)
	{
		callback(handle, status, msgPointer);
	}
	else
	{
        msgPointer = NULL;
	}
}

void nonAckDataCallback(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
	NwkFrame_t *dataFramePtr = NULL;
    buffer_t *buffer_header = NULL;
    if(nonAckFrameQueue.size)
    {
    buffer_header =  qmm_queue_remove(&nonAckFrameQueue, NULL);
    }
    if(buffer_header == NULL){
        return;
    }
    dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
#ifndef PAN_COORDINATOR
	if (myShortAddress & ENDDEVICE_MASK)
	{
		checkLinkFailureAtNoAck(status);
	}
#endif
	if (NULL != dataFramePtr || DISCONNECTED == meshCurrentState)
	{
		DataConf_callback_t callback = dataFramePtr->nwkDataFrame.confCallback;
		if (NULL != callback)
		{
			callback(handle, status, msgPointer);
		}
	}
        bmm_buffer_free(buffer_header);
	return;
}

uint16_t MiApp_MeshGetNextHopAddr(uint16_t destAddress)
{
#ifndef ENDDEVICE
	if (myShortAddress & ENDDEVICE_MASK)
	{
		return myShortAddress & COORD_MASK;
	}
	return getNextHopAddr(destAddress);
#else
	return myShortAddress & COORD_MASK;
#endif
}

#if defined(ENABLE_NETWORK_FREEZER)
bool MiApp_ResetToFactoryNew(void)
{
	if (PDS_DeleteAll(true))
	{
        NVIC_SystemReset();
		return true;
	}
	else
	{
		return false;
	}
}
#endif

#ifdef ENDDEVICE
bool MiApp_ReadyToSleep(uint32_t* sleepableTime)
{
	APP_ReadyToSleep(sleepableTime);
	// if (busyLock == 0)
	// {
    //         *sleepableTime = timerHandles.keepAliveTimerRxOffEd.timeout;
	// }
	// return (busyLock == 0);
}
#endif

bool MiApp_IsConnected(void)
{
	if (IN_NETWORK_STATE == meshCurrentState || IN_NETWORK_PARTIAL_STATE == meshCurrentState)
	{
		return true;
	}
	return false;
}

uint8_t calculateAckWaitTimeout(uint8_t hops, uint16_t destAddr)
{
	uint32_t ackTimeOut = 0UL;
	ackTimeOut = ((uint32_t)ONE_HOP_ACK_WAIT_TIMEOUT) * ((uint32_t)hops);
	
	/* If  device is end device or destination is end device, add maxDataRequest interval to the ackwaitduration */
	if (((myShortAddress & ENDDEVICE_MASK) != 0U) && ((myShortAddress & RXONWHENIDLE_ED_ADDRESS_MASK) == 0U))
	{
		ackTimeOut += (miwiDefaultRomOrRamParams->maxDataRequestInterval * 1000U);
	}
	
	if (((destAddr & ENDDEVICE_MASK) != 0U) && ((destAddr & RXONWHENIDLE_ED_ADDRESS_MASK) == 0U))
	{
		ackTimeOut += (miwiDefaultRomOrRamParams->maxDataRequestInterval * 1000U);
	}
	return ((uint8_t)((ackTimeOut/1000UL) + 1U));
}