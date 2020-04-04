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
        sprintf(answer, "Error: %d receiving information failed \
        (file name)\n", RCC_RECEIVE_ERROR);
        fprintf(stderr, "%s", answer);
        if (write(sockfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (file name)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_RECEIVE_ERROR;
    }

    printf("From client: name of file: %s\n", file_name);
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success file name)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }

    //open file, send response
    file_ptr = fopen(file_name, "w");
    if (file_ptr == NULL)
    {
        sprintf(answer, "Error: %d unable to open file %s\n", RCC_NO_FILE, file_name);
        fprintf(stderr, "%s", answer);
        if (write(sockfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (open file)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_NO_FILE;
    }
    printf("File was opened successfully\n");
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success open file)\n", RCC_SEND_ERROR);
        fclose(file_ptr);
        return RCC_SEND_ERROR;
    }

    //receiving file
    if (safe_read(sockfd, buffer, FLEN) == 0)
    {
        while (strncmp(MYEOF, buffer, 5) != 0)
        {
            fputs(buffer, file_ptr);
            if (safe_read(sockfd, buffer, FLEN) != 0)
            {
                sprintf(answer, "Error: %d receiving information failed \
                (receiving file)\n", RCC_RECEIVE_ERROR);
                fprintf(stderr, "%s", answer);
                if (write(sockfd, answer, FLEN) == -1)
                {
                    fprintf(stderr, "Error: %d sending information failed \
                    (receiving file)\n", RCC_SEND_ERROR);
                    fclose(file_ptr);
                    return RCC_SEND_ERROR;
                }
                fclose(file_ptr);
                return RCC_RECEIVE_ERROR;
            }
        }
    }
    else
    {
        sprintf(answer, "Error: %d receiving information failed \
        (receiving file)\n", RCC_RECEIVE_ERROR);
        fprintf(stderr, "%s", answer);
        if (write(sockfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (receiving file)\n", RCC_SEND_ERROR);
            fclose(file_ptr);
            return RCC_SEND_ERROR;
        }
        fclose(file_ptr);
        return RCC_RECEIVE_ERROR;
    }
    
    //file was successfully received, send response
    printf("File was successfully received\n");
    sprintf(answer, "Success\n");
    if (write(sockfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success receiving file)\n", RCC_SEND_ERROR);
        fclose(file_ptr);
        return RCC_SEND_ERROR;
    }
    fclose(file_ptr);
    return 0;
}

static int common_compile()
{
    char command[FLEN] = "gcc -o out *.c > errors 2>&1";
    return system(command);
}

static int upgrade_compile()
{
    char command[FLEN] = "gcc -o ../server *.c > errors 2>&1";
    return system(command);
}

static int send_result_of_compilation(int sockfd, int result, \
bool upgrade)
{
    FILE* file_ptr;
    char buffer[FLEN];

    //open file, send response
    file_ptr = fopen("errors", "r");
    if (file_ptr == NULL)
    {
        sprintf(buffer, "Error: %d unable to open result file\n", RCC_NO_FILE);
        fprintf(stderr, "%s", buffer);
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (open error file)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_NO_FILE;
    }
    printf("result file was opened successfully\n");
    sprintf(buffer, "Success\n");
    if (write(sockfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success open error file)\n", RCC_SEND_ERROR);
        fclose(file_ptr);
        return RCC_SEND_ERROR;
    }

    //send status
    if (result != 0)
    {
        sprintf(buffer, "Compilation failed\n");
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (status of compilation)\n", RCC_SEND_ERROR);
            fclose(file_ptr);
            return RCC_SEND_ERROR;
        }   
    }
    else
    {
        sprintf(buffer, "Compilation complited successfully\n");

        if (upgrade == true)
        {
            sprintf(buffer, "%sServer was updated successfully!\n", buffer);
        }

        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (status of compilation)\n", RCC_SEND_ERROR);
            fclose(file_ptr);
            return RCC_SEND_ERROR;
        }   
    }

    //send result file
    while (fgets(buffer, FLEN, file_ptr))
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (errors)\n", RCC_SEND_ERROR);
            fclose(file_ptr);
            return RCC_SEND_ERROR;
        }
    
    if (write(sockfd, MYEOF, sizeof(MYEOF)) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (end of errors)\n", RCC_SEND_ERROR);
        fclose(file_ptr);
        return RCC_SEND_ERROR;
    }

    //get response about file delivery
    if (safe_read(sockfd, buffer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed \
        (error file delivery)\n", RCC_RECEIVE_ERROR);
        fclose(file_ptr);
        return RCC_RECEIVE_ERROR;
    }
    if (strncmp(buffer, "Success\n", 8) != 0)
    {
        fprintf(stderr, "From client: %s", buffer);
        fclose(file_ptr);
        return RCC_SEND_ERROR;
    }

    fclose(file_ptr);
    printf("result file has been sent\n");
    return 0;
}

static int get_number_of_files(int connfd, long* number, bool* upgrade)
{
    char buffer[FLEN];

    //get number
    if (safe_read(connfd, buffer, FLEN) != 0)
    {   
        fputs("Error: while getting number of files\n", stderr);
        sprintf(buffer, "Error: %d receiving information failed \
        (number of files)\n", RCC_RECEIVE_ERROR);
        if (write(connfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (number of files)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_RECEIVE_ERROR;
    }
    *number = strtol(buffer, NULL, 10);

    //send response
    if (*number < 0)
    {
        fputs("Error: wrong number of files\n", stderr);
        sprintf(buffer, "Error: %d wrong number of files\n", RCC_WRONG_ARG);
        if (write(connfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (wrong number of files)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_WRONG_ARG;
    }

    sprintf(buffer, "Success\n");
    if (write(connfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success number of files)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }

    //get upgrade flag
    if (safe_read(connfd, buffer, FLEN) != 0)
    {   
        fputs("Error: while getting upgrade flag\n", stderr);
        sprintf(buffer, "Error: %d receiving information failed \
        (upgrade flag)\n", RCC_RECEIVE_ERROR);
        if (write(connfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (upgrade flag)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_RECEIVE_ERROR;
    }
   
    if (strncmp(buffer, "Upgrade", 7) == 0)
        *upgrade = true;
    else
        *upgrade = false;

    //send response
    sprintf(buffer, "Success\n");
    if (write(connfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success number of files)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }
    return 0;
}

static int authorization(int connfd, char* version, \
char* passwords_file_name)
{
    char buffer[FLEN];
    char answer[FLEN];
    char username[FLEN];
    char password[FLEN];
    int check;
    FILE* pass_ptr;

    //get user status, send response
    if (safe_read(connfd, buffer, FLEN) != 0)
    {   
        fputs("Error: while getting user status\n", stderr);
        sprintf(answer, "Error: %d receiving information failed \
        (user status)\n", RCC_RECEIVE_ERROR);
        if (write(connfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (user status)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_RECEIVE_ERROR;
    }
    sprintf(answer, "Success\n");
    if (write(connfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success user status)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }

    //if user is regular, return 
    if (strncmp(buffer, "regular_user", 12) == 0)
    {
        printf("User is regular\n");
        return 0;
    }

    //user is root, get username and password
    printf("User is root\n");
    if (safe_read(connfd, username, FLEN) != 0)
    {   
        fputs("Error: while getting username\n", stderr);
        sprintf(answer, "Error: %d receiving information failed \
        (username)\n", RCC_RECEIVE_ERROR);
        if (write(connfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (username)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_RECEIVE_ERROR;
    }
    if (safe_read(connfd, password, FLEN) != 0)
    {   
        fputs("Error: while getting password\n", stderr);
        sprintf(answer, "Error: %d receiving information failed \
        (password)\n", RCC_RECEIVE_ERROR);
        if (write(connfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (password)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_RECEIVE_ERROR;
    }

    //send response
    sprintf(answer, "Success\n");
    if (write(connfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success auth data)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }

    //open file with passwords
    pass_ptr = fopen(passwords_file_name, "r");
    if (pass_ptr == NULL)
    {
        check = snprintf(answer, FLEN, "Error: %d unable to open file \
        with passwords %s\n", RCC_NO_FILE, passwords_file_name);
        if (check >= FLEN)
        {
            sprintf(answer, "Error: %d too long name of file\n", RCC_UNEXPEC_VAL);
            fprintf(stderr, "%s", answer);
            if (write(connfd, answer, FLEN) == -1)
            {
                fprintf(stderr, "Error: %d sending information failed \
                (long file name)\n", RCC_SEND_ERROR);
                return RCC_SEND_ERROR;
            }
            return RCC_UNEXPEC_VAL;
        }
        fprintf(stderr, "%s", answer);
        if (write(connfd, answer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (file opening)\n", RCC_SEND_ERROR);
            return RCC_SEND_ERROR;
        }
        return RCC_NO_FILE;
    }

    //data validation
    while (fgets(buffer, FLEN, pass_ptr))
	{
		buffer[strcspn(buffer, "\n")] = 0;
        if (strncmp(buffer, username, strlen(username)) == 0)
        {
            fgets(buffer, FLEN, pass_ptr);
            buffer[strcspn(buffer, "\n")] = 0;
            if (strncmp(buffer, password, strlen(password)) == 0)
            {
                sprintf(answer, "Success\n");
                if (write(connfd, answer, FLEN) == -1)
                {
                    fprintf(stderr, "Error: %d sending information failed \
                    (success data validation)\n", RCC_SEND_ERROR);
                    return RCC_SEND_ERROR;
                }

                //authorization complete, send version
                sprintf(answer, "%s", version);
                if (write(connfd, answer, FLEN) == -1)
                {
                    fprintf(stderr, "Error: %d sending information failed \
                    (version)\n", RCC_SEND_ERROR);
                    return RCC_SEND_ERROR;
                }
                return 0;
            }
        }
	}

    //wrong username or password
    sprintf(answer, "Error: %d wrong username or password\n", RCC_WRONG_ARG);
    if (write(connfd, answer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (wrong auth data)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }
    return RCC_WRONG_ARG;
}
  
static void usage()
{
    printf("usage: ./server [-h] [-p <port>] [-a <file_with_passwords.txt>]\n\
    Default options:\n\
    Port: 1234\n\
    -h for help.\n\
    Name of file with passwords: passwords.txt\n");
}

int main(int argc, char** argv) 
{ 
    long i, number_of_files;                //i is auxiliary variable
    long PORT = RCC_PORT_DEFAULT;
    int sockfd, connfd;
    int rez = 0;                            //used in getopt
    int check;                              //used in snprintf to check result
    int result;                             //compilation result
    struct sockaddr_in servaddr; 
    struct stat st;
    char passwords_file_name[FLEN] = "passwords.txt";
    char version[LEN] = "0.3.1";
    bool upgrade = false;

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
            check = snprintf(passwords_file_name, FLEN, "%s", optarg);
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

    // socket creation and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    { 
        fputs("Error: socket creation failed.\n", stderr); 
        return RCC_SOCK_ERROR; 
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
        return RCC_BIND_ERROR; 
    } 
    else
        printf("Socket successfully binded.\n"); 
  
    // Now server is ready to listen and verification 
    if (listen(sockfd, LISTEN_BACKLOG) == -1) 
    { 
        fputs("Error: Listen failed.\n", stderr); 
        return RCC_LISTEN_ERROR; 
    } 
    else
        printf("Server listening.\n"); 
  
    // Accept the data packet from client and verification
    connfd = accept(sockfd, NULL, NULL); 
    if (connfd == -1) 
    { 
        fputs("Error: server acccept failed.\n", stderr); 
        return RCC_ACCEPT_ERROR; 
    } 
    else
        printf("Server acccept the client.\n"); 

    if (authorization(connfd, version, passwords_file_name) != 0)
        return RCC_AUTH_ERROR;

    if (get_number_of_files(connfd, &number_of_files, &upgrade) != 0)
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
            return RCC_RECEIVE_ERROR;
        i++;
    }    

    if (upgrade == true)
        result = upgrade_compile();
    else 
        result = common_compile();


    if (send_result_of_compilation(connfd, result, upgrade) != 0)
        return RCC_SERVER_ERROR;

    close(sockfd); 
    return 0;
} 