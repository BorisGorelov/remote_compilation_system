#include "source.h"
#define ANS0 "0. the name is received.\n"
#define ANS1 "1. error occurred while recieving name\n"
#define ANS_ERR_EXE "an error occurred while creating an executable command\n"
#define ANS_COMPILE_0 "compilation complited successfully\n"
#define ANS_COMPILE_1 "compilation failed\n"

int get_file(int sockfd)
{
    FILE* file_ptr = NULL;
    char buffer[FLEN];
    char answer[FLEN];
    char file_name[FLEN];

    //get file name, send response
    if (safe_read(sockfd, file_name, FLEN) != 0)
    {
        sprintf(answer, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        fprintf(stderr, "%s", answer);
        if (write(sockfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_RECEIVE_FAIL;
    }

    printf("From client: name of file: %s\n", file_name);
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }

    //open file, send response
    file_ptr = fopen(file_name, "w");
    if (file_ptr == NULL)
    {
        sprintf(answer, "Error: %d unable to open file %s\n", RCC_NO_FILE, file_name);
        fprintf(stderr, "%s", answer);
        if (write(sockfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_NO_FILE;
    }
    printf("File was opened successfully\n");
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }

    //receiving file
    if (safe_read(sockfd, buffer, FLEN) == 0)
    {
        while (strncmp(MYEOF, buffer, 5) != 0)
        {
            fputs(buffer, file_ptr);
            if (safe_read(sockfd, buffer, FLEN) != 0)
            {
                sprintf(answer, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
                fprintf(stderr, "%s", answer);
                if (write(sockfd, answer, FLEN) == -1)
                {
                    fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
                    return RCC_SEND_FAIL;
                }
                return RCC_RECEIVE_FAIL;
            }
        }
    }
    else
    {
        sprintf(answer, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        fprintf(stderr, "%s", answer);
        if (write(sockfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_RECEIVE_FAIL;
    }
    
    //file was successfully received, send response
    printf("File was successfully received\n");
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }
    fclose(file_ptr);
    return 0;
}

int send_errors(int sockfd)
{
    FILE* file_ptr = fopen("errors", "r");
    char buffer[FLEN];

    if (file_ptr == NULL)
    {
        fputs("unable to open the file with errors\n", stderr);
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), file_ptr))
        write(sockfd, buffer, sizeof(buffer));
        
    write(sockfd, "^^^^^", sizeof("^^^^^"));
    fclose(file_ptr);

    printf("File with errors has been sent\n");
    safe_answer(sockfd, buffer, FLEN);

    return 0;
}

int compile(int sockfd, char* serv_name)
{
    int status;
    char command[FLEN] = "gcc -o out *.c > errors 2>&1";

    //compile and send result
    status = system(command);
    if (status == 0)
    {
        write(sockfd, ANS_COMPILE_0, sizeof(ANS_COMPILE_0));
        printf(ANS_COMPILE_0);
        send_errors(sockfd);
        return 0;
    }

    write(sockfd, ANS_COMPILE_1, sizeof(ANS_COMPILE_1));
    printf(ANS_COMPILE_1);
    send_errors(sockfd);
    return 0;
}

int get_number_of_files(int connfd, long* number)
{
    char buf[FLEN];
    if (safe_read(connfd, buf, FLEN) != 0)
    {   
        fputs("Error: while getting number of files\n", stderr);
        sprintf(buf, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        if (write(connfd, buf, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_RECEIVE_FAIL;
    }
    *number = strtol(buf, NULL, 10);
    if (*number < 0)
    {
        fputs("Error: wrong number of files\n", stderr);
        sprintf(buf, "Error: %d wrong number of files\n", RCC_WRONG_ARG);
        if (write(connfd, buf, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_WRONG_ARG;
    }

    sprintf(buf, "Success\n");
    if (write(connfd, buf, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }
    return 0;
}
  
void usage()
{
    printf("usage: ./server [-p <port>] [-a <file_with_passwords.txt>]\n\
    Default options:\n\
    Port: 1234\n\
    Name of file with passwords: passwords.txt\n");
}

int main(int argc, char** argv) 
{ 
    long i, number_of_files;
    long PORT = RCC_PORT_DEFAULT;
    int sockfd, connfd;
    int rez = 0; 
    int check;
    struct sockaddr_in servaddr; 
    char serv_name[FLEN];
    struct stat st;
    char passwords[FLEN] = "passwords.txt";
    FILE* passwords_ptr;

	while ((rez = getopt(argc,argv,"hp:a:")) != -1)
    {
		switch (rez)
        {
		case 'h': 
            usage(); 
            return 0;
		case 'p':
            PORT = strtol(optarg, NULL, 10);
            if (PORT < 0 || PORT > 65535)
            {
                fprintf(stderr, "Error: %d Wrong port\n", RCC_WRONG_ARG);
                return RCC_WRONG_ARG;
            }
            break;
        case 'a':
            check = snprintf(passwords, FLEN, "%s", optarg);
            if (check <= 2 || check >= FLEN)
            {
                fprintf(stderr, "Error: %d Wrong file name\n", RCC_WRONG_ARG);
                usage();
                return RCC_WRONG_ARG;
            }
            break;
		default: 
            fprintf(stderr, "Error: %d Wrong argument\n", RCC_WRONG_ARG);
            usage();
            return RCC_WRONG_ARG;
        }
	}

    passwords_ptr = fopen(passwords, "r");
    if (passwords_ptr == NULL)
    {
        fprintf(stderr, "Error: %d Wrong password file\n", RCC_NO_FILE);
        return RCC_NO_FILE;
    }

    // socket creation and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    { 
        fputs("fatal error: socket creation failed.\n", stderr); 
        return RCC_SOCK_FAIL; 
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
        fputs("fatal error: socket bind failed.\n", stderr); 
        return RCC_BIND_FAIL; 
    } 
    else
        printf("Socket successfully binded.\n"); 
  
    // Now server is ready to listen and verification 
    if (listen(sockfd, LISTEN_BACKLOG) == -1) 
    { 
        fputs("fatal error: Listen failed.\n", stderr); 
        return RCC_LISTEN_FAIL; 
    } 
    else
        printf("Server listening.\n"); 
  
    // Accept the data packet from client and verification
    connfd = accept(sockfd, NULL, NULL); 
    if (connfd == -1) 
    { 
        fputs("fatal error: server acccept failed.\n", stderr); 
        return RCC_ACCEPT_FAIL; 
    } 
    else
        printf("server acccept the client.\n"); 
  
    if (get_number_of_files(connfd, &number_of_files) != 0)
        return RCC_UNEXPEC_VAL;

    //getting files
    if (stat("/source", &st) != 0)
        mkdir("source", 0777);
    chdir("source");

    i = 0;
    while (i < number_of_files)
    {
        if (get_file(connfd) != 0)
            return RCC_RECEIVE_FAIL;
        i++;
    }    

    if (compile(connfd, serv_name) != 0)
        return RCC_COMPILE_ERROR;

    close(sockfd); 
    return 0;
} 
