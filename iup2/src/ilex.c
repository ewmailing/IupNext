/** \file
 * \brief lexical analysis manager for LED
 *
 * See Copyright Notice in iup.h
 * $Id: ilex.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdarg.h>			/* va_start, va_arg, va_end */
#include <stdio.h>			/* FILE, BUFSIZ, NULL */
#include <stdlib.h>			/* exit */
#include <ctype.h>
#include <string.h>                     /* strchr */
#include <errno.h>			/* errno, sys_errlist */

#include "iglobal.h"

static struct					/* lexical variables */
{
  const char* filename;                                /* file name */
  FILE* file;                                      /* file handle */
  int iLexToken; 				        /* lookahead iLexToken */
  char name[40960];				/* lexical identifier value */
  float number;      				/* lexical number value */
  char* func;      				/* lexical iLexFunction value */
  int line;                                      /* line number */
  Iclass *ic;                                    /* control class when func is CONTROL_ */
} ilex = {NULL, NULL, 0, "", (float) 0.0, 0, 0, NULL};

static int iLexGetChar (void);
static int iLexToken(int *erro);
static int iLexCapture (char* dlm);
static int iLexSkip (char* dlm);
static char* iLexFunction (char *f);
static int iLexCaptureAttr (void);


int iupLexStart(const char* filename)			/* initialize lexical analysis */
{
  ilex.filename = filename;
  ilex.file = fopen (ilex.filename,"r");
  ilex.line = 0;
  if (!ilex.file)
    return iupLexError (FILENOTOPENED, filename);
  ilex.line = 1;
  return iupLexAdvance();
}

void iupLexClose(void)
{
  if (!ilex.file)
    return;
  fclose (ilex.file);
  ilex.file = NULL;
}

int iupLexLookAhead(void)
{
  return ilex.iLexToken;
}

int iupLexAdvance(void)
{
  int erro=0;
  ilex.iLexToken=iLexToken(&erro);
  return erro;
}

int iupLexFollowedBy(int t)
{
  return (ilex.iLexToken==t);
}

int iupLexMatch(int t)
{
  if (ilex.iLexToken==t)
    return iupLexAdvance();
  else
    return iupLexError (NOTMATCH, ilex.iLexToken, t);
}


int iupLexSeenMatch(int t, int *erro)
{
  if (ilex.iLexToken==t)
  {
    *erro=iupLexAdvance();
    return 1;
  }
  else
    return 0;
}

int iupLexInt(void)
{
  int c;
  sscanf(ilex.name,"%d",&c);
  return c;
}

char* iupLexGetName(void)
{
  /* memory leak - to be fixed */
  if(ilex.name)
    return iupStrDup(ilex.name);
  else
    return NULL;
}

char* iupLexName(void)
{
  return ilex.name;
}

float iupLexGetNumber(void)
{
  return ilex.number;
}

char* iupLexGetFunc (void)
{
  return ilex.func;
}

Iclass *iupLexGetClass(void)
{
  return ilex.ic;
}

static int iLexToken(int *erro)
{
  for (;;)
  {
    int c = iLexGetChar ();
    switch (c)
    {
    case 26:
    case EOF:
      return TK_END;

    case ']':
      return TK_ENDATTR;

    case '#':					/* iLexSkip comment */
    case '%':					/* iLexSkip comment */
      iLexSkip ("\n\r");
      continue;

    case ' ':					/* ignore whitespace */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
    case '\v':
      continue;

    case '=':					/* attribuicao */
      return TK_SET;

    case ',':
      return TK_COMMA;

    case '(':					/* begin parameters */
      return TK_BEGP;

    case ')':					/* end parameters */
      return TK_ENDP;

    case '[':					/* attributes */
      if (iLexCaptureAttr() == TK_END)
      {
        *erro=iupLexError (NOTENDATTR);
        return 0;
      }
      return TK_ATTR;

    case '\"':					/* string */
      iLexCapture ("\"");
      return TK_STR;

    case '\'':					/* string */
      iLexCapture ("\'");
      return TK_STR;

    default:
      if (c > 32)  				/* identifier */
      {
        char* fntype;
        ungetc (c,ilex.file);
        ungetc (iLexCapture ("=[](), \t\n\r\f\v"), ilex.file);
        fntype = iLexFunction(iupLexName());
        if (fntype != UNKNOWN_)
        {
          ilex.func = fntype;
          return TK_FUNC;
        }
        else
          return TK_NAME;
      }
    }
    return c;
  }
}

static int iLexCapture (char* dlm)
{
  int i=0;
  int c;
  do
  {
    c = iLexGetChar ();
    if (i < sizeof(ilex.name))
      ilex.name[i++] = (char) c;
  } while ((c > 0) && !strchr (dlm,c));
  ilex.name[i-1]='\0';                            /* discard delimiter */
  return c;                                      /* return delimiter */
}

static int iLexCaptureAttr (void)
{
  int i=0;
  int c;
  int aspas=0;
  do
  {
    c = iLexGetChar ();
    if (i < sizeof(ilex.name))
      ilex.name[i++] = (char) c;
    if (c == '"')
      ++aspas;
  } while ((c > 0) && ((aspas & 1) || c != ']'));
  ilex.name[i-1]='\0';                            /* discard delimiter */
  return c;                                      /* return delimiter */
}

static int iLexSkip (char* dlm)
{
  int c;
  do
  {
    c = iLexGetChar ();
  } while ((c > 0) && !strchr (dlm,c));
  return c;                                      /* return delimiter */
}

static int iLexGetChar (void)
{
  int c = getc (ilex.file); if (c == '\n') ++ilex.line;
  if (c == '\\')
  {
    c = getc(ilex.file);
    if (c == 'n')
      return '\n';
    else if (c == '\\')
      return '\\';
  }
  return c;
}

static Itable *ilex_functable = NULL;

void iupLexFinish(void)
{
  iupTableDestroy(ilex_functable);
  ilex_functable = NULL;
}

void iupLexInit(void)
{
  ilex_functable = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupLexRegisterLED (const char *name, Iclass *ic)
{
  char lowercase[80];
  int i;
  for (i=0; *name; ++name, ++i)
    lowercase[i]=(char)tolower(*name);
  lowercase[i]='\0';
  if (name && ic)
    iupTableSet(ilex_functable, lowercase, ic, IUP_POINTER);
}

static char* iLexFunction (char *f)
{
  if (iupStrEqualNoCase (f, "user"))      return USER_;
  else if (iupStrEqualNoCase (f, "image"))     return IMAGE_;
  else if (iupStrEqualNoCase (f, "button"))    return BUTTON_;
  else if (iupStrEqualNoCase (f, "canvas"))    return CANVAS_;
  else if (iupStrEqualNoCase (f, "dialog"))    return DIALOG_;
  else if (iupStrEqualNoCase (f, "fill"))      return FILL_;
  else if (iupStrEqualNoCase (f, "frame"))     return FRAME_;
  else if (iupStrEqualNoCase (f, "hbox"))      return HBOX_;
  else if (iupStrEqualNoCase (f, "item"))      return ITEM_;
  else if (iupStrEqualNoCase (f, "hotkeys"))   return KEYACTION_;
  else if (iupStrEqualNoCase (f, "label"))     return LABEL_;
  else if (iupStrEqualNoCase (f, "list"))      return LIST_;
  else if (iupStrEqualNoCase (f, "menu"))      return MENU_;
  else if (iupStrEqualNoCase (f, "radio"))     return RADIO_;
  else if (iupStrEqualNoCase (f, "separator")) return SEPARATOR_;
  else if (iupStrEqualNoCase (f, "submenu"))   return SUBMENU_;
  else if (iupStrEqualNoCase (f, "text"))      return TEXT_;
  else if (iupStrEqualNoCase (f, "multiline")) return MULTILINE_;
  else if (iupStrEqualNoCase (f, "toggle"))    return TOGGLE_;
  else if (iupStrEqualNoCase (f, "vbox"))      return VBOX_;
  else if (iupStrEqualNoCase (f, "zbox"))      return ZBOX_;
  else
  {
    char lowercase[80];
    int i;
    for (i=0; *f; ++f, ++i)
      lowercase[i]=(char)tolower(*f);
    lowercase[i]='\0';
    ilex.ic=iupTableGet(ilex_functable, lowercase) ;
    return ilex.ic ? CONTROL_ : UNKNOWN_;
  }
}

static char* iupTokenStr(int t)
{
  switch (t)
  {
  case TK_END  : return "end of file";
  case TK_BEGP : return "(";
  case TK_ENDP : return ")";
  case TK_ATTR : return "[";
  case TK_STR  : return "string";
  case TK_NAME : return "identifier";
  case TK_NUMB : return "number";
  case TK_SET  : return "=";
  case TK_COMMA: return ",";
  case TK_FUNC : return "iLexFunction name";
  case TK_ENDATTR : return "]";
  }
  return "";
}

static char ilex_erromsg[512];

char *iupLexGetError (void)
{
  return ilex_erromsg;
}

int iupLexError (int n, ...)
{
  char msg[256];
  va_list va;
  va_start(va,n);
  switch (n)
  {
  case FILENOTOPENED:
    {
      char *fn=va_arg(va,char *);
      sprintf (msg, "cannot open file %s", fn);
    }
    break;
  case NOTMATCH:
    {
      int tr=va_arg(va,int);        /* iLexToken read */
      int te=va_arg(va,int);        /* iLexToken expected */
      char *str=iupTokenStr(tr);
      char *ste=iupTokenStr(te);
      sprintf (msg, "expected %s but found %s", ste, str);
    }
    break;
  case NOTENDATTR:
    {
      sprintf (msg, "missing ']'");
    }
    break;
  case PARSEERROR:
    {
      char* s=va_arg(va,char*);        /* iLexToken expected */
      sprintf(msg,"%.*s",(int)(sizeof(msg)-1),s);
    }
  }
  sprintf (ilex_erromsg, "led(%s): bad input at line %d - %s\n", ilex.filename, ilex.line, msg);
  va_end(va);
  return n;
}
