#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main()
{
    // char x[100];
    // char s[100] = "Host: push.services.mozilla.com:443";
    // char *result = strstr(s, "Host:");
    // int num, i;
    // result += 6;              // skip the first six character
    // // printf("%s\n",result);
    // char *p1 = strstr(result, ":");
    // // printf("%s",p1);
    // num = (int)(p1 - result); // calculate the length of the hostname
    // printf("num: %d\n",num);
    // for (i = 0; i < num; i++)
    // {
    //     x[i] = result[i];
    // }
    // x[i] = '\0';
    // printf("%s\n",x);
    char a[50] = "abcdasvhfd0\r\n\r\n";
    char* p = strstr(a,"0\r\n\r\n");
    if (p != NULL){
        printf("p not null!\n");
    }
    printf("%s\n",a);
    return 0;
}
