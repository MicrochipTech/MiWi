
#include "config/default/definitions.h"
#include <queue.h>
//#include "config/default/MiWi/MiWi_Mesh/inc/miwi_init.h"

/* Counting semaphore length to handle internal task processing */
#define MIWI_STACK_INTERNAL_SEM_LENGTH	20

static STACK_API_Request *pApiReqQueueData, apiReqQueueData;
static QueueSetHandle_t xQueueSet;
static QueueSetMemberHandle_t xActivatedMember;

OSAL_QUEUE_HANDLE_TYPE apiRequestQueueHandle; // Queue Pointer received from Aplication
static OSAL_SEM_HANDLE_TYPE semStackInternalHandler;
OSAL_API_LIST_TYPE *mimac;
//extern OSAL_QUEUE_HANDLE_TYPE miwiRequestQueueHandle;
/**************************************************************************
\brief MiWi API CALL
***************************************************************************/
void MIWI_API_CALL(STACK_API_Request *request)
{
    if(request != NULL)
    {
  if ( OSAL_RESULT_TRUE != mimac->OSAL_QUEUE_Send(&miwiRequestQueueHandle, &request, 0UL))
  {
    while(true);   // ERROR , Should not hang here, handle with assert
  }
    }
}

/*******************************************************************************
   OS Task Handler for MiWi Stack 
   Handles both API request from Applicaion layer and also internal scheduling

*/
void MiMAC_Tasks(void)
{
    
  mimac->OSAL_SEM_Create(&semStackInternalHandler, OSAL_SEM_TYPE_COUNTING,MIWI_STACK_INTERNAL_SEM_LENGTH, 0);
  
  /* Create the queue set large enough to hold an event for every space in
    every queue and semaphore that is to be added to the set. */
  mimac->OSAL_QUEUE_CreateSet(&xQueueSet, QUEUE_LENGTH + MIWI_STACK_INTERNAL_SEM_LENGTH);

  /* Add the queues and semaphores to the set.  Reading from these queues and
  semaphore can only be performed after a call to xQueueSelectFromSet() has
  returned the queue or semaphore handle from this point on. */
  mimac->OSAL_QUEUE_AddToSet( &apiRequestQueueHandle, &xQueueSet );
  mimac->OSAL_QUEUE_AddToSet( &semStackInternalHandler, &xQueueSet );
  
   pApiReqQueueData = &apiReqQueueData;

  while (true) 
  {
    /* Block to wait for something to be available from the queues or
     semaphore that have been added to the set.*/
     mimac->OSAL_QUEUE_SelectFromSet(&xActivatedMember, &xQueueSet, OSAL_WAIT_FOREVER );

    /* Which set member was selected?  Receives/takes can use a block time
    of zero as they are guaranteed to pass because xQueueSelectFromSet()
    would not have returned the handle unless something was available. */
    
    if( xActivatedMember == semStackInternalHandler )
    {
      /*Proses Internal Stack Events*/
      mimac->OSAL_SEM_Pend(&semStackInternalHandler, 0);
      MiMac_TaskHandler();
    }
    else if( xActivatedMember == apiRequestQueueHandle )
    {
       mimac->OSAL_QUEUE_Receive(&apiRequestQueueHandle, &pApiReqQueueData, 0);
//       switch (pApiReqQueueData->uApiID)
//       {
//        case (uint8_t)MIWI_START_NW:
//        {
//#ifdef PAN_COORDINATOR
//           startNetworkReq_t *pStartNwReq = NULL;
//           pStartNwReq = (startNetworkReq_t*)(pApiReqQueueData->parameters);
//           MiApp_StartConnection(pStartNwReq->Mode, pStartNwReq->ScanDuration,pStartNwReq->ChannelMap,pStartNwReq->ConfCallback);
//           pStartNwReq = NULL;
//#endif 
//           break; 
//        }
//        case (uint8_t)MIWI_CONNECT_NW:
//        {
//#ifndef PAN_COORDINATOR
//            searchNetworkReq_t *pSearchNwReq = NULL;
//           pSearchNwReq = (searchNetworkReq_t *)(pApiReqQueueData->parameters);
//           MiApp_SearchConnection(pSearchNwReq->ScanDuration,pSearchNwReq->ChannelMap,pSearchNwReq->ConfCallback);
//           pSearchNwReq = NULL;
//#endif 
//           break;
//        }
//        case (uint8_t)MIWI_SEND_DATA:
//        {
//#ifndef PAN_COORDINATOR
//            AppMessage_t *pAppMsg = NULL;
//            pAppMsg = (AppMessage_t*)(pApiReqQueueData->parameters);
//            appSendData(pAppMsg); 
//            pAppMsg = NULL;
//#endif
//            break;
//        }
//        
////        case MIWI_TXDONE_CB:
////        {
////            TxDoneCallbackReq_t *pTxDoneCbReq = NULL;
////            pTxDoneCbReq = (TxDoneCallbackReq_t *)(pApiReqQueueData->parameters);
////            if(pTxDoneCbReq->dataConfCb != NULL)
////            {
////                pTxDoneCbReq->dataConfCb(pTxDoneCbReq->MsgHandle, pTxDoneCbReq->MiwiTxDoneStatus, pTxDoneCbReq->pMsg);
////            }            
////            pTxDoneCbReq = NULL;  
////            break;
////        }
//         default:
//         {
//           // ASSERT
//           break;
//         }
//       }
    }
    else
    {
      // ASSERT
    }

  }
}
void MiMac_PostTask(bool isISRContext)
{
    OSAL_RESULT result = (OSAL_RESULT)OSAL_RESULT_TRUE;
    
    if(semStackInternalHandler != NULL)
    {
        if(isISRContext)
        {
            result = OSAL_SEM_PostISR(&semStackInternalHandler);
        }
        else
        {
            result = OSAL_SEM_Post(&semStackInternalHandler);
        }
    }
    else
    {
        MiMac_TaskHandler();
    }
    
    (void)result;
}