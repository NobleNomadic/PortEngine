// main.c - Main program
#include "../header/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

// Loop for API server to redistribute data
void APIServerLoop(int serverSocketFD) {
  while (1) {
    fflush(stdout);
  }
}

// Loop for receiving data from uploader (runs in child process)
void writerServerLoop(int serverSocketFD) {
  printf("[WRITER] Writer process started (PID: %d)\n", getpid());
  fflush(stdout);

  // Main loop
  while (1) {
    printf("[WRITER] Waiting for upload client...\n");
    fflush(stdout);
    int clientSocketFD = acceptClientSocket(serverSocketFD);
    if (clientSocketFD == -1) {
      perror("[WRITER] acceptClientSocket failed");
      continue;
    }

    char buffer[1024];
    receiveData(clientSocketFD, buffer, sizeof(buffer));
    printf("[WRITER] Received data:\n%s\n", buffer);
    fflush(stdout);

    // TODO: Parse and write to DB
    close(clientSocketFD);
  }
}

int main(int argc, char **argv) {
  // Unbuffered stdout for immediate log visibility
  setvbuf(stdout, NULL, _IONBF, 0);

  if (argc < 2) {
    fprintf(stderr, "Usage: ./apiserver <Port>\n");
    exit(1);
  }

  int port = atoi(argv[1]);
  int dataInPort = port + 1;

  // ==== WRITER SERVER SETUP ====
  int writerServerSocketFD = createServerSocket(dataInPort);
  if (writerServerSocketFD == -1) {
    fprintf(stderr, "Error creating writer server socket\n");
    exit(1);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    // CHILD PROCESS
    printf("[WRITER] Writer server listening on port %d\n", dataInPort);
    fflush(stdout);
    writerServerLoop(writerServerSocketFD);
    close(writerServerSocketFD);
    exit(0);
  }

  // ==== API SERVER ====
  int serverSocket = createServerSocket(port);
  if (serverSocket == -1) {
    fprintf(stderr, "Error creating API server socket\n");
    exit(1);
  }

  printf("[API] API server listening on port %d\n", port);
  fflush(stdout);

  APIServerLoop(serverSocket);

  return 0;
}

