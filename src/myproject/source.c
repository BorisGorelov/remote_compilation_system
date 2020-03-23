#include "source.h"

int safe_read(int sockfd, char* ans, long size)
{
    if (read(sockfd, ans, size) < 0)
        return 1;
    if (ans[strlen(ans)] != '\0')
        return 2;
    return 0;
}

int safe_answer(int sockfd, char* ans, long size)
{
    if (read(sockfd, ans, size) < 0)
    {
        fputs("error while getting answer\n", stderr);
        return 1;
    } 
    if (ans[strlen(ans)] != '\0')
    {
        fputs("error: too long answer\n", stderr);
        return 2;
    }
    printf("answer: %s\n", ans);
    return 0;
}