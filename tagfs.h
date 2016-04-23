#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include "dataFile.h"
#include "dataBase.h"

struct tagNode{
  char *name;
  struct tagNode *next, *prev;
};

struct request{
  char *file;
  char *realpath;
  struct tagNode * headTags;
};

void tag_requestBegin(const char * path, int mode);
void tag_requestEnd();
