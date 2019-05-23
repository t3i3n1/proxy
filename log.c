#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    char* message;
    int fd = open("log.txt", O_APPEND | O_CREAT | O_RDWR, 0644);
    if (fd > 0){
        message = "success\n!";
        int num = write(fd,message, sizeof(message));
        if (num < 0){
            printf("write error\n");
        }
        close(fd);
    }else{
        printf("fail to open the log file! \n");
    }
}