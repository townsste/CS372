/*********************************************
* CS372 - Project 2
* Author: Stephen Townsend
* File: ftserver.c
* This program is used as a server. This is 
* based on my CS344 otp_enc_d.c file. This will 
* open a port and listen.  It will wait for a
* command from the python client.  This server 
* is designed to send the current directory 
* and/or start a file transfer when requested.
* This server is multi threaded where is forks
* into child processes to do the work.  If the
* child process takes to long or hangs up due 
* to waiting for the client to make a data 
* connection; the parent will kill the child
* after a timeout period.
****syntax: ftserver <PortNumber>****
*********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

/*Prototpye initialization*/
int connection (int *, int);
void validation(int, char[]);
void receiveClient(int, char[]);
void sendClient(int, char[]);
void getDirectory(char []);
char* copyFile(char []);
void clearBuffer(char[]);

/******************************************
*				main
* This will be used to direct the server.
* the socket and connections are created
* here.  This is where we can call send
* and receive.
******************************************/
void main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD;
	int listenSocketDataFD, establishedConnectionDataFD;
	int portNumber, portNumberData;
	
	char commandBuffer[10];
	char filenameBuffer[30];
	char portBuffer[7];
	char clientError[50];
	char tempBuffer[700];
	char respond[2];
	
	socklen_t sizeOfClientInfo;
	struct sockaddr_in clientAddress;
	
	struct timespec timeout;
	sigset_t mask;
	sigset_t orig_mask;
	
	/*Contents of argv[argc]*/
	//argv[0] = Current Program
	//argv[1] = Port Number

	//Check usage & args
	if (argc < 2)
	{
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}
	
	portNumber = atoi(argv[1]);							//Get the port number, convert to an integer from a string

	establishedConnectionFD = connection(&listenSocketFD, portNumber);
	
	/*Accept a connection, blocking if one is not available until one connects*/
	while (1)
	{
		printf("Server open on %d\n", portNumber);

		sizeOfClientInfo = sizeof(clientAddress);		//Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); //Accept

		if (establishedConnectionFD < 0)
			error("ERROR on accept");
		
		system("clear");
	
		/*Run a new process*/
		int spawnPid;
		int childExitMethod = -2;
		
		sigemptyset (&mask);
		sigaddset (&mask, SIGCHLD);

		if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) 
		{
			perror ("sigprocmask");
		}

		spawnPid = fork();
		
		timeout.tv_sec = 5;
		timeout.tv_nsec = 0;

		if (spawnPid == 0)
		{
			receiveClient(establishedConnectionFD, commandBuffer);
			
			//DIRECTORY COMMAND
			if(strcmp(commandBuffer, "-l") == 0)
			{
				//Received confirmation
				stpcpy(respond, "Y");
				validation(establishedConnectionFD, respond);
				
				//Receive DataPort information
				receiveClient(establishedConnectionFD, portBuffer);
				
				//Convert char arr to decimal
				sscanf(portBuffer, "%d", &portNumberData);
				
				//Finished receiving data
				stpcpy(respond, "N");
				validation(establishedConnectionFD, respond);
				
				printf("List directory requested on port %d\n", portNumberData);
				
				//Start Data Connection
				establishedConnectionDataFD = connection(&listenSocketDataFD, portNumberData);
				sizeOfClientInfo = sizeof(clientAddress);		//Get the size of the address for the client that will connect
				establishedConnectionDataFD = accept(listenSocketDataFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
				
				if (establishedConnectionDataFD < 0)
				error("ERROR on accept");
			
				clearBuffer(tempBuffer);
				
				getDirectory(tempBuffer);
				
				printf("Sending directory contents on port %d\n", portNumberData);

				//Send over Data Connection
				sendClient(establishedConnectionDataFD, tempBuffer);
				
				printf("Request Completed\n");
			}				
			else if(strcmp(commandBuffer, "-g") == 0)
			{
				int sendFile = 0;
				char* fileData;
				
				//Received confirmation
				stpcpy(respond, "Y");
				validation(establishedConnectionFD, respond);
				
				//Receive filename information
				receiveClient(establishedConnectionFD, filenameBuffer);
				
				printf("File %s requested\n", filenameBuffer);
				
				//Check to see if file exists
				if (access(filenameBuffer, F_OK) == -1) 
				{
					printf("File not found\n");
					stpcpy(clientError, "File not found");
					sendClient(establishedConnectionFD, clientError);
					sendFile = -1;
				}
				else
					fileData = copyFile(filenameBuffer);	//Copy file to buffer
				
				//Received confirmation
				validation(establishedConnectionFD, respond);
				
				//Receive DataPort information
				receiveClient(establishedConnectionFD, portBuffer);
				
				//Convert char arr to decimal
				sscanf(portBuffer, "%d", &portNumberData);
				
				//Finished receiving data
				stpcpy(respond, "N");
				validation(establishedConnectionFD, respond);
				
				//Verify no errors previously
				if (sendFile == 0)
				{
					//Start Data Connection
					establishedConnectionDataFD = connection(&listenSocketDataFD, portNumberData);
					sizeOfClientInfo = sizeof(clientAddress);		//Get the size of the address for the client that will connect
					establishedConnectionDataFD = accept(listenSocketDataFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
					
					if (establishedConnectionDataFD < 0)
						error("ERROR on accept");
					
					printf("Sending %s on %d\n", filenameBuffer, portNumberData);
					
					//Send file to client
					sendClient(establishedConnectionDataFD, fileData);
					
					printf("Finished Sending\n");
					
					//Let the client know the file transfer is complete
					clearBuffer(tempBuffer);
					strcpy(tempBuffer, "!!done!!");
					sendClient(establishedConnectionDataFD, tempBuffer);
				}				
			}
			else
			{
				stpcpy(clientError, "Invalid Command");
				sendClient(establishedConnectionFD, clientError);
				printf("Invalid Command");
			}
			_exit(0);
		}
		else //Parent
		{
			if (sigtimedwait(&mask, NULL, &timeout) < 0) 
			{
				if (errno == EINTR) 
				{
					 //Interrupted by a signal other than SIGCHLD.
					continue;
				}
				else if (errno == EAGAIN) 
				{
					printf ("Data Connection Timeout\n");
					kill (spawnPid, SIGKILL);
				}
				else 
				{
					perror ("sigtimedwait");
				}
			}
			//Reference: https://www.linuxprogrammingblog.com/code-examples/signal-waiting-sigtimedwait
		}
		close(establishedConnectionDataFD);				//Close the existing socket which is connected to the client
		close(listenSocketDataFD);						//Close the listening socket
		close(establishedConnectionFD);					//Close the existing socket which is connected to the client
		}
	close(listenSocketFD);								//Close the listening socket
}

/******************************************
*				connection
* This will be used to set up a new port that
* the server will listen on waiting for the
* client to accept.
******************************************/
int connection(int *listenSocketFD, int portNumber)
{
	struct sockaddr_in serverAddress;
	int establishedConnectionFD;
	
	/*Set up the server address struct*/
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET;					//Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);			//Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY;			//Any address is allowed for connection to this process

														/*Set up the socket*/
	*listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);	//Create the socket
	if (*listenSocketFD < 0)
		error("ERROR opening socket");

	/*Enable the socket to begin listening*/
	if (bind(*listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

	/*Flip the socket on*/
	listen(*listenSocketFD, 5);							//It can receive up to 5 connections 
	
	return establishedConnectionFD;
}

/******************************************
*				receiveClient
* This function is used to receive data from
* the client.
******************************************/
void receiveClient(int establishedConnectionFD, char buffer[])
{
	int charsRead;
	
	clearBuffer(buffer);							//clear the current buffer
	charsRead = recv(establishedConnectionFD, buffer, (strlen(buffer) - 1), 0); // Read the client's message from the socket
	if (charsRead < 0)
		error("CLIENT: ERROR reading from socket");
}

/******************************************
*				sendClient
* This function is used to send data to
* the client.
******************************************/
void sendClient(int establishedConnectionFD, char buffer[])
{
	ssize_t charsRead;
    size_t size = strlen(buffer) + 1;
    size_t resize = 0;

    // sends data until all is sent
    while (resize < size) {
        charsRead = write(establishedConnectionFD, buffer, size - resize);

        resize += charsRead;

        // checks if there was an error with the write
        if (charsRead < 0) {
            error("ERROR: unable to send a message");
        }

        // increases the total amount sent
        else if (charsRead == 0) {
            resize = size - resize;
        }
    }
}

/******************************************
*				validation
* This function is used to send small messages
* back to the client after receiving and 
* verifying any information.
******************************************/
void validation(int connectionFD, char buffer[])
{
	int charsRead;
	
	// Send a Success message back to the client
	charsRead = send(connectionFD, buffer, 1, 0);  //Send back confirmation
	if (charsRead < 0) error("ERROR writing to socket");
}

/******************************************
*				validation
* This function is used to get information 
* about the current directory.  It filters
* the results and only returns any .txt files
* in the directory.
******************************************/
void getDirectory(char buffer[])
{
	/*###############################*/
	DIR *dp;
	struct dirent *ep;
	dp = opendir ("./");
	if (dp != NULL)
	{
		while (ep = readdir (dp))
		{
			int length = strlen(ep->d_name);
			if (strncmp(ep->d_name + length - 4, ".txt", 4) == 0) 
			{
				strcat(buffer, ep->d_name);
				strcat(buffer, "\n");
			}
		}
		(void) closedir (dp);
	}
	else
		perror ("Couldn't open the directory");
	/*############################### 
		Source: http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
	*/
}

/******************************************
*				copyFile
* This function is used to open the file
* that the client is looking for.  It 
* uses a dynamic array that will fill up
* with the contents of the file.  Once the
* entire file has been copied,  it will return
* a char pointer.  I got the return idea from 
* the reference below this function.
******************************************/
char* copyFile(char* filename)
{
	FILE* fp;
	char *source = NULL;
	int size;

    // opens the file
    fp = fopen(filename, "r");

    // confirms if it can be opened
    if (!fp) {
        error("ERROR: unable to open file");
    }
	else
	{
        //Seek to the end to get the file size
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);

		//Dynamic array that will hold the files contents
		source = malloc(sizeof(char) * (size + 1));
		
		//Set FP to begging
		fseek(fp, 0, SEEK_SET);
		
		//Put file data into array
		fread(source, sizeof(char), size, fp);
    }
    
    //Close FP
    fclose(fp);

    //Return the array
    return source;
}
/*###############################
		Returning a char pointer.  Received the idea from below.
		Source: https://stackoverflow.com/questions/10131464/returning-a-char-pointer-in-c
	*/
	
/******************************************
*				clearBuffer
* This function is used to clear the
* buffer so that it can be reused without
* having trash in it.
******************************************/
void clearBuffer(char buffer[])
{
	memset(buffer, '\0', strlen(buffer));
}