# MLME\_AssociateInd\_t Struct

**Parent topic:**[MAC Data Structures](GUID-D83EFB67-1CD2-4DDB-825D-8A6090B47CA1.md)

## C

```c
typedef struct mlme_associate_ind_tag {
    /** This identifies the message as \ref MLME_ASSOCIATE_INDICATION */
    enum msg_code cmdcode;
    /** The address of the device requesting association. */
    uint64_t DeviceAddress;
    
    /** The operational capabilities of the device requesting association. */
    uint8_t CapabilityInformation;
} MLME_AssociateInd_t;

```

## Summary

MLME\_AssociateInd\_t holds the MLME-ASSOCIATE.indication message structure

## Description

None

## Remarks

None

