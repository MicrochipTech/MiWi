#define TASK_MIWI_STACK_SIZE (8 *1024 / sizeof(portSTACK_TYPE))
#define TASK_MIWI_PRIORITY (tskIDLE_PRIORITY + 3)

<#if HarmonyCore.SELECT_RTOS == "FreeRTOS">
/* Handle for the MAC_Tasks. */
TaskHandle_t xMiMAC_Tasks;

void _MiMAC_Tasks(  void *pvParameters  )
{     
    while(1)
    {
        MiMAC_Tasks();
    }
}
</#if>