Project 2

For this project you will need to Start the server, then start the client in a new window. 
You can then switch back and forth between the two terminal windows.

::EXTRA CREDIT::
For extra credit, the server utilizes forking and uses different processes for each client that may be requesting information.
This will allow the server to handle more then one client at a time when requesting or transferring files.  THe socket is set up
to handle 5 different connections.  

There are times when the connection to connect to the Data Port is refused.  As a result, this would cause the current child to hang, 
but the client can still request information from to server.  To fix this hanging issue, there is a timeout for the child process.  
If the child hangs for any issue without anything changing then the the parent will kill the child process after a 5 second timeout.

::RESOURCES::
For my python client I used the information given in the lectures for a python TCP client

For my c server, it is based on my CS344 otp_enc_d.c file.

Any other resources will be commented in the file.


INSTRUCTIONS:
1) Compile ftserver.c
    gcc ftserver.c -o ftserver

2) Run ftserver.c
    ./ftserver <port#>
EXAMPLE: ./ftserver 55333

3) Run ftclient.py
    -l command
        python ftclient.py <server-hostname> <port#> <command> <data-port#>
EXAMPLE: python ftclient.py localhost 55333 -l 55222

    -g command
        python ftclient.py <server-hostname> <port#> <command> <filename> <data-port#>
EXAMPLE: python ftclient.py localhost 55333 -g shortfile.txt 55222


4) Server side, will open a new port and send directory or file to client.

5) Server's messages will be displayed on the client.

6) The client will exit after each command.

7) The server will wait for another connection.
NOTE: Server can be be stopped by sending a  sigint command {CTR C}
