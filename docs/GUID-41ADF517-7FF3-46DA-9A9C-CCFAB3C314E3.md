# MLME\_StartConf\_t Struct

**Parent topic:**[MAC Data Structures](GUID-D83EFB67-1CD2-4DDB-825D-8A6090B47CA1.md)

## C

```c
typedef struct mlme_start_conf_tag {
    /** This identifies the message as \ref MLME_START_CONFIRM */
    enum msg_code cmdcode;
    
    /** The result of the attempt to start using an updated superframe
    * configuration. */
    uint8_t status;
} MLME_StartConf_t;

```

## Summary

MLME\_StartConf\_t holds the MLME-START.confirm message structure

## Description

None

## Remarks

None

