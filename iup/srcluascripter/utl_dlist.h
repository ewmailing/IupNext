/*
 * daVinci - Double Linked List Functions (unfinished).
 *
 * Copyright (C) 1999-2008, Antonio E. Scuri, Mauro Charão, Tecgraf/PUC-Rio.
 */
#ifndef __UTL_DLIST_H__
#define __UTL_DLIST_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _utlDListNode
{
  struct _utlDListNode *next;
  struct _utlDListNode *previous;
  void *value;
} utlDListNode;

typedef struct
{
  int total;
  utlDListNode *top;
  utlDListNode *bottom;
} utlDList;

utlDList* utlCreateDList(void);
void utlKillDList(utlDList *list, void (*free_value)(void *value));

void utlDListInsertTop(utlDList *list, void *value);
void utlDListInsertBottom(utlDList *list, void *value);
int utlDListRemoveNode(utlDList *list, utlDListNode *node);
utlDListNode* utlDListRemove(utlDList *list, void *value);
utlDListNode* utlDListFindValue(utlDList *list, void *value);
utlDListNode* utlDListFindStr(utlDList *list, char *value);

#ifdef __cplusplus
}
#endif

#endif
