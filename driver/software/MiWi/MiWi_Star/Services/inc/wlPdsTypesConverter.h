/**************************************************************************
  \file wlPdsTypesConverter.h

  \brief
    PDS types convertions definition

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2013, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    21.05.13 N. Fomin - Created.
******************************************************************************/

#ifndef WLPDS_TYPESCONVERTER
#define WLPDS_TYPESCONVERTER

/******************************************************************************
                   Includes section
******************************************************************************/
#include "MiWi/MiWi_Star/Services/inc/S_Nv.h"
#include "MiWi/MiWi_Star/Services/inc/wlPdsMemIds.h"

/******************************************************************************
                   Types section
******************************************************************************/
      
typedef struct
{
  uint64_t         extendedAddress;
  uint32_t         channelMap;
  uint16_t         panId;
  uint16_t         networkAddress;
  uint8_t          logicalChannel;
  uint8_t          capabilityInfo;
  uint8_t          nwkSecurityLevel;
} MeshGeneralInfotMem_t;

/******************************************************************************
                   Definitions section
******************************************************************************/
#ifndef FILE_TO_ITEM_MAPPING
  #define FILE_TO_ITEM_MAPPING
#endif

#define NO_ITEM_FLAGS               0x00U
#define SIZE_MODIFICATION_ALLOWED   0x01U
#define ITEM_UNDER_SECURITY_CONTROL 0x02U

//#define USE_CUSTOM_PDS_APIS

#if defined(PROTOCOL_MESH)
#define GENERAL_INFO_ITEM_SIZE                   (sizeof(MeshGeneralInfotMem_t))
#define PDS_SECURITY_KEY_ITEM_SIZE               (SECURITY_KEY_SIZE)
#define PDS_SECURITY_COUNTER_ITEM_SIZE           (sizeof(API_UINT32_UNION))
#define PDS_BLOOM_VALUE_ITEM_SIZE                (BLOOM_FILTER_SIZE)
#define PDS_COMM_DEVICE_TABLE_ITEM_SIZE          (sizeof(CommDeviceTable_t))
#define PDS_COORDINATOR_TABLE_ITEM_SIZE          (sizeof(CoordinatorTable_t))
#define PDS_DEVICE_TABLE_NONSLEEP_ITEM_SIZE      (sizeof(DeviceTable_t))
#define PDS_DEVICE_TABLE_SLEEP_ITEM_SIZE         (sizeof(SleepDeviceTable_t))
#define PDS_ROUTE_TABLE_COORD_ITEM_SIZE          (sizeof(CoordRouteTable_t))
#define PDS_ROUTE_TABLE_HOP_ITEM_SIZE            (HOP_TABLE_COUNT)
#else
#define PDS_OUTGOING_FRAME_COUNTER_ITEM_SIZE     (sizeof(API_UINT32_UNION))
#define PDS_PANID_ITEM_SIZE                      (sizeof(API_UINT16_UNION))
#define PDS_LONGADDR_ITEM_SIZE                   (LONG_ADDR_LEN)
#define PDS_CURRENT_CHANNEL_ITEM_SIZE            (sizeof(uint8_t))
#define PDS_CONNECTION_MODE_ITEM_SIZE            (sizeof(uint8_t))
#define PDS_CONNECTION_TABLE_ITEM_SIZE           (sizeof(CONNECTION_ENTRY))
#define PDS_EDC_ITEM_SIZE                        (sizeof(uint8_t))
#if defined (PROTOCOL_STAR)
#define PDS_ROLE_ITEM_SIZE                       (sizeof(uint8_t))
#define PDS_MYINDEX_ITEM_SIZE                    (sizeof(uint8_t))
#if defined(ENABLE_LINK_STATUS)
#define PDS_LINK_STATUS_ITEM_SIZE                (sizeof(uint16_t))
#endif
#endif
#endif
#if defined(ENABLE_FREQUENCY_AGILITY)
#define PDS_CHANNEL_UPDATE_ITEM_SIZE             (sizeof(uint8_t))
#endif
/******************************************************************************
                   Extern section
******************************************************************************/
extern MeshGeneralInfotMem_t genInfoMem;

#ifdef __GNUC__
extern uint32_t __pds_ff_start;
extern uint32_t __pds_ff_end;
extern uint32_t __pds_fd_start;
extern uint32_t __pds_fd_end;
#endif /* __GNUC__ */

/******************************************************************************
                    Prototypes section
******************************************************************************/
/******************************************************************************
\brief Fills General Information
******************************************************************************/
void fillGeneralInformation(void);

bool pdsUpdateMemoryCallback(PDS_UpdateMemory_t *item);

#endif // _WLPDS_TYPESCONVERTER
/* eof wlPdsTypesConverter.h */
