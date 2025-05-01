#include <libps.h>
#include <sys/ioctl.h>
#include "yario.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define PLAYER_SIZE 20

GsBOXF p1;
GsBOXF p2;

// Graphics stuff
int fntID;
GsOT worldOT[2];
GsOT_TAG worldOTTags[2][1 << 9];
PACKET packetArea[2][24 * 100];

void InitGfx(void) {
    ResetGraph(0);
    SetVideoMode(MODE_PAL);
    GsInitGraph(SCREEN_W, SCREEN_H, GsNONINTER | GsOFSGPU, 1, 0);
    GsDefDispBuff(0, 0, 0, SCREEN_H);

    worldOT[0].length = 9;
    worldOT[0].org = worldOTTags[0];
    worldOT[1].length = 9;
    worldOT[1].org = worldOTTags[1];

    FntLoad(960, 256);
    fntID = FntOpen(10, 10, 300, 200, 0, 512);
}

int GraphicsStartFrame(void) {
	int activeBuffer = GsGetActiveBuff();;
	GsSetWorkBase((PACKET*)packetArea[activeBuffer]);
    GsClearOt(0, 0, &worldOT[activeBuffer]);
	return activeBuffer;
}

void GraphicsEndFrame(int activeBuffer) {
	FntFlush(fntID);
	VSync(0);
	GsSwapDispBuff();
	GsSortClear(0, 0, 0, &worldOT[activeBuffer]);
	GsDrawOt(&worldOT[activeBuffer]);
}

void InitPlayers(void) {
    p1.x = 40;
    p1.y = 140;
    p1.w = PLAYER_SIZE;
    p1.h = PLAYER_SIZE;
    p1.r = 0;
    p1.g = 255;
    p1.b = 0;
	
	p2.x = 80;
    p2.y = 140;
    p2.w = PLAYER_SIZE;
    p2.h = PLAYER_SIZE;
    p2.r = 255;
    p2.g = 0;
    p2.b = 0;
}

void UpdatePlayer1(u_long padBuff) {
	if (padBuff & PAD1up) p1.y --;
	if (padBuff & PAD1down) p1.y ++;
	if (padBuff & PAD1left) p1.x --;
	if (padBuff & PAD1right) p1.x ++;
}

void UpdatePlayer2(u_long padBuff) {
	if (padBuff & PAD1up) p2.y --;
	if (padBuff & PAD1down) p2.y ++;
	if (padBuff & PAD1left) p2.x --;
	if (padBuff & PAD1right) p2.x ++;
}

int PlayerSelection(void) {
	u_long padBuff;
	int playerNumber = 0;
	while (!playerNumber) {
        int activeBuffer = GraphicsStartFrame();
		
		// Display the player select info
        FntPrint(fntID, "\nSelect your player number:\n\n\n");
		FntPrint(fntID, "player 1 = Triangle\n\n");
		FntPrint(fntID, "player 2 = Cross\n");
		
		// Check for player selection
		padBuff = PadRead();
        if (padBuff & PAD1triangle) playerNumber = 1;
		else if (padBuff & PAD1cross) playerNumber = 2;

		// Update the graphics 
		GraphicsEndFrame(activeBuffer);
    }
	
	return playerNumber;
}

int main(void) {
	int activeBuffer, frame = 0, playerNumber = 0;
	u_long padBuff;
	
    InitGfx();
	InitPad();
	InitPlayers();
	
	// Initialise yario (open and configure the tty connection)
    YarioInit();	
	
	// Wait for the player selection
    playerNumber = PlayerSelection();

    while (1) {
        activeBuffer = GraphicsStartFrame();

		// Display info on screen
        FntPrint(fntID, "Frame: %d\n\n", frame);
		if (playerNumber == 1) FntPrint(fntID, "You are the green square (player: %d)\n\n", playerNumber);
		else FntPrint(fntID, "You are the red square (player: %d)\n\n", playerNumber);
		FntPrint(fntID, "Player 1 X/Y: %d/%d\n", p1.x, p1.y);
		FntPrint(fntID, "Player 2 X/Y: %d/%d\n", p2.x, p2.y);
		
		// Read the pad buffer of the digital controller conected to port 1
		padBuff = PadReadPort1Digital();
		
		// Update yario (send/recieve control pad data)
		YarioUpdate(padBuff);
		
		// Update the appropriate player with the buffer
		if (playerNumber == 1) {
			UpdatePlayer1(padBuff);					// Update player 1 using the local pad buffer
			UpdatePlayer2(YarioGetRemoteBuff());	// Update player 2 using the remote pad buffer
		}
		else {
			UpdatePlayer1(YarioGetRemoteBuff());	// Update player 1 using the remote pad buffer
			UpdatePlayer2(padBuff);					// Update player 2 using the local pad buffer
		}
		
		// Draw the players
		GsSortBoxFill(&p1, &worldOT[activeBuffer], 0);
		GsSortBoxFill(&p2, &worldOT[activeBuffer], 0);
		
		// Update the graphics
        GraphicsEndFrame(activeBuffer);
		
        frame++;
    }
	
	YarioClose();

    return 0;
}