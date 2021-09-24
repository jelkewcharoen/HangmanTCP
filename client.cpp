#include <stdio.h>		    /* for printf() and fprintf() */
#include <sys/socket.h>		    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <iostream>
#define BUFFSIZE 100
using namespace std;

//this function sends a message of size "size" stored at "buffer" to socket of the descriptor "clientSock"
void sendToServer(int clientSock, char* buffer, int size) {
    int bytes = send(clientSock, buffer, size , 0);
    if (bytes < 0) {
        perror("Error: fail to send packet to the server.");
        exit(1);
    } else if (bytes != size) {
        printf("Error: send() sent unexpected number of bytes.");
        exit(1);
    }
}

//this function reads a message from a socket of descriptor "clientSock" and store it in "buffer"
//if the number of byte received is 0, indicating connection has ended, it closes the connection and exit
void readFromServer(int clientSock, char* buffer) {
    int bytes = recv(clientSock, buffer, BUFFSIZE, 0);
	if (bytes < 0) {
		printf("Error: fail to receive packet from the server.");
		exit(1);
	} else if (bytes == 0) {
		printf("Quitting the connection.\n");
        close(clientSock);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
    int clientSock;		            /* socket descriptor */
    struct sockaddr_in servAddr;   /* server address structure */
    char rcvBuf[BUFFSIZE];              //buffer for receiving message
    char sndBuf[BUFFSIZE];              //buffer for sending message
    char *servIP;		            /* Server IP address  */
    unsigned short servPort;	    /* Server Port number */
    vector<char> listLetter;        //list of already guessed letters
   
   //check for number of arguments
   if (argc != 3) {
		printf("Invalid arguments.");
		exit(1);
	} else {
        servIP = argv[1];
        servPort = atoi(argv[2]);
    }

    //create a new socket
    if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating a new TCP socket.");
		exit(1);
	}

    //construct the server address structure
    memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servIP);
	servAddr.sin_port = htons(servPort);

    //establish connection to the server
    int connectVal = connect(clientSock, (struct sockaddr*) &servAddr, sizeof(servAddr));
	if (connectVal < 0) {
		perror("Error: fail to establish a connection to the server");
		exit(1);
	}

    //read "Ready to start" message from server
    memset(&rcvBuf, 0, BUFFSIZE);
    readFromServer(clientSock, rcvBuf);
    
    //print "Ready to start game?"
    int length = rcvBuf[0];
    printf("%.*s ", length, &rcvBuf[1]);
    if (rcvBuf[1] == 'S') { //if the message was "Server-overloaded"--quit the connection
        printf("\nQuitting the connection.\n");
        close(clientSock);
        exit(1);
    }

    //send start game respond back to server
    memset(&sndBuf, 0, BUFFSIZE);
    string input; //inputs could be n,y,or a number
    cin>>input;
    //cout<<input<<endl;
    sndBuf[0] = 1;
    if (isdigit(input[0])) { //if the input is a number
        sndBuf[1] = stoi(input);
    } else { //if the input is a character
        sndBuf[1] = input[0];
    }
    sendToServer(clientSock, sndBuf, 2);
    //game start
    while (true) {
        //receive message from server and print
        readFromServer(clientSock, rcvBuf);
        if (rcvBuf[0] == 0) { //game control packet
            int wordLen = rcvBuf[1]; 
            int incorrect  = rcvBuf[2];
            cout<<endl;
            for (int i = 0; i < wordLen; i++) {
                cout<<rcvBuf[3+i]<<" ";
            }
            cout<<"\nIncorrect Guesses: ";
            for (int i = 0; i < incorrect; i++) {
                cout<<rcvBuf[3+wordLen+i]<<" ";
            }
            cout<<endl;
            //read a letter from the input and send the server a message
            while (true) {
                cout<<"\nLetter to guess: ";
                string input2;
                cin>>input2;
                if (input2.size() > 1) { //if input is not just one letter
                    cout<<"\nError: please guess only ONE letter."<<endl;
                } else if (input2[0] < 65 || input2[0] > 122 || (input2[0] >90 && input2[0] < 97)) { //if input is not a letter
                    cout<<"\nError: please guess a LETTER."<<endl;
                } else {
                    if (input2[0] > 64 || input2[0] < 91) { //if input is a capital letter
                        input += 32;
                    }
                    bool repeat = false; //keep track whether the letter is repeated
                    for (int i = 0; i < listLetter.size(); i++) {
                        if (input2[0] == listLetter[i]) { //if the letter has already been guessed
                            repeat = true;
                            cout<<"\nError: letter ";
                            cout<<input2[0];
                            cout<<" has already been guessed. Please guess another letter."<<endl;
                            break;
                        }
                    }
                    if (!repeat) {
                        listLetter.push_back(input2[0]);
                        sndBuf[0] = 1;
                        sndBuf[1] = input2[0];
                        sendToServer(clientSock, sndBuf, 2);
                        break;
                    }
                }
            }
        } else { //message packet
            length = rcvBuf[0];
            printf("%.*s ", length, &rcvBuf[1]); //print out message
            if (rcvBuf[1] == 'Y') { //if the message was either "You win" or "You lose"--quit the connection
                printf("\nGame over.\n");
                close(clientSock);
                break;
            }
        }
    }
}
    	
