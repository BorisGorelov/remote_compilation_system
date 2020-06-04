#include "source.h"

static int send_file(int sockfd)
{
    FILE* file_ptr = NULL;
    char buffer[FLEN];
    char answer[FLEN];
    char file_name[FLEN];
    char path[FLEN];
    int check;

    printf("Enter file path: ");
    fgets(path, FLEN, stdin);
  	path[strcspn(path, "\n")] = 0;

    printf("Enter the file name: ");
    fgets(file_name, FLEN, stdin);
  	file_name[strcspn(file_name, "\n")] = 0;

    check = snprintf(path + strlen(path), FLEN - strlen(path), "%s", file_name);
    if (check >= FLEN || check < 2)
    {
        fprintf(stderr, "Error: %d wrong path or file name \
        %s\n", RCC_NO_FILE, file_name);
        return RCC_NO_FILE;
    }

    file_ptr = fopen(path, "r");
    if (file_ptr == NULL)
    {
        fprintf(stderr, "Error: %d unable to open the file \
        %s\n", RCC_NO_FILE, file_name);
        return RCC_NO_FILE;
    }

    //send name, get response
    if (write(sockfd, file_name, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (file name)\n", RCC_SEND_ERROR);
        fclose(file_ptr);
        return RCC_SEND_ERROR;
    }

    if (safe_read(sockfd, answer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed \
        (file name answer)\n", RCC_RECEIVE_ERROR);
        fclose(file_ptr);
        return RCC_RECEIVE_ERROR;
    }

    if (strncmp(answer, "Success\n", 8) == 0)
    {
        //get response about file opening
        if (safe_read(sockfd, answer, FLEN) != 0)
        {
            fprintf(stderr, "Error: %d receiving information failed \
            (file opening answer)\n", RCC_RECEIVE_ERROR);
            fclose(file_ptr);
            return RCC_RECEIVE_ERROR;
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
                fprintf(stderr, "Error: %d sending information failed \
                (file sending)\n", RCC_SEND_ERROR);
                fclose(file_ptr);
                return RCC_SEND_ERROR;
            }
        
        if (write(sockfd, MYEOF, sizeof(MYEOF)) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (eof sending)\n", RCC_SEND_ERROR);
            fclose(file_ptr);
            return RCC_SEND_ERROR;
        }

        //get response about file delivery
        if (safe_read(sockfd, answer, FLEN) != 0)
        {
            fprintf(stderr, "Error: %d receiving information failed \
            (delivery response)\n", RCC_RECEIVE_ERROR);
            fclose(file_ptr);
            return RCC_RECEIVE_ERROR;
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

static int get_result_of_compilation(int sockfd)
{
    FILE* file_ptr;
    char buffer[FLEN];

    file_ptr = fopen("received_errors", "w");
    if (file_ptr == NULL)
    {
        fprintf(stderr, "Error: %d unable to open the file for \
        receiving errors\n", RCC_NO_FILE);
        return RCC_NO_FILE;
    }

    //get response about file opening
    if (safe_read(sockfd, buffer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed \
        (file opening response)\n", RCC_RECEIVE_ERROR);
        fclose(file_ptr);
        return RCC_RECEIVE_ERROR;
    }
    if (strncmp(buffer, "Success\n", 8) != 0)
    {
        fprintf(stderr, "From server: %s", buffer);
        fclose(file_ptr);
        return RCC_SERVER_ERROR;
    }

    //receiving result file
    if (safe_read(sockfd, buffer, FLEN) == 0)
    {
        while (strncmp(MYEOF, buffer, 5) != 0)
        {
            fputs(buffer, file_ptr);
            if (safe_read(sockfd, buffer, FLEN) != 0)
            {
                sprintf(buffer, "Error: %d receiving information failed \
                (result file)\n", RCC_RECEIVE_ERROR);
                fprintf(stderr, "%s", buffer);
                if (write(sockfd, buffer, FLEN) == -1)
                {
                    fprintf(stderr, "Error: %d sending information failed \
                    (result file)\n", RCC_SEND_ERROR);
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
        sprintf(buffer, "Error: %d receiving information failed \
        (result file)\n", RCC_RECEIVE_ERROR);
        fprintf(stderr, "%s", buffer);
        if (write(sockfd, buffer, FLEN) == -1)
        {
            fprintf(stderr, "Error: %d sending information failed \
            (result file)\n", RCC_SEND_ERROR);
            fclose(file_ptr);
            return RCC_SEND_ERROR;
        }
        fclose(file_ptr);
        return RCC_RECEIVE_ERROR;
    }

    //file was successfully received, send response
    printf("result file was successfully received\n");
    sprintf(buffer, "Success\n");
    if (write(sockfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (success result file)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }
    fclose(file_ptr);
    return 0;
}

static int send_number_of_files(int sockfd, long number_of_files, \
bool upgrade)
{
    char buffer[FLEN];
    sprintf(buffer, "%ld", number_of_files);
    if (write(sockfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (number of files)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }

    //get response
    if (safe_read(sockfd, buffer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed \
        (number of files)\n", RCC_RECEIVE_ERROR);
        return RCC_RECEIVE_ERROR;
    }
    if (strncmp(buffer, "Success\n", 8) != 0)
    {
        fprintf(stderr, "From server: %s", buffer);
        return RCC_RECEIVE_ERROR;
    }

    //send upgrade flag
    if (upgrade == true)
        sprintf(buffer, "Upgrade");
    else
        sprintf(buffer, "Common");

    if (write(sockfd, buffer, FLEN) == -1)
    {
        fprintf(stderr, "Error: %d sending information failed \
        (upgrade)\n", RCC_SEND_ERROR);
        return RCC_SEND_ERROR;
    }

    //get response
    if (safe_read(sockfd, buffer, FLEN) != 0)
    {
        fprintf(stderr, "Error: %d receiving information failed \
        (number of files)\n", RCC_RECEIVE_ERROR);
        return RCC_RECEIVE_ERROR;
    }
    if (strncmp(buffer, "Success\n", 8) != 0)
    {
        fprintf(stderr, "From server: %s", buffer);
        return RCC_RECEIVE_ERROR;
    }
    
    return 0;
}

int OpenConnection(const char *IP, int PORT)
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(PF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) 
    { 
        fputs("Error: socket creation failed.\n", stderr); 
        abort();
    } 
    else
        printf("Socket successfully created, socket = %d\n", sockfd); 
 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr) <= 0)
    {
        fprintf(stderr, "Error: inet_pton error for %s\n", IP);
        abort();
    }     
  
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
    { 
        fputs("Error: connection with the server failed\n", stderr); 
        abort();
    } 
    else
        printf("Connected to the server\n"); 
    return sockfd;
}
SSL_CTX* InitCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms(); 
    SSL_load_error_strings(); 
    method = TLSv1_2_client_method();
    ctx = SSL_CTX_new(method); 
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

static int safe_connection(int sockfd, bool auth, bool command)
{
    SSL_CTX *ctx;
    SSL *ssl;
    FILE* result;
    FILE* tarball;
    struct stat st;
    char buf[BUFSIZ];
    int bytes, check;
    char authdata[1024];
    char username[100];
    char password[100];
    char user_command[200];
    char folder[200];
    SSL_library_init();
    ctx = InitCTX();
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) == -1)
    {   
        ERR_print_errors_fp(stderr);
        return -1;
    }

    printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
    ShowCertificates(ssl);

    sprintf(buf, "%s", auth ? "auth" : "----");
    SSL_write(ssl, buf, 10);
    sprintf(buf, "%s", command ? "comm" : "----");
    SSL_write(ssl, buf, 10);

    if (auth)
    {    
        printf("Enter the User Name : ");
        fgets(username, 99, stdin);
  	    username[strcspn(username, "\n")] = 0;
        SSL_write(ssl, username, 100);

        printf("\n\nEnter the Password : ");
        system("stty cbreak -echo");
        fgets(password, 99, stdin);
        password[strcspn(password, "\n")] = 0;
        system("stty -cbreak echo");
        SSL_write(ssl, password, 100);
    } 
    if (command)
    {
        printf("Enter command to execute (gcc and javac are available");
        fgets(user_command, FLEN, stdin);
        user_command[strcspn(user_command, "\n")] = 0;
        if (strncmp(user_command, "gcc", 3) != 0 && strncmp(user_command, "javac", 5) != 0)
        {
            fprintf(stderr, "wrong command\n");
            return -1;
        }        
        SSL_write(ssl, user_command, 200);
    }

    printf("choose folder with code\n"); 
    fgets(folder, 200, stdin);
    folder[strcspn(folder, "\n")] = 0;   
    SSL_write(ssl, folder, sizeof(folder));   

    sprintf(buf, "tar -czvf tarball.tar.gz %s", folder);
    printf("archive command: %s\n", buf);
    system(buf);
    printf("done\n");
    tarball = fopen("tarball.tar.gz", "rb+");
    if (safe_send_file(tarball, ssl) != 0)
        return 1;
    fclose(tarball);
    printf("archive has been sent\n");

    result = fopen("result.tar.gz", "wb");
    if (safe_get_file(result, ssl) != 0)
        return -1;
    fclose(result);
    printf("result has been received\n");

    //extract an archive
    if (stat("./result", &st) == 0)
        system("rm -r ./result");
    mkdir("result", 0777);
    system("tar -xzvf result.tar.gz -C ./result");

    SSL_free(ssl);
    close(sockfd);         
    SSL_CTX_free(ctx);        
    return 0;
}

static void usage()
{
    printf("usage: ./client [-h] [-d <ip>] [-p <port>] [-n <number of files] \
    [-u] [-g] [-s]\n\
    Default options:\n\
    IP: 127.0.0.1\n\
    Port: 1234\n\
    Number of files: 1\n\
    -u for authorization (for special abilities)\
    -s for safe connection\
    -h for help.\
    -g for upgrade which can be done only by root\n");
}

int main(int argc, char** argv) 
{ 
    int sockfd;
    int i, check;                   //auxiliary variables
    int rez = 0;                    //used in getopt 
    struct sockaddr_in servaddr; 
    long PORT = RCC_PORT_DEFAULT;
    long number_of_files = 1;
    char IP[LEN] = RCC_IP_DEFAULT;
    char username[LEN];
    char password[LEN];
    bool upgrade = false;
    bool safe = false;
    bool auth = false;
    bool comm = false;

	while ((rez = getopt(argc, argv, "hd:p:n:ugcs")) != -1)
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
                auth = true;
                break;

            case 'n':
                number_of_files = strtol(optarg, NULL, 10);
                if (number_of_files <= 0 || number_of_files > 10)
                {
                    fprintf(stderr, "Error: %d Wrong number of \
                    files\n", RCC_WRONG_ARG);
                    return RCC_WRONG_ARG;
                }
                break;

            case 'g':
                upgrade = true;
                auth = true;
                safe = true;
                break;

            case 's':
                safe = true;
                break;

            case 'c':
                comm = true;
                break;

            default: 
                fprintf(stderr, "Error: %d Wrong argument\n", \
                RCC_WRONG_ARG);
                usage();
                return RCC_WRONG_ARG;
        }
	}

    sockfd = OpenConnection(IP, PORT);
    
    if (safe)
    {
        write(sockfd, "safe", 5);
        safe_connection(sockfd, auth, comm);
    }
    else
    {
        write(sockfd, "comm", 5);
        if (send_number_of_files(sockfd, number_of_files, upgrade) != 0)
            return RCC_SEND_ERROR;


        //send files
        i = 0;
        while (i < number_of_files)
        {
            if (send_file(sockfd) != 0)
                return RCC_SEND_ERROR;
            i++;
        }

        if (get_result_of_compilation(sockfd) != 0)
            return RCC_RECEIVE_ERROR;

        close(sockfd); 
    }

    return 0;
} 
