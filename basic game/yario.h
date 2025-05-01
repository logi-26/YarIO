#ifndef YARIO_H
#define YARIO_H

#include <libps.h>
#include <sys/ioctl.h>
#include "pad.h"

typedef struct {
    int ttyFD;					// TTY file descriptor
    u_long localBufferIO;		// IO buffer from the local console
    u_long RemoteBufferIO;  	// IO buffer from the remote console
} YarioBuff;

/************* FUNCTION PROTOTYPES *******************/
void YarioInit(void);
void YarioUpdate(u_long outBuff);
void YarioClose(void);
u_long YarioGetLocalBuff(void);
u_long YarioGetRemoteBuff(void);
/*****************************************************/

#endif // YARIO_H