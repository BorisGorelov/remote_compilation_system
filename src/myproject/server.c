#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdbool.h> 
#define LEN 80 
#define FLEN 256
#define PORT 1234  


int get_name(int sockfd, char* serv_name)
{
    char client_name[LEN];
    if(read(sockfd, client_name, LEN))
        write(sockfd, "the name has been recieved\n", sizeof("the name has been recieved\n"));
    else
    {
        write(sockfd, "error while recieving data\n", sizeof("error while recieving data\n"));
        fputs("error while recieving data\n", stderr);
        return 1;
    }
    printf("From client: name of file: %s\n", client_name);

    strncat(serv_name, client_name, (LEN - 6));
    return 0;
}

int get_file(int sockfd, char* serv_name)
{
    FILE* file_ptr = fopen(serv_name, "w"); 
    char fbuff[FLEN];

    if(file_ptr == NULL)
    {
        fputs("unable to open the file\n", stderr);
        return 1;
    }
    printf("file was opened successfully\n");

    //recieving file
    read(sockfd, fbuff, sizeof(fbuff));
    while(strncmp("^^^^^", fbuff, 5))
    {
        fputs(fbuff, file_ptr);
        read(sockfd, fbuff, sizeof(fbuff));
    }
    printf("file was recieved\n");
    fclose(file_ptr);
    return 0;
}

// int get_flags()
// {

// }

void compile(int sockfd, char* serv_name)
{
    int status;
    char ans[FLEN];
    char command[FLEN] = {"gcc -o out "};
    strcat(command, serv_name);

    //compile and send result
    status = system(command);
    printf("status of compiling: %d\n", status);
    sprintf(ans, "status of compiling: %d\n", status);
    write(sockfd, ans, sizeof(ans));
}
  
int main() 
{ 
    int sockfd, connfd; 
    struct sockaddr_in servaddr; 
    char serv_name[FLEN] = "serv_";
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n");  
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");; 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if (listen(sockfd, 5)) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
  
    // Accept the data packet from client and verification
    connfd = accept(sockfd, NULL, NULL); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 
  
    if(get_name(connfd, serv_name) != 0)
        return 1;

    if(get_file(connfd, serv_name) != 0)
        return 1;

    compile(connfd, serv_name);

    close(sockfd); 
} 
