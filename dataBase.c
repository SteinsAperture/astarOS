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

void db_deleteHashElt(struct hashElt* elt) {
  free(elt->name);
  free(elt);
}

void db_deleteDoubleHashTable(struct hashElt** hashTable) {
  struct hashElt *it1, *tmp1, *it2, *tmp2;

  HASH_ITER(hh,(*hashTable),it1,tmp1){
    HASH_ITER(hh,it1->nextLvl,it2,tmp2){
      db_removeHashEltPtr(&(it1->nextLvl), it2);
    }
    db_removeHashEltPtr(hashTable, it1);
  }
}

void db_removeHashElt(struct hashElt** hashTable, char * name) {
  db_removeHashEltPtr(hashTable,db_findHashElt(*hashTable, name));
}

void db_removeHashEltPtr(struct hashElt** hashTable, struct hashElt * elt) {
  if (elt != NULL) {
    HASH_DEL(*hashTable,elt);
    db_deleteHashElt(elt);
  }
}


struct eltNode * db_creatEltNode(struct hashElt* elt) {
  struct eltNode * n = malloc(sizeof(struct eltNode));
  n->elt = elt;
  n->next = n->prev = NULL;
  return n;
}


struct eltNode * db_addEltNode(struct eltNode** list, struct hashElt* elt) {
  struct eltNode * n = db_creatEltNode(elt);
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

struct eltNode* db_getFileList(char* tagName)
{  
  struct eltNode *head = NULL; // Liste de fichier à retourner.

  if (strlen(tagName) == 0) { // tag vide = Tous les fichiers
    for(struct hashElt * fh = fileTable; fh != NULL ; fh = fh->hh.next)
      db_addEltNode(&head, fh);
  } else { // Les fichiers d'un tag
    struct hashElt* th = db_findHashElt(tagTable, tagName);
    if (th != NULL)
      for(struct hashElt * it = th->nextLvl ; it != NULL ; it = it->hh.next)
	db_addEltNode(&head, it->nextLvl);
  }

  return head;
}

int db_linkExist(char* fileName,char* tagName){

  //LOG("DATABASE getTagHash : %s \n", fileName);
  struct hashElt* fh = db_findHashElt(fileTable, fileName);

  if(fh != NULL)
    return db_findHashElt(fh->nextLvl, tagName) != NULL;
  return 0;
}

void db_deleteFileList(struct eltNode * fileList) {
  if (fileList != NULL) {
    struct eltNode *elt, *it;

    DL_FOREACH_SAFE(fileList,elt,it) {
      DL_DELETE(fileList,elt);
      free(elt);
    }
  }
}

void db_removeTag(char* fileName, char* tagName)
{
  struct hashElt *fh;

  // Modifier la hashTable de tags du fichier. 
  fh = db_findHashElt(fileTable, fileName); // Récupérer le fichier.
  if (fh != NULL)
    db_removeHashElt(&(fh->nextLvl), tagName); // Retirer le tag du fichier.

  struct hashElt *th;
  // Modifier la hashTable de fichiers du tag.
  th = db_findHashElt(tagTable, tagName); // Récupérer le tag.
  if (th != NULL) {
    db_removeHashElt(&(th->nextLvl), fileName); // Retirer le fichier du tag.
    
    if(HASH_COUNT(th->nextLvl) == 0) // Retirer le tag de la BDD si vide.
      db_removeHashEltPtr(&tagTable, th);
  }
}

void db_removeFile(char* fileName)
{
  struct hashElt *fh, *th, *tmp;

  fh = db_findHashElt(fileTable, fileName); // Récupérer le fichier.
  if (fh != NULL) {
    HASH_ITER(hh,fh->nextLvl,th,tmp){ // Retirer tous les tags contenus.
      db_removeTag(fileName, th->name);
    }
    db_removeHashEltPtr(&fileTable, fh);
  }
}

void db_init() {
   mylog = mylog ? mylog : fopen(LOGFILE, "a");
}

void db_destroy() {
  db_deleteDoubleHashTable(&tagTable);
  db_deleteDoubleHashTable(&fileTable);
}
