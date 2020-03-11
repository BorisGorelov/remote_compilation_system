//usage: ./server [-p <port>]
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
#define LEN 80 
#define FLEN 256
#define LISTEN_BACKLOG 50

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
        write(sockfd, "unable to open the file\n", sizeof("unable to open the file\n"));
        return 1;
    }
    printf("file was opened successfully\n");

    //recieving file
    read(sockfd, fbuff, sizeof(fbuff));
    while(strncmp("^^^^^", fbuff, 5) != 0)
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

void get_answer(int sockfd)
{
    char ans[FLEN]; 
    read(sockfd, ans, sizeof(ans)); 
    printf("From Client: %s\n", ans);
}

int send_errors(int sockfd)
{
    FILE* file_ptr = fopen("errors", "r");
    char fbuff[FLEN];

    if(file_ptr == NULL)
    {
        fputs("unable to open the file with errors\n", stderr);
        return 1;
    }

    while(fgets(fbuff, sizeof(fbuff), file_ptr))
        write(sockfd, fbuff, sizeof(fbuff));
        
    write(sockfd, "^^^^^", sizeof("^^^^^"));
    fclose(file_ptr);

    printf("File with errors has been sent\n");
    get_answer(sockfd);

    return 0;
}

int compile(int sockfd, char* serv_name)
{
    int status;
    int cx;
    char ans[FLEN];
    char command[FLEN] = "gcc -o out ";
    cx = snprintf(command+strlen(command), FLEN - strlen(command), "%s ", serv_name);
    if (cx <= 0 || cx > FLEN - strlen(command))
    {
        fprintf(stderr, "an error occurred while creating an executable command\n");
        sprintf(ans, "an error occurred while creating an executable command\n");
        write(sockfd, ans, sizeof(ans));
        return 1;
    }

    cx = snprintf(command+strlen(command), FLEN - strlen(command), "> errors 2>&1");
    if (cx <= 0 || cx > FLEN - strlen(command))
    {
        fprintf(stderr, "an error occurred while creating an executable command\n");
        sprintf(ans, "an error occurred while creating an executable command\n");
        write(sockfd, ans, sizeof(ans));
        return 1;
    }

    //compile and send result
    status = system(command);
    if (status == 0)
    {
        write(sockfd, "compilation complited successfully\n", sizeof("compilation complited successfully\n"));
        printf("compilation complited successfully\n");
        send_errors(sockfd);
        return 0;
    }

    write(sockfd, "compilation failed\n", sizeof("compilation failed\n"));
    printf("compilation failed\n");
    send_errors(sockfd);
    return 0;
}

  
int main(int argc, char** argv) 
{ 
    long int PORT = 1234;
    int sockfd, connfd; 
    struct sockaddr_in servaddr; 
    char serv_name[FLEN] = "serv_";

    printf("hi1\n");

    if (argc > 1 && strncmp(argv[1], "-p", 2) == 0)
        PORT = strtol(argv[2], NULL, 10);
  
    // socket creation and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    { 
        fprintf(stderr, "fatal error: socket creation failed.\n"); 
        return 1; 
    } 
    else
        printf("Socket successfully created.\n");  

    // assign IP, PORT 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) 
    { 
        fprintf(stderr, "fatal error: socket bind failed.\n"); 
        return 2; 
    } 
    else
        printf("Socket successfully binded.\n"); 
  
    // Now server is ready to listen and verification 
    if (listen(sockfd, LISTEN_BACKLOG) == -1) 
    { 
        fprintf(stderr, "fatal error: Listen failed.\n"); 
        return 3; 
    } 
    else
        printf("Server listening.\n"); 
  
    // Accept the data packet from client and verification
    connfd = accept(sockfd, NULL, NULL); 
    if (connfd == -1) 
    { 
        fprintf(stderr, "fatal error: server acccept failed.\n"); 
        return 4; 
    } 
    else
        printf("server acccept the client.\n"); 
  
    if (get_name(connfd, serv_name) != 0)
        return 5;

    if (get_file(connfd, serv_name) != 0)
        return 5;

    if (compile(connfd, serv_name) != 0)
        return 5;

    close(sockfd); 
    return 0;
} 
