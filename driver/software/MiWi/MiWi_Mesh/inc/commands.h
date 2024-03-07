/**
* \file  commands.h
*
* \brief command handler interface
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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "config/default/definitions.h"

// #define CLI_ENABLED

#define AOK                               (0U)
#define ERR_UNKNOWN_CMD                   (1U)
#define ERR_INVALID_PARAM                 (2U)
#define ERR_MISSING_PARAM                 (3U)
#define ERR_INVALID_PARAM_TYPE            (4U)
#define ERR_PARAM_OUT_OF_RANGE            (5U)
#define ERR_INVALID_FORMAT                (6U)
#define ERR_SCRIPT_PARSING                (7U)
#define ERR_HEAP_FULL                     (8U)
#define ERR_INVALID_STATE                 (9U)
#define ERR_NO_RESOURCES                  (10U)
#define ERR_BLE_ERR                       (11U)
#define ERR_HAL_ERR                       (12U)
#define ERR_UNKNOWN_ERR                   (13U)

#define PENDING                           (0xFCU)
#define SCRIPT_DBG_BREAK                  (0xFDU)
#define NO_PROMPT                         (0xFEU)
#define SEND_PROMPT                       (0xFFU)

/*- Types ------------------------------------------------------------------*/
enum {
	APP_COMMAND_ID_NETWORK_INFO              = 0x01,
	APP_COMMAND_ID_IDENTIFY                  = 0x10,
	APP_COMMAND_ID_TOPOLOGY_SIMULATION_RESET = 0x20,
	APP_COMMAND_ID_SIMULATE_LINE_TOPOLOGY    = 0x21,
};

/*- Externs ------------------------------------------------------------------*/
extern SYS_Timer_t appCmdIdentifyDurationTimer;
extern SYS_Timer_t appCmdIdentifyPeriodTimer;

extern SYS_TIME_HANDLE appCmdIdentifyDurationTimerHandle;
extern SYS_TIME_HANDLE appCmdIdentifyPeriodTimerHandle;

/*- Prototypes -------------------------------------------------------------*/
void APP_CommandsInit(void);
void appCmdDataInd(RECEIVED_MESH_MESSAGE *ind);
void APP_CommandsByteReceived(uint8_t byte);
void appCmdDataRequest(uint16_t addr, uint8_t size, uint8_t* payload);

#ifdef CLI_ENABLED
void appPhyCmdProcessor_PhyStatusPrint(PHY_Retval_t status);
void appPhyCmdProcessor_PrintReturnCode(uint8_t status);
bool appPhyCmdProcessor_StrToBool(const char *str, bool *res);
bool appPhyCmdProcessor_StrToUint8(const char *str, uint8_t *res);
bool appPhyCmdProcessor_StrToInt8(const char *str, int8_t *res);
bool appPhyCmdProcessor_StrToUint8HexIp(const char *str, uint8_t *res);
bool appPhyCmdProcessor_StrToUint16(const char *str, uint16_t *res);
bool appPhyCmdProcessor_StrToUint16HexIp(const char *str, uint16_t *res);
bool appPhyCmdProcessor_StrToUint32(const char *str, uint32_t *res);
bool appPhyCmdProcessor_StrToUint64(const char *str, uint64_t *res);
bool appPhyCmdProcessor_StrToUint64DecIp(const char *str, uint64_t *res);
void appPhyCmdProcessor_PhyTrxStatusPrint(PHY_TrxStatus_t status);
void appPhyCmdProcessor_CmdDoneOk(void);
void appPhyCmdProcessor_CmdDoneFail(uint8_t err_code);
void MiWiCmdTable_Init(void);
#endif
#endif /* _COMMANDS_H_ */
