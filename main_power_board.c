// PIC16F54 Configuration Bit Settings
// 'C' source line config statements
// CONFIG
// PIC16F1826 Configuration Bit Settings
// 'C' source line config statements
// CONFIG1
#pragma config FOSC = XT    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF      // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config DEBUG = OFF      // In-Circuit Debugger Mode (In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.



#define _XTAL_FREQ 4000000 
#include <xc.h>

/*
 * File:   test.c
 * Author: MSI
 *
 * Created on 6 août 2024, 14:42
 */

#define PWM LATBbits.LATB4
#define POWER_OFF RB7
#define ENABLE LATBbits.LATB5


unsigned long cell_B; // in millivolts
unsigned long cell_N;
volatile unsigned int bat_level=0;
volatile unsigned int bat_level_percentage=0;
volatile unsigned long onTicks = 6000;  // ON duration in timer ticks
volatile unsigned long offTicks = 2000; // OFF duration in timer ticks
void custom__delay_ms(unsigned long ms);
void calculate_pwm_timing(unsigned int duty_cycle) ;
unsigned int ADC_Read(unsigned char channel);
void ADC_Init(void);
void Timer1_Init(void);

void main(void) {
   
    // Disable analog function on RB1 and RB2
    ANSELBbits.ANSB1 = 0;  // Disable analog on RB1
    ANSELBbits.ANSB2 = 0;  // Disable analog on RB2
    ANSELBbits.ANSB7 = 0;  // Disable digital on RB7
    RCSTAbits.SPEN = 0;    // Disable the serial port if using UART
    SSP1CON1bits.SSPEN = 0;  // Disable SSP (used for SPI/I2C)
    
    // Setting up the RA0 to RA3 as analog input 
    ANSELAbits.ANSA0 = 1;  // Enable analog on RA0
    ANSELAbits.ANSA1 = 1;  // Enable analog on RA1
    ANSELAbits.ANSA2 = 1;  // Enable analog on RA2
    ANSELAbits.ANSA3 = 1;  // Enable analog on RA3
    
    TRISAbits.TRISA0 = 1;  // Set RA0 as input
    TRISAbits.TRISA1 = 1;  // Set RA1 as input
    TRISAbits.TRISA2 = 1;  // Set RA2 as input
    TRISAbits.TRISA3 = 1;  // Set RA3 as input

    // Set all PORTB pins as outputs
    TRISB = 0x00;
    LATB = 0;  // Ensure all PORTB pins start low and enable the dc-dc converter
    TRISBbits.TRISB7 = 1; // Set RB7 as input
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5=1;
    __delay_ms(3000);
    LATBbits.LATB5=0;
    
    ADC_Init();
    Timer1_Init();
    while (1) {  
        
        // Reset and start Timer0 to measure loop time
        TMR1IF = 0;  // Clear overflow flag
        // Getting each cell's voltage
        
        cell_N=(ADC_Read(0) * 5000L) / 1023;
        if (cell_N < 3200) {
            LATBbits.LATB0 = 1;
            __delay_ms(50);
            LATBbits.LATB0 = 0;
            __delay_ms(50);
        }
   
        cell_B=cell_N;
        cell_N = (ADC_Read(1) *2 * 5000L) / 1023  - cell_B;
        if (cell_N < 3200) {
            LATBbits.LATB1 = 1;
            __delay_ms(50);
            LATBbits.LATB1 = 0;
            __delay_ms(50);
        }
        
        cell_B+=cell_N;
        cell_N = (ADC_Read(2) * 3*5000L) / 1023  - cell_B;
        if (cell_N < 3200) {
            LATBbits.LATB2 = 1;
            __delay_ms(50);
            LATBbits.LATB2 = 0;
            __delay_ms(50);
        }
        
        bat_level = ADC_Read(3) * 4*5000L/ 1023;
        cell_B=+cell_N;
        cell_N =  bat_level - cell_B;
        if (cell_N < 3200) {
            LATBbits.LATB3 = 1;
            __delay_ms(50);
            LATBbits.LATB3 = 0;
            __delay_ms(50);
        }
    
        
        // Integer-based calculation to avoid floating-point operations
      //  bat_level = ADC_Read(3)*4 *5000L/ 1023;
        if ((bat_level >12000L) && (bat_level <17000L) ){
         bat_level_percentage = ((bat_level - 12000L) * 100) /5000L ;
         
        if (bat_level_percentage > 82) { // FULL
            LATBbits.LATB0 = 1;
            LATBbits.LATB1 = 1;
            LATBbits.LATB2 = 1;
            LATBbits.LATB3 = 1;  // All LEDs ON
        } else if (bat_level_percentage > 75) {
             // Three LEDs ON
            LATBbits.LATB0 = 1;
            LATBbits.LATB1 = 1;
            LATBbits.LATB2 = 1;
            LATBbits.LATB3 = 0; 
        } else if (bat_level_percentage > 50) { // Half FULL
            // Two LEDs ON
            LATBbits.LATB0 = 1;
            LATBbits.LATB1 = 1;
            LATBbits.LATB2 = 0;
            LATBbits.LATB3 = 0;  
        } else if (bat_level_percentage > 25) { // Less than half
            LATBbits.LATB0 = 1;
            LATBbits.LATB1 = 0;
            LATBbits.LATB2 = 0;
            LATBbits.LATB3 = 0;  // One LED ON
        } else {
            // All LEDs OFF        
            LATBbits.LATB0 = 0;
            LATBbits.LATB1 = 0;
            LATBbits.LATB2 = 0;
            LATBbits.LATB3 = 0;
                
        }
        }else if (bat_level >17000L){
            // All LEDs OFF        
            LATBbits.LATB0 = 0;
            LATBbits.LATB1 = 0;
            LATBbits.LATB2 = 0;
            LATBbits.LATB3 = 1;
            bat_level_percentage=0;
            
        }else {
            LATBbits.LATB0 = 0;
            LATBbits.LATB1 = 0;
            LATBbits.LATB2 = 1;
            LATBbits.LATB3 = 1;
            bat_level_percentage=100;
            
        }
       
     onTicks = (8000L * ((bat_level_percentage*80)/100+10)) / 100;
     offTicks = 8000L - onTicks;
        // POwer Off command 
         
        if (POWER_OFF == 0) {
            __delay_ms(500);
            LATBbits.LATB5=1;
            for (int i=0;i<30;i++){
            LATBbits.LATB0 = 1;
            LATBbits.LATB1 = 0;
            LATBbits.LATB2 = 0;
            LATBbits.LATB3 = 0;
            __delay_ms(200);
            LATBbits.LATB0 = 0;
            LATBbits.LATB1 = 0;
            LATBbits.LATB2 = 0;
            LATBbits.LATB3 = 0;
            __delay_ms(200);
            }
            
            
        }
         
        
    }
    
     
    
    
}

void custom__delay_ms(unsigned long ms) {
    while (ms-- > 0) {
        __delay_ms(1);
    }
}

void ADC_Init(void) {
    ADCON1bits.ADCS = 0b101;  // Set the ADC conversion clock (Fosc/32)
    ADCON1bits.ADFM = 1;  // Right justify the result
    ADCON1bits.ADPREF = 0b00;  // Configure the reference voltages (VREF+ = Vdd, VREF- = Vss)
    ADCON0bits.ADON = 1;  // Enable the ADC module
}

unsigned int ADC_Read(unsigned char channel) {
    ADCON0bits.CHS = channel;  // Select the ADC channel
    __delay_us(15);  // Wait for acquisition time
    ADCON0bits.GO_nDONE = 1;  // Start the conversion
    while (ADCON0bits.GO_nDONE);  // Wait for the conversion to complete
    return ((ADRESH << 8) + ADRESL);  // Return the result
}
void Timer1_Init(void) {
   // Timer1 Setup
     OSCCON = 0x6A;  // Set internal oscillator to 4 MHz
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5=1;
    __delay_ms(3000);
    LATBbits.LATB5=0;
    // Port Setup
    TRISBbits.TRISB4 = 0;  // Set RB4 as output
    T1CONbits.TMR1CS = 0b00;  // Timer1 clock source: Fosc/4
    T1CONbits.T1CKPS = 0b11;  // 1:8 prescaler
    T1CONbits.TMR1ON = 1;     // Turn on Timer1

    // Initialize Timer1
    TMR1H = (unsigned char)((65536 - onTicks) >> 8); // Set the high byte of TMR1
    TMR1L = (unsigned char)(65536 - onTicks);        // Set the low byte of TMR1

    // Enable Interrupts
    PIR1bits.TMR1IF = 0;      // Clear the Timer1 interrupt flag
    PIE1bits.TMR1IE = 1;      // Enable Timer1 interrupt
    INTCONbits.PEIE = 1;      // Enable Peripheral Interrupt
    INTCONbits.GIE = 1;       // Enable Global Interrupt
}


void __interrupt() ISR() {
    if (PIR1bits.TMR1IF) {    // Check if Timer1 Overflow Interrupt occurred
        PIR1bits.TMR1IF = 0;  // Clear the interrupt flag

        if (LATBbits.LATB4) { // If RB4 is high, start the OFF period
            LATBbits.LATB4 = 0;           // Set RB4 low (End of pulse)
            TMR1H = (unsigned char)((65536 - offTicks) >> 8); // Reload Timer1 high byte
            TMR1L = (unsigned char)(65536 - offTicks);        // Reload Timer1 low byte
        } else {              // If RB4 is low, start the ON period
            LATBbits.LATB4 = 1;           // Set RB4 high (Start of pulse)
            TMR1H = (unsigned char)((65536 - onTicks) >> 8); // Reload Timer1 high byte
            TMR1L = (unsigned char)(65536 - onTicks);        // Reload Timer1 low byte
        }
    }
}
