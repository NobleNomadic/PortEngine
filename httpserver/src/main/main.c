// main.c - Main program
#include "../header/socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void generateResponse(char *requestBuffer, char *response) {
  // Get the method
  char *HTTPMethod = strtok(requestBuffer, " ");
  if (HTTPMethod && strcmp(HTTPMethod, "GET") == 0) {
    // Example response for GET request
    sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Hello, world!");
  } else {
    // Handle unsupported methods
    sprintf(response,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Unsupported Method");
  }

  return;
}

void serverLoop(int serverSocketFD) {
  // Get a client socket
  int clientSocketFD = acceptClientSocket(serverSocketFD);

  // Verify socket
  if (clientSocketFD == -1) {
    return;
  }

  // Get request into buffer
  char requestBuffer[1024];
  receiveData(clientSocketFD, requestBuffer, sizeof(requestBuffer));

  printf("[*] Received request\n%s", requestBuffer);

  // Process request and generate response
  char response[1024];
  generateResponse(requestBuffer, response);

  // Send the response to the socket
  sendData(clientSocketFD, response, strlen(response));

  // Close client socket
  close(clientSocketFD);

  return;
}

int main(int argc, char **argv) {
  // Args fail
  if (argc < 2) {
    fprintf(stderr, "Usage: ./httpserver <Port>\n");
    exit(1);
  }

  // Extract arguments
  int port = atoi(argv[1]);

  // Create listening server socket
  int serverSocket = createServerSocket(port);
  if (serverSocket == -1) {
    fprintf(stderr, "Error creating server socket\n");
    exit(1);
  }
  printf("[*] HTTP server listening on port %d\n", port);

  while (1) {
    serverLoop(serverSocket);
  }

  return 0;
}
