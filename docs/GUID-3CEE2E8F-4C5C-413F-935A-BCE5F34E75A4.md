# MLME\_PollConf\_t Struct

**Parent topic:**[MAC Data Structures](GUID-D83EFB67-1CD2-4DDB-825D-8A6090B47CA1.md)

## C

```c
typedef struct mlme_poll_conf_tag {
    /** This identifies the message as \ref MLME_POLL_CONFIRM */
    enum msg_code cmdcode;
    /** The status of the data request. */
    uint8_t status;
} MLME_PollConf_t;

```

## Summary

MLME\_PollConf\_t holds the MLME-POLL.confirm message structure

## Description

None

## Remarks

None

