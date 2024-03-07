# MiApp\_StartConnection

|API|bool MiApp\_StartConnection\(uint8\_t Mode, uint8\_t ScanDuration,uint32\_t ChannelMap,connectionConf\_callback\_t ConfCallback\)|
|Description|This is the primary user interface function for the application layer to start PAN. Usually, this function is called by the PAN coordinator which is the first in the PAN. The PAN coordinator may start the PAN after a noise scan if specified in the input mode.|
|Pre-Condition|Protocol initialization must be done.|
|Parameters|• uint8\_t Mode – whether to start a PAN after a noise scan. Possible modes are as follows.– START\_CONN\_DIRECT – starts PAN directly without noise scan.– START\_CONN\_ENERGY\_SCN – performs an energy scan first, then starts the PAN onthe channel with least noise.– START\_CONN\_CS\_SCN – performs a carrier-sense scan first, then starts the PAN onthe channel with least noise.• uint8\_t ScanDuration – maximum time to perform scan on single channel. The value isfrom 5 to 14. The real time to perform scan can be calculated in following formula from IEEE802.15.4 specification:960 x \(2^ScanDuration + 1\) x 10^\(-6\) secondScanDuration is discarded if the connection mode is START\_CONN\_DIRECT.• uint32\_t ChannelMap – bit map of channels to perform noise scan. The 32-bit double wordparameter uses one bit to represent corresponding channels from 0 to 31. For instance,0x00000003 represent to scan channel 0 and channel 1. ChannelMap is discarded if theconnection mode is START\_CONN\_DIRECT.• connectionConf\_callback\_t ConfCallback – callback routine which is called upon theinitiated connection procedure is performed.|
|Returns|A boolean to indicate if PAN is started successfully.|
|Example|<code\>// start the PAN on the least noisy channel after scanning all possiblechannels.MiApp\_StartConnection\(START\_CONN\_ENERGY\_SCN, 10, 0x07FFF800, callback\);</code\>|
|Remarks|None|

**Parent topic:**[MiApp API Description](GUID-A47B6424-A497-498C-8B1E-044F12F201A6.md)

