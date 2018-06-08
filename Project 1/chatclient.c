/*********************************************
* CS372 - Project 1
* Author: Stephen Townsend
* File: chatclient.c
* This program is used as a client. This is 
* based on my CS344 otp_enc.c file. This will 
* connect to a python server.  The user will 
* state a handler to use and they will then be 
* able to send messages to the server.
****syntax: chatclient {localhost} {55336}****
*********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define ARR_SIZE  600									//Define a constant file size
#define HAN_SIZE  20									//Define a constant file size

/*Prototpye initialization*/
int sendMessage(int, char[], char[], int);
int receiveMessage(int, char[]);
int getHandle(char[]);
int checkBufferExit(char buffer[], int);
void clearBuffer(char[]);
int removeNewLine(char[]);


/******************************************
*				main
* This will be used to direct the client.
* the socket and connections are created 
* here.  This is where we can call send
* and receive.
******************************************/
void main(int argc, char *argv[])
{
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[ARR_SIZE];
	int exitConvo = 0;
	char handle[HAN_SIZE];
	int counter = 0;
	
	/*Contents of argv[argc]*/
	//argv[0] = Current Program
	//argv[1] = Hostname
	//argv[2] = Port Number

	/*Check usage & args*/
	if (argc < 3) 
	{ 
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	} 
	
	/*Set up the client address struct*/
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //Clear out the address struct
	portNumber = atoi(argv[2]);					//Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;			//Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);	//Store the port number
	serverHostInfo = gethostbyname(argv[1]);	//Convert the machine name into a special form of address
	if (serverHostInfo == NULL) 
	{ 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	/*Set up the socket*/
	socketFD = socket(AF_INET, SOCK_STREAM, 0);	//Create the socket
	if (socketFD < 0) 
	{
		fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", portNumber);
		exit(2);								//Exit with error
	}

	/*Connect to server*/
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	
		system("clear");						//Clear the screen
		
		printf("connected to %s\n", argv[1]);	//Connection made
		
		counter = getHandle(handle);			//Call get handle
		
		while(exitConvo != 1)
		{
			exitConvo = sendMessage(socketFD, buffer, handle, counter);	//Send message to server
			if (exitConvo == 0)
				exitConvo = receiveMessage(socketFD, buffer);			//Receive message from server
		}
	printf("Exiting\n");
	close(socketFD);							//Close the socket	
	exit(0);									//Exit no problems
}

/******************************************
*				sendMessage
* This function is used send data to
* the server.
******************************************/
int sendMessage(int socketFD, char buffer[], char handle[], int handleCounter)
{
	int charsWritten;
	char temp[ARR_SIZE];
	
	clearBuffer(buffer);					//Clean out the buffer
	clearBuffer(temp);						//Clean out temp
	
	printf("\n%s: ", handle);
	
	fgets(temp, ARR_SIZE, stdin);
	fflush(stdin);							//Flush input buffer
	
	removeNewLine(temp);
	
	sprintf(buffer,"%s: %s", handle, temp); //Put the handle in front for sending
	
	removeNewLine(buffer);
	
	charsWritten = write(socketFD, buffer, strlen(buffer));		//Send built message to server
	
	if (charsWritten < 0)
		error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer))
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	
	if(checkBufferExit(buffer, handleCounter+1) == 0)
		return 0;
	else
		return 1;
}

/******************************************
*				receiveMessage
* This function is used receive data from
* the server.
******************************************/
int receiveMessage(int socketFD, char buffer[])
{
	int charsRead;
	
	clearBuffer(buffer);					//Clear out the buffer
	
	//printf("waiting for server message...\n");
	
	recv(socketFD, buffer, ARR_SIZE, 0);	//Read data from the socket, leaving \0 at end

	if (charsRead < 0)
		error("CLIENT: ERROR reading from socket");
	
	if(checkBufferExit(buffer, 14) == 0)
	{
		printf("\n\a%5s\n", buffer);
		return 0;
	}
	else
		return 1;
}

/******************************************
*				getHandle
* This function is used to get the users
* handle.
******************************************/
int getHandle(char handle[])
{
	int counter = 0;
	printf("Handle: ");
	fgets(handle, HAN_SIZE, stdin);
	fflush(stdin);
	
	counter = removeNewLine(handle);
	
	return counter;
}

/******************************************
*				clearBuffer
* This function is used to clear the
* buffer so that it can be reused without
* having trash in it.
******************************************/
void clearBuffer(char buffer[])
{
	memset(buffer, '\0', ARR_SIZE);
}

int checkBufferExit(char buffer[], int loc)
{
	int x = loc;

	if (buffer[x] == '\\' && 
		buffer[x+1] == 'q' && 
		buffer[x+2] == 'u' && 
		buffer[x+3] == 'i' && 
		buffer[x+4] == 't' && 
		buffer[x+5] == 0)
		return 1;
	else
		return 0;
}


/******************************************
*				removeNewLine
* This function is used to remove any
* newline characters from input to prevent
* issues with sending
******************************************/
int removeNewLine(char buffer[])
{
	int i;
    for (i = 0; i < strlen(buffer); i++) 
	{
        if (buffer[i] == '\n') 
			buffer[i] = '\0';
	}
	return i;
}