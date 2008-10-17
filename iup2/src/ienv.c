/** \file
 * \brief attributes enviroment manager
 *
 * See Copyright Notice in iup.h
 * $Id: ienv.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdio.h>   
#include <string.h>  
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>

#include "iglobal.h"
#include "idrv.h"

int IupGetAllAttributes(Ihandle* ih, char *names[], int n)
{
  char *name;
  int i = 0;

  name = iupTableFirst(env(ih));
  while (name)
  {
    strcpy(names[i], name);
    i++;
    if (i == n)
      break;

    name = iupTableNext(env(ih));
  }

  return i;
}

int iupCheck(Ihandle *n, const char *a)
{
 char* v = IupGetAttribute (n, a);
 if (v == NULL)
  return NOTDEF;
 else if (iupStrEqualNoCase(v, IUP_NO) || iupStrEqualNoCase(v, IUP_OFF))
  return NO;
 else if (iupStrEqualNoCase(v, IUP_YES) || iupStrEqualNoCase(v, IUP_ON))
  return YES;
 return NOTDEF;
}

char* IupGetAttributes (Ihandle *n)
{
 static char *buffer=NULL;
 char *a, *v;
 char sb[128];
 int first = 1;
 if ((n == NULL) || (env(n) == NULL))
  return NULL;

 if (buffer == NULL)
 {
  buffer = (char *) malloc (sizeof(char)*10240);
  if (buffer == NULL) return NULL;
 }

 buffer[0]='\0';
 a=iupTableFirst(env(n));
 while (a)
 {
  if (!first)
   strcat(buffer,",");
  first=0;
  v = IupGetAttribute(n,a);
  if (iupIsPointer(a))
  {
   sprintf(sb, "%p", (void*) v);
   v = sb;
  }
  strcat(buffer, a); strcat(buffer,"=\"");
  strcat(buffer, v);
  strcat(buffer,"\"");
  a=iupTableNext(env(n));
 }
 return buffer;
}

void IupSetAttribute (Ihandle *n, const char* a, const char *v)
{
 const char *toenv = v;
 if ((n == NULL) || (a == NULL))
  return;
 if (v && handle(n) && !iupdrvStoreAttribute(n,a))
   toenv = NULL;
 iupSetEnv (n, a, toenv);
 iupCpiSetAttribute (n, a, v);
}

void IupStoreAttribute (Ihandle *n, const char* a, const char *v)
{
 const char *toenv = v;
 if ((n == NULL) || (a == NULL))
  return;
 if (v && handle(n) && !iupdrvStoreAttribute(n,a))
   toenv = NULL;
 iupStoreEnv (n, a, toenv);
 iupCpiSetAttribute (n, a, v);
}

char* IupGetAttribute (Ihandle *n, const char* a)
{
 char *v=NULL;
 if ((n == NULL) || (a == NULL))
  return NULL;
 v=iupCpiGetAttribute (n,a);
 if (v == NULL)
  v=iupGetEnv (n, a);
 if (v == NULL)
  v = iupCpiGetDefaultAttr (n,a);
 return v;
}

float IupGetFloat (Ihandle *n, const char* a)
{
 float f=0.0F;
 char *s=IupGetAttribute (n,a);
 if (s)
  sscanf (s,"%f",&f);
 return f;
}

int IupGetInt (Ihandle *n, const char* a)
{
  int i=0;
  char *s=IupGetAttribute (n,a);
  if (s)
  {
    if (sscanf (s,"%d",&i) != 1) /* if not just one field or failed */
    {
      if (iupStrEqual(s, IUP_YES) || iupStrEqual(s, IUP_ON))
        i = 1;
    }
  }
  return i;
}

int IupGetInt2 (Ihandle *n, const char* a)
{
  int d, i=0;
  char *s=IupGetAttribute (n,a);
  if (s)
  {
    if (!iupStrToIntInt(s,&d,&i,'x'))
      iupStrToIntInt(s,&d,&i,':');
  }
  return i;
}

void IupSetfAttribute (Ihandle *n, const char* a, const char* f, ...)
{
  static char v[SHRT_MAX];
  va_list arglist;
  va_start(arglist, f);
  vsprintf(v, f, arglist);
  va_end (arglist);
  IupStoreAttribute(n, a, v);
}

void IupSetAttributeHandle(Ihandle *ih, const char* name, Ihandle *ih_named)
{
  char* handle_name = IupGetName(ih_named);
  if (handle_name)
    IupStoreAttribute(ih, name, handle_name);
  else
  {
    char str_name[100];
    sprintf(str_name, "_IUP_NAME(%p)", ih_named);
    IupSetHandle(str_name, ih_named);
    IupStoreAttribute(ih, name, str_name);
  }
}

Ihandle* IupGetAttributeHandle(Ihandle *ih, const char* name)
{
  char* handle_name = IupGetAttribute(ih, name);
  if (handle_name) 
    return IupGetHandle(handle_name);
  return NULL;
}

void iupSetEnv (Ihandle* n, const char* a, const char* v)
{
 if (n == NULL)
  return;
 if (v==NULL)
  iupTableRemove(env(n), a);
 else
  iupTableSet(env(n), a, (void*)v, IUP_POINTER);
}

void iupStoreEnv (Ihandle* n, const char* a, const char* v)
{
 if (n == NULL)
  return;
 if (v==NULL)
  iupTableRemove (env(n), a);
 else
  iupTableSet(env(n), a, (void*)v, IUP_STRING);
}

char* iupGetEnv (Ihandle* n, const char* a)
{
 char *v;
 if (n == NULL)
  return NULL;
 v = iupTableGet (env(n), a);
 if (v == NULL)
   if(iupIsInheritable(a))
     v = iupGetEnv (parent(n), a);
 return v;
}

int iupGetEnvInt (Ihandle* n, const char* a)
{
 char *v = iupGetEnv(n, a);
 if (!v) return 0;
 return atoi(v);
}

char* iupGetEnvRaw (Ihandle* n, const char* a)
{
 if (n == NULL || a == NULL)
  return NULL;
 return iupTableGet (env(n), a);
}

int iupGetWidth (Itable* e)
{
 int i=0;
 char *v = iupTableGet (e, IUP_WIDTH);
 if (v)
  sscanf (v, "%d", &i);
 return i;
}

int iupGetHeight (Itable* e)
{
 int i=0;
 char *v = iupTableGet (e, IUP_HEIGHT);
 if (v)
  sscanf (v, "%d", &i);
 return i;
}

int iupGetRGB (const char *color, unsigned int *r, unsigned int *g, unsigned int *b)
{
 if (!color) return 0;
 if (sscanf (color, "%u %u %u", r, g, b) != 3)
 {
  Ihandle *h=IupGetHandle (color);
  if (!h) return 0;
  *r=(unsigned int)IupGetInt((h),IUP_RED);
  *g=(unsigned int)IupGetInt((h),IUP_GREEN);
  *b=(unsigned int)IupGetInt((h),IUP_BLUE);
 }
 return 1;
}

int iupGetColor (Ihandle* n, const char *a, unsigned int *r, unsigned int *g, unsigned int *b)
{
 return iupGetRGB( IupGetAttribute (n, a), r, g, b );
}

static const char* env_str = NULL;
static void iupEnvCapture (char* env_buffer, char* dlm)
{
  int i=0;
  int c;
  do
  {
    c = *env_str; ++env_str;
    if (i < 256)
      env_buffer[i++] = (char) c;
  } while (c && !strchr (dlm,c));
  env_buffer[i-1]='\0';                                /* discard delimiter */
}

static int iupEnvToken (char* env_buffer)
{
 for (;;)
 {
  int c = *env_str; ++env_str;
  switch (c)
  {
   case 0:
    return TK_END;

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

   case '\"':					/* string */
    iupEnvCapture (env_buffer, "\"");
    return TK_NAME;

   default:
    if (c > 32)  				/* identifier */
    {
     --env_str;                     /* unget first character of env_buffer */
     iupEnvCapture (env_buffer, "=, \t\n\r\f\v"); /* get env_buffer until delimiter */
     --env_str;                     /* unget delimiter */
     return TK_NAME;
    }
  }
 }
}

static void iupEnvParse(Ihandle *n, const char* attr)
{
  char env_buffer[256];
  char* a=NULL;
  char* v=NULL;
  int state = 'a';               /* get attribute */
  int end = 0;

  env_str = attr;

  for (;;)
  {
    switch (iupEnvToken(env_buffer))
    {
    case TK_END:                 /* procedimento igual ao TK_COMMA */
      end = 1;
    case TK_COMMA:
      if (a != NULL)
      {
        iupTableSet(env(n), a, v, IUP_STRING);
        iupCpiSetAttribute (n, a, v);
        free(a);
      }
      if (end)
        return;
      a=v=NULL;
      state = 'a';
      break;

    case TK_SET:
      state = 'v';                /* get value */
      break;

    case TK_NAME:
      if (state == 'a')
        a = iupStrDup(env_buffer);
      else
        v = env_buffer;
      break;
    }
  }
}

Ihandle* IupSetAttributes (Ihandle *n, const char* e)
{
 if (n == NULL)
  return n;
 if (e != NULL)
  iupEnvParse(n,e);
 return n;
}


/* returns 1 if attribute is inheritable, or 0 otherwise */
int iupIsInheritable(const char *attr)
{
/* table of non-inheritable attributes */
static struct {
  const char *attr;
} inh_table[] = {
  { "SIZE" },
  { "RASTERSIZE" },
  { "X" },
  { "Y" },
  { "VALUE" },
  { "TITLE" },
  { "_IUPLUA_WIDGET_TABLE_REF" },
  { "ALIGNMENT" },
};
#define INH_TABLE_SIZE ((sizeof inh_table)/(sizeof inh_table[0])) 

  if (attr)
  {
    int i;
    for (i = 0; i < INH_TABLE_SIZE; i++)
    {
      if (iupStrEqualNoCase(attr, inh_table[i].attr))
        return 0;
    }
  }
  return 1;
}

int iupIsInternal(const char* name)
{
  if (name[0] == '_' &&
      name[1] == 'I' &&
      name[2] == 'U' &&
      name[3] == 'P')
    return 1;
  return 0;
}

int iupIsPointer(const char* attr)
{
static struct {
  const char *attr;
} ptr_table[] = {
  { "CONID" },
  { "WID" },
#ifdef WIN32
  { "HWND" },
  { "HFONT" },
  { "NATIVEPARENT"  },
  { "PREVIEWDC" },
  { "HINSTANCE" },
#else
  { "CONTEXT" },          
  { "VISUAL" },
  { "COLORMAP" },
  { "XDISPLAY" },
  { "XWINDOW" },
  { "XSCREEN" },
#endif
  { "IUNKNOWN" },  
  { "_IUPLUA_STATE_CONTEXT" }
};
#define PTR_TABLE_SIZE ((sizeof ptr_table)/(sizeof ptr_table[0])) 

  if (attr)
  {
    int i;
    for (i = 0; i < PTR_TABLE_SIZE; i++)
    {
      if (iupStrEqualNoCase(attr, ptr_table[i].attr))
        return 1;
    }
  }

  return 0;
}

char *iupGetParentBgColor (Ihandle* self)
{
  /* check native definition from parent */
  /* will check also for user definitions up to the dialog */
  char *color = IupGetAttribute(IupGetParent(self), IUP_BGCOLOR);

  /* if none check the native definition for the dialog */
  /* this will return the default value for the dialog */
  if (!color) 
    color = IupGetAttribute(IupGetDialog(self), IUP_BGCOLOR);

  if (!color) /* unlikely, but just in case */
    color = "128 128 128";

  return color;
}

char *iupGetBgColor(Ihandle* self)
{
  /* IupCanvas has a default value for BGCOLOR  */
  /* using this function we avoid this default value */

  /* do not check native definition */
  /* check only user definitions up to the dialog */
  char *color = iupGetEnv(self, IUP_BGCOLOR);

  /* If not defined check native definition from parent */
  if (!color)
    color = iupGetParentBgColor(self);

  return color;
}

