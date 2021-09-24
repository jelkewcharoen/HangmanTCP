#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <cstdlib>              //for random number
#include <fstream>
#include <iostream>
#include <errno.h>  
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/time.h>  //support fd functions for multi-connection
using namespace std;
#define BUFSIZE 1024

//this structure contains information about each client
struct client{
    int sd;                             //socket descriptor
    string answer;                      //correct answer that was randomly chosen/assigned
    int wordLen;                        //length of the answer
    string guess;                       //correctly guessed letters
    string incorrect;                    //incorrect guessed letters
    int incorrectNum;                   //number of incorrect guesses
};

void resetClient(client* c) {
    c->sd = 0; 
    c->answer = "";
    c->wordLen = 0;
    c->guess = "";
    c->incorrect = "";
    c->incorrectNum = 0;
}
//this method pick a random word out of the list
string pickAWord (int randNum) {
    //srand((unsigned) time(0));
    int random = randNum%5;
    switch(random) {
        case 0: 
            return "networking";
        case 1: 
            return "computer";
        case 2:
            return "georgia";
        case 3:
            return "tech";
        case 4:
            return "coronavirus";
        default:
            return "";
    }
}

//this method sends data of size "size" in the "buffer" to client of socket descriptor "clientSock"
void sendToClient(int clientSock, char* buffer, int size) {
    int bytes = send(clientSock, buffer, size , 0);
    if (bytes < 0) {
        perror("Error: fail to send packet to the client.");
        exit(1);
    } else if (bytes != size) {
        printf("Error: send() sent unexpected number of bytes.");
        exit(1);
    }
}

//this method reads data from client of socket descriptor "clientSock" and put in "buffer"
//returns 1 if message is received correctly
//returns 0 if message length is 0, indicating the client has closed its connection
int readFromClient(int clientSock, char* buffer) {
    int bytes = recv(clientSock, buffer, BUFSIZE, 0);
	if (bytes < 0) {
		printf("Error: fail to receive packet from the client.");
		exit(1);
	} else if (bytes == 0) {
		return 0;
	}
    return 1;
}

int main(int argc, char *argv[])
{
    int serverSock;				        /* Server Socket */
    client client_socket[3];			/* Array of 3 client Sockets*/
    int max_client = 3;                 //max number of client
    int new_socket;                     //holds new client socket
    int max_sd;                         //holds max socket descriptor
    int sd;                             //holds any arbitrary socket descriptor
    struct sockaddr_in servAddr;		/* Local address */
    struct sockaddr_in clntAddr;		/* Client address */
    unsigned short servPort;		    /* Server port */
    unsigned int clntLen = sizeof(clntAddr);			/* Length of address data struct */
    ifstream file;                      //file object
    const char* fileName;               //text file name
    char buffer[BUFSIZE];			    /* Buff to store information from client */			
    fd_set readfds;                     //set of socket descriptors
    int opt = true;

    //check number of arguments
    if (argc > 3 || argc < 2) {
        perror("Incorrect number of arguments");
        exit(EXIT_FAILURE);
    } else {
        servPort = atoi(argv[1]); //assign server port
        if (argc == 3) {
            //assign text file name
            fileName = argv[2];
        }
    }
    
    //initialize client_socket[]
    for (int i = 0; i < max_client; i++) {
        resetClient(&client_socket[i]);
    }

    //create new TCP main socket for incoming requests
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fail to create a new socket");
        exit(EXIT_FAILURE);
    }

    //set main socket to allow multiple connections
    if( setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    {   
        perror("setsockopt() fails");   
        exit(EXIT_FAILURE);   
    }   

    //Construct local address structure
    memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(servPort);

    //Bind to local address structure
    if (bind(serverSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
		perror("Fail to bind"); 
        exit(EXIT_FAILURE); 
	}

    //Listen for incoming connections
    if (listen(serverSock, 3) < 0) {
		perror("Fail to listen"); 
        exit(EXIT_FAILURE); 
	}

    //Accept incoming connection
    //Print out connection status
    char clntName[INET_ADDRSTRLEN];

    while(true) {
        FD_ZERO(&readfds); //clear socket set
        FD_SET(serverSock, &readfds); //add main socket to the set
        max_sd = serverSock;

        //add child sockets to set
        for (int i = 0; i < max_client; i++) {
            sd = client_socket[i].sd;
            if (sd > 0) 
                FD_SET(sd, &readfds);
            if (sd > max_sd) //keep track of max descriptor number
                max_sd = sd;
        }

        //wait for an activity on one of the sockets
        int act = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((act < 0) && (errno!=EINTR)) 
            cout<<"fail to select."<<endl;

        //if something happen to the main socket, perceive as new connection
        if (FD_ISSET(serverSock, &readfds)) {
            if ((new_socket = accept(serverSock,(struct sockaddr*) &clntAddr, &clntLen)) < 0) {
                perror("Fail to accept incoming connection"); 
                exit(EXIT_FAILURE); 
            }
            bool empty = false;
            //add new socket to array of client sockets
            for (int i = 0; i <max_client; i++) {
                if (client_socket[i].sd == 0) {
                    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL)
                    printf("Get connected from client IP: %s , port: %d\n", clntName, ntohs(clntAddr.sin_port));
                else
                    puts("Get connected. Fail to get client address.");
                    client_socket[i].sd = new_socket;
                    empty = true;
                    //Send "Ready to connect" message
                    char msg[] = "xReady to start game? (y/n):";
                    msg[0] = strlen(msg)-1; //input Msg Flag
                    sendToClient(new_socket, msg, strlen(msg));
                    break;
                }
            }
            if (!empty) {
                //server overload, send "server-overloaded" message
                char msg[] = "xServer-overloaded.";
                msg[0] = strlen(msg)-1; //input Msg Flag
                sendToClient(new_socket, msg, strlen(msg));
            }
        }
        //if nothing happen to main socket, then it's a message from a client
        for (int i = 0; i < max_client; i++) {
            sd = client_socket[i].sd;
            if (FD_ISSET(sd, &readfds)) { //for the socket that receives a message
                //read from client
                memset(&buffer, 0, BUFSIZE);
                if (readFromClient(sd, buffer) == 0) {
                    //client disconnected--print connection status
                    getpeername(sd, (struct sockaddr*) &clntAddr , (socklen_t*)&clntLen);
                    printf("End the connection from client IP: %s , port: %d\n",inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port));
                    //close and reset the socket data
                    close(sd);
                    resetClient(&client_socket[i]);
                } else {
                    //process its message
                    if (client_socket[i].wordLen == 0) { 
                        //answer has never been assigned--packet is a start game message
                        //find an answer for this client
                        if (buffer[1] == 'n' || buffer[1] == 'N') {
                            //close the connection
                            getpeername(sd, (struct sockaddr*) &clntAddr , (socklen_t*)&clntLen);
                            printf("End the connection from client IP: %s , port: %d\n",inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port));
                            close(sd);
                            client_socket[i].sd = 0;
                            continue;
                        }
                        else {
                            //find the word
                            srand((unsigned) time(0)*1000);
                            if (argc == 2) {
                                //choose from default dictionary
                                client_socket[i].answer = pickAWord(rand());
                                client_socket[i].wordLen = client_socket[i].answer.size();
                            } else {
                                //choose from file
                                file.open(fileName);
                                if (!file) {
                                    perror("unable to open file.");
                                }
                                string input;
                                getline(file, input, ' '); //read the word length
                                client_socket[i].wordLen = stoi(input);
                                getline(file, input); //read the number of words

                                if (buffer[1] == 'y' || buffer[1] == 'Y') { 
                                    //pick a random word
                                    int random = rand()%stoi(input); //random = 0 to n-1
                                    for (int i = -1; i < random; i++) {
                                        file>>input; //get the word specified by the input number
                                    }
                                } else { 
                                    //find the specified word
                                    for (int i = 0; i < buffer[1]; i++) {
                                        file>>input; //get the word specified by the input number
                                    }
                                }
                                client_socket[i].answer = input;
                                file.close();
                            }
                            client_socket[i].guess = string(client_socket[i].wordLen,'_');
                        }
                    } else { //packet is a letter guessing message
                        bool correct = false;               //the letter being guessed is correct/inccorrect
                        char newGuess = buffer[1];
                        //check newGuess with the correct answer
                        for (int j = 0; j < client_socket[i].wordLen; j++) {
                            if (newGuess == client_socket[i].answer[j]) { //if answer is correct, update the "guess" list
                                correct = true;
                                client_socket[i].guess[j] = newGuess;
                            }
                        }
                        if(!correct) { //if the guess is not correct, update the "incorrect" list and increment incorrectNum
                            client_socket[i].incorrect.push_back(newGuess);
                            client_socket[i].incorrectNum++;
                        }   

                        //check if the game should end
                        if (client_socket[i].incorrectNum == 6) { //lose
                            char msg[] = "xYou Lose! : ";
                            strcat(msg, client_socket[i].answer.c_str());
                            msg[0] = strlen(msg)-1; //input Msg Flag
                            sendToClient(sd, msg, strlen(msg));
                        } else if (client_socket[i].answer.compare(client_socket[i].guess) == 0) { //win--"send you win message"
                            char msg[] = "xYou win! : ";
                            strcat(msg, client_socket[i].answer.c_str());
                            msg[0] = strlen(msg)-1; //input Msg Flag
                            sendToClient(sd, msg, strlen(msg));
                        } 
                    }
                    //generate game control message packet after every turn
                    buffer[0] = 0;
                    buffer[1] = client_socket[i].wordLen;
                    buffer[2] = client_socket[i].incorrectNum;
                    for (int j = 0; j < client_socket[i].wordLen; j++) {
                        buffer[3+j] = client_socket[i].guess[j];
                    }
                    for (int j = 0; j < client_socket[i].incorrectNum; j++) {
                        buffer[3+client_socket[i].wordLen+j] = client_socket[i].incorrect[j];
                    }
                    //send message
                    sendToClient(sd, buffer, 3+client_socket[i].wordLen+client_socket[i].incorrectNum);
                }
            }
        } //end client for loop
    } //end while loop
}