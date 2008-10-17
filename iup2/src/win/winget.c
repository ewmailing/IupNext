/** \file
 * \brief Windows Driver get for attributes
 *
 * See Copyright Notice in iup.h
 * $Id: winget.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <stdio.h>
#include <assert.h>

#include <windows.h>

#include "iglobal.h"
#include "idrv.h"
#include "win.h"
#include "winhandle.h"
#include "winproc.h"

static char* getActive(Ihandle *n);
static char* getSize(Ihandle *n);
static char* getValue(Ihandle *n);
static char* getVisible(Ihandle *n);
static char* getWid(Ihandle *n);
static char* getFgcolor(Ihandle *n);
static char* getCaret(Ihandle *n);
static char* getConid(Ihandle* n);
static char* getDrawSize(Ihandle* n);
static char* getFont(Ihandle* h);
static char* getDefBgcolor(Ihandle *n);
static char* getX(Ihandle *n);
static char* getY(Ihandle *n);
static char* getSelectedText(Ihandle *n);
static char* getdefExpand( Ihandle* h);
static char* getSelection( Ihandle* n);
static char* getRastersize(Ihandle *n);
static char* getFullscreen(Ihandle *n);
static char* getmdiactive(Ihandle *n);
static char* getmdinext(Ihandle *n);

typedef char*(*get_fn)(Ihandle*);

static struct {
   char *name;
   get_fn get;
   char *def;
   get_fn getdef;
} attributes[] = {
	 {IUP_SIZE,        getSize,         NULL,    NULL},
   {IUP_VALUE,       getValue,        NULL,    NULL},
   {IUP_VISIBLE,     getVisible,      NULL,    NULL}, /* New objs are alws vsble */
   {IUP_ACTIVE,      getActive,       IUP_YES, NULL},
   {IUP_POSY,        NULL,            "0.0",   NULL},
   {IUP_POSX,        NULL,            "0.0",   NULL},
   {IUP_MULTIPLE,    NULL,            IUP_NO,  NULL},
   {IUP_CONID,       getConid,        NULL,    NULL},
   {"HWND",          getWid,          NULL,    NULL},
   {"DRAWSIZE",      getDrawSize,     NULL,    NULL},
   {IUP_IMAGE,       NULL,            NULL,    NULL},
   {IUP_IMINACTIVE,  NULL,            NULL,    NULL},
   {IUP_IMPRESS,     NULL,            NULL,    NULL},
   {IUP_BGCOLOR,     NULL,            NULL,    getDefBgcolor},
   {IUP_FGCOLOR,     getFgcolor,      NULL,    NULL},
   {IUP_CURSOR,      NULL,            "ARROW", NULL},
   {IUP_TITLE,       NULL,            NULL,    NULL},
   {IUP_NC,          NULL,            "32767", NULL},
   {IUP_ICON,        NULL,            NULL,    NULL},
   {IUP_KEY,         NULL,            NULL,    NULL},
   {IUP_MENU,        NULL,            NULL,    NULL},
   {IUP_VERTICAL,    NULL,            IUP_NO,  NULL},
   {IUP_MIN,         NULL,            "0.0",   NULL},
   {IUP_MAX,         NULL,            "1.0",   NULL},
   {IUP_BORDER,      NULL,            IUP_YES, NULL},
   {IUP_XMIN,        NULL,            "0.0",   NULL},
   {IUP_XMAX,        NULL,            "1.0",   NULL},
   {IUP_YMAX,        NULL,            "1.0",   NULL},
   {IUP_YMIN,        NULL,            "0.0",   NULL},
   {IUP_DY,          NULL,            "0.1",   NULL},
   {IUP_DX,          NULL,            "0.1",   NULL},
   {IUP_WID,         getWid,          NULL,    NULL},
   {IUP_SCROLLBAR,   NULL,            IUP_NO,  NULL},
   {IUP_CARET,       getCaret,        NULL,    NULL},
   {IUP_EXPAND,      NULL,            NULL,    getdefExpand},
   {IUP_FONT,        getFont,         NULL,    NULL},
   {"_IUPWIN_FONT",  getFont,         NULL,    NULL},
   {IUP_X,           getX,            NULL,    NULL},
   {IUP_Y,           getY,            NULL,    NULL},
   {IUP_SELECTEDTEXT,getSelectedText, NULL,    NULL},
   {IUP_SELECTION,   getSelection,    NULL,    NULL},
   {IUP_RASTERSIZE,  getRastersize,   NULL,    NULL},
   {IUP_FULLSCREEN,  getFullscreen,   NULL,    NULL},
   {"MDIACTIVE",     getmdiactive,    NULL,    NULL},
   {"MDINEXT",       getmdinext,      NULL,    NULL},
};

/* Funcoes auxiliares */

/*
 * Remove LineFeeds de uma string 
 */

static void removeLineFeeds(char *string, int maxsize)
{
  int current = 0;
  int next = 0;
  
  while ( string[next] != '\0' && next < maxsize)
  {
    if (string[next] == '\r')
    {
      next++;
      continue;
    }

    string[current] = string[next];
    next++;
    current++;
  }
  
  assert( string[next] == '\0');
  string[current] = '\0';
}

#define NATTRIBUTES (sizeof(attributes)/sizeof(attributes[0]))

char *iupdrvGetAttribute (Ihandle *n, const char *a)
{
   int i;
 
   if (n == NULL || a == NULL)
      return NULL;
 
   for (i=0; i<NATTRIBUTES; ++i)
   {
      if (iupStrEqualNoCase (a, attributes[i].name))
         break;
   }
 
   if (i<NATTRIBUTES && (attributes[i].get != NULL))
      return (*attributes[i].get)(n);

   return NULL;
}

char *iupdrvGetDefault (Ihandle *n, const char *a)
{
	int i;

	if (n == NULL || a == NULL)
		return NULL;

  for (i=0; i<NATTRIBUTES; ++i)
  {
    if (iupStrEqualNoCase (a, attributes[i].name))
    {
      if (attributes[i].getdef != NULL)
        return attributes[i].getdef(n);
      else
        return attributes[i].def;
    }
  }

	return NULL;
}

static char* getdefExpand( Ihandle* h)
{
  if(type(h) == CANVAS_ || type(h) == FILL_ ||
     type(h) == FRAME_  || type(h) == HBOX_ ||
     type(h) == RADIO_  || type(h) == VBOX_ ||
     type(h) == ZBOX_ )
  {
    return IUP_YES;
  }
  else
  {
    return IUP_NO;
  }
}

static char* getVisible (Ihandle* n)
{
 char *env = NULL;

 if (handle(n) == NULL)
	 return NULL;

 if ( type(n)==BUTTON_    || type(n)==CANVAS_ || type(n)==TOGGLE_ ||
      type(n)==FRAME_     || type(n)==LABEL_  || type(n)==LIST_   ||
      type(n)==MULTILINE_ || type(n)==TEXT_   || type(n)==DIALOG_
    )
 {
	 if (IsWindowVisible((HWND)handle(n)))
		 return IUP_YES;
	 else
		 return IUP_NO;
 }

 /* If it is not a native element, lets check if the visibility is 
    in the environment. If it is NULL then return YES (after all it
    is mapped). */
 env = iupGetEnv(n, IUP_VISIBLE);
 if(env)
   return env;
 else
   return IUP_YES;
}

static Ihandle* getradiovalue(Ihandle* n)
{
	Ihandle* aux = NULL;
	Ihandle* ret = NULL;

	if(type(n) == FRAME_ || type(n) == VBOX_ || type(n) == HBOX_ || type(n) == ZBOX_)
  {
    foreachchild(aux,n)
				  if((ret = getradiovalue(aux)) != NULL) return ret;
  }
	else if(type(n) == TOGGLE_)
	{
    if (SendMessage((HWND)handle(n), BM_GETCHECK, 0, 0L))
		  return n;
	}
	return ret;
}

static char* getValue (Ihandle* n)
{
  char *value=NULL;
  if ( handle(n) == NULL && type(n)!=RADIO_ && type(n)!=ZBOX_ )
    return NULL;

  if(type(n) == MULTILINE_ || type(n) == TEXT_)
  {
    int nc = 0; 

    if(iupGetEnv(n, "_IUPWIN_IGNORE_EDITTEXT") != NULL)
      return NULL;

    nc = GetWindowTextLength ((HWND)handle(n));

	  if(nc)
	  {
		  value = iupStrGetMemory(nc+1);
		  if (value)
      {
			  GetWindowText ((HWND)handle(n), value, nc+1);
        removeLineFeeds(value, nc);
      }
	  }
	  else
		  value = "";
  }
  else if(type(n) == TOGGLE_)
  {
    int check = (int)SendMessage((HWND)handle(n), BM_GETCHECK, 0, 0L);
    if (check == BST_INDETERMINATE)
      return "NOTDEF";
    else if (check == BST_CHECKED)
      return "ON";
    else
      return "OFF";
  }                                              
  else if(type(n) == ZBOX_)
  {
	  value = iupTableGet (env(n), IUP_VALUE);
	  if (!value)
	  {
		  Ihandle* c = child(n);
		  if (c)
			  value = IupGetName(c);
	  }
  }
  else if(type(n) == RADIO_)
  {
	  Ihandle* c = NULL;
	  Ihandle* aux = NULL;
	  foreachchild(aux,n)
	  {
		  c = getradiovalue(aux);
		  if (c)
		  {
			  value = IupGetName( c );
			  break;
		  }
	  }
  }
  else if(type(n) == ITEM_)
  {
    if (GetMenuState((HMENU)handle(n), number(n), MF_BYCOMMAND) == MF_CHECKED)
      return IUP_ON;
    else
      return IUP_OFF;
  }
  else if(type(n) == LIST_)
  {
    if (iupCheck(n, IUP_MULTIPLE)==YES)
    {
      LRESULT no = SendMessage ((HWND)handle(n), LB_GETCOUNT, 0, 0L);
	    value = iupStrGetMemory(no+1);
	    if (value)
	    {
		    WPARAM i;
		    for (i=0; i<(WPARAM)no; ++i)
			    value[i] = (SendMessage ((HWND)handle(n), LB_GETSEL, i, 0L) ? '+' : '-');
		    value[(unsigned)no]='\0';
	    }
    }
    else
    {
      int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
      if (haseditbox)
      {
        int nc;
        if (iupGetEnv(n, "_IUPWIN_IGNORE_EDITTEXT") != NULL)
          return NULL;

        nc = GetWindowTextLength ((HWND)handle(n));
	      if(nc)
	      {
		      value = iupStrGetMemory(nc+1);
		      if (value)
          {
			      GetWindowText ((HWND)handle(n), value, nc+1);
            removeLineFeeds(value, nc);
          }
	      }
	      else
		      value = "";
      }
      else
      {
        UINT getcursel = LB_GETCURSEL;
        if (iupCheck(n, IUP_DROPDOWN)==YES)
          getcursel = CB_GETCURSEL;

	      value = iupStrGetMemory(6);
	      if (value)
		      sprintf (value, "%d",(int)(1+SendMessage((HWND)handle(n), getcursel, 0, 0L)));
      }
    }
  }
  else
    value = NULL;

  return value;
}

static char* getActive(Ihandle *n)
{
   if (handle(n) == NULL)
      return NULL;

   if (type(n) == TOGGLE_)
   {
     if(IupGetAttribute(n, IUP_IMAGE) == NULL || iupwinUseComCtl32Ver6())
     {
      if (IsWindowEnabled((HWND)handle(n)))
         return IUP_YES;
      else
         return IUP_NO;
     }
     else
       return iupGetEnv(n, "_IUPWIN_TOGGLE_ACTIVE");
   }

   if ( (type(n) == TEXT_) || (type(n) == BUTTON_) ||
        (type(n) == MULTILINE_) || (type(n) == LIST_) || 
        (type(n) == CANVAS_))
   {
      if (IsWindowEnabled((HWND)handle(n)))
         return IUP_YES;
      else
         return IUP_NO;
   }
   else
      return NULL;
}

static int retrieveSize(Ihandle *n, int *w, int *h)
{
  if (type(n)==BUTTON_    || type(n)==CANVAS_ || type(n)==TOGGLE_ ||
      type(n)==FRAME_     || type(n)==LABEL_  || type(n)==LIST_   ||
      type(n)==MULTILINE_ || type(n)==TEXT_   || type(n)==DIALOG_
    )
  {
    if (handle(n))
    {
      if (iupGetEnv(n,"_IUPWIN_IS_MAPPING") != NULL)
        return 0;
      else
      {
        RECT rc;

        if ( type(n)==DIALOG_ )
          GetClientRect( (HWND)handle(n), &rc );
        else
          GetWindowRect( (HWND)handle(n), &rc );

        *w = rc.right - rc.left;
        *h = rc.bottom - rc.top;

        return 1;
      }
    }
  }
  else
    return 0;

  return 0;
}

static char* getRastersize(Ihandle *n)
{
  int width = -1, height = -1;

  assert(n != NULL);
  if(n == NULL)
    return NULL;

  if(retrieveSize(n, &width, &height) == 1)
  {
    char *size = iupStrGetMemory(20);
    sprintf(size, "%dx%d", width, height);
    return size;
  }
  else
    return NULL;
}

static char* getSize(Ihandle *n)
{
  int width = -1, height = -1;

  assert(n != NULL);
  if(n == NULL)
    return NULL;

  if(retrieveSize(n, &width, &height) == 1)
  {
    int charwidth = -1, charheight = -1;
    char *size = iupStrGetMemory(20);

    iupdrvGetCharSize(n, &charwidth, &charheight);
    sprintf(size, "%dx%d", width*4/charwidth, height*8/charheight);

    return size;
  }
  else
    return NULL;
}

static char* getWid(Ihandle *n)
{
   return (char*) handle(n);
}

static char* iupwinGetSysColor(int syscolor)
{
   COLORREF color;
   unsigned char r, g, b;
   static char val[15];

   color = GetSysColor(syscolor);
   r = GetRValue(color);
   g = GetGValue(color);
   b = GetBValue(color);
   sprintf(val,"%d %d %d", (int)r, (int)g, (int)b);
   return val;
}

static char* getFgcolor(Ihandle *n)
{
   char* color = iupGetEnv(n,IUP_FGCOLOR);
   if (color)
      return color;

   if (type(n)==BUTTON_)
      return iupwinGetSysColor(COLOR_BTNTEXT);
   else
      return iupwinGetSysColor(COLOR_WINDOWTEXT);
}

static char* getDefBgcolor(Ihandle *n)
{
  char *color=NULL;

  assert(n != NULL);
  if(n == NULL)
    return NULL;
   
  if(type(n) == BUTTON_ || type(n) == IMAGE_)
    color = iupwinGetSysColor(COLOR_BTNFACE);
  else if(type(n) == DIALOG_ || type(n) == FRAME_ || type(n) == LABEL_ || type(n) == TOGGLE_)
    color = iupwinGetSysColor(atoi(IupGetGlobal("_IUPWIN_DLGBGCOLOR")));
  else if(type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_ || type(n) == CANVAS_)
    color = iupwinGetSysColor(COLOR_WINDOW);
  else if(type(n) == ITEM_)
    color = iupwinGetSysColor(COLOR_MENU);
  else
    color = NULL;

  return color;
}

static char* getCaret(Ihandle *n)
{
  if (handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_) )
  {
    int col = 0, pos = 0, row = -1;
    char* str;
    POINT point;

    if (iupGetEnv(n, "_IUPWIN_IGNORE_EDITTEXT") != NULL)
      return NULL;

    str = iupStrGetMemory(50);

    if (GetFocus() != (HWND)handle(n) || !GetCaretPos(&point))
    {
      SendMessage((HWND)handle(n), EM_GETSEL, (WPARAM)&col, 0);
    }
    else
    {
      pos = SendMessage((HWND)handle(n), EM_CHARFROMPOS, 0, MAKELPARAM(point.x, point.y));
      col = LOWORD(pos);
      row = HIWORD(pos);
    }

    if (type(n) == TEXT_ || type(n) == LIST_)
    {
      col++;  /* IUP starts at 1 */
      sprintf(str, "%i", col);
    }
    else
    {
      if (row == -1)
        row = SendMessage((HWND)handle(n), EM_LINEFROMCHAR, (WPARAM)col, (LPARAM)0L);

      pos = SendMessage((HWND)handle(n), EM_LINEINDEX, (WPARAM)row, (LPARAM)0L);
      col -= pos;

      row++;  /* IUP starts at 1 */
      col++;
      sprintf(str, "%i,%i", row, col);
    }

    return str;
  }
  return NULL;
}

static int retrieveDrawSize(Ihandle *n, int *w, int *h)
{
  if (type(n)==CANVAS_)
  {
    if (handle(n))
    {
      if (iupGetEnv(n,"_IUPWIN_IS_MAPPING") != NULL)
        return 0;
      else
      {
        RECT rc;

        GetClientRect( (HWND)handle(n), &rc );

        *w = rc.right - rc.left;
        *h = rc.bottom - rc.top;

        return 1;
      }
    }
  }
  else
    return 0;

  return 0;
}

static char* getDrawSize( Ihandle* n)
{
  int width = -1, height = -1;

  assert(n != NULL);
  if(n == NULL)
    return NULL;

  if(retrieveDrawSize(n, &width, &height) == 1)
  {
    char *size = iupStrGetMemory(20);
    sprintf(size, "%dx%d", width, height);
    return size;
  }
  else
    return NULL;
}

static char* getConid( Ihandle* h)
{
   char* conid = iupStrGetMemory(20);
   sprintf (conid, "%p", handle(h));
   return conid;
}

/*
 * Retorna descricao da fonte corrente no elemento de
 * interface
 */

static char* getFont(Ihandle* h)
{
  char *attr=NULL;

  /* Checagem de parametros */
  assert(h != NULL);
  if(h == NULL)
    return NULL;

  attr = iupGetEnv(h, "_IUPWIN_FONT");

  return attr;
}

static void getwindowpos(Ihandle *h, int *x, int *y)
{
  RECT rect;
  BOOL ret = GetWindowRect(handle(h), &rect);
  assert(ret != 0);
  if(ret == 0)
  {
    *x = -1;
    *y = -1;
    return;
  }
  *x = rect.left;
  *y = rect.top;
}

static char *getX(Ihandle *h)
{
  static char size[6];
  int x, y;
  if(handle(h) == NULL)
    return NULL;
  getwindowpos(h, &x, &y);
  if(x == -1) return NULL;
  sprintf(size, "%d", x);
  return size;
}

static char *getY(Ihandle *h)
{
  static char size[6];
  int x, y;
  if(handle(h) == NULL)
    return NULL;
  getwindowpos(h, &x, &y);
  if(y == -1) return NULL;
  sprintf(size, "%d", y);
  return size;
}

static char *getSelectedText(Ihandle *n)
{
  char *value = NULL;
  int start = 0, end = 0;
  int nc = 0;

  assert(n);
  if(n == NULL)
    return NULL;

  if(handle(n) == NULL)
    return NULL;

  if(type(n) == MULTILINE_ || type(n) == TEXT_ || type(n) == LIST_)
  {
    nc = GetWindowTextLength (handle(n));

    value = iupStrGetMemory(nc);
    
    assert(value);
    if(value == NULL)
      return NULL;

    GetWindowText(handle(n), value, nc+1);
    
    /* Pega posicao onde comeca e termina texto selecionado */
    if (type(n) == LIST_)
      SendMessage(handle(n), CB_GETEDITSEL, 
        (WPARAM) (LPDWORD) &start, (LPARAM) (LPDWORD)&end);
    else
      SendMessage(handle(n), EM_GETSEL, 
        (WPARAM) (LPDWORD) &start, (LPARAM) (LPDWORD)&end);

    value[end] = '\0';
    value = value + start;

    removeLineFeeds(value, nc);
  }

  return value;
}


/**************************************************************************
** Converte uma posicao de caracter em numero de linha e numero de coluna
** do multiline. A posicao é recebida comecando em 0 e o numero da coluna
** e da linha retornados comecam em 1 (no estilo IUP).
**************************************************************************/
static void pos2rowcol(Ihandle* n, int pos, int* row, int* col)
{
  int line=1, a;
  assert (pos>=0);
  if(pos<0) return;
  assert (n);
  if(n == NULL) return;

  while(1)
  {
    a = SendMessage((HWND)handle(n), EM_LINEINDEX, (WPARAM)line-1, 0L);
    if (a<=pos && a != -1)
      line++;
    else if(a==-1)
    {
      if(line != 1)
        a = SendMessage((HWND)handle(n), EM_LINEINDEX, (WPARAM)line-2, 0L);
      else
      {
        a = 0;
        line = 1;
      }
      break;
    }
    else
    {
      /* Numero de caracteres até o inicio da linha */
      a = SendMessage((HWND)handle(n), EM_LINEINDEX, (WPARAM)line-2, 0L);
      break;
    }
  }
  *row = line-1;
  *col = pos - a + 1;
}

/**************************************************************************
** Retorna uma string no formato IUP contendo a posicao inicial e
** final da selecao do elemento multiline e text. 
** Limite: a string contem no maximo 100 caracteres.
**************************************************************************/
static char* getSelection( Ihandle* n)
{
  assert(n);
  if(n==NULL) 
    return NULL;

  if (handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_) )
  {
    if(iupGetEnv(n, "_IUPWIN_IGNORE_EDITTEXT") != NULL)
        return NULL;

    if(type(n) == TEXT_ || type(n) == LIST_)
    {
      int start=1, end=1;          
      char *v = NULL;

      if (type(n) == LIST_)
        SendMessage(handle(n), CB_GETEDITSEL, 
          (WPARAM) (LPDWORD) &start, (LPARAM) (LPDWORD)&end);
      else
        SendMessage(handle(n), EM_GETSEL, 
          (WPARAM) (LPDWORD) &start, (LPARAM) (LPDWORD)&end);

      assert(start >= 0 || end >= 0);
      if(start < 0 || end < 0) 
        return NULL;

      start++; /* Posicao inicial no Iup e' 1 */
      end++;
      v = iupStrGetMemory(100);
      sprintf(v,"%i:%i", start, end);
      return v;
    }
    else if(type(n) == MULTILINE_)
    {
      int  start=1,     end=1;
      int  start_col=1, start_line=1, end_col=1, end_line=1;
      char *v=NULL;

      SendMessage(handle(n), EM_GETSEL,
        (WPARAM) (LPDWORD) &start, (LPARAM) (LPDWORD)&end);
      assert(start >= 0 || end >= 0);
      if(start < 0 || end < 0) 
        return NULL;

      pos2rowcol(n, start, &start_line, &start_col);
      pos2rowcol(n, end,   &end_line,   &end_col);
      v = iupStrGetMemory(100);
      sprintf(v,"%i,%i:%i,%i", start_line, start_col, end_line, end_col);
      return v;
    }
  }
  return NULL;
}

static char* getFullscreen(Ihandle* n)
{
  if(!iupGetEnv(n, "_IUPWIN_FS_STYLE"))
    return IUP_NO;
  else
    return IUP_YES;
}

static HWND iupwin_mdifirst = NULL;
static HWND iupwin_mdinext = NULL;

static char* getmdiactive  (Ihandle *n)
{
  Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
  if (client)
  {
    HWND hchild = (HWND)SendMessage((HWND)handle(client), WM_MDIGETACTIVE, 0, 0);
    Ihandle* child = iupwinHandleGet(hchild); 
    if (child)
    {
      iupwin_mdinext = NULL;
      iupwin_mdifirst = hchild;
      return IupGetName(child);
    }
  }

  iupwin_mdifirst = NULL;
  iupwin_mdinext = NULL;
  return NULL;
}

static char* getmdinext(Ihandle *n)
{
  Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
  if (client)
  {
    Ihandle* child;
    HWND hchild = iupwin_mdinext? iupwin_mdinext: iupwin_mdifirst;

	  /* Skip the icon title windows */
	  while (hchild && GetWindow (hchild, GW_OWNER))
	    hchild = GetWindow(hchild, GW_HWNDNEXT);

	  if (!hchild || hchild == iupwin_mdifirst)
    {
      iupwin_mdinext = NULL;
      return NULL;
    }

    child = iupwinHandleGet(hchild); 
    if (child)
    {
      iupwin_mdinext = hchild;
      return IupGetName(child);
    }
  }

  iupwin_mdinext = NULL;
  return NULL;
}
