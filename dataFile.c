#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX 128

int readDataFile(const char *path){

	FILE* dataFile;
	char line[MAX];
	char* fileName = NULL;
	dataFile =  fopen(path,"r");
	if(f==NULL){
		perror("Fichier introuvable");
	}
	else{
		while((line=fgets(line,MAX,dataFile))!=NULL){
			fileName = processLine(line,fileName);
		}
	}
	return 0;
}

char* processLine(char* line,char* fileName){
	char c = line[0];
	int commentLine = 0;
	int tagLine = 0;
	int fileLine = 0;
	if(c=='['){
		fileLine = 1;
		int i=0;
		while(i<MAX){
			c[i]=c[i+1];
			if(c[i]==']'){
				c[i]='\0';
				break;
				return line;
			}
			else if(c[i]=='\0')
			{
				return -1;
			}
			i++;
		}
	}else if(c=='#' || c=='\0'){
		commentLine = 1;
	}else{
		tagLine = 1;
	}
}