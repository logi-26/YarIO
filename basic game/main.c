#include <libps.h>
#include <sys/ioctl.h>
#include "yario.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define MIN_X 5
#define MAX_X 315
#define MIN_Y 70
#define MAX_Y 235
#define CENTRE 160

#define PLAYER_SIZE 20

typedef struct {
    int active;
    GsBOXF rectangle;
} Bullet;

typedef struct {
    int lives;
    GsBOXF rectangle;
    Bullet* bullet;
} Player;

typedef struct {
    GsLINE left;
    GsLINE right;
    GsLINE bottom;
    GsLINE top;
	GsLINE centre;
} GameArea;

// Create 2 players
Player player1;
Player player2;

// Create a game area
GameArea gameArea;

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
    player1.rectangle.x = 40;
    player1.rectangle.y = 140;
    player1.rectangle.w = PLAYER_SIZE;
    player1.rectangle.h = PLAYER_SIZE;
    player1.rectangle.r = 0;
    player1.rectangle.g = 255;
    player1.rectangle.b = 0;
    player1.lives = 3;

    player1.bullet = (Bullet*)malloc(sizeof(Bullet));
    if (player1.bullet) {
        player1.bullet->rectangle.x = -1;
        player1.bullet->rectangle.y = -1;
        player1.bullet->rectangle.w = 5;
        player1.bullet->rectangle.h = 5;
        player1.bullet->rectangle.r = 0;
        player1.bullet->rectangle.g = 255;
        player1.bullet->rectangle.b = 0;
        player1.bullet->active = 0;
    }

    player2.rectangle.x = 260;
    player2.rectangle.y = 140;
    player2.rectangle.w = PLAYER_SIZE;
    player2.rectangle.h = PLAYER_SIZE;
    player2.rectangle.r = 255;
    player2.rectangle.g = 0;
    player2.rectangle.b = 0;
    player2.lives = 3;

    player2.bullet = (Bullet*)malloc(sizeof(Bullet));
    if (player2.bullet) {
        player2.bullet->rectangle.x = -1;
        player2.bullet->rectangle.y = -1;
        player2.bullet->rectangle.w = 5;
        player2.bullet->rectangle.h = 5;
        player2.bullet->rectangle.r = 255;
        player2.bullet->rectangle.g = 0;
        player2.bullet->rectangle.b = 0;
        player2.bullet->active = 0;
    }
}

void UpdatePlayer1(u_long padBuff){
	// Move
	if ((padBuff & PAD1up) && (player1.rectangle.y > MIN_Y +2)) player1.rectangle.y--;
	if ((padBuff & PAD1down) && (player1.rectangle.y < MAX_Y -PLAYER_SIZE)) player1.rectangle.y++;
	if ((padBuff & PAD1left) && (player1.rectangle.x > MIN_X)) player1.rectangle.x--;
	if ((padBuff & PAD1right) && (player1.rectangle.x < CENTRE -PLAYER_SIZE)) player1.rectangle.x++;
	
	// Shoot
	if ((padBuff & PAD1square) && !(player1.bullet->active == 1)) {
		player1.bullet->rectangle.x = player1.rectangle.x +PLAYER_SIZE;
        player1.bullet->rectangle.y = player1.rectangle.y +10;
		player1.bullet->active = 1;
	}
}

void UpdatePlayer2(u_long padBuff){
	// Move
	if ((padBuff & PAD1up) && (player2.rectangle.y > MIN_Y +2)) player2.rectangle.y--;
	if ((padBuff & PAD1down) && (player2.rectangle.y < MAX_Y -PLAYER_SIZE)) player2.rectangle.y++;
	if ((padBuff & PAD1left) && (player2.rectangle.x > CENTRE +2)) player2.rectangle.x--;
	if ((padBuff & PAD1right) && (player2.rectangle.x < MAX_X -PLAYER_SIZE)) player2.rectangle.x++;
	
	// Shoot
	if ((padBuff & PAD1square) && !(player2.bullet->active == 1)) {
		player2.bullet->rectangle.x = player2.rectangle.x;
        player2.bullet->rectangle.y = player2.rectangle.y + 10;
		player2.bullet->active = 1;
	}
}

void UpdateBullets(void){
	// Update the player 1 bullet
	if (player1.bullet->active == 1) {
		player1.bullet->rectangle.x+=4;
		
		// P1 bullet hit P2
		if ((player1.bullet->rectangle.x >= player2.rectangle.x) && (player1.bullet->rectangle.x <= player2.rectangle.x + 20)){
			if ((player1.bullet->rectangle.y >= player2.rectangle.y) && (player1.bullet->rectangle.y <= player2.rectangle.y + 20)){
				player1.bullet->active = 0;
				player2.lives --;
			}
		}
	}
	
	// Update the player 2 bullet
	if (player2.bullet->active == 1) {
		player2.bullet->rectangle.x-=4;
		
		// P2 bullet hit P1
		if ((player2.bullet->rectangle.x >= player1.rectangle.x) && (player2.bullet->rectangle.x <= player1.rectangle.x + 20)){
			if ((player2.bullet->rectangle.y >= player1.rectangle.y) && (player2.bullet->rectangle.y <= player1.rectangle.y + 20)){
				player2.bullet->active = 0;
				player1.lives --;
			}
		}
	}

	// De-activate the bullets if out of bounds
	if (player1.bullet->rectangle.x > 320) player1.bullet->active = 0;
	if (player2.bullet->rectangle.x < 0) player2.bullet->active = 0;
}

void DrawGameArea(int activeBuffer) {
    gameArea.left.x0 = MIN_X;
    gameArea.left.y0 = MIN_Y;
    gameArea.left.x1 = MIN_X;
    gameArea.left.y1 = MAX_Y;
    gameArea.left.r = 0;
    gameArea.left.g = 0;
    gameArea.left.b = 255;
    GsSortLine(&gameArea.left, &worldOT[activeBuffer], 0);
    
    gameArea.right.x0 = MAX_X - 1;
    gameArea.right.y0 = MIN_Y;
    gameArea.right.x1 = MAX_X - 1;
    gameArea.right.y1 = MAX_Y;
    gameArea.right.r = 0;
    gameArea.right.g = 0;
    gameArea.right.b = 255;
    GsSortLine(&gameArea.right, &worldOT[activeBuffer], 0);
    
    gameArea.bottom.x0 = MIN_X;
    gameArea.bottom.y0 = MAX_Y;
    gameArea.bottom.x1 = MAX_X - 1;
    gameArea.bottom.y1 = MAX_Y;
    gameArea.bottom.r = 0;
    gameArea.bottom.g = 0;
    gameArea.bottom.b = 255;
    GsSortLine(&gameArea.bottom, &worldOT[activeBuffer], 0);
    
    gameArea.top.x0 = MIN_X;
    gameArea.top.y0 = MIN_Y;
    gameArea.top.x1 = MAX_X - 1;
    gameArea.top.y1 = MIN_Y;
    gameArea.top.r = 0;
    gameArea.top.g = 0;
    gameArea.top.b = 255;
    GsSortLine(&gameArea.top, &worldOT[activeBuffer], 0);
	
	gameArea.centre.x0 = CENTRE;
    gameArea.centre.y0 = MIN_Y;
    gameArea.centre.x1 = CENTRE;
    gameArea.centre.y1 = MAX_Y;
    gameArea.centre.r = 0;
    gameArea.centre.g = 0;
    gameArea.centre.b = 255;
    GsSortLine(&gameArea.centre, &worldOT[activeBuffer], 1);
}

void DrawPlayers(int activeBuffer) {
	GsSortBoxFill(&player1.rectangle, &worldOT[activeBuffer], 0);
	GsSortBoxFill(&player2.rectangle, &worldOT[activeBuffer], 0);
}

void DrawBullets(int activeBuffer) {
	if (player1.bullet->active == 1) GsSortBoxFill(&player1.bullet->rectangle, &worldOT[activeBuffer], 0);
	if (player2.bullet->active == 1) GsSortBoxFill(&player2.bullet->rectangle, &worldOT[activeBuffer], 0);
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
	u_long padBuff, remoteIOBuffer;

    InitGfx();
	InitPad();
	InitPlayers();
	
	// Initialise yario (open and configure the tty connection)
    YarioInit();	
	
	// Wait for the player selection
    playerNumber = PlayerSelection();

    while (1) {
        activeBuffer = GraphicsStartFrame();
		
		if (player1.lives >0 && player2.lives >0)  {
			
			// Display info on screen
			FntPrint(fntID, "Frame: %d\n", frame);
			FntPrint(fntID, "Remote Buffer: %08x\n\n", remoteIOBuffer);
			FntPrint(fntID, "You are Player: %d\n\n", playerNumber);
			FntPrint(fntID, "Player 1  lives: %d  X/Y: %d/%d\n", player1.lives, player1.rectangle.x, player1.rectangle.y);
			FntPrint(fntID, "Player 2  lives: %d  X/Y: %d/%d\n", player2.lives, player2.rectangle.x, player2.rectangle.y);
			
			DrawGameArea(activeBuffer);
			
			// Read the pad buffer of the digital controller conected to port 1
			padBuff = PadReadPort1Digital();
			
			// Update yario (send/recieve control pad data)
			YarioUpdate(padBuff);
			
			// Get the remote control pad buffer from yario
			remoteIOBuffer = YarioGetRemoteBuff();
			
			// Update the appropriate player with the appropriate buffer
			if (playerNumber == 1) {
				UpdatePlayer1(padBuff);				// Update player 1 using the local pad buffer
				UpdatePlayer2(remoteIOBuffer);		// Update player 2 using the remote pad buffer
			}
			else {
				UpdatePlayer1(remoteIOBuffer);		// Update player 1 using the remote pad buffer
				UpdatePlayer2(padBuff);				// Update player 2 using the local pad buffer
			}
			
			// Update the players bullets
			UpdateBullets();

			// Draw the players
			DrawPlayers(activeBuffer);
			
			// Draw the players bullets
			DrawBullets(activeBuffer);
		}
		else {
			// Display the winner
			if (player1.lives <=0) FntPrint(fntID, "\nPlayer 2 is the winner!\n\n\n");
			else FntPrint(fntID, "\nPlayer 1 is the winner!\n\n\n\n");
			
			// Check for game restart
			FntPrint(fntID, "Press Start to restart the game.\n");
			padBuff = PadRead();
			if (padBuff & PAD1start) InitPlayers();
		}
		
		// Update the graphics
        GraphicsEndFrame(activeBuffer);
		
        frame++;
    }
	
	YarioClose(); // Call YarioClose to close the tty connection

    return 0;
}