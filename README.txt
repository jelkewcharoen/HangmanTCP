This project is done only by me (Panalee Kewcharoen) and is written in C++
The program should run on Linux environment (Ubuntu)
Note: The default dictionary is written in the server.cpp.


How to compile:
Run make on the command line. This should generate "client" and "server" executable files.

----------------------------------------------------------------------------------------------------------------------------
Implementation idea:

1. server

The server uses select() to handle multilple connections. It runs in a loop and keeps checking if there has been any activity on 
the main socket (meaning that there has been a new connection attemped), or the child socket (meaning that the client has sent a message to the server.)
However, if it already has 3 clients, it will send a message to the 4th client that it's full and the client will quit its connection.

It stores information about the clients in an array of structure called "client." Each "client" contains the socket descriptor, the answer for that client, 
the word length of the answer, the string of guessed letters, the string of incorrect letters, and the number of incorrect guesses.

Once it receives a packet, it first determines whether it is from a client that has ended its connection. If yes, it will reset the information of this client
to open a spot for a new client. 

After that, it determines whether this client has been assigned a word before. If not, then this is a start game packet from a new client
-if the message is 'n'/'N', then close the connection and reset the client information
-if the message is 'y'/'Y'/ or a number, then it will pick a random word or find the specified word as requested by the client. The default dictionary
is hardcoded in the program. There are 5 words listed in the function "pickAWord".
If this client already has a word assigned, then this is a general game packet. The data in the packet is a guessed letter.
Then it will check to see if the letter is correct, and then update the string of correct letters and incorrect letters accordingly. 
Note that client takes care of preventing a repeating letter and other errors.

If the number of incorrect letters has reached 6, the client has lost. 
If the string of correct letters equals the assigned answer, the client wins
The server will send a message accordingly.

There are 4 functions in this code.
1.resetClient() : reset the given structure "client" to default
2.pickAWord(): pick a random word from default dictionary
3.sendToClient(): send message to the given client
4.readFromClient(): read message from the given client
	
------------------------------------------------------------------------------------
2. client

The client first start the connection to the client. The first packet it receives will be a "ready to start" packet. 
Otherwise, if the message starts with 'S' indicating "server-overloaded" then it will quit the connection.
If not, it will read input from the user and send to the server.

After that it will enter a loop that starts with reading a packet from the client.
If the packet has length 0, the packet is a game control packet. It will print out the information about the guessed letters.
After that, it will read the input from the user and checks if it is in the correct format before sending it to the server.

If the packet has length of something other than 0, that means it is a message packet. It prints out the message and indicates
whether the message starts with 'Y' (meaning that it is either 'You win' or 'You lose'). If it is then the game is over and
the client closes the connection.

the client has 2 functions
1.sendToServer(): send message to the server
2.readFromServer(): read message from the server
----------------------------------------------------------------------------------------------------------------------------

Test result: Here I have given 6 different scenarios

1) Client wins

server:
$ ./server 2017
Get connected from client IP: 127.0.0.1 , port: 57096
End the connection from client IP: 127.0.0.1 , port: 57096

client:
$ ./client 127.0.0.1 2017
Ready to start game? (y/n): y

_ _ _ _
Incorrect Guesses:

Letter to guess: t

t _ _ _
Incorrect Guesses:

Letter to guess: t

Error: letter t has already been guessed. Please guess another letter.

Letter to guess: e

t e _ _
Incorrect Guesses:

Letter to guess: c

t e c _
Incorrect Guesses:

Letter to guess: h
You win! : tech
Game over.

------------------------------------------------------------------------------
2) Client loses

server:
$ ./server 2017
Get connected from client IP: 127.0.0.1 , port: 57121
End the connection from client IP: 127.0.0.1 , port: 57121

client:
$ ./client 127.0.0.1 2017
Ready to start game? (y/n): y

_ _ _ _ _ _ _
Incorrect Guesses:

Letter to guess: t

_ _ _ _ _ _ _
Incorrect Guesses: t

Letter to guess: g

g _ _ _ g _ _
Incorrect Guesses: t

Letter to guess: e

g e _ _ g _ _
Incorrect Guesses: t

Letter to guess: w

g e _ _ g _ _
Incorrect Guesses: t w

Letter to guess: q

g e _ _ g _ _
Incorrect Guesses: t w q

Letter to guess: a

g e _ _ g _ a
Incorrect Guesses: t w q

Letter to guess: z

g e _ _ g _ a
Incorrect Guesses: t w q z

Letter to guess: c

g e _ _ g _ a
Incorrect Guesses: t w q z c

Letter to guess: v
You Lose! : georgia
Game over.
-----------------------------------------------------------------------------
3). Some other errors

server:
$ ./server 2017
Get connected from client IP: 127.0.0.1 , port: 57127
End the connection from client IP: 127.0.0.1 , port: 57127

client:
$ ./client 127.0.0.1 2017
Ready to start game? (y/n): y

_ _ _ _ _ _ _
Incorrect Guesses:

Letter to guess: aaaaa

Error: please guess only ONE letter.

Letter to guess: 4

Error: please guess a LETTER.

Letter to guess: g

g _ _ _ g _ _
Incorrect Guesses:

Letter to guess: e

g e _ _ g _ _
Incorrect Guesses:

Letter to guess: o

g e o _ g _ _
Incorrect Guesses:

Letter to guess: r

g e o r g _ _
Incorrect Guesses:

Letter to guess: g

Error: letter g has already been guessed. Please guess another letter.

Letter to guess: i

g e o r g i _
Incorrect Guesses:

Letter to guess: a
You win! : georgia
Game over.
-----------------------------------------------------------------------------------------------
4). using textfile & backdoor

server:
$ ./server 2017 words.txt
Get connected from client IP: 127.0.0.1 , port: 57131
End the connection from client IP: 127.0.0.1 , port: 57131

client:
$ ./client 127.0.0.1 2017
Ready to start game? (y/n): 10

_ _ _ _
Incorrect Guesses:

Letter to guess: q

q _ _ _
Incorrect Guesses:

Letter to guess: u

q u _ _
Incorrect Guesses:

Letter to guess: i

q u i _
Incorrect Guesses:

Letter to guess: z
You win! : quiz
Game over.
----------------------------------------------------------------------------------------
5).using text file & no backdoor

server:
$ ./server 2017 words.txt
Get connected from client IP: 127.0.0.1 , port: 57136
End the connection from client IP: 127.0.0.1 , port: 57136

client:
$ ./client 127.0.0.1 2017
Ready to start game? (y/n): y

_ _ _ _
Incorrect Guesses:

Letter to guess: u

_ _ _ _
Incorrect Guesses: u

Letter to guess: i

_ i _ _
Incorrect Guesses: u

Letter to guess: z

_ i z z
Incorrect Guesses: u

Letter to guess: f
You win! : fizz
Game over.
---------------------------------------------------------------------------------------------------
6). Multiple connections & client choosing 'n'
what happens here is that 3 clients connect, and the 4th client attemps to connect but gets rejected. Then after one of the 3 clients (client3) quits its connection, the 4th client can connect. 
server: 
$./server 2017 words.txt
Get connected from client IP: 127.0.0.1 , port: 57137
Get connected from client IP: 127.0.0.1 , port: 57148
Get connected from client IP: 127.0.0.1 , port: 57154
End the connection from client IP: 127.0.0.1 , port: 57154
Get connected from client IP: 127.0.0.1 , port: 57160

client1:
$./client 127.0.0.1 2017
Ready to start game? (y/n):

client2:
$./client 127.0.0.1 2017
Ready to start game? (y/n):                  

client3:
$ ./client 127.0.0.1 2017
Ready to start game? (y/n): n
Quitting the connection.

client4:
$ ./client 127.0.0.1 2017
Server-overloaded.
Quitting the connection.
$ ./client 127.0.0.1 2017
Ready to start game? (y/n):

              




































