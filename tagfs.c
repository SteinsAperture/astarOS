/*
 * Copyright (c) 2014-2015 Inria. All rights reserved.
 */

#define _GNU_SOURCE
#define FUSE_USE_VERSION 26

#include "tagfs.h"
#include <pthread.h>

static DIR *dir;
static char *dirpath;
static char fakeFile[255] = {'\0'};
pthread_rwlock_t rwlock;


/*******************
 * Logs
 */

#define LOGFILE "tagfs.log"
FILE *mylog = NULL;
#ifdef WRITELOG
#define LOG(args...) do { if (mylog == NULL) mylog = fopen(LOGFILE, "a"); \
    fprintf(mylog, args); fflush(mylog);			  \
  } while (0)
#else
#define LOG(args...) do { } while (0)
#endif
/*************************
 * TODO
 */

struct request * tag_requestBegin(const char * path, int mode) // 0->dir, 1->file
{
  struct request * req = malloc(sizeof(struct request));
  req->file = NULL;
  req->realpath = NULL;
  req->headTags = NULL;
  char * p = strdup(path);

  if (mode == 1) {
    // Realpath
    char *beginFile = strrchr(p,'/');
    if (beginFile != NULL) {
      *beginFile = '\0'; // Permet de stoper strtok avant le fichier.
      ++beginFile; // Décalage pour lecture du fichier.
      asprintf(&req->realpath, "%s/%s", dirpath, beginFile ? beginFile : path);
    } else {
      beginFile = p;
    }
    req->file = strdup(beginFile);
    //LOG("REQUEST realpath : %s \n", req->realpath);
    //LOG("REQUEST file : %s \n", req->file);
  }
  
  char * token = strtok(p,"/");
  while (token != NULL) {
    //LOG("REQUEST tagDIR : %s \n",token);

    struct tagNode * tn = malloc(sizeof(struct tagNode));
    tn->name = strdup(token);
    tn->next = tn->prev = NULL;

    DL_APPEND(req->headTags, tn);
    
    token = strtok(NULL,"/");
  }

  free(p);

  //LOG("REQUEST BUILD \n");
  return req;
}

void tag_requestEnd(struct request * req)
{
  if (req != NULL) {
    struct tagNode *elt, *it;
    
    DL_FOREACH_SAFE(req->headTags,elt,it) {
      DL_DELETE(req->headTags,elt);
      free(elt->name);
      free(elt);
    }
    if (req->file != NULL)
      free(req->file);
    if (req->realpath != NULL)
      free(req->realpath);
    req = NULL;
  }
}

/*************************
 * File operations
 */

/* get attributes */
static int tag_getattr(const char *path, struct stat *stbuf)
{
  struct request * req = tag_requestBegin(path,1);
  char * file = req->file;
  
  int res;
  if (file != NULL && file[0] == '.') {
    res = stat(fakeFile, stbuf);
    file++; // enlever le .
  }
  else
    res = stat(req->realpath, stbuf);
  
  char * endPath = strrchr(req->realpath,'/');
  ++endPath; // Fichier ou tag

  
  
  if (res != -1) {
    struct tagNode *elt; 
    pthread_rwlock_rdlock(&rwlock);
    DL_FOREACH(req->headTags,elt) {
      if (!db_linkExist(file, elt->name)) {
	res = -ENOENT;
	break;
      }
    }
    pthread_rwlock_unlock(&rwlock);
  }
  
  if ((res == -ENOENT || res == -1) && endPath[0] != '\0' && strchr(endPath,'.') == NULL) { // Tag
    // C'est un répertoire virtuel, stat le répertoire racine.
    res = stat(dirpath, stbuf);
  }

  if (res == -1)
    res = -ENOENT;
  
  LOG("getattr %s returning %s\n", req->realpath ,strerror(-res));
  tag_requestEnd(req);
  
  return res;
}


static int tag_opendir(const char* path, struct fuse_file_info* fi)
{
  int res = 0;
  LOG("OPENDIR %s\n", path);
  struct request * req = tag_requestBegin(path,0);
  
  pthread_rwlock_rdlock(&rwlock);
    
  struct eltNode * files = NULL;
  files = db_getFileList(req->headTags != NULL ? req->headTags->name : "");

  if (files != NULL) {
    // Que les fichiers qui matchent tous les tags
    if (req->headTags != NULL) {
      // Pour chaque tags qui suivent.
      for (struct tagNode *tn = req->headTags->next; tn != NULL; tn = tn->next){
	struct eltNode *elt, *it;
	struct hashElt *th;
      
	DL_FOREACH_SAFE(files,elt,it) {
	  th = NULL;
	  // Recherche du tag
	  HASH_FIND_STR(elt->elt->nextLvl, tn->name, th);
	  if (th == NULL) {
	    DL_DELETE(files,elt);
	    free(elt);
	  }
	}
      }
    }

    struct eltNode *elt, *it;
      
    DL_FOREACH_SAFE(files,elt,it) {
      LOG("file : %s \t",elt->elt->name);
    }
    
    /*struct eltNode *eltForCount;
    int count = 0;
    DL_COUNT(files,eltForCount,count);*/

    pthread_rwlock_unlock(&rwlock);
    
    //if (count == 0)
    if (files == NULL)
      res = -ENOENT;

    LOG("OPENDIR nettoyage\n");
    
    db_deleteFileList(files);
    
    LOG("readdir returning %s\n", strerror(-res));
  } else {
    res = -ENOENT;
    LOG("readdir returning %s\n", strerror(-res));
  }
  
  tag_requestEnd(req);
  LOG("OPENDIR END \n");
  return res;
}


/* list files within directory */
static int tag_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  int res = 0;

  struct request * req = tag_requestBegin(path,0);
  
  LOG("readdir '%s'\n", path);
  filler (buf, ".", NULL, 0);
  filler (buf, "..", NULL, 0);

  pthread_rwlock_rdlock(&rwlock);
  
  struct eltNode * files = NULL;
  files = db_getFileList(req->headTags != NULL ? req->headTags->name : "");

  if (files != NULL) {
    // Que les fichiers qui matchent tous les tags
    if (req->headTags != NULL) {
      // Pour chaque tags qui suivent.
      for (struct tagNode *tn = req->headTags->next; tn != NULL; tn = tn->next){
	struct eltNode *elt, *it;
	struct hashElt *th;
      
	DL_FOREACH_SAFE(files,elt,it) {
	  th = NULL;
	  // Recherche du tag
	  HASH_FIND_STR(elt->elt->nextLvl, tn->name, th);
	  if (th == NULL) {
	    DL_DELETE(files,elt);
	    free(elt);
	  }
	}
      }
    }

    struct tagHash *availableTags = NULL, *it = NULL, *at = NULL;
    
    for(struct eltNode * file = files; file != NULL ; file = file->next) {
      struct hashElt *th;
      for(th = file->elt->nextLvl ; th != NULL ; th = th->hh.next){
	at = NULL;
	HASH_FIND_STR(availableTags, th->name, at);
	if (at == NULL) {
	  LOG("READDIR new available tag %s \n",th->name);
	  at = malloc(sizeof(struct tagHash));
	  at->name = strdup(th->name);
	  HASH_ADD_KEYPTR( hh, availableTags, at->name, strlen(at->name), at );
	}
      } 
      //res = tag_getattr(file->elt->name, &stbuf);
      filler(buf, file->elt->name, NULL, 0);

      char secretTagFile[255] = {'\0'};
      strcat(secretTagFile, ".");
      strcat(secretTagFile, file->elt->name);
      filler(buf, secretTagFile, NULL, 0);
    }

    LOG("READDIR fichiers OK");

    // Retirer les tags filtrés des tags disponible.
    for (struct tagNode *tn = req->headTags; tn != NULL; tn = tn->next){
      struct tagHash *th;
      
      th = NULL;
      // Recherche du tag
      LOG("READDIR filtre tag : %s \n", tn->name);
      HASH_FIND_STR(availableTags, tn->name, th);
      if (th != NULL) {
	LOG("READDIR retire tag valide \n");
	HASH_DEL(availableTags, th);
	free(th->name);
	free(th);
      }
    }

    LOG("READDIR Ajout tag au res \n");
    // Ajouter au résultat les tags valides.
    HASH_ITER(hh, availableTags, at, it) {
      HASH_DEL(availableTags, at);

      LOG("READDIR tagres : %s \n", at->name);
      filler(buf, at->name, NULL, 0);
    
      free(at->name);
      free(at);
    }

    LOG("READDIR nettoyage\n");
    
    db_deleteFileList(files);
    
    LOG("readdir returning %s\n", strerror(-res));
  } else {
    res = -ENOENT;
    LOG("readdir returning %s\n", strerror(-res));
  }
  pthread_rwlock_unlock(&rwlock);
  
  tag_requestEnd(req);
  return res;
}

void tag_readTags (const char *file) {
  file++; // Enlever le .
  LOG("READTAF %s \n",file);


  FILE* tmpTagFile;
  tmpTagFile = fopen(fakeFile, "w");
  if (tmpTagFile == NULL) {
    perror("Fichier introuvable");
  }else{
    pthread_rwlock_rdlock(&rwlock);
    
    struct hashElt * hashTags = db_getTags(file);
    if (hashTags != NULL) {
      for(struct hashElt * it = hashTags; it != NULL; it = it->hh.next) {
	fprintf(tmpTagFile, "%s ",it->name);
      }
    }
    fprintf(tmpTagFile, "\n");
    pthread_rwlock_unlock(&rwlock);
  }
  fclose(tmpTagFile);
}


/* read the grepped contents of the file */
int tag_read (const char *path, char *buffer, size_t len, off_t off, struct fuse_file_info *fi)
{
  struct request * req = tag_requestBegin(path,1);

  int res = 0;
  
  char *realpath = req->realpath;
  if (req->file[0] == '.') {
    tag_readTags(req->file);
    realpath = fakeFile;
  }
  char *command;
  FILE *fd;
  char *cmdoutput;
   
  LOG("read '%s' for %ld bytes starting at offset %ld\n", path, len, off);

  asprintf(&command, "cat %s", realpath);
  
  char * p = strdup(path);
  char * token = strtok(p,"/");
  while (token != NULL) {
    LOG("token : %s \n",token);
    token = strtok(NULL,"/");
  }
  
  LOG("read using command %s\n", command);
  
  /* open the output of the command */
  fd = popen(command, "r");
  if (!fd) {
    res = -errno;
  } else {
  
    /* read up to len+off bytes from the command output */
    cmdoutput = malloc(len+off);
    if (!cmdoutput) {
      res = -ENOMEM;
	
      pclose(fd);
    
    } else {
      res = fread(cmdoutput, 1, len+off, fd);
      LOG("read got %d bytes out of %ld requested\n", res, len+off);
      if (res > off) {
	/* we read more than off, ignore the off first bytes and copy the remaining ones */
	memcpy(buffer, cmdoutput+off, res-off);
	res -= off;
      } else {
	/* we failed to read enough */
	res = 0;
      }

      free(cmdoutput);
    }
  }
  free(command); 

  LOG("content buf: %s", buffer);
  if (res < 0)
    LOG("read returning %s\n", strerror(-res));
  else
    LOG("read returning success (read %d)\n", res);
  tag_requestEnd(req);
  return res;
}

static int tag_link(const char* from, const char* to){
  int res = 0;
  
  LOG("LINK %s to %s \n", from, to);
  struct request * req1 = tag_requestBegin(to,1),
    *req2 = tag_requestBegin(from,1);

  if (strcmp(req1->file,req2->file) == 0) {
    struct tagNode *elt; 

    pthread_rwlock_wrlock(&rwlock);   
    
    DL_FOREACH(req1->headTags,elt) {
      if (!db_linkExist(req1->file, elt->name)) {
	db_addTag(req1->file, elt->name);
      }
    }

    pthread_rwlock_unlock(&rwlock);
  } else {
    res = -ENOENT;
  }
  tag_requestEnd(req1);
  tag_requestEnd(req2);
  LOG("LINK RETURN : %s", strerror(-res));
  return res;
}

static int tag_unlink(const char* path){
  LOG("UNLINK %s \n", path);

  struct request * req = tag_requestBegin(path,1);

  struct tagNode *elt;
  int removeFile = 1;

  pthread_rwlock_wrlock(&rwlock);
    
  DL_FOREACH(req->headTags,elt) {
    removeFile = 0;
    db_removeTag(req->file,elt->name);
    LOG("TAG REMOVE %s FROM %s \n",elt->name,req->file);
  }

  if (removeFile) {
    db_removeFile(req->file);
    unlink(req->realpath);
    LOG("FILE REMOVE %s \n",req->file);
  }

  pthread_rwlock_unlock(&rwlock);
  
  tag_requestEnd(req);
  return 0;
}

static int tag_rename(const char* from, const char* to){
  int res = 0;
  
  LOG("RENAME %s to %s \n", from, to);
  struct request * req1 = tag_requestBegin(to,1),
    *req2 = tag_requestBegin(from,1);

  if (strcmp(req1->file,req2->file) == 0 && req2->headTags != NULL) {
    res = tag_unlink(from);
    if (res != -ENOENT)
      res = tag_link(req2->file,to);
  } else {
    res = -ENOENT;
  }
  tag_requestEnd(req1);
  tag_requestEnd(req2);
  return res;
}

static struct fuse_operations tag_oper = {
  .getattr = tag_getattr,
  .readdir = tag_readdir,
  .read = tag_read,
  .link = tag_link,
  .opendir = tag_opendir,
  .unlink = tag_unlink,
  .rename = tag_rename,
};

/**************************
 * Main
 */

int main(int argc, char *argv[])
{
  int err;

  if (argc < 2) {
    fprintf(stderr, "missing destination directory\n");
    exit(EXIT_FAILURE);
  }
  /* find the absolute directory because fuse_main()
   * doesn't launch the daemon in the same current directory.
   */
  dirpath = realpath(argv[1], NULL);
  dir = opendir(dirpath);
  if (!dir) {
    fprintf(stderr, "couldn't open directory %s\n", dirpath);
    exit(EXIT_FAILURE);
  }

  struct dirent *dirent;
  rewinddir(dir);

  LOG("\n");

  LOG("starting tagfs in %s\n", dirpath);

  strcat(fakeFile,dirpath);
  strcat(fakeFile,"/.fakefile");
  mode_t mode; 
  dev_t dev;
  
  mode = S_IFREG | 0660;
  dev = 0;
  mknod(fakeFile, mode, dev);
  
  while ((dirent = readdir(dir)) != NULL) {
    if (dirent->d_name[0] != '.') {
      LOG("INDIR : %s \n",dirent->d_name);
      db_addFile(dirent->d_name);
    }
  }
   
  df_load(dirpath);
  argv++;
  argc--;


  if (pthread_rwlock_init(&rwlock, NULL) < 0) {
    perror("pthread_rwlock_init");
    return -1;
  }
  
  err = fuse_main(argc, argv, &tag_oper, NULL);
  LOG("stopped tagfs with return code %d\n", err);

  pthread_rwlock_destroy(&rwlock);

  
  df_save(dirpath);
  db_destroy();
  closedir(dir);
  free(dirpath);

  LOG("\n");
  return err;
}
