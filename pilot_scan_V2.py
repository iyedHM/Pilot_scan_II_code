import serial
import time
import sys
import re
import threading
from getpass import getpass

PORT = "/dev/ttyACM0"
BAUDRATE = 9600
TIMEOUT = 2

# Event to control logging
log_event = threading.Event()
# Lock to prevent simultaneous serial access
serial_lock = threading.Lock()

def send_command(command):
    with serial_lock:  # Ensure only one thread accesses the serial port at a time
        with serial.Serial(port=PORT, baudrate=BAUDRATE, bytesize=8, timeout=TIMEOUT, stopbits=serial.STOPBITS_ONE) as ser:
            ser.write(command.encode("Ascii"))  # Send the command to the microcontroller
            time.sleep(1) 
            response = ser.read_all().decode("Ascii").strip()  # Read and decode the response
            return response

def log_battery(freq):
    command_to_send = f"BL {freq}"
    while log_event.is_set():
        response = send_command(command_to_send)
        if response :
            if response=="OK" :                
                print(f"BATT: {response}")
            elif 120<int(response)<170 :
                response=response[:-1]+"."+response[-1]
                print(f"BATT: {response}")
            
        
    time.sleep(1/freq)  # Adjust sleep time based on frequency

def main():
    global log_event
    log_thread = None
    logging_freq = 1  # Default frequency for battery logging

    while True:
        command = getpass("").upper().strip()

        if re.match(r"EXIT", command):
            print("Fermeture du programme.")
            if log_thread and log_thread.is_alive():
                log_event.clear()  # Stop logging
                log_thread.join()  # Wait for logging thread to finish
            break
        elif re.match(r".*HELP.*", command):
            print("""
            The valid commands are:
            - POWER OFF: Turns off the device.
            - TRIGGER ON [FREQ(Hz)]: Activates the trigger at a specified frequency (defaults to 10Hz if not specified).
            - TRIGGER OFF: Deactivates the trigger.
            - LOG BATTERY [FREQ(Hz)]: Displays the battery level at a specified frequency (defaults to 1Hz if not specified).
            - EXIT: Closes the program.
            """)
        elif re.match(r"TRIGGER ON *[0-9]*$", command):
            if log_event.is_set():
                log_event.clear()  # Pause logging
                log_thread.join()  # Wait for logging thread to finish
            try:
                _, _, FreqT = command.split(" ")
            except ValueError:
                command_to_send = "TON 10 "
                print("default frequency 10hz")
            else:
                print(f" You have chosen {FreqT} Hz")
                command_to_send = f"TON {FreqT} "
            response = send_command(command_to_send)
            k=0
            with serial.Serial(port=PORT, baudrate=BAUDRATE, bytesize=8, timeout=TIMEOUT, stopbits=serial.STOPBITS_ONE) as ser:
                while(k==0):
                    if re.search(r".*OK.*",response) :
                        print(f"TRIGGER_ON: OK ")
                        k=1
                    response = ser.read_all().decode("Ascii").strip()
                    
            time.sleep(1)  # Sleep for 1 second
            if log_thread and not log_event.is_set():
                log_event.set()  # Resume logging
                log_thread = threading.Thread(target=log_battery, args=(logging_freq,))
                log_thread.start()
        elif re.match(r"TRIGGER OFF$", command):
            if log_event.is_set():
                log_event.clear()  # Pause logging
                log_thread.join()  # Wait for logging thread to finish
            time.sleep(1)
            command_to_send = "TOFF "
            response = send_command(command_to_send)
            k=0
            with serial.Serial(port=PORT, baudrate=BAUDRATE, bytesize=8, timeout=TIMEOUT, stopbits=serial.STOPBITS_ONE) as ser:
                while(k==0):
                    if re.search(r".*OK.*",response) :
                        print(f"TRIGGER_OFF: OK ")
                        k=1
                    response = ser.read_all().decode("Ascii").strip()
                    
                        
            time.sleep(1)  # Sleep for 1 second
            if log_thread and not log_event.is_set():
                log_event.set()  # Resume logging
                log_thread = threading.Thread(target=log_battery, args=(logging_freq,))
                log_thread.start()
        elif re.match(r"POWER OFF$", command):
            if log_event.is_set():
                log_event.clear()  # Pause logging
                log_thread.join()  # Wait for logging thread to finish
            command_to_send = "POFF "
            response = send_command(command_to_send)
            k=0
            with serial.Serial(port=PORT, baudrate=BAUDRATE, bytesize=8, timeout=TIMEOUT, stopbits=serial.STOPBITS_ONE) as ser:
                while(k==0):
                    if re.search(r".*OK.*",response) :
                        print(f"POWER_OFF: {response}")
                        k=1
                    response = ser.read_all().decode("Ascii").strip()
            
            time.sleep(1)  # Sleep for 1 second
            if log_thread and not log_event.is_set():
                log_event.set()  # Resume logging
                log_thread = threading.Thread(target=log_battery, args=(logging_freq,))
                log_thread.start()
        elif re.match(r"LOG BATTERY *[0-9]*$", command):
            try:
                _, _, FreqB = command.split(" ")
            except ValueError:
                logging_freq = 10  # Default frequency
            else:
                logging_freq = int(FreqB)
            if log_thread and log_thread.is_alive():
                log_event.clear()  # Stop current logging
                log_thread.join()  # Wait for the logging thread to finish
            time.sleep(1)  # Sleep for 1 second
            log_event.set()  # Start new logging
            log_thread = threading.Thread(target=log_battery, args=(logging_freq,))
            log_thread.start()
        else:
            print("Invalid command. If you need assistance, write 'help'.")
            continue

if __name__ == "__main__":
    main()
