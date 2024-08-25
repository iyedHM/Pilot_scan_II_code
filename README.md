# Pilot Scan II Code

## Usage Manual

### 1. Sync Board

#### a. Hardware Modifications

For the Sync Board, I began by soldering all components onto the circuit. However, when attempting to program the PIC through the PICKIT connector (J2), I encountered the following error: 

*Target Device ID (0x0) is an Invalid Device ID. Please check your connections to the Target Device.*

This error message doesn't provide much information, but if you encounter it, here are some steps to troubleshoot:

1. **Check PIC Version Compatibility**: Verify that the version of the PIC in MPLAB is compatible with the PIC you are using.
2. **Verify Connections**: Ensure all connections are secure and that the 5V/3.3V power supplied to the PIC at VDD is stable.
3. **Inspect Circuit Components**: If everything appears correct, a capacitor or another component in the circuit might be affecting the signal to the PGD, PGC, or MCLR. 

In our case, after checking the power supply, I removed the Supervisory Circuits Microprocessor Reset (CI2) and the resistor R4 to determine if MCLR was the source of the problem. The same error persisted. Next, I removed capacitors C20, C21, and C22, which might have been interfering with the PGC and PGD signals. After these modifications, I successfully programmed the microchip. I then resoldered the IC4, although the reset button is still non-functional at this point.

#### b. Code

The code has two components: 

1. **Custom Code**: The code I developed specifically for this project.
2. **USB Connection Firmware**: This is provided by the Microchip Libraries for Applications (MLA), available [here](https://www.microchip.com/en-us/tools-resources/develop/libraries/microchip-libraries-for-applications).

To ensure compatibility, find the correct firmware that matches your PIC. **Note:** Do not attempt to use firmware from a different family of PICs; it must be specifically compatible.

Once you've found the right code, execute it and verify its functionality. If it does not work, double-check that all configuration bits are set correctly, as different PICs within the same family may have varying configurations.

After establishing a connection with your PC (no error message on the PC, and using "usbipd list" in the command terminal on Windows to see the port), you can start coding in the `main.c` file located in the "Source" directory.

**Interrupts**: If you need to use interrupts with this USB configuration, **do not declare interrupts in the main file**. Doing so will interfere with system interrupts (which can be found in `system.c`). Instead, integrate your interrupt within the system interrupt and ensure it is called correctly. You can refer to the example in the provided `system.c` file. Be mindful of using Global Interrupt Enable (GEI) and other interrupt flags and timers to avoid conflicts with system interrupts.

### 2. Power Board

#### a. Hardware Modifications

For the Power Board, I also made some modifications. Initially, I tried to program it and encountered the same error as with the Sync Board. So, I followed the troubleshooting steps specified above. This time, the Supervisory Circuits Microprocessor Reset (CI3) was the source of the problem. I removed it and created a short circuit using the resistor R12 and some soldering material. Additionally, we removed the button (S1). After these changes, I was able to program the board successfully.

While programming, I attempted to perform an ADC reading but was unable to do so because the PIC16F54 does not have an ADC module. Fortunately, we found a PIC that has the same size and pin layout as the PIC16F54, which was the PIC16F186. With this new microcontroller, I was able to perform ADC readings and complete the program without any issues.

#### b. Code

In this code, I used TMR1 because it is a 16-bit timer with a prescaler of 1:8, which can produce a PWM at any required frequency.

don't forget when usinf the ADC to use the /1023 given you are reading by 10 bits and the multiply the VDD(or Vref) that you have set 5v for our case 

i made a test were if one of the cells of the battery is less the 3V we would see a blinking led for the respective cell (CELL1 --> LED1 , CELL2-->LED2 ,CELL3 --> LED3 , CELL4-->LED4)

The voltage interval that i'm working on is 12 v to 17 v so use the total voltage read i substract 12 and then multiply it by a 100 and then devide it by 5(17-12) , So i found the percentage /100 i use it to light up the led with specific threshold for our case we have less 25% the alert led is shining , between 25%  and 50% ony the first led (LED1) si shining then between 50% and 75 %  two led are shining and from 75% to 90% Three are lighted up and then between 90% and 100% the four leds are shininig 



### 3. Sync Board + Power Board

