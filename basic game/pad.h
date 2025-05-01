#ifndef PAD_H
#define PAD_H

#include <libps.h> 

// Low-level pad buffers
extern volatile u_char *bb0, *bb1;

// Analog axis for each controller
extern u_char PAD1lh, PAD1lv, PAD1rh, PAD1rv;                  
extern u_char PAD2lh, PAD2lv, PAD2rh, PAD2rv;

/************* FUNCTION PROTOTYPES *******************/
void InitPad(void);
u_long PadRead(void);
u_long PadReadPort1Digital(void);
/*****************************************************/

// Definitions for controller port 1 buttons
#define PAD1up       	(1<<12)
#define PAD1down     	(1<<14)
#define PAD1left     	(1<<15)
#define PAD1right    	(1<<13)
#define PAD1triangle 	(1<< 4)
#define PAD1cross    	(1<< 6)
#define PAD1square   	(1<< 7)
#define PAD1circle   	(1<< 5)
#define PAD1select   	(1<< 8)
#define PAD1start    	(1<<11)
#define PAD1R1       	(1<< 3)
#define PAD1R2       	(1<< 1)
#define PAD1L1       	(1<< 2)
#define PAD1L2       	(1<< 0)
#define PAD1L3       	(1<< 9)
#define PAD1R3       	(1<<10)

#endif // PAD_H