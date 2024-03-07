# MAC APIs

Following are the list of WPAN APIs which can be used by the higher layer to interact with MAC<br /> layer.

|S.No|API Name|Description|API Type|
|----|--------|-----------|--------|
|1|**[WPAN\_Init\(\)](GUID-5C5F77E5-098C-42C6-ADB0-61CF0FAF1BCD.md)**|<br /> Initializes all stack resources including the micro-controller and<br /> transceiver using functions.<br /> provided by the PHY and the PAL<br />|Asynchronous|
|2|**[WPAN\_Task\(\)](GUID-1A6C9289-EEB1-4913-A363-9B561B259530.md)**|<br /> It invokes the corresponding task functions of the MCL, PHY, and<br /> PAL<br /> Using the MAC software package it is required to call this function<br /> frequently supporting a round robin approach. <br /> This ensures that the different layers state machines are served and<br /> their queues are processed.<br />|Asynchronous|
|3|**[WPAN\_MCPS\_Datareq](GUID-AB56B4B4-C121-434F-9F04-B2D6815A2C5D.md)**\(uint8\_t SrcAddrMode,<br /> WPAN\_AddrSpec\_t \*DstAddrSpec, uint8\_t msduLength, uint8\_t \*msdu, uint8\_t<br /> msduHandle, uint8\_t TxOptions\)|This function is called to Initiate MCPS-DATA.request service and<br /> have it placed in the MCPS-SAP queue.|Asynchronous|
|4|**[WPAN\_MCPS\_PurgeReq](GUID-6B2A5252-89FC-4B6F-9084-FBD3CFE5C425.md)**\(const uint8\_t<br /> msduHandle\)|This function is called to Initiate MCPS-PURGE.request service and<br /> have it placed in the MCPS-SAP queue.|Asynchronous|
|5|**[WPAN\_MLME\_AssociateReq](GUID-874B7D32-43B7-4092-AEDF-D600FDD95170.md)**\(uint8\_t<br /> LogicalChannel, uint8\_t ChannelPage, WPAN\_AddrSpec\_t \*CoordAddrSpec,<br /> uint8\_t CapabilityInformation\)|This function is called to Initiate MLME-ASSOCIATE.request service<br /> and have it placed in the MLME-SAP queue.|Asynchronous|
|6|**[WPAN\_MLME\_AssociateResp](GUID-BBCEC43E-A5E9-4BA8-9190-18537871CF38.md)**\(uint64\_t<br /> DeviceAddress, uint16\_t AssocShortAddress, uint8\_t status\);|This function called to Initiate MLME-ASSOCIATE.response service and<br /> place it in the MLME-SAP queue.|Asynchronous|
|7|**[WPAN\_MLME\_DisassociateReq](GUID-54BDDC9E-02B9-44FF-9DE0-753E12D96FD8.md)**\(WPAN\_AddrSpec\_t \*DeviceAddrSpec,<br /> uint8\_t DisassociateReason, bool TxIndirect\)|This function is called to Initiate MLME-DISASSOCIATE.request service<br /> and have it placed in the MLME-SAP queue.|Asynchronous|
|8|**[WPAN\_MLME\_GetReq](GUID-3437A75F-F875-46A5-8B65-705340A0E015.md)**\(uint8\_t<br /> PIBAttribute\)|This function is called to Initiate MLME-GET.request service and have<br /> it placed in the MLME-SAP queue and for retrieve the PIB<br /> attributes.|Asynchronous|
|9|**[WPAN\_MLME\_OrphanResp](GUID-5D096751-E6A0-4345-A02A-CF32C2599CF0.md)**\(uint64\_t<br /> OrphanAddress, uint16\_t ShortAddress, bool AssociatedMember\)|This function is called to Initiate MLME-ORPHAN.response service and<br /> have it placed in MLME\_SAP queue.|Asynchronous|
|10|**[WPAN\_MLME\_PollReq](GUID-B4684D7F-3E09-4D26-A45B-17843F107F93.md)**\(WPAN\_AddrSpec\_t<br /> \*CoordAddrSpec\)|This function is called to Initiate MLME-POLL.request service and<br /> have it placed in the MLME-SAP queue.|Asynchronous|
|11|**[WPAN\_MLME\_ResetReq](GUID-E48D2F16-7917-4A45-894C-7B80A33B71C8.md)**\(bool<br /> SetDefaultPib\)|This function is called to Initiate MLME-RESET.request service and<br /> have it placed in the MLME-SAP queue, SetDefaultPib to set all PIB<br /> values to their respective defaults.|Asynchronous|
|12|**[WPAN\_MLME\_SetReq](GUID-A33C8D52-8A11-44AC-909C-7777B56AB41A.md)**\(uint8\_t PIBAttribute,<br /> void \*PIBAttributeValue\)|This function is called to Initiate MLME-SET.request service and have<br /> it placed in MLME\_SAP queue and for set PIB attributes.|Asynchronous|
|13|**[WPAN\_MLME\_RxEnableReq](GUID-46FD057C-039F-4A30-A491-D7F659FFAD69.md)**\(bool DeferPermit,<br /> uint32\_t RxOnTime, uint32\_t RxOnDuration\)|This function is called to Initiate MLME-RX-ENABLE.request service<br /> and have it placed in the MLME-SAP queue.|Asynchronous|
|14|**[WPAN\_MLME\_ScanReq](GUID-B509C8FA-73E4-41DD-919F-053955BEB0FA.md)**\(uint8\_t ScanType,<br /> uint32\_t ScanChannels, uint8\_t ScanDuration, uint8\_t<br /> ChannelPage\)|This function is called to Initiate MLME-SCAN.request service and<br /> have it placed in the MLME-SAP queue.|Asynchronous|
|15|**[WPAN\_MLME\_StartReq](GUID-F06E1F1E-B885-41E1-BC72-68CDC78EF002.md)**\(uint16\_t PANId,<br /> uint8\_t LogicalChannel, uint8\_t ChannelPage, uint8\_t BeaconOrder,<br /> uint8\_t SuperframeOrder, bool PANCoordinator, bool BatteryLifeExtension,<br /> bool CoordRealignment\)|This function is called to Initiate MLME-START service and have it<br /> placed in the MLME-SAP queue|Asynchronous|
|16|**[USR\_MCPS\_DataConf](GUID-8DE2481C-46C5-439C-A20B-3828D7CCAA32.md)**\(uint8\_t msduHandle,<br /> uint8\_t status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MCPS-DATA.confirm|NA|
|17|**[USR\_MCPS\_DataInd](GUID-FA859A36-6D24-4253-9687-9AE118A09C84.md)**\(WPAN\_AddrSpec\_t \*<br /> SrcAddrSpec, WPAN\_AddrSpec\_t \* DstAddrSpec,uint8\_t msduLength, uint8\_t \*<br /> msdu,uint8\_t mpduLinkQuality, uint8\_t DSN, uint32\_t Timestamp, uint8\_t<br /> SecurityLevel, uint8\_t KeyIdMode, uint8\_t KeyIndex\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MCPS-DATA.indication|NA|
|18|**[USR\_MCPS\_PurgeConf](GUID-0396B8BE-9E1D-471A-BED3-35784CFE0C2F.md)**\(uint8\_t msduHandle,<br /> uint8\_t status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MCPS-PURGE.confirm\(Handle of MSDU to be purged and status of<br /> requested purge operation\)|NA|
|19|**[USR\_MLME\_AssociateConf](GUID-3E9A6E9D-4B4A-4C44-95AB-9697C6854F85.md)**\(uint16\_t<br /> AssocShortAddress, uint8\_t status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-ASSOCIATE.confirm\(association status and allocated<br /> short address by coordinator\)|NA|
|20|**[USR\_MLME\_AssociateInd](GUID-6175DA83-63D9-4558-AF54-8D26E5FAB62A.md)**\(uint64\_t<br /> DeviceAddress, uint8\_t CapabilityInformation\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-ASSOCIATE.indication\(Extended address of device<br /> requesting and Capabilities of device requesting association|NA|
|21|**[USR\_MLME\_BeaconNotifyInd](GUID-9D37CED5-A262-492D-A49A-F4C81FE8643C.md)**\(uint8\_t BSN,<br /> WPAN\_Pandescriptor\_t \*PANDescriptor, uint8\_t PendAddrSpec, uint8\_t<br /> \*AddrList, uint8\_t sduLength, uint8\_t \*sdu\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-BEACON-NOTIFY.indication|NA|
|22|**[USR\_MLME\_CommStatusInd](GUID-BC4F0ACF-ACB4-4AE1-9ACE-1B763BA599D3.md)**\(WPAN\_AddrSpec\_t<br /> \*SrcAddrSpec, WPAN\_AddrSpec\_t \*DstAddrSpec, uint8\_t status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-COMM-STATUS.indication|NA|
|23|**[USR\_MLME\_DisassociateConf](GUID-46B983E5-AD18-4C30-A938-60E6D39594E0.md)**\(uint8\_t<br /> status, WPAN\_AddrSpec\_t \*DeviceAddrSpec\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-DISASSOCIATE.confirm\(status of requested disassociate<br /> operation and DeviceAddrSpec Pointer to WPAN\_AddrSpec\_t structure for<br /> device that has either requested disassociation or been instructed to<br /> disassociate by its coordinator\)|NA|
|24|**[USR\_MLME\_DisassociateInd](GUID-A4275CCB-6BF5-409B-A6DA-E965AE3A5935.md)**\(uint64\_t<br /> DeviceAddress, uint8\_t DisassociateReason\)|User call back function for MLME-DISASSOCIATE.indication\(Extended<br /> address of device which initiated the disassociation request and Reason<br /> for the disassociation|NA|
|25|**[USR\_MLME\_GetConf](GUID-A899D464-DAA9-4B1A-B70F-2FA6E5C4EEB3.md)**\(uint8\_t status,uint8\_t<br /> PIBAttribute, uint8\_t PIBAttributeIndex, void<br /> \*PIBAttributeValue\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-GET.confirm\(Status of requested PIB attribute get<br /> operation and retrieved PIB attribute and values\)|NA|
|26|**[USR\_MLME\_OrphanInd](GUID-FAE3C695-2193-4956-8460-14DCE23D0BBB.md)**\(uint64\_t<br /> OrphanAddress\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-ORPHAN.indication\(Address of orphaned device\)|NA|
|27|**[USR\_MLME\_PollConf](GUID-D4CBDBBD-37CF-448E-8F8C-2F429B363338.md)**\(uint8\_t<br /> status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-POLL.confirm\(Result of requested poll<br /> operation\)|NA|
|28|**[USR\_MLME\_ResetConf](GUID-44AA5421-9434-403B-8152-A8E4021BC3DD.md)**\(uint8\_t<br /> status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-RESET.confirm\(Result of requested reset<br /> operation\)|NA|
|29|**[USR\_MLME\_RxEnableConf](GUID-631A25C6-F3EE-4A61-A4AC-67F8B988F9DB.md)**\(uint8\_t<br /> status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-RX-ENABLE.confirm\(Result of requested receiver enable<br /> operation\)|NA|
|30|**[USR\_MLME\_ScanConf](GUID-23961FA1-BC0C-4920-A921-4EEC1F257DF6.md)**\(uint8\_t status,<br /> uint8\_t ScanType, uint8\_t ChannelPage, uint32\_t UnscannedChannels,<br /> uint8\_t ResultListSize, void \*ResultList\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-SCAN.confirm| |
|31|**[USR\_MLME\_SetConf](GUID-02EC4650-C25C-4942-9FD5-2B069ED9B88B.md)**\(uint8\_t status,<br /> uint8\_t PIBAttribute\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-SET.confirm\(Result of requested PIB attribute set<br /> operation and updated PIB attribute\)|NA|
|32|**[USR\_MLME\_StartConf](GUID-CFF9F342-F6F1-4A80-A8BD-CFD7E88A5B67.md)**\(uint8\_t<br /> status\)|Callback function that must be implemented by application \(NHLE\) for<br /> MAC service MLME-START.confirm\(Result of requested start<br /> operation\)|NA|
|33|**[MAC\_ReadyToSleep](GUID-3B828C15-8319-40E2-9B07-BFC9AA68D9D2.md)**\(\)|This function is used to check the MAC layer for its status before<br /> Device going to sleep. This function verifies the MAC internal queues<br /> for the pending request/indication to process and returns the<br /> status.|Asynchronous|
|34|**[MAC\_Wakeup](GUID-60D77ADC-0A40-4826-8C33-18DC43750ADF.md)**\(\)|This function implements the post sleep functionalities. If device is<br /> going to Deep sleep, then this function must be called after device<br /> wakeup routine to retrieve the Retention RAM parameters to system<br /> RAM|Asynchronous|

-   **[System APIs](GUID-EFC10569-E631-492C-967A-D52BE5CD43A7.md)**  

-   **[Reset APIs](GUID-2885E7AA-7EF0-44BC-A4EA-281259DCD251.md)**  

-   **[Set & Get PIB Attributes APIs](GUID-60837011-98E8-4371-BD13-E71E384FBA2A.md)**  

-   **[Channel Scan APIs](GUID-B33CFFD6-DB3D-4314-81F1-23D388AE9F67.md)**  

-   **[Start APIs](GUID-14E2302D-8D9E-4A99-86B4-89FAA3FB6C35.md)**  

-   **[Association APIs](GUID-8A8077CE-E940-4E10-8DA9-815C6406A784.md)**  

-   **[Disassociation APIs](GUID-6FE28373-B7C9-4A15-B1B8-390B864120B2.md)**  

-   **[Poll APIs](GUID-06A663DE-FB0B-4E6D-A4AE-37A8123DB08C.md)**  

-   **[Data Service APIs](GUID-626BC179-2C84-45A5-9075-47D7BA9DB491.md)**  

-   **[Purge APIs](GUID-1F251613-C300-488B-AECA-DBE46E6E21F2.md)**  

-   **[Beacon Notification APIs](GUID-5B2DF770-3054-4D19-B9EC-7943F6EC1253.md)**  

-   **[Orphan Notification APIs](GUID-19870B21-B264-4D75-9015-8AFE421C5173.md)**  

-   **[Communication Status APIs](GUID-1F9CC9B0-7DC1-4B70-B925-58E315ECDE0D.md)**  

-   **[Rx Enable APIs](GUID-881C03A5-AD3D-4A69-AAAD-659F664E313D.md)**  

-   **[Sleep APIs](GUID-3A8B83A3-ACFD-4B67-A34D-852B5485CC37.md)**  


