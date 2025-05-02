import serial
from time import sleep
from itertools import cycle

# List of 10 6-letter words
WORDS = [
    "planet",
    "rocket",
    "module",
    "galaxy",
    "signal",
    "launch",
    "comets",
    "apollo",
    "meteor",
    "binary"
]

# Safety check — trim or pad to 6 bytes
WORDS = [word[:6].ljust(6) for word in WORDS]

# Create the 8-byte packet in a format that YarIO is expecting
def create_packet(word):
    if len(word) != 6:
        raise ValueError("Word must be exactly 6 characters")

    # Encode to ascii
    user_bytes = bytearray(word.encode("ascii"))
    
    # Create the XOR checksum byte
    xor_byte = 0
    for b in user_bytes:
        xor_byte ^= b

    # Return the created packet
    packet = bytearray([0xAA]) + user_bytes + bytearray([xor_byte])
    return packet

# Send the 8-byte packet using COM
def send_packets(com_port="COM3", baudrate=115200):
    
    # Set the baud-rate to match YarIO
    try:
        ser = serial.Serial(com_port, baudrate=baudrate, timeout=1)
        print(f"Connected to {com_port} at {baudrate} baud.")
    except serial.SerialException as e:
        print(f"Failed to open {com_port}: {e}")
        return

    word_cycle = cycle(WORDS)

    # Send the packets repeatedly with a short delay
    try:
        while True:
            word = next(word_cycle)
            packet = create_packet(word)
            ser.write(packet)
            print(f"Sent: {word} -> {packet.hex()}")
            
            # Sleep between packets
            sleep(0.3)  
    except KeyboardInterrupt:
        print("\nTransmission stopped.")
    finally:
        ser.close()
        print("Serial port closed.")

# Main
if __name__ == "__main__":
    send_packets("COM3") 
