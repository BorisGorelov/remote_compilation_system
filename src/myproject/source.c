#include "source.h"

int safe_read(int sockfd, char* ans, long size)
{
    if (read(sockfd, ans, size) < 0)
        return 1;
    if (ans[strlen(ans)] != '\0')
        return 2;
    return 0;
}