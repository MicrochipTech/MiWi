# USR\_MLME\_StartConf callback Function

**Parent topic:**[Start APIs](GUID-14E2302D-8D9E-4A99-86B4-89FAA3FB6C35.md)

## C

```c
void USR_MLME_StartConf(uint8_t status)
```

## Summary

Callback function that must be implemented by application \(NHLE\) for MAC service MLME-START.confirm.

## Description

This function implemented by application \(NHLE\) for MAC service MLME-START.confirm.

## Precondition

WPAN\_Init\(\) should have been called before calling this function.

## Parameters

|Param|Description|
|-----|-----------|
|status|Result of requested start operation|

## Returns

None.

## Example

```c
void USR_MLME_StartConf(uint8_t status)
{
    status = status;
}
```

## Remarks

There is weak function for this callback. User has to define own implementation for required operation on the reception of particular callback.

