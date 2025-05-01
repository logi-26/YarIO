#include <libps.h>
#include "pad.h"

volatile u_char *bb0 = NULL;
volatile u_char *bb1 = NULL;
u_char PAD1lh, PAD1lv, PAD1rh, PAD1rv;
u_char PAD2lh, PAD2lv, PAD2rh, PAD2rv;

// Initialise the pad buffer
void InitPad (void) {
	GetPadBuf(&bb0, &bb1);
}

// Read the pad buffer data
u_long PadRead(void) {
	
	// Get port 1 analog stick positions
	PAD1rh = *(bb0+4);
	PAD1rv = *(bb0+5);
	PAD1lh = *(bb0+6);
	PAD1lv = *(bb0+7);
	
	// Get port 2 analog stick positions
	PAD2rh = *(bb1+4);
	PAD2rv = *(bb1+5);
	PAD2lh = *(bb1+6);
	PAD2lv = *(bb1+7);
	
	// Return the pad buffer
	return(~(*(bb0+3) | *(bb0+2) << 8 | *(bb1+3) << 16 | *(bb1+2) << 24));
}

// Read only port 1 digital controller button states (2 bytes, bits 0–15)
u_long PadReadPort1Digital(void) {
    return (~(*(bb0+3) | *(bb0+2) << 8)) & 0xFFFF;
}