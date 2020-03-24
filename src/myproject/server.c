#include "source.h"
#define ANS0 "0. the name is received.\n"
#define ANS1 "1. error occurred while recieving name\n"
#define ANS_ERR_EXE "an error occurred while creating an executable command\n"
#define ANS_COMPILE_0 "compilation complited successfully\n"
#define ANS_COMPILE_1 "compilation failed\n"

int get_name(int sockfd, char* serv_name)
{
    char client_name[FLEN];
    int check;

    if(safe_read(sockfd, client_name, FLEN) == 0)
        write(sockfd, ANS0, sizeof(ANS0));
    else
    {
        write(sockfd, ANS1, sizeof(ANS1));
        fputs("error while recieving name\n", stderr);
        return 1;
    }
    printf("From client: name of file: %s\n", client_name);

    check = snprintf(serv_name, FLEN, "serv_%s", client_name);
    if (check <= 5 || check >= FLEN)
    {
        fputs("error occurred while creating serv_name\n", stderr);
        return 1;
    }
    return 0;
}

int get_file(int sockfd, char* serv_name)
{
    FILE* file_ptr = fopen(serv_name, "w"); 
    char fbuff[FLEN];

    if(file_ptr == NULL)
    {
        sprintf(fbuff, "unable to open file %s\n", serv_name);
        fprintf(stderr, "%s", fbuff);
        write(sockfd, fbuff, FLEN);
        return 1;
    }
    printf("file was opened successfully\n");

    //recieving file
    if(safe_read(sockfd, fbuff, FLEN) == 0){
        while(strncmp("^^^^^", fbuff, 5) != 0)
        {
            fputs(fbuff, file_ptr);
            if(safe_read(sockfd, fbuff, FLEN) != 0)
                return 2;
        }
    }
    else return 2;

    printf("file was recieved\n");
    fclose(file_ptr);
    return 0;
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
    safe_answer(sockfd, fbuff, FLEN);

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

int get_number_of_files(int connfd, long int* number)
{
    char buf[FLEN];
    if(safe_read(connfd, buf, FLEN) != 0)
    {   
        fputs("error while getting number of files\n", stderr);
        return 1;
    }
    *number = strtol(buf, NULL, 10);
    if(*number < 0)
    {
        fputs("fatal error: wrong number of files\n", stderr);
        return 2;
    }
    return 0;
}
  
void usage()
{
    printf("usage: ./server [-p <port>]\n");
}

int main(int argc, char** argv) 
{ 
    long int i, number_of_files, PORT = RCC_PORT_DEFAULT;
    int sockfd, connfd, rez=0; 
    struct sockaddr_in servaddr; 
    char serv_name[FLEN] = "serv_";
    struct stat st;

	while ( (rez = getopt(argc,argv,"hp:")) != -1)
    {
		switch (rez)
        {
		case 'h': 
            usage(); 
            return 0;
		case 'p':
            PORT = strtol(optarg, NULL, 10);
            if(PORT < 0 || PORT > 65535)
            {
                fputs("fatal error. wrong port.\n", stderr);
                return RCC_WRONG_PORT;
            }
            break;
		case '?': 
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
  
    if(get_number_of_files(connfd, &number_of_files) != 0)
        return RCC_UNEXPEC_VAL;

    //getting files
    if (stat("/source", &st) != 0)
        mkdir("source", 0777);
    chdir("source");

    i = 0;
    while(i < number_of_files)
    {
        bzero(serv_name, sizeof(serv_name));
        if (get_name(connfd, serv_name) != 0)
            return RCC_RECEIVE_FAIL;

        if (get_file(connfd, serv_name) != 0)
            return RCC_RECEIVE_FAIL;
        i++;
    }    

    if (compile(connfd, serv_name) != 0)
        return RCC_COMPILE_ERROR;

    close(sockfd); 
    return 0;
} 
