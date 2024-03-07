# WPAN\_Pandescriptor\_t Struct

**Parent topic:**[MAC Data Structures](GUID-D83EFB67-1CD2-4DDB-825D-8A6090B47CA1.md)

## C

```c
typedef struct wpan_pandescriptor_tag {
    /** Coordinator address specification in received beacon frame */
    WPAN_AddrSpec_t CoordAddrSpec;
    /** The current logical channel used by the network */
    uint8_t LogicalChannel;
    /** The current channel page occupied by the network */
    uint8_t ChannelPage;
    /** Superframe specification in received beacon frame */
    uint16_t SuperframeSpec;
    /** Set to true if the beacon is from a PAN coordinator accepting GTS requests */
    bool GTSPermit;
    /** LQI at which the beacon was received.Lower values represent poorer link quality. */
    uint8_t LinkQuality;

#ifdef ENABLE_TSTAMP
    /** Time at which the beacon frame was received, in symbol counts.  This
    * quantity shall be interpreted as only 24-bits, with the most significant
    * 8-bits entirely ignored. */
    uint32_t TimeStamp;
#endif
} WPAN_Pandescriptor_t;

```

## Summary

This structure contain PAN descriptor information

## Description

None

## Remarks

None

