
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

 
void compile(int sockfd)
{
    //get name of file
    char client_name[LEN]; 
    int n;
    if(read(sockfd, client_name, sizeof(client_name)))
        write(sockfd, "the name has been recieved\n", sizeof("the name has been recieved\n"));
    else
    {
        write(sockfd, "error while recieving data\n", sizeof("error while recieving data\n"));
        exit(0);
    }
    printf("From client: name of file: %s\n", client_name); 

    //create new file and write recieved information 
    char serv_name[FLEN] = "serv_";
    strcat(serv_name, client_name);
    FILE* file_ptr = fopen(serv_name, "w"); 
    if(file_ptr == NULL)
    {
        fputs("unable to open the file\n", stderr);
        exit(1);
    }
    printf("file opened successfully\n");

    //recieving file
    char fbuff[FLEN];
    read(sockfd, fbuff, sizeof(fbuff));
    while(strncmp("^^^^^", fbuff, 5))
    {
        fputs(fbuff, file_ptr);
        //fputs(fbuff, stdout);
        read(sockfd, fbuff, sizeof(fbuff));
    }
    printf("file has been recieved\n");
    fclose(file_ptr);

    //forming an executable command
    char command[LEN] = {"gcc -o out "};
    strcat(command, serv_name);
    //printf("%s\n", command);

    //compile and send result
    int status = system(command);
    char ans[256];

    printf("status of compiling: %d\n", status);
    sprintf(ans, "status of compiling: %d\n", status);
    write(sockfd, ans, sizeof(ans));
}
  
int main() 
{ 
    int sockfd, connfd; 
    struct sockaddr_in servaddr; 
  
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
  
    compile(connfd); 

    close(sockfd); 
} 
