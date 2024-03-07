# USR\_MLME\_CommStatusInd callback Function

**Parent topic:**[Communication Status APIs](GUID-1F9CC9B0-7DC1-4B70-B925-58E315ECDE0D.md)

## C

```c
void USR_MLME_CommStatusInd(WPAN_AddrSpec_t *SrcAddrSpec, WPAN_AddrSpec_t *DstAddrSpec, uint8_t status);
```

## Summary

Callback function that must be implemented by application \(NHLE\) for MAC service MLME-COMM-STATUS.indication.

## Description

This function implemented by application \(NHLE\) for MAC service MLME-COMM-STATUS.indication.

## Precondition

WPAN\_Init\(\) should have been called before calling this function.

## Parameters

|Param|Description|
|-----|-----------|
|SrcAddrSpec|Pointer to source address specification|
|DstAddrSpec|Pointer to destination address specification|
|status|Result for related response operation|

## Returns

None.

## Example

```c
void USR_MLME_CommStatusInd(WPAN_AddrSpec_t *SrcAddrSpec, WPAN_AddrSpec_t *DstAddrSpec, uint8_t status);

{
    SrcAddrSpec = SrcAddrSpec;
    DstAddrSpec = DstAddrSpec;
    status = status;
}
```

## Remarks

There is weak function for this callback. User has to define own implementation for required operation on the reception of particular callback.

