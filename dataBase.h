#ifndef __DATABASE_H_
#define __DATABASE_H_

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash.h"
#include "utlist.h"

struct file{
  int id;
  char* name;
  struct file *next, *prev;
};

struct tag{
  int id;
  char* name;
  struct file* headFiles;
};

void db_initTag();
void db_addTag(char* fileName, char* tagName);

#endif
