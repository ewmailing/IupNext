/** \file
 * \brief Motif Driver Core and Initialization
 *
 * See Copyright Notice in iup.h
 *  */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <stdio.h>		/* scanf */
#include <string.h>		/* strlen */
#include <assert.h>		/* strlen */
#include <Xm/Xm.h>

#include "iglobal.h"
#include "ifiledlg.h"
#include "itimer.h"
#include "idrv.h"
#include "motif.h"

/* FIXME: must be obtained from the window manager */
#define MOTIF_BORDER_SIZE 0
#define MOTIF_TITLE_SIZE 0

void iupdrvGetTextLineBreakSize(Ihandle* n, const char* text, int *w, int *h)
{
  const char *curstr; 
  const char *nextstr;
  int len, num_lin, lw;
  XFontStruct *fs = iupmotGetFontStruct(n);
  
  *w = 0;
  curstr = text;
  num_lin = 0;
  do
  {
    nextstr = iupStrNextLine(curstr, &len);
    lw = XTextWidth (fs, curstr, len);
    *w = MAX(*w, lw);

    curstr = nextstr;
    num_lin++;
  } while(*nextstr);

	*h = (fs->ascent + fs->descent) * num_lin;
}

void iupdrvStringSize(Ihandle *n, const char*s, int *w, int *h)
{
   XFontStruct *fs = iupmotGetFontStruct(n);
   if (w) *w = XTextWidth (fs, s, strlen(s));
   if (h) *h = fs->ascent + fs->descent;
}

void iupdrvGetCharSize (Ihandle *n, int *w, int *h)
{
   XFontStruct *fs = iupmotGetFontStruct(n);
   if (h) *h = fs->ascent + fs->descent;
   if (w)
   {
      if (fs->per_char)
      {
         int i, all=0;
         int first = fs->min_char_or_byte2;
         int last  = fs->max_char_or_byte2;
         if (first < 'A')  first = 'A';
         if (last  > 'Z') last  = 'Z';
         for (i=first; i<=last; i++)
            all += fs->per_char[i].width;
         *w = all/(last-first) + 1;
      }
      else
      {
         *w = fs->max_bounds.width;
      }
   }
}

int iupdrvWindowSizeX (Ihandle *n, int scale)
{
   int sw = DisplayWidth(iupmot_display, iupmot_screen);
   int decorx=0;

   if (iupCheck(n,IUP_RESIZE) ||
       iupCheck(n,IUP_BORDER))
   {
      decorx += 2*MOTIF_BORDER_SIZE;
   }
   return sw/scale - decorx;
}

int iupdrvWindowSizeY (Ihandle *n, int scale)
{
   int sh = DisplayHeight(iupmot_display, iupmot_screen);
   int decory=0;

   if (iupCheck(n,IUP_MENUBOX) ||
       iupCheck(n,IUP_MINBOX)  ||
       iupCheck(n,IUP_MAXBOX)  ||
       IupGetAttribute(n,IUP_TITLE) )
   {
      decory += MOTIF_TITLE_SIZE;
   }
   if (iupCheck(n,IUP_RESIZE) ||
       iupCheck(n,IUP_BORDER))
   {
      decory += 2*MOTIF_BORDER_SIZE;
   }
   if (IupGetAttribute (n,IUP_MENU) != NULL)
   {
      decory += 30;
   }
   return sh/scale - decory;
}

int iupdrvGetBorderSize(Ihandle* n)
{
  return 2+1;     /* border+focus area */
}

int iupdrvGetScrollSize(void) 
{ 
  return 18; 
}

