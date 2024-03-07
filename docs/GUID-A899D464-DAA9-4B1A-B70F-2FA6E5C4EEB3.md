# USR_MLME_GetConf callback Function

## C

```c
void USR_MLME_GetConf(uint8_t status,uint8_t PIBAttribute, void *PIBAttributeValue)
```

## Summary

Callback function that must be implemented by application (NHLE) for MAC service MLME-GET.confirm.

## Description

This function implemented by application (NHLE) for MAC service MLME-GET.confirm.

## Precondition

WPAN_Init() should have been called before calling this function.  

## Parameters

| Param | Description |
|:----- |:----------- |
| status | Result of requested PIB attribute get operation |
| PIBAttribute | Retrieved PIB attribute |
| PIBAttributeValue | Pointer to data containing retrieved PIB attribute|

## Returns

None.

## Example

```c
void USR_MLME_GetConf(uint8_t status,uint8_t PIBAttribute, void *PIBAttributeValue)
{
    status = status;
    PIBAttribute = PIBAttribute;
    PIBAttributeValue = PIBAttributeValue;
}

```

## Remarks

There is weak function for this callback. User has to define own implementation for required operation on the reception of particular callback.