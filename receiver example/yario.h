#ifndef YARIO_H
#define YARIO_H

#include <libps.h>
#include <sys/ioctl.h>

typedef struct {
    int ttyFD;						// TTY file descriptor
    u_long RemoteBufferIO;  		// IO buffer from the remote console
	unsigned char dataBuffer[60];  	// Store 10 packets of 6 bytes
    int packetCount;               	// How many full packets have been stored
    int dataReady;                 	// Flag to indicate full 60 bytes is ready
} YarioBuff;

/************* FUNCTION PROTOTYPES *******************/
void YarioInit(void);
void YarioUpdate(void);
void YarioClose(void);
u_long YarioGetRemoteBuff(void);

const unsigned char* YarioGetDataBuffer(void); 
int YarioIsDataReady(void);
void YarioClearDataReady(void);
int YarioGetPacketCount(void); 
/*****************************************************/

#endif // YARIO_H