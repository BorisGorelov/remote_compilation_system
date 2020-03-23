#pragma once
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdbool.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <locale.h>
#include <sys/stat.h>

#define LEN 80 
#define FLEN 256
#define LISTEN_BACKLOG 50
int safe_answer(int sockfd, char* ans, long size);
int safe_read(int sockfd, char* ans, long size);