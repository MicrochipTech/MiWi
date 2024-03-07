# MiApp\_Set

|API|bool MiApp\_Set\(set\_params id, uint8\_t \*value\)|
|Description|This is the primary user interface function to set the different values in the MiWiTM stack.|
|Pre-Condition|Protocol initialization must be done.|
|Parameters|• set\_params id – The identifier of the value to be set• value – The value to be set|
|Returns|A boolean to indicate if set operation is performed successfully.|
|Example|<code\>if\( true == MiApp\_Set\(CHANNEL, 15\) \)\{// channel changes successfully\}</code\>|
|Remarks|None|

**Parent topic:**[MiApp API Description](GUID-A47B6424-A497-498C-8B1E-044F12F201A6.md)

