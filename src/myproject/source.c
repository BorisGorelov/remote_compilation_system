#include "source.h"

int safe_read(int sockfd, char* ans, long size)
{
    if (read(sockfd, ans, size) < 0)
        return 1;
    if (ans[strlen(ans)] != '\0')
        return 2;
    return 0;
}

int safe_send_file(FILE* file_ptr, SSL* ssl)
{
    char buffer[BUFSIZ];
    int bytes;

    if (file_ptr == NULL) 
    {
        perror("open");
        return 3;
    }       
    //read(file_ptr, buffer, BUFSIZ)
    //fread(buffer, BUFSIZ, 1, file_ptr)
    while (fgets(buffer, BUFSIZ, file_ptr))
        if (SSL_write(ssl, buffer, BUFSIZ) == -1) 
        {
            perror("ssl_write");
            return 2;
        }
    SSL_write(ssl, MYEOF, sizeof(MYEOF));
    printf("safe_send: myeof has been sent\n");
    return 0;
}

int safe_get_file(FILE* file_ptr, SSL* ssl)
{
    char buffer[BUFSIZ];
    int bytes;
    if (file_ptr == NULL) 
    {
        perror("open");
        return 3;
    }
    if(SSL_read(ssl, buffer, BUFSIZ) == -1)
    {
        perror("ssl_read");
        return 1;
    }
    while (strncmp(buffer, MYEOF, 5) != 0)
    {
        if(SSL_read(ssl, buffer, BUFSIZ) == -1)
        {
            perror("ssl_read");
            return 1;
        }
        //write(file_ptr, buffer, bytes)
        //fwrite(buffer, bytes, 1, file_ptr)
        fputs(buffer, file_ptr);
    }
    printf("safe_get: done\n");
    return 0;
}


void ShowCertificates(SSL* ssl)
{
    X509 *certificate;
    char *line;
    certificate = SSL_get_peer_certificate(ssl);
    if ( certificate != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(certificate), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(certificate), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(certificate);
    }
    else
        printf("No certificates.\n");
}