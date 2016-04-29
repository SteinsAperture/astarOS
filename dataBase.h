#ifndef __DATABASE_H_
#define __DATABASE_H_

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash.h"
#include "utlist.h"

#define TAGNAME_SIZE 128

struct tagHash{
  char * name;
  struct fileNode* headFiles;
  UT_hash_handle hh;         /* makes this structure hashable */
};

struct fileHash{
  char * name;
  struct tagHash* headTags;
  UT_hash_handle hh;
};

struct fileNode{
  struct fileHash* file;
  struct fileNode *next, *prev;
};

//add a tag in the hashtable
void db_addTag(char* fileName, char* tagName);

//return files by tag
struct fileNode* db_getFileList(char* tagName);

struct tagHash* db_linkExist(char* fileName,char* tagName);

struct fileHash* db_getAllFiles();

void db_deleteFileList(struct fileNode * fileList);

void db_addFile(char* fileName);
int db_tagExist(char* tagName);


void db_deleteTagTable();
void db_deleteFileTable();

void test(void);


#endif
