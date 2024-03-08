/**
* \file  miwi_mesh_routing.c
*
* \brief MiWi Mesh Protocol Route Handling implementation
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
#if defined(PAN_COORDINATOR) || defined (COORDINATOR)


/************************ Static Prototypes **********************************/
static void sendRouteUpdate(void);
static void routeUpdateConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
static void routeReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
static void routeReplyConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
static void sendRouteReply(uint16_t dstAddr, uint8_t coordIndex, uint8_t hopCount);

/************************ Static Declarations **********************************/

uint8_t routeUpdateInterval;

queue_t routeFrameQueue;

#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
bool routeTestMode = false;
#endif

/******************************************************************************
* Function:
*      void initRouteTable(void)
*
* Summary:
*      This function initializes the route table and route queue
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
void initRouteTable(void)
{
	uint8_t loopIndex;

	for (loopIndex = 0; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
	{
		miwiDefaultRomOrRamParams->coordinatorRouteTable[loopIndex].nextHopAddr = NO_NEXT_HOP;
		miwiDefaultRomOrRamParams->coordinatorRouteTable[loopIndex].lqi = NO_LQI;
		miwiDefaultRomOrRamParams->coordinatorRouteTable[loopIndex].score = NO_SCORE;
	}

	memset(miwiDefaultRomOrRamParams->coordinatorHopCount, NO_HOP_COUNT, HOP_TABLE_COUNT);

    qmm_queue_init(&routeFrameQueue, QUEUE_CAPACITY);
	routeUpdateInterval = miwiDefaultRomOrRamParams->routeUpdateInterval;
}

/******************************************************************************
* Function:
*      bool handleRouteMessage(MeshFrameHeader_t *meshHeader, uint8_t
*      macSrcAddrLen, uint8_t* macSrcAddr, uint8_t* payload, uint8_t lqiValue)
*
* Summary:
*      This function handles route messages received from other coordinators
*
* Parameters:
*      MeshFrameHeader_t *meshHeader - Pointer of the Mesh header
*      uint8_t macSrcAddrLen - MAC address length of source
*      uint8_t* macSrcAddr - Pointer of MAC address of source
*      uint8_t* payload - Network payload of the received frame
*      uint8_t rcvdLqi - LQI of the next hop coordinator
*
* Returns:
*      True is packet need to be rebroadcast, false otherwise
******************************************************************************/
bool handleRouteMessage(MeshFrameHeader_t *meshHeader, uint8_t macSrcAddrLen,
						uint8_t* macSrcAddr, uint8_t* nwkPayload, uint8_t rcvdLqi)
{
	API_UINT16_UNION shortAddr;
	shortAddr.v[0] = macSrcAddr[0];
	shortAddr.v[1] = macSrcAddr[1];
	bool retStatus = false;

	if ((myShortAddress & ENDDEVICE_MASK) || (shortAddr.Val & ENDDEVICE_MASK) || (meshHeader->srcAddr & ENDDEVICE_MASK))
	{
		return retStatus;
	}

	switch (nwkPayload[0])
	{
	case CMD_MESH_ROUTE_TABLE_UPDATE:
	{
#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
		if (!routeTestMode)
#endif //MIWI_MESH_TOPOLOGY_SIMULATION_MODE
		{
			uint8_t index;
			uint8_t totalHopCount;
			uint8_t hopCount = nwkPayload[1];
			CoordHopCount_t *hopCountlist = (CoordHopCount_t *) (nwkPayload + 2);
			for(index = 0U; index < hopCount; index++)
			{
				if (0 != hopCountlist->msb)
				{
					totalHopCount = hopCountlist->msb + 1;
					if (totalHopCount > 0x0FU)
					{
						totalHopCount = 0x0FU;
					}
					if (0 == miwiDefaultRomOrRamParams->coordinatorHopCount[index].msb 
						|| totalHopCount < miwiDefaultRomOrRamParams->coordinatorHopCount[index].msb)
					{
						miwiDefaultRomOrRamParams->coordinatorHopCount[index].msb = totalHopCount;
						miwiDefaultRomOrRamParams->coordinatorRouteTable[index * 2].nextHopAddr = shortAddr.v[1];
						miwiDefaultRomOrRamParams->coordinatorRouteTable[index * 2].lqi = rcvdLqi >> 4;
						miwiDefaultRomOrRamParams->coordinatorRouteTable[index * 2].score = DEFAULT_SCORE;
					}
				}
				if (0 != hopCountlist->lsb)
				{
					totalHopCount = hopCountlist->lsb + 1;
					if (totalHopCount > 0x0FU)
					{
						totalHopCount = 0x0FU;
					}
					if (0 == miwiDefaultRomOrRamParams->coordinatorHopCount[index].lsb 
						|| totalHopCount < miwiDefaultRomOrRamParams->coordinatorHopCount[index].lsb)
					{
						miwiDefaultRomOrRamParams->coordinatorHopCount[index].lsb = totalHopCount;
						miwiDefaultRomOrRamParams->coordinatorRouteTable[(index * 2) + 1].nextHopAddr = shortAddr.v[1];
						miwiDefaultRomOrRamParams->coordinatorRouteTable[(index * 2) + 1].lqi = rcvdLqi >> 4;
						miwiDefaultRomOrRamParams->coordinatorRouteTable[(index * 2) + 1].score = DEFAULT_SCORE;
					}
				}
				hopCountlist++;
			}
			/* To add the device directly in route table */
			index = shortAddr.v[1];
			if (index % 2U)
			{
				miwiDefaultRomOrRamParams->coordinatorHopCount[index / 2].lsb = 1;
			}
			else
			{
				miwiDefaultRomOrRamParams->coordinatorHopCount[index / 2].msb = 1;
			}
			miwiDefaultRomOrRamParams->coordinatorRouteTable[index].nextHopAddr = shortAddr.v[1];
			miwiDefaultRomOrRamParams->coordinatorRouteTable[index].lqi = rcvdLqi >> 4;
			miwiDefaultRomOrRamParams->coordinatorRouteTable[index].score = DEFAULT_SCORE;
			
			/* Removing my index in Route table if updated thro sender's Route Update*/
			index = myShortAddress >> 8U;
			if (miwiDefaultRomOrRamParams->coordinatorRouteTable[index].nextHopAddr != NO_NEXT_HOP)
			{
				if (index % 2U)
				{
					miwiDefaultRomOrRamParams->coordinatorHopCount[index / 2].lsb = 0;
				}
				else
				{
					miwiDefaultRomOrRamParams->coordinatorHopCount[index / 2].msb = 0;
				}
				miwiDefaultRomOrRamParams->coordinatorRouteTable[index].nextHopAddr = NO_NEXT_HOP;
				miwiDefaultRomOrRamParams->coordinatorRouteTable[index].lqi = NO_LQI;
				miwiDefaultRomOrRamParams->coordinatorRouteTable[index].score = NO_SCORE;
			}
		}
		break;
	}
	case CMD_MESH_ROUTE_REQUEST:
	{
		API_UINT16_UNION routeReqAddr;
		routeReqAddr.v[0] = nwkPayload[1];
		routeReqAddr.v[1] = nwkPayload[2];
		uint8_t hopCount;
		hopCount = ((miwiDefaultRomOrRamParams->numOfCoordinators + 1) - meshHeader->hops) + 1;
		addRoute(meshHeader->srcAddr, shortAddr.Val, hopCount, rcvdLqi);
		if (miwiDefaultRomOrRamParams->numOfCoordinators > routeReqAddr.v[1])
		{
			hopCount = getHopCount(routeReqAddr.v[1]);
			/* Send Route reply if request address is my address or if valid hop count(i.e. not 0)*/
			if (myShortAddress == routeReqAddr.Val || NO_HOP_COUNT != hopCount)
			{
				sendRouteReply(meshHeader->srcAddr, routeReqAddr.v[1], hopCount);
			}
			else
			{
				retStatus = true;
			}
		}
		else
		{
			retStatus = true;
		}
		break;
	}
	case CMD_MESH_ROUTE_REPLY:
	{
#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
		if (!routeTestMode)
#endif //MIWI_MESH_TOPOLOGY_SIMULATION_MODE
		{
			API_UINT16_UNION routeAddr;
			routeAddr.v[0] = nwkPayload[1];
			routeAddr.v[1] = nwkPayload[2];
			uint8_t totalHopCounts = nwkPayload[3] + ((miwiDefaultRomOrRamParams->numOfCoordinators + 1) - meshHeader->hops) + 1;
			addRoute(routeAddr.Val, shortAddr.Val, totalHopCounts, rcvdLqi);
		}
		break;
	}
	default:
        //Handle exceptions if any
		break;
	}
	return retStatus;
}

/******************************************************************************
* Function:
*      bool addRoute(uint16_t coordAddr, uint16_t nextHopAddr,
*                    uint8_t noOfHops, uint8_t rcvdLqi)
*
* Summary:
*      This function adds a route for Coordinator after validating
*
* Parameters:
*      uint16_t coordAddr - Coordinator address to set next hop
*      uint16_t nextHopAddr - Next hop address to be set
*      uint8_t numOfHops - Number of hops to reach the coordinator
*      uint8_t rcvdLqi - Lqi of the next hop coordinator
*
* Returns:
*      True is added successfully, false otherwise
******************************************************************************/
bool addRoute(uint16_t coordAddr, uint16_t nextHopAddr, uint8_t numOfHops, uint8_t rcvdLqi)
{
	uint8_t coordIndex = coordAddr >> 8;
	uint8_t nextHopIndex = nextHopAddr >> 8;
	uint8_t currHopCount = 0;
	uint8_t lqiCost = rcvdLqi >> 4;
	if(miwiDefaultRomOrRamParams->numOfCoordinators <= coordIndex
		|| miwiDefaultRomOrRamParams->numOfCoordinators <= nextHopIndex
		|| ENDEVICE_ADDRESS_MASK & coordAddr)
	{
		return false;
	}
	if(MAX_HOP_COUNT < numOfHops)
	{
		numOfHops = MAX_HOP_COUNT;
	}
	currHopCount = getHopCount(coordIndex);
	if (NO_HOP_COUNT == currHopCount || numOfHops < currHopCount)
	{
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].nextHopAddr = nextHopIndex;
		setHopCount(coordIndex, numOfHops);
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].lqi = lqiCost;
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score = DEFAULT_SCORE;
		return true;
	}
	return false;
}

/******************************************************************************
* Function:
*      bool removeRoute(uint16_t coordAddr)
*
* Summary:
*      This function removes route for Coordinator Address by setting default
*
* Parameters:
*      uint16_t coordAddr - Coordinator address to set next hop
*
* Returns:
*      True is removed successfully, false otherwise
******************************************************************************/
bool removeRoute(uint16_t coordAddr)
{
#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
	if (!routeTestMode)
#endif //MIWI_MESH_TOPOLOGY_SIMULATION_MODE
	{
		uint8_t coordIndex = coordAddr >> 8U;
		if(miwiDefaultRomOrRamParams->numOfCoordinators <= coordIndex)
		{
			return false;
		}
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].nextHopAddr = NO_NEXT_HOP;
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].lqi = NO_LQI;
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score = NO_SCORE;
		setHopCount(coordIndex, NO_HOP_COUNT);
		return true;
	}
#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
	else
	{
		return false;
	}
#endif //MIWI_MESH_TOPOLOGY_SIMULATION_MODE
}

/******************************************************************************
* Function:
*      void setNextHopAddr(uint16_t coordAddr, uint16_t nextHopAddr)
*
* Summary:
*      This function sets the next hop address for Coordinator Address
*
* Parameters:
*      uint16_t coordAddr - Coordinator address to set next hop
*      uint16_t nextHopAddr - Next hop address to be set
*
* Returns:
*      None
******************************************************************************/
void setNextHopAddr(uint16_t coordAddr, uint16_t nextHopAddr)
{
	if (miwiDefaultRomOrRamParams->numOfCoordinators > (coordAddr >> 8))
	{
		miwiDefaultRomOrRamParams->coordinatorRouteTable[coordAddr >> 8].nextHopAddr = nextHopAddr >> 8;
	}
}

/******************************************************************************
* Function:
*      uint16_t getNextHopAddr(uint16_t coordAddr)
*
* Summary:
*      This function provides the next hop address for Coordinator Address
*
* Parameters:
*      uint16_t coordAddr - Coordinator address to find next hop
*
* Returns:
*      Next hop address to reach the coordinator
******************************************************************************/
uint16_t getNextHopAddr(uint16_t coordAddr)
{
	uint16_t nextHopAddr = DEFAULT_NEXT_HOP_ADDR;
	if (miwiDefaultRomOrRamParams->numOfCoordinators > (coordAddr >> 8)
		&& (NO_NEXT_HOP != miwiDefaultRomOrRamParams->coordinatorRouteTable[coordAddr >> 8].nextHopAddr))
	{
		nextHopAddr = (miwiDefaultRomOrRamParams->coordinatorRouteTable[coordAddr >> 8].nextHopAddr) << 8;
	}
	return nextHopAddr;
}

/******************************************************************************
* Function:
*      void routeTimerHandler(void)
*
* Summary:
*      This function handles Route Timers at 1sec periodicity
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
void routeTimerHandler(void)
{
	NwkFrame_t *dataFramePtr = NULL;
    buffer_t *buffer_header = NULL;
	uint8_t loopIndex;

	for (loopIndex = 0U; loopIndex < routeFrameQueue.size; loopIndex++)
	{
        buffer_header =  qmm_queue_remove(&routeFrameQueue, NULL);
		if (NULL == buffer_header)
		{
			return;
		}
        dataFramePtr = (NwkFrame_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataFramePtr)
		{
			return;
		}
		if ((0UL != dataFramePtr->nwkDataFrame.timeout) && (--dataFramePtr->nwkDataFrame.timeout) == 0UL)
		{
			API_UINT16_UNION nextHopAddr;
			nextHopAddr.Val = getNextHopAddr(dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
			if (DEFAULT_NEXT_HOP_ADDR == nextHopAddr.Val)
			{
				if (NULL != dataFramePtr->nwkDataFrame.confCallback)
				{
					DataConf_callback_t callback = dataFramePtr->nwkDataFrame.confCallback;
					if (NULL != callback)
					{
						callback(dataFramePtr->nwkDataFrame.msghandle, NO_ROUTE, (uint8_t *)dataFramePtr);
					}
                    bmm_buffer_free(buffer_header);
                    
				}
			}
			else
			{
				if (dataFramePtr->nwkDataFrame.meshFrame.header.frameControl.ackRequest)
				{
					dataFramePtr->nwkDataFrame.timeout = calculateAckWaitTimeout(miwiDefaultRomOrRamParams->numOfCoordinators + 1, dataFramePtr->nwkDataFrame.meshFrame.header.dstAddr);
					dataFramePtr->nwkDataFrame.retry = miwiDefaultRomOrRamParams->frameRetry + 1;
					dataFramePtr->nwkDataFrame.nextHopAddr = nextHopAddr.Val;
					if (!sendDataFrame(dataFramePtr, nextHopAddr.Val, ackReqDataConfcb))
					{
						dataFramePtr->nwkDataFrame.retry -= 1U;
					}
                    qmm_queue_append(&ackWaitFrameQueue, buffer_header);
				}
				else
				{
					dataFramePtr->nwkDataFrame.retry = 0U;
					if (!sendDataFrame(dataFramePtr, nextHopAddr.Val, dataFramePtr->nwkDataFrame.confCallback))
					{
						if (NULL != dataFramePtr->nwkDataFrame.confCallback)
						{
							dataFramePtr->nwkDataFrame.confCallback(dataFramePtr->nwkDataFrame.msghandle, FAILURE, (uint8_t *)dataFramePtr);
						}
					}
                    bmm_buffer_free(buffer_header);
				}
			}
		}
		else
		{
            qmm_queue_append(&routeFrameQueue, buffer_header);
		}
	}

	if ((0U != routeUpdateInterval) && (0U == (--routeUpdateInterval)))
	{
		routeUpdateInterval = miwiDefaultRomOrRamParams->routeUpdateInterval;
		sendRouteUpdate();
	}
}

/******************************************************************************
* Function:
*      static void sendRouteUpdate(void)
*
* Summary:
*      This function sends the Route Update frame with current Route details
*
* Parameters:
*      None
*
* Returns:
*      None
******************************************************************************/
static void sendRouteUpdate(void)
{
	uint8_t numOfCoord = getNumberOfCoordinators();
	if (numOfCoord)
	{
		uint8_t* dataPtr = NULL;
		uint8_t dataLen = 0U, headerLen = 0U;
		uint8_t hopCount = CURRENT_HOP_COUNT(numOfCoord);
		MeshFrameHeader_t meshHeader;
		uint16_t broadcastAddr = MESH_BROADCAST_TO_COORDINATORS;
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

		prepareGenericHeader(SINGLE_HOP, myShortAddress, broadcastAddr, &meshHeader);
		headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

		//Fill payload of the command
		dataPtr[dataLen++] = CMD_MESH_ROUTE_TABLE_UPDATE;
		dataPtr[dataLen++] = hopCount;
		memcpy(&dataPtr[dataLen], miwiDefaultRomOrRamParams->coordinatorHopCount, hopCount);
		dataLen += hopCount;
		broadcastAddr = MAC_BROADCAST_ADDR;
		if(!(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t *)&broadcastAddr, 0, routeUpdateConfcb, buffer_header)))
        {
            bmm_buffer_free(buffer_header);	
        }
    }
	return;
}

/******************************************************************************
* Function:
*      void setHopCount(uint8_t index, uint8_t value)
*
* Summary:
*      This function sets the hop count for coordinator index
*
* Parameters:
*      uint8_t index - Coordinator Index
*      uint8_t value - Value to be set in the Index
*
* Returns:
*      None
******************************************************************************/
void setHopCount(uint8_t index, uint8_t value)
{
	if (MAX_HOP_COUNT < value)
	{
		value = MAX_HOP_COUNT;
	}
	if(index % 2U)
	{
		miwiDefaultRomOrRamParams->coordinatorHopCount[(index / 2U)].lsb = value;
	}
	else
	{
		miwiDefaultRomOrRamParams->coordinatorHopCount[(index / 2U)].msb = value;
	}
}

/******************************************************************************
* Function:
*      uint8_t getNumberOfCoordinators(void)
*
* Summary:
*      This function gets total number of coordinators in the routing table
*
* Parameters:
*      None
*
* Returns:
*      Number of Coordinators in the Route table.
******************************************************************************/
uint8_t getNumberOfCoordinators(void)
{
	uint8_t loopIndex, numOfCoordinators = 0U;
	for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
	{
		if (NO_NEXT_HOP != miwiDefaultRomOrRamParams->coordinatorRouteTable[loopIndex].nextHopAddr)
		{
			numOfCoordinators = loopIndex + 1U;
		}
	}
	return numOfCoordinators;
}

/************************************************************************************
* Function:
*      static void routeUpdateConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function frees the memory used for the sending Route Update
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void routeUpdateConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
#if defined(ENABLE_NETWORK_FREEZER)
    /*Store route table and hop table Information in Persistent Data Server */
	PDS_Store(PDS_ROUTE_TABLE_COORD_ID);
    PDS_Store(PDS_ROUTE_TABLE_HOP_ID);
#endif
  (void)handle;
  (void)status;
  (void)msgPointer;
}

/************************************************************************************
* Function:
*      static void routeReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function frees the memory used for the sending Route Request
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void routeReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
  (void)handle;
  (void)status;
  (void)msgPointer;
}

/************************************************************************************
* Function:
*      static void routeReplyConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function frees the memory used for the sending Route Reply
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void routeReplyConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
  (void)handle;
  (void)status;
  (void)msgPointer;
}

/******************************************************************************
* Function:
*      bool initiateRouteReq(uint16_t devAddr)
*
* Summary:
*      This function sends a route request to the provided Destination Address
*
* Parameters:
*      uint16_t devAddr - Device address to which the route request has to be initiated.
*
* Returns:
*      None
******************************************************************************/
bool initiateRouteReq(uint16_t devAddr)
{
	uint8_t* dataPtr = NULL;
	uint8_t dataLen = 0U, headerLen = 0U;
	MeshFrameHeader_t meshHeader;
	uint16_t broadcastAddr = MAC_BROADCAST_ADDR;
    buffer_t *buffer_header = NULL;
    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);

		if (NULL == buffer_header)
		{
			return false;
		}
        dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
		if (NULL == dataPtr)
		{
			return false;
		}

	prepareGenericHeader(MAX_HOP, myShortAddress, broadcastAddr, &meshHeader);

	headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

	if (!addRebroadcastTableEntry(myShortAddress, dataPtr[2]))
	{
        bmm_buffer_free(buffer_header);
		return false;
	}

	devAddr = devAddr & COORD_MASK;

	//Fill payload of the command
	dataPtr[dataLen++] = CMD_MESH_ROUTE_REQUEST;
	dataPtr[dataLen++] = devAddr;
	dataPtr[dataLen++] = devAddr >> 8U;

//	return (frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t *)&broadcastAddr, 0, routeReqConfcb));//tbc
    if(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2U, (uint8_t *)&broadcastAddr, 0, routeReqConfcb,buffer_header))
    {
        return true;
    }
    else
    {
        bmm_buffer_free(buffer_header);
        return false;
    }
}

/******************************************************************************
* Function:
*      static bool sendRouteReply(uint16_t dstAddr, uint8_t coordIndex,
*								 uint8_t hopCount)
*
* Summary:
*      This function sends a route reply to the provided Destination Address
*
* Parameters:
*      uint16_t dstAddr - Destination address to send route reply.
*      uint8_t coordIndex - Index of the coordinator.
*      uint8_t hopCount - Hop count to reach the coordinator.
*
* Returns:
*      None
******************************************************************************/
static void sendRouteReply(uint16_t dstAddr, uint8_t coordIndex, uint8_t hopCount)
{
	uint8_t* dataPtr = NULL;
	uint8_t dataLen = 0U, headerLen = 0U;
	uint16_t nextHopAddr;
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
	prepareGenericHeader(0xFFU, myShortAddress, dstAddr, &meshHeader);
	meshHeader.frameControl.ackRequest = 0U;
	headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

	//Fill payload of the command
	dataPtr[dataLen++] = CMD_MESH_ROUTE_REPLY;
	dataPtr[dataLen++] = 0x00U;
	dataPtr[dataLen++] = coordIndex;
	dataPtr[dataLen++] = hopCount;

	nextHopAddr = getNextHopAddr(dstAddr);
	if(!(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t *)&nextHopAddr, 0, routeReplyConfcb,buffer_header)))
    {
        bmm_buffer_free(buffer_header);
    }
}

/******************************************************************************
* Function:
*      uint8_t getHopCount(uint8_t index)
*
* Summary:
*      This function gets the hop count of Coordinator Index
*
* Parameters:
*      uint8_t index - Index of the coordinator.
*
* Returns:
*      Current hop count to reach the coordinator
******************************************************************************/
uint8_t getHopCount(uint8_t index)
{
	if(index % 2U)
	{
		return miwiDefaultRomOrRamParams->coordinatorHopCount[(index / 2U)].lsb;
	}
	return miwiDefaultRomOrRamParams->coordinatorHopCount[(index / 2U)].msb;
}

#ifdef MIWI_MESH_TOPOLOGY_SIMULATION_MODE
void MiApp_MeshGetRouteEntry(uint8_t coordIndex, RouteEntry_t *routeEntry)
{
	routeEntry->coordNextHop = miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].nextHopAddr;
	routeEntry->coordNextHopLQI = miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].lqi;
	routeEntry->coordScore = miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score;
	routeEntry->coordHopCount = getHopCount(coordIndex);
}

void MiApp_MeshSetRouteEntry(uint8_t coordIndex, RouteEntry_t *routeEntry)
{
	miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].nextHopAddr = routeEntry->coordNextHop;
	miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].lqi = routeEntry->coordNextHopLQI;
	miwiDefaultRomOrRamParams->coordinatorRouteTable[coordIndex].score = routeEntry->coordScore;
	setHopCount(coordIndex, routeEntry->coordHopCount);
#if defined(ENABLE_NETWORK_FREEZER)
    /*Store route table and hop table Information in Persistent Data Server */
    PDS_Store(PDS_ROUTE_TABLE_COORD_ID);
    PDS_Store(PDS_ROUTE_TABLE_HOP_ID);
#endif
}
#endif

#endif //(defined(PAN_COORDINATOR) || defined COORDINATOR)