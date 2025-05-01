#include "yario.h"

YarioBuff yarioBuff;

// Accessors for the yario buffers
u_long YarioGetLocalBuff() { return yarioBuff.localBufferIO; }
u_long YarioGetRemoteBuff() { return yarioBuff.RemoteBufferIO; }

static int InitSerial(void) {
    int tty = open("tty:", 2); 				// O_RDWR
    if (tty < 0) return -1;

	ioctl(tty, TIOCRAW, 1);                 // Disable XON/XOFF
    ioctl(tty, TIOCBAUD, 115200 );          // Set baud rate
    ioctl(tty, TIOCLEN, (1 << 16) | 3);     // 8N1
    ioctl(tty, TIOCPARITY, 0);              // No parity
    ioctl(tty, FIOCNBLOCK, 1);              // Non-blocking
    ioctl(tty, TIOCFLUSH, 1);               // Flush
    ioctl(tty, TIOERRRST, 1);               // Reset errors
	
    return tty;
}

// Send 8-byte packet: 0xAA + 2 bytes pad data + 4 padding bytes + XOR checksum
static void SendData(int tty, u_long outBuff) {
    unsigned char packet[8];
    packet[0] = 0xAA; 																		// Start byte
    packet[1] = (outBuff >> 8) & 0xFF;  													// Pad high byte (bits 8–15)
    packet[2] = outBuff & 0xFF;         													// Pad low byte (bits 0–7)
    packet[3] = 0;                      													// Padding byte
    packet[4] = 0;                      													// Padding byte
    packet[5] = 0;                      													// Padding byte
    packet[6] = 0;                      													// Padding byte
    packet[7] = packet[1] ^ packet[2] ^ packet[3] ^ packet[4] ^ packet[5] ^ packet[6]; 		// XOR checksum
    write(tty, packet, 8);
}

// Receive 8-byte packet
static int ReceiveData(int tty, u_long* inBuff) {
    static unsigned char buf[8];
    static int index = 0;
    unsigned char temp[8];
    int bytesRead, i;
    unsigned char check, byte;

    // Try to read up to 8 bytes at once
    bytesRead = read(tty, temp, 8 - index);
	
    if (bytesRead <= 0) return 0;

    // Process each byte
    for (i = 0; i < bytesRead; i++) {
        byte = temp[i];

        // Check for start byte
        if (index == 0 && byte != 0xAA) continue;

        buf[index++] = byte;

        // Process when the full 8-byte packet is received
        if (index == 8) {
			
			// Perform XOR
            check = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5] ^ buf[6];
            
			// If the XOR has passed, put the data in the buffer
			if (buf[7] == check) {
                *inBuff = ((u_long)buf[1] << 8) | (u_long)buf[2];
                index = 0;
                return 1;
            }
            index = 0; // Reset on invalid packet
        }
    }
    return 0;
}

void YarioInit(void) {
    memset(&yarioBuff, 0, sizeof(YarioBuff));
    yarioBuff.ttyFD = InitSerial();
    yarioBuff.localBufferIO = 0xFFFFFFFF;
    yarioBuff.RemoteBufferIO = 0xFFFFFFFF;
}

void YarioUpdate(u_long outBuff) {
	// Default to last state
    u_long latestRemoteBuff = yarioBuff.RemoteBufferIO;
    int receivedSomething = 0;

    if (yarioBuff.ttyFD >= 0) {
        // Read the data
        while (ReceiveData(yarioBuff.ttyFD, &latestRemoteBuff)) {
            receivedSomething = 1;
        }

        // Send the data
		if (outBuff != 0) {
            SendData(yarioBuff.ttyFD, outBuff);
            yarioBuff.localBufferIO = outBuff;
        }

		// If we recieved data, store it in the buffer
        if (receivedSomething) {
            yarioBuff.RemoteBufferIO = latestRemoteBuff;
        } else {
            yarioBuff.RemoteBufferIO = 0;
        }
    }
}

void YarioClose(void) {
    if (yarioBuff.ttyFD >= 0) {
        close(yarioBuff.ttyFD);
    }
}
