#include "source.h"


static int get_file(int sockfd)
{
    FILE* file_ptr;
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
        fclose(file_ptr);
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
                    fclose(file_ptr);
                    return RCC_SEND_FAIL;
                }
                fclose(file_ptr);
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
            fclose(file_ptr);
            return RCC_SEND_FAIL;
        }
        fclose(file_ptr);
        return RCC_RECEIVE_FAIL;
    }
    
    //file was successfully received, send response
    printf("File was successfully received\n");
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        fclose(file_ptr);
        return RCC_SEND_FAIL;
    }
    fclose(file_ptr);
    return 0;
}

static int compile()
{
    char command[FLEN] = "gcc -o out *.c > errors 2>&1";
    return system(command);
}

static int send_result_of_compilation(int sockfd)
{
    int result;
    FILE* file_ptr;
    char buffer[FLEN];

    result = compile();

    //open file, send response
    file_ptr = fopen("errors", "r");
    if (file_ptr == NULL)
    {
        sprintf(buffer, "Error: %d unable to open file with errors\n", RCC_NO_FILE);
        fprintf(stderr, "%s", buffer);
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_NO_FILE;
    }
    printf("File with errors was opened successfully\n");
    sprintf(buffer, "Success\n");
    if (write(sockfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        fclose(file_ptr);
        return RCC_SEND_FAIL;
    }

    //send status
    if (result != 0)
    {
        sprintf(buffer, "Compilation failed\n");
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            fclose(file_ptr);
            return RCC_SEND_FAIL;
        }   
    }
    else
    {
        sprintf(buffer, "Compilation complited successfully\n");
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            fclose(file_ptr);
            return RCC_SEND_FAIL;
        }   
    }

    //send file with errors
    while (fgets(buffer, FLEN, file_ptr))
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            fclose(file_ptr);
            return RCC_SEND_FAIL;
        }
    
    if (write(sockfd, MYEOF, sizeof(MYEOF)) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        fclose(file_ptr);
        return RCC_SEND_FAIL;
    }

    //get response about file delivery
    if (safe_read(sockfd, buffer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        fclose(file_ptr);
        return RCC_RECEIVE_FAIL;
    }
    if (strncmp(buffer, "Success\n", 8) != 0)
    {
        fprintf(stderr, "From client: %s", buffer);
        fclose(file_ptr);
        return RCC_SEND_FAIL;
    }

    fclose(file_ptr);
    printf("File with errors has been sent\n");
    return 0;
}

static int get_number_of_files(int connfd, long* number)
{
    char buffer[FLEN];

    //get number
    if (safe_read(connfd, buffer, FLEN) != 0)
    {   
        fputs("Error: while getting number of files\n", stderr);
        sprintf(buffer, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        if (write(connfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_RECEIVE_FAIL;
    }
    *number = strtol(buffer, NULL, 10);

    //send response
    if (*number < 0)
    {
        fputs("Error: wrong number of files\n", stderr);
        sprintf(buffer, "Error: %d wrong number of files\n", RCC_WRONG_ARG);
        if (write(connfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }
        return RCC_WRONG_ARG;
    }

    sprintf(buffer, "Success\n");
    if (write(connfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }
    return 0;
}
  
static void usage()
{
    printf("usage: ./server [-p <port>] [-a <file_with_passwords.txt>]\n\
    Default options:\n\
    Port: 1234\n\
    Name of file with passwords: passwords.txt\n");
}

int main(int argc, char** argv) 
{ 
    long i, number_of_files;                //i is auxiliary variable
    long PORT = RCC_PORT_DEFAULT;
    int sockfd, connfd;
    int rez = 0;                            //used in getopt
    int check;                              //used in snprintf to check result
    struct sockaddr_in servaddr; 
    struct stat st;                         //used in stat call
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
        fputs("Error: socket creation failed.\n", stderr); 
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
        fputs("Error: socket bind failed.\n", stderr); 
        return RCC_BIND_FAIL; 
    } 
    else
        printf("Socket successfully binded.\n"); 
  
    // Now server is ready to listen and verification 
    if (listen(sockfd, LISTEN_BACKLOG) == -1) 
    { 
        fputs("Error: Listen failed.\n", stderr); 
        return RCC_LISTEN_FAIL; 
    } 
    else
        printf("Server listening.\n"); 
  
    // Accept the data packet from client and verification
    connfd = accept(sockfd, NULL, NULL); 
    if (connfd == -1) 
    { 
        fputs("Error: server acccept failed.\n", stderr); 
        return RCC_ACCEPT_FAIL; 
    } 
    else
        printf("Server acccept the client.\n"); 

    if (get_number_of_files(connfd, &number_of_files) != 0)
        return RCC_UNEXPEC_VAL;

    //create empty directory for files
    if (stat("./source", &st) == 0)
        system("rm -r ./source");
    mkdir("source", 0777);
    chdir("source");

    //getting files
    i = 0;
    while (i < number_of_files)
    {
        if (get_file(connfd) != 0)
            return RCC_RECEIVE_FAIL;
        i++;
    }    

    if (send_result_of_compilation(connfd) != 0)
        return RCC_SERVER_ERROR;

    close(sockfd); 
    return 0;
} 