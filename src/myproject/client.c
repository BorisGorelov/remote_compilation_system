#include "source.h"
#define ANS_OPEN_ERR_FILE "unable to open the file with errors\n"
#define ANS_ERR_RECEIVED "file with errors was recieved\n"

int send_file(int sockfd, char* name)
{
    FILE* file_ptr = fopen(name, "r");
    char fbuff[FLEN];
    char buf[FLEN];

    if(file_ptr == NULL)
    {
        fprintf(stderr, "unable to open the file %s\n", name);
        return 1;
    }

    //send name, get response
    write(sockfd, name, sizeof(name));
    if(safe_answer(sockfd, buf, FLEN) != 0)
        return 1;

    if(strncmp("1.", buf, 2) == 0)
    {
        fputs("error sending the name to the server\n", stderr);
        fclose(file_ptr);
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
        fclose(file_ptr);
        return 3;
    }
}

int get_errors(int sockfd)
{
    FILE* file_ptr = fopen("received_errors", "w"); 
    char fbuff[FLEN];

    if(file_ptr == NULL)
    {
        fputs(ANS_OPEN_ERR_FILE, stderr);
        write(sockfd, ANS_OPEN_ERR_FILE, sizeof(ANS_OPEN_ERR_FILE));
        return 1;
    }

    //recieving file
    if(safe_read(sockfd, fbuff, FLEN) != 0)
        return 1;
    while(strncmp("^^^^^", fbuff, 5) != 0)
    {
        fputs(fbuff, file_ptr);
        if(safe_read(sockfd, fbuff, FLEN) != 0)
            return 2;
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
        fputs("unable to open the file with received errors\n", stderr);
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

    write(sockfd, ANS_ERR_RECEIVED, sizeof(ANS_ERR_RECEIVED));
    fclose(file_ptr);
    return 0;
}

void usage()
{
    printf("usage: ./client [-d <ip>] [-p <port>] file[s]");
}

int main(int argc, char** argv) 
{ 
    int sockfd, number_of_files, i, check; 
    int flag = 0; //tracking whether the user specified a port and ip
    struct sockaddr_in servaddr; 
    long int PORT = RCC_PORT_DEFAULT;
    char IP[LEN] = RCC_IP_DEFAULT;
    char buf[FLEN];

    setlocale(LC_ALL, "");
    if(argc < 2 /*|| second option is -h*/)
    {
        usage();
        return RCC_WRONG_ARG;
    }

    //check if user want to change default ip or port
    for (i = 1; i < argc; ++i)
    {
        if(strncmp(argv[i], "-d", 2) == 0 && i < argc - 1)
        {
            check = snprintf(IP, 16, "%s", argv[i+1]);
            flag++;
            if(check <= 0 || check >= 16)
            {
                fputs("fatal error. wrong ip.\n", stderr);
                return 2;
            }
        }
        if(strncmp(argv[i], "-p", 2) == 0 && i < argc - 1)
        {
            PORT = strtol(argv[i+1], NULL, 10);
            flag++;
            if(PORT < 0 || PORT > 65535)
            {
                fputs("fatal error. wrong port.\n", stderr);
                return 2;
            }
        }   
    }

    // socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    { 
        fputs("fatal error: socket creation failed.\n", stderr); 
        return RCC_SOCK_FAIL;
    } 
    else
        printf("Socket successfully created, socket = %d\n", sockfd); 

    // assign IP, PORT 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr) <= 0)
    {
        fprintf(stderr, "fatal error: inet_pton error for %s\n", IP);
        return RCC_INETPTON_ERROR;
    }     
  
    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
    { 
        fputs("connection with the server failed...\n", stderr); 
        return RCC_CONNECTION_FAIL;
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
            fprintf(stderr, "an unexpected value of a variable (i = %d)\n", i);
            return RCC_UNEXPEC_VAL;
    }

    //send number of files
    number_of_files = argc - flag * 2 - 1;
    sprintf(buf, "%d", number_of_files);
    write(sockfd, buf, sizeof(buf));

    while(i < argc)
    {
        if(send_file(sockfd, argv[i]) != 0)
            return RCC_SEND_FAIL;
        i++;
    }

    if (get_errors(sockfd) != 0)
        return RCC_RECEIVE_FAIL;

    if (print_errors(sockfd) != 0)
        return RCC_PRINT_FAIL;

    close(sockfd); 

    return 0;
} 
