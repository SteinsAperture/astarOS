#ifndef _DATA_BASE_
#define _DATA_BASE_

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash.h"
#include "utlist.h"

typedef struct file{
  int id;
  char* name;
  struct file *next, *prev;
};

typedef struct tag{
  int id;
  char* name;
  file* headFiles;
};

void initTag();
void addTag(char* fileName, char* tagName);

#endif
