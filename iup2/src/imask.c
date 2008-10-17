/** \file
 * \brief iupmask binding for Lua 5.
 *
 * See Copyright Notice in iup.h
 * $Id: imask.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "imask.h"

static int getnumber(char* mask, int pos, int* number)
{
   char n[6];
   int i = 0;
   while (mask[pos]>='0' && mask[pos]<='9')
   {
      n[i] = mask[pos];
      i++;
      pos++;
   }
   n[i] = 0;
   *number = atoi(n);
   return pos;
}

static ImaskElem* createnode( ImaskElem* curr, int i, int j )
{
   curr->next = (ImaskElem*)malloc(sizeof(ImaskElem));
   curr->next->min = (i<=j)?i:j;
   curr->next->max = (i<=j)?j:i;
   curr->next->elem = NULL;
   curr->next->next = NULL;
   return curr->next;
}

static void insertset( ImaskElem* currptr, char* str, int notset )
{
   ImaskSet* set = (ImaskSet*) malloc(sizeof(ImaskSet));

   currptr->type = MASKSET;
   currptr->elem = set;
   strcpy(set->str, str);
   set->len = strlen(str);
   set->not = notset;
}

static void insertchar( ImaskElem* currptr, char c )
{
   ImaskChar* nc = (ImaskChar*) malloc(sizeof(ImaskChar));

   currptr->type = MASKCHAR;
   currptr->elem = nc;
   nc->ch = c;
}

static void destroymask( ImaskElem* mask )
{
   ImaskElem* ptr;
   while (mask)
   {
      ptr = mask;
      if (mask->elem)
         free(mask->elem);
      mask = mask->next;
      free(ptr);
   }
}


ImaskElem* iupCreateMask(char *mask)
{
   ImaskElem* maskptr = NULL;
   ImaskElem* currptr = NULL;
   int currpos   = 0;
   int backslash = 0;
   int brackets  = 0;
   int notset    = 0;
   char buffer[50];


   if (mask == NULL)
      return NULL;


   maskptr = (ImaskElem*) malloc(sizeof(ImaskElem));
   maskptr->elem = NULL;
   maskptr->next = NULL;
   if (mask[0]=='$')
   {
      maskptr->type = MASKNOAUTOFILL;
      currpos++;
   }
   else
   {
      maskptr->type = MASKAUTOFILL;
   }
   currptr = maskptr;

   while (mask[currpos] != 0)
   {
      switch (mask[currpos])
      {
         case '[':
            if (brackets)
            {
               destroymask(maskptr);
               return NULL;
            }
            else if (backslash)
            {
               backslash = 0;
            }
            else
               currptr = createnode(currptr,0,1);

            brackets = 1;
            buffer[0] = 0;
            currpos ++;

            if (mask[currpos] == '~')
            {
               notset = 1;
               currpos++;
            }
            else
               notset =0;
            break;
         case ']':
            if (backslash || !brackets)
            {
               destroymask(maskptr);
               return NULL;
            }
            else
            {
               /*insere o conjunto*/
               insertset(currptr,buffer,notset);
               brackets = 0;
               currpos++;
            }
            break;
         case '*':
            if (brackets)
            {
               strcat(buffer,"*");
            }
            else if (backslash)
            {
               insertchar(currptr,MASKALLCHARS);
               backslash = 0;
            }
            else
            {
               currptr = createnode(currptr,0,1);
               insertchar(currptr,MASKALLCHARS);
            }
            currpos++;
            break;
         case '#':
            if (brackets)
            {
              strcat(buffer,"0-9");
            }
            else if (backslash)
            {
               insertset(currptr,"0-9",0);
               backslash = 0;
            }
            else
            {
               currptr = createnode(currptr,0,1);
               insertset(currptr,"0-9",0);
            }
            currpos++;
            break;
         case '@':
            if (brackets)
            {
              strcat(buffer,"a-zA-Z");
            }
            else if (backslash)
            {
               insertset(currptr,"a-zA-Z",0);
               backslash = 0;
            }
            else
            {
               currptr = createnode(currptr,0,1);
               insertset(currptr,"a-zA-Z",0);
            }
            currpos++;
            break;
         case '\\':
            if (brackets)
            {
               /*currptr = createnode(currptr,1,1);*/
               /*insertchar(currptr,mask[currpos+1]);*/
               char auxstr[2];
               auxstr[1] = 0;
               auxstr[0] = mask[currpos+1];
               strcat(buffer,auxstr);
               currpos+=2;
               backslash = !backslash;
            }
            else if (backslash)
            {
               destroymask(maskptr);
               return NULL;
            }
            else
            {
               switch(mask[currpos+1])
               {
                  case 0:
                     destroymask(maskptr);
                     return NULL;
                  case '(':
                  {
                     int pos = currpos+2;
                     int n1 = 0, n2 = 0;

                     if (mask[pos]>='0' && mask[pos]<='9')
                        pos = getnumber(mask,pos,&n1);
                     else
                     {
                        destroymask(maskptr);
                        return NULL;
                     }
                     if (mask[pos]!='-')
                     {
                        destroymask(maskptr);
                        return NULL;
                     }
                     pos++;
                     if (mask[pos]>='0' && mask[pos]<='9')
                        pos = getnumber(mask,pos,&n2);
                     else
                     {
                        destroymask(maskptr);
                        return NULL;
                     }
                     if (mask[pos]!=')')
                     {
                        destroymask(maskptr);
                        return NULL;
                     }
                     currpos = pos+1;

                     currptr = createnode(currptr,n1,n2);
                     break;
                  }
                  default:
                  {
                     if (mask[currpos+1]>='0' && mask[currpos+1]<='9')
                     {
                        int n = 0;
                        currpos = getnumber(mask, currpos+1, &n);
                        currptr = createnode(currptr,0,n);
                     }
                     else
                     {
                        currptr = createnode(currptr,1,1);
                        insertchar(currptr,mask[currpos+1]);
                        currpos+=2;
                        backslash = !backslash;
                     }
                     break;
                  }
               }
            }
            backslash = !backslash;
            break;
         default:
            if (brackets)
            {
               char str[2];
               str[0] = mask[currpos];
               str[1] = 0;
               strcat(buffer,str);
            }
            else if (backslash)
            {
               insertchar(currptr,mask[currpos]);
               backslash = 0;
            }
            else
            {
               currptr = createnode(currptr,1,1);
               insertchar(currptr,mask[currpos]);
            }
            currpos++;
            break;
      }
   }

   return maskptr;
}

static int       autofill;
static int       autopos;
static int       autocount;
static char      autostr[80];
static ImaskElem *automask;

static int matchSet(char c, char *set, int len, int not)
{
   int i = 0;
   for (i=0; i<len; i++)
   {
      if (set[i+1] == '-' && set[i+2] >= set[i]) /* faixa valida */
      {
         if (c >= set[i] && c <= set[i+2])
            return not ? 0 : 1;
         i += 2;
      }
      else
      {
         if (c == set[i]) return not ? 0 : 1;
      }
   }
   return not ? 1 : 0;
}

static int match(char c, int type, void* v)
{
   if (!c) return 0;
   if (type)
   {  
      return (c == ((ImaskChar*)v)->ch || ((ImaskChar*)v)->ch == 0);
   }
   else
   {
      return matchSet(c, ((ImaskSet*)v)->str, ((ImaskSet*)v)->len, ((ImaskSet*)v)->not);
   }
}

static void getFill(ImaskElem *me)
{
   char fillstr[80];
   ImaskElem *p = me;

   if (autocount) strcpy(fillstr, autostr);

   autopos = 0;
   while (p && p->min == p->max && p->type)
   {
      int i;
      for (i=0; i<p->min; i++)
         autostr[autopos++] = ((ImaskChar*)(p->elem))->ch;
      p = p->next;
   }
   automask = p;
   autostr[autopos] = 0;

   if (! (autocount && strcmp(fillstr, autostr) == 0)) autocount++;
}

static int missing(ImaskElem *p)
{
   while (p)
   {
      if (p->min) return 1;
      p = p->next;
   }
   return 0;
}

static int check(ImaskElem *me, char *str, int strpos)
{
   int localautofill = autofill;
   int counter = 0;
   int result = 0;
   
   if (!str[strpos] && !me) return 1; /* acabou a string e a mascara */
   if ( str[strpos] && !me) return 0; /* acabou a mascara mas nao a str */

   /* acabou a string mas nao a mascara */
   if (!str[strpos] &&  me)
   {
      if (autofill) getFill(me);
      return missing(me) ? 2 : 1;
   }

   while (counter < me->min)
   {
      if (!str[strpos])
      {
         if (autofill) getFill(me);
         return 2;
      }
      if (!match(str[strpos++], me->type, me->elem)) return 0;
      counter ++;
   }
/*
   autofill = 0;
*/
   while (counter < me->max)
   {
      result = check(me->next, str, strpos);
      if (result == 1) /* aceitou */ return 1;
      if (!match(str[strpos++], me->type, me->elem)) return result; 
      counter++;
   }
   autofill = localautofill;
   {
      int r = check(me->next, str, strpos);
      return r==1? 1 : (r == 2 ? 2 : result);
   }
}

int iupCheckMask(ImaskElem *m, char *str, char *fill)
{
   int result;
   if (!m) return 1;
   if (fill && m->type == MASKAUTOFILL)
   {
      autofill = 1;
      autocount = 0;
      autostr[0] = 0;
   }
   else
   {
      autofill = 0;
   }
   result = check(m->next, str, 0); 
   if (autofill && autocount == 1 && autostr[0])
   {
      strcpy(fill, autostr);
      result = missing(automask) ? 2 : 1;
   }
   else
   {
      if (fill) fill[0] = 0;
   }
   return result;
}

