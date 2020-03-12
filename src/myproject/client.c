//usage: ./client [-d <ip>] [-p <port>] file[s]
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#define LEN 70 
#define FLEN 256

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
    char buf[LEN];

    if(file_ptr == NULL)
    {
        fputs("unable to open the file\n", stderr);
        return 1;
    }

    //send name, get response
    write(sockfd, name, sizeof(name));
    read(sockfd, buf, sizeof(buf));

    if(strncmp("1.", buf, 2) == 0)
    {
        fputs("error sending the name to the server\n", stderr);
        return 2;
    }
    else if(strncmp("0.", buf, 2) == 0)
    {
        while(fgets(fbuff, sizeof(fbuff), file_ptr))
        write(sockfd, fbuff, sizeof(fbuff));
        
        write(sockfd, "^^^^^", sizeof("^^^^^"));
        fclose(file_ptr);

        printf("File has been sent\n");
        return 0;
    }
    else 
    {
        fputs("error: unexpected response\n", stderr);
        return 3;
    }
}

int get_errors(int sockfd)
{
    FILE* file_ptr = fopen("received_errors", "w"); 
    char fbuff[FLEN];

    if(file_ptr == NULL)
    {
        fputs("unable to open the file with errors\n", stderr);
        write(sockfd, "unable to open the file with errors\n", sizeof("unable to open the file with errors\n"));
        return 1;
    }

    //recieving file
    read(sockfd, fbuff, sizeof(fbuff));
    while(strncmp("^^^^^", fbuff, 5) != 0)
    {
        fputs(fbuff, file_ptr);
        read(sockfd, fbuff, sizeof(fbuff));
    }
    fclose(file_ptr);
    return 0;
}

int print_errors(int sockfd)
{
    FILE* file_ptr = fopen("received_errors", "r");
    long size;
    char fbuff[FLEN];
    if (file_ptr == NULL)
    {
        fprintf(stderr, "unable to open the file with received errors\n");
        return 1;
    }

    //check if file with errors is empty
    fseek(file_ptr, 0, SEEK_END);
    size = ftell(file_ptr);
    if(size == 0)
        return 0;

    rewind(file_ptr);
    printf("file with errors was recieved.\nerrors:\n");
    while(fgets(fbuff, sizeof(fbuff), file_ptr))
        fputs(fbuff, stdout);

    write(sockfd, "file with errors was recieved\n", sizeof("file with errors was recieved\n"));
    fclose(file_ptr);
    return 0;
}

int main(int argc, char** argv) 
{ 
    int sockfd, number_of_files, i, check; 
    int flag = 0; //tracking whether the user specified a port and ip
    struct sockaddr_in servaddr; 
    long int PORT = 1234;
    char IP[LEN] = "127.0.0.1";
    char buf[LEN];

    //check if user want to change default ip or port
    for (i = 1; i < argc; ++i)
    {
        if(strncmp(argv[i], "-d", 2) == 0 && i < argc - 1)
        {
            check = snprintf(IP, 16, "%s", argv[i+1]);
            flag++;
            if(check <= 0 || check >= 16)
            {
                fprintf(stderr, "fatal error. wrong ip.\n");
                return 2;
            }
        }
        if(strncmp(argv[i], "-p", 2) == 0 && i < argc - 1)
        {
            PORT = strtol(argv[i+1], NULL, 10);
            flag++;
            if(PORT < 0 || PORT > 65535)
            {
                fprintf(stderr, "fatal error. wrong port.\n");
                return 2;
            }
        }   
    }

    // socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    { 
        fprintf(stderr, "fatal error: socket creation failed.\n"); 
        return 3;
    } 
    else
        printf("Socket successfully created, socket = %d\n", sockfd); 
  
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr) <= 0)
    {
        fprintf(stderr, "fatal error: inet_pton error for %s\n", IP);
        return 4;
    }     
  
    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
    { 
        fprintf(stderr, "connection with the server failed...\n"); 
        return 5;
    } 
    else
        printf("connected to the server.\n"); 
   
    //find position of first filename
    switch(flag)
    {
        case 0: 
            i = 1;
            break;
        case 1:
            i = 3;
            break;
        case 2:
            i = 5;
            break;
        default:
            fprintf(stderr, "an unexpected value of a variable\n");
            return 6;
    }

    //send number of files
    number_of_files = argc - flag * 2 - 1;
    printf("number of files: %d, i = %d, argc = %d\n", number_of_files, i, argc);
    sprintf(buf, "%d", number_of_files);
    write(sockfd, buf, sizeof(buf));

    while(i < argc)
    {
        if(send_file(sockfd, argv[i]) != 0)
            return 6;
        i++;
    }

    if (get_errors(sockfd) != 0)
        return 6;

    if (print_errors(sockfd) != 0)
        return 6;

    close(sockfd); 

    return 0;
} 
