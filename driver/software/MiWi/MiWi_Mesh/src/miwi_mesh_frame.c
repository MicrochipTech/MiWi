/**
* \file  miwi_mesh_frame.c
*
* \brief MiWi Mesh Protocol Frame Handling implementation
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
#include <string.h>
#include "config/default/definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************

typedef struct rebroadcastTable
{
	uint16_t nwkSrcAddr;
	uint8_t nwkSeqNo;
	uint8_t timeout;
}rebroadcastTable_t;
/************************ Static Declarations **********************************/
/* Data Indication callback */
static PacketIndCallback_t pktRxcallback = NULL;
/* Data Indication callback */
static PacketIndCallback_t pktManuSpecRxcallback = NULL;
/************************ Static Prototypes **********************************/
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
#ifndef ENDDEVICE
static bool checkRebroadcastTableEntry(uint16_t srcAddr, uint8_t seqNo);
#endif
static void sendNwkAck(uint8_t srcAddrLen, uint8_t *srcAddr, uint16_t dstAddr, uint8_t seqNum);
static void memFreeConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
/************************************************************************************
 * Function:
 *      bool  MiApp_SubscribeDataIndicationCallback(PacketIndCallback_t callback)
 *
 * Summary:
 *      This function return a boolean if subscribtion for rx message is successful
 *
 * Description:
 *      This is the primary user interface functions for the application layer to
 *      call the Microchip proprietary protocol stack to register for message indication
 *      callback to the application. The function will call the protocol stack state machine
 *      to keep the stack running.
 *
 * PreCondition:
 *      Protocol initialization has been done.
 *
 * Parameters:
 *      None
 *
 * Returns:
 *      A boolean to indicates if the subscription operation is successful or not.
 *
 * Example:
 *      <code>
 *      if( true == MiApp_SubscribeDataIndicationCallback(ind) )
 *      {
 *      }
 *      </code\
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
bool  MiApp_SubscribeDataIndicationCallback(PacketIndCallback_t callback)
{
    if (NULL != callback)
    {
        pktRxcallback = callback;
        return true;
    }
    return false;
}

/************************************************************************************
 * Function:
 *      bool  MiApp_SubscribeMAnuSpecDataIndicationCallback(PacketIndCallback_t callback)
 *
 * Summary:
 *      This function return a boolean if subscription for rx message is successful
 *
 * Description:
 *      This is the primary user interface functions for the application layer to
 *      call the Microchip proprietary protocol stack to register for message indication
 *      callback to the application. The function will call the protocol stack state machine
 *      to keep the stack running.
 *
 * PreCondition:
 *      Protocol initialization has been done.
 *
 * Parameters:
 *      None
 *
 * Returns:
 *      A boolean to indicates if the subscription operation is successful or not.
 *
 * Example:
 *      <code>
 *      if( true == MiApp_SubscribeMAnuSpecDataIndicationCallback(ind) )
 *      {
 *      }
 *      </code\
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
bool  MiApp_SubscribeManuSpecDataIndicationCallback(PacketIndCallback_t callback)
{
    if (NULL != callback)
    {
        pktManuSpecRxcallback = callback;
        return true;
    }
    return false;
}

/************************************************************************************
 * Function:
 *      prepareHeader(uint8_t hops, uint8_t sourceAddressLen, uint8_t* SourceAddress,
 *                                uint16_t destAddress, NwkFrameHeader_t* meshHeader)
 *
 * Summary:
 *      This function prepares the general header of the mesh frame
 *
 * Description:
 *      This is the primary MiWi interface to construct general Mesh frame
 *
 * PreCondition:
 *      Initialization has been done.
 *
 * Parameters:
 *      uint8_t hops -    The number of hops maximum to be transmitted
 *      uint8_t sourceAddressLen - length of the source address
 *      uint8_t* SourceAddress - source address/orginator address
 *      uint16_t destAddress - destionation address of the devcie
 *      NwkFrameHeader_t* meshHeader - header of the mesh frame
 *
 * Returns:
 *      None.
 * Remarks:
 *      None
 *
 *****************************************************************************************/
void prepareGenericHeader(uint8_t hops, uint16_t srcAddr, uint16_t destAddr, MeshFrameHeader_t* meshHeader)
{
    /* Fill Mesh Header information */
    /* Number of hops the the packet can be sent maximum */
	if (0xFFU != hops)
	{
		meshHeader->hops = hops;
	}
	else
	{
		meshHeader->hops = miwiDefaultRomOrRamParams->numOfCoordinators + 1U;
	}
    

    /* By default, frame type is set to command, this can be changed after this function call */
    meshHeader->frameControl.frameType = NWK_FRAMETYPE_COMMAND;

    /* Fill interPan as 0 since it is reserved */
    meshHeader->frameControl.interPan = 0U;

#ifdef MESH_SECURITY
    /* By default, security should be set to 1 for all transmission */
    meshHeader->frameControl.securityEnabled = 1U;
#else
    /* By default, security should be set to 0 for all transmission */
    meshHeader->frameControl.securityEnabled = 0U;
#endif

    /* addressSameAsMAC  should be set based on the command */
    meshHeader->frameControl.addressSameAsMAC = 0U;

    /* Data pending for end device*/
    meshHeader->frameControl.dataPending = 0U;

    /* Set ackReq to 0 as default */
    meshHeader->frameControl.ackRequest = 0U;

    /* Fill reserved as 0 since it is reserved */
    meshHeader->frameControl.reserved = 0U;

    /* Fill the destination pan id */
    meshHeader->dstPanId = myPANID.Val;

    /* Fill the destination source and destination address */
    meshHeader->dstAddr = destAddr;
    meshHeader->srcAddr = srcAddr;

    /* Fill Sequence number */
    meshHeader->sequenceNumber = ++meshSequenceNumber;
}

/************************************************************************************
 * Function:
 *      uint8_t generalFrameConstruct(MeshFrameHeader_t* meshHeader, uint8_t* meshframe)
 *
 * Summary:
 *      This function prepares the common frame based on the header
 *
 * Description:
 *      This is the primary MiWi interface to construct general Mesh frame
 *
 * PreCondition:
 *      Initialization has been done.
 *
 * Parameters:
 *      uint8_t* meshframe -    The frame to be transmitted
 *      NwkFrameHeader_t* meshHeader - header of the mesh frame
 *
 * Returns:
 *      None.
 * Remarks:
 *      None
 *
 *****************************************************************************************/
uint8_t generalFrameConstruct(MeshFrameHeader_t* meshHeader, uint8_t* meshframe)
{
    uint8_t meshFrameIndex = 0U;

    /* Fill the actual frame to be transmitted based on mesh header information */
    meshframe[meshFrameIndex++] = meshHeader->hops;

    /* Fill the frame control information */
    memcpy(&meshframe[meshFrameIndex], (uint8_t*)&meshHeader->frameControl , sizeof(MeshFrameControl_t));//misra 21.15
    meshFrameIndex+= (uint8_t)sizeof(MeshFrameControl_t);

    /* Fill the sequence number */
    meshframe[meshFrameIndex++] = meshHeader->sequenceNumber;

    /* Fill the addressing information if addressSameAsMAC is 0, dont set otherwise */
    if (meshHeader->frameControl.addressSameAsMAC == 0U)
    {
        /* Fill the destination pan id */
        meshframe[meshFrameIndex++] = (uint8_t)meshHeader->dstPanId;
        meshframe[meshFrameIndex++] = (uint8_t)(meshHeader->dstPanId >> 8U);

        /* Fill the destination address*/
        meshframe[meshFrameIndex++] = (uint8_t)meshHeader->dstAddr;
        meshframe[meshFrameIndex++] = (uint8_t)(meshHeader->dstAddr >> 8U);

        /* Fill the source address*/
        meshframe[meshFrameIndex++] = (uint8_t)meshHeader->srcAddr;
        meshframe[meshFrameIndex++] = (uint8_t)(meshHeader->srcAddr >> 8U);
    }
#ifdef MESH_SECURITY
    /* if security is enabled, then axillary security header to be added into the frame */
    if (meshHeader->frameControl.securityEnabled)
    {
        /* Update Security Headers*/
        meshHeader->meshAuxSecHeader.securityLevel = miwiDefaultRamOnlyParams->securityLevel;
        meshHeader->meshAuxSecHeader.frameCounter.Val = meshOutgoingFrameCounter.Val;

        /* Fill the security level used for the frame*/
        meshframe[meshFrameIndex++] = meshHeader->meshAuxSecHeader.securityLevel;

        /* Fill the outgoing framecounter of the device */
        memcpy(&meshframe[meshFrameIndex], &meshHeader->meshAuxSecHeader.frameCounter, FRAME_COUNTER_LEN);
        meshFrameIndex+= FRAME_COUNTER_LEN;

        /* Fill the source Extended Address */
        memcpy(&meshframe[meshFrameIndex], myLongAddress, LONG_ADDR_LEN);
        meshFrameIndex+= LONG_ADDR_LEN;
    }
#endif
    return meshFrameIndex;
}

/************************************************************************************
 * Function:
 *      bool frameTransmit(NwkFrameHeader_t* meshHeader, uint8_t meshFrameHeaderLen, uint8_t meshFramePayloadLen
 *                                   uint8_t* meshframe, uint8_t macDstAddrLen, uint8_t* macDstAddress, DataConf_callback_t ConfCallback)
 *
 * Summary:
 *      This function transmit a packet
 *
 * Description:
 *      This is the primary MiMesh interface for the protocol layer to
 *      send a packet.
 *
 * PreCondition:
 *      Initialization has been done.
 *
 * Parameters:
 *      NwkFrameHeader_t* meshHeader -    header of the mesh frame
 *      uint8_t * meshframe -             Pointer to the buffer of frame payload
 *      uint8_t meshFrameHeaderLen -            The size of the frame header
 *      uint8_t meshFramePayloadLen -            The size of the frame payload
 *      DataConf_callback_t ConfCallback Callback function to be called once packet is sent
 *
 * Returns:
 *      A boolean to indicate if a packet has been received by the RF transceiver.
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
bool frameTransmit(MeshFrameHeader_t* meshHeader, uint8_t meshFrameHeaderLen, uint8_t meshFramePayloadLen, uint8_t* meshframe,
                    uint8_t macDstAddrLen, uint8_t* macDstAddress, uint8_t handle, DataConf_callback_t ConfCallback, buffer_t* memClrPtr)
{
    MAC_TRANS_PARAM *tParam = NULL;
    uint16_t dstiAddr;
	TxFrame_t *txFramePtr = NULL;
    buffer_t *buffer_header = NULL;
    buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
    if (NULL == buffer_header) 
    {
		/* Buffer is not available */
	    return false;
	}
    txFramePtr =  (TxFrame_t *)BMM_BUFFER_POINTER(buffer_header);
    if (NULL == txFramePtr)
	{
		return false;
	}
	tParam = &(txFramePtr->txFrameEntry.frameParam);
    /* Fill the MAC_TRANS_PARAM structure */
    tParam->flags.Val = 0U;
    tParam->flags.bits.packetType = PACKET_TYPE_DATA;

#ifdef MESH_SECURITY
    if (meshHeader->frameControl.securityEnabled)
    {
        uint8_t increaseInFrameLen = 0U;
        /* Determine which key should be used for outgoing security frame */
        uint8_t* keyTobeUsed = keyDetermineProcedure(macDstAddrLen);

        /* Secure the outgoing frame */
        increaseInFrameLen = secureFrame(meshHeader, meshFrameHeaderLen, meshFramePayloadLen, meshframe, keyTobeUsed);
        if (increaseInFrameLen == SECURITY_FAILURE)
            return false;
        else
           meshFramePayloadLen += increaseInFrameLen;
    }
#endif

    dstiAddr = ((uint16_t)macDstAddress[0] + ((uint16_t)macDstAddress[1] << 8U));

    /* If broadcast, set ackRequest to 0, set 1 otherwise */
    if ((macDstAddrLen == SHORT_ADDR_LEN) && (dstiAddr >= MESH_BROADCAST_TO_COORDINATORS)) //MISRA C 14.3 rule
    {
        tParam->flags.bits.broadcast = 1U;
		tParam->flags.bits.ackReq = 0U;
    }
    else
    {
        tParam->flags.bits.broadcast = 0U;
		tParam->flags.bits.ackReq = 1U;
    }
    /* security is disabled in MAC */
    tParam->flags.bits.secEn = 0U;

    #if defined(IEEE_802_15_4)
    if ((myShortAddress == 0xFFFFU) || ((meshCurrentState != IN_NETWORK_STATE) && (meshCurrentState != IN_NETWORK_PARTIAL_STATE)))
    {
        tParam->altSrcAddr = false;
    }
    else
    {
        tParam->altSrcAddr = true;
    }

    if (macDstAddrLen == SHORT_ADDR_LEN)
    {
       tParam->altDestAddr = true;
    }
    else
    {
       tParam->altDestAddr = false;
    }
    #endif

    #if defined(INFER_DEST_ADDRESS)
    tParam->flags.bits.destPrsnt = 0U;
    #else
    tParam->flags.bits.destPrsnt = (tParam->flags.bits.broadcast != 0U) ? 0U:1U;
    #endif

    #if defined(SOURCE_ADDRESS_ABSENT)
    if( tParam->flags.bits.packetType == PACKET_TYPE_COMMAND )
    {
        tParam->flags.bits.sourcePrsnt = 1U;
    }
    else
    {
        tParam->flags.bits.sourcePrsnt = 0U;
    }
    #else
    tParam->flags.bits.sourcePrsnt = 1U;
    #endif

    #if defined(IEEE_802_15_4)
    tParam->DestPANID.Val =  meshHeader->dstPanId;
    #endif

	txFramePtr->txFrameEntry.frame = meshframe;

	memcpy((uint8_t*)&(txFramePtr->txFrameEntry.frameDstAddr), macDstAddress, macDstAddrLen);//misra 21.15

	tParam->DestAddress = (uint8_t*)&(txFramePtr->txFrameEntry.frameDstAddr);
	txFramePtr->txFrameEntry.frameConfCallback = ConfCallback;
	txFramePtr->txFrameEntry.frameHandle = handle;
	txFramePtr->txFrameEntry.frameLength = meshFrameHeaderLen + meshFramePayloadLen;
    txFramePtr->pMemClr = memClrPtr;
    qmm_status_t variable = qmm_queue_append(&frameTxQueue, buffer_header);
    if (variable == QMM_SUCCESS)
    {
       MiMac_PostTask(false); 
       return true;
    }
    else
    {
        bmm_buffer_free(buffer_header);
        return false;
    }

}

static void memFreeConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
    (void)handle;
    (void)status;
    (void)msgPointer;
}

/******************************************************************************
* Function:
*      void frameParse(MAC_RECEIVED_PACKET *macRxPacket)
*
* Summary:
*      This function parses all the received frames
*
* Parameters:
*      MAC_RECEIVED_PACKET *macRxPacket - Pointer of the received MAC frame
*
* Returns:
*      None
******************************************************************************/
void frameParse(MAC_RECEIVED_PACKET *macRxPacket)
{
    MeshFrameHeader_t meshHeaderRcv;
    uint8_t *macSrcAddr = NULL;
    uint8_t macSrcAddrLen = 0U;
    uint8_t *nwkPayload = NULL;
    uint8_t parsedLen = 0U;
	uint8_t nwkPayloadLen = 0U;
#ifndef ENDDEVICE
	bool rebroadcast = false;
	bool rebroadcastedFrame = false;
#endif

	if (macRxPacket->SourcePANID.Val != myPANID.Val)
	{
		return;
	}

    /* Parse the Network Header fields */
    /* Extract Hops field */
    meshHeaderRcv.hops = macRxPacket->Payload[parsedLen++];

    /* Parse Mesh Frame Control field */
    meshHeaderRcv.frameControl =  (*(MeshFrameControl_t*)(macRxPacket->Payload + parsedLen));
    parsedLen += (uint8_t)sizeof(MeshFrameControl_t);
    /* Extract the received packet sequence number */
    meshHeaderRcv.sequenceNumber = macRxPacket->Payload[parsedLen++];

    if (macRxPacket->altSourceAddress)
    {
        macSrcAddrLen = SHORT_ADDR_LEN;
    }
    else
    {
        macSrcAddrLen = LONG_ADDR_LEN;
    }

    if (meshHeaderRcv.frameControl.addressSameAsMAC != 0U)
    {
        macSrcAddr = macRxPacket->SourceAddress;
        meshHeaderRcv.dstPanId = macRxPacket->SourcePANID.Val;

        if (macRxPacket->altSourceAddress)
        {
            meshHeaderRcv.srcAddr = ((uint16_t)macRxPacket->SourceAddress[0] | ((uint16_t)macRxPacket->SourceAddress[1] << 8U));
        }
		if (macRxPacket->flags.bits.broadcast)
		{
			meshHeaderRcv.dstAddr = 0xFFFFU;
		}
		else
		{
			meshHeaderRcv.dstAddr = myShortAddress;
		}
    }
    else
    {
        macSrcAddr = macRxPacket->SourceAddress;
        meshHeaderRcv.dstPanId = ((uint16_t)macRxPacket->Payload[parsedLen] | ((uint16_t)macRxPacket->Payload[parsedLen+1] << 8U));
        parsedLen += 2U;
        meshHeaderRcv.dstAddr = ((uint16_t)macRxPacket->Payload[parsedLen] | ((uint16_t)macRxPacket->Payload[parsedLen+1] << 8U));
        parsedLen += 2U;
        meshHeaderRcv.srcAddr = ((uint16_t)macRxPacket->Payload[parsedLen] | ((uint16_t)macRxPacket->Payload[parsedLen+1] << 8U));
        parsedLen += 2U;
    }
    nwkPayload = macRxPacket->Payload + parsedLen;

#ifdef MESH_SECURITY
    if (meshHeaderRcv.frameControl.securityEnabled)
    {
        meshHeaderRcv.meshAuxSecHeader.securityLevel = macRxPacket->Payload[parsedLen++];
        memcpy(&meshHeaderRcv.meshAuxSecHeader.frameCounter,&macRxPacket->Payload[parsedLen], sizeof(uint32_t));
        parsedLen += sizeof(uint32_t);
        memcpy(&meshHeaderRcv.meshAuxSecHeader.extendedAddress,&macRxPacket->Payload[parsedLen], sizeof(uint64_t));
        parsedLen += sizeof(uint64_t);
    }
	else
	{
		return;
	}
#else
    if (meshHeaderRcv.frameControl.securityEnabled)
	    return;
#endif

    nwkPayload = macRxPacket->Payload + parsedLen;
    nwkPayloadLen = macRxPacket->PayloadLen - parsedLen;

#ifdef MESH_SECURITY
    if (meshHeaderRcv.frameControl.securityEnabled != 0U)
    {
        uint8_t decreaseInFrameLen;
        /* Determine which key should be used for outgoing security frame */
        uint8_t* keyTobeUsed = keyDetermineProcedure(macSrcAddrLen);
        decreaseInFrameLen = unsecureFrame(&meshHeaderRcv, (uint8_t*)&meshHeaderRcv.meshAuxSecHeader.extendedAddress, parsedLen,nwkPayloadLen, macRxPacket->Payload, keyTobeUsed);
        if (decreaseInFrameLen == SECURITY_FAILURE)
            return;
        else
            nwkPayloadLen -= decreaseInFrameLen;
    }
#endif

#ifdef ENDDEVICE
   if( !(SYS_TIME_TimerPeriodHasExpired(timerHandles.dataWaitIntervalTimerEdHandle))) 
    {
       if (meshHeaderRcv.frameControl.dataPending)
       {
           startDataWaitIntervalTimer();
       }
    }
#endif

	if (MESH_BROADCAST_TO_COORDINATORS <= meshHeaderRcv.dstAddr || myShortAddress == meshHeaderRcv.dstAddr)
	{
#ifndef ENDDEVICE

		if (MESH_BROADCAST_TO_COORDINATORS <= meshHeaderRcv.dstAddr)
		{
			if (checkRebroadcastTableEntry(meshHeaderRcv.srcAddr, meshHeaderRcv.sequenceNumber))
			{
				rebroadcastedFrame = true;
			}
			else
			{
//				rebroadcast = true;
				if ((MESH_BROADCAST_TO_ALL == meshHeaderRcv.dstAddr) && ((meshHeaderRcv.hops - 1U) > 0U))
				{
					for (uint8_t loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
					{
						if (true == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr))
						{
//							if (50 < MiMem_PercentageOfFreeBuffers()) //this feature not implemented in bmm_buffer in Standalone 15.4 PHY-will be updated in future
                            if (true)   
							{
								NwkFrame_t *nwkFramePtr = NULL;
                                buffer_t *buffer_header = NULL;
                              
                                buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
                                if (NULL == buffer_header) 
                                {
		                            /* Buffer is not available */
		                            return;
	                            }
                                nwkFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);

			                    if (NULL == nwkFramePtr)
			                    {
				                    return;
			                    }

								memcpy(&(nwkFramePtr->nwkDataFrame.meshFrame.header), &meshHeaderRcv, sizeof(MeshFrameHeader_t));
								nwkFramePtr->nwkDataFrame.meshFrame.payloadLen = nwkPayloadLen;
								nwkFramePtr->nwkDataFrame.confCallback = NULL;
								memcpy(nwkFramePtr->nwkDataFrame.meshFrame.payload, nwkPayload, nwkPayloadLen);
								nwkFramePtr->nwkDataFrame.routedOrRetransmitFrame = true;
								/* End device address as next hop Address */
								nwkFramePtr->nwkDataFrame.nextHopAddr = myShortAddress + loopIndex;
								nwkFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->indirectDataWaitInterval + 1U;
								nwkFramePtr->nwkDataFrame.retry = 0U;
                                qmm_queue_append(&indirectFrameQueue, buffer_header);
							}
						}
					}
				}
			}
		}
		if (!rebroadcastedFrame)
#endif
		{
			if ((1U == meshHeaderRcv.frameControl.ackRequest) && (myShortAddress == meshHeaderRcv.dstAddr) && (MESH_BROADCAST_TO_COORDINATORS > meshHeaderRcv.dstAddr))
			{
				sendNwkAck(macSrcAddrLen, macSrcAddr, meshHeaderRcv.srcAddr, meshHeaderRcv.sequenceNumber);
			}


			/* Dispatch command frames based on its function */
			if (NWK_FRAMETYPE_COMMAND == (meshHeaderRcv.frameControl).frameType)
			{
				uint8_t commandIdMask;
				commandIdMask = *nwkPayload & CMD_MASK;
				switch (commandIdMask)
				{
					case CMD_JOIN:
					{
						handleJoinMessage(&meshHeaderRcv, macSrcAddrLen, macSrcAddr, nwkPayload, macRxPacket->LQIValue);
						break;
					}
#ifndef ENDDEVICE
					case CMD_ROUTE:
					{
						if (IN_NETWORK_STATE == meshCurrentState)
						{
							rebroadcast = handleRouteMessage(&meshHeaderRcv, macSrcAddrLen, macSrcAddr, nwkPayload, macRxPacket->LQIValue);
						}
						break;
					}

		            case CMD_COMM:
		            {
		                handleCommissiongMessage(&meshHeaderRcv, nwkPayload);
		                break;
		            }
#endif
		            case CMD_DATA:
		            {
			            handleDataMessage(&meshHeaderRcv, nwkPayload);
			            break;
		            }
#if defined(ENABLE_FREQUENCY_AGILITY)
                    case CMD_FREQ_AGILITY:
                    {
                        handleFreqAgilityMessage(&meshHeaderRcv, nwkPayload);
                        break;
                    }
#endif
		            default:
		            {
		                //printf("Invalid command ID...\r\n");
		                break;
                    }
		        }
		    }
			if (((NWK_FRAMETYPE_DATA == (meshHeaderRcv.frameControl).frameType) || (NWK_FRAMETYPE_MANU_SPEC == (meshHeaderRcv.frameControl).frameType))
			   && (IN_NETWORK_STATE == meshCurrentState || IN_NETWORK_PARTIAL_STATE == meshCurrentState))
			{
				bool sendIndication = true;
				if (MESH_BROADCAST_TO_COORDINATORS <= meshHeaderRcv.dstAddr)
				{
#ifdef ENDDEVICE
					API_UINT16_UNION macSrcShortAddr;
					macSrcShortAddr.v[0] = macSrcAddr[0];
					macSrcShortAddr.v[1] = macSrcAddr[1];
#endif
					if (((myShortAddress & ENDDEVICE_MASK) != 0U) && (RXON_ENDEVICE_ADDRESS_MASK != (myShortAddress & RXON_ENDEVICE_ADDRESS_MASK)) 
						&& MESH_BROADCAST_TO_FFDS >= meshHeaderRcv.dstAddr)
					{
						sendIndication = false;
					}
					else if (((myShortAddress & RXON_ENDEVICE_ADDRESS_MASK) != 0U) && (MESH_BROADCAST_TO_COORDINATORS == meshHeaderRcv.dstAddr))
					{
						sendIndication = false;
					}
#ifdef ENDDEVICE
					else if ((0U == (myShortAddress & RXON_ENDEVICE_ADDRESS_MASK)) && (macSrcShortAddr.Val != (myShortAddress & COORD_MASK)))
					{
						sendIndication = false;
					}
                    else
                    {
                        //do nothing
                    }
#endif
				}
				else 
				{
					if (checkDuplicateRejectionTableEntry(meshHeaderRcv.srcAddr, meshHeaderRcv.sequenceNumber))
					{
						sendIndication = false;
					}
					else
					{
						addDuplicateRejectionTableEntry(meshHeaderRcv.srcAddr, meshHeaderRcv.sequenceNumber);
					}
				}
				
				if (true == sendIndication)
				{
					// send indication to app
					RECEIVED_MESH_MESSAGE meshDataInd;
					meshDataInd.sourceAddress = meshHeaderRcv.srcAddr;
					meshDataInd.payloadSize = nwkPayloadLen;
					meshDataInd.payload = nwkPayload;
					meshDataInd.packetLQI = macRxPacket->LQIValue;
					meshDataInd.packetRSSI = macRxPacket->RSSIValue;
					if (NWK_FRAMETYPE_DATA == (meshHeaderRcv.frameControl).frameType)
					{
						if (NULL != pktRxcallback)
							pktRxcallback(&meshDataInd);
					}
					else if(NWK_FRAMETYPE_MANU_SPEC == (meshHeaderRcv.frameControl).frameType)
					{
						if (NULL != pktManuSpecRxcallback)
						   pktManuSpecRxcallback(&meshDataInd);
						
					}
					else
					{
						return;
					}
				}
			}
		}
	}
#ifndef ENDDEVICE
	else if ((myShortAddress == (meshHeaderRcv.dstAddr & COORD_MASK)) && IN_NETWORK_STATE == meshCurrentState)
	{
		// Data for my end device
		if (RXONWHENIDLE_ED_ADDRESS_MASK & meshHeaderRcv.dstAddr)
		{
			uint8_t* dataPtr = NULL;
			uint8_t dataLen = 0, headerLen = 0;;
			MeshFrameHeader_t meshHeader;
            buffer_t *buffer_header = NULL;  
            buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
		    if (NULL == buffer_header)
		    {
			    return;
		    }
            dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
		    if (NULL == dataPtr)
		    {
			    return;
		    }
			prepareGenericHeader(meshHeaderRcv.hops, meshHeaderRcv.srcAddr, meshHeaderRcv.dstAddr, &meshHeader);
			meshHeader.frameControl = meshHeaderRcv.frameControl;
			headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

			dataPtr[2] = meshHeaderRcv.sequenceNumber;

			//Fill payload
			memcpy(&dataPtr[dataLen], nwkPayload, nwkPayloadLen);
			dataLen += nwkPayloadLen;
			if(!(frameTransmit(&meshHeader, headerLen, dataLen - headerLen, dataPtr, 2, (uint8_t *)&meshHeaderRcv.dstAddr, 0, memFreeConfcb,buffer_header)))
            {           
            bmm_buffer_free(buffer_header);		
            }
        }
		else
		{
			//post in indirect queue
			NwkFrame_t *nwkFramePtr = NULL;
            buffer_t *buffer_header = NULL;
            buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
            if (NULL == buffer_header) 
            {
		        /* Buffer is not available */
		        return;
	        }
            nwkFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);

			if (NULL == nwkFramePtr)
			{
			    return;
			}

			memcpy(&(nwkFramePtr->nwkDataFrame.meshFrame.header), &meshHeaderRcv, sizeof(MeshFrameHeader_t));
			nwkFramePtr->nwkDataFrame.meshFrame.payloadLen = nwkPayloadLen;
			nwkFramePtr->nwkDataFrame.confCallback = NULL;
			nwkFramePtr->nwkDataFrame.nextHopAddr = meshHeaderRcv.dstAddr;
			memcpy(nwkFramePtr->nwkDataFrame.meshFrame.payload, nwkPayload, nwkPayloadLen);
			nwkFramePtr->nwkDataFrame.routedOrRetransmitFrame = true;
			nwkFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->indirectDataWaitInterval + 1;
            qmm_queue_append(&indirectFrameQueue, buffer_header);
		}
	}
	else if ((myShortAddress != meshHeaderRcv.dstAddr) && (0 != meshHeaderRcv.hops) && IN_NETWORK_STATE == meshCurrentState)
	{
		//Route the packet
		uint8_t* dataPtr = NULL;
		uint8_t dataLen = 0, headerLen = 0;;
		API_UINT16_UNION dstAddr;
		API_UINT16_UNION nextHopAddr;
		
		if (meshHeaderRcv.srcAddr == myShortAddress)
		{
			/* if nwk src addr is my address do not route as it will form a circular route */
			return;
		}
		
		dstAddr.Val = meshHeaderRcv.dstAddr;

		--meshHeaderRcv.hops;
        nextHopAddr.Val = getNextHopAddr(dstAddr.Val);
		if(DEFAULT_NEXT_HOP_ADDR == nextHopAddr.Val)
		{
			NwkFrame_t *nwkFramePtr = NULL;
            buffer_t *buffer_header = NULL;
            buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
            if (NULL == buffer_header) 
            {
		        /* Buffer is not available */
		        return;
	        }
            nwkFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
			if (NULL == nwkFramePtr)
			{
			    return;
			}
			memcpy(&(nwkFramePtr->nwkDataFrame.meshFrame.header), &meshHeaderRcv, sizeof(MeshFrameHeader_t));
			nwkFramePtr->nwkDataFrame.meshFrame.payloadLen = nwkPayloadLen;
			nwkFramePtr->nwkDataFrame.confCallback = NULL;
			memcpy(nwkFramePtr->nwkDataFrame.meshFrame.payload, nwkPayload, nwkPayloadLen);
			nwkFramePtr->nwkDataFrame.routedOrRetransmitFrame = true;

			if (false == initiateRouteReq(meshHeaderRcv.dstAddr))
			{
                bmm_buffer_free(buffer_header);
				return;
			}
			nwkFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->routeReqWaitInterval + 1;
            qmm_queue_append(&routeFrameQueue, buffer_header);
			return;
		}
        buffer_t *buffer_header_dataptr = NULL;
        buffer_header_dataptr =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	    if (NULL == buffer_header_dataptr)
	    {
		    return;
        }
        dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header_dataptr);
	    if (NULL == dataPtr)
	    {
		    return;
	    }

		headerLen = dataLen = generalFrameConstruct(&meshHeaderRcv, dataPtr);
		dataPtr[2] = meshHeaderRcv.sequenceNumber;

		//Fill payload of the command
		memcpy(&dataPtr[dataLen], nwkPayload, nwkPayloadLen);
		dataLen += nwkPayloadLen;
		if (false == frameTransmit(&meshHeaderRcv, headerLen, dataLen - headerLen, dataPtr, 2, (uint8_t *)&(nextHopAddr.Val), 0, memFreeConfcb, buffer_header_dataptr))
		{
            bmm_buffer_free(buffer_header_dataptr);
		}
	}

	if (rebroadcast && (0 != meshHeaderRcv.hops) && (IN_NETWORK_STATE == meshCurrentState))
	{
		//Rebroadcast the packet
		uint8_t* dataPtr = NULL;
		uint8_t dataLen = 0, headerLen = 0;
		API_UINT16_UNION dstAddr;
		dstAddr.Val = MAC_BROADCAST_ADDR;
        buffer_t *buffer_header = NULL;
        buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
		if (NULL == buffer_header)
		{
			return;
		}
        dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataPtr)
		{
			return;
		}
		meshHeaderRcv.hops--;
		headerLen = dataLen = generalFrameConstruct(&meshHeaderRcv, dataPtr);
		dataPtr[2] = meshHeaderRcv.sequenceNumber;

		if(!addRebroadcastTableEntry(meshHeaderRcv.srcAddr, meshHeaderRcv.sequenceNumber))
		{
            bmm_buffer_free(buffer_header);
			return;
		}
		//Fill payload of the command
		memcpy(&dataPtr[dataLen], nwkPayload, macRxPacket->PayloadLen - parsedLen);
		dataLen += macRxPacket->PayloadLen - parsedLen;
		if(false == frameTransmit(&meshHeaderRcv, headerLen, dataLen - headerLen, dataPtr, 2, (uint8_t *)&dstAddr.Val, 0, memFreeConfcb, buffer_header))
		{
            bmm_buffer_free(buffer_header);
		}
	}
#endif
}

/******************************************************************************
* Function:
*      bool sendDataFrame(NwkFrame_t *nwkFrame, uint16_t nextHopAddr,
*                         DataConf_callback_t confCallback)
*
* Summary:
*      This function sends the data frame to next hop coordinator
*
* Parameters:
*      NwkFrame_t *nwkFrame - Pointer of the Nwk frame to be sent
*      uint16_t nextHopAddr - Next hop address to be sent
*      DataConf_callback_t confCallback - Callback to be called after sending
*
* Returns:
*      None
******************************************************************************/
bool sendDataFrame(NwkFrame_t *nwkFrame, uint16_t nextHopAddr, DataConf_callback_t confCallback)
{
	uint8_t* dataPtr = NULL;
	uint8_t dataLen = 0, headerLen = 0;
	MeshFrameHeader_t meshHeader;
    buffer_t *buffer_header = NULL;
    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return false;
	}
	if (myShortAddress == nextHopAddr)
    {
		return false;
    }

	if (NULL == nwkFrame)
    {
		return false;
    }

    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
	if (NULL == dataPtr)
	{
		return false;
	}

	prepareGenericHeader(0xFF, nwkFrame->nwkDataFrame.meshFrame.header.srcAddr, nwkFrame->nwkDataFrame.meshFrame.header.dstAddr, &meshHeader);
	meshHeader.frameControl.frameType = nwkFrame->nwkDataFrame.meshFrame.header.frameControl.frameType;
	meshHeader.frameControl.ackRequest = nwkFrame->nwkDataFrame.meshFrame.header.frameControl.ackRequest;
#ifndef ENDDEVICE
    //if child end device, set data pending bit to 1
    if (((ENDDEVICE_MASK & nwkFrame->nwkDataFrame.meshFrame.header.dstAddr) && (myShortAddress == (nwkFrame->nwkDataFrame.meshFrame.header.dstAddr & COORD_MASK))) || ((MESH_BROADCAST_TO_ALL == nwkFrame->nwkDataFrame.meshFrame.header.dstAddr) && (ENDDEVICE_MASK & nwkFrame->nwkDataFrame.nextHopAddr) && (myShortAddress == (nwkFrame->nwkDataFrame.nextHopAddr & COORD_MASK))))
    {
        if(indirectFrameQueue.size)
        {
        buffer_t *buffer_header = NULL;
        buffer_header = qmm_queue_read(&indirectFrameQueue, NULL);
        NwkFrame_t *indirectdataFramePtr = (NwkFrame_t *) BMM_BUFFER_POINTER(buffer_header);
        for (uint8_t loopIndex = 0; loopIndex < indirectFrameQueue.size; loopIndex++)
        {
            if (NULL != indirectdataFramePtr)
            {
               if ((nwkFrame->nwkDataFrame.meshFrame.header.dstAddr == indirectdataFramePtr->nwkDataFrame.meshFrame.header.dstAddr) || ((MESH_BROADCAST_TO_ALL == indirectdataFramePtr->nwkDataFrame.meshFrame.header.dstAddr) && (nextHopAddr == indirectdataFramePtr->nwkDataFrame.nextHopAddr)))
                {
                    meshHeader.frameControl.dataPending = 1;
                }
            }
            indirectdataFramePtr = (NwkFrame_t *)indirectdataFramePtr->nextFrame;
        }
    }
    }
#endif

	headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

	if (nwkFrame->nwkDataFrame.routedOrRetransmitFrame)
	{
		 dataPtr[2] = nwkFrame->nwkDataFrame.meshFrame.header.sequenceNumber;
		 --meshSequenceNumber;
	}
	else
	{
		nwkFrame->nwkDataFrame.meshFrame.header.sequenceNumber = dataPtr[2];
	}

	//Fill payload of the command
	memcpy(&dataPtr[dataLen], nwkFrame->nwkDataFrame.meshFrame.payload,	nwkFrame->nwkDataFrame.meshFrame.payloadLen);
	dataLen += nwkFrame->nwkDataFrame.meshFrame.payloadLen;

#ifndef ENDDEVICE
	if (MAC_BROADCAST_ADDR == nextHopAddr)
	{
		if (!addRebroadcastTableEntry(myShortAddress, dataPtr[2]))
		{
            bmm_buffer_free(buffer_header);
			return false;
		}
	}
#endif
    
	if (false == frameTransmit(&meshHeader, headerLen, dataLen - headerLen, dataPtr, 2, (uint8_t *)&nextHopAddr,nwkFrame->nwkDataFrame.msghandle, confCallback, buffer_header))
	{
        bmm_buffer_free(buffer_header);
        return false;
	}
	return true;
}

#ifndef ENDDEVICE
/******************************************************************************
* Function:
*      bool checkRebroadcastTableEntry(uint16_t srcAddr, uint8_t seqNo)
*
* Summary:
*      This function checks for existing entry in Rebroadcast table
*
* Parameters:
*      uint16_t srcAddr - Source address of the broadcasted frame initiator
*      uint8_t seqNum - Sequence number of the broadcasted frame
*
* Returns:
*      True if available, false otherwise
******************************************************************************/
static bool checkRebroadcastTableEntry(uint16_t srcAddr, uint8_t seqNo)
{
	uint8_t loopIndex;

	for (loopIndex = 0; loopIndex < miwiDefaultRomOrRamParams->rebroadcastTableSize; loopIndex++)
	{
		if (miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSrcAddr == srcAddr && miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSeqNo == seqNo)
		{
			return true;
		}
	}
	return false;
}

/******************************************************************************
* Function:
*      bool addRebroadcastTableEntry(uint16_t srcAddr, uint8_t seqNum)
*
* Summary:
*      This function adds an entry in Rebroadcast table
*
* Parameters:
*      uint16_t srcAddr - Source address of the broadcasted frame initiator
*      uint8_t seqNum - Sequence number of the broadcasted frame
*
* Returns:
*      None
******************************************************************************/
bool addRebroadcastTableEntry(uint16_t srcAddr, uint8_t seqNum)
{
	uint8_t loopIndex;

	for (loopIndex = 0; loopIndex < miwiDefaultRomOrRamParams->rebroadcastTableSize; loopIndex++)
	{
		if (miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSrcAddr == 0xFFFF)
		{
			miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSrcAddr = srcAddr;
			miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSeqNo = seqNum;
			miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].timeout = miwiDefaultRomOrRamParams->rebroadcastTimeout;
			return true;
		}
	}
	return false;
}

/******************************************************************************
* Function:
*      void initRebroadcastTable(void)
*
* Summary:
*      This function initializes the Rebroadcast table
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
void initRebroadcastTable(void)
{
	uint8_t loopIndex;

	for (loopIndex = 0; loopIndex < miwiDefaultRomOrRamParams->rebroadcastTableSize; loopIndex++)
	{
		miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSrcAddr = 0xFFFF;
	}
}

/******************************************************************************
* Function:
*      void rebroadcastTimerHandler(void)
*
* Summary:
*      This function handles Rebroadcast Timers at 1sec periodicity
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
void rebroadcastTimerHandler(void)
{
	uint8_t loopIndex;

	for (loopIndex = 0; loopIndex < miwiDefaultRomOrRamParams->rebroadcastTableSize; loopIndex++)
	{
		if ((0xFFFF != miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSrcAddr)
			&& (0 != miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].timeout)
			&& (0 == (--miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].timeout)))
		{
			miwiDefaultRomOrRamParams->rebroadcastTable[loopIndex].nwkSrcAddr = 0xFFFF;
		}
	}
}
#endif

/************************************************************************************
* Function:
*      void nwkAckConfCb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function frees the memory allocated for nwkAck
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void nwkAckConfCb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
    (void)handle;
    (void)status;
    (void)msgPointer;
}

/******************************************************************************
* Function:
*      static void sendNwkAck(uint8_t srcAddrLen, uint8_t *srcAddr,
*                             uint16_t dstAddr, uint8_t seqNum)
*
* Summary:
*      This function sends a Network Ack for the data
*
* Parameters:
*      uint8_t srcAddrLen - Length of source address
*      uint8_t *srcAddr - Pointer for Srource address
*      uint16_t dstAddr - Destination address to send Ack
*      uint8_t seqNum - Sequence number for which Ack to be sent
*
* Returns:
*      None
******************************************************************************/
static void sendNwkAck(uint8_t srcAddrLen, uint8_t *srcAddr, uint16_t dstAddr, uint8_t seqNum)
{
	uint8_t* dataPtr = NULL;
	uint8_t dataLen = 0, headerLen = 0;
	MeshFrameHeader_t meshHeader;
	prepareGenericHeader(MAX_HOP, myShortAddress, dstAddr, &meshHeader);
	meshHeader.sequenceNumber = seqNum;
	meshHeader.frameControl.frameType = NWK_FRAMETYPE_COMMAND;
	meshHeader.frameControl.ackRequest = 0;

#ifndef ENDDEVICE
	if ((ENDDEVICE_MASK & dstAddr) && (myShortAddress == (dstAddr & COORD_MASK)) && !(RXONWHENIDLE_ED_ADDRESS_MASK & dstAddr))
	{
		// For RX OFF device - post in indirect queue
		NwkFrame_t *nwkFramePtr = NULL;
		/* Payload of the command */
		uint8_t payloadAck = CMD_MESH_ACK;
        buffer_t *buffer_header = NULL;
        buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
        if (NULL == buffer_header) 
        {
        /* Buffer is not available */
	        return;
        }
        nwkFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == nwkFramePtr)
		{
		    return;
		}
		memcpy(&(nwkFramePtr->nwkDataFrame.meshFrame.header), &meshHeader, sizeof(MeshFrameHeader_t));
		nwkFramePtr->nwkDataFrame.meshFrame.payloadLen = 1;
		nwkFramePtr->nwkDataFrame.confCallback = NULL;
		memcpy(nwkFramePtr->nwkDataFrame.meshFrame.payload, &payloadAck, nwkFramePtr->nwkDataFrame.meshFrame.payloadLen);
		nwkFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->indirectDataWaitInterval + 1;
		nwkFramePtr->nwkDataFrame.retry = 0;
		nwkFramePtr->nwkDataFrame.routedOrRetransmitFrame = true;
        qmm_queue_append(&indirectFrameQueue, buffer_header); 
	}
	else
#endif
	{
        buffer_t *buffer_header = NULL;
        buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE); 
		if (NULL == buffer_header)
		{
			return;
		}
        dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataPtr)
		{
			return;
		}
		headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);
        --meshSequenceNumber;

		//Fill payload of the command
		dataPtr[dataLen++] = (uint8_t)CMD_MESH_ACK;

       if(!(frameTransmit(&meshHeader, headerLen, dataLen - headerLen, dataPtr,	srcAddrLen, srcAddr, 0,	nwkAckConfCb, buffer_header)))
       {
            bmm_buffer_free(buffer_header);
       }
	}
}

/******************************************************************************
* Function:
*      void handleDataMessage(MeshFrameHeader_t *meshHeader, uint8_t* payload)
*
* Summary:
*      This function handles the data messages
*
* Parameters:
*      MeshFrameHeader_t *meshHeader - Pointer of the Mesh header
*      uint8_t* nwkPayload - Network payload of the received frame
*
* Returns:
*      None
******************************************************************************/
void handleDataMessage(MeshFrameHeader_t *meshHeader, uint8_t* nwkPayload)
{
	switch ((MeshCmdIdentifier_t)nwkPayload[0])
	{
#ifndef ENDDEVICE
		case CMD_MESH_DATA_REQUEST:
		{
            uint8_t shortAddressIndex = (uint8_t)(meshHeader->srcAddr & ENDEVICE_ADDRESS_MASK);

			/* If invalid ieee address found in the short address index of sleep device table,
             then send leave command to the end device so that it can rejoin for proper communication */
            if (false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressIndex].ieeeaddr))
			{
				sendForceLeaveNetwork(meshHeader->srcAddr);
				return;
			}

			/*Reload the actual timeout since data request is received from End device */
			miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressIndex].currentTimeOut = miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressIndex].actualTimeOut;

			//send data from indirect queue
			if (indirectFrameQueue.size && myShortAddress == (meshHeader->srcAddr & COORDINATOR_ADDRESS_MASK))
			{
				NwkFrame_t *dataFramePtr = NULL;
                buffer_t *buffer_header = NULL;
				uint8_t loopIndex;
				uint8_t numberOfElementsinQueue = indirectFrameQueue.size;
				for (loopIndex = 0; loopIndex < numberOfElementsinQueue; loopIndex++)
				{
//					dataFramePtr = (NwkFrame_t *) miQueueRemove(&indirectFrameQueue, NULL);
                    if(indirectFrameQueue.size > 0)
                    {
                    buffer_header =  qmm_queue_remove(&indirectFrameQueue, NULL);
                    if(buffer_header == NULL)
                    {
                        return;
                    }
                    dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
                    }
					if (NULL == dataFramePtr)
					{
						return;
					}

					//check the received data request and send data
					if ((meshHeader->srcAddr == dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr) ||
					    ((MESH_BROADCAST_TO_ALL == dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr) && (meshHeader->srcAddr == dataFramePtr->nwkDataFrame.nextHopAddr)))
					{
						if (dataFramePtr->nwkDataFrame.meshFrame.header.frameControl.ackRequest && dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr == myShortAddress)
						{
							dataFramePtr->nwkDataFrame.timeout = calculateAckWaitTimeout(miwiDefaultRomOrRamParams->numOfCoordinators + 1, dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
							dataFramePtr->nwkDataFrame.retry = miwiDefaultRomOrRamParams->frameRetry + 1;
							if (!sendDataFrame(dataFramePtr, meshHeader->srcAddr,ackReqDataConfcb))
							{
								dataFramePtr->nwkDataFrame.retry -= 1;
							}
                            qmm_queue_append(&ackWaitFrameQueue, buffer_header);
						}
						else
						{
							if(sendDataFrame(dataFramePtr, meshHeader->srcAddr,nonAckDataCallback))
							{
								dataFramePtr->nwkDataFrame.retry = 0;
                                qmm_queue_append(&nonAckFrameQueue, buffer_header);
							}
							else
							{
								if (NULL != dataFramePtr->nwkDataFrame.confCallback)
								{
									dataFramePtr->nwkDataFrame.confCallback(dataFramePtr->nwkDataFrame.msghandle, FAILURE, (uint8_t *)dataFramePtr);
								}
                                bmm_buffer_free(buffer_header);
								return;
							}
						}
					}
					else
					{
                        qmm_queue_append(&indirectFrameQueue, buffer_header);
					}
				}
			}
			break;
		}
#endif
		case CMD_MESH_ACK:
		{
			NwkFrame_t *dataFramePtr = NULL;
            buffer_t *buffer_header = NULL;
			uint8_t loopIndex;

			for (loopIndex = 0; loopIndex < ackWaitFrameQueue.size; loopIndex++)
			{
//				dataFramePtr = (NwkFrame_t *) miQueueRemove(&ackWaitFrameQueue, NULL);//michp
                buffer_header =  qmm_queue_remove(&ackWaitFrameQueue, NULL);
                dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);

				if (NULL == dataFramePtr)
				{
					return;
				}

				//check the received ack and send confirmation
				if (meshHeader->sequenceNumber == dataFramePtr->nwkDataFrame.meshFrame.header.sequenceNumber &&
						meshHeader->srcAddr == dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr)
				{
					DataConf_callback_t callback = dataFramePtr->nwkDataFrame.confCallback;
					if (NULL != callback)
					{
						callback(dataFramePtr->nwkDataFrame.msghandle, SUCCESS, (uint8_t *)dataFramePtr);
					}

                    bmm_buffer_free(buffer_header);
#ifndef ENDDEVICE
					{
						uint16_t dstAddr = dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr;
						if ((!(dstAddr & ENDDEVICE_MASK)) && ((dstAddr & COORD_MASK) != myShortAddress))
						{
							uint8_t coordIndex = dstAddr >> 8;
							miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score = DEFAULT_SCORE;
						}
					}
#endif                
				}
				else
				{
                    qmm_queue_append(&ackWaitFrameQueue, buffer_header);
				}
			}
			break;
		}
        
        default:
            //Handle exceptions if any
            break;
	}
}

#ifndef ENDDEVICE
void indirectDataTimerHandler(void)
{
	NwkFrame_t *dataFramePtr = NULL;
	uint8_t loopIndex;
    buffer_t *buffer_header = NULL;
	for (loopIndex = 0U; loopIndex < indirectFrameQueue.size; loopIndex++)
	{
        buffer_header =  qmm_queue_remove(&indirectFrameQueue, NULL);
        if(buffer_header == NULL)
        {
            return;
        }
        dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataFramePtr)
		{
			return;
		}

		if ((0U != dataFramePtr->nwkDataFrame.timeout) &&
			(--dataFramePtr->nwkDataFrame.timeout) == 0U)
		{
			if (NULL != dataFramePtr->nwkDataFrame.confCallback)
			{
				DataConf_callback_t callback = dataFramePtr->nwkDataFrame.confCallback;
				if (NULL != callback)
				{
					callback(dataFramePtr->nwkDataFrame.msghandle, TRANSACTION_EXPIRED, (uint8_t *)dataFramePtr);
				}
			}

            bmm_buffer_free(buffer_header);
		}
		else
		{
            qmm_queue_append(&indirectFrameQueue, buffer_header);
		}        
	}
}
#endif

#if defined(ENABLE_FREQUENCY_AGILITY)
extern bool txCallbackReceived;
void channelUpdateTimerExpired(uintptr_t context)
{
#ifndef ENDDEVICE
    uint8_t loopIndex;
#ifdef PAN_COORDINATOR
    for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
    {
        if (false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->coordTable[loopIndex].ieeeaddr))
        {
             miwiDefaultRomOrRamParams->coordTable[loopIndex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveCoordTimeoutSec;
        }
    }
#endif

    for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; loopIndex++)
    {
        if (isCorrectIeeeAddr(miwiDefaultRomOrRamParams->devTable[loopIndex].ieeeaddr))
        {
             miwiDefaultRomOrRamParams->devTable[loopIndex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveRxOnEdTimeoutSec;
        }
    }
    for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
    {
        if (isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr))
        {
            miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].currentTimeOut = miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].actualTimeOut;
        }
    }
#endif

	if (newChannelToUpdate != 0xFFU)
	{
        APP_Msg_T *p_appModes;
       APP_Msg_T appModes;
       p_appModes = &appModes;
		MiApp_Set(CHANNEL, &newChannelToUpdate);
        txCallbackReceived = true;
		newChannelToUpdate = 0xFF;
		#if defined(ENABLE_NETWORK_FREEZER)
		/* update Channel update with 0xFF in Persistent Data Server */
		PDS_Store(PDS_CHANNEL_UPDATE_ID);
		#endif

#ifdef COORDINATOR
        appStates = APP_STATE_SEND;
        p_appModes->msgId = APP_STATE_SEND;
        OSAL_QUEUE_Send(&appData.appQueue, p_appModes, 0U);
#endif
#ifdef ENDDEVICE
		edInPollingState = false;
		SYS_TIME_TimerStop(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle);
        SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle);
		if ((meshCurrentState != IN_NETWORK_STATE) && (meshCurrentState != IN_NETWORK_PARTIAL_STATE))
		{
			/* In case of End device, try to rejoin using establish connection procedure */
			MiApp_EstablishConnection(currentChannel, SHORT_ADDR_LEN, (uint8_t *)&backupParentNwkAddress, gCapabilityInfo, connectionConfirm);
		}
		else
		{
			sendPollRequest();
		}
		edLinkFailureAttempts = ED_LINK_FAILURE_ATTEMPTS;
#endif
	}
    (void)context;
}

static void channelUpdateReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer)
{
	if (SUCCESS == confstatus)
	{
		#if defined(ENABLE_NETWORK_FREEZER)
		/*Store new channel in Persistent Data Server */
		PDS_Store(PDS_CHANNEL_UPDATE_ID);
		#endif
#ifdef ENDDEVICE
		edLinkFailureAttempts += 10U;
#endif
		/* Start the timer for channel update time */
		timerHandles.joinTimerchannelUpdate.handler = channelUpdateTimerExpired;
		timerHandles.joinTimerchannelUpdate.timeout = (CHANNEL_UPDATE_TIME_IN_SEC * 1000);
		timerHandles.joinTimerchannelUpdate.interval = (CHANNEL_UPDATE_TIME_IN_SEC * 1000);
		timerHandles.joinTimerchannelUpdate.mode = SYS_TIME_SINGLE;
        
        uint8_t myData = 0U;
        timerHandles.joinTimerchannelUpdateHandle = SYS_TIME_CallbackRegisterMS(&channelUpdateTimerExpired, (uintptr_t)&myData, timerHandles.joinTimerchannelUpdate.interval, SYS_TIME_SINGLE);
        if(timerHandles.joinTimerchannelUpdateHandle == SYS_TIME_HANDLE_INVALID)
        {
           return;	
        }
	}
}

bool MiApp_InitChannelHopping( uint32_t ChannelMap, uint8_t optimalChannel)
{
	uint8_t *dataPtr = NULL;
	uint16_t broadcastAddr = MAC_BROADCAST_ADDR;
	NwkFrame_t *nwkFramePtr = NULL;
    buffer_t *buffer_header = NULL;
    buffer_t *buffer_header_dataptr = NULL;

//	channelMapSupported = MiMAC_GetPHYChannelInfo();
//
//	/* Check the given channel is within the range and callback is not NULL*/
//	if (!(channelMapSupported & ChannelMap))
//        return false;
//
//	optimalChannel = MiApp_NoiseDetection(ChannelMap, 3, NOISE_DETECT_ENERGY, &RSSIValue);

	MiApp_Set(CHANNEL, &backupChannel);
	if ( optimalChannel == backupChannel )
	{
		return false;
	}
//    MiApp_Set(CHANNEL, &optimalChannel);
    buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);

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
    buffer_header_dataptr =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return false;
	}
    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header_dataptr);
	if (NULL == dataPtr)
	{ 
		return false;
	}

	newChannelToUpdate = optimalChannel;
	dataPtr[0] = CMD_MESH_CHANNEL_UPDATE;
	dataPtr[1] = optimalChannel;

	nwkFramePtr->nwkDataFrame.meshFrame.header.frameControl.frameType = NWK_FRAMETYPE_COMMAND;
	nwkFramePtr->nwkDataFrame.meshFrame.header.dstAddr = broadcastAddr;
	nwkFramePtr->nwkDataFrame.meshFrame.header.srcAddr = myShortAddress;
	nwkFramePtr->nwkDataFrame.meshFrame.header.frameControl.ackRequest = 0;
	nwkFramePtr->nwkDataFrame.meshFrame.payloadLen = 2U;
	nwkFramePtr->nwkDataFrame.msghandle = 0xA5U;
	nwkFramePtr->nwkDataFrame.routedOrRetransmitFrame = false;
	nwkFramePtr->nwkDataFrame.confCallback = channelUpdateReqConfcb;
	memcpy(nwkFramePtr->nwkDataFrame.meshFrame.payload, dataPtr, nwkFramePtr->nwkDataFrame.meshFrame.payloadLen);

#ifndef ENDDEVICE
	for (uint8_t loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
	{
		if (true == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr))
		{
			if (true)
			{
                buffer_t *buffer_header_indirectfptr = NULL;
                buffer_header_indirectfptr = bmm_buffer_alloc(LARGE_BUFFER_SIZE); 
                if (NULL == buffer_header_indirectfptr) 
                {
		          /* Buffer is not available */
		            return false;
	            }
                NwkFrame_t *indirectBroadcastFramePtr =  (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header_indirectfptr);

				if (NULL != indirectBroadcastFramePtr)
				{
					memcpy(indirectBroadcastFramePtr, nwkFramePtr, sizeof(NwkFrame_t));
					indirectBroadcastFramePtr->nwkDataFrame.timeout = miwiDefaultRomOrRamParams->indirectDataWaitInterval + 1;
					indirectBroadcastFramePtr->nwkDataFrame.retry = 0U;
					/* End device address as next hop Address */
					indirectBroadcastFramePtr->nwkDataFrame.nextHopAddr = myShortAddress + loopIndex;
					indirectBroadcastFramePtr->nwkDataFrame.confCallback = NULL;
                    qmm_queue_append(&indirectFrameQueue, buffer_header_indirectfptr);

				}
                else
                {     
		            return false;                    
                }
			}
		}
	}
#endif

	/* Initiate the frame transmission */
	if (!sendDataFrame(nwkFramePtr, MAC_BROADCAST_ADDR, nonAckDataCallback))
	{ 
    bmm_buffer_free(buffer_header);        
		return false;
	}

    qmm_queue_append(&nonAckFrameQueue, buffer_header);
    bmm_buffer_free(buffer_header_dataptr);
	return true;
}

/******************************************************************************
* Function:
*      void handleFreqAgilityMessage(MeshFrameHeader_t *meshHeader, uint8_t* payload)
*
* Summary:
*      This function handles the freq agility related messages
*
* Parameters:
*      MeshFrameHeader_t *meshHeader - Pointer of the Mesh header
*      uint8_t* nwkPayload - Network payload of the received frame
*
* Returns:
*      None
******************************************************************************/
void handleFreqAgilityMessage(MeshFrameHeader_t *meshHeader, uint8_t* nwkPayload)
{
	switch (nwkPayload[0])
	{
		case CMD_MESH_CHANNEL_UPDATE:
		{
			if (meshHeader->srcAddr == PAN_COORDINATOR_ADDRESS)
			{
				uint32_t channelMap = 0UL;
                
//				channelMap = MiMAC_GetPHYChannelInfo();
                newChannelToUpdate = 0U;
				/* Check the given channel is within the range */
//				if ((channelMap & (1UL << nwkPayload[1])) == 0)
//				return;

				newChannelToUpdate = nwkPayload[1] & 0x00ff;
#ifdef ENDDEVICE
				edLinkFailureAttempts += 10U;
#endif

				/* Start the timer for channel update time */
                timerHandles.joinTimerchannelUpdate.handler = channelUpdateTimerExpired;
	            timerHandles.joinTimerchannelUpdate.timeout = (CHANNEL_UPDATE_TIME_IN_SEC * 1000);
		        timerHandles.joinTimerchannelUpdate.interval = (CHANNEL_UPDATE_TIME_IN_SEC * 1000);
		        timerHandles.joinTimerchannelUpdate.mode = SYS_TIME_SINGLE;

                
                uint8_t myData = 0U;
         timerHandles.joinTimerchannelUpdateHandle = SYS_TIME_CallbackRegisterMS(&channelUpdateTimerExpired, (uintptr_t)&myData, timerHandles.joinTimerchannelUpdate.interval, SYS_TIME_SINGLE);
        if(timerHandles.joinTimerchannelUpdateHandle == SYS_TIME_HANDLE_INVALID)
        {
           return;	
        }

				#if defined(ENABLE_NETWORK_FREEZER)
				/*Store new channel in Persistent Data Server */
				PDS_Store(PDS_CHANNEL_UPDATE_ID);
				#endif
			}
		}
		break;

		default:
            //Handle exceptions if any
		    break;
	}
}
#endif

/******************************************************************************
* Function:
*      bool checkDuplicateRejectionTableEntry(uint16_t srcAddr, uint8_t seqNo)
*
* Summary:
*      This function checks for existing entry in Rebroadcast table
*
* Parameters:
*      uint16_t srcAddr - Source address of the data frame initiator
*      uint8_t seqNum - Sequence number of the data frame
*
* Returns:
*      True if available, false otherwise
******************************************************************************/
bool checkDuplicateRejectionTableEntry(uint16_t srcAddr, uint8_t seqNo)
{
	uint8_t loopIndex;

	for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->duplicateRejectionTableSize; loopIndex++)
	{
		if (miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].srcAddr == srcAddr && miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].seqNo == seqNo)
		{
			return true;
		}
	}
	return false;
}

/******************************************************************************
* Function:
*      bool addDuplicateRejectionTableEntry(uint16_t srcAddr, uint8_t seqNum)
*
* Summary:
*      This function adds an entry in Duplicate Rejection table
*
* Parameters:
*      uint16_t srcAddr - Source address of the data frame initiator
*      uint8_t seqNum - Sequence number of the data frame
*
* Returns:
*      None
******************************************************************************/
bool addDuplicateRejectionTableEntry(uint16_t srcAddr, uint8_t seqNum)
{
	uint8_t loopIndex;

	for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->duplicateRejectionTableSize; loopIndex++)
	{
		if (miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].srcAddr == 0xFFFFU)
		{
			miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].srcAddr = srcAddr;
			miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].seqNo = seqNum;
			/* + 1 as timeout is reduced and checked, + 1 to avoid immediate timeout and
			Total times the retry count as we may receive the first and the last retry only*/
			miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].timeout = (calculateAckWaitTimeout(miwiDefaultRomOrRamParams->numOfCoordinators + 1U, srcAddr) + 1U + 1U) * miwiDefaultRomOrRamParams->frameRetry;
			return true;
		}
	}
	return false;
}

/******************************************************************************
* Function:
*      void initDuplicateRejectionTable(void)
*
* Summary:
*      This function initializes the Rebroadcast table
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
void initDuplicateRejectionTable(void)
{
	uint8_t loopIndex;

	for (loopIndex = 0; loopIndex < miwiDefaultRomOrRamParams->duplicateRejectionTableSize; loopIndex++)
	{
		miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].srcAddr = 0xFFFF;
	}
}

/******************************************************************************
* Function:
*      void duplicateRejectionTimerHandler(void)
*
* Summary:
*      This function handles Rebroadcast Timers at 1sec periodicity
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
void duplicateRejectionTimerHandler(void)
{
	uint8_t loopIndex;

	for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->duplicateRejectionTableSize; loopIndex++)
	{
		if ((0xFFFFU != miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].srcAddr)
		&& (0U != miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].timeout)
		&& (0U == (--miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].timeout)))
		{
			miwiDefaultRomOrRamParams->duplicateRejectionTable[loopIndex].srcAddr = 0xFFFFU;
		}
	}
}
