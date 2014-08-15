/** \file
 * \brief IupScanf implementation.
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iup.h"
#include "ipredial.h"
#include "istrutil.h"

#define ALLOC(n,t)  ((t *)calloc((n),sizeof(t)))
#define REQUIRE(b)	{if (!(b)) goto cleanup;}

int IupScanf (const char *format, ...)
{
  int i;
  int rc=(-1);		/* return code if not error (erro <  0) */
  int erro=(-1);		/* return code if error     (erro >= 0) */
  int fields;
  int *width=NULL;
  int *scroll=NULL;
  char **prompt=NULL;
  char **text=NULL;
  char *title=NULL;
  char *s=NULL;
  char *s1=NULL;
  char *outf=NULL;
  va_list va;

  fields=iupStrCountChar(format,'\n')-1;
  REQUIRE(fields>0);
  width=ALLOC(fields, int);
  REQUIRE(width!=NULL);
  scroll=ALLOC(fields, int);
  REQUIRE(scroll!=NULL);
  prompt=ALLOC(fields, char *);
  REQUIRE(prompt!=NULL);
  text=ALLOC(fields, char *);
  REQUIRE(text!=NULL);

  va_start(va,format);
  REQUIRE ((s1=s=(char *)iupStrDup(format)) != NULL);
  title=iupStrCopyUntil(&s,'\n');
  REQUIRE(title!=NULL);
  for (i=0; i<fields; ++i)
  {
    int n;
    prompt[i]=iupStrCopyUntil(&s,'%');
    REQUIRE(prompt[i]!=NULL);
    n=sscanf(s,"%d.%d",width+i,scroll+i);
    REQUIRE(n==2);
    s=strchr(s,'%');
    REQUIRE(s!=NULL);
    if (outf) free(outf);
    outf = iupStrCopyUntil(&s, '\n');
    text[i]=ALLOC(width[i]+1,char);
    REQUIRE(text[i]!=NULL);

    switch (s[-2])
    {
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
      if (s[-3]=='l')
        sprintf(text[i],outf,*((long *)va_arg(va,long *)));
      else if (s[-3]=='h')
        sprintf(text[i],outf,*((short *)va_arg(va,short *)));
      else
        sprintf(text[i],outf,*((int *)va_arg(va,int *)));
      break;
    case 'e':
    case 'f':
    case 'g':
    case 'E':
    case 'G':
      if (s[-3]=='l')
        sprintf(text[i],outf,*((double *)va_arg(va,double *)));
      else
        sprintf(text[i],outf,*((float *)va_arg(va,float *)));
      break;
    case 's':
      sprintf(text[i],outf,((char *)va_arg(va,char *)));
      break;
    default:
      goto cleanup;
    }
  }
  va_end(va);

  REQUIRE(iupDataEntry(fields,width,scroll,title,prompt,text)>0);

  rc=0;
  va_start(va,format);
  s=strchr(format,'\n')+1;
  for (i=0; i<fields; ++i)
  {
    s=strchr(s,'\n')+1;
    switch (s[-2])
    {
    case 'd':
    case 'u':
      if (s[-3]=='l')
      {
        if (sscanf(text[i],"%ld",((long *)va_arg(va,long *)))!=1)
          if (erro < 0) erro = rc;
      }
      else if (s[-3]=='h')
      {
        if (sscanf(text[i],"%hd",((short *)va_arg(va,short *)))!=1)
          if (erro < 0) erro = rc;
      }
      else
      {
        if (sscanf(text[i],"%d",((int *)va_arg(va,int *)))!=1)
          if (erro < 0) erro = rc;
      }
      break;
    case 'i':
    case 'o':
    case 'x':
    case 'X':
      if (s[-3]=='l')
      {
        if (sscanf(text[i],"%li",((long *)va_arg(va,long *)))!=1)
          if (erro < 0) erro = rc;
      }
      else if (s[-3]=='h')
      {
        if (sscanf(text[i],"%hi",((short *)va_arg(va,short *)))!=1)
          if (erro < 0) erro = rc;
      }
      else
      {
        if (sscanf(text[i],"%i",((int *)va_arg(va,int *)))!=1)
          if (erro < 0) erro = rc;
      }
      break;
    case 'e':
    case 'f':
    case 'g':
    case 'E':
    case 'G':
      if (s[-3]=='l')
      {
        if (sscanf(text[i],"%lg",((double *)va_arg(va,double *)))!=1)
          if (erro < 0) erro = rc;
      }
      else
      {
        if (sscanf(text[i], "%g", ((float *)va_arg(va,float *)))!=1)
          if (erro < 0) erro = rc;
      }
      break;
    case 's':
      {
        char *s=(char *)va_arg(va,char *);
        strcpy(s,text[i]);
      }
      break;
    }
    ++rc;
  }
  va_end(va);

cleanup:
  if (s1) free(s1);
  if (title) free(title);
  if (width) free(width);
  if (scroll) free(scroll);
  if (outf) free(outf);
  if (prompt)
  {
    for (i=0; i<fields; ++i)
      if (prompt[i]) free(prompt[i]);
    free(prompt);
  }
  if (text)
  {
    for (i=0; i<fields; ++i)
      if (text[i]) free(text[i]);
    free(text);
  }

  return (erro < 0) ? rc : erro;
}

