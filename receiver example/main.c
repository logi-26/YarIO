#include <libps.h>
#include <sys/ioctl.h>
#include "yario.h"

#define SCREEN_W 320
#define SCREEN_H 240

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
    int activeBuffer = GsGetActiveBuff();
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

int main(void) {
    int i, j, activeBuffer, framesSinceLast, packetsReceived, frame = 0, showData = 0, lastDataFrame = -300;
    const unsigned char* data;

    InitGfx();
    YarioInit();

    while (1) {
        activeBuffer = GraphicsStartFrame();

        FntPrint(fntID, "Yario Data Receiver\n");
        FntPrint(fntID, "===================\n");
        FntPrint(fntID, "Frame: %d\n\n", frame);

		// If we have recieved the full 60-bytes of data
        if (YarioIsDataReady()) {
            data = YarioGetDataBuffer();
            showData = 1;
            lastDataFrame = frame;
            YarioClearDataReady();
        }

		// Display the data
        if (showData) {
            FntPrint(fntID, "Status: 60 bytes ready!\n\n");
            FntPrint(fntID, "HEX + ASCII view:\n\n");

            for (i = 0; i < 60; i++) {
				
				// Display the byte value
                unsigned char byte = data[i];
                FntPrint(fntID, "%02X ", byte);
				
				// Display the ascii
                if ((i + 1) % 6 == 0) {
                    FntPrint(fntID, " | ");
                    for (j = i - 5; j <= i; j++) {
                        unsigned char ch = data[j];
                        if (ch >= 32 && ch <= 126) {
                            FntPrint(fntID, "%c", ch);
                        } else {
                            FntPrint(fntID, ".");
                        }
                    }
                    FntPrint(fntID, "\n");
                }
            }

            FntPrint(fntID, "\nWaiting for next 10 packets...\n");
        } else {
			// Waiting for packets
            framesSinceLast = frame - lastDataFrame;
            packetsReceived = YarioGetPacketCount();

            FntPrint(fntID, "Waiting for data...\n");
            FntPrint(fntID, "Received packets: %d/10\n", packetsReceived);

			// Count the frames since the last full 60-bytes where received
            if (framesSinceLast > 300) {
                FntPrint(fntID, "No data for %d frames.\n", framesSinceLast);
            }
        }

		// Check if there are any packets to recieve
        YarioUpdate();
		
        GraphicsEndFrame(activeBuffer);
        frame++;
    }

	// Close the tty connection
    YarioClose();
    return 0;
}


