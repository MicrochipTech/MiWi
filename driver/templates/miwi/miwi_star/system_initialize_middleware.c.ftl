    // Create MiWi Stack Message QUEUE
    OSAL_QUEUE_Create(&miwiRequestQueueHandle, QUEUE_LENGTH_MIWI, QUEUE_ITEM_SIZE_MIWI);

    // Initialize MiWi Stack
    MiWi_Init(&osalAPIList, &miwiRequestQueueHandle);

