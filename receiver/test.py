import serial
import time

# Open the serial port
ser = serial.Serial(
    port='/dev/ttyACM0',      # Replace 'COM1' with your port name
    baudrate=115200,    # Set the baud rate
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

# Check if the serial port is open
is_on = False
if ser.isOpen():
    print("Serial port is open.")
    try:
        while True:
            # Data to send (90 bytes)
            
            if is_on:
                data = [255, 255, 0] * 30
            else:
                data = [0, 0, 0] * 30
            
            is_on = not is_on
            # Write data to serial port
            ser.write(data)

            # Print data to console for debugging
            print("Data sent:", data)

            # Wait for 25 milliseconds
            time.sleep(0.025)
    except KeyboardInterrupt:
        print("Exiting program.")
    finally:
        # Close serial port
        ser.close()
        print("Serial port closed.")
else:
    print("Failed to open serial port.")
