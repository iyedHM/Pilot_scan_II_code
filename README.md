# Pilot Scan II Code

## Usage Manual

### 1. Sync Board

#### a. Hardware Modifications

For the Sync Board, all components were initially soldered onto the circuit. However, during programming of the PIC through the PICKIT connector (J2), the following error was encountered: 

*Target Device ID (0x0) is an Invalid Device ID. Please check your connections to the Target Device.*

This error message provides limited information. To troubleshoot this issue, the following steps can be taken:

1. **Check PIC Version Compatibility**: Ensure that the version of the PIC in MPLAB is compatible with the PIC being used.
2. **Verify Connections**: Check that all connections are secure and that the 5V/3.3V power supplied to the PIC at VDD is stable.
3. **Inspect Circuit Components**: If all connections appear correct, consider that a capacitor or another component in the circuit might be affecting the signal to the PGD, PGC, or MCLR. 

In this instance, after verifying the power supply, the Supervisory Circuits Microprocessor Reset (CI2) and resistor R4 were removed to determine if MCLR was causing the problem. The error persisted, so capacitors C20, C21, and C22, which might have been interfering with the PGC and PGD signals, were removed. Following these modifications, the microchip was successfully programmed. The IC4 was then resoldered, although the reset button remains non-functional at this stage.

#### b. Code

The code consists of two components:

1. **Custom Code**: Developed specifically for this project.
2. **USB Connection Firmware**: Provided by the Microchip Libraries for Applications (MLA), available [here](https://www.microchip.com/en-us/tools-resources/develop/libraries/microchip-libraries-for-applications).

To ensure compatibility, the correct firmware that matches the specific PIC must be used. **Note:** Do not use firmware from a different family of PICs; it must be specifically compatible.

Once the appropriate code has been located, it should be executed and its functionality verified. If it does not work, double-check that all configuration bits are correctly set, as different PICs within the same family may have varying configurations.

After establishing a connection with the PC (indicated by no error message on the PC and verifying the port with "usbipd list" in the Windows command terminal), you can code in the `main.c` file located in the "Source" directory.

**Interrupts**: When using interrupts with this USB configuration, **do not declare interrupts in the main file**. Doing so will interfere with system interrupts (which can be found in `system.c`). Instead, integrate any custom interrupt within the system interrupt to ensure it is called correctly. An example can be found in the provided `system.c` file. Be cautious with using Global Interrupt Enable (GEI) and other interrupt flags and timers to avoid conflicts with system interrupts.

**Note:** the configuration bit LVP should be set to OFF or it would result in an non predectible stats for the RC3

### 2. Power Board

#### a. Hardware Modifications

For the Power Board, several modifications were made. Initially, the same error encountered with the Sync Board was experienced. Following the earlier troubleshooting steps, it was discovered that the issue was with the Supervisory Circuits Microprocessor Reset (CI3). To resolve this, CI3 was removed, and a short circuit was created using resistor R12 and some soldering material. Additionally, the button (S1) was removed. After these changes, the board was successfully programmed.

During the programming process, an attempt was made to perform an ADC (Analog-to-Digital Converter) reading, but it was not possible because the PIC16F54 does not have an ADC module. A microcontroller with the same size and pin layout, the PIC16F186, was then found and used. With this new microcontroller, ADC readings were successfully performed, and the program was completed .

While testing the board, it was found after a number of tests that there was no signal going to the connector J8. The issue was identified as a faulty Digital Isolator (IC4). The reason for its failure remains unknown, but during the investigation, it was discovered that resistor R14 was causing a problem by dividing the voltage received from the J8 connector (expected to be 5V) to 2.5V. This resistor was removed, and a new IC4 was resoldered.

Resistors R8 and R9 were not soldered because ...

**Note:** The switch (S2) is crucial for the proper functioning of the circuit; without it, programming will not be possible.

**Note:** This boards has two different ground GND(the ground shared with the sync layer) and GND_BATT(refers to the ground of the battery)

#### b. Code

In this code, TMR1 was used because it is a 16-bit timer with a prescaler of 1:8, capable of producing PWM (Pulse Width Modulation) at the required frequency.

When using the ADC, remember to divide the reading by 1023, as it is a 10-bit reading. Then, multiply the result by the VDD (or Vref), which is set to 5V in this case.

A test was conducted where, if the voltage of one of the battery cells drops below 3V, a corresponding LED will blink (e.g., CELL1 → LED1, CELL2 → LED2, CELL3 → LED3, CELL4 → LED4).

The voltage range being worked with is from 12V to 17V. To calculate the total voltage read, 12 is subtracted, the result is multiplied by 100, and then divided by 5 (which is the difference between 17 and 12). This gives the percentage (out of 100), which is used to light up the LEDs based on specific thresholds:

- **Below 25%**: The alert LED is illuminated.
- **25% to 50%**: Only the first LED (LED1) is illuminated.
- **50% to 75%**: Two LEDs are illuminated (LED1, LED2).
- **75% to 90%**: Three LEDs are illuminated (LED1, LED2, LED3).
- **90% to 100%**: All four LEDs are illuminated (LED1, LED2, LED3, LED4).


### 3. Sync Board + Power Board

the two boards are connected by the connector J8 from the power board and J1 from the sync board ; it should be at a height of 18 mm(with +2mm of uncertainty)

**Note:** The 8000 ticks chosen were not random; they should be compatible with the PIC that is reading it (in the Sync Board). It should not be too fast to avoid a high error rate, and it should not be too slow to prevent loss of precision due to multiple (thousands) overflows of the timer used to decode the PWM. The suitable frequency for the implementation should be determined based on the timer used, considering how many times the reception will overflow, allowing for proper setting of the sender frequency.

### what to read next 
-**`README_main.md`**: the `main.c` explained more in depth 


-**`README_Pilot_scan_V2.md`**: the `Pilot_scan_V2.c` explained in depth


-**`README_system.md`**: the `system.c` explained more in depth


-**`README_test.md`**: the `test.c` explained more in depth




