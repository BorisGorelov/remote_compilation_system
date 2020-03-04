#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <stdbool.h>
#define LEN 70 
#define FLEN 256
#define PORT 1234  


void send_file(int sockfd)
{
    //send header
    char buff[LEN]; 

    printf("Enter the string like: === File <No>. <filename>\n");
    fgets(buff, sizeof(buff), stdin);
    buff[strcspn(buff, "\n")] = '\0'; //fgets write \n to buff, due to strcspn change it to \0

    //put <filename> in name
    char name[LEN];
    int i = 0;
    int count = 0;
    while(i < strlen(buff))
    {
        if (buff[i] == ' ')
        {
            bzero(name, sizeof(name));
            count = 0;
            i++;
            continue;
        }
        name[count] = buff[i];
        i++;
        count++;
    }

    //send name to server
    if(strlen(name))
        write(sockfd, name, sizeof(name)); 
    else
    {
        printf("wrong header\n");
        exit(1);
    }

    //answer about file creation
    char ans[256]; 
    read(sockfd, ans, sizeof(ans)); 
    printf("From Server: %s\n", ans);
     
    //send file
    FILE* file_ptr = fopen(name, "r");
    if(file_ptr == NULL)
    {
        fputs("unable to open the file\n", stderr);
        exit(1);
    }

    char fbuff[FLEN];

    while(fgets(fbuff, sizeof(fbuff), file_ptr))
    {
        write(sockfd, fbuff, sizeof(fbuff));
        //fputs(fbuff, stdout);
    }
        
    write(sockfd, "^^^^^", sizeof("^^^^^"));
    fclose(file_ptr);
    printf("file has been sent\n");

    //file have been already sent, waiting for response
    bzero(buff, sizeof(buff)); 
    read(sockfd, buff, sizeof(buff)); 
    printf("From Server: %s\n", buff);

}
  
int main() 
{ 
    int sockfd, connfd; 

    // describes socket for working with IP protocol
    struct sockaddr_in servaddr, cli; 
  
    // socket create and varification 
    // af_inet => ipv4
    // SOCK_STREAM  => Provides  sequenced,  reliable,  two-way, 
    // connection-based byte streams.
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created, socket = %d\n", sockfd); 
    //bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 

    //IPv4 numbers-and-dots notation into  binary  data  in network  byte  order
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    //from host byte order to network
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
   
    send_file(sockfd); 
  
    close(sockfd); 
} 
