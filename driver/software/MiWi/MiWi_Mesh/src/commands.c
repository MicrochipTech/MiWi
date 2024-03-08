/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Microchip Technology pvt Ltd

  @File Name
    MiWi_cmd_processor.c

  @Summary
    Command Interface file for PHY & App layer functions

  @Description
    Command Interface file for PHY & App layer functions
 */
 /*******************************************************************************/

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
/*- Includes ---------------------------------------------------------------*/
#include <errno.h>
#include <limits.h>
#include "config/default/definitions.h"
#ifdef CLI_ENABLED
#include "system/console/sys_console.h"
#endif
/*- Definitions ------------------------------------------------------------*/
#define APP_CMD_UART_BUFFER_SIZE    16U
#define APP_CMD_PENDING_TABLE_SIZE  5U
#define APP_CMD_INVALID_ADDR        0xffffU

/*- Types ------------------------------------------------------------------*/
typedef enum {
	APP_CMD_UART_STATE_IDLE,
	APP_CMD_UART_STATE_SYNC,
	APP_CMD_UART_STATE_DATA,
	APP_CMD_UART_STATE_MARK,
	APP_CMD_UART_STATE_CSUM,
} AppCmdUartState_t;


typedef struct {
	uint8_t commandId;
	uint64_t dstAddr;
} AppCmdUartHeader_t;

typedef struct {
	uint8_t commandId;
	uint64_t dstAddr;
	uint16_t duration;
	uint16_t period;
} AppCmdUartIdentify_t;

typedef struct {
	uint8_t id;
} AppCmdHeader_t;

typedef struct {
	uint8_t id;
	uint16_t duration;
	uint16_t period;
} AppCmdIdentify_t;

/*- Prototypes -------------------------------------------------------------*/
//Commands definitions
#ifdef CLI_ENABLED
static void miwi_getParentAddress(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void miwi_broadcast_data(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void miwi_enable_sleep(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibgetChannel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibsetChannel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibgetChannelPage(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibsetChannelPage(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibgetPanId(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibsetPanId(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibgetTxPwr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibsetTxPwr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibgetIeeeAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibsetIeeeAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibgetSrcAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void phy_pibsetSrcAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
#endif
static void appCmdUartProcess(uint8_t *data, uint8_t size);
static void appCmdBuffer(uint16_t addr, uint8_t *data, uint8_t size);
static void appCmdDataConf(uint8_t msgConfHandle, miwi_status_t status, uint8_t* msgPointer);
static bool appCmdHandle(uint8_t *data, uint8_t size);
static void appCmdIdentifyDurationTimerHandler(uintptr_t context);
static void appCmdIdentifyPeriodTimerHandler(uintptr_t context);

/*- Variables --------------------------------------------------------------*/
static AppCmdUartState_t appCmdUartState = APP_CMD_UART_STATE_IDLE;
static uint8_t appCmdUartPtr;
static uint8_t appCmdUartBuf[APP_CMD_UART_BUFFER_SIZE];
static uint8_t appCmdUartCsum;
static uint8_t wsnmsghandle;
SYS_Timer_t appCmdIdentifyDurationTimer;
SYS_Timer_t appCmdIdentifyPeriodTimer; 

SYS_TIME_HANDLE appCmdIdentifyDurationTimerHandle;
SYS_TIME_HANDLE appCmdIdentifyPeriodTimerHandle;

SYS_TIME_HANDLE joinTimerHandle;
SYS_TIME_HANDLE keepAliveTimerHandle;
SYS_Timer_t joinTimer;
SYS_Timer_t keepAliveTimer;
/*- Implementations --------------------------------------------------------*/
//CMD descriptor table definition
#ifdef CLI_ENABLED
static const SYS_CMD_DESCRIPTOR MiWiCmdsTbl[] =
{
    {"getParentAddress", miwi_getParentAddress, ":Get Parent device address if the device is ED/Coordinator: getParentAddr\r\n"},
    {"getChannel", phy_pibgetChannel, ":Get PHY PIB attribute: getChannel\r\n"},
    {"setChannel", phy_pibsetChannel, ":Set PHY PIB attribute: setChannel,<channel>\r\n"},
    {"getChannelPg", phy_pibgetChannelPage, ":Get PHY PIB attribute: getChannelPg\r\n"},
    {"setChannelPg", phy_pibsetChannelPage, ":Set PHY PIB attribute: setChannelPg,<channel page>\r\n"},
    {"getPanId", phy_pibgetPanId, ":Get PHY PIB attribute: getPanId\r\n"},
    {"setPanId", phy_pibsetPanId, ":Set PHY PIB attribute: setPanId,<PAN ID>\r\n"},
    {"getSrcAddr", phy_pibgetSrcAddr, ":Get PHY PIB attribute: getDestAddr\r\n"},
    {"setSrcAddr", phy_pibsetSrcAddr, ":Set PHY PIB attribute: setDestAddr,<src address>\r\n"},
    {"getTxPwr", phy_pibgetTxPwr, ":Get PHY PIB attribute: getTxPwr\r\n"},
    {"setTxPwr", phy_pibsetTxPwr, ":Set PHY PIB attribute: setTxPwr,<tx pwr>\r\n"},
    {"getIeeeAddr", phy_pibgetIeeeAddr, ":Get PHY PIB attribute: getIeeeAddr\r\n"},
    {"setIeeeAddr", phy_pibsetIeeeAddr, ":Set PHY PIB attribute: setIeeeAddr,<Ieee address 64 bit>\r\n"},
    {"broadcastData", miwi_broadcast_data, ":broadcast data\r\n"},
    {"enableDeepSleep", miwi_enable_sleep, ":Enable Deep Sleep\r\n"},
};
//PHY commands init - registering the command table to sys commands
void MiWiCmdTable_Init(void)
{
    if(SYS_CMD_ADDGRP(MiWiCmdsTbl, (int)(sizeof(MiWiCmdsTbl)/sizeof(*MiWiCmdsTbl)), "MiWiCmds", ": MiWi commands"))
    {
        SYS_CONSOLE_MESSAGE("\r\nSYS CMD TBL INITIALISED\r\n");
    }   
}
#endif
/*************************************************************************//**
*****************************************************************************/
void APP_CommandsInit(void)
{
//	appCmdIdentifyDurationTimer.mode = SYS_TIMER_INTERVAL_MODE;
//	appCmdIdentifyDurationTimer.handler = appCmdIdentifyDurationTimerHandler;
//
//	appCmdIdentifyPeriodTimer.mode = SYS_TIMER_PERIODIC_MODE;
//	appCmdIdentifyPeriodTimer.handler = appCmdIdentifyPeriodTimerHandler;
}

/*************************************************************************//**
*****************************************************************************/
void APP_CommandsByteReceived(uint8_t byte)
{
	switch (appCmdUartState) {
	case APP_CMD_UART_STATE_IDLE:
	{
		if (0x10U == byte) {
			appCmdUartPtr = 0U;
			appCmdUartCsum = byte;
			appCmdUartState = APP_CMD_UART_STATE_SYNC;
		}
	}
	break;

	case APP_CMD_UART_STATE_SYNC:
	{
		appCmdUartCsum += byte;

		if (0x02U == byte) {
			appCmdUartState = APP_CMD_UART_STATE_DATA;
		} else {
			appCmdUartState = APP_CMD_UART_STATE_IDLE;
		}
	}
	break;

	case APP_CMD_UART_STATE_DATA:
	{
		appCmdUartCsum += byte;

		if (0x10U == byte) {
			appCmdUartState = APP_CMD_UART_STATE_MARK;
		} else {
			appCmdUartBuf[appCmdUartPtr++] = byte;
		}

		if (appCmdUartPtr == APP_CMD_UART_BUFFER_SIZE) {
			appCmdUartState = APP_CMD_UART_STATE_IDLE;
		}
	}
	break;

	case APP_CMD_UART_STATE_MARK:
	{
		appCmdUartCsum += byte;

		if (0x10U == byte) {
			appCmdUartBuf[appCmdUartPtr++] = byte;

			if (appCmdUartPtr == APP_CMD_UART_BUFFER_SIZE) {
				appCmdUartState = APP_CMD_UART_STATE_IDLE;
			} else {
				appCmdUartState = APP_CMD_UART_STATE_DATA;
			}
		} else if (0x03U == byte) {
			appCmdUartState = APP_CMD_UART_STATE_CSUM;
		} else {
			appCmdUartState = APP_CMD_UART_STATE_IDLE;
		}
	}
	break;

	case APP_CMD_UART_STATE_CSUM:
	{
		if (byte == appCmdUartCsum) {
			appCmdUartProcess(appCmdUartBuf, appCmdUartPtr);
		}

		appCmdUartState = APP_CMD_UART_STATE_IDLE;
	}
	break;

	default:
        //Handle exceptions if any
		break;
	}
}

/*************************************************************************//**
*****************************************************************************/
static void appCmdUartProcess(uint8_t *data, uint8_t size)
{
	AppCmdUartHeader_t *header = (AppCmdUartHeader_t *)data;

	if (size < (uint8_t)sizeof(AppCmdUartHeader_t)) {
		return;
	}

	if ((uint8_t)APP_COMMAND_ID_IDENTIFY == header->commandId) {
		AppCmdUartIdentify_t *uartCmd = (AppCmdUartIdentify_t *)data;
		AppCmdIdentify_t cmd;

		cmd.id = (uint8_t)APP_COMMAND_ID_IDENTIFY;
		cmd.duration = uartCmd->duration;
		cmd.period = uartCmd->period;

		appCmdBuffer(header->dstAddr, (uint8_t *)&cmd,
				sizeof(AppCmdIdentify_t));
	}
}

/*************************************************************************//**
*****************************************************************************/
static void appCmdBuffer(uint16_t addr, uint8_t *data, uint8_t size)
{
	if (0U == addr)
	{
		appCmdHandle(data, size);
		appCmdDataRequest(0xFFFFU, size, data);
	}
	else
	{
		appCmdDataRequest(addr, size, data);
	}
}


/*************************************************************************//**
*****************************************************************************/
void appCmdDataRequest(uint16_t addr, uint8_t size, uint8_t* payload)
{
	MiApp_SendData(SHORT_ADDR_LEN, (uint8_t*)&addr, size, payload, wsnmsghandle, true, appCmdDataConf);
}

/*************************************************************************//**
*****************************************************************************/
static void appCmdDataConf(uint8_t msgConfHandle, miwi_status_t status, uint8_t* msgPointer)
{
    APP_Msg_T *p_appModes;
    APP_Msg_T appModes;
    p_appModes = &appModes;
    appStates = APP_STATE_SENDING_DONE;
    p_appModes->msgId = APP_STATE_SENDING_DONE;
    OSAL_QUEUE_Send(&appData.appQueue, p_appModes, 0);
}

/*************************************************************************//**
*****************************************************************************/
void appCmdDataInd(RECEIVED_MESH_MESSAGE *ind)
{
	//appCmdHandle(ind->payload, ind->payloadSize);
}

/*************************************************************************//**
*****************************************************************************/
static bool appCmdHandle(uint8_t *data, uint8_t size)
{
	AppCmdHeader_t *header = (AppCmdHeader_t *)data;

	if (size < sizeof(AppCmdHeader_t)) {
		return false;
	}

	if (APP_COMMAND_ID_IDENTIFY == header->id) {
		AppCmdIdentify_t *req = (AppCmdIdentify_t *)data;

		if (sizeof(AppCmdIdentify_t) != size) {
			return false;
		}

        SYS_TIME_TimerStop(appCmdIdentifyDurationTimerHandle);
        SYS_TIME_TimerStop(appCmdIdentifyDurationTimerHandle);
        
        
        uint8_t myData = 0U;
		appCmdIdentifyDurationTimer.interval = req->duration;
		appCmdIdentifyDurationTimer.mode = SYS_TIME_SINGLE;
		appCmdIdentifyDurationTimer.handler = appCmdIdentifyDurationTimerHandler;
        
        appCmdIdentifyDurationTimerHandle = SYS_TIME_CallbackRegisterMS(&appCmdIdentifyDurationTimerHandler, (uintptr_t)&myData, appCmdIdentifyDurationTimer.interval, SYS_TIME_SINGLE);
        if(appCmdIdentifyDurationTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
        	return false;
        }
		appCmdIdentifyPeriodTimer.interval = req->period;
		appCmdIdentifyPeriodTimer.mode = SYS_TIME_PERIODIC;
		appCmdIdentifyPeriodTimer.handler = appCmdIdentifyPeriodTimerHandler;
        appCmdIdentifyPeriodTimerHandle = SYS_TIME_CallbackRegisterMS(&appCmdIdentifyPeriodTimerHandler, (uintptr_t)&myData, appCmdIdentifyPeriodTimer.interval, SYS_TIME_PERIODIC);
        if(appCmdIdentifyPeriodTimerHandle == SYS_TIME_HANDLE_INVALID)
        {
           return false;
        }
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_On();
#else
	LED_On(1,LED_IDENTIFY);
#endif
#endif
#endif        
		return true;
	}
	return false;
}

/*************************************************************************//**
*****************************************************************************/
//static void appCmdIdentifyDurationTimerHandler(SYS_Timer_t *timer) //michp
static void appCmdIdentifyDurationTimerHandler(uintptr_t context)
{
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Off();
#else
	LED_Off(1,LED_IDENTIFY);
#endif
#endif
#endif  
    uint32_t isTimePending = 0U;
    SYS_TIME_TimerCounterGet(appCmdIdentifyPeriodTimerHandle,&isTimePending);
    if(isTimePending > 0U)
    {
        SYS_TIME_TimerStop(appCmdIdentifyPeriodTimerHandle);
    }

	(void)context;
}

/*************************************************************************//**
*****************************************************************************/
static void appCmdIdentifyPeriodTimerHandler(uintptr_t context)
{
#if defined(LED_ENABLED)
#if (LED_COUNT > 0U)
#if defined(CHIMERA_SOC)
    RGB_LED_GREEN_Toggle();
#else
	LED_Toggle(1,LED_IDENTIFY);
#endif
#endif
#endif  
	(void)context;
}
#ifdef CLI_ENABLED
static void phy_pibgetChannel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv){
    
    PHY_Retval_t status;
    PibValue_t pibValue;
    status = PHY_PibGet(phyCurrentChannel,(uint8_t *)&pibValue);
    appPhyCmdProcessor_PhyStatusPrint(status);
    if(status == PHY_SUCCESS)
    {
      SYS_CONSOLE_PRINT("\r\n Channel  - 0x%x\n ",pibValue.pib_value_8bit); 
    }
    else
    {
        appPhyCmdProcessor_PhyStatusPrint(status);
    }
   
}

static void phy_pibsetChannel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{    
    PHY_Retval_t status;
    uint8_t retcode, attribute_val = 0U;
    PibValue_t pibValue;
    if(argc<2)
    {
        retcode = ERR_MISSING_PARAM;
        appPhyCmdProcessor_PrintReturnCode(retcode);
        return;
    }
    if(appPhyCmdProcessor_StrToUint8(argv[1], &attribute_val))
    {
        if((attribute_val < 11U) || (attribute_val > 26U))
        {
            retcode = ERR_PARAM_OUT_OF_RANGE;
            appPhyCmdProcessor_PrintReturnCode(retcode);
            return;
        }
    }
    pibValue.pib_value_8bit = attribute_val;
    status = PHY_PibSet(phyCurrentChannel, &pibValue);
    appPhyCmdProcessor_PhyStatusPrint(status);
    if(status == PHY_SUCCESS)
    {
      if(PHY_PibGet(phyCurrentChannel,&attribute_val) == PHY_SUCCESS)
      {
          SYS_CONSOLE_PRINT("\r\n Channel  - 0x%x\n ",attribute_val); 
      }
      MiApp_Set(CHANNEL, &attribute_val);
    }
   
}

static void phy_pibgetChannelPage(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{    
    PHY_Retval_t status;
    uint8_t channelPage;
    status = PHY_PibGet(phyCurrentPage,&channelPage);
    appPhyCmdProcessor_PhyStatusPrint(status);
    if(status == PHY_SUCCESS)
    {
      SYS_CONSOLE_PRINT("\r\n Channel Page - %d\n ",channelPage); 
    }
    else
    {
        appPhyCmdProcessor_PhyStatusPrint(status);
    }
}

static void phy_pibsetChannelPage(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{    
    PHY_Retval_t status;
    uint8_t retcode, attribute_val = 0U;
    PibValue_t pibValue;
    if(argc<2)
    {
        retcode = ERR_MISSING_PARAM;
        appPhyCmdProcessor_PrintReturnCode(retcode);
        return;
    }
    if(appPhyCmdProcessor_StrToUint8(argv[1], &attribute_val))
    {
        if((attribute_val != 0x0U) && (attribute_val != 0x2U) && (attribute_val != 0x10U) && (attribute_val != 0x11U))
        {
            retcode = ERR_PARAM_OUT_OF_RANGE;
            appPhyCmdProcessor_PrintReturnCode(retcode);
            return;
        }
    }
    pibValue.pib_value_8bit = attribute_val;
    status = PHY_PibSet(phyCurrentPage, &pibValue);
    appPhyCmdProcessor_PhyStatusPrint(status);
    if(status == PHY_SUCCESS)
    {
        if(PHY_PibGet(phyCurrentPage,&attribute_val) == PHY_SUCCESS)
        {
            SYS_CONSOLE_PRINT("\r\n Channel Page - %d\n ",attribute_val); 
        }
    }
   
}

static void phy_pibgetPanId(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{  
    PHY_Retval_t status;
    PibValue_t pibValue;
    status = PHY_PibGet(macPANId,(uint8_t *)&pibValue);
    appPhyCmdProcessor_PhyStatusPrint(status);
    if(status == PHY_SUCCESS)
    {
      SYS_CONSOLE_PRINT("\r\n PAN ID - 0x%x\n ",pibValue.pib_value_16bit); 
    }
    else
    {
        appPhyCmdProcessor_PhyStatusPrint(status);
    }
}

static void phy_pibsetPanId(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    PHY_Retval_t status;
    uint8_t retcode;
    uint16_t attr_val_16 = 0U;
    PibValue_t pibValue;
    if(argc<2)
    {
        retcode = ERR_MISSING_PARAM;
        appPhyCmdProcessor_PrintReturnCode(retcode);
        return;
    }
    if(appPhyCmdProcessor_StrToUint16HexIp(argv[1],&attr_val_16))
    {
        pibValue.pib_value_16bit = attr_val_16;
        status = PHY_PibSet(macPANId , &pibValue);
        appPhyCmdProcessor_PhyStatusPrint(status);
        if(status == PHY_SUCCESS)
        {
            if(PHY_PibGet(macPANId, (uint8_t*)&attr_val_16) == PHY_SUCCESS)
            {
                SYS_CONSOLE_PRINT("\r\n PAN ID : 0x%x\n ",attr_val_16); 
            }
            MiApp_Set(PANID, (uint8_t*)&attr_val_16);
        }
    }
 

}

static void phy_pibgetTxPwr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    
    PHY_Retval_t status;
    PibValue_t pibValue;
    status = PHY_PibGet(phyTransmitPower,(uint8_t *)&pibValue);	
    if(status == PHY_SUCCESS)
    {
        SYS_CONSOLE_PRINT("\r\n Tx Power - %d\r\n ",pibValue.pib_value_8bit); 
    }
    else
    {
        appPhyCmdProcessor_PhyStatusPrint(status);
    }
}
static void phy_pibsetTxPwr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    PHY_Retval_t status;
    uint8_t retcode = 0U;
    int8_t attribute_val = 0;
    PibValue_t pibValue;
    if(argc<2){
        retcode = ERR_MISSING_PARAM;
       appPhyCmdProcessor_PrintReturnCode(retcode);
        return;
    }
    if(appPhyCmdProcessor_StrToInt8(argv[1], &attribute_val))
    {
        if((attribute_val < -12) || (attribute_val > 14))
        {
            retcode = ERR_PARAM_OUT_OF_RANGE;
            appPhyCmdProcessor_PrintReturnCode(retcode);
            return;
        }
            SYS_CONSOLE_PRINT("\r\n Tx Power - %d DBm\r\n ",attribute_val); 
            pibValue.pib_value_8bit = (uint8_t)CONV_DBM_TO_phyTransmitPower(attribute_val);
            status = PHY_PibSet(phyTransmitPower, &pibValue);	
            if(status == PHY_SUCCESS)
            {
                if(PHY_PibGet(phyTransmitPower,&pibValue.pib_value_8bit) == PHY_SUCCESS)
                {
                    SYS_CONSOLE_PRINT("\r\nSet Tx Power - %d DBm\r\n ",pibValue.pib_value_8bit); 
                }
            }
            else
            {
               appPhyCmdProcessor_PhyStatusPrint(status);
            }
    }

}

static void phy_pibgetSrcAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    PibValue_t pibValue;
    PHY_Retval_t attr_stat;
    attr_stat = PHY_PibGet(macShortAddress,(uint8_t *)&pibValue);
    if(attr_stat == PHY_SUCCESS)
    {
        SYS_CONSOLE_PRINT("\r\n Source Address : 0x%x\r\n",pibValue.pib_value_16bit);
    }
    else
    {
        appPhyCmdProcessor_PhyStatusPrint(attr_stat);
    }
}
static void phy_pibsetSrcAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    
    PibValue_t pibValue;
    uint8_t retcode;
    PHY_Retval_t attr_stat;
    uint16_t attr_val_16 = 0;
    if(argc<2)
    {
        retcode = ERR_MISSING_PARAM;
        appPhyCmdProcessor_PrintReturnCode(retcode);
        return;
    }
    if(appPhyCmdProcessor_StrToUint16HexIp(argv[1],&attr_val_16))
    {
        SYS_CONSOLE_PRINT("\r\n Existing Source Address - 0x%x\n ",attr_val_16); 
        attr_stat = PHY_PibSet(macShortAddress, &pibValue);
        if(attr_stat != PHY_SUCCESS)
        {
            appPhyCmdProcessor_PhyStatusPrint(attr_stat);
            return;
        }
        attr_stat = PHY_PibGet(macShortAddress,(uint8_t *)&attr_val_16);
        if(attr_stat == PHY_SUCCESS)
        {
            SYS_CONSOLE_PRINT("\r\n New Source Address : %d \r\n",attr_val_16);
            MiApp_Set(SHORT_ADDRESS, (uint8_t*)&attr_val_16);
        }
    }
}

static void phy_pibgetIeeeAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    PHY_Retval_t attributeStatus;
    PibValue_t pibValue;
    attributeStatus = PHY_PibGet(macIeeeAddress,(uint8_t *)&pibValue);
    if(attributeStatus == PHY_SUCCESS)
    {
        SYS_CONSOLE_PRINT("\r\n IEEE Address : 0x%x\r\n",pibValue.pib_value_64bit);
    }
}

static void phy_pibsetIeeeAddr(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    PHY_Retval_t attributeStatus;
    PibValue_t pibValue;
    uint64_t ieeeaddr = 0U;
    if(appPhyCmdProcessor_StrToUint64(argv[1], &ieeeaddr))
    {
        pibValue.pib_value_64bit = ieeeaddr;
        attributeStatus = PHY_PibSet(macIeeeAddress,&pibValue);
        if(attributeStatus == PHY_SUCCESS)
        {
            attributeStatus = PHY_PibGet(macIeeeAddress,(uint8_t *)&ieeeaddr);
            SYS_CONSOLE_PRINT("\r\n IEEE Address : 0x%x\r\n",ieeeaddr);
        }
    }
}

static void miwi_getParentAddress(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
#ifndef PAN_COORDINATOR
    SYS_CONSOLE_PRINT("\r\n My Parent Address : 0x%x\r\n",myParentShortAddress);
#else
    SYS_CONSOLE_PRINT("\r\n I'm parent PAN coordinator\r\n");
#endif
}

static void miwi_broadcast_data(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    uint8_t pload = 0xac;
    uint16_t address = 0xFFFF;
    SYS_CONSOLE_PRINT("\r\n broadcast data\r\n");
//    appStates = APP_STATE_SEND;
#ifdef PAN_COORDINATOR
//    address = getNextHopAddr(myShortAddress);
//    address = 0x0001;
    appCmdDataRequest(address, sizeof(pload), &pload);
#else
    if(myParentShortAddress != 0xFFFF)
    {
      appCmdDataRequest(myParentShortAddress, sizeof(pload), &pload);   
    }
    else
    {
        appCmdDataRequest(address, sizeof(pload), &pload);
    }
#endif
}

static void miwi_enable_sleep(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
#if defined(PROTOCOL_MESH)
#if defined(ENABLE_SLEEP_FEATURE) && defined(ENDDEVICE)
    if(CAPABILITY_INFO == CAPABILITY_INFO_ED)
    {
    deviceCanSleep = true;
    APP_Msg_T sleepReq;
    sleepReq.msgId = APP_STATE_DATA_RECEIVE_IND;   
    appStates = APP_STATE_DATA_RECEIVE_IND;
    OSAL_QUEUE_Send(&appData.appQueue, &sleepReq, 0);
    }
#endif
#elif defined(PROTOCOL_STAR)
    #if defined(ENABLE_SLEEP_FEATURE)
    if(role == END_DEVICE)
    {
        deviceCanSleep = true;
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg; 
    appState->msgId = APP_STATE_PREPARE_TO_SLEEP;
    OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
    }
    #endif
#else
    #if defined(ENABLE_SLEEP_FEATURE)
    deviceCanSleep = true;
    APP_Msg_T    appMsg;
    APP_Msg_T *appState;
    appState = &appMsg; 
    appState->msgId = APP_STATE_PREPARE_TO_SLEEP;
    OSAL_QUEUE_Send(&appData.appQueue, appState, 0U);
    #endif
#endif
}
void appPhyCmdProcessor_PrintReturnCode(uint8_t status)
{
  switch(status)
  {
    case AOK:
      appPhyCmdProcessor_CmdDoneOk();
      break;
      
    case NO_PROMPT:
      break;
        
    case PENDING:
      break;

    default:
      appPhyCmdProcessor_CmdDoneFail(status);
      break;
  }
  //time_to_process_cmd = 0;
}

/**************************************************************************************************/

bool appPhyCmdProcessor_StrToBool(const char *str, bool *res)
{
    if (strcmp(str, "true") == 0 || strcmp(str, "True") == 0)
    {
        *res = true;
        return true;
    }
    else if (strcmp(str, "false") == 0 || strcmp(str, "False") == 0)
    {
        *res = false;
        return true;
    }
    else
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input. Please enter either 'true' or 'false'\r\n");
        return false;
    }
}

bool appPhyCmdProcessor_StrToUint8HexIp(const char *str, uint8_t *res)
{
    char *end;
    errno = 0;
    uint16_t val1 = (uint16_t)strtol(str, &end, 16);
    if (errno != 0 || (*end != '\0') || (val1 > 0xffU)) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    errno = 0;
    uint8_t val = (uint8_t)strtol(str, &end, 16);
    if(errno != 0)
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = val;
    return true;
}

bool appPhyCmdProcessor_StrToUint8(const char *str, uint8_t *res)
{
    char *end;
    errno = 0;  
    uint16_t val1 = (uint16_t)strtol(str, &end, 10);
    if (errno != 0 || (*end != '\0') || (val1 > 255U)) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    errno = 0;
    uint8_t val = (uint8_t)strtol(str, &end, 10);
    if(errno != 0)
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = val;
    return true;
}

bool appPhyCmdProcessor_StrToInt8(const char *str, int8_t *res)
{
    char *end;
    errno = 0;  
    int8_t val = (int8_t)strtol(str, &end, 10);
    if (errno != 0 || (*end != '\0')) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = val;
    return true;
}

bool appPhyCmdProcessor_StrToUint16(const char *str, uint16_t *res)
{
    char *end;
    errno = 0;
    uint32_t val1 = (uint32_t)strtoul(str, &end, 10);
    if (errno != 0 || (*end != '\0') || (val1 > 65535U)) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    errno = 0;
    uint16_t val = (uint16_t)strtoul(str, &end, 10);
    if(errno != 0)
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = val;
    return true;
}

bool appPhyCmdProcessor_StrToUint16HexIp(const char *str, uint16_t *res)
{
    char *end;
    errno = 0;
    uint32_t val1 = (uint32_t)strtoul(str, &end, 16);
    if (errno != 0 || (*end != '\0') || (val1 > 0xffffU)) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    errno = 0;
    uint16_t val = (uint16_t)strtoul(str, &end, 16);
    if(errno != 0)
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = val;
    return true;
}

bool appPhyCmdProcessor_StrToUint32(const char *str, uint32_t *res) 
{    
    char *end;
    errno = 0;
    uint64_t val1 = (uint64_t)strtoul(str, &end, 10);
    if (errno != 0 || (*end != '\0') || (val1 > 4294967295U))
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    errno = 0;
    uint32_t val = (uint32_t)strtoul(str, &end, 10);
    if(errno != 0)
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = val;
    return true;
}

bool appPhyCmdProcessor_StrToUint64(const char *str, uint64_t *res) 
{
    char *endptr;
    errno = 0;
    uint64_t result = (uint64_t)strtoull(str, &endptr, 16);
    if (errno != 0 || (*endptr != '\0')) //|| (result > 18446744073709551615U) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = result;
    return true;
}

bool appPhyCmdProcessor_StrToUint64DecIp(const char *str, uint64_t *res) 
{
    char *endptr;
    errno = 0;
    uint64_t result = (uint64_t)strtoull(str, &endptr, 10);
    if (errno != 0 || (*endptr != '\0')) //|| (result > 18446744073709551615U) 
    {
        SYS_CONSOLE_MESSAGE("\r\nInvalid input\r\n");
        return false;
    }
    *res = result;
    return true;
}

void appPhyCmdProcessor_PhyStatusPrint(PHY_Retval_t status){

    switch((uint8_t)status)
    {
        case 0x00:
            SYS_CONSOLE_PRINT("\r\nPHY_SUCCESS\r\n");
            break;
        case 0x81:
            SYS_CONSOLE_PRINT("\r\nPHY_TRX_ASLEEP\r\n");
            break;
        case 0x82:
            SYS_CONSOLE_PRINT("\r\nPHY_TRX_AWAKE\r\n");
            break;
#ifdef CHIMERA_SOC
        case 0x83:
            SYS_CONSOLE_PRINT("\r\nPHY_RF_REQ_ABORTED\r\n");
            break;
        case 0x84:
            SYS_CONSOLE_PRINT("\r\nPHY_RF_UNAVAILABLE\r\n");
            break;
#endif
        case 0x85:
            SYS_CONSOLE_PRINT("\r\nPHY_FAILURE\r\n");
            break;
        case 0x86:
            SYS_CONSOLE_PRINT("PHY_BUSY\r\n");
            break;
        case 0x87:
            SYS_CONSOLE_PRINT("\r\nPHY_FRAME_PENDING\r\n");
            break;
        case 0x88:
            SYS_CONSOLE_PRINT("\r\nPHY_INVALID_PARAMETER\r\n");
            break;
        case 0x89:
            SYS_CONSOLE_PRINT("\r\nPHY_UNSUPPORTED_ATTRIBUTE\r\n");
            break;
        case 0x8A:
            SYS_CONSOLE_PRINT("\r\nPHY_CHANNEL_BUSY\r\n");
            break;
        case 0x8B:
            SYS_CONSOLE_PRINT("\r\nPHY_CHANNEL_IDLE\r\n");
            break;
        case 0x8C:
            SYS_CONSOLE_PRINT("\r\nPHY_NO_ACK\r\n");
            break;
        case 0x8D:
            SYS_CONSOLE_PRINT("\r\nPHY_CHANNEL_ACCESS_FAILURE\r\n");
            break;
        default:
            SYS_CONSOLE_PRINT("\r\nPHY_UNKNOWN_STATE\r\n");
            break;
            
    }
} 

void appPhyCmdProcessor_PhyTrxStatusPrint(PHY_TrxStatus_t status){
    
    switch((uint8_t)status){
        case 0x08:
            SYS_CONSOLE_PRINT("PHY_TRX_OFF\r\n");
            break;
        case 0x16:
            SYS_CONSOLE_PRINT("PHY_RX_ON\r\n");
            break;
        case 0x19:
            SYS_CONSOLE_PRINT("PHY_TX_ON\r\n");
            break;
        case 0x11:
            SYS_CONSOLE_PRINT("PHY_BUSY_RX\r\n");
            break;
        case 0x12:
            SYS_CONSOLE_PRINT("PHY_BUSY_TX\r\n");
            break;
        case 0x0F:
            SYS_CONSOLE_PRINT("PHY_TRX_SLEEP\r\n");
            break;
        case 0x20:
            SYS_CONSOLE_PRINT("PHY_TRX_DEEP_SLEEP\r\n");
            break;
        default:
            SYS_CONSOLE_PRINT("PHY_UNKNOWN_STATE\r\n");
            break;
            
    }
} 
void appPhyCmdProcessor_CmdDoneOk(void)
{
  SYS_CONSOLE_PRINT("\r\nSuccess\r\n");
}
void appPhyCmdProcessor_CmdDoneFail(uint8_t err_code)
{
  SYS_CONSOLE_PRINT("Err%d: ",err_code);
  switch(err_code)
  {
    case ERR_UNKNOWN_CMD:
      SYS_CONSOLE_PRINT("Unknown Command\r\n");
      break;
    case ERR_INVALID_PARAM:
      SYS_CONSOLE_PRINT("Invalid Parameter\r\n");
      break;
    case ERR_MISSING_PARAM:
      SYS_CONSOLE_PRINT("Missing Parameter\r\n");
      break;
    case ERR_INVALID_PARAM_TYPE:
      SYS_CONSOLE_PRINT("Invalid Parameter Type\r\n");
      break;
    case ERR_PARAM_OUT_OF_RANGE:
      SYS_CONSOLE_PRINT("Parameter Out of Range\r\n");
      break;
    case ERR_INVALID_FORMAT:
      SYS_CONSOLE_PRINT("Invalid Format\r\n");
      break;
    case ERR_SCRIPT_PARSING:
      SYS_CONSOLE_PRINT("Script Parsing Error\r\n");
      break;
    case ERR_HEAP_FULL:
      SYS_CONSOLE_PRINT("Heap Full\r\n");
      break;
    case ERR_INVALID_STATE:
      SYS_CONSOLE_PRINT("Invalid State\r\n");
      break;
    case ERR_NO_RESOURCES:
      SYS_CONSOLE_PRINT("No Resources\r\n");
      break;
    case ERR_BLE_ERR:
      SYS_CONSOLE_PRINT("BLE Err\r\n");
      break;
    case ERR_HAL_ERR:
      SYS_CONSOLE_PRINT("HAL Err\r\n");
      break;  
    case ERR_UNKNOWN_ERR:
      SYS_CONSOLE_PRINT("Unknown Err\r\n");
        break;
    default:
      SYS_CONSOLE_PRINT("Unknown Err\r\n");
      break;
  }
}
#endif
