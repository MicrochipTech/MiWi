/*
* \file  led.c
*
* \brief led Abstraction for MiWi Protocol implementation
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
*
* Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
*/


#include "definitions.h"
#if defined(LED_ENABLED)
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
void LED_Initialize(void)
{

}    
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
void USER_LED_On(LED_ENUM LedNumber)
{
    if (LedNumber == LED0)  
    { 
//        USER_LED_On();
    }  
    else if (LedNumber == LED1)
    {
//        RGB_LED_GREEN_On();
    }
    else if (LedNumber == LED2)
    {
        
    }
    else
    {
        
    }
}

void RGB_LED_GREEN_On(LED_ENUM LedNumber)
{
    if (LedNumber == LED0)  
    { 
//        USER_LED_On();
    }  
    else if (LedNumber == LED1)
    {
//        RGB_LED_GREEN_On();
    }
    else if (LedNumber == LED2)
    {
       /* Empty else if block due to specific logic requirements */ 
    }
    else
    {
        /* Empty else block due to specific logic requirements */
    }
}
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
void USER_LED_Off(LED_ENUM LedNumber)
{
    if (LedNumber == LED0)  
    { 
//        USER_LED_Off();
    }  
    else if (LedNumber == LED1)
    {
//        RGB_LED_GREEN_Off();
    }
    else if (LedNumber == LED2)
    {
        /* Empty else if block due to specific logic requirements */
    }
    else
    {
        /* Empty else block due to specific logic requirements */
    }

}
void RGB_LED_GREEN_Off(LED_ENUM LedNumber)
{
    if (LedNumber == LED0)  
    { 
//        USER_LED_On();
    }  
    else if (LedNumber == LED1)
    {
//        RGB_LED_GREEN_On();
    }
    else if (LedNumber == LED2)
    {
        /* Empty else if block due to specific logic requirements */
    }
    else
    {
       /* Empty else block due to specific logic requirements */ 
    }
}
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
void USER_LED_Toggle(LED_ENUM LedNumber)
{
    if (LedNumber == LED0)  
    { 
//        USER_LED_Toggle();
    }  
    else if (LedNumber == LED1)
    {
//        RGB_LED_GREEN_Toggle();
    }
    else if (LedNumber == LED2)
    {
        /* Empty else if block due to specific logic requirements */
    }
    else
    {
        /* Empty else block due to specific logic requirements */
    }

}

void RGB_LED_GREEN_Toggle(LED_ENUM LedNumber)
{
    if (LedNumber == LED0)  
    { 
//        USER_LED_Toggle();
    }  
    else if (LedNumber == LED1)
    {
//        RGB_LED_GREEN_Toggle();
    }
    else if (LedNumber == LED2)
    {
       /* Empty else if block due to specific logic requirements */ 
    }
    else
    {
      /* Empty else block due to specific logic requirements */  
    }

}
#endif