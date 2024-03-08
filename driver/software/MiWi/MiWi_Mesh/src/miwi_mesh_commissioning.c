/**
* \file  miwi_mesh_commissioning.c
*
* \brief MiWi Mesh Protocol Commissioning implementation
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

/************************ Macro Definitions **********************************/
#define BYTE_TO_BITS            8U
#define JOINER_ID_LEN_BIT_POS   7U

/************************ Static Prototypes **********************************/
static uint16_t crc16(uint16_t crc, uint8_t dataByte, Crctype_t crcType);
static uint16_t crc16Compute(uint8_t *data, uint16_t len, Crctype_t crcType);
#ifdef PAN_COORDINATOR
static uint8_t initiateNetworkBloomUpdate(void);
#endif

/************************ Type Definitions **********************************/
/************************ Static Declarations **********************************/
#if defined(PAN_COORDINATOR)
static uint8_t NumOfCommnDevices = 0;
#endif
bool bloomFilterAutoJoin = true;

/************************ Function Definitions **********************************/

#ifdef PAN_COORDINATOR
/************************************************************************************
* Function:
*      bool MiApp_Commissioning_AddNewDevice(uint64_t joinerAddress, bool triggerBloomUpdate)
*
* Summary:
*      This function adds the given address into the commissioning device address list
*      if free entry available
*
* Parameters:
*      uint8_t* joinerAddress - the ieee address to be added.
*      bool triggerBloomUpdate - if set to true then bloom update is sent immediately
*
* Returns:
*      true if found, false otherwise.
*****************************************************************************************/
bool MiApp_Commissioning_AddNewDevice(uint64_t joinerAddress, bool triggerBloomUpdate)
{
    if (miwiDefaultRomOrRamParams->maxNumOfDevicesInNetwork > NumOfCommnDevices)
    {
        miwiDefaultRomOrRamParams->commDevTable[NumOfCommnDevices].deviceAddress = joinerAddress;
        NumOfCommnDevices++;
        if (triggerBloomUpdate)
        {
            generateBloomFilterValue(bloomFilterValue, 0, BLOOM_FILTER_SIZE);
#if defined(ENABLE_NETWORK_FREEZER)
            /*Store Commissioning Table Information in Persistent Data Server */
            PDS_Store(PDS_COMM_DEVICE_TABLE_ID);
#endif
        }
        return true;
    }
    return false;
}
#endif

/************************************************************************************
* Function:
*     static uint16_t crc16(uint16_t crc, uint8_t dataByte, Crctype_t crcType)
*
* Summary:
*      This function calculates CRC 16 based on ANSI or CCITT for single byte
*
* Parameters:
*      uint16_t crc - Initial value of CRC
*      uint8_t dataByte - data to be CRC'ed
*      Crctype_t crcType - type of CRC polynomial
*
* Returns:
*      calculated CRC
*****************************************************************************************/
static uint16_t crc16(uint16_t crc, uint8_t dataByte, Crctype_t crcType)
{
    uint16_t crcPoly;
    if (CRC_ANSI == crcType)
    {
        crcPoly = CRC_16_ANSI;
    }
    else if (CRC_CCITT == crcType)
    {
        crcPoly = CRC_16_CCITT;
    }
    else
    {
        return 0U; /*unsupported crc-16 type*/
    }

    crc ^= (uint16_t)dataByte << 8U;

    for (uint8_t i= 0U; i < 8U; ++i)
    {
        if (crc & 0x8000U)
            crc = (crc << 1U) ^ crcPoly;
        else
            crc = (crc << 1U);
    }
    return crc;
}

/************************************************************************************
* Function:
*     static uint16_t crc16(uint16_t crc, uint8_t dataByte, Crctype_t crcType)
*
* Summary:
*      This function calculates CRC 16 based on ANSI or CCITT for a given buffer
*
* Parameters:
*      uint16_t len - Length of the data buffer
*      uint8_t* data - pointer to data buffer
*      Crctype_t crcType - type of CRC polynomial
*
* Returns:
*      calculated CRC
*****************************************************************************************/
static uint16_t crc16Compute(uint8_t *data, uint16_t len, Crctype_t crcType)
{
    /* Initial value of CRC*/
    uint16_t crc = 0U;
    uint16_t i = 0U;

    /* Caculate the CRC for the given length*/
    for (i = 0U; i < len; ++i)
    {
        crc = crc16(crc, *data++, crcType);
    }
    return crc;
}

/************************************************************************************
* Function:
*      bool bloomFilterAddressCheck(uint8_t* bloomValue, uint8_t* address)
*
* Summary:
*      This function checks whether the given address is available in the bloom filter
*
* Parameters:
*      uint8_t* address - the ieee address to be checked.
*
* Returns:
*      true if found, false otherwise.
*****************************************************************************************/
bool bloomFilterAddressCheck(uint8_t* bloomValue, uint8_t bloomFilterLen, uint8_t* addressToCheck)
{
	if (!bloomFilterAutoJoin)
	{
		/* Bloom Filter is checked for the given IEEE address, If found true should be returned,
				 otherwise false is returned */
		uint8_t hash1Bit, hash2Bit, filterLenBits;
		uint16_t hash1, hash2;
		uint8_t hash1_bit_match, hash2_bit_match;

		filterLenBits = (bloomFilterLen * BYTE_TO_BITS) - 1U;

		hash1 = crc16Compute(addressToCheck, 8U, CRC_CCITT);
		hash2 = crc16Compute(addressToCheck, 8U, CRC_ANSI);

		hash1Bit = (uint8_t)(hash1 % filterLenBits);
		hash2Bit = (uint8_t)(hash2 % filterLenBits);

		/*Find which byte and bit of the bloom_filter_buf needs to be updated and update it*/
		hash1_bit_match = bloomValue[bloomFilterLen - ((hash1Bit / BYTE_TO_BITS) + 1U)] & (1U << (hash1Bit % BYTE_TO_BITS));
		hash2_bit_match = bloomValue[bloomFilterLen - ((hash2Bit / BYTE_TO_BITS) + 1U)] & (1U << (hash2Bit % BYTE_TO_BITS));

		if ((hash1_bit_match != 0U) && (hash2_bit_match != 0U))
		{
			//printf("\n\nBloom filter Verification Success :Hash1bit:%d,Hash2bit %d",hash1Bit,hash2Bit);
			return true;
		}

		//printf("\n\nBloom filter Verification Failed :Hash1bit:%d,Hash2bit %d",hash1Bit,hash2Bit);
		return false;
	}
	return true;
}

#if defined(PAN_COORDINATOR)
/************************************************************************************
* Function:
*      void generateBloomFilterValue(uint8_t *bloomFilterBuf, uint8_t joinerAddressLenMode, uint8_t bloomFilterLen)
*
* Summary:
*      This function generates the bloom filter value by hashing all the device addresses in the commnDeviceTable
*
* Parameters:
*      uint8_t* bloomFilterBuf - pointer where the bloom filter value to be stored.
*      uint8_t joinerAddressLenMode - address len mode either 0 for long address or 1 for short address
*      uint8_t bloomFilterLen - length of bloom filter value
*
* Returns:
*      None
*****************************************************************************************/
void generateBloomFilterValue(uint8_t *bloomFilterBuf, uint8_t joinerAddressLenMode, uint8_t bloomFilterLen)
{
    uint8_t hash1Bit, hash2Bit, filterLenBits, deviceAddressLen, i;

    uint16_t hash1, hash2;
    uint8_t *joinerAddr;
    filterLenBits = (bloomFilterLen * BYTE_TO_BITS) - 1;

    *bloomFilterBuf = joinerAddressLenMode << JOINER_ID_LEN_BIT_POS;

    if (joinerAddressLenMode)
    {
        deviceAddressLen = 2; /* 24bits //note: this is not trx16 */
    }
    else
    {
        deviceAddressLen = 8; /* eui64 */
    }

    /* find the hash of the devicesAddresses */
    for (i = 0U; i < NumOfCommnDevices; i++)
    {
        joinerAddr = ((uint8_t*)&(miwiDefaultRomOrRamParams->commDevTable[i].deviceAddress)) + 8 - deviceAddressLen;
        hash1 = crc16Compute(joinerAddr, deviceAddressLen, CRC_CCITT);
        hash2 = crc16Compute(joinerAddr, deviceAddressLen, CRC_ANSI);
        hash1Bit = (hash1 % filterLenBits);
        hash2Bit = (hash2 % filterLenBits);

        /*Find which byte and bit of the bloomFilterBuf needs to be updated and update it*/
        bloomFilterBuf[bloomFilterLen - ((hash1Bit /BYTE_TO_BITS) + 1)] |= (1 << (hash1Bit % BYTE_TO_BITS));
        bloomFilterBuf[bloomFilterLen - ((hash2Bit /BYTE_TO_BITS) + 1)] |= (1 << (hash2Bit % BYTE_TO_BITS));
    }
    /* Bloom Filter is updated, so update this to entire network */
    initiateNetworkBloomUpdate();
}
#endif

#ifdef PAN_COORDINATOR
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
*      static uint8_t initiateNetworkBloomUpdate(void)
*
* Summary:
*      This function sends the Mesh Device authentication bloom update command
*
* Parameters/Returns:
*      status of the request
*****************************************************************************************/
static uint8_t initiateNetworkBloomUpdate(void)
{
    uint8_t dataLen = 0U, headerLen = 0U;
    uint8_t *dataPtr = NULL;
    MeshFrameHeader_t meshHeader;
    uint16_t broadcastAddr = 0xFFFFU;
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
    prepareGenericHeader(MAXIMUM_HOP, myShortAddress, MESH_BROADCAST_TO_COORDINATORS, &meshHeader);

    /* Construct the general frame based on mesh header information */
    headerLen = dataLen = generalFrameConstruct(&meshHeader, dataPtr);

    dataPtr[dataLen++] = CMD_MESH_DEVICEAUTH_BLOOMUPDATE;
    dataPtr[dataLen++] = BLOOM_FILTER_SIZE;
    memcpy(&dataPtr[dataLen], bloomFilterValue, BLOOM_FILTER_SIZE);
    dataLen += BLOOM_FILTER_SIZE;

    /* Initiate the frame transmission */
//    return (frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t*)&broadcastAddr, 0, commandConfcb));//tbc
    if(frameTransmit(&meshHeader, headerLen, dataLen-headerLen, dataPtr, 2, (uint8_t*)&broadcastAddr, 0, commandConfcb, buffer_header))
    {
//        bmm_buffer_free(buffer_header);
        return true;
    }
    else
    {
        bmm_buffer_free(buffer_header);
        return false;
    }
}
#endif

/************************************************************************************
* Function:
*      void handleCommissiongMessage(MeshFrameHeader_t *meshHeader, uint8_t* payload)
*
* Summary:
*      This function parses and handles all commissioning related messages
*
* Parameters:
*      uint8_t* meshHeader - the received mesh header.
*      uint8_t* payload - command payload
*
* Returns:
*      None
*****************************************************************************************/
void handleCommissiongMessage(MeshFrameHeader_t *meshHeader, uint8_t* payload)
{
    switch ((MeshCmdIdentifier_t)payload[0])
    {
        case CMD_MESH_DEVICEAUTH_BLOOMUPDATE:
        {
            if (meshHeader->srcAddr == PAN_COORDINATOR_ADDRESS)
            {
                if (BLOOM_FILTER_SIZE == payload[1])
                {
                    memcpy(bloomFilterValue,&payload[2],BLOOM_FILTER_SIZE);
#if defined(ENABLE_NETWORK_FREEZER)
                    /*Store bloom filter value in Persistent Data Server */
                    PDS_Store(PDS_BLOOM_VALUE_ID);
#endif
                }
                else
                {
                    /* Bloom Filter Size mismatch */
                }
            }
        }
        break;

        default:
            //Handle exceptions if any
            break;
    }
}