# MLME\_BeaconNotifyInd\_t Struct

**Parent topic:**[MAC Data Structures](GUID-D83EFB67-1CD2-4DDB-825D-8A6090B47CA1.md)

## C

```c
typedef struct mlme_beacon_notify_ind_tag {
    /** This identifies the message as \ref MLME_BEACON_NOTIFY_INDICATION */
    enum msg_code cmdcode;
    /** The beacon sequence number. */
    uint8_t BSN;
    /** The PANDescriptor for the received beacon. */
    WPAN_Pandescriptor_t PANDescriptor;
    /** The beacon pending address specification. */
    uint8_t PendAddrSpec;
    
    /** The list of addresses of the devices for which the beacon source has
    * data. */
    uint8_t *AddrList;
    
    /** The number of octets contained in the beacon payload of the beacon frame
    * received by the MAC sublayer. */
    uint8_t sduLength;
    
    /** The set of octets comprising the beacon payload to be transferred from
    * the MAC sublayerentity to the next higher layer. */
    uint8_t *sdu;
} MLME_BeaconNotifyInd_t;

```

## Summary

MLME\_BeaconNotifyInd\_t holds the MLME-BEACON-NOTIFY.indication message structure

## Description

None

## Remarks

None

