/**
* \file  led.h
*
* \brief led Abstraction for MiWi Protocol interface
*
* Copyright (c) 2024 Microchip Technology Inc. and its subsidiaries. 
*
* \asf_license_start
*
* \page License
*
* Subject to your compliance with these terms, you may use Microchip
* software and any derivatives exclusively with Microchip products. 
* It is your responsibility to comply with third party license terms applicable 
* to your use of third party software (including open source software) that 
* may accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, 
* WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, 
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, 
* AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE 
* LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL 
* LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE 
* SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE 
* POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT 
* ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY 
* RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, 
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
* \asf_license_stop
*
*/
/*
* Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
*/

#ifndef LEDBLOCKING_H
#define LEDBLOCKING_H

#include "config/default/definitions.h"

typedef enum
{
	LED0,
	LED1,
	LED2
}LED_ENUM;

#if defined(LED_ENABLED)
#ifndef LED_COUNT
#define LED_COUNT 3
#endif

#define LED_On(lednum, ledenum) ((lednum == 1) ? USER_LED_On(ledenum) : RGB_LED_GREEN_On(ledenum))
#define LED_Off(lednum, ledenum) ((lednum == 1) ? USER_LED_Off(ledenum) : RGB_LED_GREEN_Off(ledenum))
#define LED_Toggle(lednum, ledenum) ((lednum == 1) ? USER_LED_Toggle(ledenum) : RGB_LED_GREEN_Toggle(ledenum))

#if LED_COUNT > 2
#define LED_NETWORK       LED0
#define LED_DATA          LED1
#define LED_BLINK         LED2
#define LED_IDENTIFY      LED0
#elif LED_COUNT == 2

#define LED_NETWORK       LED0
#define LED_DATA          LED1
#define LED_BLINK         LED1
#define LED_IDENTIFY      LED1

#elif LED_COUNT == 1
#define LED_NETWORK       LED0
#define LED_DATA          LED0
#define LED_BLINK         LED0
#define LED_IDENTIFY      LED0
#endif

#ifdef LED0_ACTIVE_LEVEL
#define LED_NETWORK_GPIO       LED0
#define LED_DATA_GPIO          LED0
#define LED_BLINK_GPIO         LED0
#define LED_IDENTIFY_GPIO      LED0
#define LED_IDENTIFY_ACTIVE_LEVEL  LED0_ACTIVE_LEVEL
#define LED_IDENTIFY_INACTIVE_LEVEL  LED0_ACTIVE_LEVEL
#define LED_NETWORK_ACTIVE_LEVEL  LED0_INACTIVE_LEVEL
#define LED_NETWORK_INACTIVE_LEVEL  LED0_INACTIVE_LEVEL
#define LED_DATA_ACTIVE_LEVEL  LED0_ACTIVE_LEVEL
#define LED_DATA_INACTIVE_LEVEL  LED0_INACTIVE_LEVEL
#define LED_BLINK_ACTIVE_LEVEL  LED0_ACTIVE_LEVEL
#define LED_BLINK_INACTIVE_LEVEL  LED0_INACTIVE_LEVEL
#endif
#endif
/*********************************************************************
* Function:         void LED_Initialize(void)
*
* PreCondition:     none
*
* Input:	    none
*
* Output:	    none
*
* Side Effects:	    LED is configured
*
* Overview:         Configure the LCD
*
* Note:             None
********************************************************************/
void LED_Initialize(void);

/*********************************************************************
* Function:         void LED_On(LED_ENUM LedNumber)
*
* PreCondition:     none
*
* Input:	    none
*
* Output:	    none
*
* Side Effects:	    None
*
* Overview:         LED turns on
*
* Note:             None
********************************************************************/
void USER_LED_On(LED_ENUM LedNumber);
void RGB_LED_GREEN_On(LED_ENUM LedNumber);
/*********************************************************************
* Function:         void LED_Off(LED_ENUM LedNumber)
*
* PreCondition:     none
*
* Input:	    none
*
* Output:	    none
*
* Side Effects:	    None
*
* Overview:         LED turns off
*
* Note:             None
********************************************************************/
void USER_LED_Off(LED_ENUM LedNumber);
void RGB_LED_GREEN_Off(LED_ENUM LedNumber);
/*********************************************************************
* Function:         void LED_Toggle(LED_ENUM LedNumber)
*
* PreCondition:     none
*
* Input:	    none
*
* Output:	    none
*
* Side Effects:	    None
*
* Overview:         LED turns off
*
* Note:             None
********************************************************************/
void USER_LED_Toggle(LED_ENUM LedNumber);
void RGB_LED_GREEN_Toggle(LED_ENUM LedNumber);  
#endif
