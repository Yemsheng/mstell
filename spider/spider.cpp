/*
* 抓一个网页---->分析---->算MD5---->去重---->加到队列
*
*/


#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <queue>
#include <string>
using namespace std;

const int URL_SIZE = 512;
const int DOMAIN_SIZE = 128;
const int FILE_NAME_SIZE = 512;
const int BUFFERSIZE = 1024;
const int HTTP_MSG_BUFFER_SIZE = 1024;
const int ReadWriteBufSize = 2048;


char *MakeHttpSendMsgContent(char *msg, const int msgSize, const char *domain, const char *url);
char *getDomainFromUrl(const char *url, char *domainBuf, int domainBufSize);
bool spiderOnePage(const char *domain, const char *url, const char* saveFileName);
bool analyzeOnePage(const char *fileName, queue<string> *q_url, bool spiderFlag);
char *getSaveFileName(char *fileNameBuf, const int NameBufSize);


int main(int agrc, char **argv)
{

	queue<string> q_url;
	string firstPage = "http://www.dangdang.com";
	q_url.push(firstPage);
	
	string url;
	char domainBuffer[DOMAIN_SIZE];
	char saveFileName[FILE_NAME_SIZE];
	memset(domainBuffer, 0, sizeof(domainBuffer));
	while(!q_url.empty())
	{
		url = q_url.front();
		const char *p_url = url.c_str();
		fprintf(stdout, "url = %s\n", p_url);

		char *getResult = getDomainFromUrl(p_url, domainBuffer, sizeof(domainBuffer));
		if(getResult!=NULL)
		{
			memset(saveFileName, 0, sizeof(saveFileName));
			getResult = getSaveFileName(saveFileName, sizeof(saveFileName));
			if(getResult==NULL)
			{
				fprintf(stderr, "spiderOnePage func:getResult = getSaveFileName(saveFileName, sizeof(saveFileName)) fail, return");
				q_url.pop();
				continue;
			}

			bool spiderFlag = spiderOnePage(domainBuffer, p_url, saveFileName);
			bool analyzeFlag = analyzeOnePage(saveFileName, &q_url, spiderFlag);
		}
		memset(domainBuffer, 0, sizeof(domainBuffer));
		q_url.pop();
	}

	return 0;	
	
}


char *MakeHttpSendMsgContent(char *msg, const int msgSize, const char *domain, const char *url)
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


bool spiderOnePage(const char *domain, const char *url, const char* saveFileName)
{
	fprintf(stdout, "spiderOnePage function\n");

	if(domain==NULL||url==NULL||saveFileName==NULL)
	{
		fprintf(stderr, "if(domain==NULL||url==NULL||saveFileName==NULL)  return false\n"); 
		return false;
	}

	struct hostent *h; 
	char *ipAddr = NULL;
	
	
	if ((h=gethostbyname(domain)) == NULL) 
	{
			fprintf(stderr, "gethostbyname error\n"); 
			return false;
	} 
	
	printf("Host name : %s\n", h->h_name); 
	ipAddr = inet_ntoa(*((struct in_addr*)h->h_addr));
	printf("IP Address : %s\n", ipAddr); 

	int client_fd;
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd==-1)
	{
			fprintf(stderr, "open socket failed\n");
			return false;
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
			fprintf(stderr, "conncet failed\n");
			close(client_fd);
			return false;
	}

	char msg[HTTP_MSG_BUFFER_SIZE];
	MakeHttpSendMsgContent(msg, sizeof(msg), domain, url);

	int sendState = 0;
	sendState = send(client_fd, msg, strlen(msg), 0);
	if(sendState==-1)
	{
			fprintf(stderr, "send Error\n");
			close(client_fd);
			return false;
	}

	int receiveLen = 0;
	char receiveBuffer[BUFFERSIZE];

	FILE *fout;
	fout= fopen(saveFileName, "w");
	int totalReceiveLen = 0;
	while(true)
	{
			
			receiveLen = recv(client_fd, receiveBuffer,sizeof(receiveBuffer), 0);	
			if(receiveLen<=0)
				break;
			totalReceiveLen += receiveLen;
			fwrite(receiveBuffer, receiveLen, 1, fout);
	}
	fprintf(stdout, "\nurl = %s\ntotal receive len = %d byte(s)\n\n", url, totalReceiveLen);
	fclose(fout);
	close(client_fd);
	
	return true;

}


char *getDomainFromUrl(const char *url, char *domainBuf, int domainBufSize)
{
	fprintf(stdout, "getDomainFromUrl function\n");

	if(url==NULL||domainBuf==NULL)
			return NULL;


	char *httpProtocol = "http://";
	const int urlLen = strlen(url);
	const int protocolLen = strlen(httpProtocol);
	char *domainEnd = "/";
	char *isFind = strstr((char*)(url+protocolLen), domainEnd);
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

void analyzeAndAddQueue(FILE** fread_h, queue<string> *q_url);
bool analyzeOnePage(const char *fileName, queue<string> *q_url, bool spiderFlag)
{
	if(spiderFlag==false)
	{
		if(access(fileName, 0))
			remove(fileName);
		fprintf(stdout, "in analyzeOnePage func, spiderFlag==false, return");
	}
	
	FILE *file_h = fopen(fileName, "r");
	if(file_h==NULL)
	{
			fprintf(stderr, "fopen for read failed: %s\n", fileName);
	}

	analyzeAndAddQueue(&file_h, q_url);

	//remove(fileName);
}


bool hasDomain(char *url, char *domain);
void analyzeAndAddQueue(FILE** fread_h, queue<string> *q_url)
{
	if(fread_h==NULL||q_url==NULL)
	{
		return;
	}
	FILE *file_h = *fread_h;
	
		
	char readBuf[ReadWriteBufSize];
	char urlBuf[URL_SIZE];
	char *httpStr = "\"http://";
	char *httpFindIndex = NULL;
	char *httpLastIndex = NULL;
	char *quoteSymbol = "\"";
	int readLen = 0;
	while(true){
		memset(readBuf, 0, sizeof(readBuf));
		readLen = fread(readBuf, sizeof(readBuf), 1, file_h);
		if(readLen!=1)
		{
				if(feof(file_h))
						printf("read file end\n");
				else
						printf("read file error\n");
				break;
		}
		httpFindIndex = strstr(readBuf, httpStr);
		if(httpFindIndex!=NULL){
				httpFindIndex++; //"http:// jump the head char
				httpLastIndex = strstr(httpFindIndex, quoteSymbol);
				if(httpLastIndex!=NULL){
						memset(urlBuf, 0, sizeof(urlBuf));
						size_t len = httpLastIndex - httpFindIndex;
						if(len<sizeof(urlBuf)){
							strncpy(urlBuf, httpFindIndex, len);
							urlBuf[len] = '\0';

							if(!hasDomain(urlBuf, "dangdang"))
									continue;

							printf("find an url: %s\n", urlBuf);
							//add to the queue
							q_url->push(urlBuf);
						}
				}

		}
	}

}


/**
*检查url中是否有客户(举例：dangdang)
*/
bool hasDomain(char *url, char *client)
{
		if(url==NULL||client==NULL)
			return false;

		char *findclient = NULL;
		findclient = strstr(url, client);
		if(findclient!=NULL)
			return true;
		else
			return false;
}


