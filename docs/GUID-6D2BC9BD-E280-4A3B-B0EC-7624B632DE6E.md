# MLME\_DisassociateConf\_t Struct

**Parent topic:**[MAC Data Structures](GUID-D83EFB67-1CD2-4DDB-825D-8A6090B47CA1.md)

## C

```c
typedef struct mlme_disassociate_conf_tag {
    /** This identifies the message as \ref MLME_DISASSOCIATE_CONFIRM */
    enum msg_code cmdcode;
    /** The status of the disassociation attempt. */
    uint8_t status;
    
    /** The addressing mode of the device that has either requested
    * disassociation or been instructed to disassociate by its coordinator. */
    uint8_t DeviceAddrMode;
    
    /** The PAN identifier of the device that has either requested disassociation
    * or been instructed to disassociate by its coordinator. */
    uint16_t DevicePANId;
    
    /** The address of the device that has either requested disassociation or
    * been instructed to disassociate by its coordinator. */
    uint64_t DeviceAddress;
} MLME_DisassociateConf_t;

```

## Summary

MLME\_DisassociateConf\_t holds the MLME-DISASSOCIATE.confirm message structure

## Description

None

## Remarks

None

