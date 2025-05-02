#include "yario.h"

YarioBuff yarioBuff;

// Yario accessors
u_long YarioGetRemoteBuff() { return yarioBuff.RemoteBufferIO; }
const unsigned char* YarioGetDataBuffer(void) { return yarioBuff.dataBuffer; }
int YarioIsDataReady(void) { return yarioBuff.dataReady; }
void YarioClearDataReady(void) { yarioBuff.dataReady = 0; }
int YarioGetPacketCount(void) {return yarioBuff.packetCount;}


static int InitSerial(void) {
    int tty = open("tty:", 2);  			// O_RDWR
    if (tty < 0) return -1;

    ioctl(tty, TIOCRAW, 1);               	// Disable XON/XOFF
    ioctl(tty, TIOCBAUD, 115200);         	// Set baud rate
    ioctl(tty, TIOCLEN, (1 << 16) | 3);   	// 8N1
    ioctl(tty, TIOCPARITY, 0);            	// No parity
    ioctl(tty, FIOCNBLOCK, 1);            	// Non-blocking
    ioctl(tty, TIOCFLUSH, 1);             	// Flush
    ioctl(tty, TIOERRRST, 1);             	// Reset errors

    return tty;
}

// Receive 8-byte packet
static int ReceiveData(int tty, unsigned char* outUserData) {
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
            check = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5] ^ buf[6];

            if (buf[7] == check) {
				
                // Copy only the 6 user bytes to the data buffer (ignore start-byte and XOR)
                memcpy(outUserData, &buf[1], 6);
                index = 0;
                return 1;
            }

            index = 0;  // Reset on invalid packet
        }
    }
    return 0;
}

void YarioInit(void) {
    memset(&yarioBuff, 0, sizeof(YarioBuff));
    yarioBuff.ttyFD = InitSerial();
    yarioBuff.RemoteBufferIO = 0xFFFFFFFF;
	yarioBuff.packetCount = 0;               	
    yarioBuff.dataReady = 0;  
}

void YarioUpdate(void) {
    unsigned char userData[6];
    int i, receivedSomething = 0;

    if (yarioBuff.ttyFD >= 0) {
		
		// Read available data
        while (ReceiveData(yarioBuff.ttyFD, userData)) {
            receivedSomething = 1;

            if (yarioBuff.packetCount < 10) {
                
				// Add the latest packet to the data buffer
				for (i = 0; i < 6; i++) {
                    yarioBuff.dataBuffer[yarioBuff.packetCount * 6 + i] = userData[i];
                }
				
				// Increment the packet count for displaying in the UI
                yarioBuff.packetCount++;

				// Once we have all 60-bytes we set the data-ready flag
                if (yarioBuff.packetCount == 10) {
                    yarioBuff.dataReady = 1;
                    yarioBuff.packetCount = 0;
                }
            } else {
				// Safety reset
                yarioBuff.packetCount = 0;  
            }
        }

        if (!receivedSomething) {
            yarioBuff.RemoteBufferIO = 0;
        }
    }
}

void YarioClose(void) {
    if (yarioBuff.ttyFD >= 0) {
        close(yarioBuff.ttyFD);
    }
}
