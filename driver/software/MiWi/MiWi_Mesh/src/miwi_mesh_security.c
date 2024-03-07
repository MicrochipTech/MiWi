/**
* \file  miwi_mesh_security.c
*
* \brief MiWi Mesh Protocol Security Handling implementation
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

#ifdef MESH_SECURITY
/************************ Macro Definitions **********************************/
#define NWK_SECURITY_KEY_SIZE         16U

/************************ Static Prototypes **********************************/
/************************ Type Definitions **********************************/
/************************ Static Declarations **********************************/
API_UINT32_UNION meshOutgoingFrameCounter;

/************************ Function Definitions **********************************/
static uint8_t getMicLength(uint8_t securityLevel);
static void createNonce(uint8_t *ieeeAddr, uint8_t *frameCnt,uint8_t securityLevel, uint8_t *nonce);
/************************************************************************************
 * Function:
 *      uint8_t* keyDetermineProcedure(uint8_t macSourceAddrLen)
 *
 * Summary:
 *      This function determines which key to be used for the current security procedure
 *
 * Description:
 *      returns the key to be used
 *
 * PreCondition:
 *      None
 *
 * Parameters:
 *      uint8_t macSourceAddrLen - source address of the mac frame
 *
 * Returns:
 *      pointer to the key
 * Remarks:
 *      None
 *
 *****************************************************************************************/
uint8_t* keyDetermineProcedure(uint8_t macAddrLen)
{
    /* Determine which key should be used for outgoing security frame */
    if ((meshCurrentState == IN_NETWORK_STATE) || (meshCurrentState == IN_NETWORK_PARTIAL_STATE))
    {
        /* If destination mac address if transmit frame or if source mac address is Extended address,
        then public key should be used since the corresponding device do not have network key yet */
        if (macAddrLen == LONG_ADDR_LEN)
        {
            return miwiDefaultRamOnlyParams->miwiPublicKey;
        }
        else
        {
            return miwiDefaultRamOnlyParams->miwiNetworkKey;
        }
    }
    else
    {
        /* If device is not in network state , it will use the public key*/
        return miwiDefaultRamOnlyParams->miwiPublicKey;
    }
}

/************************************************************************************
 * Function:
 *      static uint8_t getMicLength(uint8_t securityLevel)
 *
 * Summary:
 *      This function Calculates the length of the MIC
 *
 * Description:
 *      returns the length of the MIC depending on the given security level
 *
 * PreCondition:
 *      None
 *
 * Parameters:
 *      uint8_t securityLevel Security Level of current frame
 *
 * Returns:
 *      Length of MIC in octets.
 * Remarks:
 *      None
 *
 *****************************************************************************************/
static uint8_t getMicLength(uint8_t securityLevel)
{
    uint8_t micLen = 0;

    switch (securityLevel)
    {
        case 1:
        case 5:
            micLen = LEN_MIC_32;
        break;

        case 2:
        case 6:
            micLen = LEN_MIC_64;
        break;

        case 3:
        case 7:
            micLen = LEN_MIC_128;
        break;

        default:
            //Handle exceptions if any
            break;
    }
    return micLen;
}

/************************************************************************************
 * Function:
 *      static void createNonce(uint8_t *ieeeAddr, uint8_t *frameCnt,
 *        uint8_t securityLevel, uint8_t *nonce)
 *
 * Summary:
 *      This function creates Nonce which is to be needed for CCM*
 *
 * Description:
 *      This function is used to create the nonce using frame counter for
 * replay protection with unique IEEE address and security levels
 *
 * Parameters:
 *   ieeeAddr IEEE Address of the destination
 *   frameCnt Frame Counter
 *   securityLevel Security Level
 *   nonce pointer to store the nonce
 *
 * Returns:
 *      None
 * Remarks:
 *      None
 *
 *****************************************************************************************/
static void createNonce(uint8_t *ieeeAddr, uint8_t *frameCnt,
        uint8_t securityLevel, uint8_t *nonce)
{
    uint8_t *ptr;
    uint8_t *noncePtr = &nonce[1];

    /* Fill Source IEEE Address */
    ptr = ieeeAddr;
    ptr += LONG_ADDR_LEN - 1;
    for (uint8_t i = 0; i < LONG_ADDR_LEN; i++)
    {
        *noncePtr++ = *ptr--;
    }
    /* Fill FrameCounter */
    ptr = frameCnt;
    ptr += FRAME_COUNTER_LEN - 1;
    for (uint8_t i = 0; i < FRAME_COUNTER_LEN; i++)
    {
        *noncePtr++ = *ptr--;
    }
    /* Fill Security Level */
    *noncePtr = securityLevel;
}

/************************************************************************************
 * Function:
 *      uint8_t secureFrame(MeshFrameHeader_t* meshHeader, uint8_t meshFrameHeaderLen,
 *                    uint8_t meshFramePayloadLen, uint8_t* meshframe, uint8_t* key)
 *
 * Summary:
 *      This function secures the outgoing Mesh Frames
 *
 * Description:
 *      secures the outgoing mesh frame based on CCM*, and returns the incrasedFrameLen
 *      and stores the secured frame in the given frame pointer itself.
 *
 * PreCondition:
 *      None
 *
 * Parameters:
 *      MeshFrameHeader_t* meshHeader - header of mesh frame to be secured
 *      uint8_t meshFrameHeaderLen - header length of mesh frame to be secured
 *      uint8_t meshFramePayloadLen - payload length of mesh frame to be secured
 *      uint8_t* meshframe - mesh frame to be secured
 *      uint8_t* key - key to be used for securing the frame
 *
 * Returns:
 *      increase in framelen in octets or 0xFF if it is failure
 * Remarks:
 *      None
 *
 *****************************************************************************************/
uint8_t secureFrame(MeshFrameHeader_t* meshHeader, uint8_t meshFrameHeaderLen,
                              uint8_t meshFramePayloadLen, uint8_t* meshframe, uint8_t* key)
{
    uint8_t nonce[AES_BLOCKSIZE] = {0};
    uint8_t enc_data[125+16];
    STB_Ccm_t status;
    /* Get the MIC length based on given security level*/
    uint8_t micLen = getMicLength(meshHeader->meshAuxSecHeader.securityLevel);

    /* Create Nonce */
    createNonce(myLongAddress, (uint8_t *)&meshOutgoingFrameCounter, meshHeader->meshAuxSecHeader.securityLevel, nonce);

    /* Copy header to enc_data where security operation will be done */
    memcpy(enc_data, meshframe, meshFrameHeaderLen);

    /* Append payload */
    memcpy(&enc_data[meshFrameHeaderLen], meshframe+meshFrameHeaderLen, meshFramePayloadLen);

    /* Make sure Transceiver is wakeup */
    MiMAC_PowerState(POWER_STATE_OPERATE);

    /* Initiate security processing */
    status = STB_CcmSecure(enc_data,  /* plain text header (string a) and payload concatenated */
            nonce,                  /*  Nonce */
            key,                    /*security_key */
            meshFrameHeaderLen,     /* plain text header length */
            meshFramePayloadLen,    /* Length of payload to be encrypted */
            meshHeader->meshAuxSecHeader.securityLevel,  /* security level **/
            AES_DIR_ENCRYPT);

    if (status == STB_CCM_OK)
    {
        /* Increment outgoing frame counter */
        meshOutgoingFrameCounter.Val++;
#if defined(ENABLE_NETWORK_FREEZER)
		/*Store security counter Information in Persistent Data Server */
        PDS_Store(PDS_SECURITY_COUNTER_ID);
#endif

        /* Replace original payload by secured payload */
        memcpy(meshframe+meshFrameHeaderLen, &enc_data[meshFrameHeaderLen],
                    meshFramePayloadLen + micLen);
        /* return the increase in frame length */
        return micLen;
    }
    else
    {
        /* Failure in security processing */
        return SECURITY_FAILURE;
    }
}

/************************************************************************************
 * Function:
 *      uint8_t unsecureFrame(MeshFrameHeader_t* meshHeader, uint8_t* macSrcAddr, uint8_t meshFrameHeaderLen,
 *                                           uint8_t meshFramePayloadLen, uint8_t* meshframe, uint8_t* key)
 *
 * Summary:
 *      This function unsecures the incoming Mesh Frames
 *
 * Description:
 *      unsecures the incoming mesh frame based on CCM*, and returns the decrease in frame length
 *      and stores the secured frame in the given frame pointer itself.
 *
 * PreCondition:
 *      None
 *
 * Parameters:
 *      MeshFrameHeader_t* meshHeader - header of mesh frame to be secured
 *      uint8_t meshFrameHeaderLen - header length of mesh frame to be secured
 *      uint8_t meshFramePayloadLen - payload length of mesh frame to be secured
 *      uint8_t* meshframe - mesh frame to be secured
 *      uint8_t* key - key to be used for securing the frame
 *      uint8_t* macSrcAddr - source address of the received frame
 *
 * Returns:
 *      decrease in frame length in octets or 0xFF if it is failure
 * Remarks:
 *      None
 *
 *****************************************************************************************/
uint8_t unsecureFrame(MeshFrameHeader_t* meshHeader, uint8_t* macSrcAddr, uint8_t meshFrameHeaderLen,
        uint8_t meshFramePayloadLen, uint8_t* meshframe, uint8_t* key)
{
    uint8_t nonce[AES_BLOCKSIZE] = {0};
    uint8_t encrypPayloadLen = 0;

    /* Get the MIC length based on given security level*/
    uint8_t micLen = getMicLength(meshHeader->meshAuxSecHeader.securityLevel);

    /* Create Nonce */
    createNonce(macSrcAddr, (uint8_t *)&meshHeader->meshAuxSecHeader.frameCounter,
    meshHeader->meshAuxSecHeader.securityLevel, nonce);

    /* Length of the actual encrypted payload */
    encrypPayloadLen = meshFramePayloadLen - micLen;

    /* Make sure Transceiver is wakeup */
	MiMAC_PowerState(POWER_STATE_OPERATE);

    if (STB_CcmSecure(meshframe, /* plaintext header (string a) and payload concatenated */
                nonce,  /* Nonce */
                key,    /* security_key */
                meshFrameHeaderLen,    /* plaintext header length */
                encrypPayloadLen,    /* Length of payload to be encrypted */
                meshHeader->meshAuxSecHeader.securityLevel, /* security level*/
                AES_DIR_DECRYPT)
                == STB_CCM_OK)
    {
        /* return the decrease in payload length after securing processing */
        return micLen;
    }
    else
    {
        return SECURITY_FAILURE;
    }
}
#endif /*#ifdef MESH_SECURITY*/
