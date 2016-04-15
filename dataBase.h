#ifndef __DATABASE_H_
#define __DATABASE_H_

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash.h"
#include "utlist.h"

#define TAGNAME_SIZE 128

struct file{
  char* name;
  struct file *next, *prev;
};

struct tag{
  char name[TAGNAME_SIZE];
  struct file* headFiles;
  UT_hash_handle hh;         /* makes this structure hashable */
};


//add a tag in the hashtable
void db_addTag(char* fileName, char* tagName);

//return files by tag
struct file* db_getFiles(char* tagName);


void test(void);


#endif
