/*
 * daVinci - Double Linked List Functions (unfinished).
 *
 * Copyright (C) 1999-2008, Antonio E. Scuri, Mauro Charão, Tecgraf/PUC-Rio.
 */
#include "utl_dlist.h"
#include <stdlib.h>
#include <string.h>


utlDList* utlCreateDList(void)
{
  utlDList *list = (utlDList*)malloc(sizeof(utlDList));

  list->top = NULL;
  list->bottom = NULL;
  list->total = 0;
  return list;
}

void utlKillDList(utlDList *list, void (*free_value)(void *value))
{
  utlDListNode *node = list->top;

  while(node != NULL)
  {
    utlDListNode *next = node->next;
    free_value(node->value);
    free(node);
    node = next;
  }
  free(list);
}

void utlDListInsertTop(utlDList *list, void *value)
{
  utlDListNode *node = (utlDListNode*)malloc(sizeof(utlDListNode));

  node->value = value;
  node->previous = NULL;
  node->next = list->top;
  if(list->bottom == NULL)
    list->bottom = node;
  else list->top->previous = node;
  list->top = node;
  list->total++;
}

void utlDListInsertBottom(utlDList *list, void *value)
{
  utlDListNode *node = (utlDListNode*)malloc(sizeof(utlDListNode));

  node->value = value;
  node->next = NULL;
  node->previous = list->bottom;
  if(list->top == NULL)
    list->top = node;
  else list->bottom->next = node;
  list->bottom = node;
  list->total++;
}

int utlDListRemoveNode(utlDList *list, utlDListNode *node)
{
  if(list != NULL && list->total>0 && node != NULL)
  {
    if(node == list->top) list->top = list->top->next;
    if(node == list->bottom) list->bottom = list->bottom->previous;
    if(node->next != NULL) node->next->previous = node->previous;
    if(node->previous != NULL) node->previous->next = node->next;
    node->next = NULL;
    node->previous = NULL;
    node->value = NULL;
    list->total--;
    return 1;
  }
  return 0;
}

utlDListNode* utlDListRemove(utlDList *list, void *value)
{
  utlDListNode *node = list->top;

  for(; node != NULL; node = node->next)
  {
    if(node->value == value)
    {
      utlDListRemoveNode(list, node);
      return node;
    }
  }
  return NULL;
}

utlDListNode* utlDListFindValue(utlDList *list, void *value)
{
  utlDListNode *node;

  for(node = list->top; node!=NULL; node = node->next)
  {
    if(node->value==value)
      return node;
  }
  return NULL;
}

utlDListNode* utlDListFindStr(utlDList *list, char *value)
{
  utlDListNode *node;

  for(node = list->top; node!=NULL; node = node->next)
  {
    if(!strcmp(node->value,value))
      return node;
  }
  return NULL;
}
