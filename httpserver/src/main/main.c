// main.c - Main program
#include "../header/socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // Args fail
  if (argc < 2) {
    fprintf(stderr, "Usage: ./httpserver <Port> <Configuration file>\n");
    exit(1);
  }

  // Extract arguments
  int port = atoi(argv[1]);
  char *configFilename = argv[2];

  // Create listening server socket
  int serverSocket = createServerSocket(port);
  if (serverSocket == -1) {
    fprintf(stderr, "Error creating server socket\n");
    exit(1);
  }
  printf("[*] HTTP server listening on port %d\n", port);

  // Accept a client connection
  int clientSocket = acceptClientSocket(serverSocket);

  // Send data
  sendData(clientSocket, "Noble Server\r\n", strlen("Noble Server\r\n"));
  // Receive response
  char buffer[256];
  receiveData(clientSocket, buffer, sizeof(buffer));
  printf("%s\n", buffer);

  close(clientSocket);

  return 0;
}
