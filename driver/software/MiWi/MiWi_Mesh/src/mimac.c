/**
* \file  mimac.c
*
* \brief MAC Layer Abstraction for AT86RFx implementation
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

#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "config/default/definitions.h"

#define ENABLE_HW_AES
 
    
typedef struct
{
	uint8_t PayloadLen;
	uint8_t Payload[LARGE_BUFFER_SIZE];
    bool parseValid;
} RxBuffer_t;

/*- Variables --------------------------------------------------------------*/
static uint8_t MACCurrentChannel;
static API_UINT16_UNION MAC_PANID;
static API_UINT16_UNION myNetworkAddress;
static uint8_t IEEESeqNum;
static MACINIT_PARAM MACInitParams;
static DataConf_callback_t dataConfCallback;
PHY_CSMAMode_t csmaMode = CSMA_UNSLOTTED;
bool performRetry = true;
MAC_RECEIVED_PACKET  MACRxPacket;
extern bool txCallbackReceived;
/************************************************************************************
 * Function:
 *      bool MiMAC_SetAltAddress(uint8_t *Address, uint8_t *PANID)
 *
 * Summary:
 *      This function set the alternative network address and PAN identifier if
 *      applicable
 *
 * Description:
 *      This is the primary MiMAC interface for the protocol layer to
 *      set alternative network address and/or PAN identifier. This function
 *      call applies to only IEEE 802.15.4 compliant RF transceivers. In case
 *      alternative network address is not supported, this function will return
 *      FALSE.
 *
 * PreCondition:
 *      MiMAC initialization has been done.
 *
 * Parameters:
 *      uint8_t * Address -    The alternative network address of the host device.
 *      uint8_t * PANID -      The PAN identifier of the host device
 *
 * Returns:
 *      A boolean to indicates if setting alternative network address is successful.
 *
 * Example:
 *      <code>
 *      uint16_t NetworkAddress = 0x0000;
 *      uint16_t PANID = 0x1234;
 *      MiMAC_SetAltAddress(&NetworkAddress, &PANID);
 *      </code>
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/

bool MiMAC_SetPanId(uint8_t *PanId)
{
    PibValue_t pibval;
	MAC_PANID.v[0] = PanId[0];
	MAC_PANID.v[1] = PanId[1];
    pibval.pib_value_16bit = MAC_PANID.Val;
    PHY_PibSet(macPANId, &pibval); 
	return true;
}

bool MiMAC_SetAltAddress(uint8_t *Address)
{
    PibValue_t pibval;
	myNetworkAddress.v[0] = Address[0];
	myNetworkAddress.v[1] = Address[1];
    pibval.pib_value_16bit = myNetworkAddress.Val;
    PHY_PibSet(macShortAddress, &pibval);
	return true;
}




/************************************************************************************
 * Function:
 *      bool MiMAC_PowerState(uint8_t PowerState)
 *
 * Summary:
 *      This function puts the RF transceiver into sleep or wake it up
 *
 * Description:
 *      This is the primary MiMAC interface for the protocol layer to
 *      set different power state for the RF transceiver. There are minimal
 *      power states defined as deep sleep and operating mode. Additional
 *      power states can be defined for individual RF transceiver depends
 *      on hardware design.
 *
 * PreCondition:
 *      MiMAC initialization has been done.
 *
 * Parameters:
 *      uint8_t PowerState -   The power state of the RF transceiver to be set to.
 *                          The minimum definitions for all RF transceivers are
 *                          * POWER_STATE_DEEP_SLEEP RF transceiver deep sleep mode.
 *                          * POWER_STATE_OPERATE RF transceiver operating mode.
 * Returns:
 *      A boolean to indicate if chaning power state of RF transceiver is successful.
 *
 * Example:
 *      <code>
 *      // Put RF transceiver into sleep
 *      MiMAC_PowerState(POWER_STATE_DEEP_SLEEP);
 *      // Put MCU to sleep
 *      Sleep();
 *      // Wake up the MCU by WDT, external interrupt or any other means
 *
 *      // Wake up the RF transceiver
 *      MiMAC_PowerState(POWER_STATE_OPERATE);
 *      </code>
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
bool MiMAC_PowerState(uint8_t PowerState)
{
    PHY_TrxStatus_t trxStat;
    switch (PowerState)
    {
        case POWER_STATE_DEEP_SLEEP:
        {
            //;clear the WAKE pin in order to allow the device to go to sleep
            PHY_SleepMode_t sleepMode = SLEEP_MODE_1;
            PHY_TrxSleep(sleepMode);
        }
            break;

        case POWER_STATE_OPERATE:
        {
            PHY_TrxStatus_t trxStat = PHY_GetTrxStatus();
            if((trxStat == PHY_TRX_SLEEP) || (trxStat == PHY_TRX_DEEP_SLEEP))
            {
            PHY_TrxWakeup();
            }
        }
            break;

        default:
            //Handle exceptions if any
            break;
    }
    return true;
}

/************************************************************************************
     * Function:
     *      bool MiMAC_Set(mac_set_params_t id, uint8_t *value);
     *
     * Summary:
     *      This function sets the values
     *
     * Description:
     *      This is the primary MiMAC interface for the protocol layer to
     *      set the valuesr. for ex:Valid channel
     *      number are from 0 to 31. For different frequency band, data rate
     *      and other RF settings, some channels from 0 to 31 might be
     *      unavailable. Paramater offsetFreq is used to fine tune the center
     *      frequency across the frequency band. For transceivers that follow
     *      strict definition of channels, this parameter may be discarded.
     *      The center frequency is calculated as
     *      (LowestFrequency + Channel * ChannelGap + offsetFreq)
     *
     * PreCondition:
     *      Hardware initialization on MCU has been done.
     *
     * Parameters:
     *      set_params id -  The identifier of the value to be set
     *      value - value to be set
     *
     * Returns:
     *      A boolean to indicates if channel setting is successful.
     *
     * Example:
     *      <code>
     *      // Set center frequency to be exactly channel 12
     *      MiMAC_Set(CHANNEL, &channel);
     *      </code>
     *
     * Remarks:
     *      None
     *
     *****************************************************************************************/
bool MiMAC_Set(mac_set_params_t id, uint8_t *value)
{
    switch(id)
    {
      case MAC_CHANNEL:
      {
          PibValue_t pibval;
         if(*value > 26U) //TODO: this check is necessary here? as we may connect a sub-gig or a 2.4gig?
         {
           return false;
         }
          MACCurrentChannel = *value;
          pibval.pib_value_8bit = MACCurrentChannel;
          if(PHY_SUCCESS == PHY_PibSet(phyCurrentChannel,&pibval))
          {
            return true;   
          }
      }
      break;

     default:
        //Handle any exceptions
        break;
    }
    return false;
}

/************************************************************************************
 * Function:
 *      bool MiMAC_Init(MACINIT_PARAM initValue)
 *
 * Summary:
 *      This function initialize MiMAC layer
 *
 * Description:
 *      This is the primary MiMAC interface for the protocol layer to
 *      initialize the MiMAC layer. The initialization parameter is
 *      assigned in the format of structure MACINIT_PARAM.
 *
 * PreCondition:
 *      MCU initialization has been done.
 *
 * Parameters:
 *      MACINIT_PARAM initValue -   Initialization value for MiMAC layer
 *
 * Returns:
 *      A boolean to indicates if initialization is successful.
 *
 * Example:
 *      <code>
 *      MiMAC_Init(initParameter);
 *      </code>
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
bool MiMAC_Init(void)
{
    myPANID.Val = miwiDefaultRomOrRamParams->myPANID;
    MACInitParams.PAddress = myLongAddress;
    MACInitParams.actionFlags.bits.CCAEnable = 1;
    MACInitParams.actionFlags.bits.PAddrLength = MY_ADDRESS_LENGTH;
    #if defined(ENABLE_NETWORK_FREEZER)
    MACInitParams.actionFlags.bits.NetworkFreezer = 1;
    #else
    MACInitParams.actionFlags.bits.NetworkFreezer = 0;
    #endif
    MACInitParams.actionFlags.bits.RepeaterMode = 0;

	return true;
}


    /************************************************************************************
     * Function:
     *      BOOL MiMAC_SendPacket(  MAC_TRANS_PARAM transParam,
     *                              uint8_t *MACPayload, uint8_t MACPayloadLen,
                                    uint8_t msghandle,MiMacDataConf_callback_t ConfCallback)
     *
     * Summary:
     *      This function transmit a packet
     *
     * Description:
     *      This is the primary MiMAC interface for the protocol layer to
     *      send a packet. Input parameter transParam configure the way
     *      to transmit the packet.
     *
     * PreCondition:
     *      MiMAC initialization has been done.
     *
     * Parameters:
     *      MAC_TRANS_PARAM transParam -    The struture to configure the transmission way
     *      uint8_t * MACPaylaod -             Pointer to the buffer of MAC payload
     *      uint8_t MACPayloadLen -            The size of the MAC payload
     *      uint8_t msghandle                   Message handle
     *      MiMacDataConf_callback_t ConfCallback Callback function to be called once packet is sent
     *
     * Returns:
     *      A boolean to indicate if a packet has been received by the RF transceiver.
     *
     * Example:
     *      <code>
     *      MiMAC_SendPacket(transParam, MACPayload, MACPayloadLen, 6, callback);
     *      </code>
     *
     * Remarks:
     *      None
     *
     *****************************************************************************************/
bool MiMAC_SendPacket( MAC_TRANS_PARAM transParam,
         uint8_t *MACPayload,
         uint8_t MACPayloadLen, uint8_t msghandle,
         DataConf_callback_t ConfCallback, buffer_t* pMiMemClr)
{
    uint8_t headerLength;
    uint8_t loc = 0;
    uint8_t i = 0;
	uint8_t frameControl = 0;
    PHY_FrameInfo_t *pTxFrame = NULL;
    PHY_TrxStatus_t trxStat;
    buffer_t *buffer_header = NULL;
    buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
    if (buffer_header == NULL)
	{
		return false;
	}
    pTxFrame = (PHY_FrameInfo_t*)BMM_BUFFER_POINTER(buffer_header);
	if (pTxFrame == NULL)
	{
		return false;
	}

     pTxFrame->mpdu = (uint8_t*)pTxFrame + 12;

	#ifndef TARGET_SMALL
		bool IntraPAN;
	#endif

    if (transParam.flags.bits.broadcast != 0U)
    {
        transParam.altDestAddr = true;
    }

    // set the frame control in variable i
    if (transParam.flags.bits.packetType == PACKET_TYPE_COMMAND)
    {
        frameControl = 0x03U;
    } else if (transParam.flags.bits.packetType == PACKET_TYPE_DATA)
    {
        frameControl = 0x01U;
    }
    else
    {
        //do nothing
    }

    // decide the header length for different addressing mode
#ifndef TARGET_SMALL
    if ((transParam.DestPANID.Val == MAC_PANID.Val) && (MAC_PANID.Val != 0xFFFFU)) // this is intraPAN
#endif
    {
        headerLength = 5U;
        frameControl |= 0x40U;
#ifndef TARGET_SMALL
        IntraPAN = true;
#endif
    }
#ifndef TARGET_SMALL
    else
    {
        headerLength = 7U;
        IntraPAN = false;
    }
#endif

    if (transParam.altDestAddr)
    {
        headerLength += 2U;
    } else
    {
        headerLength += 8U;
    }

    if (transParam.altSrcAddr)
    {
        headerLength += 2U;
    } else
    {
        headerLength += 8U;
    }

    if ((transParam.flags.bits.ackReq != 0U) && transParam.flags.bits.broadcast == (uint8_t)false)
    {
        frameControl |= 0x20U;
    }

    // use PACKET_TYPE_RESERVE to represent beacon. Fixed format for beacon packet
    if (transParam.flags.bits.packetType == PACKET_TYPE_RESERVE)
    {
        frameControl = 0x00U;
        headerLength = 7U;
#if !defined(TARGET_SMALL)
        IntraPAN = false;
#endif
        transParam.altSrcAddr = true;
        transParam.flags.bits.ackReq = false;
    }

    // set packet length
		pTxFrame->mpdu[loc++] = MACPayloadLen+headerLength+2U; 
    // set frame control LSB
	pTxFrame->mpdu[loc++] = frameControl;

    // set frame control MSB
    if (transParam.flags.bits.packetType == PACKET_TYPE_RESERVE)
    {
		pTxFrame->mpdu[loc++] = 0x80U;
        // sequence number
		pTxFrame->mpdu[loc++] = IEEESeqNum++;
    } else
    {
        if (transParam.altDestAddr && transParam.altSrcAddr)
        {
			pTxFrame->mpdu[loc++] = 0x88U;

        } else if (transParam.altDestAddr && (!(transParam.altSrcAddr)))
        {
			pTxFrame->mpdu[loc++] = 0xC8U;
        } else if (!transParam.altDestAddr && transParam.altSrcAddr)
        {
			pTxFrame->mpdu[loc++] = 0x8CU;
        } else
        {
			pTxFrame->mpdu[loc++] = 0xCCU;
        }

        // sequence number
		pTxFrame->mpdu[loc++] = IEEESeqNum++;

        // destination PANID
		pTxFrame->mpdu[loc++] = transParam.DestPANID.v[0];
		pTxFrame->mpdu[loc++] = transParam.DestPANID.v[1];

        // destination address
        if (transParam.flags.bits.broadcast == 1U)
        {
			pTxFrame->mpdu[loc++] = 0xFFU;
			pTxFrame->mpdu[loc++] = 0xFFU;
        } else
        {
            if (transParam.altDestAddr)
            {
				pTxFrame->mpdu[loc++] = transParam.DestAddress[0];
				pTxFrame->mpdu[loc++] = transParam.DestAddress[1];
            } else
            {
                for (i = 0U; i < 8U; i++)
                {
					pTxFrame->mpdu[loc++] = transParam.DestAddress[i];
                }
            }
        }
    }

#ifndef TARGET_SMALL
    // source PANID if necessary
    if (IntraPAN == false)
    {
		pTxFrame->mpdu[loc++] = MAC_PANID.v[0];
		pTxFrame->mpdu[loc++] = MAC_PANID.v[1];
    }
#endif

    // source address
    if (transParam.altSrcAddr)
    {
		pTxFrame->mpdu[loc++] = myNetworkAddress.v[0];
		pTxFrame->mpdu[loc++] = myNetworkAddress.v[1];
    } else
    {
        for (i = 0U; i < 8U; i++)
        {
			pTxFrame->mpdu[loc++] = MACInitParams.PAddress[i];
        }
    }

#ifndef ENABLE_SECURITY
    // write the payload
    for (i = 0U; i < MACPayloadLen; i++)
    {
		pTxFrame->mpdu[loc++] = MACPayload[i];
    }
#endif 
    // set the trigger value
    if ((transParam.flags.bits.ackReq != 0U) && (transParam.flags.bits.broadcast == false))
    {
        i = 0x05U;
    } else
    {
        i = 0x01U; //misra 2.2
    }

	dataConfCallback = ConfCallback;
    msghandle = msghandle;  
    trxStat = PHY_GetTrxStatus();
    if(trxStat == PHY_TRX_SLEEP)
    {
    MiMAC_PowerState(POWER_STATE_OPERATE);
    Rx_On(false);
    trxStat = PHY_GetTrxStatus(); 
    }
    pTxFrame->buffer_header = buffer_header;

    if(PHY_SUCCESS == PHY_TxFrame(pTxFrame, csmaMode, performRetry))
    {
        if(pMiMemClr != NULL)
        {
            bmm_buffer_free(pMiMemClr);
        }
       return true;
    }
    else
    {
        txCallbackReceived = true;
        busyLock--;
        if(pMiMemClr != NULL)
        {
            bmm_buffer_free(pMiMemClr);
        }
        if(buffer_header != NULL)
        {
            bmm_buffer_free(buffer_header);
        }
        return false;
    }
}

/************************************************************************************
 * Function:
 *      void MiMAC_DiscardPacket(void)
 *
 * Summary:
 *      This function discard the current packet received from the RF transceiver
 *
 * Description:
 *      This is the primary MiMAC interface for the protocol layer to
 *      discard the current packet received from the RF transceiver.
 *
 * PreCondition:
 *      MiMAC initialization has been done.
 *
 * Parameters:
 *      None
 *
 * Returns:
 *      None
 *
 * Example:
 *      <code>
 *      if( true == MiMAC_ReceivedPacket() )
 *      {
 *          // handle the raw data from RF transceiver
 *
 *          // discard the current packet
 *          MiMAC_DiscardPacket();
 *      }
 *      </code>
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
void MiMAC_DiscardPacket(void)
{
	//re-enable buffer for next packets
//	if (BankIndex < BANK_SIZE)
//	{
//		RxBuffer[BankIndex].PayloadLen = 0;
//        memset(&RxBuffer[BankIndex].Payload, 0, LARGE_BUFFER_SIZE);
//	}
//    RxBuffer[BankIndex].parseValid = true;
//    printf("\r\n cleraing indx %d\r\n",BankIndex);
}

/************************************************************************************
 * Function:
 *      bool MiMAC_ReceivedPacket(void)
 *
 * Summary:
 *      This function check if a new packet has been received by the RF transceiver
 *
 * Description:
 *      This is the primary MiMAC interface for the protocol layer to
 *      check if a packet has been received by the RF transceiver. When a packet has
 *      been received, all information will be stored in the global variable
 *      MACRxPacket in the format of MAC_RECEIVED_PACKET;
 *
 * PreCondition:
 *      MiMAC initialization has been done.
 *
 * Parameters:
 *      None
 *
 * Returns:
 *      A boolean to indicate if a packet has been received by the RF transceiver.
 *
 * Example:
 *      <code>
 *      if( true == MiMAC_ReceivedPacket() )
 *      {
 *          // handle the raw data from RF transceiver
 *
 *          // discard the current packet
 *          MiMAC_DiscardPacket();
 *      }
 *      </code>
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/
bool MiMAC_ReceivedPacket(void)
{
    PHY_FrameInfo_t *rxFramePtr = NULL;
    buffer_t *buffer_header = NULL;
    uint8_t PayloadLen = 0U;
    buffer_header = qmm_queue_remove(&frameRxQueue, NULL);
    if(buffer_header == NULL)
    {
        return false;
    }
    rxFramePtr = (PHY_FrameInfo_t *)BMM_BUFFER_POINTER(buffer_header);
	if (rxFramePtr != NULL)
	{
		uint8_t addrMode;
		#ifndef TARGET_SMALL
		bool bIntraPAN = true;
	    PayloadLen = rxFramePtr->mpdu[0] + 2U; 
		if ((rxFramePtr->mpdu[1] & 0x40U) == 0U)
		{
			bIntraPAN = false;
		}
		#endif
		MACRxPacket.flags.Val = 0U;
		MACRxPacket.altSourceAddress = false;
		MACRxPacket.SourcePANID.Val = 0xFFFFU;

		//Determine the start of the MAC payload
		addrMode = rxFramePtr->mpdu[2] & 0xCCU;
		switch (addrMode)
		{
			case 0xC8: //short dest, long source
			// for P2P only broadcast allows short destination address
			if (rxFramePtr->mpdu[6] == 0xFFU && rxFramePtr->mpdu[7] == 0xFFU)
			{
				MACRxPacket.flags.bits.broadcast = 1U;
			}
			MACRxPacket.flags.bits.sourcePrsnt = 1U;

			#ifndef TARGET_SMALL
			if (bIntraPAN) // check if it is intraPAN
			#endif
			{
				#ifndef TARGET_SMALL
				MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[4];
				MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[5];
				#endif
				MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[8]);

				MACRxPacket.PayloadLen = PayloadLen - 19U;
				MACRxPacket.Payload = &(rxFramePtr->mpdu[16]);
			}
			#ifndef TARGET_SMALL
			else
			{
				MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[8];
				MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[9];
				MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[10]);
				MACRxPacket.PayloadLen = PayloadLen - 21U;
				MACRxPacket.Payload = &(rxFramePtr->mpdu[18]);
			}
			#endif

			break;

			case 0xCC: // long dest, long source
			MACRxPacket.flags.bits.sourcePrsnt = 1U;
			#ifndef TARGET_SMALL
			if (bIntraPAN) // check if it is intraPAN
			#endif
			{
				//rxFrame.flags.bits.intraPAN = 1;
				#ifndef TARGET_SMALL
				MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[4];
				MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[5];
				#endif
				MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[14]);
				MACRxPacket.PayloadLen = PayloadLen - 25U;
				MACRxPacket.Payload = &(rxFramePtr->mpdu[22]);
			}
			#ifndef TARGET_SMALL
			else
			{
				MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[14];
				MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[15];
				MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[16]);
				MACRxPacket.PayloadLen = PayloadLen - 27U;
				MACRxPacket.Payload = &(rxFramePtr->mpdu[24]);
			}
			#endif
			break;

			case 0x80: // short source only. used in beacon
			{
				MACRxPacket.flags.bits.broadcast = 1U;
				MACRxPacket.flags.bits.sourcePrsnt = 1U;
				MACRxPacket.altSourceAddress = true;
				MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[4];
				MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[5];
				MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[6]);
				MACRxPacket.PayloadLen = PayloadLen - 11U;
				MACRxPacket.Payload = &(rxFramePtr->mpdu[8]);
			}
			break;

			case 0x88: // short dest, short source
			{
				if (rxFramePtr->mpdu[6] == 0xFFU && rxFramePtr->mpdu[7] == 0xFFU)
				{
					MACRxPacket.flags.bits.broadcast = 1U;
				}
				MACRxPacket.flags.bits.sourcePrsnt = 1U;
				MACRxPacket.altSourceAddress = true;
				#ifndef TARGET_SMALL
				if (bIntraPAN == false)
				{
					MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[8];
					MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[9];
					MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[10]);
					MACRxPacket.PayloadLen = PayloadLen - 15U;
					MACRxPacket.Payload = &(rxFramePtr->mpdu[12]);
				} else
				#endif
				{
					MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[4];
					MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[5];
					MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[8]);
					MACRxPacket.PayloadLen = PayloadLen - 13U;
					MACRxPacket.Payload = &(rxFramePtr->mpdu[10]);
				}
			}
			break;

			case 0x8C: // long dest, short source
			{
				MACRxPacket.flags.bits.sourcePrsnt = 1U;
				MACRxPacket.altSourceAddress = true;
				#ifndef TARGET_SMALL
				if (bIntraPAN) // check if it is intraPAN
				#endif
				{
					#ifndef TARGET_SMALL
					MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[4];
					MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[5];
					#endif
					#if defined(PROTOCOL_MESH)
					MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[14]);
					#else
					MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[13]);
					#endif

					MACRxPacket.PayloadLen = PayloadLen - 19U;
					MACRxPacket.Payload = &(rxFramePtr->mpdu[16]);
				}
				#ifndef TARGET_SMALL
				else
				{
					MACRxPacket.SourcePANID.v[0] = rxFramePtr->mpdu[13];
					MACRxPacket.SourcePANID.v[1] = rxFramePtr->mpdu[14];
					MACRxPacket.SourceAddress = &(rxFramePtr->mpdu[15]);
					MACRxPacket.PayloadLen = PayloadLen - 21U;
					MACRxPacket.Payload = &(rxFramePtr->mpdu[18]);
				}
				#endif
			}
			break;


			case 0x08: //dest-short, source-none
			{
				if (rxFramePtr->mpdu[6] == 0xFFU && rxFramePtr->mpdu[7] == 0xFFU)
				{
					MACRxPacket.flags.bits.broadcast = 1U;
				}
				MACRxPacket.PayloadLen = PayloadLen - 10U;
				MACRxPacket.Payload = &(rxFramePtr->mpdu[7]);
			}
			break;

			// all other addressing mode will not be supported in P2P
			default:
            {
             	// not valid addressing mode or no addressing info discard packet
                bmm_buffer_free(buffer_header);
			    return false;   
            }
            break;
		}

		if (rxFramePtr->mpdu[1] & 0x08U)
		{
            bmm_buffer_free(buffer_header);
			return false;
		}

		// check the frame type. Only the data and command frame type
		// are supported. Acknowledgement frame type is handled in
		// AT96RF233 transceiver hardware.
		switch (rxFramePtr->mpdu[1] & 0x07U) // check frame type
		{
			case 0x01: // data frame
			MACRxPacket.flags.bits.packetType = PACKET_TYPE_DATA;
			break;
			case 0x03: // command frame
			MACRxPacket.flags.bits.packetType = PACKET_TYPE_COMMAND;
			break;
			case 0x00:
			// use reserved packet type to represent beacon packet
			MACRxPacket.flags.bits.packetType = PACKET_TYPE_RESERVE;
			break;
			default: // not support frame type
                bmm_buffer_free(buffer_header);
			    return false;
                break;
		}
		#ifndef TARGET_SMALL
		MACRxPacket.LQIValue = rxFramePtr->mpdu[PayloadLen - 2U];
		MACRxPacket.RSSIValue = rxFramePtr->mpdu[PayloadLen - 1U];
		#endif
        bmm_buffer_free(buffer_header);
		return true;
	}
	return false;
}
/************************************************************************************
     * Function:
     *      uint8_t MiMAC_ChannelAssessment(uint8_t AssessmentMode)
     *
     * Summary:
     *      This function perform the noise detection on current operating channel
     *
     * Description:
     *      This is the primary MiMAC interface for the protocol layer to
     *      perform the noise detection scan. Not all assessment modes are supported
     *      for all RF transceivers.
     *
     * PreCondition:
     *      MiMAC initialization has been done.
     *
     * Parameters:
     *      uint8_t AssessmentMode -   The mode to perform noise assessment. The possible
     *                              assessment modes are
     *                              * CHANNEL_ASSESSMENT_CARRIER_SENSE Carrier sense detection mode
     *                              * CHANNEL_ASSESSMENT_ENERGY_DETECT Energy detection mode
     *
     * Returns:
     *      A byte to indicate the noise level at current channel.
     *
     * Example:
     *      <code>
     *      NoiseLevel = MiMAC_ChannelAssessment(CHANNEL_ASSESSMENT_CARRIER_SENSE);
     *      </code>
     *
     * Remarks:
     *      None
     *
     *****************************************************************************************/
uint8_t MiMAC_ChannelAssessment(uint8_t AssessmentMode, uint8_t ScanDuration)
{   
    PHY_Retval_t retVal = PHY_FAILURE;
	if( AssessmentMode == CHANNEL_ASSESSMENT_ENERGY_DETECT)
	{ 
         retVal = PHY_EdStart(ScanDuration);
	}
	return ((uint8_t)retVal);
}

void PHY_EdEndCallback(uint8_t energyLevel)
{
    APP_Msg_T    appMsg;
    APP_Msg_T    *p_appmsg;
    p_appmsg = &appMsg;
    appStates = APP_STATE_FREQUENCY_AGILITY;
    p_appmsg->msgId = (uint8_t)APP_STATE_FREQUENCY_AGILITY;
    p_appmsg->msgData[0] = energyLevel + (uint8_t)PHY_GetRSSIBaseVal();
    OSAL_QUEUE_Send(&appData.appQueue, p_appmsg, 0);
    
}
/************************************************************************************
* Function:
*      uint32_t MiMAC_SymbolToTicks(uint32_t symbols)
*
* Summary:
*      This function converts symbol to ticks
*
* Description:
*      This is the primary MiMAC interface for the protocol layer to
*      convert symbol to ticks for all RF transceivers.
*
* Parameters:
*      uint8_t symbols - The number of symbols to convert
*
* Returns:
*      converted value in uint32.
*****************************************************************************************/
uint32_t MiMAC_SymbolToTicks(uint32_t symbols)
{
	return SYMBOLS_TO_TICKS(symbols);
}

/************************************************************************************
* Function:
*      uint32_t MiMAC_GetPHYChannelInfo(uint32_t supportedChannelMap)
*
* Summary:
*      This function gets the supported channel map
*
* Description:
*      This is the primary MiMAC interface for the protocol layer to
*      get the supported channel mask
*
* Parameters:
*      None
*
* Returns:
*      channel map uint32.
*****************************************************************************************/
uint32_t MiMAC_GetPHYChannelInfo(void)
{
	uint32_t channelMap = FULL_CHANNEL_MAP;
	return channelMap;
}

/************************************************************************************
 * Function:
 *      void PHY_TxDoneCallback(uint8_t status,, PHY_FrameInfo_t *frame)
 *
 * Summary:
 *      This function returns the confirmation of the packet sent
 *
 * PreCondition:
 *      MCU initialization has been done.
 *
 * Parameters:
 *      status -  status of the transmit operation
 *
 * Returns:
 *      None.
 *
 * Remarks:
 *      None
 *
 *****************************************************************************************/

void PHY_TxDoneCallback(PHY_Retval_t status, PHY_FrameInfo_t *frame)
{ 
    static miwi_status_t dataStatus;
    static uint8_t dataHandle = 0U;
    dataStatus = PhyToMiwi_Status(status);
    if((dataConfCallback != NULL) && (frame->buffer_header != NULL))
    {
	  dataConfCallback(dataHandle, dataStatus, frame->mpdu);
    }
    if(frame->buffer_header != NULL)
    {
       bmm_buffer_free(frame->buffer_header);
    }
    
}

void PHY_RxFrameCallback(PHY_FrameInfo_t *rxFrame)
{
    buffer_t *buffer_header;
    PHY_FrameInfo_t *rxFramePtr = NULL;
    uint8_t frameLen = 0U;
    uint8_t *framePtr = NULL;
    frameLen = rxFrame->mpdu[0];
	/* Allocate a buffer */
	buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
	/* Check for buffer availability */
	if (NULL == buffer_header) 
    {
		return;
	}
    rxFramePtr =  (PHY_FrameInfo_t *)BMM_BUFFER_POINTER(buffer_header);
    if (NULL == rxFramePtr)
	{
		return;
	}
    framePtr = (uint8_t *)rxFramePtr + LARGE_BUFFER_SIZE - frameLen;
    rxFramePtr->mpdu = framePtr;
	if((frameLen > 0U) && (NULL != rxFrame->mpdu))
    {
        (void)memcpy(rxFramePtr->mpdu, rxFrame->mpdu, frameLen);
    }
        qmm_queue_append(&frameRxQueue, buffer_header);
    if(rxFrame != NULL && rxFrame->buffer_header != NULL)
    {
        bmm_buffer_free(rxFrame->buffer_header);
    }
    MiMac_PostTask(false);

}
#ifndef PHY_AT86RF212B
bool MiMAC_SetPower(int8_t outputPower)
{
    if (PHY_SUCCESS == PHY_ConfigTxPwr(false, outputPower))
    {
        return true;
    }  
    else
    {
        return false;
    }
}
#endif


