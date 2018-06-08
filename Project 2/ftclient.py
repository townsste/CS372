#/***********************************************************
#* CS372 - Project 2
#* Author: Stephen Townsend
#* File: ftclient.py
############################################
#* This py file is used to create a file transfer client.
#* This will be used to communicate with a server and get the
#*servers current directory and/or transfer a file that is in
#*the current directory of the server.
#***********************************************************/
#*Inputs:
#*    For Directory
#*      <ServerName> <ConnectionPort> -l <DataPort>
#*    File Transfer
#*      <ServerName> <ConnectionPort> -g <FileName> <DataPort>

from socket import *
from socket import error
import errno
import sys
import os
from os import path

#This function is used to connect to the server.  Both with the initial connection and
#through the data port.
def Connect(port):
    serverName = sys.argv[1]
    try:
        portNumber = int(sys.argv[port])
    except ValueError:
        print "Please enter a valid port number"
        return 0
    
    try:   
        serverSocket = socket(AF_INET, SOCK_STREAM)
        serverSocket.connect((serverName, portNumber))
    except error, serr:
        print "Connection Refused."
        return 0
    return serverSocket

#This function is used to check the amount of arguments the user inputed.
def CheckArgTotal():

    #sys.argv[]
    #0 - Program Name
    #1 - Server Name
    #2 - Connection Port #
    #3 - Command
    #(-l)
    #4 - Data Port #
    #(-g)
    #4 - Filename
    #5 - Data Port #
    #6 - Should be null
    
    try:
        if (sys.argv[3] != IndexError):
            if (sys.argv[3] == "-l"):
                number = 4
            elif (sys.argv[3] == "-g"):
                number = 5
            else:
                number = -3

            #with the number from above we can check the sys.argv inputs to
            #make sure the user has the correct number of inputs.
            #below we will take advantage of the IndexError if there is no information
            #in the sys.argv areas that this checks.
            try:
                if (number == -3):   #flag for an invalid command
                    InputErrorHandle("invalid", sys.argv[3])
                else:
                    if (sys.argv[number] != IndexError):
                        #The comparison will prompt an IndexError if there is one.
                        #if port number is missing then sys.argv[4] will IndexError.
                        #Catch the error and prompt user.
                        try:
                            if (sys.argv[number+1] != IndexError):
                                #Want an index error. Indicates correct number of inputs
                                #Check for more inputs, if found, then prompt error.
                                InputErrorHandle("high", sys.argv[3])   #error
                                return "error"   
                        except IndexError:  #IndexError catch indicates the input is valid.
                            return sys.argv[3]
            except IndexError:  #Catch IndexError from sys.argv[number] missing
                InputErrorHandle("low", sys.argv[3])
                return "error"
    except IndexError: #Catch IndexError from sys.argv[3] missing
        InputErrorHandle("", "")
        
#This is the handler for check ing the total of arguments
def InputErrorHandle(condition, command):
    if condition == "high":
        print "Too many arguments with the " + command + " command"
    elif condition == "low":
        print "Too few arguments with the " + command + " command"
    elif condition == "invalid":
        print command + " is not recognized as a command"
    else:
        print "Missing command"

#This function is used to check the users ports and make sure they are valid before
#being sent to the server
def CheckPortNumbers():
    try:
        int(sys.argv[2])
        if (int(sys.argv[2]) > 65535):
            return "Connection Port must be within 0 and 65535"
    except ValueError:
        return "Connection Port must be an integer" 
    if (sys.argv[3] == "-l"):
        try:
            int(sys.argv[4])
            if (int(sys.argv[4]) > 65535):
                return "Data Port must be within 0 and 65535"
        except ValueError:
            return "Data Port must be an integer"
    elif (sys.argv[3] == "-g"):
        try:
            int(sys.argv[5])
            if (int(sys.argv[5]) > 65535):
                return "Data Port must be within 0 and 65535"
        except ValueError:
            return "Data Port must be an integer"

    #All ports are valid
    return 0

#This function is used to send a message to the server
def SendMessage(connectionSocket, number):
    command = sys.argv[number]
    connectionSocket.send(command)

#This function is used to recieve a message from the server
def ReceiveMessage(connectionSocket):
    exitConvo = False
    clientMessage = connectionSocket.recv(800)
    return clientMessage

#This function is used just to recieve a file from the server.
def ReceiveFile(connectionSocket, fileName):
    #Create and open the file in write mode
    file = open(fileName,"w")

    #Recieve file information from server
    fileBuffer = connectionSocket.recv(1024)

    #Loop until final response is sent
    while "!!done!!" not in fileBuffer:
        file.write(fileBuffer)
        fileBuffer = connectionSocket.recv(1024)
        
    #Close the open file
    file.close()

os.system('clear')

case = CheckArgTotal()

if (case == "-l" or case == "-g"):
    validPorts = CheckPortNumbers()

    if (validPorts == 0):   #Make sure ports are valid

        serverSocket = Connect(2)   #Initial connection to server

        allValid = True     #Flag indicating everything is valid so far

        if (serverSocket != 0):     #If there is a socket continue
            #Starting to send data to the server for later use.  Data port and filename. 
            SendMessage(serverSocket, 3)    #Send command
            message = ReceiveMessage(serverSocket)  #Server Response
            #Check Response
            if message == "Y":  
                SendMessage(serverSocket, 4)    #Send Data Port if -l or filename if -g
                message = ReceiveMessage(serverSocket)  #Server Response
                #Check Response
                if message == "Y":
                    SendMessage(serverSocket, 5)    #Send Data Port if -g
                    message = ReceiveMessage(serverSocket)  #Server Response
                #Faild Response
                elif message != "N":
                    allValid = False    #Server did not like the data port
                    print message
            #Faild Response
            elif message != "N":
                allValid = False        #Server did not like the dataPort or filename
                print message        

        if(allValid):
            if (case == "-l"):
                serverSocketData = Connect(4)   #Data Connection

                if(serverSocketData != 0):
                    print "Receiving directory structure from " + sys.argv[1] + ":" + sys.argv[4]
                    message = ReceiveMessage(serverSocketData)
                    print message
                    serverSocketData.close()
                else:
                    print "Please Check the Data Port"
            elif (case == "-g"):
                serverSocketData = Connect(5)   #Data Connection

                #Check if file exists
                if(serverSocketData != 0):
                    checkName = sys.argv[4]
                    if (path.isfile(checkName)):
                        #Append copy if previous file exists
                        checkName = checkName.split(".")[0] + "_copy.txt"
                        if (path.isfile(checkName)):
                            #Append and incremented number if previous file exists
                            i = 1
                            while (True):
                                checkName = checkName.split(".")[0]
                                checkName = checkName.split("(")[0] + "(" + str (i) + ").txt"
                                if (path.isfile(checkName)):
                                    i = i + 1
                                else:
                                    break

                    print "Receiving " + sys.argv[4] + " from " + sys.argv[1] + ":" + sys.argv[5]
                    ReceiveFile(serverSocketData, checkName)

                    print "File transfer complete."
                    
                    serverSocketData.close()
                else:
                    print "Please Check the Data Port"
                
            serverSocket.close()
    else:
        print validPorts
