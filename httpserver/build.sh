#!/bin/bash
set -e

# Compile main
gcc -o build/httpserver src/main/main.c src/header/socket.c
