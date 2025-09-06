#!/bin/bash
set -e

# Compile main
gcc -o build/apiserver src/main/main.c src/header/socket.c
