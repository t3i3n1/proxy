#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define MAXLINE 1024
#define PROXY_SERVER_PORT 12345
#define LISTENNQ 5
#define HTTP_HEADER_LAST_CHAR_NUM 4

/*
    https://blog.51cto.com/laokaddk/989911 - set timeout
    https://zake7749.github.io/2015/03/17/SocketProgramming/ 
    https://github.com/vmsandeeprao/HTTP-proxy-server/blob/master/proxy.c
    SO_REUSEPORT
*/

/*
    Client ------> Proxy ------> Server
*/

/* helper function

        void get_hostname(char* packet,char* hostname) - extract the hostname string from 'packet' and return to 'hostname'
        int http_or_https(char*) - return 1 if CONNECT method is used, 0 otherwise
        void log_init() - open a log file for debug purpose
        //void prepare_https_header(char* in, char*out, int fd) - delete the 'Proxy-Connection' from 'in' and return to 'out'
        int establish_connection(char* msg, int client_fd) - 
        void read_msg(int fd, char* msg) - read from the file descriptor and put the header messages into msg
        void reconstruct_http_header
*/

void reconstruct_http_header(char *msg, char *new_msg)
{
    char *method_p;
    method_p = strstr(msg, " ");
    if (method_p == NULL)
    {
        printf("strstr error\n");
    }
    char hostname[50];
    get_hostname(msg, hostname);
    //printf("hostname: %s\n",hostname);
    int host_length = strlen(hostname) + 7; // 7 for http://
    //printf("host name length: %d\n", host_length);
    strncpy(new_msg, msg, method_p - msg + 1);
    int i = method_p - msg + 1;
    for (; i < strlen(msg); i++)
    {
        new_msg[i] = msg[i + host_length];
    }
    //printf("new_msg: \n%s", new_msg);

    return;
}

int establish_connection(char *msg, int client_fd)
{
    //printf("message: \n%s", msg);
    char hostname[100] = {0};
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int server_fd = 0;
    int s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    //hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    //hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0;       /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    /* bind the destination ip to a socket, then send back 200 OK to the source*/
    get_hostname(msg, hostname);
    s = getaddrinfo(hostname, "443", &hints, &result); // port = 443

    if (s != 0)
    {
        printf("getaddrinfo error: %s\n", gai_strerror(s));
        freeaddrinfo(result); /* No longer needed */
        return -1;
    }
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        server_fd = socket(rp->ai_family, rp->ai_socktype,
                           rp->ai_protocol);
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        {
            printf("setsockopt(SO_REUSEADDR) failed");
        }
        if (connect(server_fd, rp->ai_addr, rp->ai_addrlen) < 0)
        {
            printf("https_connection Error: fail to connect\n");
        }
        else
        {
            freeaddrinfo(result); /* No longer needed */
            break;                // established connection
        }
    }
    if (rp == NULL)
    { /* No address succeeded */
        printf("https_connection error: Could not connect to any address\n");
        freeaddrinfo(result); /* No longer needed */
        return -1;
    }
    /* connected successfully, now construct the response message and send back to client*/
    printf("connect successful\n");
    if (connect(server_fd, rp->ai_addr, rp->ai_addrlen) < 0)
    {
        printf("this is normal!\n");
    }
    char response_msg[50] = {0};
    sprintf(response_msg, "%s", "HTTP/1.1 200 Connection Established\r\n\r\n");
    if (write(client_fd, response_msg, strlen(response_msg)) < 0)
    {
        printf("write to client error!\n");
    }
    printf("write response code 200 to client!\n");
    return server_fd;
}

// remember to close the file
// void log_init(int *fd)
// {
//     *fd = open("log.txt", O_APPEND | O_CREAT | O_RDWR, 0644);
//     if (*fd > 0)
//     {
//         printf("log init success!\n");
//     }
//     else
//     {
//         printf("fail to open the log file! \n");
//     }
// }

// void prepare_https_header(char* in, char* out, int fd)
// {
//     char temp[500];
//     char* p = strstr(in,"Proxy-Connection");
//     *p = '\0';
// 	char* s = strcpy(temp,in);
// 	int i = 0;
// 	char* pointer;
// 	for(pointer = p; *pointer != '\n';pointer++){
// 		i++;
// 	}
//     p+=i+1;
//     char result[1000];
// 	sprintf(result,"%s%s",s,p);
//     write(fd,result,strlen(result));
//     write(fd,'\n',1);
//     strcpy(out,result);
//     return;
// }

void get_hostname(char *msg, char *hostname)
{
    char *result = strstr(msg, "Host:");
    if (result == NULL)
    {
        return;
    }
    //printf("result: \n%s\n", result);
    int num, i;
    result += 6; // skip the first six character
    //printf("result:\n%s\n",result);
    char *p1;
    char *p2;
    p1 = strstr(result, "\n");
    p2 = strstr(result, "\r");
    if (p2 > p1)
    {
        /* host not in the last line*/
        num = (int)(p1 - result); // calculate the length of the hostname
        printf("hostname length: %d\n", num);
        for (i = 0; i < num; i++)
        {
            hostname[i] = result[i];
        }
        hostname[i] = '\0';
    }else
    {
        /* host is in the last line */
        num = (int)(p2 - result); // calculate the length of the hostname
        printf("hostname length: %d\n", num);
        for (i = 0; i < num; i++)
        {
            hostname[i] = result[i];
        }
        hostname[i] = '\0';
    }
    return;
}

int http_or_https(char *str)
{
    if (strstr(str, "CONNECT") == NULL)
    {
        return 0;
    }
    else
        return 1;
}

void read_msg(int fd, char *msg)
{
    char last4chars[HTTP_HEADER_LAST_CHAR_NUM + 1] = {0};
    char *terminator = "\r\n\r\n";
    int i = 0, n = 0, j;
    char buff = 0;
    while (i < MAXLINE)
    {
        n = read(fd, &buff, sizeof(buff));

        if (n <= 0)
        {
            break;
        }

        msg[i++] = buff;

        /* retrive last 4 chars */
        for (j = 0; j < HTTP_HEADER_LAST_CHAR_NUM; ++j)
        {
            last4chars[j] = msg[i - HTTP_HEADER_LAST_CHAR_NUM + j];
        }

        /* HTTP header end with "\r\n\r\n" */
        if (strcmp(last4chars, terminator) == 0)
        {
            break;
        }
    }
    msg[i] = '\0';
    return;
}

int main(int argc, char **argv)
{
    int destination_fd = 0;
    int listenfd, connfd, logfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);
    char recv_msg[MAXLINE] = {0};
    char https_res[MAXLINE] = {0};
    char ip_str[INET_ADDRSTRLEN] = {0};
    char buff = 0;
    int n, i, j;
    char hostname[100];

    /* initialize the log file*/
    //log_init(&logfd);

    /* initialize server socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0); /* SOCK_STREAM : TCP */
    if (listenfd < 0)
    {
        printf("Error: init socket\n");
        return 0;
    }
    //REUSEPORT
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed");
    }
    /* initialize server address (IP:port) */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;        /* IP address: 0.0.0.0 */
    servaddr.sin_port = htons(PROXY_SERVER_PORT); /* port number */

    /* bind the socket to the server address */
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("Error: bind\n");
        return 0;
    }

    if (listen(listenfd, LISTENNQ) < 0)
    {
        printf("Error: listen\n");
        return 0;
    }

    /* keep processing incoming requests */
    while (1)
    {
        /* accept an incoming connection from the remote side */
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0)
        {
            printf("Error: accept\n");
            return 0;
        }
        read_msg(connfd, recv_msg);
        //printf("msg: \n%s", recv_msg);
        if (http_or_https(recv_msg))
        {
            return 0;
            /* https connection */
            destination_fd = establish_connection(recv_msg, connfd); // establish https connection
            if (destination_fd == -1)
            {
                printf("https connection fail\n");
                //return 0;
                break;  
            }
            int num_bytes;
            time_t start_t, end_t;
            time(&start_t);
            while (difftime(time(&end_t), start_t) < 10) // 10 sec timeout
            {
                memset(https_res, 0, MAXLINE);
                num_bytes = recv(connfd, https_res, MAXLINE, MSG_DONTWAIT);
                if (num_bytes > 0)
                {
                    time(&start_t); // reset the start time
                    if (send(destination_fd, https_res, num_bytes, MSG_DONTWAIT) < 0)
                    {
                        printf("write to server error, error message: %s\n", strerror(errno));
                    }
                }
                else if (num_bytes < 0)
                {
                    printf("browser error: %s\n", strerror(errno));
                }
                else if (num_bytes == 0)
                {
                    printf("browser shutdown or zero-length datagrams!\n");
                }
                memset(https_res, 0, MAXLINE);
                num_bytes = recv(destination_fd, https_res, MAXLINE, MSG_DONTWAIT);
                if (num_bytes > 0)
                {
                    time(&start_t); // reset the start time
                    if (send(connfd, https_res, num_bytes, MSG_DONTWAIT) < 0)
                    {
                        printf("write to browser error, error message: %s\n", strerror(errno));
                    }
                }
                else if (num_bytes < 0)
                {
                    printf("server error: %s\n", strerror(errno));
                }
                else
                {
                    printf("server shutdown!\n");
                }
            }
            // printf("out of while loop!\n");
            // return 0;
        }
        else
        {
            /* http connection */
            printf("http message: \n%s\n", recv_msg);
            char reconstructed_msg[500];
            reconstruct_http_header(recv_msg,reconstructed_msg);
            printf("new msg: \n%s\n",reconstructed_msg);
            // destination_fd = establish_connection(recv_msg, connfd); // establish https connection
            // if (destination_fd == -1)
            // {
            //     printf("https connection fail\n");
            //     break;
            // }


        }

        //printf("exit https_connection\n");

        //printf("%s", recv); // print message
        /* close the connection */
        //close(connfd);
    }

    return 0;
}
