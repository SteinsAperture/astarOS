#include "dataBase.h"
#define LOGFILE "tagfs.log"
FILE *mylog = NULL;
#define LOG(args...) do { fprintf(mylog, args); fflush(mylog); } while (0)

struct tagHash* tagTable = NULL;
struct fileHash* fileTable = NULL;

void db_addTag(char* fileName, char* tagName)
{
  mylog = mylog ? mylog : fopen(LOGFILE, "a");
  LOG("FIND TAGNAME : %s\t FILENAME : %s \n",tagName, fileName );

  // Recherche du tag
  struct tagHash* th = NULL;
  HASH_FIND_STR(tagTable, tagName, th);

  // Tentative d'ajout du fichier
  db_addFile(fileName);
  
  // Recherche du fichier
  struct fileHash* fh = NULL;
  HASH_FIND_STR(fileTable, fileName, fh);
  
  //Le tag n'existe pas ?
  if( th == NULL ) {
    LOG("CREATE TAG %s \n", tagName);
    th = malloc(sizeof(struct tagHash));
    th->name = strdup(tagName); // Mémoire non free
    th->headFiles = NULL;

    HASH_ADD_KEYPTR( hh, tagTable, th->name, strlen(th->name), th );
  } 
  
  // Le tag existe, stocké dans th.

  // Ajouter le fichier dans la liste de fichiers du tag.
  struct fileNode* fn = malloc(sizeof(struct fileNode));
  fn->file = fh;
  fn->next = fn->prev = NULL;
  DL_APPEND(th->headFiles, fn);

  // Ajouter le tag dans la hashTable de tags du fichier.
  struct tagHash* fth = malloc(sizeof(struct tagHash));
  fth->name = strdup(tagName); // Mémoire non free
  fth->headFiles = NULL;
  HASH_ADD_KEYPTR( hh, fh->headTags, fth->name, strlen(fth->name), fth);
}

void db_addFile(char* fileName)
{
  // Recherche du fichier
  struct fileHash* fh = NULL;
  HASH_FIND_STR(fileTable, fileName, fh);
  
  //Le fichier n'existe pas ? Alors l'ajouter.
  if( fh == NULL ) {
    LOG("CREATE FILE %s \n", fileName);
    fh = malloc(sizeof(struct fileHash));
    fh->name = strdup(fileName);
    fh->headTags = NULL;

    HASH_ADD_KEYPTR( hh, fileTable, fh->name, strlen(fh->name), fh );
  } 
}

int db_tagExist(char* tagName)
{
  struct tagHash* th = NULL;
  HASH_FIND_STR(tagTable, tagName, th);
  return th != NULL;
}

struct fileNode* db_getFileList(char* tagName)
{

  LOG("DATABASE getFileList : %s \n", tagName);
  struct tagHash* th = NULL; // Tag à trouver
  HASH_FIND_STR(tagTable, tagName, th);

  struct fileNode *head = NULL; // Liste de fichier.
  
  if ( th == NULL) { // Pas de tag trouvé
    if (strlen(tagName) == 0) { // tag vide = Tous les fichiers
      LOG("ALL FILE \n");
      struct fileHash * fh;
      for(fh = fileTable; fh != NULL ; fh = fh->hh.next) {
       struct fileNode * fn = malloc(sizeof(struct fileNode));
       fn->file = fh;
       fn->next = fn->prev = NULL;
       DL_APPEND(head, fn);
      }
    } else {
      LOG("AUCUN \n");
      return NULL;
    }
  } else { // Les fichiers d'un tag
  LOG("PAR TAG \n");
  struct fileNode * it;
  for(it = th->headFiles ; it != NULL ; it = it->next) {
    struct fileNode * fn = malloc(sizeof(struct fileNode));
    fn->file = it->file;
    fn->next = fn->prev = NULL;
    DL_APPEND(head, fn);
  } 
}

struct tagHash* db_linkExist(char* fileName,char* tagName){

  LOG("DATABASE getTagHash : %s \n", fileName);
  struct fileHash* fh = NULL;
  HASH_FIND_STR(fileTable,fileName,fh);

  if(fh == NULL){
    LOG("AUCUN \n");
    return NULL;
  }else{
    struct tagHash* head = NULL;
    HASH_FIND_STR(fh->headTags,tagName,head);
    if(head==NULL){
      LOG("N'EXISTE PAS \n");
      return 0;
    }else{
      LOG("EXISTE \n");
      return 1;
    }
  }
}

  return head; // Danger, devra être libéré.
}

void db_deleteFileList(struct fileNode * fileList) {
  if (fileList != NULL) {
    struct fileNode *elt, *it;

    DL_FOREACH_SAFE(fileList,elt,it) {
      DL_DELETE(fileList,elt);
      free(elt);
    }
  }
}

struct fileHash* db_getAllFiles()
{
  return fileTable;
}

void test(void)
{
  struct tagHash* s;

  LOG("HERE\n");

  for(s=tagTable; s != NULL ; s=s->hh.next) 
  {
    LOG("tag id %s:", s->name);
    struct fileNode *f;
    for(f = s->headFiles ; f != NULL ; f = f->next)
    {
     LOG("%s\t", f->file->name);
     } 
     LOG("\n");
   }
}


void db_deleteTagTable()
{
  struct tagHash *current_tag,*tmp;

  HASH_ITER(hh,tagTable,current_tag,tmp){
    db_deleteFileList(current_tag->headFiles);
    HASH_DEL(tagTable,current_tag);
    free(current_tag->name);
    free(current_tag);
  }
}

void db_deleteFileTable(){
  struct fileHash *current_file,*tmp;

  HASH_ITER(hh,fileTable,current_file,tmp){
    struct tagHash *current_tag,*tmp2;

    HASH_ITER(hh,current_file->headTags,current_tag,tmp2){
      HASH_DEL(current_file->headTags,current_tag);
      free(current_tag->name);
      free(current_tag);
    }
    HASH_DEL(fileTable,current_file);
    free(current_file->name);
    free(current_file);
  }
}