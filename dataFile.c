#include "dataFile.h"
#define MAX 128

#define LOGFILE "tagfs.log"
FILE *mylog;
#define LOG(args...) do { fprintf(mylog, args); fflush(mylog); } while (0)

int df_load(char *path){

  mylog = fopen(LOGFILE, "a"); /* append logs to previous executions */

  LOG("dirpath : %s \n", path);
char file[MAX];
strcpy(file, path);
strcat(file, "/.tags");
LOG("fileazr : %s \n", file);
FILE* dataFile;
char line[MAX], f[MAX], t[MAX];
f[0]=0;
dataFile =  fopen(file, "r");
if (dataFile == NULL) {
  perror("Fichier introuvable");
  exit(EXIT_FAILURE);
}else{
  while (fgets(line, MAX, dataFile) != NULL){
      if (*line == '[') { // file
        sscanf(line, "[%[a-zA-Z.]]", f);
        LOG("file : %s\n", f);
      }else if (*line != '#' && *line != '\n') { // Un tag
        sscanf(line, "%s", t);
        LOG("tag : %s\n", t);
        if(f[0]!=0){
          db_addTag(f, t);
        }else{

        }
      }
      // Reste on ignore
    }
  }
  return 0;
}

int df_save(char *path){

  LOG("dirpath : %s \n", path);
  char file[MAX];
  strcpy(file, path);
  strcat(file, "/.tags");
  LOG("fileazr : %s \n", file);
  FILE* dataFile;
  char line[MAX], f[MAX], t[MAX];
  f[0]=0;
  dataFile =  fopen(file, "w");
  if (dataFile == NULL) {
    perror("Fichier introuvable");
    exit(EXIT_FAILURE);
  }else{
    struct hashElt *fh, *th;
    int write = 0;
    for(fh = db_getFileTable(); fh != NULL ; fh = fh->hh.next) {
      if(HASH_COUNT(fh->nextLvl)>0){
	if (write == 1)
	  fprintf(dataFile, "\n");
	write = 1;
        fprintf(dataFile, "[%s]\n",fh->name);
        for(th = fh->nextLvl; th != NULL ; th = th->hh.next) {
          fprintf(dataFile, "%s\n", th->name);
        }
        LOG("file %s saved\n",fh->name);
      }
    }
  }
  return 0;
}
