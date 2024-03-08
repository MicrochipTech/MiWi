# MiApp APIs

The following table lists the supported APIs.

|S. No.|Supported APIs|SupportedTopology|
|------|--------------|-----------------|
|1|miwi\_status\_t MiApp\_ProtocolInit \(void\)|Mesh|
|2|bool MiApp\_Set\(enum id, uint8\_t value\)|Mesh|
|3|bool MiApp\_StartNetwork\(uint8\_t Mode, uint8\_t ScanDuration, uint32\_t ChannelMap, FUNC ConfCallback\)|Mesh|
|4|uint8\_t MiApp\_SearchConnection\(uint8\_t ScanDuration, uint32\_t ChannelMap, FUNC ConfCallback\)|Mesh|
|5|uint8\_t MiApp\_EstablishConnection\(uint8\_t Channel, uint8\_t addr\_len, uint8\_t addr, uint8\_t Capability\_info, FUNCConfCallback\)|Mesh|
|6|void MiApp\_RemoveConnection\(uint8\_t ConnectionIndex\)|Mesh|
|7|void MiApp\_ConnectionMode\(uint8\_t Mode\)|Mesh|
|8|MiApp\_SendData\(uint8\_t addr\_len, uint8\_t addr, uint8\_t len, uint8\_t pointer, FUNC ConfCallback\)|Mesh|
|9|MiApp\_SubscribeDataIndicationCallback\(FUNC callback\)|Mesh|
|10|uint8\_t MiApp\_NoiseDetection\(uint32\_t ChannelMap, uint8\_t ScanDuration, uint8\_t DetectionMode, OUTPUT uint8\_tNoiseLevel\)|Mesh|
|11|uint8\_t MiApp\_TransceiverPowerState\(uint8\_t Mode\)|Mesh|
|12|bool MiApp\_InitChannelHopping\(uint32\_t ChannelMap\)|Mesh|
|13|bool MiApp\_ResyncConnection\(uint8\_t ConnectionIndex, uint32\_t ChannelMap\)|Mesh|
|16|bool MiApp\_Set\(enum id, uint8\_t value \)|Mesh|
|17|bool MiApp\_IsMemberOfNetwork\(void\)|Mesh|
|18|bool MiApp\_Get\(enum id, uint8\_t value \)|Mesh|
|20|bool MiApp\_SubscribeReConnectionCallback\(ReconnectionCallback\_t callback\)|Mesh|
|21|bool MiApp\_ResetToFactoryNew\(void\)|Mesh|
|22|bool MiApp\_ReadyToSleep\(uint32\_t\* sleepTime\)|Mesh|
|<br /> 23<br />|<br /> bool MiApp\_ManuSpecSendData\(uint8\_t addr\_len, uint8\_t \*addr, uint8\_t msglen,<br /> uint8\_t \*msgpointer, uint8\_t msghandle, bool ackReq, DataConf\_callback\_t<br /> ConfCallback\)<br />|<br /> Mesh<br />|
|<br /> 24<br />|<br /> bool MiApp\_SubscribeManuSpecDataIndicationCallback\(PacketIndCal lback\_t<br /> callback\)<br />|<br /> Mesh<br />|
|<br /> 25<br />|<br /> bool MiApp\_IsConnected\(void\)<br />|<br /> Mesh<br />|
|<br /> 26<br />|<br /> uint16\_t MiApp\_MeshGetNextHopAddr\(uint16\_t destAddress\)<br />|<br /> Mesh<br />|

