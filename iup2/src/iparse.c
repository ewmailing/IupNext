/** \file
 * \brief parser for LED. implements IupLoad.
 *
 * See Copyright Notice in iup.h
 * $Id: iparse.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#define __PARSER__
#include <stdio.h>         /* NULL */
#include <assert.h>

#include "iglobal.h"
#include "itree.h"

#define SYMBEXIST       1
#define SYMBNOTDEF      2

static Ihandle* iParseExp (void);
static Ihandle* iParseFunction (char *fntype, Iclass *ic);
static int iParseError (int n, char *s);

static int iparse_error = 0;
#define RETURN_IF_ERRO(_)        {iparse_error=(_); if (iparse_error) return NULL;}

char *IupLoad (const char* fn)
{
 iparse_error=iupLexStart(fn);
 if (iparse_error)
  return iupLexGetError();

 while (iupLexLookAhead() != TK_END)
 {
  iParseExp ();
  if (iparse_error)
   return iupLexGetError();
 }

 iupLexClose ();
 return NULL;
}

static Ihandle* iParseExp (void)
{
 char* nm=NULL;
 Ihandle* n=NULL;
 int match=iupLexSeenMatch (TK_FUNC,&iparse_error);
 RETURN_IF_ERRO(iparse_error);

 if (match)
  return iParseFunction (iupLexGetFunc(),iupLexGetClass());

 if (iupLexLookAhead() == TK_NAME)
 {
  nm=iupLexGetName();
  RETURN_IF_ERRO(iupLexAdvance());
 }
 else
 {
  iparse_error=iupLexMatch (TK_NAME);
  return (Ihandle*)(NULL);  /* forca iparse_error */
 }

 match=iupLexSeenMatch (TK_SET,&iparse_error); RETURN_IF_ERRO(iparse_error);

 if (match)
 {
  /*  if (IupGetHandle(nm)) iParseError (SYMBEXIST, nm); */
  n = iParseExp ();  RETURN_IF_ERRO(iparse_error);
  IupSetHandle (nm, n);
 }
 else
 {
  n = IupGetHandle (nm);
  if (!n)
   RETURN_IF_ERRO(iParseError (SYMBNOTDEF,nm));
 }
 return n;
}

static char* parsecontrolparam( char type )
{
  switch(type)
  {
    case 'n':
      RETURN_IF_ERRO(iupLexMatch (TK_NAME));
      return iupLexGetName();

    case 's':
      RETURN_IF_ERRO(iupLexMatch (TK_STR));
      return iupLexGetName();

    case 'c':
    {
      char *result = (char*)iParseExp ();
      RETURN_IF_ERRO(iparse_error);
      return result;
    }

    default:
      assert(0); /* Erro */
      return 0;
  }
}

static Ihandle* parsecontrol(Iclass *ic)
{
  const char *format = iupCpiGetClassFormat(ic);
  if (format == NULL)
    return IupCreate(iupCpiGetClassName(ic)); /* might be better */
  else
  { 
    Ihandle *result;
    char** arg = NULL;
    int num_arg = strlen(format);
    int i = 0;

    assert(num_arg != 0); /* ou o formato e' nulo (nenhum
                             parametro) ou contem pelo menos
                             um parametro */

    arg = (char **) malloc (sizeof(char*)*(num_arg+1));
     
    for (i=0; i < num_arg; )
    {
      if (i > 0)
        RETURN_IF_ERRO(iupLexMatch (TK_COMMA));

      if (islower(format[i]))
      {
        arg[i] = parsecontrolparam(format[i]);
        i++;
        RETURN_IF_ERRO( iparse_error );
      }
      else
      {
        int j = i;
        do
        {
          if (num_arg == i)
            arg = realloc(arg, sizeof(char *) * ((++num_arg)+1));
          arg[i++] = parsecontrolparam( (char)tolower(format[j]) );
          RETURN_IF_ERRO( iparse_error );
        } while (iupLexSeenMatch (TK_COMMA,&iparse_error)); 
        RETURN_IF_ERRO(iparse_error);
        break;
      }
    }
        
    arg[i]=NULL;
    result = IupCreatev(iupCpiGetClassName(ic), (void**)arg); /* ver isso aqui... o name nao esta la pra todos... arg e' um vetor de string? */
    if(arg != NULL)
      free(arg);
    return result;
  }
}

static Ihandle* iParseFunction (char* fntype, Iclass *ic)
{
 Ihandle* n=NULL;
 char *attr=NULL;

 int match=iupLexSeenMatch (TK_ATTR,&iparse_error); RETURN_IF_ERRO(iparse_error);

 if (match)
  attr = (char*)iupStrDup(iupLexName());

 RETURN_IF_ERRO(iupLexMatch (TK_BEGP));

 if(fntype == IMAGE_)
 {
    unsigned i,j,k,width,height;
    unsigned char *pixmap;
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); width=iupLexInt();
    RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); height=iupLexInt();
    pixmap=(unsigned char *) malloc (sizeof(unsigned char)*width*height);
    if (pixmap != NULL)
    {
      k=0;
      for (j=0; j < height; ++j)
      for (i=0; i < width; ++i)
        {
        RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
        RETURN_IF_ERRO(iupLexMatch (TK_NAME));
        pixmap[k++]=(unsigned char)iupLexInt();
        }
      n = IupImage (width,height,(unsigned char*)pixmap);
      free(pixmap);
    }
 }
 else if(fntype == BUTTON_)
 {
    char* label;
    char* action;
    RETURN_IF_ERRO(iupLexMatch (TK_STR)); label = iupLexGetName();
    RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); action = iupLexGetName();
    n = IupButton (label, action);
 }
 else if(fntype == CANVAS_)
 {
    char* repaint;
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); repaint = iupLexGetName();
    n = IupCanvas (repaint);
 }
 else if(fntype == DIALOG_)
 {
    n = iParseExp ();     RETURN_IF_ERRO(iparse_error);
    n = IupDialog (n);
 }
 else if(fntype == FILL_)
 {
   n = IupFill ();
 }
 else if(fntype == FRAME_)
 {
   n = iParseExp ();     RETURN_IF_ERRO(iparse_error);
   n = IupFrame (n);
 }
 else if(fntype == HBOX_)
 {
    Ihandle *box = iupTreeCreateNode (NULL);
    int match;
    do
    {
     Ihandle *aux=iParseExp();     RETURN_IF_ERRO(iparse_error);
     box = iupTreeAppendNode (box, aux);
     match=iupLexSeenMatch (TK_COMMA,&iparse_error); RETURN_IF_ERRO(iparse_error);
    } while (match);
    n = iupMkHbox (box);
 }
 else if(fntype == ITEM_)
 {
    char* label;
    char* action;
    RETURN_IF_ERRO(iupLexMatch (TK_STR)); label = iupLexGetName();
    RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); action = iupLexGetName();
    n = IupItem (label, action);
 }
 else if(fntype == KEYACTION_)
 {
    char* k;
    char* action;
    Itable* list=iupTableCreate(IUPTABLE_STRINGINDEXED);
    int match;
    do
    {
     RETURN_IF_ERRO(iupLexMatch (TK_STR)); k = iupLexGetName();
     RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
     RETURN_IF_ERRO(iupLexMatch (TK_NAME)); action = iupLexGetName();
     iupTableSet(list, k, action, IUP_POINTER);
     match=iupLexSeenMatch (TK_COMMA,&iparse_error); RETURN_IF_ERRO(iparse_error);
    } while (match);
    n = iupMkKeyAction (list);
    iupTableDestroy(list); /* untested */
 }
 else if(fntype == LABEL_)
 {
    char* label;
    RETURN_IF_ERRO(iupLexMatch (TK_STR)); label = iupLexGetName();
    n = IupLabel (label);
 }
 else if(fntype == LIST_)
 {
    char* action;
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); action = iupLexGetName();
    n = IupList (action);
 }
 else if(fntype == MENU_)
 {
    Ihandle *menu = iupTreeCreateNode (NULL);
    int match;
    do
    {
     Ihandle *aux=iParseExp();    RETURN_IF_ERRO(iparse_error);
     menu=iupTreeAppendNode (menu, aux);
     match=iupLexSeenMatch (TK_COMMA,&iparse_error); RETURN_IF_ERRO(iparse_error);
    } while (match);
    n = iupMkMenu (menu);
 }
 else if(fntype == RADIO_)
 {
   n = iParseExp ();     RETURN_IF_ERRO(iparse_error);
   n = IupRadio (n);
 }
 else if(fntype == SUBMENU_)
 {
    char* label;
    Ihandle *aux;
    RETURN_IF_ERRO(iupLexMatch (TK_STR)); label = iupLexGetName();
    RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
    aux=iParseExp ();     RETURN_IF_ERRO(iparse_error);
    n = IupSubmenu (label, aux);
 }
 else if(fntype == MULTILINE_ || fntype == TEXT_)
 {
    char* action;
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); action = iupLexGetName();
    if (fntype == TEXT_)
     n = IupText (action);
    else
     n = IupMultiLine(action);
 }
 else if(fntype == TOGGLE_)
 {
    char* label;
    char* action;
    RETURN_IF_ERRO(iupLexMatch (TK_STR)); label = iupLexGetName();
    RETURN_IF_ERRO(iupLexMatch (TK_COMMA));
    RETURN_IF_ERRO(iupLexMatch (TK_NAME)); action = iupLexGetName();
    n = IupToggle (label, action);
 }
 else if(fntype == SEPARATOR_)
 {
   n = IupSeparator ();
 }
 else if(fntype == VBOX_)
 {
    Ihandle *box = iupTreeCreateNode(NULL);
    int match;
    do
    {
     Ihandle *aux=iParseExp ();     RETURN_IF_ERRO(iparse_error);
     box = iupTreeAppendNode (box, aux);
     match=iupLexSeenMatch (TK_COMMA,&iparse_error); RETURN_IF_ERRO(iparse_error);
    } while (match);
    n = iupMkVbox (box);
 }
 else if(fntype == ZBOX_)
 {
    Ihandle *box = iupTreeCreateNode(NULL);
    int match;
    do
    {
     Ihandle *aux=iParseExp ();     RETURN_IF_ERRO(iparse_error);
     box = iupTreeAppendNode (box, aux);
     match=iupLexSeenMatch (TK_COMMA,&iparse_error); RETURN_IF_ERRO(iparse_error);
    } while (match);
    n = iupMkZbox (box);
 }
 else if(fntype == USER_)
 {
   n = IupUser();
 }
 else if(fntype == CONTROL_)
 {
    n = parsecontrol(ic);
    RETURN_IF_ERRO(iparse_error);
 }
 else
 {
   assert(0); /* inexistant control... */
 }

 if (attr)
 {
  IupSetAttributes(n,attr);
  free(attr);
  attr=NULL;
 }
 RETURN_IF_ERRO(iupLexMatch (TK_ENDP));
 return n;
}

#ifdef WIN32
#include <windows.h>
#endif

static int iParseError (int n, char *s)
{
 static char msg[256];
 switch (n)
 {
  case SYMBEXIST:
   sprintf (msg, "symbol '%s' %s",s,"already exists");
   break;
  case NOTMATCH:
   sprintf (msg, "symbol '%s' %s",s,"not defined");
   break;
 }
 return iupLexError(PARSEERROR,msg);
}

