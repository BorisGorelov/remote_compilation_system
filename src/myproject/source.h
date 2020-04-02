#ifndef SOURCE_H
#define SOURCE_H
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
#define MYEOF "^^^^^"
extern int safe_read(int sockfd, char* ans, long size);

typedef enum {
    RCC_NO_FILE = 1,    // No such file or directory 
    RCC_WRONG_ARG,
    RCC_SOCK_ERROR,
    RCC_INETPTON_ERROR,
    RCC_CONNECTION_ERROR,
    RCC_UNEXPEC_VAL,
    RCC_SEND_ERROR,
    RCC_RECEIVE_ERROR,
    RCC_BIND_ERROR,
    RCC_LISTEN_ERROR,
    RCC_ACCEPT_ERROR,
    RCC_COMPILE_ERROR,
    RCC_SERVER_ERROR,
    RCC_AUTH_ERROR
} rcc_errno_codes;

#endif
