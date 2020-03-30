#include "source.h"
#define ANS_OPEN_ERR_FILE "unable to open the file with errors\n"
#define ANS_ERR_RECEIVED "file with errors was recieved\n"


int send_file(int sockfd)
{
    FILE* file_ptr = NULL;
    char buffer[FLEN];
    char answer[FLEN];
    char file_name[FLEN];

    printf("Enter the file name : ");
    fgets(file_name, FLEN, stdin);
  	file_name[strcspn(file_name, "\n")] = 0;

    file_ptr = fopen(file_name, "r");
    if (file_ptr == NULL)
    {
        fprintf(stderr, "Error: %d unable to open the file %s\n", RCC_NO_FILE, file_name);
        return RCC_NO_FILE;
    }

    //send name, get response
    if (write(sockfd, file_name, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }

    if (safe_read(sockfd, answer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        return RCC_RECEIVE_FAIL;
    }

    if (strncmp(answer, "Success\n", 8) == 0)
    {
        //get response about file opening
        if (safe_read(sockfd, answer, FLEN) != 0)
        {
            fprintf(stderr, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
            return RCC_RECEIVE_FAIL;
        }
        if (strncmp(answer, "Success\n", 8) != 0)
        {
            fprintf(stderr, "From server: %s", answer);
            fclose(file_ptr);
            return RCC_SERVER_ERROR;
        }

        //send file
        while (fgets(buffer, FLEN, file_ptr))
            if (write(sockfd, buffer, FLEN) == -1)
            {
                fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
                return RCC_SEND_FAIL;
            }
        
        if (write(sockfd, MYEOF, sizeof(MYEOF)) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
            return RCC_SEND_FAIL;
        }

        //get response about file delivery
        if (safe_read(sockfd, answer, FLEN) != 0)
        {
            fprintf(stderr, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
            return RCC_RECEIVE_FAIL;
        }
        if (strncmp(answer, "Success\n", 8) != 0)
        {
            fprintf(stderr, "From server: %s", answer);
            fclose(file_ptr);
            return RCC_SERVER_ERROR;
        }

        fclose(file_ptr);
        printf("File has been sent\n");
        return 0;
    }
    else 
    {
        fprintf(stderr, "From server: %s", answer);
        fclose(file_ptr);
        return RCC_SERVER_ERROR;
    }
}

int get_errors(int sockfd)
{
    FILE* file_ptr = fopen("received_errors", "w"); 
    char buffer[FLEN];

    if (file_ptr == NULL)
    {
        fputs(ANS_OPEN_ERR_FILE, stderr);
        write(sockfd, ANS_OPEN_ERR_FILE, sizeof(ANS_OPEN_ERR_FILE));
        return 1;
    }

    //recieving file
    if (safe_read(sockfd, buffer, FLEN) != 0)
        return 1;
    while (strncmp("^^^^^", buffer, 5) != 0)
    {
        fputs(buffer, file_ptr);
        if (safe_read(sockfd, buffer, FLEN) != 0)
            return 2;
    }
    fclose(file_ptr);
    return 0;
}

int print_errors(int sockfd)
{
    FILE* file_ptr = fopen("received_errors", "r");
    long size;
    char buffer[FLEN];
    if (file_ptr == NULL)
    {
        fputs("unable to open the file with received errors\n", stderr);
        return 1;
    }

    //check if file with errors is empty
    fseek(file_ptr, 0, SEEK_END);
    size = ftell(file_ptr);
    if (size == 0)
        return 0;

    rewind(file_ptr);
    printf("file with errors was recieved.\nerrors:\n");
    while (fgets(buffer, sizeof(buffer), file_ptr))
        fputs(buffer, stdout);

    write(sockfd, ANS_ERR_RECEIVED, sizeof(ANS_ERR_RECEIVED));
    fclose(file_ptr);
    return 0;
}

int send_number_of_files(int sockfd, long number_of_files)
{
    char buf[FLEN];
    sprintf(buf, "%ld", number_of_files);
    if (write(sockfd, buf, sizeof(buf)) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed\n", RCC_SEND_FAIL);
        return RCC_SEND_FAIL;
    }
    if (safe_read(sockfd, buf, sizeof(buf)) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed\n", RCC_RECEIVE_FAIL);
        return RCC_RECEIVE_FAIL;
    }
    if (strncmp(buf, "Success\n", 8) != 0)
    {
        fprintf(stderr, "From server: %s", buf);
        return RCC_RECEIVE_FAIL;
    }
    return 0;
}

void usage()
{
    printf("usage: ./client [-d <ip>] [-p <port>] [-n <number of files] [-u Username].\n\
    Default options:\n\
    IP: 127.0.0.1\n\
    Port: 1234\n\
    Number of files: 1\n\
    If the -u flag is specified, the user is treated as a root\
    and next he will be asked to enter a password.\n");
}

int main(int argc, char** argv) 
{ 
    int sockfd;
    int i, check;
    int rez = 0; 
    struct sockaddr_in servaddr; 
    long PORT = RCC_PORT_DEFAULT;
    long number_of_files = 1;
    char IP[LEN] = RCC_IP_DEFAULT;
    char username[LEN];
    char password[LEN];

	while ((rez = getopt(argc, argv, "hd:p:n:u:")) != -1)
    {
		switch (rez)
        {
            case 'h': 
                usage(); 
                return 0;
            case 'p':
                PORT = strtol(optarg, NULL, 10);
                if (PORT < 0 || PORT > UINT16_MAX)
                {
                    fprintf(stderr, "Error: %d Wrong port\n", RCC_WRONG_ARG);
                    return RCC_WRONG_ARG;
                }
                break;
            case 'd':
                check = snprintf(IP, 16, "%s", optarg);
                if (check <= 0 || check >= 16)
                {
                    fprintf(stderr, "Error: %d Wrong IP\n", RCC_WRONG_ARG);
                    return RCC_WRONG_ARG;
                }
                break;
            case 'u':
                check = snprintf(username, 70, "%s", optarg);
                if (check <= 0 || check >= 70)
                {
                    fprintf(stderr, "Error: %d Wrong Username\n", RCC_WRONG_ARG);
                    return RCC_WRONG_ARG;
                }

                printf("Enter the password\n");
                system("stty cbreak -echo");
                fgets(password, 70, stdin);
	            system("stty -cbreak echo");
	            password[strcspn(password, "\n")] = 0;

                /*
                 * check if password is correct
                 * for example 3 attempts
                 * if okay, send version and continue
                 */
                break;
            case 'n':
                number_of_files = strtol(optarg, NULL, 10);
                if (number_of_files <= 0 || number_of_files > 10)
                {
                    fprintf(stderr, "Error: %d Wrong number of files\n", RCC_WRONG_ARG);
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
        fputs("fatal error: socket creation failed.\n", stderr); 
        return RCC_SOCK_FAIL;
    } 
    else
        printf("Socket successfully created, socket = %d\n", sockfd); 

    // assign IP, PORT 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr) <= 0)
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

    if (send_number_of_files(sockfd, number_of_files) != 0)
        return RCC_SEND_FAIL;

    i = 0;
    while (i < number_of_files)
    {
        if (send_file(sockfd) != 0)
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
