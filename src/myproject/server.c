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
    char command[FLEN] = "gcc -Wall -o out *.c > errors 2>&1";
    return system(command);
}

static int upgrade_compile()
{
    char command[FLEN] = "gcc -Wall -o ../server *.c > errors 2>&1";
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


static int OpenListener(int PORT)
{
    int sockfd;
    struct sockaddr_in servaddr;

    // socket creation and verification 
    sockfd = socket(PF_INET, SOCK_STREAM, 0); 
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
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) 
    { 
        fputs("Error: socket bind failed.\n", stderr); 
        abort(); 
    } 
    else
        printf("Socket successfully binded.\n"); 
  
    // Now server is ready to listen and verification 
    if (listen(sockfd, LISTEN_BACKLOG) != 0) 
    { 
        fputs("Error: Listen failed.\n", stderr); 
        abort(); 
    } 
    else
        printf("Server listening.\n"); 
    return sockfd;
}


static SSL_CTX* InitServerContext(void)
{
    SSL_METHOD *method;
    SSL_CTX *context;
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    method = TLSv1_2_server_method();
    context = SSL_CTX_new(method);   
    if ( context == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return context;
}

static void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) 
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}


static int exec_and_send(char command[], char arch[], SSL* ssl)
{
    FILE* result;
    if(system(command) != 0)
        system("tar -czvf result.tar.gz errors");
    else
    {
        chdir("../");
        system(arch);
    }
    result = fopen("result.tar.gz","rb+");
    if (safe_send_file(result, ssl) != 0)
        return 1;
    fclose(result);
    return 0;
}

static int safe_servlet(SSL* ssl, char* passwords_file_name) 
{
    char buf[1024];
    char buffer[BUFSIZ];
    int sd, bytes, check;
    char username[100];
    char password[100];
    char command[200];
    char codefolder[200];
    char arch[50];
    bool auth_complete = false;
    bool auth;
    bool comm;
    FILE* pass_ptr;
    FILE* tarball;
    FILE* result;
    struct stat st;
    bool make = false, mvn = false; 

    if (SSL_accept(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    ShowCertificates(ssl);     
    SSL_read(ssl, buf, 10);
    auth = strncmp(buf, "auth", 4) == 0 ? 1 : 0;
    SSL_read(ssl, buf, 10);
    comm = strncmp(buf, "comm", 4) == 0 ? 1 : 0;

    //authentication
    if (auth)
    {   
        SSL_read(ssl, username, 100);
        SSL_read(ssl, password, 100);

        //open file with passwords
        pass_ptr = fopen(passwords_file_name, "r");

        //data validation
        while (fgets(buf, FLEN, pass_ptr))
        {
            buf[strcspn(buf, "\n")] = 0;
            if (strncmp(buf, username, strlen(username)) == 0)
            {
                fgets(buf, FLEN, pass_ptr);
                buf[strcspn(buf, "\n")] = 0;
                if (strncmp(buf, password, strlen(password)) == 0)
                {
                    SSL_write(ssl, "Success", strlen("Success"));
                    auth_complete = true;
                }
            }
        }
        if(!auth_complete)
        {
            SSL_write(ssl, "wrong auth data", strlen("wrong auth data"));
            return -1;
        }

        //change directory for user's files
        sprintf(buf, "./%s", username);
        if (stat(buf, &st) == 0)
            mkdir(buf, 0777);
        chdir(buf);
    }
    else 
    {
        //create empty directory for files
        if (stat("./source", &st) == 0)
            system("rm -r ./source");
        mkdir("source", 0777);
        chdir("source");
    }

    if(comm)
        SSL_read(ssl, command, sizeof(command));
    
    SSL_read(ssl, codefolder, sizeof(codefolder));
    //get tarball
    sprintf(buf, "%s%s", username, "archive.tar.gz");
    printf("archive name: %s\n", buf);
    tarball = fopen(buf,"wb+");
    if (safe_get_file(tarball, ssl) != 0)
        return 1;
    fclose(tarball);
    printf("got archive\n");

    //extract an archive
    mkdir("extracted", 0777);
    system("tar -xzvf archive.tar.gz -C ./extracted");
    chdir("extracted");
    chdir(codefolder);

    //if user specialized command, run it
    if (comm)
    {
        sprintf(arch, "tar -czvf result.tar.gz ./extracted");
        if (exec_and_send(command, arch, ssl) == 0)
            return 0;
        else
            return 1;
    }

    //run make or maven
    if (system("find -name '[mM]akefile' | read REPLY") == 0)
    {
        make = true;
        sprintf(buf, "%s", "make");
    }
    if (system("find -name 'pom.xml' | read REPLY") == 0)
    {
        mvn = true;
        sprintf(buf, "%s", "mvn clean package");
    }

    if (make || mvn)
    {
        sprintf(buf, "%s%s", buf, " > errors 2>&1");
        sprintf(arch, "tar -czvf result.tar.gz ./extracted");
        if (exec_and_send(buf, arch, ssl) == 0)
            return 0;
        else
            return 1;
    }
    //check file extensions
    if (system("find -name '*.c' | read REPLY") == 0)
    //.c file have been found
    {
        sprintf(buf, "gcc -Wall -o out *.c > errors 2>&1");
        sprintf(arch, "tar -czvf result.tar.gz ./out ./errors");
        if (exec_and_send(buf, arch, ssl) == 0)
            return 0;
        else
            return 1;
    }
    else if (system("find -name '*.java' | read REPLY") == 0)
    {
        sprintf(buf, "javac -d bin '*.java' > errors 2>&1");
        sprintf(arch, "tar -czvf result.tar.gz ./bin ./errors");
        if (exec_and_send(buf, arch, ssl) == 0)
            return 0;
        else
            return 1;
    }
    else
    {
        fprintf(stderr, "unidentified file type\n");
        return -1;
    }
    sd = SSL_get_fd(ssl);     
    SSL_free(ssl);         
    close(sd);          
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
    char type[5];
    bool upgrade = false;
    SSL_CTX* ctx;

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

    SSL_library_init();
    ctx = InitServerContext();
    sockfd = OpenListener(PORT);
    LoadCertificates(ctx, "mycert.pem", "mycert.pem");

    while(1)
    {    // Accept the data packet from client and verification
        connfd = accept(sockfd, NULL, NULL); 
        if (connfd == -1) 
        { 
            fputs("Error: server acccept failed.\n", stderr); 
            return RCC_ACCEPT_ERROR; 
        } 
        else
            printf("Server acccept the client.\n"); 

        read(connfd, type, 5);
        if (strncmp(type, "safe", 4) == 0)
        {
            SSL *ssl;
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, connfd);
            safe_servlet(ssl, passwords_file_name);
        }
        else
        {
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
        }      
    }

    close(sockfd); 
    return 0;
} 