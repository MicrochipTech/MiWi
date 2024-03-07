<#if HarmonyCore.SELECT_RTOS == "BareMetal">
    /* Call the IEEE_802154_MAC Task Handler function */
	MiMac_TaskHandler();
<#else>
<#if HarmonyCore.SELECT_RTOS == "FreeRTOS">

    /* Create FreeRTOS task for IEEE_802154_MAC */
	 (void)xTaskCreate((TaskFunction_t) _MiMAC_Tasks,
                "MiMAC_Tasks",
                TASK_MIWI_STACK_SIZE,
                NULL,
                TASK_MIWI_PRIORITY,
                &xMiMAC_Tasks);
</#if>
</#if>