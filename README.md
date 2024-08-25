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

For the Power Board, I made several modifications. Initially, when I tried to program the board, I encountered the same error as with the Sync Board. Following the troubleshooting steps mentioned earlier, I discovered that the issue was with the Supervisory Circuits Microprocessor Reset (CI3). To resolve this, I removed CI3 and created a short circuit using the resistor R12 and some soldering material. Additionally, I removed the button (S1). After these changes, I was able to successfully program the board.

During the programming process, I attempted to perform an ADC (Analog-to-Digital Converter) reading but was unable to do so because the PIC16F54 does not have an ADC module. Fortunately, we found a microcontroller with the same size and pin layout as the PIC16F54, which was the PIC16F186. With this new microcontroller, I was able to perform ADC readings and complete the program without any issues.

#### b. Code

In this code, I used TMR1 because it is a 16-bit timer with a prescaler of 1:8, which can produce PWM (Pulse Width Modulation) at the required frequency.

When using the ADC, remember to divide the reading by 1023, since it is a 10-bit reading. Then, multiply the result by the VDD (or Vref), which is set to 5V in our case.

I conducted a test where, if the voltage of one of the battery cells drops below 3V, a corresponding LED will blink (e.g., CELL1 → LED1, CELL2 → LED2, CELL3 → LED3, CELL4 → LED4).

The voltage range I'm working with is from 12V to 17V. To calculate the total voltage read, I subtract 12, multiply by 100, and then divide by 5 (which is the difference between 17 and 12). This gives me the percentage (out of 100), which I use to light up the LEDs based on specific thresholds:

- **Below 25%**: The alert LED is illuminated.
- **25% to 50%**: Only the first LED (LED1) is illuminated.
- **50% to 75%**: Two LEDs are illuminated.
- **75% to 90%**: Three LEDs are illuminated.
- **90% to 100%**: All four LEDs are illuminated.



### 3. Sync Board + Power Board

