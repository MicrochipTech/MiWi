/*******************************************************************************
  MiWi Demo Source File

  Company:
    Microchip Technology Inc.

  File Name:
    miwi_mesh_join.c

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
/************************ HEADERS **********************************/
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

/************************ Type Definitions **********************************/
/************************ Static Declarations **********************************/
/* Network Attributes */
uint8_t coordinatorHops;
volatile uint16_t myShortAddress;
#ifndef PAN_COORDINATOR
uint16_t myParentShortAddress = 0xFFFFU;
#endif
static bool connectionPermit = false;
static uint8_t endDeviceCapacityPercent = 0U;
static uint8_t SleepEndDeviceCapacityPercent = 0U;
uint8_t bloomFilterValue[BLOOM_FILTER_SIZE];
uint8_t backupChannel = 0U;
#ifndef PAN_COORDINATOR
uint8_t edLinkFailureAttempts;
LinkFailureCallback_t linkFailureCallback;
bool edInPollingState = false;
#endif
#if defined(ENABLE_NETWORK_FREEZER)
ReconnectionCallback_t reconnectionCallback;
#endif

/* local variables */
static uint8_t gScanDuration;
uint32_t gChannelMap;
uint8_t gCapabilityInfo;
static SearchConnectionConf_callback_t gSearchConfCallback = NULL;
static connectionConf_callback_t gEstConfCallback = NULL;
static searchConf_t* searchConf = NULL;
TimerHandles_t timerHandles;
/* Role Upgrade Done callback */
static roleUpgrade_callback_t roleUpgradeCb = NULL;
static bool noiseDetectionInProgress = false;

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
/************************ Static Prototypes **********************************/
#ifndef ENDDEVICE
static void commandConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
static uint16_t assignAddress(uint8_t* ieeeAddress, JoinWish_t joinwish,
CapabilityInfo_t capabilityinfo, uint8_t* timeout);
bool calculatePermitCapacity(uint8_t* endDeviceCapPer,  uint8_t* SlepEndDeviceCapPer);
#endif
#ifndef PAN_COORDINATOR
static void EstcommandConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer);
static uint8_t ScanChannel(searchConf_t* searchConf);
static void scanDurationExpired(uintptr_t context);
static uint8_t initiateBeaconReq(void);
static inline  uint32_t miwi_scan_duration_ticks(uint8_t scan_duration);
static void establishTimerExpired(uintptr_t context);
#endif
#ifndef PAN_COORDINATOR
//static void rxOffEdKeepAliveTimerHandler(uintptr_t context);
static void dataReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer);
static void keepAliveReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer);
#endif
static void edScanDurationExpired(uintptr_t context);

#ifndef PAN_COORDINATOR
/************************************************************************************
* Function:
*      static void establishTimerExpired(struct SYS_Timer_t *timer)
*
* Summary:
*      This callback function is called when establish timer is expired, so
*      it posts confirmation that establish not success
*
* Parameters/Returns:
*      struct SYS_Timer_t *timer - timer instant
*****************************************************************************************/
static void establishTimerExpired(uintptr_t context)
{
    /* Change back state  */
    meshCurrentState = SEARCH_COMPLETE;

    /* Post Confirmation since no connection response received */
    gEstConfCallback(TIMER_EXPIRED);
    gEstConfCallback = NULL;
}

/************************************************************************************
* Function:
*      static void EstcommandConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function frees the memory used for the command sent
*      and starts connection response timer if packet sent successfully otherwise it post confirm
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void EstcommandConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
    if (SUCCESS == status)
    {
        uint8_t myData = 0U;
        /* Start the timer to wait for connection response */
        timerHandles.joinTimerEstcommandConfcb.handler = &establishTimerExpired;
        timerHandles.joinTimerEstcommandConfcb.timeout = miwiDefaultRomOrRamParams->connRespWaitInSec *1000U;
        timerHandles.joinTimerEstcommandConfcb.interval = miwiDefaultRomOrRamParams->connRespWaitInSec *1000U;
        timerHandles.joinTimerEstcommandConfcb.mode = SYS_TIME_SINGLE;
        timerHandles.joinTimerHandleEstcommandConfcbHandle = SYS_TIME_CallbackRegisterMS(&establishTimerExpired, (uintptr_t)&myData, timerHandles.joinTimerEstcommandConfcb.interval, SYS_TIME_SINGLE);
        if(timerHandles.joinTimerHandleEstcommandConfcbHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
    }
    else
    {
        /* Change back state  */
        meshCurrentState = SEARCH_COMPLETE;

        /* Connection request is not sent successfully, post confirmation */
        gEstConfCallback(status);
        gEstConfCallback = NULL;
    }
    (void)handle;
    (void)msgPointer;
}

/************************************************************************************
* Function:
*      uint8_t    MiApp_EstablishConnection(uint8_t Channel, uint8_t addr_len, uint8_t *addr, uint8_t Capability_info,
*                                                                    connectionConf_callback_t ConfCallback)
*
* Summary:
*      This function establish a connection with one or more nodes in an existing
*      PAN.
*
* Description:
*      This is the primary user interface function for the application layer to
*      start communication with an existing PAN. For P2P protocol, this function
*      call can establish one or more connections. For network protocol, this
*      function can be used to join the network, or establish a virtual socket
*      connection with a node out of the radio range. There are multiple ways to
*      establish connection(s), all depends on the input parameters.
*
* PreCondition:
*      Protocol initialization has been done. If only to establish connection with
*      a predefined device, an active scan must be performed before and valid active
*      scan result has been saved.
*
* Parameters:
*      uint8_t channel -  The selected channel to invoke join procedure.
*      uint8_t addr_len - Address length
*      uint8_t *addr  - address of the parent
*      uint8_t Capability_info - capability information of the device
*      connectionConf_callback_t ConfCallback - The callback routine which will be called upon
*                                               the initiated connection procedure is performed
*
* Returns:
*      The index of the peer device on the connection table.
*
* Example:
*      <code>
*      // Establish one or more connections with any device
*      PeerIndex = MiApp_EstablishConnection(14, 8, 0x12345678901234567,0x80, callback);
*      </code>
*
* Remarks:
*      If more than one connections have been established through this function call, the
*      return value points to the index of one of the peer devices.
*
*****************************************************************************************/
uint8_t MiApp_EstablishConnection(uint8_t Channel, uint8_t addr_len, uint8_t *addr, uint8_t Capability_info,
                                                                    connectionConf_callback_t ConfCallback)
{
    uint8_t* dataPtr = NULL;
    uint8_t dataLen = 0U, headerLen = 0U;
    MeshFrameHeader_t meshHeader;
    uint32_t deviceTimeout = miwiDefaultRomOrRamParams->deviceTimeout;
    uint16_t dstAddr;
    uint32_t channelMap = 0UL;
    buffer_t *buffer_header = NULL;

    channelMap = MiMAC_GetPHYChannelInfo();

    /* Check the given channel is within the range and callback is not NULL*/
    if ((((channelMap & (uint32_t)(1UL << Channel)) == 0UL)) || (NULL == ConfCallback))
        return ((uint8_t)FAILURE);

    /* If state is initial state , don't process further */
    if (meshCurrentState == INITIAL_STATE)
        return 0U;

    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return 0U;
    }
    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
	if (NULL == dataPtr)
	{
		return 0U;
	}

    /* Set the given channel */
    MiApp_Set(CHANNEL, &Channel);

    /* Change state while processing establish connection */
    meshCurrentState = ESTABLISHING_NETWORK;

    /* Store Capability info */
    gCapabilityInfo = Capability_info;

    dstAddr = (uint16_t)(addr[0] + (addr[1] << 8));

    /* Prepare the common header */
    prepareGenericHeader(SINGLE_HOP, 0xFFFF, dstAddr, &meshHeader);

    /* Update the header for beacon response command */
    meshHeader.frameControl.addressSameAsMAC = 1;

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    /* Construct the full frame with command id and payload */
    dataPtr[dataLen++] = (uint8_t)CMD_MESH_CONNECTION_REQUEST;
    dataPtr[dataLen++] = JOIN_WISH;
    dataPtr[dataLen++] = Capability_info;
    memcpy(&dataPtr[dataLen], (uint8_t*)&deviceTimeout, sizeof(deviceTimeout));//misra 21.15
    dataLen += (uint8_t)sizeof(deviceTimeout);

    /* Store the confirmation callback */
    gEstConfCallback = ConfCallback;

    /* Initiate the frame transmission */
//    return (frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, SHORT_ADDR_LEN, addr, 0, EstcommandConfcb));//tbc
    if(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, SHORT_ADDR_LEN, addr, 0, EstcommandConfcb, buffer_header))
    { 
        return true;
    }
    else
    {
        bmm_buffer_free(buffer_header);
        return false;
    }
}

/************************************************************************************
* Function:
*      uint8_t    MiApp_SearchConnection(uint8_t ScanDuartion, uint32_t ChannelMap,
*                                             SearchConnectionConf_callback_t ConfCallback)
*
* Summary:
*      This function perform an active scan to locate operating PANs in the
*      neighborhood.
*
* Description:
*      This is the primary user interface function for the application layer to
*      perform an active scan. After this function call, all active scan response
*      will be stored in the global variable ActiveScanResults in the format of
*      structure ACTIVE_SCAN_RESULT. The return value indicates the total number
*      of valid active scan response in the active scan result array.
*
* PreCondition:
*      Protocol initialization has been done.
*
* Parameters:
*      uint8_t ScanDuration - The maximum time to perform scan on single channel. The
*                          value is from 5 to 14. The real time to perform scan can
*                          be calculated in following formula from IEEE 802.15.4
*                          specification
*                              960 * (2^ScanDuration + 1) * 10^(-6) second
*      uint32_t ChannelMap -  The bit map of channels to perform noise scan. The 32-bit
*                          double word parameter use one bit to represent corresponding
*                          channels from 0 to 31. For instance, 0x00000003 represent to
*                          scan channel 0 and channel 1.
*      SearchConnectionConf_callback_t ConfCallback - The callback routine which will be called upon
*                                               the initiated connection procedure is performed
*
* Returns:
*      The number of valid active scan response stored in the global variable ActiveScanResults.
*
* Example:
*      <code>
*      // Perform an active scan on all possible channels
*      NumOfActiveScanResponse = MiApp_SearchConnection(10, 0xFFFFFFFF, callback);
*      </code>
*
* Remarks:
*      None
*
*****************************************************************************************/
uint8_t MiApp_SearchConnection(uint8_t ScanDuration, uint32_t ChannelMap, SearchConnectionConf_callback_t ConfCallback)
{
    uint32_t supportedChannelMap = 0U;
    uint32_t index = 1UL;

    /* Check the given scan duration is within the range and callback is not NULL*/
    if ((ScanDuration > MAX_SCAN_DURATION) || (NULL == ConfCallback))
        return FAILURE;

    /* If state is not Initial state or search complete , don't process further */
    if ((meshCurrentState != INIT_STATE) && (meshCurrentState != SEARCH_COMPLETE)
        && (meshCurrentState != DISCONNECTED))
        {
            return FAILURE;
        }

    /* Change state while processing start connection */
    meshCurrentState = SEARCHING_NETWORK;

    /* Store scan duration */
    gScanDuration = ScanDuration;

    /* Store scan duration */
    gChannelMap = ChannelMap;

    /* Allocate memory for scan and confirmation */
    searchConf = miwiDefaultRomOrRamParams->searchConfMem;

    /* Initialize the scan confirm parameters */
    searchConf->resultSize = 0;
    searchConf->status = SCAN_NO_BEACON;
    searchConf->unscannedChannels = ChannelMap;
    gSearchConfCallback = ConfCallback;

    /* Get the supported channel and find the first channel to start scan */
    supportedChannelMap = MiMAC_GetPHYChannelInfo();

    currentChannel = 0U;

    while (!(index & supportedChannelMap))
    {
        // Unset current bit and set the next bit in 'i'
        index = index << 1;

        // increment position
        ++currentChannel;
    }

    /* Start the scan procedure */
    return (ScanChannel(searchConf));
}

#else

/************************************************************************************
    * Function:
    *      bool    MiApp_StartConnection(uint8_t Mode, uint8_t ScanDuration, uint32_t ChannelMap,
    *                                                    connectionConf_callback_t ConfCallback)
    *
    * Summary:
    *      This function start a PAN without connected to any other devices
    *
    * Description:
    *      This is the primary user interface function for the application layer to
    *      a PAN. Usually, this function is called by the PAN Coordinator who is the
    *      first in the PAN. The PAN Coordinator may start the PAN after a noise scan
    *      if specified in the input mode.
    *
    * PreCondition:
    *      Protocol initialization has been done.
    *
    * Parameters:
    *      uint8_t Mode - Whether to start a PAN after a noise scan. Possible modes are
    *                  * START_CONN_DIRECT Start PAN directly without noise scan
    *                  * START_CONN_ENERGY_SCN Perform an energy scan first, then
    *                                          start the PAN on the channel with least
    *                                          noise.
    *                  * START_CONN_CS_SCN     Perform a carrier-sense scan first,
    *                                          then start the PAN on the channel with
    *                                          least noise.
    *      uint8_t ScanDuration - The maximum time to perform scan on single channel. The
    *                          value is from 5 to 14. The real time to perform scan can
    *                          be calculated in following formula from IEEE 802.15.4
    *                          specification:
    *                              960 * (2^ScanDuration + 1) * 10^(-6) second
    *                          ScanDuration is discarded if the connection mode is
    *                          START_CONN_DIRECT.
    *      uint32_t ChannelMap -  The bit map of channels to perform noise scan. The 32-bit
    *                          double word parameter use one bit to represent corresponding
    *                          channels from 0 to 31. For instance, 0x00000003 represent to
    *                          scan channel 0 and channel 1. ChannelMap is discarded if the
    *                          connection mode is START_CONN_DIRECT.
    *
    *      connectionConf_callback_t ConfCallback - The callback routine which will be called upon
    *                                               the initiated connection procedure is performed
    * Returns:
    *      a boolean to indicate if PAN has been started successfully.
    *
    * Example:
    *      <code>
    *      // start the PAN on the least noisy channel after scanning all possible channels.
    *      MiApp_StartConnection(START_CONN_ENERGY_SCN, 10, 0xFFFFFFFF, callback);
    *      </code>
    *
    * Remarks:
    *      None
    *
    *****************************************************************************************/
bool MiApp_StartConnection(uint8_t Mode, uint8_t ScanDuration, uint32_t ChannelMap,
                                                       connectionConf_callback_t ConfCallback)
{
    /* Check the given scan duration is within the range and callback is not NULL*/
    if ((ScanDuration > MAX_SCAN_DURATION) || (NULL == ConfCallback))
    {
        return false;
    }

    /* If state is not Initial state , don't process further */
    if (meshCurrentState != INIT_STATE)
    {
        return false;
    }

    /* Change state while processing start connection */
    meshCurrentState = STARTING_NETWORK;

    /* Store scan duration */
    gChannelMap = ChannelMap;

    switch(Mode)
    {
        case START_CONN_DIRECT:
        {
            uint8_t channel = 0U;
            uint32_t index = 1UL;
            while (!(index & ChannelMap))
            {
                // Unset current bit and set the next bit in 'i'
                index = index << 1;

                // increment position
                ++channel;
            }
            /* Set the best channel */
            MiApp_Set(CHANNEL, &channel);

            /* Set Short Address and PAN ID */
            myShortAddress = PAN_COORDINATOR_ADDRESS;
                MiMAC_SetAltAddress((uint8_t *)&myShortAddress);
                MiMAC_SetPanId((uint8_t *)&myPANID.Val);
            /* Send Confirmation to caller */
            ConfCallback(SUCCESS);

            /* Set the state to IDLE which is the in network state*/
            meshCurrentState = IN_NETWORK_STATE;

#if defined(ENABLE_NETWORK_FREEZER)
            /*Store Network Information in Persistent Data Server */
            PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif

            return true;
        }
        break;

        case START_CONN_ENERGY_SCN:
#if defined(ENABLE_ED_SCAN)
        {
            uint8_t channel;
            uint8_t RSSIValue;

            /* Set Short Address and PAN ID */
            myShortAddress = PAN_COORDINATOR_ADDRESS;
                MiMAC_SetAltAddress((uint8_t *)&myShortAddress);
                MiMAC_SetPanId((uint8_t *)&myPANID.Val);
            /* Identify the best channel by doing energy detection scan */
            channel = MiApp_NoiseDetection(ChannelMap, ScanDuration, NOISE_DETECT_ENERGY, &RSSIValue);

            /* Set the best channel */
            MiApp_Set(CHANNEL, &channel);

            /* Send Confirmation to caller */
            ConfCallback(SUCCESS);
#if defined(ENABLE_NETWORK_FREEZER)
            /*Store Network Information in Persistent Data Server */
            PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif

            /* Set the state to IDle which is the in network state*/
            meshCurrentState = IN_NETWORK_STATE;

            return true;
        }
#else
        {
            ConfCallback(FAILURE);
            /* Set the state to Initial since failure to start network */
            meshCurrentState = INIT_STATE;
            return false;
        }
#endif
        break;

        case START_CONN_CS_SCN:
        {
           /* Carrier sense scan is not supported for current available transceivers */
           ConfCallback(FAILURE);
           /* Set the state to Initial since failure to start network */
           meshCurrentState = INIT_STATE;
           return false;
        }
        break;

        default:
            //Handle exceptions if any
            break;
    }
    ConfCallback(FAILURE);
    /* Set the state to Initial since failure to start network */
    meshCurrentState = INIT_STATE;
    return false;
}
#endif

#ifndef ENDDEVICE
bool calculatePermitCapacity(uint8_t* endDeviceCapPer,  uint8_t* SlepEndDeviceCapPer)
{
    uint8_t loopIndex = 0U;
    bool connectionPermitLocal = false;
    uint8_t numberOfEnddevices = 0U;
    uint8_t numberOfSleepEnddevices = 0U;

#ifdef PAN_COORDINATOR
    for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
    {
        if (false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->coordTable[loopIndex].ieeeaddr))
        {
            connectionPermitLocal = true;
        }
    }
#endif

    for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; loopIndex++)
    {
        if (false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->devTable[loopIndex].ieeeaddr))
        {
             connectionPermitLocal = true;
        }
        else
        {
            numberOfEnddevices += 1U;
        }
    }
    for (loopIndex = 1; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
    {
        if (false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr))
        {
            connectionPermitLocal = true;
        }
        else
        {
            numberOfSleepEnddevices += 1U;
        }
    }
    *endDeviceCapPer = (((miwiDefaultRomOrRamParams->numOfRxOnEnddevices - numberOfEnddevices) * 100)/miwiDefaultRomOrRamParams->numOfRxOnEnddevices);
    *SlepEndDeviceCapPer = (((miwiDefaultRomOrRamParams->numOfRxOffEnddevices - numberOfSleepEnddevices) *100)/miwiDefaultRomOrRamParams->numOfRxOffEnddevices);
    return connectionPermitLocal;
}
#endif
/************************************************************************************
* Function:
*      void handleJoinMessage(MeshFrameHeader_t *meshHeader, uint8_t macSrcAddrLen,
                        uint8_t* macSrcAddr, uint8_t* payload, uint8_t lqiValue)
*
* Summary:
*      This function parses and handles all join related messages
*
* Parameters:
*      uint8_t* meshHeader - the received mesh header.
*      uint8_t macSrcAddrLen - Source Address length of the MAC Header
*      uint8_t* macSrcAddr - Source Address of received MAC Header
*      uint8_t* payload - command payload
*      uint8_t lqiValue - lqi of the received packet
*
* Returns:
*      None
*****************************************************************************************/
void handleJoinMessage(MeshFrameHeader_t *meshHeader, uint8_t macSrcAddrLen,
                                uint8_t* macSrcAddr, uint8_t* payload, uint8_t lqiValue)
{
    switch (payload[0])
    {
#ifndef ENDDEVICE
        case CMD_MESH_BEACON_REQUEST:
        {
            uint8_t dataLen = 0U, headerLen = 0U;
            uint8_t *dataPtr = NULL;
            MeshFrameHeader_t meshHeaderlocal;
            bool bloomEntryFound = false;
            buffer_t *buffer_header = NULL;


            /* If state is not in network state , don't process further, ignore packet */
            if (meshCurrentState != IN_NETWORK_STATE)
                return;

            /*If I am a not coordinator, ignore the packet */
            if (myShortAddress & ENDDEVICE_MASK)
                return;

            /* Validate BloomFilter value before sending beacon response */
            bloomEntryFound = bloomFilterAddressCheck(bloomFilterValue, BLOOM_FILTER_SIZE, macSrcAddr);

            if (bloomEntryFound)
            {
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

                /* Prepare the common header */
                prepareGenericHeader(SINGLE_HOP, myShortAddress, 0U, &meshHeaderlocal);

                /* Update the header for beacon response command */
                meshHeaderlocal.frameControl.addressSameAsMAC = 1U;

                /* Construct the general frame based on mesh header information */
                headerLen = dataLen = generalFrameConstruct(&meshHeaderlocal, dataPtr);

                /* Construct the full frame with command id and payload */
                dataPtr[dataLen++] = CMD_MESH_BEACON_RESPONSE;
                dataPtr[dataLen++] = coordinatorHops;
                connectionPermit = calculatePermitCapacity(&endDeviceCapacityPercent, &SleepEndDeviceCapacityPercent);
                dataPtr[dataLen++] = connectionPermit;
                dataPtr[dataLen++] = endDeviceCapacityPercent;
                dataPtr[dataLen++] = SleepEndDeviceCapacityPercent;

                memcpy(&dataPtr[dataLen], bloomFilterValue, BLOOM_FILTER_SIZE);
                dataLen += BLOOM_FILTER_SIZE;
                
                /* Initiate the frame transmission */
                if(!(frameTransmit(&meshHeaderlocal, headerLen, dataLen - headerLen,dataPtr, macSrcAddrLen, macSrcAddr, 0, commandConfcb,buffer_header)))
                { 
                bmm_buffer_free(buffer_header);
                }
            }
        }
        break;
#endif
#ifndef PAN_COORDINATOR
        case CMD_MESH_BEACON_RESPONSE:
        {
            uint8_t loopIndex;
            /* If state is not in searching for network state , don't process further, ignore packet */
            if (meshCurrentState != SEARCHING_NETWORK)
                return;

            /* if enough space is available for storing new beacon information, then store the beacon info */
            if (searchConf->resultSize < miwiDefaultRomOrRamParams->maxNoOfBeacons)
            {
                bool bloomEntryFound = false;

                /*Check whether my own address is there in bloom or not, if available, store it in beacon list
                 else drop the packet */
                bloomEntryFound = bloomFilterAddressCheck(&payload[5], BLOOM_FILTER_SIZE, myLongAddress);

                if (!bloomEntryFound)
                {
                    return;
                }

                searchConf->status = SUCCESS;

                /* Find already entry exists for the received source address */
                for (loopIndex = 0U; loopIndex < searchConf->resultSize; loopIndex++)
                {
                    if (meshHeader->srcAddr == searchConf->beaconList[loopIndex].shortAddress)
                    {
                       /* Entry already exists, drop the packet */
                       return;
                    }
                }

                searchConf->beaconList[searchConf->resultSize].shortAddress = meshHeader->srcAddr;

                searchConf->beaconList[searchConf->resultSize].logicalChannel = currentChannel;
                searchConf->beaconList[searchConf->resultSize].panId = meshHeader->dstPanId;

                searchConf->beaconList[searchConf->resultSize].coordinatorHop = payload[1];
                searchConf->beaconList[searchConf->resultSize].connectionPermit = payload[2];
                searchConf->beaconList[searchConf->resultSize].enddeviceCapacity = payload[3];
                searchConf->beaconList[searchConf->resultSize].sleepEnddeviceCapacity = payload[4];

                memcpy(searchConf->beaconList[searchConf->resultSize].bloomFilterValue,&payload[5],BLOOM_FILTER_SIZE);
                memcpy(bloomFilterValue,&payload[5],BLOOM_FILTER_SIZE);
#if defined(ENABLE_NETWORK_FREEZER)
                /*Store Network Information in Persistent Data Server */
                PDS_Store(PDS_BLOOM_VALUE_ID);
#endif
                searchConf->beaconList[searchConf->resultSize].LinkQuality = lqiValue;
#ifndef ENDDEVICE
                /* Add Route since the coordinator is in the range */
                addRoute(searchConf->beaconList[searchConf->resultSize].shortAddress, searchConf->beaconList[searchConf->resultSize].shortAddress,
                                     SINGLE_HOP, lqiValue);
#endif
                searchConf->resultSize += 1U;
            }
            /* If stored no.of beacon entries reaches maximum , then stop the timer, post the confirmation to caller */
            if (searchConf->resultSize == miwiDefaultRomOrRamParams->maxNoOfBeacons)
            {
                /* Stop the search timer since the memory for storing beacons is full */
                SYS_TIME_TimerStop(timerHandles.joinTimerHandleEstcommandConfcbHandle);
                SYS_TIME_TimerDestroy(timerHandles.joinTimerHandleEstcommandConfcbHandle);

                searchConf->status = SCAN_MAX_REACHED;

                /* Change state while completing search */
                meshCurrentState = SEARCH_COMPLETE;

                /* Send confirmation to the upper layer. */
                gSearchConfCallback(searchConf->resultSize, (uint8_t*)searchConf);
                gSearchConfCallback = NULL;
            }
            
        }
        break;
#endif
#ifndef ENDDEVICE
        case CMD_MESH_CONNECTION_REQUEST:
        {
            
            uint8_t* dataPtr = NULL;
            uint8_t dataLen = 0U, headerLen = 0U;
            uint16_t newAddress;
            MeshFrameHeader_t meshHeader1;
            JoinWish_t joinWish;
            CapabilityInfo_t capInfo;
            buffer_t *buffer_header = NULL;

            joinWish.Val = payload[1];
            capInfo.Val = payload[2];

            /* If state is not in network state , don't process further, ignore packet */
            if (meshCurrentState != IN_NETWORK_STATE)
                return;

            /*If I am a not coordinator, ignore the packet */
            if (myShortAddress & ENDDEVICE_MASK)
                return;

            newAddress = assignAddress(macSrcAddr, joinWish, capInfo, &payload[3]);

            /* Prepare the common header */
            prepareGenericHeader(SINGLE_HOP, myShortAddress, BROADCAST_TO_ALL, &meshHeader1);

            /* Update the header for mesh connection response command */
            meshHeader1.frameControl.addressSameAsMAC = 1U;

            /* Construct and Send Mesh Connection Response frame */
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
            /* Construct the general frame based on mesh header information */
            headerLen = dataLen = generalFrameConstruct(&meshHeader1, dataPtr);

            /* Fill payload of the command */
            dataPtr[dataLen++] = CMD_MESH_CONNECTION_RESPONSE;

            if (newAddress != 0xFFFF)
            {
                dataPtr[dataLen++] = SUCCESS;
				addRoute(newAddress, newAddress, SINGLE_HOP, lqiValue);
            }
            else
            {
                dataPtr[dataLen++] = FAILURE;
            }
            dataPtr[dataLen++] = (uint8_t)newAddress;
            dataPtr[dataLen++] = (uint8_t)(newAddress >> 8U);

#ifdef MESH_SECURITY
            memcpy(&dataPtr[dataLen], miwiDefaultRamOnlyParams->miwiNetworkKey, SECURITY_KEY_SIZE);
            dataLen += SECURITY_KEY_SIZE;
#endif
            /* Initiate the frame transmission */
            if(!(frameTransmit(&meshHeader1, headerLen, dataLen-headerLen, dataPtr, macSrcAddrLen, macSrcAddr, 0, commandConfcb,buffer_header)))
            { 
            bmm_buffer_free(buffer_header);
            }
        }
        break;
#endif
#ifndef PAN_COORDINATOR
        case CMD_MESH_CONNECTION_RESPONSE:
        {
            /* Stop the Establish timer */
            SYS_TIME_TimerStop(timerHandles.joinTimerHandleEstcommandConfcbHandle);
            SYS_TIME_TimerDestroy(timerHandles.joinTimerHandleEstcommandConfcbHandle);
            /* If state is not in establishing network state , don't process further, ignore packet */
            if (meshCurrentState != ESTABLISHING_NETWORK)
                return;

            /* If Status is success in mesh connection response, then store the short address and network
            key information */
            if (payload[1] == SUCCESS)
            {
                myShortAddress = ((uint16_t)payload[2] | ((uint16_t)payload[3] << 8));
                myParentShortAddress = meshHeader->srcAddr;
                myPANID.Val = meshHeader->dstPanId;
                MiMAC_SetAltAddress((uint8_t *)&myShortAddress);
                MiMAC_SetPanId((uint8_t *)&myPANID.Val);
                edLinkFailureAttempts = miwiDefaultRomOrRamParams->edLinkFailureAttempts;
#ifdef MESH_SECURITY
                memcpy(miwiDefaultRamOnlyParams->miwiNetworkKey, &payload[4], SECURITY_KEY_SIZE);
#endif
#ifdef COORDINATOR
                if ((miwiDefaultRomOrRamParams->joinWish & JOINWISH_ED_MASK) && (myShortAddress & ENDEVICE_ADDRESS_MASK))
                {
                    /* Change state since establishment is complete */
                    meshCurrentState = IN_NETWORK_PARTIAL_STATE;

                    /* Start the search timer for scan Duration time */
                    timerHandles.joinTimerRoleUpgrade.handler = roleUpgradeTimerExpired;
                    timerHandles.joinTimerRoleUpgrade.timeout = miwiDefaultRomOrRamParams->roleUpgradeIntervalInSec *1000U; 
                    timerHandles.joinTimerRoleUpgrade.interval = miwiDefaultRomOrRamParams->roleUpgradeIntervalInSec *1000U;
                    timerHandles.joinTimerRoleUpgrade.mode = SYS_TIME_PERIODIC;
                    
                    uint8_t myData = 0U;
                    timerHandles.joinTimerRoleUpgradeHandle = SYS_TIME_CallbackRegisterMS(&roleUpgradeTimerExpired, (uintptr_t)&myData, timerHandles.joinTimerRoleUpgrade.interval, SYS_TIME_PERIODIC);
                   if(timerHandles.joinTimerRoleUpgradeHandle == SYS_TIME_HANDLE_INVALID)
                   {
                   	  return;
                   }
                    

                    /* Keep alive timer - load with rxon end device since it got address as rxonED*/
                    timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout = generateJitterTimeout(miwiDefaultRomOrRamParams->keepAliveRxOnEdSendInterval * 1000, 5);
                    timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval = timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout;
                }
                else if ((miwiDefaultRomOrRamParams->joinWish & JOINWISH_COORD_ALONE_MASK) && (myShortAddress & ENDEVICE_ADDRESS_MASK))
                {
                    /* Failure Status received in connection response...post confirmation */
                    /* Change back state since establishment is not success */
                    meshCurrentState = SEARCH_COMPLETE;
                    myShortAddress = 0xFFFFU;

                    gEstConfCallback((miwi_status_t)FAILURE);
                    gEstConfCallback = NULL;
                    return;
                }
                else
                {
                    /* Change state since establishment is complete */
                    meshCurrentState = IN_NETWORK_STATE;

                    timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout = generateJitterTimeout(miwiDefaultRomOrRamParams->keepAliveCoordSendInterval * 1000, 5U);
                    timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval = timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout;
                }
                timerHandles.keepAliveTimerSendKeepAliveRxOnEd.handler = sendKeepAlive;
                timerHandles.keepAliveTimerSendKeepAliveRxOnEd.mode = SYS_TIME_PERIODIC;
                uint8_t myData = 0U;
                timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle = SYS_TIME_CallbackRegisterMS(&sendKeepAlive, (uintptr_t)&myData, timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval, SYS_TIME_PERIODIC);
                if(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle == SYS_TIME_HANDLE_INVALID)
                {
                   	  return;
                }

#if defined(ENABLE_NETWORK_FREEZER)
                /*Store Network Information in Persistent Data Server */
                PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif
#else
                /* Change state since establishment is complete */
                meshCurrentState = IN_NETWORK_STATE;

                /* Reset BusyLock */
                busyLock = 0U;
#if defined(ENABLE_NETWORK_FREEZER)
                /*Store Network Information in Persistent Data Server */
                PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif
#endif
                if (myShortAddress & ENDDEVICE_MASK)
                {
                    if (RXONWHENIDLE_ED_ADDRESS_MASK & myShortAddress)
                    {
                        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout = generateJitterTimeout(miwiDefaultRomOrRamParams->keepAliveRxOnEdSendInterval * 1000, 5);
                        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval = timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout;
                        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.handler = sendKeepAlive;
                        timerHandles.keepAliveTimerSendKeepAliveRxOnEd.mode = SYS_TIME_PERIODIC;
                        uint8_t myData = 0U;
                    timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle = SYS_TIME_CallbackRegisterMS(&sendKeepAlive, (uintptr_t)&myData, timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval, SYS_TIME_PERIODIC);
                    if(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle == SYS_TIME_HANDLE_INVALID)
                    {
                   	  return;
                    }
                    }
                    else
                    {
                        /* Poll immediately after join- small timer for context break */
						timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
                        timerHandles.keepAliveTimerRxOffEd.timeout = 50U;
                        timerHandles.keepAliveTimerRxOffEd.interval = 50U;
                        timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
                        uint8_t myData = 0U;
                        timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
                        if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
                        {
                         return;
                        }
                    }
                }
            }
            else
            {
                /* Failure Status received in connection response...post confirmation */
                /* Change back state since establishment is not success */
                meshCurrentState = SEARCH_COMPLETE;
            }
            gEstConfCallback((miwi_status_t)payload[1]);
            gEstConfCallback = NULL;
        
        }
        break;
#endif
#ifdef PAN_COORDINATOR
        case CMD_MESH_ROLE_UPGRADE_REQUEST:
        {
            uint8_t* dataPtr = NULL;
            uint8_t dataLen = 0U, headerLen = 0U;
            uint16_t newAddress, nextHopAddr = 0U;
            MeshFrameHeader_t meshHeader1;
            CapabilityInfo_t capability;
            JoinWish_t joinwish;
            capability.Val= CAPABILITY_INFO_COORD;
            joinwish.Val = JOINWISH_COORD_ALONE_MASK;
            buffer_t *buffer_header = NULL;

            /* If state is not in network state , don't process further, ignore packet */
            if (meshCurrentState != IN_NETWORK_STATE)
                return;

            /* First check address available for the coordinator for the up gradation*/
            newAddress = assignAddress(&payload[1], joinwish, capability, NULL);

            /* Address unavailable, don't send any response */
            if (newAddress == 0xFFFFU)
                return;

            /* Construct and Send Mesh Connection Response frame */
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

            /* Prepare the common header */
            prepareGenericHeader(MAX_HOP, myShortAddress, meshHeader->srcAddr, &meshHeader1);

            /* Construct the general frame based on mesh header information */
            headerLen = dataLen = generalFrameConstruct(&meshHeader1, dataPtr);

            /* Fill payload of the command */
            dataPtr[dataLen++] = (uint8_t)CMD_MESH_ROLE_UPGRADE_RESPONSE;

            dataPtr[dataLen++] = (uint8_t)newAddress;
            dataPtr[dataLen++] = (uint8_t)(newAddress >> 8U);

            nextHopAddr = getNextHopAddr(meshHeader1.dstAddr & COORD_MASK);

            /* Initiate the frame transmission */
            if(!(frameTransmit(&meshHeader1, headerLen, dataLen-headerLen, dataPtr,SHORT_ADDR_LEN, (uint8_t *)&nextHopAddr, 0, commandConfcb,buffer_header)))
            {

            bmm_buffer_free(buffer_header);
            }
        }
        break;
#endif
#if (defined(PAN_COORDINATOR) || defined (COORDINATOR))
        case CMD_MESH_KEEP_ALIVE:
        {
            uint8_t shortAddressIndex = 0U;
            /* If state is not in network state , don't process further, ignore packet */
            if (meshCurrentState != IN_NETWORK_STATE)
                return;
#ifdef PAN_COORDINATOR
            if (!(meshHeader->srcAddr & ENDEVICE_ADDRESS_MASK) && (meshHeader->srcAddr & COORDINATOR_ADDRESS_MASK))
            {
                for (shortAddressIndex = 1U; shortAddressIndex < miwiDefaultRomOrRamParams->numOfCoordinators; shortAddressIndex++)
                {
                    if (shortAddressIndex == ((meshHeader->srcAddr & COORD_MASK)>>8))
                    {
                        if (memcmp(miwiDefaultRomOrRamParams->coordTable[shortAddressIndex].ieeeaddr, &payload[1], LONG_ADDR_LEN) == 0)
                        {
                            miwiDefaultRomOrRamParams->coordTable[shortAddressIndex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveCoordTimeoutSec;
                        }
                        else
                        {
                           sendForceLeaveNetwork(meshHeader->srcAddr);
                        }
                    }
                }
            }
#endif
            if (meshHeader->srcAddr & RXON_ENDEVICE_ADDRESS_MASK)
            {
                for (shortAddressIndex = 0U; shortAddressIndex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; shortAddressIndex++)
                {
                    if (shortAddressIndex == (meshHeader->srcAddr & RXON_ENDEVICE_ADDRESS_MASK))
                    {
                        if (memcmp(miwiDefaultRomOrRamParams->devTable[shortAddressIndex].ieeeaddr, &payload[1], LONG_ADDR_LEN) == 0)
                        {
                            miwiDefaultRomOrRamParams->devTable[shortAddressIndex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveRxOnEdTimeoutSec;
                        }
                        else
                        {
                            sendForceLeaveNetwork(meshHeader->srcAddr);
                        }
                    }
                }
            }
        }
        break;
#endif
#ifndef PAN_COORDINATOR
        case CMD_MESH_LEAVE:
        {
            edInPollingState = false;
            SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
            SYS_TIME_TimerStop(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle);
            SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
            SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle);
            meshCurrentState = DISCONNECTED;
            myShortAddress = 0xFFFFU;
#if defined(ENABLE_NETWORK_FREEZER)
            /*Store Network Information in Persistent Data Server */
            PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif
            if (NULL != linkFailureCallback)
            {
                linkFailureCallback();
            }
        }
        break;
#endif
#ifdef COORDINATOR
        case CMD_MESH_ROLE_UPGRADE_RESPONSE:
        {
            /* If state is not in network partial state , don't process further, ignore packet */
            if (meshCurrentState != IN_NETWORK_PARTIAL_STATE)
                return;

            /* Store the new short address */
            myShortAddress = ((uint16_t)payload[1] | ((uint16_t)payload[2] << 8));
            myParentShortAddress = meshHeader->srcAddr;
            myPANID.Val = meshHeader->dstPanId;
                MiMAC_SetAltAddress((uint8_t *)&myShortAddress);
                MiMAC_SetPanId((uint8_t *)&myPANID.Val);
            /* Role upgrade response received, stop roleupgrade periodic timer*/
            SYS_TIME_TimerStop(timerHandles.joinTimerRoleUpgradeHandle);
            SYS_TIME_TimerDestroy(timerHandles.joinTimerRoleUpgradeHandle);
            

            /* Stop Keep Alive timer which is running for RxonEnddevice keep alive,
            reload with coord send interval and start timer */
            SYS_TIME_TimerStop(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle);
            SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle);
            timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout = generateJitterTimeout(miwiDefaultRomOrRamParams->keepAliveCoordSendInterval * 1000, 5);
            timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval = timerHandles.keepAliveTimerSendKeepAliveRxOnEd.timeout;
            timerHandles.keepAliveTimerSendKeepAliveRxOnEd.handler = sendKeepAlive;
            timerHandles.keepAliveTimerSendKeepAliveRxOnEd.mode = SYS_TIME_PERIODIC;
            uint8_t myData = 0U;
            timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle = SYS_TIME_CallbackRegisterMS(&sendKeepAlive, (uintptr_t)&myData, timerHandles.keepAliveTimerSendKeepAliveRxOnEd.interval, SYS_TIME_PERIODIC);
            if(timerHandles.keepAliveTimerSendKeepAliveRxOnEdHandle == SYS_TIME_HANDLE_INVALID)
            {
            	 return;
            }
            

            /* Change state since establishment is complete */
            meshCurrentState = IN_NETWORK_STATE;
#if defined(ENABLE_NETWORK_FREEZER)
            /*Store Network Information in Persistent Data Server */
            PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif

            /* Inform application about the address change */
            if (roleUpgradeCb)
                roleUpgradeCb(myShortAddress);
        }
        break;
#endif
        default:
        break;
    }
}

#ifdef PAN_COORDINATOR
/************************************************************************************
* Function:
*      void coordinatorTableInit(void)
*
* Summary:
*      This function initializes the coordinator Table
*
* Parameters and Returns:
*      None
*****************************************************************************************/
void coordinatorTableInit(void)
{
    for (uint8_t shortAddressindex = 0U; shortAddressindex < miwiDefaultRomOrRamParams->numOfCoordinators; shortAddressindex++)
    {
        memset(miwiDefaultRomOrRamParams->coordTable[shortAddressindex].ieeeaddr, 0xFF, LONG_ADDR_LEN) ;
        memset(&miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut, 0xFF,
             sizeof(miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut)) ;
    }
}
#endif

#ifndef ENDDEVICE
/************************************************************************************
* Function:
*      void deviceTableInit(void)
*
* Summary:
*      This function initializes the device Table
*
* Parameters and Returns:
*      None
*****************************************************************************************/
void deviceTableInit(void)
{
    for (uint8_t shortAddressindex = 0U; shortAddressindex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; shortAddressindex++)
    {
        memset(miwiDefaultRomOrRamParams->devTable[shortAddressindex].ieeeaddr, 0xFF, LONG_ADDR_LEN);
        memset(&miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut, 0xFF,
             sizeof(miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut)) ;
        miwiDefaultRomOrRamParams->devTable[shortAddressindex].capabilityInfo.Val = 0xFF;
    }

    for (uint8_t shortAddressindex = 0U; shortAddressindex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; shortAddressindex++)
    {
        memset(miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].ieeeaddr, 0xFF, LONG_ADDR_LEN);
        memset(&miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].actualTimeOut, 0xFF,
             sizeof(miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].actualTimeOut)) ;
        memset(&miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].currentTimeOut, 0xFF,
             sizeof(miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].currentTimeOut)) ;
        miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].capabilityInfo.Val = 0xFF;
    }
}

/************************************************************************************
* Function:
*      void sendForceLeaveNetwork(uint16_t orphAddress)
*
* Summary:
*      This function sends leave command
*
* Parameters/Returns:
*      status of the request
*****************************************************************************************/
void sendForceLeaveNetwork(uint16_t orphAddress)
{
    uint8_t dataLen = 0U, headerLen = 0U;
    uint8_t *dataPtr = NULL;
    MeshFrameHeader_t meshHeader;
	uint16_t nextHopAddr;
    buffer_t *buffer_header = NULL;
	
	if (orphAddress & ENDDEVICE_MASK)
	{
		nextHopAddr = orphAddress;
	}
	else
	{
		nextHopAddr = getNextHopAddr(orphAddress);
	}
	if (DEFAULT_NEXT_HOP_ADDR == nextHopAddr)
	{
		return;
	}
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

    /* Prepare the common header */
    prepareGenericHeader(MAX_HOP, myShortAddress, orphAddress, &meshHeader);

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    dataPtr[dataLen++] = (uint8_t)CMD_MESH_LEAVE;

    /* Initiate the frame transmission */
    if(!(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2U, (uint8_t*)&nextHopAddr, 0U, commandConfcb, buffer_header)))
    { 
        bmm_buffer_free(buffer_header);
    }
}

#endif


/************************************************************************************
* Function:
*      uint8_t    miwi_scan_duration_ticks(uint8_t ScanDuartion)
*
* Summary:
*      This function calculates the scan duration time in microseconds
*
* Parameters:
*      uint8_t ScanDuration - The maximum time to perform scan on single channel.
*
* Returns:
*      scan duration time in microseconds.
*****************************************************************************************/
static inline uint32_t miwi_scan_duration_ticks(uint8_t scan_duration)
{
    uint32_t scan_symbols;

    scan_symbols =  ABASESUPERFRAMEDURATION *((1<<scan_duration) + 1);
    return MiMAC_SymbolToTicks(scan_symbols);
}
#ifndef PAN_COORDINATOR
#ifndef ENDDEVICE
/************************************************************************************
* Function:
*      static void roleUpGradeReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void roleUpGradeReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer)
{
	checkLinkFailureAtNoAck(confstatus);
    (void)handle;
    (void)msgPointer;

}
/************************************************************************************
* Function:
*      void roleUpgradeTimerExpired(struct SYS_Timer_t *timer)
*
* Summary:
*      This callback function is called when role upgrade timer is expired, so
*      it tries for role upgrade
*
* Parameters/Returns:
*      struct SYS_Timer_t *timer - timer instant
*****************************************************************************************/
//void roleUpgradeTimerExpired(struct SYS_Timer_t *timer)
void roleUpgradeTimerExpired(uintptr_t context)
{
    uint8_t dataLen = 0U, headerLen = 0U;
    uint8_t *dataPtr = NULL;
    MeshFrameHeader_t meshHeader;
    uint16_t destiAddr = PAN_COORDINATOR_ADDRESS;
    buffer_t *buffer_header = NULL;
    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return;
    }
    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);

    if (NULL == dataPtr)
    {
        /* Handling to be done */
        return;
    }

    /* Prepare the common header */
    prepareGenericHeader(MAX_HOP, myShortAddress, destiAddr, &meshHeader);

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    dataPtr[dataLen++] = CMD_MESH_ROLE_UPGRADE_REQUEST;

    memcpy(&dataPtr[dataLen], myLongAddress, LONG_ADDR_LEN);
    dataLen += LONG_ADDR_LEN;

    /* Initiate the frame transmission */
    if(!(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2U, (uint8_t*)&myParentShortAddress, 0U, roleUpGradeReqConfcb, buffer_header)))
    { 
        bmm_buffer_free(buffer_header);
    }
}
#endif

/************************************************************************************
* Function:
*      static void scanDurationExpired(struct SYS_Timer_t *timer)
*
* Summary:
*      This callback function is called when scan timer is expired, so
*      it tries to scan the next channel if available
*
* Parameters/Returns:
*      struct SYS_Timer_t *timer - timer instant
*****************************************************************************************/
static void scanDurationExpired(uintptr_t context)
{
    uint8_t status;

    /* Remove the scanned channel from unscanned channels list */
    searchConf->unscannedChannels &= ~(1UL << currentChannel);

    status = ScanChannel(searchConf);
    if ( status != SUCCESS)
    {
        searchConf->status = status;
        /* Change state while completing search */
        meshCurrentState = SEARCH_COMPLETE;

        /* All channels were scanned. Send confirmation to the upper layer. */
        gSearchConfCallback(searchConf->resultSize, (uint8_t*)searchConf);
        gSearchConfCallback = NULL;
    }
}

/************************************************************************************
* Function:
*      static void beaconReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function starts the search timer if beacon is sent successfully
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void beaconReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer)
{
    if (SUCCESS == confstatus)
    {
        /* Start the search timer for scan Duration time */
        timerHandles.joinTimerBeaconReqConfcb.handler = &scanDurationExpired;
        timerHandles.joinTimerBeaconReqConfcb.timeout = (miwi_scan_duration_ticks(gScanDuration)/1000);
        timerHandles.joinTimerBeaconReqConfcb.interval = (miwi_scan_duration_ticks(gScanDuration)/1000);
        timerHandles.joinTimerBeaconReqConfcb.mode = SYS_TIME_SINGLE;
        
        uint8_t myData = 0U;
        timerHandles.joinTimerBeaconReqConfcbHandle = SYS_TIME_CallbackRegisterMS(&scanDurationExpired, (uintptr_t)&myData, timerHandles.joinTimerBeaconReqConfcb.interval, SYS_TIME_SINGLE);
        if(timerHandles.joinTimerBeaconReqConfcbHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
    }
    else
    {
        /* Beacon request is not sent successfully , then try again */
        uint8_t status = ScanChannel(searchConf);
        if ( status != SUCCESS)
        {
            searchConf->status = status;
            /* Change state while completing search */
            meshCurrentState = SEARCH_COMPLETE;

            /* All channels were scanned. Send confirmation to the upper layer. */
            gSearchConfCallback(searchConf->resultSize, (uint8_t*)searchConf);
            gSearchConfCallback = NULL;
        }
    }
        (void)handle;
    (void)msgPointer;
}

/************************************************************************************
* Function:
*      static uint8_t initiateBeaconReq(void)
*
* Summary:
*      This function sends the beacon request
*
* Parameters/Returns:
*      status of the request
*****************************************************************************************/
static uint8_t initiateBeaconReq(void)
{
    uint8_t dataLen = 0U, headerLen = 0U;
    uint8_t *dataPtr = NULL;
    MeshFrameHeader_t meshHeader;
    uint16_t broadcastAddr = MAC_BROADCAST_ADDR;
    buffer_t *buffer_header = NULL;
    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return MEMORY_UNAVAILABLE;
    }
    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
	if (NULL == dataPtr)
	{
		return MEMORY_UNAVAILABLE;
	}

    /* Prepare the common header */
    prepareGenericHeader(SINGLE_HOP, 0xFFFF, broadcastAddr, &meshHeader);

    /* Update the header for beacon request command */
    meshHeader.frameControl.addressSameAsMAC = 1;

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    dataPtr[dataLen++] = CMD_MESH_BEACON_REQUEST;

    /* Initiate the frame transmission */
    if (false == frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t*)&broadcastAddr, 0, beaconReqConfcb, buffer_header))
	{
        bmm_buffer_free(buffer_header);
		return MEMORY_UNAVAILABLE;
	}
	return SUCCESS;
}

/************************************************************************************
* Function:
*      static uint8_t ScanChannel(searchConf_t* searchConfirm)
*
* Summary:
*      This function does the channel scanning by initiating beacon req
*
* Parameters:
*      searchConf_t* searchConfirm - confirm of search
* Returns:
*      uint8_t - status of operation
*****************************************************************************************/
static uint8_t ScanChannel(searchConf_t* searchConfirm)
{
    bool channelSetStatus = false;

    uint8_t curChannel = 0U;
    /* Find the next unscanned channel to scan and initiate the scan, */
    for (curChannel = currentChannel; curChannel <= 26U; curChannel++)
    {
        if ((searchConfirm->unscannedChannels & (1UL << curChannel)) != 0)
        {
            /* Set the selected channel */
            currentChannel = curChannel;
            channelSetStatus = MiApp_Set(CHANNEL, &currentChannel);

            if (channelSetStatus)
            {
                return (initiateBeaconReq());
            }
        }
    }

    /* Change state while completing search */
    meshCurrentState = SEARCH_COMPLETE;

    /* All channels were scanned. Send confirmation to the upper layer. */
    gSearchConfCallback(searchConfirm->resultSize, (uint8_t*)searchConfirm);
    gSearchConfCallback = NULL;
    return SUCCESS;
}
#endif
#ifndef ENDDEVICE
/************************************************************************************
* Function:
*      static void commandConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function frees the memory used for the command sent
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void commandConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
{
    (void)handle;
    (void)status;
    (void)msgPointer;
}

/************************************************************************************
* Function:
*      static bool isCorrectIeeeAddr(const uint8_t *ieeeAddr)
*
* Summary:
*      checks whether the given address is a valid ieee address or not
*
* Parameters/Returns:
*      address
*****************************************************************************************/
bool isCorrectIeeeAddr(const uint8_t *ieeeAddr)
{
	uint64_t invalidIEEEAddr;
	/* Check if a valid IEEE address is available.
	0x0000000000000000 and 0xFFFFFFFFFFFFFFFF is persumed to be invalid */
	/* Check if IEEE address is 0x0000000000000000 */
	memset((uint8_t *)&invalidIEEEAddr, 0x00, LONG_ADDR_LEN);
	if (0 == memcmp((uint8_t *)&invalidIEEEAddr, ieeeAddr, LONG_ADDR_LEN))
	{
		return false;
	}

	/* Check if IEEE address is 0xFFFFFFFFFFFFFFFF */
	memset((uint8_t *)&invalidIEEEAddr, 0xFF, LONG_ADDR_LEN);
	if (0 == memcmp((uint8_t *)&invalidIEEEAddr, ieeeAddr, LONG_ADDR_LEN))
	{
		return false;
	}
    return true;
}

/************************************************************************************
* Function:
*      uint16_t assignAddress(uint8_t* ieeeAddress, CapabilityInfo_t capabilityinfo, uint8_t* timeout)
*
* Summary:
*      This function assigns new address if free entry is found, if entry is existing, then
       returns the corresponding address
*****************************************************************************************/
static uint16_t assignAddress(uint8_t* ieeeAddress, JoinWish_t joinwish, CapabilityInfo_t capabilityinfo, uint8_t* timeout)
{
    uint8_t shortAddressindex = 0xFFU;
    uint8_t loopIndex = 0U;

#ifdef PAN_COORDINATOR
     /* If Join wish is request for coordinator and capability matches with coord capability,
            then assign the available coordinator address to the device */
    if ((joinwish.bits.coordinator) && (DEVICE_TYPE_COORDINATOR == capabilityinfo.bits.deviceType))
    {
        for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
        {
            if (memcmp(miwiDefaultRomOrRamParams->coordTable[loopIndex].ieeeaddr, ieeeAddress, LONG_ADDR_LEN) == 0)
            {
                miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveCoordTimeoutSec;
                return ((uint16_t)(loopIndex << 8U));
            }
        }
        for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
        {
            if ((shortAddressindex == 0xFFU) && ( false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->coordTable[loopIndex].ieeeaddr)))
            {
                shortAddressindex = loopIndex;
                /* Assign new coordinator address to the device */
                memcpy(&miwiDefaultRomOrRamParams->coordTable[shortAddressindex].ieeeaddr, ieeeAddress, LONG_ADDR_LEN);
                miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveCoordTimeoutSec;
#if defined(ENABLE_NETWORK_FREEZER)
                /*Store coordinator table Information in Persistent Data Server */
                PDS_Store(PDS_COORDINATOR_TABLE_ID);
#endif
                return ((uint16_t)(shortAddressindex << 8U));
            }
        }
        /* If the joiner device is not already in table and no free entry available, then find the 
         stale entry which has '0' current time out and use the entry for the new device */
        for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfCoordinators; loopIndex++)
        {
            if (0U == miwiDefaultRomOrRamParams->coordTable[loopIndex].currentTimeOut)
            {
                shortAddressindex = loopIndex;
                /* Assign new coordinator address to the device */
                memcpy(&miwiDefaultRomOrRamParams->coordTable[shortAddressindex].ieeeaddr, ieeeAddress, LONG_ADDR_LEN);
                miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveCoordTimeoutSec;
#if defined(ENABLE_NETWORK_FREEZER)
                /*Store coordinator table Information in Persistent Data Server */
                PDS_Store(PDS_COORDINATOR_TABLE_ID);
#endif
                return ((uint16_t)(shortAddressindex << 8U));
            }
        }
    }
#endif

    /* if join wish as any of the device , then  assign ED address */
    if ((joinwish.Val & JOINWISH_ANY_MASK))
    {
        if (capabilityinfo.bits.rxonwhenIdle)
        {
            for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; loopIndex++)
            {
                if (memcmp(miwiDefaultRomOrRamParams->devTable[loopIndex].ieeeaddr, ieeeAddress, LONG_ADDR_LEN) == 0)
                {
                    miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveRxOnEdTimeoutSec;
                    return (myShortAddress + (RXONWHENIDLE_ED_ADDRESS_MASK + loopIndex));
                }
            }
            for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; loopIndex++)
            {
                if ((shortAddressindex == 0xFFU) && false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->devTable[loopIndex].ieeeaddr))
                {
                    shortAddressindex = loopIndex;
                    memcpy(&miwiDefaultRomOrRamParams->devTable[shortAddressindex].ieeeaddr[0], ieeeAddress, LONG_ADDR_LEN);
                    miwiDefaultRomOrRamParams->devTable[shortAddressindex].capabilityInfo = capabilityinfo;
                    miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveRxOnEdTimeoutSec;
#if defined(ENABLE_NETWORK_FREEZER)
                    /*Store device table Information in Persistent Data Server */
                    PDS_Store(PDS_DEVICE_TABLE_NONSLEEP_ID);
#endif
                    return (myShortAddress + (RXONWHENIDLE_ED_ADDRESS_MASK + shortAddressindex));
                }
            }
            /* If the joiner device is not already in table and no free entry available, then find the 
            stale entry which has '0' current time out and use the entry for the new device */
            for (loopIndex = 0U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; loopIndex++)
            {
                if (0U == miwiDefaultRomOrRamParams->devTable[loopIndex].currentTimeOut)
                {
                    shortAddressindex = loopIndex;
                    memcpy(&miwiDefaultRomOrRamParams->devTable[shortAddressindex].ieeeaddr[0], ieeeAddress, LONG_ADDR_LEN);
                    miwiDefaultRomOrRamParams->devTable[shortAddressindex].capabilityInfo = capabilityinfo;
                    miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->keepAliveRxOnEdTimeoutSec;
#if defined(ENABLE_NETWORK_FREEZER)
                    /*Store device table Information in Persistent Data Server */
                    PDS_Store(PDS_DEVICE_TABLE_NONSLEEP_ID);
#endif
                    return (myShortAddress + (RXONWHENIDLE_ED_ADDRESS_MASK + shortAddressindex));
                }
            }
        }
        else
        {
            for (loopIndex = 1; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
            {
                if (memcmp(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr, ieeeAddress, LONG_ADDR_LEN) == 0)
                {
                    memcpy(&miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].actualTimeOut, timeout, sizeof(uint32_t));
                    miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].actualTimeOut;
#if defined(ENABLE_NETWORK_FREEZER)
                    /*Store sleep device table Information in Persistent Data Server */ //ntc
                    PDS_Store(PDS_DEVICE_TABLE_SLEEP_ID);
#endif                    
                    return (myShortAddress + loopIndex);
                }
            }
            for (loopIndex = 1U; loopIndex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; loopIndex++)
            {
                if ((shortAddressindex == 0xFFU) && false == isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[loopIndex].ieeeaddr))
                {
                    shortAddressindex = loopIndex;
                    memcpy(&miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].ieeeaddr[0], ieeeAddress, LONG_ADDR_LEN);
                    miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].capabilityInfo = capabilityinfo;
                    memcpy(&miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].actualTimeOut, timeout, sizeof(uint32_t));
                    miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].currentTimeOut = miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].actualTimeOut;
#if defined(ENABLE_NETWORK_FREEZER)
                    /*Store sleep device table Information in Persistent Data Server */
                    PDS_Store(PDS_DEVICE_TABLE_SLEEP_ID);
#endif
                    return (myShortAddress + shortAddressindex);
                }
            }
        }
    }
    return 0xFFFFU;
}
#endif
#ifdef COORDINATOR
/************************************************************************************
* Function:
* bool MiApp_RoleUpgradeNotification_Subscribe(roleUpgrade_callback_t callback)
*
* Summary:
*      This function subscribes for role upgrade notification
*
* Description:
*      This is used to subscribe to notify the role upgrade. Upon successful role
*      upgrade, callback will be called with new short address
*
* PreCondition:
*      Protocol initialization has been done.
*
* Parameters:
*      roleUpgrade_callback_t callback - The callback routine which will be called upon
*                                               the role upgrade done
*
* Returns:
*      A boolean to indicates if the subscribtion is success or not
*
*****************************************************************************************/
bool MiApp_RoleUpgradeNotification_Subscribe(roleUpgrade_callback_t callback)
{
    if (NULL != callback)
    {
        roleUpgradeCb = callback;
        return true;
    }
    return false;
}
#endif

#ifndef PAN_COORDINATOR
/******************************************************************************
* Function:
*      static void rxOffEdKeepAliveTimerHandler(struct SYS_Timer_t *timer)
*
* Summary:
*      This function handles timer for sending data request
*
* Parameters:
*      struct SYS_Timer_t *timer - timer instant
*
* Returns:
*      None
******************************************************************************/
void rxOffEdKeepAliveTimerHandler(uintptr_t context)
{
	sendPollRequest();
}
/******************************************************************************
* Function:
*      static void sendPollRequest(void)
*
* Summary:
*      This function handles timer for sending data request
*
* Parameters:
*      struct SYS_Timer_t *timer - timer instant
*
* Returns:
*      None
******************************************************************************/
void sendPollRequest(void)
{
    uint8_t dataLen = 0U, headerLen = 0U;
    uint8_t *dataPtr = NULL;
    MeshFrameHeader_t meshHeader;
    uint16_t destiAddr = myShortAddress & COORD_MASK;
    buffer_t *buffer_header = NULL;
    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return;
    }
    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
    if (NULL == dataPtr)
    {
        /* Handling to be done */
        return;
    }
    else
    {
        uint32_t isTimePending = 0UL;
        SYS_TIME_TimerCounterGet(timerHandles.keepAliveTimerRxOffEdHandle,&isTimePending);
        if(isTimePending > 0UL)
        {
            SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
            SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
            
        }
        
        timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
        timerHandles.keepAliveTimerRxOffEd.timeout = miwiDefaultRomOrRamParams->dataRequestInterval * 1000UL;
        timerHandles.keepAliveTimerRxOffEd.interval = miwiDefaultRomOrRamParams->dataRequestInterval * 1000UL;
        timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
        uint8_t myData = 0U;
        timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
        if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
        {
            return;
        }
    }

    busyLock++;  

    /* Prepare the common header */
    prepareGenericHeader(SINGLE_HOP, myShortAddress, destiAddr, &meshHeader);

    meshHeader.frameControl.addressSameAsMAC = 1U;
    meshHeader.frameControl.ackRequest = 0U;

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    dataPtr[dataLen++] = (uint8_t)CMD_MESH_DATA_REQUEST;

    /* Initiate the frame transmission */
    if(false == frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t*)&destiAddr, 0, dataReqConfcb, buffer_header))
	{ 
        bmm_buffer_free(buffer_header);
		busyLock--;
        timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
        timerHandles.keepAliveTimerRxOffEd.timeout = miwiDefaultRomOrRamParams->dataRequestInterval * 1000;
        timerHandles.keepAliveTimerRxOffEd.interval = miwiDefaultRomOrRamParams->dataRequestInterval * 1000;
        timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
        uint8_t myData = 0U;
        timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
       if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
       {
           return;
        }
	}
    else
    {
         //do nothing 
    }
}

void startDataWaitIntervalTimer(void)
{
	uint32_t dataWaitInterval;
	if (currentChannel > 10U)
	{
		dataWaitInterval = 50UL;
	}
	else
	{
		dataWaitInterval = 100UL;
	}
    SYS_TIME_TimerStop(timerHandles.dataWaitIntervalTimerEdHandle);
    SYS_TIME_TimerDestroy(timerHandles.dataWaitIntervalTimerEdHandle);
    uint32_t isTimePending = 0UL;
    SYS_TIME_TimerCounterGet(timerHandles.keepAliveTimerRxOffEdHandle,&isTimePending);
    if(isTimePending > 0UL)
    {
        SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
        SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
    }
    else
    {
        SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
    }
	timerHandles.dataWaitIntervalTimerEd.handler = dataWaitIntervalTimerHandler;
	timerHandles.dataWaitIntervalTimerEd.timeout = dataWaitInterval;
	timerHandles.dataWaitIntervalTimerEd.interval = dataWaitInterval;
	timerHandles.dataWaitIntervalTimerEd.mode = SYS_TIME_SINGLE;
    uint8_t myData = 0U;
    timerHandles.dataWaitIntervalTimerEdHandle = SYS_TIME_CallbackRegisterMS(&dataWaitIntervalTimerHandler, (uintptr_t)&myData, timerHandles.dataWaitIntervalTimerEd.interval, SYS_TIME_SINGLE);
    if(timerHandles.dataWaitIntervalTimerEdHandle == SYS_TIME_HANDLE_INVALID)
    {
        return;
    }

	edInPollingState = true;
}
/************************************************************************************
* Function:
*      static void dataReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function for return status of data request to parent
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void dataReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer)
{
    if (SUCCESS == confstatus)
    {
		meshCurrentState = IN_NETWORK_STATE;
		startDataWaitIntervalTimer();
    }
    else
    {
            SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
            SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
		busyLock--;
		if (busyLock == 0U)
		{
            MiMAC_PowerState(POWER_STATE_DEEP_SLEEP);
		}
        timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
        timerHandles.keepAliveTimerRxOffEd.timeout = miwiDefaultRomOrRamParams->dataRequestInterval * 1000;
        timerHandles.keepAliveTimerRxOffEd.interval = miwiDefaultRomOrRamParams->dataRequestInterval * 1000;
        timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
        uint8_t myData = 0U;
        timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
       if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
       {
           return;
        }
    }
    checkLinkFailureAtNoAck(confstatus);
}
#endif

#ifndef ENDDEVICE
void keepAliveTimerHandler(void)
{
    uint8_t shortAddressindex = 0U;
#ifdef PAN_COORDINATOR
    for (shortAddressindex = 0U; shortAddressindex < miwiDefaultRomOrRamParams->numOfCoordinators; shortAddressindex++)
    {
        if (isCorrectIeeeAddr(miwiDefaultRomOrRamParams->coordTable[shortAddressindex].ieeeaddr))
        {
            if (miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut)
            {
                --miwiDefaultRomOrRamParams->coordTable[shortAddressindex].currentTimeOut;
            }
        }
    }
#endif
    for (shortAddressindex = 0; shortAddressindex < miwiDefaultRomOrRamParams->numOfRxOnEnddevices; shortAddressindex++)
    {
        if (isCorrectIeeeAddr(miwiDefaultRomOrRamParams->devTable[shortAddressindex].ieeeaddr))
        {
            if (miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut)
            {
                --miwiDefaultRomOrRamParams->devTable[shortAddressindex].currentTimeOut;
            }
        }
    }
    for (shortAddressindex = 1; shortAddressindex < miwiDefaultRomOrRamParams->numOfRxOffEnddevices; shortAddressindex++)
    {
        if (isCorrectIeeeAddr(miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].ieeeaddr))
        {
            if ((0 != miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].currentTimeOut) &&
            (--miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].currentTimeOut) == 0)
            {
                memset(miwiDefaultRomOrRamParams->sleepingDevTable[shortAddressindex].ieeeaddr, 0xFF, LONG_ADDR_LEN);
#if defined(ENABLE_NETWORK_FREEZER)
                /*Store sleep device table Information in Persistent Data Server */
                PDS_Store(PDS_DEVICE_TABLE_SLEEP_ID);
#endif
            }
        }
    }
}
#endif

#ifndef PAN_COORDINATOR
/************************************************************************************
* Function:
*      void sendKeepAlive(struct SYS_Timer_t *timer)
*
* Summary:
*      This function sends the keep alive
*
* Parameters/Returns:
*      status of the request
*****************************************************************************************/
void sendKeepAlive(uintptr_t context)
{
    uint8_t dataLen = 0U, headerLen = 0U;
    uint8_t *dataPtr = NULL;
    MeshFrameHeader_t meshHeader;
	uint16_t nextHopAddr = 0U;
    buffer_t *buffer_header = NULL;
	
	if (myShortAddress & ENDDEVICE_MASK)
	{
		nextHopAddr = myParentShortAddress;
	}
#ifdef COORDINATOR
	else
	{
		nextHopAddr = getNextHopAddr(myParentShortAddress);
	}
#endif
	if (DEFAULT_NEXT_HOP_ADDR == nextHopAddr)
	{
		return;
	}

    buffer_header =  bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	if (NULL == buffer_header)
	{
		return;
    }
    dataPtr = (uint8_t *)BMM_BUFFER_POINTER(buffer_header);
    if (NULL == dataPtr)
    {
        /* Handling to be done */
        return;
    }

    /* Prepare the common header */
    prepareGenericHeader(MAX_HOP, myShortAddress, myParentShortAddress, &meshHeader);

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    dataPtr[dataLen++] = (uint8_t)CMD_MESH_KEEP_ALIVE;

    memcpy(&dataPtr[dataLen], myLongAddress, LONG_ADDR_LEN);
    dataLen += LONG_ADDR_LEN;

    /* Initiate the frame transmission */
    if(!(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2U, (uint8_t*)&nextHopAddr, 0U, keepAliveReqConfcb, buffer_header)))
    {
        (void)context;
        bmm_buffer_free(buffer_header);
    }
}

/************************************************************************************
* Function:
*      static void keepAliveReqConfcb(uint8_t handle, miwi_status_t status, uint8_t* msgPointer)
*
* Summary:
*      This callback function
*
* Parameters/Returns:
*      uint8_t handle, miwi_status_t status, uint8_t* msgPointer
*****************************************************************************************/
static void keepAliveReqConfcb(uint8_t handle, miwi_status_t confstatus, uint8_t* msgPointer)
{
#ifndef PAN_COORDINATOR
    if (myShortAddress & ENDDEVICE_MASK)
    {
        checkLinkFailureAtNoAck(confstatus);
    }
#endif
}
#endif

/************************************************************************************
* Function:
*      static void edScanDurationExpired(struct SYS_Timer_t *timer)
*
* Summary:
*      This callback function is called when scan timer is expired, so
*      it tries to scan the next channel if available
*
* Parameters/Returns:
*      struct SYS_Timer_t *timer - timer instant
*****************************************************************************************/
static void edScanDurationExpired(uintptr_t context)
{
    noiseDetectionInProgress = false;
}

uint8_t MiApp_NoiseDetection(uint32_t ChannelMap, uint8_t ScanDuration, uint8_t DetectionMode, uint8_t *NoiseLevel)
{
    uint8_t i;
    uint8_t OptimalChannel = 0xFFU;
    uint8_t minRSSI = 0xFFU;
    uint32_t channelMask = 0x00000001UL;

    noiseDetectionInProgress = false;
    if( DetectionMode != NOISE_DETECT_ENERGY )
    {
        return 0xFFU;
    }
    i = 0U;
    while( i < 32U )
    {
        if( ChannelMap & MiMAC_GetPHYChannelInfo() & (channelMask << i) )
        {
            uint8_t RSSIcheck = 0U;//misra 9.1
            uint8_t maxRSSILocal = 0U;

            /* choose appropriate channel */
            MiApp_Set(CHANNEL, &i);

            /* Start the timer for scan Duration time */
            timerHandles.joinTimerEdScan.handler = edScanDurationExpired;
            timerHandles.joinTimerEdScan.timeout = (miwi_scan_duration_ticks(ScanDuration)/1000);
            timerHandles.joinTimerEdScan.interval = (miwi_scan_duration_ticks(ScanDuration)/1000);
            timerHandles.joinTimerEdScan.mode = SYS_TIME_SINGLE;
            
            uint8_t myData = 0U;
           timerHandles.joinTimerEdScanHandle = SYS_TIME_CallbackRegisterMS(&edScanDurationExpired, (uintptr_t)&myData, timerHandles.joinTimerEdScan.interval, SYS_TIME_SINGLE);
          if(timerHandles.joinTimerEdScanHandle == SYS_TIME_HANDLE_INVALID)
         {
            return 0U;
         }


            noiseDetectionInProgress = true;

            while(true)
            {
                MiMAC_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, ScanDuration);
//                RSSIcheck = energyLevel;
                if( RSSIcheck > maxRSSI )
                {
                    maxRSSI = RSSIcheck;
                }

                if( false == noiseDetectionInProgress)
                {
                    // if scan time exceed scan duration, prepare to scan the next channel
                    break;
                }
            }

            if( maxRSSI < minRSSI )
            {
                minRSSI = maxRSSI;
                OptimalChannel = i;
                if(NoiseLevel != NULL)
                {
                    *NoiseLevel = minRSSI;
                }
            }
        }
        i++;
    }

    return OptimalChannel;
}


bool MiApp_SubscribeLinkFailureCallback(LinkFailureCallback_t callback)
{
#ifndef PAN_COORDINATOR
    if (NULL != callback)
    {
        linkFailureCallback = callback;
        return true;
    }
#endif
    return false;
}

#if defined(ENABLE_NETWORK_FREEZER)
bool MiApp_SubscribeReConnectionCallback(ReconnectionCallback_t callback)
{
    if (NULL != callback)
    {
        reconnectionCallback = callback;
        return true;
    }
    return false;
}
#endif

#ifndef PAN_COORDINATOR
void dataWaitIntervalTimerHandler(uintptr_t context)
{
    edInPollingState = false;
        SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
        SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
	busyLock--;
	if (busyLock == 0U)
	{          
      MiMAC_PowerState(POWER_STATE_DEEP_SLEEP);
	}
        timerHandles.keepAliveTimerRxOffEd.handler = rxOffEdKeepAliveTimerHandler;
        timerHandles.keepAliveTimerRxOffEd.timeout = miwiDefaultRomOrRamParams->dataRequestInterval * 1000;
        timerHandles.keepAliveTimerRxOffEd.interval = miwiDefaultRomOrRamParams->dataRequestInterval * 1000;
        timerHandles.keepAliveTimerRxOffEd.mode = SYS_TIME_SINGLE;
    uint8_t myData = 0U;
timerHandles.keepAliveTimerRxOffEdHandle = SYS_TIME_CallbackRegisterMS(&rxOffEdKeepAliveTimerHandler, (uintptr_t)&myData, timerHandles.keepAliveTimerRxOffEd.interval, SYS_TIME_SINGLE);
if(timerHandles.keepAliveTimerRxOffEdHandle == SYS_TIME_HANDLE_INVALID)
{
      return;
}
        (void)context;
}


void checkLinkFailureAtNoAck(miwi_status_t status)
{
    if (NO_ACK == status)
    {
        if (0U != edLinkFailureAttempts && (--edLinkFailureAttempts) == 0U)
        {
            edInPollingState = false;
            SYS_TIME_TimerStop(timerHandles.keepAliveTimerRxOffEdHandle);
            SYS_TIME_TimerDestroy(timerHandles.keepAliveTimerRxOffEdHandle);
            meshCurrentState = DISCONNECTED;
            myShortAddress = 0xFFFFU;
#if defined(ENABLE_NETWORK_FREEZER)
            /*Store Network Information in Persistent Data Server */
            PDS_Store(MIWI_ALL_MEMORY_MEM_ID);
#endif
            if (NULL != linkFailureCallback)
            {
                linkFailureCallback();
            }
        }
    }
    else
    {
        edLinkFailureAttempts = miwiDefaultRomOrRamParams->edLinkFailureAttempts;
    }
}
#endif

/* Generates the jitter time based on the given jitter percentage and random
   percentage generation
   For ex: Given input timeout is 1000 and jitterPercent is 5, then range will be 950 to 1050
   and it is generated randomly 
*/
uint32_t generateJitterTimeout(uint32_t inputTimeout, uint8_t jitterPercent)
{
    uint32_t outputTimeout, jitterRange, minInterval;
    uint8_t randPercent;

    jitterRange = ((inputTimeout/1000) * 10 * jitterPercent);
    minInterval = inputTimeout - jitterRange;
    randPercent = (uint8_t)rand() % 100U;
    jitterRange <<= 1UL;
    outputTimeout = minInterval + ((jitterRange * (uint32_t)randPercent) / 100UL);

    return outputTimeout;
}

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