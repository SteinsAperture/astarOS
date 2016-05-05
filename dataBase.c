#include "dataBase.h"
#define LOGFILE "tagfs.log"

FILE *mylog = NULL;
#define LOG(args...) do { fprintf(mylog, args); fflush(mylog); } while (0)


struct hashElt* fileTable = NULL;
/*
   hashTab (file) 
        file -> name : nom du fichier
	     -> nextLvl : hashTab (tag) = Liste de tags associés à un fichier.
	     tag -> name : nom du tag
	         -> nextLvl : NULL.
*/
struct hashElt* tagTable = NULL;
/* 
   hashTab (tag) 
        tag -> name : nom du tag
	    -> nextLvl : hashTab (file) = Liste de fichiers associés à un tag.
	    file -> name : nom du fichier
	         -> nextLvl : pointeur vers le vrai fichier dans fileTable.
   Le pointeur évite de dupliquer les listes de tags d'un fichier.
*/

char* strdup(const char * s);


struct hashElt * db_creatHashElt(char * name) {
  struct hashElt * he = malloc(sizeof(struct hashElt));
  he->name = strdup(name);
  he->nextLvl = NULL;
  return he;
}

struct hashElt * db_findHashElt(struct hashElt* hashTable, char * name) {
  struct hashElt* elt = NULL;
  HASH_FIND_STR(hashTable,name,elt);
  return elt;
}

struct hashElt * db_addHashElt(struct hashElt** hashTable, char * name) {
  return db_addFullHashElt(hashTable, name, NULL);
}

struct hashElt * db_addFullHashElt(struct hashElt** hashTable, char * name, struct hashElt* next) {
  struct hashElt* elt = db_creatHashElt(name);
  elt->nextLvl = next;
  HASH_ADD_KEYPTR( hh, (*hashTable), elt->name, strlen(elt->name), elt);
  return elt;
}



struct fileNode * db_creatFileNode(struct hashElt* elt) {
  struct fileNode * n = malloc(sizeof(struct fileNode));
  n->file = elt;
  n->next = n->prev = NULL;
  return n;
}


struct fileNode * db_addFileNode(struct fileNode** list, struct hashElt* elt) {
  struct fileNode * n = db_creatFileNode(elt);
  DL_APPEND((*list), n);
  return n;
}


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


struct hashElt * db_getFileTable() {
  return fileTable;
}

void db_addTag(char* fileName, char* tagName)
{
  mylog = mylog ? mylog : fopen(LOGFILE, "a");
  LOG("FIND TAGNAME : %s\t FILENAME : %s \n",tagName, fileName );

  struct hashElt *th = db_findHashElt(tagTable, tagName); // Récupérer le tag s'il existe.
  
  if( th == NULL ) // Le tag n'existe pas, le créer.
    th = db_addHashElt(&tagTable, tagName);

  struct hashElt *fh = db_findHashElt(fileTable, fileName); // Récupérer le fichier.

  // Ajouter le fichier dans la hashTable de fichiers du tag.
  struct hashElt* tfh = db_addFullHashElt(&(th->nextLvl), fileName, fh);
  
  // Ajouter le tag dans la hashTable de tags du fichier.
  struct hashElt* fth = db_addHashElt(&(fh->nextLvl),tagName);
}

void db_addFile(char* fileName)
{
  if(db_findHashElt(fileTable, fileName) == NULL) // Créer le fichier s'il n'existe pas.
    db_addHashElt(&fileTable,fileName);
}

int db_tagExist(char* tagName)
{
  return db_findHashElt(tagTable, tagName) != NULL;
}

struct fileNode* db_getFileList(char* tagName)
{  
  struct fileNode *head = NULL; // Liste de fichier à retourner.

  if (strlen(tagName) == 0) { // tag vide = Tous les fichiers
    for(struct hashElt * fh = fileTable; fh != NULL ; fh = fh->hh.next)
      db_addFileNode(&head, fh);
  } else { // Les fichiers d'un tag
    struct hashElt* th = db_findHashElt(tagTable, tagName);
    if (th != NULL)
      for(struct hashElt * it = th->nextLvl ; it != NULL ; it = it->hh.next)
	db_addFileNode(&head, it->nextLvl);
  }

  return head;
}

int db_linkExist(char* fileName,char* tagName){

  /* //LOG("DATABASE getTagHash : %s \n", fileName);
  struct hashElt* fh = NULL;
  HASH_FIND_STR(fileTable,fileName,fh);

  if(fh != NULL) {
    struct hashElt* th = NULL;
    HASH_FIND_STR(fh->headTags,tagName,th);
    return th != NULL;
    }*/
  return 0;
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
