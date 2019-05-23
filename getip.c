#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
// 	write(fd,result,strlen(result));
// 	write(fd,'\n',1);
//     strcpy(out,result);
//     return;
// }

int main()
{
	int s;
	char ip[100] = {0};
	struct sockaddr_in *h;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	//hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0; /* Any protocol */
	s = getaddrinfo("google.com:443", "80 ", &hints, &result);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return 0;
	}
	rp = result;
	h = (struct sockaddr_in *)rp->ai_addr;
	strcpy(ip, inet_ntoa(h->sin_addr));
	printf("ip: %s\n", ip);
	printf("port: %hu\n", htons(h->sin_port));
	return 0;
}
