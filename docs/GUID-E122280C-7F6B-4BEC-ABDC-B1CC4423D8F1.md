# MiApp\_TransceiverPowerState

|API|uint8\_t MiApp\_TransceiverPowerState\(uint8\_t Mode\)|
|Description|This is the primary user interface function for the application layer to set the RF transceiver into sleep or wake up. This function is only available to those wireless nodes that have to disable the transceiver to save battery power.|
|Pre-Condition|Protocol initialization is done.|
|Parameters|uint8\_t Mode – mode of the power state for the RF transceiver to be set. The possible powerstates are following.• POWER\_STATE\_SLEEP – deep sleep mode for the RF transceiver• POWER\_STATE\_WAKEUP – Wake-Up state, or operating state for the RF transceiver• POWER\_STATE\_WAKEUP\_DR – Set the device into the Wake-Up mode and transmit the data request to the device's associated device|
|Returns|The status of the operation. The following are the possible status.• SUCCESS – operation is successful.• ERR\_TRX\_FAIL – Transceiver fails to go to the Sleep or Wake-Up mode.• ERR\_TX\_FAIL – transmission of Data Request command failed. Only available if the inputmode is POWER\_STATE\_WAKEUP\_DR.• ERR\_RX\_FAIL – failed to receive any response to Data Request command. Only available ifthe input mode is POWER\_STATE\_WAKEUP\_DR.• ERR\_INVLAID\_INPUT – invalid input mode.|
|Example|<code\>// put RF transceiver into sleepMiApp\_TransceiverPowerState\(POWER\_STATE\_SLEEP;// Put the MCU into sleepSleep\(\);// wakes up the MCU by WDT, external interrupt or any other means// make sure that RF transceiver to wake up and send out Data RequestMiApp\_TransceiverPowerState\(POWER\_STATE\_WAKEUP\_DR\);</code\>|
|Remarks|None|

**Parent topic:**[MiApp API Description](GUID-A47B6424-A497-498C-8B1E-044F12F201A6.md)

