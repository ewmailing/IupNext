/** \file
 * \brief String Utilities
 *
 * See Copyright Notice in iup.h
 * $Id: istrutil.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

 
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <stdlib.h>
#include <assert.h>

#include "istrutil.h"

int iupStrEqual(const char* str1, const char* str2) 
{
  if (str1 == str2) return 1;
  if (!str1 || !str2 || *str1 != *str2) return 0;
  return (strcmp(str1, str2)==0)? 1: 0;
}

int iupStrEqualNoCase(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2 || tolower(*str1) != tolower(*str2)) return 0;

  while (str1[i] && str2[i] && tolower(str1[i])==tolower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1; 

  return 0;
}

int iupStrEqualPartial(const char* str1, const char* str2) 
{
  if (str1 == str2) return 1;
  if (!str1 || !str2 || *str1 != *str2) return 0;
  return (strncmp(str1, str2, strlen(str2))==0)? 1: 0;
}

char *iupStrDup(const char *str)
{
  if (str)
  {
    int size = strlen(str)+1;
    char *newstr = malloc(size);
    if (newstr) memcpy(newstr, str, size);
    return newstr;
  }
  return NULL;
}

const char* iupStrNextLine(const char* str, int *len)
{
  *len = 0;

  assert(str);

  while(*str != 0 && *str != '\n') 
  {
    if (!(*str == '\r' && *(str+1) == '\n')) /* detects CR/LF pairs */
      (*len)++;

    str++;
  }

  /* There is a next line ? */
  if (*str == 0)
    return str;  
  else
    return str+1;
}

void iupStrLineCount(const char* str, int *max_len, int *num_lin)
{
  *num_lin = 1;
  *max_len = 0;

  assert(str);

  while(*str != 0)
  {
    int len = 0;

    while(*str != 0 && *str != '\n') 
    {
      if (!(*str == '\r' && *(str+1) == '\n')) /* detects also CR/LF pairs */
        len++;

      str++;
    }

    if (len > *max_len)
      *max_len = len;

    if (*str == '\n')
    {
      (*num_lin)++;
      str++;
    }
  }
}

int iupStrCountChar(const char *str, int c)
{
  int n;
  if (!str) return 0;
  for(n=0; *str; str++)
  {
    if (*str==(char)c)
      n++;
  }
  return n;
}

char *iupStrCopyUntil(char **str, int c)
{
  char *p_str,*new_str;
  if (!str || *str==NULL)
    return NULL;

  p_str=strchr(*str,c);
  if (!p_str) return NULL;

  {
    int i;
    int sl=(int)(p_str - (*str));

    new_str = (char *) malloc (sl + 1);
    if (!new_str) return NULL;

    for (i = 0; i < sl; ++i)
      new_str[i] = (*str)[i];

    new_str[sl] = 0;
  }

  *str = p_str+1;
  return new_str;
}

char *iupStrGetMemory(int size)
{
#define MAX_BUFFERS 20
  static char* buffers[MAX_BUFFERS];
  static int buf_sizes[MAX_BUFFERS];
  static int buf_index = -1;

  int i;

  if(size == -1) /* Frees memory */
  {
    buf_index = -1;
    for (i = 0; i < MAX_BUFFERS; i++)
    {
      if (buffers[i]) 
      {
        free(buffers[i]);
        buffers[i] = NULL;
      }
      buf_sizes[i] = 0;
    }
    return NULL;
  }
  else
  {
    char* ret_buf;

    if (buf_index == -1)
    {
      memset(buffers, 0, sizeof(char*)*MAX_BUFFERS);
      memset(buf_sizes, 0, sizeof(int)*MAX_BUFFERS);
      buf_index = 0;
    }

    if (!(buffers[buf_index]))
    {
      buf_sizes[buf_index] = size+1;
      buffers[buf_index] = (char*)malloc(buf_sizes[buf_index]);
    }
    else if (buf_sizes[buf_index] < size+1)
    {
      buf_sizes[buf_index] = size+1;
      buffers[buf_index] = (char*)realloc(buffers[buf_index], buf_sizes[buf_index]);
    }

    memset(buffers[buf_index], 0, buf_sizes[buf_index]);
    ret_buf = buffers[buf_index];

    buf_index++;
    if (buf_index == MAX_BUFFERS)
      buf_index = 0;

    return ret_buf;
  }
}

int iupStrToIntInt(const char *str, int *i1, int *i2, char c)
{
  if (!str) return 0;

  if (*str == c) /* no first value */
  {
    str++; /* skip separator */
    if (sscanf(str, "%d", i2) != 1) return 0;
    return 1;
  }
  else 
  {
    char* p_str = iupStrCopyUntil((char**)&str, c);
 
    if (p_str==NULL ||  /* no separator means no second value */
        *str==0)        /* second value empty also means no second value */
    {                               
      if (sscanf(str, "%d", i1) != 1) return 0;
      return 1;
    }
    else
    {
      int ret = 0;
      if (sscanf(p_str, "%d", i1) == 1) ret++;
      if (sscanf(str, "%d", i2) == 1) ret++;
      free(p_str);
      return ret;
    }
  }
}

