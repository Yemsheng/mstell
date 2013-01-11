#include<stdio.h>
#include<stdlib.h>
#include"common.h"
#include<string.h>


void buildUrlFile(FILE** fread_h, const char *freadName);

int main(int argc, char **argv)
{
		if(argc!=2)
		{
				perror("argc!=2 exit(1)\n");
				exit(1);
		}
		char *fileName = argv[1];

		FILE *file_h = fopen(fileName, "r");
		if(file_h==NULL)
		{
				fprintf(stderr, "fopen for read failed: %s\n", fileName);
		}

		buildUrlFile(&file_h, fileName);

		
		fclose(file_h);


		return 0;

}


void buildUrlFile(FILE** fread_h, const char *freadName)
{
	if(fread_h==NULL||freadName==NULL)
	{
		return;
	}
	FILE *file_h = *fread_h;
	
	int freadNameLen = strlen(freadName);
	char *urlFileNamePostfix = "_url.txt";
	int fwriteUrlNameLen = freadNameLen + strlen(urlFileNamePostfix) + 1;
	char *fwriteUrlName = new char[fwriteUrlNameLen];
	memset(fwriteUrlName, 0, fwriteUrlNameLen);
	strcpy(fwriteUrlName, freadName);
	strcat(fwriteUrlName, urlFileNamePostfix);
	
	FILE *fWriteUrl_h = fopen(fwriteUrlName, "w");
		
	char readBuf[ReadWriteBufSize];
	char urlBuf[UrlLength];
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
							printf("find an url: %s\n", urlBuf);
							fwrite(urlBuf, strlen(urlBuf), 1, fWriteUrl_h);
							fwrite("\n", 1, 1, fWriteUrl_h);
						}
				}

		}
	}

	fclose(fWriteUrl_h);
	delete fwriteUrlName;
}
