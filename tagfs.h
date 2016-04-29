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

struct request * tag_requestBegin(const char * path, int mode);
void tag_requestEnd(struct request * req);
static int tag_getattr(const char *path, struct stat *stbuf);
static int tag_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int tag_read (const char *path, char *buffer, size_t len, off_t off, struct fuse_file_info *fi);
static int tag_link(const char* from, const char* to);
static int tag_readlink(const char* path, char* buf, size_t size);
static int tag_symlink(const char* to, const char* from);
static int tag_opendir(const char* path, struct fuse_file_info* fi);
