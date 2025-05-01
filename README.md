# YarIO

## PlayStation Net Yaroze Basic TTY IO

The professional PlayStation SDK (PSYQ) has raw access to SIO/serial port on the back of the PlayStation console.
The PSYQ SDK also has high-level libraries (libsio, libcomb) for adding interupts, controlling the SIO port and data flow etc.
These where used to create PlayStation link games, by linking two PlayStation consoles together using the game link cable.

Only a very small percentage of commercial PlayStation games made use of the game link cable for linked gameplay.

Unlike the professional PlayStation SDK, the PlayStation Net Yaroze SDK does not have access to the SIO.
Because of this, the Net Yaroze alos does not have any libraries of linked gameplay using a game link cable between two PlayStation consoles.
The Net Yaroze SDK uses the serial port for uploading code/assets and for sending debug messages to a connected PC using TTY.

I wanted to make a PlayStation game using the Net Yaroze SDK that uses the game link cable for linked gameplay between two consoles.
Because the Net Yaroze SDK does not have access to the SIO port or any libraries for communication nobody had ever attempted it.

The goal of this project was to be able to send the complete control pad buffer between two connected PlayStation consoles on every frame.
The extended goal was to see how many bytes could be transfered between the console's on each frame.
Because the Net Yaroze SDK only has access to TTY over the serial port, that was be used as a SIO substitute.

Based on my limited testing, 8-bytes of data per frame is the most that the console's could handle without glitching/freezing.
At 50 FPS this is equivalent to 400 bytes per second in both directions.

The complete control pad buffer for both controller ports is 8-bytes, so this data can be sent between the consoles on each frame.
Most link games will not need the entire control pad buffer because most link games will only need to read controller port 1 on each console.
Controller port 1 data is 4-bytes, but if you do not need the controller status or analog stick data it can be reduce to 2-bytes.

This Net Yaroze code sends/receives 8-byte packets of data every frame.
Each packet consists of a start-byte, 6-bytes of data and the packet ends with an XOR of all the bytes in the packet.
Any 6-bytes of data could be inserted into the packet.

The example code is sending/receiving the 2-bytes of data from the digital controller connected to port 1 in each packet.
The remaining 4-bytes in the packet just contain padding bytes as placeholder data, which could be replaced with any data.

I have tried to make the code easy to drop into any Net Yaroze project.

You only need to import yario.h into your project and call YarioInit() to setup and initialise the TTY.
Call YarioUpdate() and pass it a u_long containing the data to be sent/received.
Call YarioGetRemoteBuff() to access the recieved data from the buffer.
Call YarioClose() when your game ends.

The "basic example" demonstrates how to use the yarIO code to connect two PlayStation consoles and have each console comtrol a seperate player using TTY.
The "basic game" takes the example and adds some basic gameplay elements.