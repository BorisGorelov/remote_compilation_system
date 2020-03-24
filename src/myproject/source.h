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
#include <ctype.h>
#include <getopt.h>
#include <stdint.h>
#define LEN 80 
#define FLEN 256
#define LISTEN_BACKLOG 50
#define RCC_PORT_DEFAULT 1234
#define RCC_IP_DEFAULT "127.0.0.1"

int safe_answer(int sockfd, char* ans, long size);
int safe_read(int sockfd, char* ans, long size);

typedef enum {
    RCC_NO_FILE = 1,    // No such file or directory 
    RCC_WRONG_ARG,
    RCC_WRONG_PORT,
    RCC_WRONG_IP,
    RCC_SOCK_FAIL,
    RCC_INETPTON_ERROR,
    RCC_CONNECTION_FAIL,
    RCC_UNEXPEC_VAL,
    RCC_SEND_FAIL,
    RCC_RECEIVE_FAIL,
    RCC_PRINT_FAIL,
    RCC_BIND_FAIL,
    RCC_LISTEN_FAIL,
    RCC_ACCEPT_FAIL,
    RCC_COMPILE_ERROR
} rcc_errno_codes;