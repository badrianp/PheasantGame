# Pheasant Game

## Introduction

Playing and training the mind is a combination of activities merged perfectly with a small amount of _funny_
as long as the house rules stays at home and you stick with the **SERVER's** rules as they are. No "no kick-outs in the first round" rule
has ever been written to the gameLogic() so you better _aim_ fast.

## Requirements

This application requires no other libraries except the C Standard library and a little bit of the POSIX system operations

## Interaction

### Compilation

To compile the application you have to install the **[GCC]**(https://gcc.gnu.org/install/) compiler first.
Open a Terminal and navigate to the application's folder location.

To compile all the project files type:
`make`
You can compile separately the server, respectively the client, with the commands:
`make server`
`make client`

To open this README file in default editor hit:
`make help`

### Run

You can _run_ a server with the command:
`./server`
and a client with the command:
`./client`

To start a game one has to _run_ the server and for each one player a corresponding client run has to be done.
Players choosing the same room name will play together. You can open multiple game rooms on the same server.
The room creator (the first to type the room name) decides when to start the session. (y/n)
Further information will be shown on the apps' interface.

If anything unexpected happens during the game please restart the application using
the same methods (make->run->HaveFun) and feel free to send a detailed
report through any contact method you'd like.

Executable files can be deleted with:
`make clean`

## Bibliography

The most usefull resources found on the web helped me on:

[network programming](https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/)

[writing my first README](https://www.ionos.com/digitalguide/websites/web-development/markdown/)

[multithreading](https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)

[some other spontaneously found material](https://gitlab.com/DavidGriffith/frotz/-/issues/125)

[the most wanted stack](https://stackoverflow.com/questions/5242524/converting-int-to-string-in-c)
