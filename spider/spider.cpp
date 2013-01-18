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
#include <time.h>
#include<sys/time.h>


const int URL_SIZE = 512;
const int DOMAIN_SIZE = 128;
const int FILE_NAME_SIZE = 512;
const int BUFFERSIZE = 1024;
const int HTTP_MSG_BUFFER_SIZE = 1024;


char *MakeHttpSendMsgContent(char *msg, const int msgSize, char *domain, char *url);
char *getDomainFromUrl(char *url, char *domainBuf, int domainBufSize);
void spiderOnePage(char *domain, char *url);

/*
* spider2 xxxxx.txt
*/
int main(int argc, char **argv) 
{ 
        char *fileName = argv[1];
        FILE *f_handle = fopen(fileName, "r");
	if(f_handle==NULL)
	{
		fprintf(stderr, "open file %s fail, exit\n", fileName);
		exit(1);
	}

	fprintf(stdout, "open file %s success\n", fileName);

        char urlBuffer[URL_SIZE];
	memset(urlBuffer, 0, sizeof(urlBuffer));
	char domainBuffer[DOMAIN_SIZE];
	memset(domainBuffer, 0, sizeof(domainBuffer));

	while(fgets(urlBuffer, sizeof(urlBuffer), f_handle))
	{
		fprintf(stdout, "fgets(urlBuffer, sizeof(urlBuffer), f_handle) success\n  urlBuffer = %s", urlBuffer);
		int lenGet = strlen(urlBuffer);
		fprintf(stdout, "lenGet = %d\n", lenGet);
		if(urlBuffer[lenGet-1]=='\n')
		{
			urlBuffer[lenGet-1]='\0';
			char *getResult = getDomainFromUrl(urlBuffer, domainBuffer, sizeof(domainBuffer));
			if(getResult!=NULL)
			{
				spiderOnePage(domainBuffer, urlBuffer);
			}
		}
		//如果url太长，丢弃

		memset(urlBuffer, 0, sizeof(urlBuffer));
		memset(domainBuffer, 0, sizeof(domainBuffer));
	}
	fclose(f_handle);
        
	return 0; 
} 

char *MakeHttpSendMsgContent(char *msg, const int msgSize, char *domain, char *url)
{
	fprintf(stdout, "MakeHttpSendMsgContent function\n");

	if(msg==NULL||msgSize<=0)
		return NULL;
	if(domain==NULL||url==NULL)
		return NULL;

	memset(msg, 0, msgSize);
	int leftSize = msgSize;

	snprintf(msg, leftSize, "GET ");

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		//strncat(msg, "http://product.dangdang.com/product.aspx?product_id=1039656721", leftSize);
		strncat(msg, url, leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)		
		strncat(msg, " HTTP/1.1", leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, "\r\nHost: ", leftSize);

	leftSize = msgSize - strlen(msg);
	if(leftSize>0)
		strncat(msg, domain, leftSize);

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


char *getSaveFileName(char *fileNameBuf, const int NameBufSize)
{
	fprintf(stdout, "getSaveFileName function\n");

	if(fileNameBuf==NULL)
		return NULL;

	struct  timeval    tv;
        struct  timezone   tz;
        gettimeofday(&tv,&tz);
	char timerStr[256];
	memset(timerStr, 0, sizeof(timerStr));
	sprintf(timerStr,"%d%d", tv.tv_sec, tv.tv_usec);

	memset(fileNameBuf, 0, NameBufSize);
	strncpy(fileNameBuf, timerStr, NameBufSize);
	const int fileNameLen = strlen(fileNameBuf);
	char *postfix = ".html";
	const int postFixLen = strlen(postfix);
	if(fileNameLen+postFixLen < NameBufSize)
	{
		strcat(fileNameBuf, postfix);
		printf("save file name = %s\n", fileNameBuf);
		return fileNameBuf;
	}

	return NULL;
}


void spiderOnePage(char *domain, char *url)
{
	fprintf(stdout, "spiderOnePage function\n");

	if(domain==NULL||url==NULL)
		return;


	char *getResult = NULL;

	char saveFileName[FILE_NAME_SIZE];
	memset(saveFileName, 0, sizeof(saveFileName));
	getResult = getSaveFileName(saveFileName, sizeof(saveFileName));
	if(getResult==NULL)
	{
		fprintf(stderr, "spiderOnePage func:getResult = getSaveFileName(saveFileName, sizeof(saveFileName)) fail, return");
		return;
	}

	struct hostent *h; 
	char *ipAddr = NULL;
	
	
	if ((h=gethostbyname(domain)) == NULL) 
	{
			herror("gethostbyname"); 
			return;
	} 
	
	printf("Host name : %s\n", h->h_name); 
	ipAddr = inet_ntoa(*((struct in_addr*)h->h_addr));
	printf("IP Address : %s\n", ipAddr); 

	int client_fd;
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd==-1)
	{
			perror("socket failed\n");
			return;
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
			return;
	}

	char msg[HTTP_MSG_BUFFER_SIZE];
	MakeHttpSendMsgContent(msg, sizeof(msg), domain, url);

	int sendState = 0;
	sendState = send(client_fd, msg, strlen(msg), 0);
	if(sendState==-1)
	{
			perror("send Error\n");
			return;
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

}


char *getDomainFromUrl(char *url, char *domainBuf, int domainBufSize)
{
	fprintf(stdout, "getDomainFromUrl function\n");

	if(url==NULL||domainBuf==NULL)
			return NULL;


	char *httpProtocol = "http://";
	const int urlLen = strlen(url);
	const int protocolLen = strlen(httpProtocol);
	char *domainEnd = "/";
	char *isFind = strstr(url+protocolLen, domainEnd);
	int domainLen = 0;
	if(isFind==NULL)
	{
			domainLen = urlLen - protocolLen;
	}
	else
	{
			domainLen = isFind - url - protocolLen;
	}

	if(domainLen>=domainBufSize)
			return NULL;

	strncpy(domainBuf, url+protocolLen, domainLen);
	domainBuf[domainLen] = '\0';

	return domainBuf;
}



















