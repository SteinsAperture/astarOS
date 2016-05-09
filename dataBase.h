#ifndef __DATABASE_H_
#define __DATABASE_H_

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash.h"
#include "utlist.h"

#define TAGNAME_SIZE 128

extern FILE* mylog;

struct hashElt{
  char * name;
  struct hashElt * nextLvl;
  UT_hash_handle hh;         /* makes this structure hashable */
};

struct eltNode{
  struct hashElt* elt;
  struct eltNode *next, *prev;
};

/**
   METHODES PRIVEES
 */
struct hashElt * db_creatHashElt(const char * name);
struct hashElt * db_findHashElt(struct hashElt* hashTable, const char * name);
struct hashElt * db_addHashElt(struct hashElt** hashTable, const char * name);
struct hashElt * db_addFullHashElt(struct hashElt** hashTable, const char * name, struct hashElt* next);
void db_deleteHashElt(struct hashElt* elt);
void db_deleteDoubleHashTable(struct hashElt** hashTable);
void db_removeHashElt(struct hashElt** hashTable, const char * name);
void db_removeHashEltPtr(struct hashElt** hashTable, struct hashElt * elt);

struct eltNode * db_creatEltNode(struct hashElt* elt);
struct eltNode * db_addEltNode(struct eltNode** list, struct hashElt* elt);
/**
   METHODES PUBLIQUES
 */

struct hashElt * db_getFileTable();
//add a tag in the hashtable
void db_addTag(char* fileName, char* tagName);

//return files by tag
struct eltNode* db_getFileList(char* tagName);

int db_linkExist(char* fileName,char* tagName);

void db_deleteFileList(struct eltNode * fileList);

void db_addFile(char* fileName);
int db_tagExist(char* tagName);

struct hashElt * db_getTags(const char* fileName);
void db_removeTag(char* fileName, char* tagName);
void db_removeFile(char* fileName);

void db_destroy();


#endif
