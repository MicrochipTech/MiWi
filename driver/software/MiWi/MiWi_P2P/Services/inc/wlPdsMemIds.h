/**************************************************************************
  \file wlPdsMemIds.h

  \brief
    PDS file and directory memory identificators definitions

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2013, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    21.05.13 N. Fomin - Created.
******************************************************************************/

#ifndef WLPDSMEMIDS_H
#define WLPDSMEMIDS_H

/******************************************************************************
                               Includes section
******************************************************************************/
#include "config/default/definitions.h"


/* Compatibility with std PDS mem IDs  */
//#include "peripheral/pds/pds.h"

#define PDS_MIWI_STACK_OFFSET_ID    PDS_MODULE_APP_OFFSET + 0x0


#define PDS_MIWI_DIR_OFFSET        (PDS_MODULE_APP_OFFSET + PDS_DIRECTORY_ID_MASK)

#if defined(PROTOCOL_MESH)
// Required to be consecutive starting from 0
typedef enum {
//	PDS_NULL_ID = PDS_MIWI_STACK_OFFSET_ID,    // To be maintained as the first entry
	PDS_GENERAL_INFO_ID = PDS_MIWI_STACK_OFFSET_ID,
#ifdef MESH_SECURITY
	PDS_SECURITY_KEY_ID,
	PDS_SECURITY_COUNTER_ID,
#endif
	PDS_BLOOM_VALUE_ID,
#if defined(ENABLE_FREQUENCY_AGILITY)
	PDS_CHANNEL_UPDATE_ID,
#endif
#if defined(PAN_COORDINATOR)
	PDS_COMM_DEVICE_TABLE_ID,
	PDS_COORDINATOR_TABLE_ID,
#endif
#if defined(PAN_COORDINATOR) || defined (COORDINATOR)
	PDS_DEVICE_TABLE_NONSLEEP_ID,
	PDS_DEVICE_TABLE_SLEEP_ID,
	PDS_ROUTE_TABLE_COORD_ID,
	PDS_ROUTE_TABLE_HOP_ID,
#endif
	PDS_MAX_ID          // To be maintained as the last entry
} miwi_mesh_pds_id_t;

#else
// Required to be consecutive starting from 0
typedef enum {
    PDS_GENERAL_INFO_ID = PDS_MIWI_STACK_OFFSET_ID,    // To be maintained as the first entry
    PDS_OUTGOING_FRAME_COUNTER_ID,
    PDS_PANID_ID,
    PDS_LONGADDR_ID,
    PDS_CURRENT_CHANNEL_ID,
    PDS_CONNECTION_MODE_ID,
    PDS_CONNECTION_TABLE_ID,
    PDS_EDC_ID,
#if defined (PROTOCOL_STAR)
    PDS_ROLE_ID,
    PDS_MYINDEX_ID,
#endif
#if defined(ENABLE_FREQUENCY_AGILITY)
	  PDS_CHANNEL_UPDATE_ID,
#endif
    PDS_MAX_ID          // To be maintained as the last entry
} miwi_p2pstar_pds_id_t;

#endif
/* Total files and directories amount supported by PDS */
#define PDS_ITEM_AMOUNT                      (PDS_MAX_ID - PDS_MIWI_STACK_OFFSET_ID)
#define PDS_DIRECTORIES_AMOUNT               1
#define PDS_ITEM_IDS_AMOUNT                 (PDS_ITEM_AMOUNT +  PDS_DIRECTORIES_AMOUNT)

#define PDS_ITEM_MASK_SIZE                  (PDS_ITEM_AMOUNT / 8U + (PDS_ITEM_AMOUNT % 8U ? 1U : 0U))

/* Directory mask */
#define  PDS_DIRECTORY_MASK                 PDS_DIRECTORY_ID_MASK
#define  MIWI_ALL_MEMORY_MEM_ID             PDS_DIRECTORY_ID_MASK + 1

#define PDS_ALL_EXISTENT_MEMORY             PDS_DIRECTORY_ID_MASK + 2



/******************************************************************************
                               Types section
******************************************************************************/

#endif // _WLPDSMEMIDS_H_
/* eof wlPdsMemIds.h */
