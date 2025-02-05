import serial
import time

ser = serial.Serial(
    port='/dev/ttyACM0',  
    baudrate=921600,    
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

is_on = False
num_leds = 654
import time
if ser.isOpen():
    time.sleep(2)
    print("Serial port is open.")
    try:
        while True:
            # Data to send (90 bytes)
            
            if is_on:
                data = [50, 0, 50] * num_leds
            else:
                data = [0, 0, 0] * num_leds
            
            is_on = not is_on
            # Write data to serial port

            ser.write(data)

            # Wait for 25 milliseconds
            time.sleep(0.05)
    except KeyboardInterrupt:
        print("Exiting program.")
    finally:
        # Close serial port
        ser.close()
        print("Serial port closed.")
else:
    print("Failed to open serial port.")
