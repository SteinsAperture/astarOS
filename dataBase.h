#ifndef __DATABASE_H_
#define __DATABASE_H_

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash.h"
#include "utlist.h"

#define TAGNAME_SIZE 128


struct hashElt{
  char * name;
  struct hashElt * nextLvl;
  UT_hash_handle hh;         /* makes this structure hashable */
};

struct fileNode{
  struct hashElt* file;
  struct fileNode *next, *prev;
};

//add a tag in the hashtable
void db_addTag(char* fileName, char* tagName);

//return files by tag
struct fileNode* db_getFileList(char* tagName);

int db_linkExist(char* fileName,char* tagName);

void db_deleteFileList(struct fileNode * fileList);

void db_addFile(char* fileName);
int db_tagExist(char* tagName);

void db_deleteTag(char* fileName, char* tagName);
void db_deleteFile(char* fileName);


void db_deleteTagTable();
void db_deleteFileTable();


#endif
