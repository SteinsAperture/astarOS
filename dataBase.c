#include "dataBase.h"
#define LOGFILE "tagfs.log"
FILE *mylog = NULL;
#define LOG(args...) do { fprintf(mylog, args); fflush(mylog); } while (0)

struct tag* hashTable = NULL;


void db_addTag(char* fileName, char* tagName)
{
  mylog = mylog ? mylog : fopen(LOGFILE, "a"); /* append logs to previous executions */
  LOG("FIND TAGNAME : %s\t FILENAME : %s",tagName, fileName );
  struct tag* t = NULL;
  HASH_FIND_STR(hashTable, tagName, t);
  

  struct file* f = malloc(sizeof(struct file));

  f->name = strdup(fileName);

  f->next = f->prev = NULL;

  //Le tag existe déjà ?
  if( t == NULL )
    {
      //il existe pas : on le met
      LOG("CREATE %s", tagName);
      t = malloc(sizeof(struct tag));
      strcpy(t->name, tagName);
      t->headFiles = NULL;

      HASH_ADD_STR( hashTable, name, t);
    } 
  
  LOG("APPEND %s", fileName);
  DL_APPEND(t->headFiles, f);
}

struct file* db_getFiles(char* tagName)
{
  struct tag* f = NULL;
  HASH_FIND_STR(hashTable, tagName, f);
  return f ? f->headFiles : NULL;
}


void test(void)
{
  struct tag* s;

  LOG("HERE\n");

  for(s=hashTable; s != NULL ; s=s->hh.next) 
    {
      LOG("tag id %s:", s->name);
      struct file *f;
      for(f = s->headFiles ; f != NULL ; f = f->next)
	{
	  LOG("%s\t", f->name);
	} 
      LOG("\n");
    }
}

