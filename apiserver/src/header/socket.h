// socket.h - Header for socket handling
#ifndef SOCKET_H
#define SOCKET_H

#include <stddef.h>

int createServerSocket(int port); // Return file descriptor for a server socket
int acceptClientSocket(int serverSocketFD); // Accept and return a client socket using a server socket FD

int sendData(int socketFD, const char *data, size_t dataLen); // Send data to a socket
int receiveData(int socketFD, char *buffer, size_t bufferSize); // Receive data over a socket into a buffer

#endif
