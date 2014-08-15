/** \file
 * \brief Windows Driver Control Element Size Get
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>              /* NULL */
#include <string.h>             /* strlen */
#include <assert.h>    

#include <windows.h>

#include "iglobal.h"
#include "idrv.h"
#include "win.h"

/************************************************************************
 *
 *  Funcoes do driver para consulta 
 *  dos tamanhos dos elementos de interface
 *
 ************************************************************************/

/* used by Label and Multiline Only */
void iupdrvGetTextLineBreakSize(Ihandle* n, const char* text, int *w, int *h)
{
  const char *curstr; 
  const char *nextstr;
	TEXTMETRIC tm;
  int len, num_lin;
  SIZE size;
	HDC hdc = GetDC((HWND)handle(n));
  HFONT hfont, oldhfont = NULL;
  
  hfont = (HFONT)IupGetAttribute(n, "HFONT");

  assert(hfont);
  if (hfont != NULL)
    oldhfont = SelectObject(hdc, hfont);

  *w = 0;
  curstr = text;
  num_lin = 0;
  do
  {
    nextstr = iupStrNextLine(curstr, &len);
    GetTextExtentPoint32(hdc, curstr, len, &size);
    *w = MAX(*w, size.cx);

    curstr = nextstr;
    num_lin++;
  } while(*nextstr);

	GetTextMetrics(hdc, &tm);
	*h = tm.tmHeight * num_lin;

  if (oldhfont) SelectObject(hdc, oldhfont);
	ReleaseDC((HWND)handle(n),hdc);
}

void iupdrvStringSize(Ihandle *n, const char*s, int *w, int *h)
{
  HDC hDC = GetDC((HWND)handle(n));
  HFONT hf = (HFONT)IupGetAttribute(n, "HFONT");

  assert(hf);
  if (hf != NULL)
    SelectObject(hDC, hf);

  if (w) 
  {
    SIZE size;
    GetTextExtentPoint32(hDC, s, strlen(s), &size);
    *w = size.cx;
  }

  if (h)
  {
  	TEXTMETRIC tm;
    GetTextMetrics(hDC, &tm);
    *h = tm.tmHeight;
  }

  ReleaseDC((HWND)handle(n),hDC);
}

void iupdrvGetCharSize (Ihandle* n, int *w, int *h)
{
	TEXTMETRIC tm;
	HDC hdc = GetDC((HWND)handle(n));
  HFONT font, oldfont = NULL;
  
  font = (HFONT)IupGetAttribute(n, "HFONT");

  assert(font);
  if (font != NULL)
    oldfont = SelectObject(hdc, font);

	GetTextMetrics(hdc, &tm);

  if (oldfont) SelectObject(hdc, oldfont);
	ReleaseDC((HWND)handle(n),hdc);
	
	if (w) *w = tm.tmAveCharWidth;
	if (h) *h = tm.tmHeight;
}

int iupwinDialogDecorX(Ihandle* n)
{
   int decor = 0;
   int no_titlebar = 0;

   assert(n);
   if(n == NULL)
     return 0;

   no_titlebar = !iupCheck(n, IUP_MAXBOX)  && 
              !iupCheck(n, IUP_MINBOX)  &&
              !iupCheck(n, IUP_MENUBOX) && 
              !IupGetAttribute(n,IUP_TITLE);

   if (iupCheck(n,IUP_RESIZE))
      decor = 2*GetSystemMetrics(SM_CXFRAME);
   else if(!no_titlebar)
     decor = 2*GetSystemMetrics(SM_CXFIXEDFRAME);
   else if (iupCheck(n,IUP_BORDER))
      decor = 2*GetSystemMetrics(SM_CXBORDER);
   					 
   return decor;
}

int iupwinDialogDecorLeft(Ihandle* n)
{
   int decor = 0;
   int no_titlebar = 0;

   assert(n);
   if(n == NULL)
     return 0;

   no_titlebar = !iupCheck(n, IUP_MAXBOX)  && 
              !iupCheck(n, IUP_MINBOX)  &&
              !iupCheck(n, IUP_MENUBOX) && 
              !IupGetAttribute(n,IUP_TITLE);

   if (iupCheck(n,IUP_RESIZE))
      decor = GetSystemMetrics(SM_CXFRAME);
   else if(!no_titlebar)
     decor = GetSystemMetrics(SM_CXFIXEDFRAME);
   else if (iupCheck(n,IUP_BORDER))
      decor = GetSystemMetrics(SM_CXBORDER);
   					 
   return decor;
}

int iupwinDialogDecorTop(Ihandle* n)
{
   int decor = 0;
   int no_titlebar = 0;

   assert(n);
   if(n == NULL)
     return 0;

   no_titlebar = !iupCheck(n, IUP_MAXBOX)  && 
                 !iupCheck(n, IUP_MINBOX)  &&
                 !iupCheck(n, IUP_MENUBOX) && 
                 !IupGetAttribute(n,IUP_TITLE);

   if (!no_titlebar)
   {
     if(iupCheck(n, IUP_TOOLBOX) == YES &&
        IupGetAttribute(n, IUP_PARENTDIALOG) != NULL)
       decor += GetSystemMetrics(SM_CYSMCAPTION); /* tool window */
     else
       decor += GetSystemMetrics(SM_CYCAPTION); /* janela normal */
   }

   if (iupCheck(n,IUP_RESIZE))
     decor += GetSystemMetrics(SM_CYFRAME);
   else if (!no_titlebar)
     decor += GetSystemMetrics(SM_CYFIXEDFRAME);
   else if (iupCheck(n, IUP_BORDER))
     decor += GetSystemMetrics(SM_CYBORDER);

   if (IupGetAttribute (n,IUP_MENU) != NULL)
      decor += GetSystemMetrics(SM_CYMENU);
		 
   return decor;
}

int iupwinDialogDecorY(Ihandle* n)
{
   int decor = 0;

   int no_titlebar = 0;

   assert(n);
   if(n == NULL)
     return 0;

   no_titlebar = !iupCheck(n, IUP_MAXBOX)  && 
                 !iupCheck(n, IUP_MINBOX)  &&
                 !iupCheck(n, IUP_MENUBOX) && 
                 !IupGetAttribute(n,IUP_TITLE);

   if (!no_titlebar)
   {
     if(iupCheck(n, IUP_TOOLBOX) == YES &&
        IupGetAttribute(n, IUP_PARENTDIALOG) != NULL)
       decor += GetSystemMetrics(SM_CYSMCAPTION); /* tool window */
     else
       decor += GetSystemMetrics(SM_CYCAPTION); /* janela normal */
   }
   
   if (iupCheck(n,IUP_RESIZE))
     decor += 2*GetSystemMetrics(SM_CYFRAME);
   else if (!no_titlebar)
     decor += 2*GetSystemMetrics(SM_CYFIXEDFRAME);
   else if (iupCheck(n, IUP_BORDER))
     decor += 2*GetSystemMetrics(SM_CYBORDER);

   if (IupGetAttribute (n,IUP_MENU) != NULL)
      decor += GetSystemMetrics(SM_CYMENU);
			 
   return decor;
}

int iupdrvWindowSizeX (Ihandle *n, int scale)
{
   int decor = iupwinDialogDecorX(n);
   RECT rc;
		  
   GetWindowRect( GetDesktopWindow(), &rc );

   return (rc.right-rc.left+1)/scale - decor;
}

int iupdrvWindowSizeY (Ihandle *n, int scale)
{
   int decor = iupwinDialogDecorY(n);
   RECT rc;
		  
   GetWindowRect( GetDesktopWindow(), &rc );

   return (rc.bottom-rc.top+1)/scale - decor;
}

int iupdrvGetBorderSize(Ihandle* n)
{
  int tb = iupwinThemeBorder((HWND)handle(n));
  if (tb) return tb+1;
  else return 2+1;     /* border+focus area */
}

int iupdrvGetScrollSize(void) 
{ 
  int xv = GetSystemMetrics(SM_CXVSCROLL);
  int yh = GetSystemMetrics(SM_CYHSCROLL);
  return xv>yh? xv: yh;
}

