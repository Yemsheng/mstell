#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{ 
		if(argc!=2)
		{
			perror("argc != 2");
			exit(1);
		}
		char *domain = argv[1];
		char url[512];
		memset(url, 0, sizeof(url));
		sprintf(url, "http://");
		strcat(url, domain);
		strcat(url, "/");

		struct hostent *h; 
		char *ipAddr = NULL;
		
		
		if ((h=gethostbyname(domain)) == NULL) 
		{
				herror("gethostbyname"); 
				exit(1); 
		} 
		
		printf("Host name : %s\n", h->h_name); 
		ipAddr = inet_ntoa(*((struct in_addr*)h->h_addr));
		printf("IP Address : %s\n", ipAddr); 

		int client_fd;
		client_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(client_fd==-1)
		{
				perror("socket failed\n");
				exit(1);
		}

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(80);
		server_addr.sin_addr = *((struct in_addr*)h->h_addr);
		
		int connectState;
		connectState = connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		if(connectState==-1)
		{
				perror("conncet failed\n");
		}

		char msg[1024];
		memset(msg, 0, sizeof(msg));
		sprintf(msg,"GET ");
		strcat(msg, url);
		strcat(msg, " HTTP/1.1");

		strcat(msg, "\r\nHost: ");
		strcat(msg, domain);

		strcat(msg, "\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:15.0) Gecko/20100101 Firefox/15.0.1");
		strcat(msg, "\r\nAccept: */*");
		strcat(msg, "\r\nConnection: close\r\n\r\n");
		printf("%s\n",msg);

		int sendState = 0;
		sendState = send(client_fd, msg, strlen(msg), 0);
		if(sendState==-1)
		{
				perror("send Error\n");
				exit(1);
		}

		int receiveLen = 0;
		char receiveBuffer[1024];
		while(true)
		{
				
				receiveLen = recv(client_fd, receiveBuffer,sizeof(receiveBuffer), 0);	
				printf("receive len = %d\n", receiveLen);
				if(receiveLen<=0)
					break;
		}
		close(client_fd);

		return 0; 
} 
