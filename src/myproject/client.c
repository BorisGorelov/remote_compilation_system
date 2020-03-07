#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <stdbool.h>
#define LEN 70 
#define FLEN 256
#define PORT 1234  

void get_answer(int sockfd)
{
    char ans[FLEN]; 
    read(sockfd, ans, sizeof(ans)); 
    printf("From Server: %s\n", ans);
}

int send_file(int sockfd, char* name)
{
    FILE* file_ptr = fopen(name, "r");
    char fbuff[FLEN];

    if(file_ptr == NULL)
    {
        fputs("unable to open the file\n", stderr);
        return 1;
    }

    while(fgets(fbuff, sizeof(fbuff), file_ptr))
        write(sockfd, fbuff, sizeof(fbuff));
        
    write(sockfd, "^^^^^", sizeof("^^^^^"));
    fclose(file_ptr);

    printf("File has been sent\n");
    get_answer(sockfd);

    return 0;
}

// int send_flags()
// {

// }

int send_name(int sockfd, char* name)
{
    char header[LEN]; 
    int i = 0;
    int count = 0;

    printf("Enter the string like: === File <No>. <filename>\n");
    fgets(header, sizeof(header), stdin);
    header[strcspn(header, "\n")] = '\0';

    //put <filename> in name
    while(i < strlen(header))
    {
        if (header[i] == ' ')
        {
            bzero(name, LEN);
            count = 0;
            i++;
            continue;
        }
        name[count] = header[i];
        i++;
        count++;
    }

    //send name to server
    if(strlen(name) == 0)
    {
        printf("wrong header\n");
        return 1;
    }
    
    write(sockfd, name, LEN);
    get_answer(sockfd);
    return 0;
}
  
int main() 
{ 
    int sockfd; 
    struct sockaddr_in servaddr; 
    char file_name[LEN];
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
    { 
        printf("socket creation failed...\n"); 
        return 1;
    } 
    else
        printf("Socket successfully created, socket = %d\n", sockfd); 
  

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) 
    { 
        printf("connection with the server failed...\n"); 
        return 2;
    } 
    else
        printf("connected to the server..\n"); 
   
    if (send_name(sockfd, file_name) != 0) 
        return 1;

    if (send_file(sockfd, file_name) != 0)
        return 1;

    close(sockfd); 

    return 0;
} 
