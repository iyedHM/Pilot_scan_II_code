/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#include "system.h"
#include "usb.h"
/** CONFIGURATION Bits **********************************************/

#pragma config CPUDIV = NOCLKDIV
#pragma config USBDIV = OFF
#pragma config FOSC   = HS
#pragma config PLLEN  = ON
#pragma config FCMEN  = OFF
#pragma config IESO   = OFF
#pragma config PWRTEN = OFF
#pragma config BOREN  = OFF
#pragma config BORV   = 30
#pragma config WDTEN  = OFF
#pragma config WDTPS  = 32768
#pragma config MCLRE  = OFF
#pragma config HFOFST = OFF
#pragma config STVREN = ON
#pragma config LVP    = OFF
#pragma config XINST  = OFF
#pragma config BBSIZ  = OFF
#pragma config CP0    = OFF
#pragma config CP1    = OFF
#pragma config CPB    = OFF
#pragma config WRT0   = OFF
#pragma config WRT1   = OFF
#pragma config WRTB   = OFF
#pragma config WRTC   = OFF
#pragma config EBTR0  = OFF
#pragma config EBTR1  = OFF
#pragma config EBTRB  = OFF

/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
extern int freq_T ;
extern int timerPeriod;
extern int tmr0Value;
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:
            LED_Enable(LED_USB_DEVICE_STATE);
            BUTTON_Enable(BUTTON_DEVICE_CDC_BASIC_DEMO);
            break;
            
        case SYSTEM_STATE_USB_SUSPEND: 
            break;
            
        case SYSTEM_STATE_USB_RESUME:
            break;
    }
}

#if(__XC8_VERSION < 2000)
    #define INTERRUPT interrupt
#else
    #define INTERRUPT __interrupt(high_priority)
#endif

#define _XTAL_FREQ 12000000

void setupTimer0(void) {

    
    

    // Split the 16-bit TMR1 value into high and low bytes
    TMR0H = (tmr0Value >> 8) & 0xFF;// 0xFF is a mask
    TMR0L = tmr0Value & 0xFF;

    // Timer0 configuration
    
    T0CONbits.TMR0ON = 0;   // Turn off Timer0
    T0CONbits.T08BIT = 0;   // Configure Timer0 as 16-bit timer
    T0CONbits.T0CS = 0;     // Select internal instruction cycle clock
    T0CONbits.PSA = 0;      // Enable prescaler
    T0CONbits.T0PS = 0b110; // Set prescaler to 1:256
    T0CONbits.TMR0ON = 1;   // Turn on Timer0

    // Enable Timer1 interrupt
    INTCONbits.TMR0IF = 0;     // Clear Timer1 overflow flag
    INTCONbits.TMR0IE = 1;     // Enable Timer1 overflow interrupt
}

void CustomInterruptHandler(void) {
    if (INTCONbits.RABIF) { // Check if the RB Port Change Interrupt occurred
        uint8_t currentRB4State = PORTBbits.RB4;

        // Check for falling edge on RB4
        if ((IOCBbits.IOCB4) && (currentRB4State == 0)) {
           // signalActive = 1;
          //  toggleCount = 0;
            LATCbits.LATC3 = 1; // Start with a high pulse
            setupTimer0();
            
        }

        // Clear the interrupt flag
        INTCONbits.RABIF = 0;
    }
    
    if (INTCONbits.TMR0IF) { // Check if Timer1 overflowed
        // Your Timer1 interrupt handling code here
        
        // Example: Toggle a pin using the frequency
        LATCbits.LATC3 = ~LATCbits.LATC3;

        // Clear the Timer1 interrupt flag
        INTCONbits.TMR0IF = 0;

        // Reload the Timer1 value based on freq if needed
        setupTimer0();
     
   }
}



void INTERRUPT SYS_InterruptHigh(void)
{
    
   #if defined(USB_INTERRUPT)
       
    USBDeviceTasks();
    
    
   #endif
CustomInterruptHandler();
}
