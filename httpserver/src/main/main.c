// main.c - Main program
#include "../header/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

// Security function to validate file paths
int isPathSecure(const char *requestedPath, char *safePath, size_t safePathSize) {
    // Check for obvious directory traversal attempts
    if (strstr(requestedPath, "..") != NULL) {
        printf("[!] Directory traversal attempt blocked: %s\n", requestedPath);
        return 0;
    }
    
    // Check for attempts to escape www directory
    if (strncmp(requestedPath, "www/", 4) != 0 && strcmp(requestedPath, "www") != 0) {
        printf("[!] Path outside www directory blocked: %s\n", requestedPath);
        return 0;
    }
    
    // Check for null bytes (can be used to bypass checks)
    if (strlen(requestedPath) != strcspn(requestedPath, "\0")) {
        printf("[!] Null byte in path blocked\n");
        return 0;
    }
    
    // Check for other dangerous sequences
    if (strstr(requestedPath, "//") != NULL || 
        strstr(requestedPath, "./") != NULL ||
        strstr(requestedPath, "~") != NULL) {
        printf("[!] Suspicious path blocked: %s\n", requestedPath);
        return 0;
    }
    
    // Path looks safe, copy it
    strncpy(safePath, requestedPath, safePathSize - 1);
    safePath[safePathSize - 1] = '\0';
    
    printf("[DEBUG] Path validated: %s\n", safePath);
    return 1;
}

void generateResponse(char *requestBuffer, char *response) {
    // Copy requestBuffer because strtok modifies it
    char requestCopy[1024];
    strncpy(requestCopy, requestBuffer, sizeof(requestCopy) - 1);
    requestCopy[sizeof(requestCopy) - 1] = '\0';
    
    // Get the method
    char *HTTPMethod = strtok(requestCopy, " ");
    // Get the path
    char *HTTPPath = strtok(NULL, " ");
    
    // Handle NULL path or root path "/"
    if (!HTTPPath || strcmp(HTTPPath, "/") == 0) {
        HTTPPath = "www/index.html";
    } else if (HTTPPath[0] == '/') {
        HTTPPath++;  // remove leading '/'
        // Prepend www/ if not already there
        if (strncmp(HTTPPath, "www/", 4) != 0) {
            static char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "www/%s", HTTPPath);
            HTTPPath = fullPath;
        }
    }
    
    // Security validation
    char safePath[512];
    if (!isPathSecure(HTTPPath, safePath, sizeof(safePath))) {
        sprintf(response,
                "HTTP/1.1 403 Forbidden\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "403 Forbidden: Access denied");
        return;
    }
    
    if (HTTPMethod && strcmp(HTTPMethod, "GET") == 0) {
        FILE *file = fopen(safePath, "r");
        if (!file) {
            sprintf(response,
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n"
                    "File not found");
            return;
        }
        
        // pick content type based on extension
        const char *ext = strrchr(safePath, '.');
        const char *contentType = "application/octet-stream";
        if (ext) {
            if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
                contentType = "text/html";
            } else if (strcmp(ext, ".css") == 0) {
                contentType = "text/css";
            } else if (strcmp(ext, ".js") == 0) {
                contentType = "application/javascript";
            } else if (strcmp(ext, ".png") == 0) {
                contentType = "image/png";
            } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
                contentType = "image/jpeg";
            } else if (strcmp(ext, ".gif") == 0) {
                contentType = "image/gif";
            } else if (strcmp(ext, ".ico") == 0) {
                contentType = "image/x-icon";
            } else if (strcmp(ext, ".txt") == 0) {
                contentType = "text/plain";
            } else if (strcmp(ext, ".json") == 0) {
                contentType = "application/json";
            }
        }
        
        // Create the response with the file content
        sprintf(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "\r\n",
                contentType);
                
        char lineBuffer[1024];
        size_t responseLen = strlen(response);
        while (fgets(lineBuffer, sizeof(lineBuffer), file) != NULL) {
            size_t lineLen = strlen(lineBuffer);
            if (responseLen + lineLen < 1024) {
                strcat(response, lineBuffer);
                responseLen += lineLen;
            } else {
                break;
            }
        }
        fclose(file);
        
        printf("[*] Served file: %s\n", safePath);
        
    } else {
        sprintf(response,
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Unsupported Method");
    }
    
    printf("[*] Sending response:\n%s", response);
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
    printf("[*] Serving files from www/ directory only\n");
    
    while (1) {
        serverLoop(serverSocket);
    }
    
    return 0;
}
