#include "dataBase.h"
#define LOGFILE "tagfs.log"

FILE *mylog = NULL;
#define LOG(args...) do { fprintf(mylog, args); fflush(mylog); } while (0)

struct hashElt* tagTable = NULL;
struct hashElt* fileTable = NULL;

char* strdup(const char * s);

void db_debugFiles() {
  struct hashElt* it, *it2;
  LOG("DEBUG FILE TABLE \n");
  for (it = fileTable; it != NULL; it = it->hh.next) {
    LOG("%s :", it->name);
    if (it->nextLvl != NULL) {
      for (it2 = it->nextLvl; it2 != NULL; it2 = it2->hh.next) {
	LOG(" %s", it2->name);
      }
    }
    LOG("\n");
  }
  LOG("\n");
}

void db_debugTags() {
  struct hashElt* it, *it2, *it3;
  LOG("DEBUG TAG TABLE \n");
  for (it = tagTable; it != NULL; it = it->hh.next) {
    LOG("tag %s \n", it->name);
    if (it->nextLvl != NULL) {
      for (it2 = it->nextLvl; it2 != NULL; it2 = it2->hh.next) {
	LOG("\t file %s :", it2->name);
	if (it2->nextLvl != NULL) { // filehash
	  if (it2->nextLvl->nextLvl != NULL) { // Tags d'un fichier
	    for (it3 = it2->nextLvl->nextLvl; it3 != NULL; it3 = it3->hh.next) {
	      LOG(" %s", it3->name);
	    }
	  }
	}
	
	LOG("\n");
      }
    }
    LOG("\n");
  }
  LOG("\n");
}

void db_addTag(char* fileName, char* tagName)
{
  db_debugFiles();
  db_debugTags();

  mylog = mylog ? mylog : fopen(LOGFILE, "a");
  LOG("FIND TAGNAME : %s\t FILENAME : %s \n",tagName, fileName );

  // Recherche du tag
  struct hashElt* th = NULL;
  HASH_FIND_STR(tagTable, tagName, th);

  // Tentative d'ajout du fichier
  db_addFile(fileName);

  // Recherche du fichier
  struct hashElt* fh = NULL;
  HASH_FIND_STR(fileTable, fileName, fh);
  
  //Le tag n'existe pas ?
  if( th == NULL ) {
    LOG("CREATE TAG %s \n", tagName);
    th = malloc(sizeof(struct hashElt));
    th->name = strdup(tagName); // Mémoire non free
    th->nextLvl = NULL;

    HASH_ADD_KEYPTR( hh, tagTable, th->name, strlen(th->name), th );
  }

  // Le tag existe, stocké dans th. Le fichier est stocké dans fh

  // Ajouter le fichier dans la hashTable de fichiers du tag.
  struct hashElt* tfh = malloc(sizeof(struct hashElt));
  tfh->name = strdup(fileName);
  tfh->nextLvl = fh; // Attention à ne pas free // Le fichier avec la hashtable de tag.
  HASH_ADD_KEYPTR(hh, th->nextLvl , tfh->name, strlen(tfh->name), tfh);
  
  // Ajouter le tag dans la hashTable de tags du fichier.
  struct hashElt* fth = malloc(sizeof(struct hashElt));
  fth->name = strdup(tagName);
  fth->nextLvl = NULL;
  HASH_ADD_KEYPTR( hh, fh->nextLvl, fth->name, strlen(fth->name), fth);
}

void db_addFile(char* fileName)
{
  // Recherche du fichier
  struct hashElt* fh = NULL;
  HASH_FIND_STR(fileTable, fileName, fh);

  //Le fichier n'existe pas ? Alors l'ajouter.
  if( fh == NULL ) {
    LOG("CREATE FILE %s \n", fileName);
    fh = malloc(sizeof(struct hashElt));
    fh->name = strdup(fileName);
    fh->nextLvl = NULL;

    HASH_ADD_KEYPTR( hh, fileTable, fh->name, strlen(fh->name), fh );
  }
}

int db_tagExist(char* tagName)
{
  struct hashElt* th = NULL;
  HASH_FIND_STR(tagTable, tagName, th);
  return th != NULL;
}

struct fileNode* db_getFileList(char* tagName)
{

  LOG("DATABASE getFileList : %s \n", tagName);
  struct hashElt* th = NULL; // Tag à trouver
  HASH_FIND_STR(tagTable, tagName, th);

  struct fileNode *head = NULL; // Liste de fichier.

  if (th == NULL) { // Pas de tag trouvé
    if (strlen(tagName) == 0) { // tag vide = Tous les fichiers
      LOG("ALL FILE \n");
      struct hashElt * fh;
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
    struct hashElt * it;
    for(it = th->nextLvl ; it != NULL ; it = it->hh.next) {
      struct fileNode * fn = malloc(sizeof(struct fileNode));
      fn->file = it->nextLvl;
      fn->next = fn->prev = NULL;
      DL_APPEND(head, fn);
    }
  }

  return head;
}

int db_linkExist(char* fileName,char* tagName){

  /*//LOG("DATABASE getTagHash : %s \n", fileName);
  struct fileHash* fh = NULL;
  HASH_FIND_STR(fileTable,fileName,fh);

  if(fh == NULL){
    return 0;
  }else{
    struct tagHash* th = NULL;
    HASH_FIND_STR(fh->headTags,tagName,th);
    return th != NULL;
    }*/
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

struct hashElt* db_getAllFiles()
{
  return fileTable;
}

void db_deleteTag(char* fileName, char* tagName)
{
  /* struct fileHash* fH;
  struct tagHash* tH;

  HASH_FIND_STR(fileTable, fileName, fH);
  if(fH == NULL){
    LOG("Delete Tag : file not found\n");
  }
  else{
    HASH_FIND_STR(fH->headTags, tagName, tH);
    if(tH == NULL){
      LOG("Delete Tag : tag not found in file\n");
    }
    else{
      HASH_DEL(fH->headTags,tH);
      free(tH->name);
      free(tH);
    }
  }

  HASH_FIND_STR(tagTable, tagName, tH);
  if(tH == NULL){
    LOG("Delete Tag : tag not found\n");
  }
  else{
    HASH_FIND_STR(tH->headFiles, fileName, fH);
    if(fH == NULL){
      LOG("Delete Tag : file not found in tag\n");
    }
    else{
      HASH_DEL(tH->headFiles,fH);
      free(fH->name);
      free(fH);

      if(HASH_COUNT(tH->headFiles) == 0){
        HASH_DEL(tagTable, tH);
        free(tH->name);
        free(tH);
      }
    }
    }*/
}

void db_deleteFile(char* fileName)
{
  /*struct fileHash* fH;
  struct tagHash *tH,*tmp;


  HASH_FIND_STR(fileTable, fileName, fH);
  if(fH == NULL){
    LOG("Delete File : file not found\n");
  }
  else{
    HASH_ITER(hh,fH->headTags,tH,tmp){
      HASH_DEL(fH->headTags,tH);
      free(tH->name);
      free(tH);
    }
    HASH_DEL(fileTable, fH);
    free(fH->name);
    free(fH);
  }

  HASH_ITER(hh, tagTable, tH, tmp){
    HASH_FIND_STR(tH->headFiles, fileName, fH);
    if(fH != NULL){
      HASH_DEL(tH->headFiles, fH);
      free(fH->name);
      free(fH);

      if(HASH_COUNT(tH->headFiles) == 0){
        HASH_DEL(tagTable, tH);
        free(tH->name);
        free(tH);
      }
    }
    }*/
}

void db_deleteTagTable()
{
  /*struct tagHash *current_tag,*tmp;
  struct fileHash *current_file,*tmp2;

  HASH_ITER(hh,tagTable,current_tag,tmp){
    HASH_ITER(hh,current_tag->headFiles,current_file,tmp2){
      HASH_DEL(current_tag->headFiles,current_file);
      free(current_file->name);
      free(current_file);
    }
    HASH_DEL(tagTable,current_tag);
    free(current_tag->name);
    free(current_tag);
    }*/
}

void db_deleteFileTable(){
  /* struct fileHash *current_file,*tmp;
  struct tagHash *current_tag,*tmp2;

  HASH_ITER(hh,fileTable,current_file,tmp){
    

    HASH_ITER(hh,current_file->headTags,current_tag,tmp2){
      HASH_DEL(current_file->headTags,current_tag);
      free(current_tag->name);
      free(current_tag);
    }
    HASH_DEL(fileTable,current_file);
    free(current_file->name);
    free(current_file);
    }*/
}
