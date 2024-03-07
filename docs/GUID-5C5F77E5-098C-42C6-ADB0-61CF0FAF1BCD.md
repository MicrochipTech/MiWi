# WPAN\_Init Function

**Parent topic:**[System APIs](GUID-EFC10569-E631-492C-967A-D52BE5CD43A7.md)

## C

```c
MAC_Retval_t WPAN_Init(void)
```

## Summary

Initialization of MAC Layer

## Description

This function initializes the MAC Layer. This is function is called to Initializes all stack resources<br />including the microcontroller and transceiver using functions provided by the PHY and the PAL.

## Precondition

PHY\_Init\(\) should have been called before calling this function.

## Parameters

None.

## Returns

*MAC\_SUCCESS* - if MAC initialized successfully

*FAILURE* - otherwise

## Example

```c
MAC_Retval_t retVal = FAILURE;

retVal = WPAN_Init();
if (MAC_SUCCESS != retVal)
{
     while(1);
}
```

## Remarks

This routine must be called before any of the MAC-WPAN function is called

