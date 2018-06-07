Project 1

For this project you will need to Start the server, then start the client in a new window. 
You can then switch back and forth between the two terminalwindows.

RESOURCES:
For my pyhton server I used the information given in the lectures for a python TCP server.

For my c client, it is based on my CS344 otp_enc.c file.

Any other resources will be commented in the file.


INSTRUCTIONS:
1) Run chatserver.py
    python chatserver.py <port#>
EXAMPLE: python chatserver.py 55333

2) Compile chatclient.c
    gcc chatclient.c -o chatclient

3) Run chatclient.c
    ./chatclient <server-hostname> <port#>
EXAMPLE: ./chatclient localhost 55333

4) Client side, enter a handle

5) Client side, enter a message and press enter.

6) Client's message will be displayed on the server.

7) Server side, enter a message and press enter.

8) Server's message will be displayed on the client.  
NOTE: For EXTRA effect there will be a ping sound on the client side when the server responds

9) This process will continue untill either side sends a \quit command.
Note: \quit will close the connection to the client.

10) The server will wait for another connection.
NOTE: Server can be be stopped by sending a  sigint command {CTR C}
