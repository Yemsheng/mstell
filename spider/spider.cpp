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


const int URL_SIZE = 512;
const int FILE_NAME_SIZE = 512;
const int BUFFERSIZE = 1024;
const int HTTP_MSG_BUFFER_SIZE = 1024;

char *g_domain;
char g_url[URL_SIZE];

char *MakeHttpSendMsgContent(char *msg, const int msgSize);

int main(int argc, char *argv[]) 
{ 
		if(argc!=2)
		{
			perror("argc != 2");
			exit(1);
		}
		g_domain = argv[1];
		memset(g_url, 0, sizeof(g_url));
		sprintf(g_url, "http://");
		strcat(g_url, g_domain);
		strcat(g_url, "/");

		char saveFileName[FILE_NAME_SIZE];
		memset(saveFileName, 0, sizeof(saveFileName));
		sprintf(saveFileName, g_domain);
		strcat(saveFileName, ".html");

		struct hostent *h; 
		char *ipAddr = NULL;
		
		
		if ((h=gethostbyname(g_domain)) == NULL) 
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

		char msg[HTTP_MSG_BUFFER_SIZE];
		MakeHttpSendMsgContent(msg, sizeof(msg));

		int sendState = 0;
		sendState = send(client_fd, msg, strlen(msg), 0);
		if(sendState==-1)
		{
				perror("send Error\n");
				exit(1);
		}

		int receiveLen = 0;
		char receiveBuffer[BUFFERSIZE];

		FILE *fout;
		fout= fopen(saveFileName, "w");
		while(true)
		{
				
				receiveLen = recv(client_fd, receiveBuffer,sizeof(receiveBuffer), 0);	
				printf("receive len = %d\n", receiveLen);
				if(receiveLen<=0)
					break;
				fwrite(receiveBuffer, receiveLen, 1, fout);
		}
		fclose(fout);
		close(client_fd);

		return 0; 
} 

char *MakeHttpSendMsgContent(char *msg, const int msgSize)
{
	if(msg==NULL||msgSize<=0)
		return NULL;

	memset(msg, 0, msgSize);
	int leftSize = msgSize;

	snprintf(msg, leftSize, "GET ");

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		//strncat(msg, "http://product.dangdang.com/product.aspx?product_id=1039656721", leftSize);
		strncat(msg, g_url, leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)		
		strncat(msg, " HTTP/1.1", leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, "\r\nHost: ", leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, g_domain, leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, "\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:15.0) Gecko/20100101 Firefox/15.0.1", leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, "\r\nAccept: */*", leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, "\r\nConnection: close\r\n\r\n", leftSize);

	printf("%s\ntoatl msgBuffer size = %d  msglen = %d  leftSize = %d\n",msg,msgSize, strlen(msg), leftSize);
	
	return msg;
}
