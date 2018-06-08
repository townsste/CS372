#/***********************************************************
#* CS372 - Project 1
#* Author: Stephen Townsend
#* File: chatserver.py
#* This py file is used to create a chat server.  This will 
#* start the server and listening on the specified port.
#* The server will then wait for a client to connect.  Once 
#* a client connects you can send messages back and forth 
#* between the client.
#***********************************************************/

from socket import *
import sys
import os

def StartUp():
    portNumber = int(sys.argv[1])
    serverSocket = socket(AF_INET, SOCK_STREAM)
    serverSocket.bind(('', portNumber))
    return (serverSocket)


def SendMessage(connectionSocket):
    sentence = raw_input("\nWhite Rabbit: ")
    connectionSocket.send("White Rabbit: " + sentence)
    if sentence == "\quit":
        exitConvo = True
        return exitConvo


def ReceiveMessage(connectionSocket):
    exitConvo = False
    clientMessage = connectionSocket.recv(600)
    test = clientMessage
    
    if "\quit" in clientMessage:
        exitConvo = True

    ###############################
        
    sys.stdout.write("\033[F") #back to previous line
    sys.stdout.write("\033[K") #clear line
    
    ############################### 
    #Source: https://www.quora.com/How-can-I-delete-the-last-printed-line-in-Python-language

    if exitConvo:
        return exitConvo
    else:
        print clientMessage

serverSocket = StartUp()

while 1:
    os.system('cls')
    os.system('clear')
    
    serverSocket.listen (1)
    print "waiting for a connection"
    connectionSocket, addr = serverSocket.accept()
    print "connection made"

    while 1:
        print "\nwaiting for client message..."
        exitConvo = ReceiveMessage(connectionSocket)
        if exitConvo:
            print "Exiting"
            connectionSocket.close()
            break
        else:
            exitConvo = SendMessage(connectionSocket)
            if exitConvo:
                print "Exiting"
                connectionSocket.close()
                break