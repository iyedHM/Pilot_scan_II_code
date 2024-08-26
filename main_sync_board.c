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

/** INCLUDES *******************************************************/
#include "system.h"
#include <stdio.h> 
#include <string.h>
#include "app_device_cdc_basic.h"
#include "app_led_usb_status.h"

#include "usb.h"
#include "usb_device.h"
#include "usb_device_cdc.h"

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
#define _XTAL_FREQ 12000000
int freq_T ;
int timerPeriod;
int tmr0Value;
static uint8_t readBuffer[64];
static uint8_t writeBuffer[64];
int i = 0;
volatile uint8_t prevRB4State ;
#define TRIGGER  LATCbits.LATC3 // PIN_5, RC3 
#define PWM      PORTBbits.RB5  // PIN_12, RB5
#define PPS      PORTBbits.RB4  // PIN_13, RB4
#define POWER_COM   LATBbits.LATB6  // PIN_11, RB6

void init(void);
void custom__delay_ms(unsigned int ms);
void measure_pwm(unsigned int* duty_cycle, unsigned int* period);
void Send_back(void);
void setup(void);
void setdown(void);

MAIN_RETURN main(void)
{
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);

    USBDeviceInit();
    USBDeviceAttach();
    init();
    //setup();

    while(1)
    {
        SYSTEM_Tasks();

        #if defined(USB_POLLING)
            // Interrupt or polling method.  If using polling, must call
            // this function periodically.  This function will take care
            // of processing and responding to SETUP transactions
            // (such as during the enumeration process when you first
            // plug in).  USB hosts require that USB devices should accept
            // and process SETUP packets in a timely fashion.  Therefore,
            // when using polling, this function should be called
            // regularly (such as once every 1.8ms or faster** [see
            // inline code comments in usb_device.c for explanation when
            // "or faster" applies])  In most cases, the USBDeviceTasks()
            // function does not take very long to execute (ex: <100
            // instruction cycles) before it returns.
            USBDeviceTasks();
        #endif

        //Application specific tasks
        //APP_DeviceCDCBasicDemoTasks();
            
            Send_back();
            //USBDeviceTasks();
            __delay_ms(500);

    }//end while
}//end main
void Send_back(void) {
    
    if (USBGetDeviceState() < CONFIGURED_STATE) {
        return;
    }

    if (USBIsDeviceSuspended() == true) {
        return;
    }

     if (USBUSARTIsTxTrfReady() == true) {
        unsigned short int time;
        uint8_t numBytesRead;
        char* command,*f;
        
        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
        
        if (numBytesRead > 0) {
            // Null-terminate the read buffer to treat it as a string
           command = strtok(readBuffer, " ");
           f = strtok(NULL, " ");
            
            command[strlen(command)] = '\0';

            // Check if the received command is "Battery_LEVEL"
            if (strcmp(command, "BL") == 0) {
                if (i == 0) {
                    __delay_ms(500);
                 strcpy((char *)writeBuffer, "OK ");
                putUSBUSART(writeBuffer, strlen((char *)writeBuffer));
                CDCTxService();
                }
                i=1;
                if (f != NULL) {
                 time=1000/atoi(f);
                }
                unsigned int level;
                 unsigned int periode;
                 measure_pwm(&level, &periode);
                 
                // Convert the battery level to a string and change the scale 
                char levelStr[10];
                level=(((level-10)*100/80)*5+1200)/10;// 90% =>100% and 10%=>10%
                sprintf(levelStr, "%d", level);

                // Prepare the write buffer with the response "OK" and battery level
                strcpy((char *)writeBuffer, levelStr);

                // Send the modified writeBuffer back to the USB USART
                putUSBUSART(writeBuffer, strlen((char *)writeBuffer));
                CDCTxService();
                custom__delay_ms(10000);
        }
               
            
            if (strcmp((char *)command, "POFF") == 0) {
                __delay_ms(500);
                strcpy((char *)writeBuffer, "OK ");
                putUSBUSART(writeBuffer, strlen((char *)writeBuffer));
                CDCTxService();
                POWER_COM=1;
                custom__delay_ms(500);
               
            }
            if (strcmp((char *)command, "TON") == 0) {
                if (f != NULL) {
                 freq_T=atoi(f);
            }
                // Calculate the Timer1 period (in clock cycles) needed for the desired frequency
                timerPeriod = (_XTAL_FREQ / (4* 128 * freq_T))*2;  // Using Prescaler 1:8 , the *2 is because timerperiode is going to be for the high time and low time not both of them together

                     // Calculate the TMR1 value to load (65536 - timerPeriod) so that the overflow of timer1 is at the time desired 
                    tmr0Value = 65536 - timerPeriod;
                __delay_ms(500);
                strcpy((char *)writeBuffer, "OK ");
                putUSBUSART(writeBuffer, strlen((char *)writeBuffer));
                CDCTxService();
                setup();
                
            }
            if (strcmp((char *)command, "TOFF") == 0) {
                __delay_ms(800);
                strcpy((char *)writeBuffer, "OK ");
                putUSBUSART(writeBuffer, strlen((char *)writeBuffer));
                CDCTxService();
                setdown();
               
            }
        }
        }
    

    CDCTxService();
            __delay_ms(1000);
            }

void init(void) {
    ADCON0 = 0x00;
    TRISCbits.TRISC3 = 0; // Set RB2 as output
    TRISBbits.TRISB4 = 1; // Set RB4 as input
    TRISBbits.TRISB5 = 1; // Set RB5 as input
    TRISBbits.TRISB6 = 0; // Set RB6 as output

    ANSEL = 0;  // Set PORTA pins to digital
    ANSELH = 0; // Set PORTB pins to digital
    
    CM1CON0 = 0; // Disable comparator 1
    CM2CON0 = 0; // Disable comparator 2
    POWER_COM=0;
    
    // Configure Timer1
    T1CONbits.T1CKPS = 0b11; // Prescaler 1:8(the biggest possible for the Timer1)
    T1CONbits.TMR1CS = 0;    // Timer1 clock source is Fosc/4
    T1CONbits.TMR1ON = 1;
    T0CONbits.TMR0ON = 0;
    INTCONbits.TMR0IE = 0;
        // Enable Timer1
    
   
}

void measure_pwm(unsigned int* duty_cycle, unsigned int* period) {
    unsigned long high_time = 0;
    unsigned long total_time = 0;

    // Wait for the rising edge
    while (PWM == 0);

    // Reset Timer1 and start measuring high time
    TMR1H = 0;
    TMR1L = 0;
    TMR1IF = 0;

    // Measure high time (PWM == 1)
    while (PWM == 1) {
        if (TMR1IF) {  // Timer1 overflow handling
            TMR1IF = 0;
            high_time += 65536UL;  // Add full 16-bit range for each overflow
        }
    }
    high_time += ((unsigned long)TMR1H << 8) | TMR1L;  // Add the current timer value

    // Measure the full period
    TMR1H = 0;
    TMR1L = 0;
    TMR1IF = 0;

    // Wait for falling edge (PWM == 0)
    while (PWM == 0){
        if (TMR1IF) {  // Timer1 overflow handling
            TMR1IF = 0;
            total_time += 65536UL;  // Add full 16-bit range for each overflow
        }
    }  // This will exit as soon as PWM goes high again

    // Measure total period (from rising edge to the next rising edge)
    while (PWM == 1) {
        if (TMR1IF) {  // Timer1 overflow handling
            TMR1IF = 0;
            total_time += 65536UL;  // Add full 16-bit range for each overflow
        }
    }
    total_time += ((unsigned long)TMR1H << 8) | TMR1L;  // Add the current timer value

    // Calculate period and duty cycle
    *period = total_time;
    if (total_time != 0) {
        *duty_cycle = high_time*100/total_time;  // Calculate duty cycle as percentage
    } else {
        *duty_cycle = 0;  // Handle the case of period being zero
    }
}




void setup(void) {
   

    // Set all ANSEL and ANSELH bits to zero for digital functionality
    ANSEL = 0x00;
    ANSELH = 0x00;

    // Set all TRISx bits to zero for output direction
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    
    TRISBbits.TRISB4 = 1; // Set RB4 as input
    TRISCbits.TRISC3 = 0; // Set RC3 as output
    LATCbits.LATC3 = 0;   // Ensure RC3 is initially low

    // Enable interrupt-on-change for port B
    IOCBbits.IOCB4 = 1;   // Enable interrupt-on-change for RB4
    INTCONbits.RABIE = 1;  // Enable RB port change interrupt
    INTCONbits.PEIE = 1;  // Enable peripheral interrupts
    INTCONbits.GIE = 1;   // Enable global interrupts
    TRIGGER=0;
}

void custom__delay_ms(unsigned int ms) { // ms means milliseconds
    while (ms-- >0) {
        __delay_ms(1);
    }
}

void setdown(void) {
        
    IOCBbits.IOCB4 = 0;   // Enable interrupt-on-change for RB4
    INTCONbits.RABIE = 0;  // Enable RB port change interrupt
     INTCONbits.TMR0IF = 0;     // Clear Timer1 overflow flag
    INTCONbits.TMR0IE = 0;
     T0CONbits.TMR0ON = 0; 
    //INTCONbits.PEIE = 0;  // Enable peripheral interrupts
    //INTCONbits.GIE = 0; 
    TRIGGER=0;
}


/*******************************************************************************
 End of File
*/
