import serial
import time

ser = serial.Serial(
    port='/dev/ttyUSB0',  
    baudrate=115200,    
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

is_on = False
val = 19000
import time
if ser.isOpen():
    time.sleep(2)
    print("Serial port is open.")
    try:
        while True:
            now_ms = int(time.time() * 1000)

            ser.write(str(val).encode())
            ser.write(b'\n')
            time.sleep(0.05)

            # If received data is available
            if ser.in_waiting > 0:
                # Read data from serial port
                data = ser.read(ser.in_waiting)
                # Parse into string and print
                print(data)

            print(val)
            val += int((time.time() * 1000) - now_ms)

            if val > 21000:
                val = 19000

            # If enter is pressed, then reset val to 0
            # Not blocking!

            
    except KeyboardInterrupt:
        print("Exiting program.")
    finally:
        # Close serial port
        ser.close()
        print("Serial port closed.")
else:
    print("Failed to open serial port.")