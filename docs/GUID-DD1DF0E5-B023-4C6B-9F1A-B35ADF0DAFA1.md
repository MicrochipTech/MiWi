# MiApp\_SubscribeDataIndicationCallback

|API|bool MiApp\_SubscribeDataIndicationCallback\(PacketIndCallback\_t callback\)|
|Description|This is the primary user interface functions for the application layer to call the Microchip proprietary protocol stack to register the message indication callback to the application. The function calls the protocol stack state machine to keep the stack running.|
|Pre-Condition|Protocol initialization is done.|
|Parameters|None|
|Returns|A boolean to indicate if the subscription operation is successful or not.|
|Example|<code\>if\( true == MiApp\_SubscribeDataIndicationCallback\(ind\) \)\{\}</code\\\>|
|Remarks|None|

**Parent topic:**[MiApp API Description](GUID-A47B6424-A497-498C-8B1E-044F12F201A6.md)

