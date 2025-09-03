#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int createServerSocket(int port) {
  int serverFD;
  struct sockaddr_in serverAddr;

  serverFD = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFD < 0) {
    perror("socket");
    return -1;
  }

  int opt = 1;
  if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    close(serverFD);
    return -1;
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);

  if (bind(serverFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind");
    close(serverFD);
    return -1;
  }

  if (listen(serverFD, 5) < 0) {
    perror("listen");
    close(serverFD);
    return -1;
  }

  return serverFD;
}

int acceptClientSocket(int serverSocketFD) {
  struct sockaddr_in clientAddr;
  socklen_t clientLen = sizeof(clientAddr);

  int clientFD = accept(serverSocketFD, (struct sockaddr *)&clientAddr, &clientLen);
  if (clientFD < 0) {
    perror("accept");
    return -1;
  }

  printf("Accepted connection from %s:%d\n",
        inet_ntoa(clientAddr.sin_addr),
        ntohs(clientAddr.sin_port));

  return clientFD;
}

// Send data over socket
int sendData(int socketFD, const char *data, size_t dataLen) {
  ssize_t bytesSent = send(socketFD, data, dataLen, 0);
  if (bytesSent < 0) {
    perror("sendData: Error sending data");
    return -1;
  }
  return (int)bytesSent;
}

// Receive data from socket
int receiveData(int socketFD, char *buffer, size_t bufferSize) {
  ssize_t bytesReceived = recv(socketFD, buffer, bufferSize - 1, 0); // Leave space for null terminator
  if (bytesReceived < 0) {
    perror("receiveData: Error receiving data");
    return -1;
  }

  buffer[bytesReceived] = '\0'; // Null-terminate for safe string use
  return (int)bytesReceived;
}

