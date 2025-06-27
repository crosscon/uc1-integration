#!/usr/bin/python3
import serial
import time
import random
import string

# Serial port configuration
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200

def generate_random_string(length=60):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

def test_uart_echo():
    print("Starting test")
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            while True:
                message = generate_random_string()
                expected_response = f"UART1: {message}"

                ser.write((message + '\n').encode())
                response = ser.readline().decode().strip()

                if response != expected_response:
                    print(f"Expected '{expected_response}' but got '{response}'")
                else:
                    print(f'Got {response}')

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nStopped by user.")

if __name__ == "__main__":
    test_uart_echo()
