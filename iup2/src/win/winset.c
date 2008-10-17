/** \file
 * \brief Windows Driver set for attributes
 *
 * See Copyright Notice in iup.h
 * $Id: winset.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <stdio.h>              
#include <assert.h>
#include <math.h>

#include <windows.h>
#include <commctrl.h>

#include "iglobal.h"
#include "imask.h"
#include "idrv.h"
#include "win.h"
#include "wintips.h"
#include "wincreat.h"
#include "winicon.h"
#include "winproc.h"
#include "winhandle.h"
#include "wintoggle.h"

static void setoption   (Ihandle* n, int op, char* v);
static void setactive   (Ihandle* n, char* v);
static void setborder   (Ihandle* n, char* v);
static void setfgcolor  (Ihandle* n, char* v);
static void setbgcolor  (Ihandle* n, char* v);
static void setcursor   (Ihandle* n, char* v);
static void setcurcursor(Ihandle* n, char* v);
static void setmenu     (Ihandle* n, char* v);
static void setsize     (Ihandle* n, char* v);
static void settitle    (Ihandle* n, char* v);
static void setvalue    (Ihandle* n, char* v);
static void setvisible  (Ihandle* n, char* v);
static void setposx     (Ihandle* n, char* v);
static void setposy     (Ihandle* n, char* v);
static void setimage    (Ihandle* n, char* v);
static void setnc       (Ihandle* n, char* v);
static void setappend   (Ihandle* n, char* v);
static void setmask     (Ihandle* n, char* v);
static void settip      (Ihandle* n, char* v);
static void settabsize  (Ihandle* n, char* v);
static void setcaret    (Ihandle* n, char* v);
static void setinsert   (Ihandle* n, char* v);
static void setfont     (Ihandle* n, char* v);
static void seticon     (Ihandle* n, char* v);
static void settopmost  (Ihandle* n, char* v);
static void setselection(Ihandle* n, char* v);
static void setrastersize (Ihandle* n, char* v);
static void setalignment  (Ihandle *n, char *v);
static void setreadonly   (Ihandle* n, char* v);
static void setfullscreen (Ihandle* n, char* v);
static void setbringfront (Ihandle *n, char *v);
static void settray       (Ihandle *n, char *v);
static void settraytip    (Ihandle *n, char *v);
static void settrayimage  (Ihandle *n, char *v);
static void sethidetaskbar(Ihandle *n, char *v);
static void setseparator  (Ihandle *n, char *v);
static void setselectedtext(Ihandle *n, char *v);
static void setshowdropdown(Ihandle *n, char *v);
static void setlayered     (Ihandle *n, char *v);
static void setlayeralpha  (Ihandle *n, char *v);
static void setmdiarrange  (Ihandle *n, char *v);
static void setmdiactivate (Ihandle *n, char *v);
static void setmdicloseall (Ihandle *n, char *v);
static void setzorder      (Ihandle *n, char *v);
static void setclipboard   (Ihandle *n, char *v);

static struct
{
  char *name;
	void (*set)(Ihandle*, char*);
} attributes[] = { 
                   {"ACTIVE",   setactive   },
				           {"BORDER",   setborder   },
				           {"FGCOLOR",  setfgcolor  },
				           {"BGCOLOR",  setbgcolor  },
				           {"CONID",    NULL        },
				           {"CURSOR",   setcursor   },
				           {"CURCURSOR",setcurcursor},
				           {"MENU",     setmenu     },
				           {"SIZE",     setsize     },
				           {"TITLE",    settitle    },
				           {"VALUE",    setvalue    },
				           {"VISIBLE",  setvisible  },
				           {"POSX",     setposx     },
				           {"POSY",     setposy     },
				           {"IMAGE",    setimage    },
				           {"MASK",     setmask     },
				           {"NC",       setnc       },
				           {"APPEND",   setappend   },
                   {"TIP",      settip      },
                   {"TABSIZE",  settabsize  },
                   {"CARET",    setcaret    },
                   {"INSERT",   setinsert   },
                   {"_IUPWIN_FONT", setfont },
                   {"FONT",     setfont     },
                   {"ICON",     seticon     },
                   {"SELECTEDTEXT",setselectedtext},
                   {"SEPARATOR",  setseparator},
                   {"TOPMOST",    settopmost  },
                   {"SELECTION",  setselection},
                   {"RASTERSIZE", setrastersize},
                   {"ALIGNMENT",  setalignment},
                   {"READONLY",   setreadonly},
                   {"FULLSCREEN", setfullscreen},
                   {"BRINGFRONT", setbringfront},
                   {"TRAYIMAGE",  settrayimage},
                   {"TRAYTIP",    settraytip},
                   {"TRAY",       settray},
                   {"HIDETASKBAR",sethidetaskbar},
                   {"LAYERED",    setlayered},
                   {"LAYERALPHA", setlayeralpha},
                   {"MDIARRANGE", setmdiarrange},
                   {"MDIACTIVATE",setmdiactivate},
                   {"MDICLOSEALL",setmdicloseall},
                   {"ZORDER",     setzorder},
                   {"SHOWDROPDOWN",setshowdropdown},
                   {"CLIPBOARD",  setclipboard}
};                                               


#define NATTRIBUTES (sizeof(attributes)/sizeof(attributes[0]))

#define NBRUSHES   256

static struct
		  {
         int free;
         HBRUSH b;
         COLORREF c;
        }brushes[NBRUSHES];


static char *wincolor[] = {
        "COLOR_SCROLLBAR",
        "COLOR_BACKGROUND",
        "COLOR_ACTIVECAPTION",
        "COLOR_INACTIVECAPTION",
        "COLOR_MENU",
        "COLOR_WINDOW",
        "COLOR_WINDOWFRAME",
        "COLOR_MENUTEXT",
        "COLOR_WINDOWTEXT",
        "COLOR_CAPTIONTEXT",
        "COLOR_ACTIVEBORDER",
        "COLOR_INACTIVEBORDER",
        "COLOR_APPWORKSPACE",
        "COLOR_HIGHLIGHT",
        "COLOR_HIGHLIGHTTEXT",
        "COLOR_BTNFACE",
        "COLOR_BTNSHADOW",
        "COLOR_GRAYTEXT",
        "COLOR_BTNTEXT",
        "COLOR_INACTIVECAPTIONTEXT",
        "COLOR_BTNHIGHLIGHT",
        NULL };

void iupdrvSetAttribute(Ihandle* n, const char* a, const char* v)
{
 if (n == NULL)
  return;
 iupwinSet (n, a, (char*)v);
}

int iupdrvStoreAttribute(Ihandle*h, const char*attr)
{
  return 1;
}

void iupwinSetInit (void)
{
 int i;
 for (i=0; i<NBRUSHES; ++i)
  brushes[i].free= 1;
}

void iupwinSetFinish (void)
{
 int i;
 for (i=0; i<NBRUSHES; ++i)
  if (!brushes[i].free)
  {
    DeleteObject (brushes[i].b);
    brushes[i].free = 1;
  }
}

void iupwinUpdateAttr(Ihandle *n, const char *a)
{
  char *v;
  assert(n);
  if(n==NULL)
    return;

  assert(a);
  if(a == NULL)
    return;

  v = iupGetEnv(n, a);
  if(v)
  {
    int i;
    for (i=0; i<NATTRIBUTES; ++i)                  /* find attribute */
    {
      if (!iupStrEqual(a, attributes[i].name) == 0)     /* found atribute in table */
	      break;
    }

    if (i == NATTRIBUTES)                         
	      return;

    if ((i < NATTRIBUTES) && (attributes[i].set != NULL))
      attributes[i].set(n, v);                   /* set attribute for node   */
  }

}

void iupwinSet (Ihandle* n, const char* a, char* v)
{
 int i;
 Ihandle *c;

 if (n == NULL)
  return;

 for (i=0; i<NATTRIBUTES; ++i)                  /* find attribute */
 {
   if (!iupStrEqual(a, attributes[i].name) == 0)     /* found atribute in table */
	   break;
 }

 if (i == NATTRIBUTES)                          /* try attributes that defines list options */
 {
  int op;
  if (sscanf (a, "%d", &op) != 1)
	  return;
  setoption (n, op, v);
 }

 if ((i < NATTRIBUTES) && (attributes[i].set != NULL))
   attributes[i].set(n, v);                   /* set attribute for node   */

 /* Inheritance (if not size, value, visible...) */
 if (iupIsInheritable(a) && !iupStrEqual(a, IUP_VISIBLE))
 {
	 foreachchild(c,n)                              /* and for your childrens   */
   {
		 if(iupTableGet(env(c), a) == NULL)           /* that do not have the     */
       iupwinSet(c, a, v);
   }
 }
}

void iupwinSetCursor (Ihandle *n)
{
  setcursor (n, iupGetEnv (n, IUP_CURSOR));
}

int iupwinGetColorRef (Ihandle *n, const char *attr, COLORREF *cr)
{
 char *c;

 if (!n)
  return 0;

 c = IupGetAttribute (n, attr);
 if (!c)
 {
  return 0;
 }
 else
 {
  /* non official behavior */
  int i;
  for (i=0; wincolor[i] && !iupStrEqual(c,wincolor[i]); ++i)
   ;
  if (wincolor[i])
   *cr = GetSysColor(i);
  else
  {
	 unsigned int r, g, b;
   iupGetColor(n,attr, &r, &g, &b);
   *cr = RGB(r,g,b);
  }
  return 1;
 }  
}

int iupwinGetBrushIndex (COLORREF c)
{
 int i;

 /* If a brush with the desired color already exists... */
 for (i=0; i<NBRUSHES; ++i)
  if (!brushes[i].free && (brushes[i].c == c))
   return i;

 /* If it doesn't exist, it should be created... */
 for (i=0; i<NBRUSHES; ++i)
  if (brushes[i].free)
  {
   brushes[i].free = 0;
   brushes[i].c = c;
   brushes[i].b = CreateSolidBrush (c);
   assert(brushes[i].b); /* Assert windows creation of the solid brush */
   return i;
  }

 assert(0); /* Array size is over */
 return 0;
}

HBRUSH iupwinGetBrush (int i)
{
 assert(brushes[i].b); /* referenced brush should exist */
 return brushes[i].b;
}

void iupwinSetBgColor (Ihandle *n)
{
  COLORREF c;
  /* this function is called only for dialog */
  if (iupwinGetColorRef(n, IUP_BGCOLOR, &c))
    SetClassLongPtr( (HWND)handle(n), GCLP_HBRBACKGROUND, (LONG_PTR)brushes[iupwinGetBrushIndex (c)].b );
}

char* iupwinTransMultiline( char* txt, int* to_free )
{
	char* auxstr;
	char* result;
	int i, n = 0;

	result = txt;
  *to_free = 0;

	if (result)
	{
		auxstr = strchr(txt,'\n');
		while(auxstr)
		{
			if(auxstr==txt || *(auxstr-1)!='\r')
         {
            n++;
         }
			auxstr = strchr(auxstr+1,'\n');
		}
		if (n)
		{
			result = (char*)malloc((n+strlen(txt)+1)*sizeof(char));
      *to_free = 1;
			for(i=0,auxstr=result;txt[i];i++,auxstr++)
			{
				if (txt[i] == '\n' && (i==0 || txt[i-1]!='\r'))
				{
					*auxstr = '\r';
					auxstr++;
				}
				*auxstr=txt[i];
			}
			*auxstr=0;
		}
	}
	return result;	
}

static void updatemenu( Ihandle* n )
{
   Ihandle* dial = (Ihandle*) IupGetAttribute( n, "_IUPWIN_DIALOG" );
   if (dial && handle(dial))
      DrawMenuBar((HWND)handle(dial));
}

static int Check(char *v)
{
 if (v == NULL)
  return NOTDEF;
 else if (iupStrEqualNoCase(v, IUP_NO) || iupStrEqualNoCase(v, IUP_OFF))
  return NO;
 else if (iupStrEqualNoCase(v, IUP_YES) || iupStrEqualNoCase(v, IUP_ON))
  return YES;
 return NOTDEF;
}

static void setnc(Ihandle* n, char* v)
{
   if (n==NULL || handle(n)==NULL || (type(n)!=TEXT_ && type(n)!=MULTILINE_ && type(n)!=LIST_))
      return;

   if(v)
   {
      int maxchar = atoi(v);

      if (type(n)==LIST_)
        SendMessage( (HWND)handle(n), CB_LIMITTEXT, maxchar, 0L );
      else
        SendMessage( (HWND)handle(n), EM_LIMITTEXT, maxchar, 0L );
   }

}

#ifndef BUTTON_IMAGELIST_ALIGN_CENTER
#define BUTTON_IMAGELIST_ALIGN_CENTER   4      
#define BUTTON_IMAGELIST_ALIGN_LEFT     0
typedef struct
{
    HIMAGELIST  himl;   
    RECT        margin; 
    UINT        uAlign;
} BUTTON_IMAGELIST, *PBUTTON_IMAGELIST;
#define BCM_FIRST               0x1600      
#define BCM_SETIMAGELIST        (BCM_FIRST + 0x0002)
#endif

static void winSetBitmap(Ihandle* n, Ihandle* image)
{
  HBITMAP bmp = iupwinGetBitmap(image, n);
  if (!iupwinUseComCtl32Ver6())
    SendMessage((HWND)handle(n), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
  else
  {
    HIMAGELIST himl;
    BUTTON_IMAGELIST btn_img_list;
    int idx;

    himl = (HIMAGELIST)IupGetAttribute(image, "_IUPWIN_HIMAGELIST");
    if (himl)
      ImageList_Destroy(himl);

    himl = ImageList_Create(IupGetInt(image, "WIDTH"), IupGetInt(image, "HEIGHT"), ILC_COLOR24, 1, 0);
    idx = ImageList_Add(himl, bmp, NULL);

    btn_img_list.himl = himl;
    btn_img_list.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
    btn_img_list.margin.top = 0;
    btn_img_list.margin.bottom = 0;
    btn_img_list.margin.left = 0;
    btn_img_list.margin.right = 0;
     
    SendMessage((HWND)handle(n), BCM_SETIMAGELIST, 0, (LPARAM)&btn_img_list);
    IupSetAttribute(image, "_IUPWIN_HIMAGELIST", (char*)himl);
  }
}

static void setimage(Ihandle* n, char* v)
{
  if (n==NULL || handle(n)==NULL)
    return;

  if(type(n) == TOGGLE_)
  {
    /* Updates image, but only if allowed by the toggle, i.e. only if no
    * image_press is set or the toggle is not selected. */
    Ihandle *pressimage = IupGetAttributeHandle(n, IUP_IMPRESS);
    Ihandle *image      = IupGetAttributeHandle(n, IUP_IMAGE);
    int active = iupCheck(n, "ACTIVE")? 1: 0;
    int pressed = iupCheck(n, "VALUE")==YES? 1: 0;

    if(active)
    {
      if(pressed && pressimage)
      {
        winSetBitmap(n, pressimage);
      }
      else if(image)           
      {
        winSetBitmap(n, image);
      }
    }
    else
    {
      Ihandle* iminactive = IupGetAttributeHandle( n, IUP_IMINACTIVE );
      if (iminactive)
      {
        winSetBitmap(n, iminactive);
      }
      else if(image)
      {
        IupSetAttribute(image, "_IUPWIN_MAKE_INACTIVE", "1");
        winSetBitmap(n, image);
        IupSetAttribute(image, "_IUPWIN_MAKE_INACTIVE", NULL);
      }
    }

    InvalidateRect((HWND)handle(n),NULL,FALSE);
   }
  else if(type(n) == BUTTON_ || type(n) == LABEL_)
    InvalidateRect((HWND)handle(n),NULL,FALSE);
}

static void setposx(Ihandle* n, char* v)
{
 if (n == NULL || handle(n)==NULL)
  return;

 if ( currentwidth(n) &&
      ((IupGetAttribute(n,IUP_SCROLLBAR) && iupStrEqual(IupGetAttribute(n,IUP_SCROLLBAR),IUP_YES))||
       (IupGetAttribute(n,IUP_SCROLLBAR) && iupStrEqual(IupGetAttribute(n,IUP_SCROLLBAR),IUP_HORIZONTAL))||
       (IupGetAttribute(n,IUP_SBH) && iupStrEqual(IupGetAttribute(n,IUP_SBH),IUP_YES))
      )
    )
 {
    iupwinSetHScrollInfo(n);
 }
}

static void setfullscreen(Ihandle* n, char* v)
{
   BOOL visible;
   if(handle(n) == NULL || type(n) != DIALOG_)
     return;

   if(v && iupStrEqualNoCase(v, "YES"))
   {
     if(!iupGetEnv(n, "_IUPWIN_FS_STYLE"))
     {
       RECT rect;
       char size_str[100];
       int width, height;
       LONG off_style, new_style;

       visible = ShowWindow ((HWND)handle(n), SW_HIDE);

       off_style = WS_BORDER | WS_THICKFRAME | WS_CAPTION | 
                   WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
       new_style = GetWindowLong(handle(n), GWL_STYLE);
       iupSetEnv(n, "_IUPWIN_FS_STYLE", (void*)new_style);
       new_style &= (~off_style);
       SetWindowLong(handle(n), GWL_STYLE, new_style);

       iupStoreEnv(n, "_IUPWIN_FS_MAXBOX", iupGetEnv(n, IUP_MAXBOX));
       iupStoreEnv(n, "_IUPWIN_FS_MINBOX", iupGetEnv(n, IUP_MINBOX));
       iupStoreEnv(n, "_IUPWIN_FS_MENUBOX",iupGetEnv(n, IUP_MENUBOX));
       iupStoreEnv(n, "_IUPWIN_FS_TITLE",  iupGetEnv(n, IUP_TITLE));
       iupStoreEnv(n, "_IUPWIN_FS_RESIZE", iupGetEnv(n, IUP_RESIZE));
       iupStoreEnv(n, "_IUPWIN_FS_BORDER", iupGetEnv(n, IUP_BORDER));
       iupStoreEnv(n, "_IUPWIN_FS_X", IupGetAttribute(n, IUP_X));
       iupStoreEnv(n, "_IUPWIN_FS_Y", IupGetAttribute(n, IUP_Y));
       iupStoreEnv(n, "_IUPWIN_FS_SIZE", IupGetAttribute(n, IUP_RASTERSIZE));

       iupSetEnv(n, IUP_MAXBOX, "NO");
       iupSetEnv(n, IUP_MINBOX, "NO");
       iupSetEnv(n, IUP_MENUBOX, "NO");
       iupSetEnv(n, IUP_TITLE, NULL);
       iupSetEnv(n, IUP_RESIZE, "NO");
       iupSetEnv(n, IUP_BORDER, "NO");

       GetWindowRect(GetDesktopWindow(), &rect);
       width = rect.right - rect.left;
       height = rect.bottom - rect.top;
       sprintf(size_str, "%dx%d", width, height);

       SetWindowPos(handle(n), HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
       IupRefresh(n);
       if(visible != 0)
         ShowWindow ((HWND)handle(n), SW_SHOW);
       UpdateWindow ((HWND)handle(n));
     }
   }
   else
   {
     char *v = iupGetEnv(n, "_IUPWIN_FS_STYLE");
     if (v)
     {
       visible = ShowWindow ((HWND)handle(n), SW_HIDE);

       iupStoreEnv(n, "MAXBOX", iupGetEnv(n, "_IUPWIN_FS_MAXBOX"));
       iupStoreEnv(n, "MINBOX", iupGetEnv(n, "_IUPWIN_FS_MINBOX"));
       iupStoreEnv(n, "MENUBOX",iupGetEnv(n, "_IUPWIN_FS_MENUBOX"));
       iupStoreEnv(n, "TITLE",  iupGetEnv(n, "_IUPWIN_FS_TITLE"));
       iupStoreEnv(n, "RESIZE", iupGetEnv(n, "_IUPWIN_FS_RESIZE"));
       iupStoreEnv(n, "BORDER", iupGetEnv(n, "_IUPWIN_FS_BORDER"));

       SetWindowLong(handle(n), GWL_STYLE, (LONG) v);
       SetWindowPos(handle(n), HWND_TOP, IupGetInt(n, "_IUPWIN_FS_X"), 
                                         IupGetInt(n, "_IUPWIN_FS_Y"), 
                                         IupGetInt(n, "_IUPWIN_FS_SIZE"), 
                                         IupGetInt2(n, "_IUPWIN_FS_SIZE"), 0);
       IupRefresh(n);
       if(visible != 0)
         ShowWindow ((HWND)handle(n), SW_SHOW);

       iupSetEnv(n, "_IUPWIN_FS_STYLE", NULL);
     }
   }
}

static void setposy(Ihandle* n, char* v)
{
 if (n == NULL || handle(n)==NULL)
  return;

 if ( currentheight(n) &&
      ((IupGetAttribute(n,IUP_SCROLLBAR) && iupStrEqual(IupGetAttribute(n,IUP_SCROLLBAR),IUP_YES))||
       (IupGetAttribute(n,IUP_SCROLLBAR) && iupStrEqual(IupGetAttribute(n,IUP_SCROLLBAR),IUP_VERTICAL))||
       (IupGetAttribute(n,IUP_SBV) && iupStrEqual(IupGetAttribute(n,IUP_SBV),IUP_YES))
      )
    )
 {
    iupwinSetVScrollInfo(n);
 }
/* InvalidateRect((HWND)handle(n),NULL,FALSE);*/
}

/* Calls the given function with values a and v for every descendent of a provided Ihandle */
static void foreverychild(Ihandle *n, void (*fptr)(Ihandle *, char*), char *v)
{
  Ihandle *c = NULL;
  foreachchild(c, n)
  {
    foreverychild(c, fptr, v);
    fptr(c, v);
  }
}

static void setactive  (Ihandle* n, char* v)
{
 if (n == NULL || handle(n)==NULL)
  return;

 switch (Check (v))
 {
  case NO:
	if ((type(n) == TEXT_) || (type(n) == LABEL_) || type(n) == BUTTON_ ||
		 (type(n) == MULTILINE_) || (type(n) == LIST_) || (type(n) == CANVAS_) ||
     (type(n) == DIALOG_) || (type(n) == FRAME_))
  {
    EnableWindow ((HWND)handle(n), 0);
  }
  else if(type(n) == TOGGLE_)
  {
    /* If there is NO IMAGE, let Windows handle the inactive toggle */
    if(IupGetAttribute(n, IUP_IMAGE) == NULL)
      EnableWindow ((HWND)handle(n), 0);
    else
    {
      if (iupwinUseComCtl32Ver6())
        EnableWindow ((HWND)handle(n), 0);
      else
        iupSetEnv(n, "_IUPWIN_TOGGLE_ACTIVE", "NO"); 
      iupwinUpdateAttr(n, IUP_IMAGE);
    }
  }
  else if(type(n) == SUBMENU_)
  {
    assert(number(n)>0);
    EnableMenuItem((HMENU)handle(parent(n)), 
      (UINT)handle(n), MF_GRAYED);
    updatemenu(n);
  }
	else if (type(n) == ITEM_)
        {
          assert(number(n)>0);
	        EnableMenuItem ((HMENU)handle(n), number(n), MF_GRAYED);
          updatemenu(n);
        }
  else if (type(n) == RADIO_)
    foreverychild(n, setactive, v);
	break;
  case YES:
  case NOTDEF:
  default:
	if ((type(n) == TEXT_) || (type(n) == LABEL_) || type(n) == BUTTON_ ||
	    (type(n) == MULTILINE_) || (type(n) == LIST_) || (type(n) == CANVAS_) ||
     (type(n) == DIALOG_) || (type(n) == FRAME_))
   {
     EnableWindow ((HWND)handle(n), 1);
   }
   else if(type(n) == TOGGLE_)
   {
     if(IupGetAttribute(n, IUP_IMAGE) == NULL)
       EnableWindow ((HWND)handle(n), 1);
     else
     {
       if (iupwinUseComCtl32Ver6())
         EnableWindow ((HWND)handle(n), 1);
       else
         iupSetEnv(n, "_IUPWIN_TOGGLE_ACTIVE", "YES");
       iupwinUpdateAttr(n, IUP_IMAGE);
     }
   }
   else if(type(n) == SUBMENU_)
   {
    assert(number(n)>0);
    EnableMenuItem((HMENU)handle(parent(n)), 
      (UINT)handle(n), MF_ENABLED);
    updatemenu(n);
   }
   else if (type(n) == ITEM_)
   {
    assert(number(n)>0);
    EnableMenuItem ((HMENU)handle(n), number(n), MF_ENABLED);
    updatemenu(n);
   }
   else if (type(n) == RADIO_)
     foreverychild(n, setactive, v);
 }
}

static void setborder  (Ihandle* n, char* v)
{
 LONG a;
 if ((n == NULL) || (handle(n) == NULL) || (type(n) == DIALOG_) ||
     (type(n) == HBOX_) || (type(n) == VBOX_) || (type(n) == FILL_))
  return;

 ShowWindow ((HWND)handle(n), SW_HIDE);

 a = GetWindowLong((HWND)handle(n), GWL_STYLE);
 if (Check (v) == NO)
  a &= ~WS_BORDER;
 else
  a |= WS_BORDER;

 SetWindowLong((HWND)handle(n), GWL_STYLE, a);
 ShowWindow ((HWND)handle(n), SW_SHOW);
}

static void setfgcolor (Ihandle* n, char* v)
{
 if ((n == NULL) || (handle(n) == NULL) || (type(n) == HBOX_) ||
     (type(n) == VBOX_) || (type(n) == FILL_))
  return;
 RedrawWindow((HWND)handle(n),NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

static void setbgcolor (Ihandle* n, char* v)
{
 if ((n == NULL) || (handle(n) == NULL) || (type(n) == HBOX_) ||
     (type(n) == VBOX_) || (type(n) == FILL_))
  return;

 if (type(n) == DIALOG_)
    iupwinSetBgColor (n);

 if (type(n) == CANVAS_)
   RedrawWindow((HWND)handle(n),NULL,NULL,RDW_ERASE|RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
 else
   RedrawWindow((HWND)handle(n),NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif
#ifndef IDC_APPSTARTING
#define IDC_APPSTARTING     MAKEINTRESOURCE(32650) 
#endif
#ifndef IDC_HELP
#define IDC_HELP            MAKEINTRESOURCE(32651)
#endif

static HCURSOR winGetCursor(char* cursor)
{
 if (iupStrEqual(cursor,"NULL") || iupStrEqual(cursor,"NONE"))
  return NULL;
 else if (iupStrEqual(cursor,"ARROW"))
  return LoadCursor (NULL, IDC_ARROW);
 else if (iupStrEqual(cursor,"TEXT"))
  return LoadCursor (NULL, IDC_IBEAM);
 else if (iupStrEqual(cursor,"BUSY"))
  return LoadCursor (NULL, IDC_WAIT);
 else if (iupStrEqual(cursor,"CROSS"))
  return LoadCursor (NULL, IDC_CROSS);
 else if (iupStrEqual(cursor,"RESIZE_S") || iupStrEqual(cursor,"RESIZE_N"))
  return LoadCursor (NULL, IDC_SIZENS);
 else if (iupStrEqual(cursor,"RESIZE_W") || iupStrEqual(cursor,"RESIZE_E"))
  return LoadCursor (NULL, IDC_SIZEWE);
 else if (iupStrEqual(cursor,"RESIZE_NW") || iupStrEqual(cursor,"RESIZE_SE"))
  return LoadCursor (NULL, IDC_SIZENWSE);
 else if (iupStrEqual(cursor,"RESIZE_NE") || iupStrEqual(cursor,"RESIZE_SW"))
  return LoadCursor (NULL, IDC_SIZENESW);
 else if (iupStrEqual(cursor,"MOVE"))
  return LoadCursor (NULL, IDC_SIZEALL);
 else if (iupStrEqual(cursor,"APPSTARTING"))
  return LoadCursor (NULL, IDC_APPSTARTING);
 else if (iupStrEqual(cursor,"HELP"))
  return LoadCursor (NULL, IDC_HELP);
 else if (iupStrEqual(cursor,"NO"))
  return LoadCursor (NULL, IDC_NO);
 else if (iupStrEqual(cursor,"UPARROW"))
  return LoadCursor (NULL, IDC_UPARROW);
 else if (iupStrEqual(cursor,"PEN"))
  return LoadCursor (iupwin_hinstance, "CURSOR_PEN");
 else if (iupStrEqual(cursor,"HAND"))
  return LoadCursor (NULL, IDC_HAND);
 else if (iupStrEqual(cursor,"HELP"))
  return LoadCursor (iupwin_hinstance, "CURSOR_HELP");
 else
  return LoadCursor(iupwin_hinstance, cursor);
}

static void setcursor  (Ihandle* n, char* cursor)
{
  HCURSOR hCur;
  if ((handle(n) == NULL) || ((type(n) != DIALOG_) && (type(n) != CANVAS_)) )
    return;

  if (!cursor) 
    return;

  hCur = winGetCursor(cursor);
  if (!hCur && cursor)
    hCur = iupwinGetIcon(cursor, handle(n), IUP_CURSOR);

  SetClassLongPtr((HWND)handle(n), GCLP_HCURSOR, (LONG_PTR)hCur); 
  if(IsIconic((HWND)handle(n)) == 0)
    SetCursor(hCur); 
}

static void setcurcursor  (Ihandle* n, char* cursor)
{
  HCURSOR hCur;
  if ((handle(n) == NULL))
    return;

  hCur = winGetCursor(cursor);
  if (!hCur)
    hCur = iupwinGetIcon(cursor, handle(n), IUP_CURSOR);

  SetCursor(hCur);
}

static void setmenu (Ihandle* n, char* v)
{
 if ((n == NULL) || (handle(n) == NULL) || (type(n) != DIALOG_))
  return;
 SetMenu ((HWND) handle(n), iupwinGetMenu (n));
}

/* If there is no item selected, in a simple list, then set first item */
static void assertdefaultselection(Ihandle *n)
{
  int nop = SendMessage ((HWND)handle(n), CB_GETCOUNT, 0, 0L);

  /* If there is at least one element... */
  if(nop > 0)
  {
    if(SendMessage ((HWND)handle(n), CB_GETCURSEL, 0L, 0L) == CB_ERR)
    {
      SendMessage ((HWND)handle(n), CB_SETCURSEL, 0, 0L);
      iupSetEnv(n, "_IUPWIN_LISTSEL", "1");
    }
  }
  else
    SendMessage ((HWND)handle(n), CB_RESETCONTENT, 0, 0L);
}

static void setoption (Ihandle* n, int op, char* v)
{
 LRESULT nop, i;
 int iscombobox = iupCheck(n, IUP_DROPDOWN)==YES||iupCheck(n, "EDITBOX")==YES? 1: 0;
 UINT addstring = (iscombobox) ? CB_ADDSTRING : LB_ADDSTRING;
 UINT deletestring = (iscombobox) ? CB_DELETESTRING : LB_DELETESTRING;
 UINT insertstring = (iscombobox) ? CB_INSERTSTRING : LB_INSERTSTRING;
 UINT getcount     = (iscombobox) ? CB_GETCOUNT : LB_GETCOUNT;

 if ((n == NULL) || (handle(n) == NULL) || (type(n) != LIST_))
  return;

 nop = SendMessage ((HWND)handle(n), getcount, 0, 0L);
 
 if (v!=NULL)
 {
	 if (op > nop)  /* option not exist, insert new option */
	 {
     char *old_sel = NULL;

	   for (i = nop; i < (op-1); ++i)           /* insert blank options */
       SendMessage ((HWND)handle(n), addstring, 0, (LPARAM)" ");

     if (iupCheck(n, IUP_MULTIPLE)==YES)
     {
       char *new_sel = NULL;
       int size = 0;

       old_sel = iupGetEnv(n, "_IUPWIN_LISTSEL");
       if(old_sel)
         size = strlen(old_sel);

       new_sel = (char*) malloc (sizeof(char)*(op + 1));
       if(old_sel)
         strncpy(new_sel, old_sel, MIN(size, op));
       else
         new_sel[0] = 0;
       for(i = nop; i < op; ++i)
         new_sel[i] = '-';
         
       new_sel[op] = 0;
       iupStoreEnv(n, "_IUPWIN_LISTSEL", new_sel);
       free(new_sel);
     }

	   SendMessage ((HWND)handle(n), addstring, 0, (LPARAM)v);  /* insert new option */

     if(iscombobox) 
       assertdefaultselection(n);

	 }
	 else           /* change old option */
	 {
     int setselected = 0;

     if(iscombobox)
       if(SendMessage ((HWND)handle(n), CB_GETCURSEL, 0L, 0L)==op-1)
         setselected  = 1;

	   SendMessage ((HWND)handle(n), deletestring, op-1, 0L);
	   if (v)
	   SendMessage ((HWND)handle(n), insertstring, op-1, (LPARAM)v);

     if(setselected)
       SendMessage ((HWND)handle(n), CB_SETCURSEL, op-1, 0L);
	 }

   if (iupCheck(n, IUP_DROPDOWN)==YES && iupCheck(n, "DROPEXPAND")!=NO)
   {
     int w, h, old_w, sb = iupdrvGetScrollSize()+6; /* +borders */
     iupdrvStringSize(n, v, &w, &h);
     old_w = SendMessage((HWND)handle(n), CB_GETDROPPEDWIDTH, 0, 0);
     if (w+sb > old_w)
      SendMessage((HWND)handle(n), CB_SETDROPPEDWIDTH, w+sb, 0);
   }
 }
 else
 {
	 if (op <= nop)
	 {
     char *old_set = iupGetEnv(n, "_IUPWIN_LISTSEL");  

     /* Antigo elemento selecionado. Se este for deletado, entao o iup nao deve
        chamar a callback de unselect. */
     if (old_set && iupCheck(n, IUP_MULTIPLE)!=YES && atoi(old_set) >= op)
       iupSetEnv(n, "_IUPWIN_LISTSEL", NULL);

     for (i=op; i<=nop; i++)
	     SendMessage ((HWND)handle(n), deletestring, op-1, 0L);

     if (iupCheck(n, IUP_MULTIPLE)==YES)
     {
       /* In a multiple list box it is only possible to remove 
          from the bottom of the list */
       char *sel = iupGetEnv(n, "_IUPWIN_LISTSEL");
       if(sel)
         if((int)strlen(sel)>=op)
         {
           char *v = (char*) malloc(sizeof(char)*(strlen(sel)+1));
           assert(v);
           strcpy(v, sel);
           v[op-1] = 0;
           iupStoreEnv(n, "_IUPWIN_LISTSEL", v);
           free(v);
         }
     } 
     else if(iscombobox) 
       assertdefaultselection(n); /* If there is no item selected, set first item */
	 }
 }
}

static void common_setsize(Ihandle *n, char *v)
{
  if (handle(n) != NULL)
  {
    if(iupSetSize (n) != IUP_ERROR)
      iupdrvResizeObjects(n);
  }
}

static void setrastersize (Ihandle* n, char* v)
{
  assert(n != NULL);
  if(n == NULL)
    return;

  if(type(n) == DIALOG_)
  {
    iupSetEnv(n, IUP_SIZE, NULL);
    common_setsize(n, v);
  }
}

static void setsize (Ihandle* n, char* v)
{
  assert(n != NULL);
  if(n == NULL)
    return;

  if(type(n) == DIALOG_)
  {
    iupSetEnv(n, IUP_RASTERSIZE, NULL);
    common_setsize(n, v);
  }
}


static void settitle (Ihandle* n, char* v)
{
 if (n == NULL)
   return;

 if(type(n) == DIALOG_ || type(n) == TOGGLE_ ||
    type(n) == BUTTON_ || type(n) == FRAME_  || 
    type(n) == LABEL_ )
 {

   if(handle(n) == NULL)
     return;

   SetWindowText ((HWND)handle(n), v);
 }
 else if(type(n) == ITEM_)
 { 
     MENUITEMINFO iteminfo;
     BOOL result;

     if(handle(n) == NULL)
       return;

     iteminfo.cbSize = sizeof(MENUITEMINFO);
     iteminfo.fMask = MIIM_TYPE;
     iteminfo.fType = MFT_STRING;
     iteminfo.dwTypeData = iupwinMenuLabel(n, IupGetAttribute(n, IUP_TITLE));

     assert(number(n)>0);
     result = SetMenuItemInfo((HMENU)handle(n), number(n), FALSE, &iteminfo);
     assert(result);
 }

 return;
}

/* Searches for the toggle (parameter value) inside the IUP controls
   (n parameter) and sets it as "ON" */
static void setradiovalue(Ihandle* n, Ihandle* value)
{
  Ihandle* aux = NULL;
  if (value == NULL)
    return;

  assert(IupGetClassName(value) == TOGGLE_);

  if(type(n) == FRAME_ || type(n) == VBOX_ ||
     type(n) == HBOX_  || type(n) == ZBOX_)
  {
	  foreachchild(aux,n)
		  setradiovalue(aux, value);
  }
  else if(type(n) == TOGGLE_)
  {
    if (value == n)
    {
      IupSetAttribute(n, IUP_VALUE, IUP_ON);
      return;
    }
  }
  return;
}

static void setvalue (Ihandle* n, char* v)
{
  if (n == NULL)
    return;

  if(type(n) == MULTILINE_)
  {
    int to_free;
    char* str;
    if(handle(n) != NULL)
    {
      str = iupwinTransMultiline(v,&to_free);
	    SetWindowText ((HWND)handle(n), str);
      if (to_free)
        free(str);
    }
  }
  else if(type(n) == TEXT_)
  {
    ITextInfo* txtinfo = (ITextInfo*)IupGetAttribute(n,"_IUPWIN_TXTINFO__");
    if (!v || !txtinfo || iupCheckMask(txtinfo->mask, v, 0))
      if(handle(n) != NULL)
	      SetWindowText ((HWND)handle(n), v);
  }
  else if(type(n) == TOGGLE_)
  {
    int check = iupStrEqualNoCase(v,IUP_ON)? BST_CHECKED: iupStrEqualNoCase(v,"NOTDEF")? BST_INDETERMINATE: BST_UNCHECKED;

    if(handle(n) != NULL)
    {
      int oldcheck = SendMessage ((HWND)handle(n), BM_GETCHECK, 0L, 0L);
	    if (oldcheck != check)
      {
        Ihandle *radio = iupGetRadio(n);
        if(radio && check)
        {
          Ihandle *last_tg = (Ihandle *) iupGetEnv(radio, "_IUPWIN_LASTTOGGLE");
          if (last_tg && last_tg != n)
          {
            /* This is necessary because sometimes Windows does not unset the other 
              toggle inside the radio. */
            iupSetEnv(last_tg, IUP_VALUE, IUP_OFF);
            SendMessage ((HWND)handle(last_tg), BM_SETCHECK, BST_UNCHECKED, 0L);
            iupwinUpdateAttr(last_tg, IUP_IMAGE);
            SetWindowLong((HWND)handle(last_tg), GWL_STYLE, (LONG)GetWindowLongPtr((HWND)handle(last_tg), GWL_STYLE)&(~WS_TABSTOP));
          }
          iupSetEnv(radio,"_IUPWIN_LASTTOGGLE", (char*) n);

          /* sets the tabstop only for the checked toggle */
          SetWindowLong((HWND)handle(n), GWL_STYLE, (LONG)GetWindowLongPtr((HWND)handle(n), GWL_STYLE)|WS_TABSTOP);
        }

        SendMessage ((HWND)handle(n), BM_SETCHECK, check, 0L);
        iupwinUpdateAttr(n, IUP_IMAGE);
      }
    }
    else
    {
      /* Not mapped toggle can also change value */
      Ihandle *radio = iupGetRadio(n);
      if (radio && check)
      {
        /* Control is not mapped yet, but we have to deal with toggle selection */
        Ihandle *last_tg = (Ihandle *) iupGetEnv(radio, "_IUPWIN_LASTTOGGLE");
        if (last_tg && last_tg != n)
          iupSetEnv(last_tg, IUP_VALUE, IUP_OFF);
        iupSetEnv(radio, "_IUPWIN_LASTTOGGLE", (char*) n);
      }
    }
  }
  else if(type(n) == RADIO_)
  {
	  Ihandle* aux = NULL;
    Ihandle *value = IupGetHandle(v);
    foreachchild(aux,n)
		  setradiovalue(aux, value);
  }
  else if(type(n) == ITEM_)
  {
    if (handle(n) != NULL)
    {
      assert(number(n)>0);
	    if (iupStrEqualNoCase(v,IUP_ON))
      {
        assert(number(n) >= 0);
        CheckMenuItem((HMENU)handle(n), number(n), MF_CHECKED );
      }
	    else
      {
        assert(number(n) >= 0);
	      CheckMenuItem((HMENU)handle(n), number(n), MF_UNCHECKED);
      }
    }
    updatemenu(n);
  }
  else if(type(n) == LIST_)
  {
    if(handle(n) == NULL)
      return;

	  if (v && iupCheck(n, IUP_MULTIPLE) == YES)
    {
      /* User has changed the selection of a multiple list. Clear all
          selections and update selection list */
      char *new_sel = NULL;
		  int i, size;

      size = SendMessage ((HWND)handle(n), LB_GETCOUNT, 0, 0L);

      new_sel = (char*)malloc(sizeof(char)*size+1);
      memset(new_sel, '-', size);

      SendMessage((HWND)handle(n),LB_SETSEL,(WPARAM)FALSE,MAKELPARAM(-1,0));

      for (i = 0; i<size; i++)
      {
        SendMessage((HWND)handle(n),LB_SETSEL,(WPARAM)(v[i]=='+'),MAKELPARAM(i,0));
        if(v[i]=='+' || v[i]=='-')
          new_sel[i] = v[i]; 
      }

      new_sel[size]=0;
      iupStoreEnv(n, "_IUPWIN_LISTSEL", new_sel);
      free(new_sel);

    }
    else if (v)
    {
      int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
      if (haseditbox)
      {
	      SetWindowText ((HWND)handle(n), v);
      }
      else
      {
        int iscombobox = iupCheck(n, IUP_DROPDOWN)==YES? 1: 0;
        UINT setcursel = (iscombobox) ? CB_SETCURSEL: LB_SETCURSEL;

        SendMessage((HWND)handle(n),setcursel,atoi(v)-1,0L);
        iupStoreEnv(n, "_IUPWIN_LISTSEL", v);
      }
    }
  }
  else if(type(n) == ZBOX_)
  {
    Ihandle *children  = NULL,
            *old_layer = NULL,
            *new_layer = NULL;

    if(handle(n) == NULL) 
      return; /* ZBOX ainda nao foi mapeada */

    old_layer = IupGetHandle (iupGetEnv(n, "_IUPWIN_ZBOX_OLDVALUE")),
    assert(old_layer != NULL); /* checagem de consistencia */
                                  
            
    iupStoreEnv(n, "_IUPWIN_ZBOX_OLDVALUE", v); /* guarda o nome do layer ativo */

    /* Esconde toda a arvore que comeca no elemento
        anteriormente ativo (se houver) */

    IupSetAttribute(old_layer, IUP_VISIBLE, IUP_NO);
        
    new_layer = IupGetHandle (v);

	  foreachchild(children, n)
    {
      if(new_layer == children)
      {
        IupSetAttribute(new_layer, IUP_VISIBLE, IUP_YES);
        break;
      }
    }
    
    assert(children == new_layer); /* testa se usuario passou o nome de
                                      um elemento que nao pertence a ZBOX */
  }
}

static int startRadio = 0;
static Ihandle* firsttoggle = NULL;
static void setvisibility (Ihandle* n, char *v);

/*
 *  
 */
static void hideChildren(Ihandle* n)
{
  Ihandle *child = NULL;

  assert(n != NULL);
  if(n == NULL)
    return;

  foreachchild(child, n)
    setvisibility(child, IUP_NO);
}

static int IsHiddenByAncestor(Ihandle *n)
{
  Ihandle *parent = NULL;
  int parent_visibility = 0;

  if(n == NULL)
    return YES;

  parent = parent(n);

  /* If it has no parent then it is not hidden by any ancestor.
   * We can't ask n its own value because IsHiddenByAncestor is called
   * while the user is setting a new value, therefore the current 
   * value is invalid */
  if(parent == NULL)
    return YES; 
    
  parent_visibility = Check(
    IupGetAttribute(parent, IUP_VISIBLE));
    
  switch(parent_visibility)
  {
  case YES:
  case NOTDEF:

    if(Check(iupGetEnv(parent, "_IUPWIN_VISIBLE")) == NO)
      return NO;
    else
      return IsHiddenByAncestor(parent);
    break;

  case NO:
    return NO;
    break;
  }
  return NO;
}

/*
 *  Atualiza visibilidade de todos os elementos de uma arvore
 */
static void updateChildrenVisible (Ihandle* n)
{
  Ihandle *child = NULL;

  assert(n != NULL);
  if(n == NULL)
    return;

  foreachchild(child ,n)
    setvisibility(child, iupTableGet(env(child), IUP_VISIBLE));
}

static void setvisibility (Ihandle* n, char* v)
{
   int visible = 0;

   if (type(n) != DIALOG_)
   {
      Ihandle* dial = IupGetDialog(n);
      if (dial == NULL || handle(dial) == NULL)
         return;
   }

   if(v == NULL)
     v = IUP_YES;

   if (iupStrEqualNoCase(v,IUP_YES) || iupStrEqualNoCase(v,IUP_ON))
      visible = 1;

	if (n==NULL)
		return;

	if (handle(n) == NULL && visible &&
		 (type(n)!=RADIO_ || (type(n)==RADIO_ && handle(child(n))==NULL) ) )
	{
      IupMap(n);
	}

   if (handle(n) == NULL)
      return;

   {
     switch(Check(v))
     {
     case YES:
       updateChildrenVisible(n);
       break;

     case NO:
       hideChildren(n);
       break;

     case NOTDEF:
       assert(0);
     }
   }

	if(type(n) == DIALOG_)
  {
    switch (Check (v))
    {
      case NO:
        IupHide(n);
        break;
        
      case YES:
      case NOTDEF:
      {
        Ihandle *startfocus = IupGetAttributeHandle(n, "STARTFOCUS");
        if (startfocus)
          IupSetFocus(startfocus);
        else
          IupNextField(n);

        if(iupCheck(n, "_IUPWIN_SHOWING") != YES)
          IupShow(n);

        break;
      }
    }
  }
  else if(type(n) == ZBOX_ || type(n) == HBOX_ || type(n) == VBOX_)
  {
    iupSetEnv(n, "_IUPWIN_VISIBLE", v);
	}
  else if(type(n) == TOGGLE_)
  {
    switch (Check (v))
    {
      case NO:
        ShowWindow ((HWND)handle(n), SW_HIDE);
        break;
      case YES:
      case NOTDEF:
      default:
        ShowWindow ((HWND)handle(n), SW_SHOWNORMAL);
        {
          Ihandle* radio = iupGetRadio(n);
          if (radio)
          {
            if (startRadio)
            {
              startRadio = 0;
              firsttoggle = n;
            }

            if ( iupCheck(n,IUP_VALUE) )
              firsttoggle = NULL;
          }
        }
    }
  }
  else if(type(n) == LIST_)
  {
    switch (Check (v))
    {
      case NO:
      {
        char* vl = IupGetAttribute(n,IUP_VALUE);
        if (vl)
          iupStoreEnv(n,IUP_VALUE,vl);
        ShowWindow ((HWND)handle(n), SW_HIDE);
        break;
      }
      case YES:
      case NOTDEF:
      default:
      {
        /* inicializa opcoes da lista */
        char* vl = NULL;
        UINT setcursel;
        unsigned i;

        {
          int iscombobox = iupCheck(n, IUP_DROPDOWN)==YES||iupCheck(n, "EDITBOX")==YES? 1: 0;
          setcursel = (iscombobox)? CB_SETCURSEL: LB_SETCURSEL;
        }

        vl = IupGetAttribute (n, IUP_VALUE);

        if (vl)
        {
          int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;

          if (iupCheck(n, IUP_MULTIPLE) == YES)
          {
            for (i=0; i < strlen(vl); ++i)
              if (vl[i] == '+')
              SendMessage ((HWND)handle(n), LB_SETSEL, 1, i);
          }
          else if (!haseditbox)
          {
            sscanf (vl, "%d", &i);
            SendMessage ((HWND)handle(n), setcursel, i-1, 0L);
          }
        }
        ShowWindow ((HWND)handle(n), SW_SHOWNORMAL);
      }
    }
  }
	else if(type(n) == BUTTON_ || type(n) == LABEL_ || type(n) == MULTILINE_ || 
          type(n) == TEXT_   || type(n) == ITEM_  || type(n) == SUBMENU_   || 
          type(n) == CANVAS_ || type(n) == MENU_  || type(n) == FRAME_ )
  {
         switch (Check (v))
         {
            case NO:
               ShowWindow ((HWND)handle(n), SW_HIDE);
               break;
            case YES:
				case NOTDEF:
            default:
					ShowWindow ((HWND)handle(n), SW_SHOWNORMAL);
					if(type(n)==FRAME_ && child(n)!= NULL)
						SetWindowPos( (HWND)handle(child(n)), (HWND)handle(n), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
			}
  }
	else if(type(n) == RADIO_)
  {
      startRadio = 1;
      firsttoggle = NULL;
      setvisibility(child(n),v);

      /* if there is no toggle marked, mark the first one. */
      if (firsttoggle)
        SendMessage((HWND)handle(firsttoggle), BM_SETCHECK, BST_CHECKED, 0L);
  }
}

static void setvisible(Ihandle *n, char *v)
{
  switch(Check(v))
  {
  case NO:
    setvisibility(n, IUP_NO);
    break;

  case YES:
    if(IsHiddenByAncestor(n) != NO)
      setvisibility(n, IUP_YES);
    break;

  case NOTDEF:
    {
      Ihandle *parent = parent(n);

      if(parent != NULL)
        setvisible(n, iupGetEnv(parent, IUP_VISIBLE));
      else
        setvisible(n, IUP_YES);
    }
    break;
  }
}

static void setmask (Ihandle* n, char* v)
{
   if (type(n) == TEXT_)
   {
      ImaskElem* mask;
      ITextInfo* txtinfo = NULL;
      
      mask = iupCreateMask(v);
		txtinfo =(ITextInfo*) IupGetAttribute(n,"_IUPWIN_TXTINFO__");

      if (mask)
      {
         char* val;
         char fill[30];

         IupSetAttribute(n, "_IUPWIN_TXTINFO__", NULL);
         if (!txtinfo)
         {
            txtinfo = (ITextInfo*)malloc(sizeof(ITextInfo));
            if (!txtinfo)
               return;
         }
         val = IupGetAttribute(n, IUP_VALUE);
         txtinfo->mask = mask;
         txtinfo->status = iupCheckMask(mask, val, fill);
         if (txtinfo->status == 0)
         {
            txtinfo->status = iupCheckMask(mask, "", fill);
            IupSetAttribute(n,IUP_VALUE,fill);
         }
         else if (fill[0])
         {
            char txt[50];
            txt[0] = 0;
            if (val)
               strcat(txt,val);
            strcat(txt,fill);
            IupSetAttribute(n, IUP_VALUE, txt);
         }
         IupSetAttribute(n, "_IUPWIN_TXTINFO__", (char*)txtinfo);
      }
      else
      {
         if (txtinfo)
         {
            free(txtinfo);
            IupSetAttribute(n,"_IUPWIN_TXTINFO__",NULL);
         }
      }
   }
}

static void settip(Ihandle* n, char* v)
{
   iupwinTipsSet(n,v);
}

static void settabsize (Ihandle* n, char* v)
{
   int tabsize;

   if (handle(n) == NULL || type(n)!=MULTILINE_)
      return;

   if (v == NULL)
      SendMessage((HWND)handle(n), EM_SETTABSTOPS, (WPARAM)0L, (LPARAM)NULL);
   else
   {
     sscanf(v,"%i",&tabsize);
     tabsize *= 4;
     SendMessage((HWND)handle(n), EM_SETTABSTOPS, (WPARAM)1L, (LPARAM)&tabsize);
   }
}

static void readRowCol( char* v, int* row, int* col)
{
   char* irow = iupStrDup(v);
   char* icol;
   int i;

   for (i=0; irow[i] != 0 && irow[i]!=','; i++);
   
   if (irow[i] == ',')
   {
      icol = irow + i + 1;
      irow[i] = 0;
      sscanf(irow,"%i", row);
      sscanf(icol,"%i", col);
   }
   free(irow);
}

static void setcaret(Ihandle* n, char* v)
{
  if (handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_) )
  {
    if (type(n) == TEXT_ || type(n) == LIST_)
    {
      int pos;
      sscanf(v,"%i",&pos);
      SetFocus((HWND)handle(n));
      if (type(n) == LIST_)
      {
        LPARAM dwPos = MAKELPARAM(pos-1,pos-1);
        SendMessage((HWND)handle(n), CB_SETEDITSEL, 0, dwPos);
      }
      else
        SendMessage((HWND)handle(n), EM_SETSEL, (WPARAM)pos-1, (LPARAM)pos-1);
    }
    else
    {
      int n_lines, pos, length;
      int row = 1, col = 1;

      readRowCol(v, &row, &col);

      n_lines = SendMessage((HWND)handle(n), EM_GETLINECOUNT, 0L, 0L);
      if (row > n_lines)
      {
        row = n_lines;
        col = -1;
      }

      pos = SendMessage((HWND)handle(n), EM_LINEINDEX, (WPARAM)row-1, 0L);
      length = SendMessage((HWND)handle(n), EM_LINELENGTH, (WPARAM)pos, 0L);
      if (col == -1 || col > length)
        col = length + 1;

      pos = pos + col - 1;
      SetFocus((HWND)handle(n));
      SendMessage((HWND)handle(n), EM_SETSEL, (WPARAM)pos, (LPARAM)pos);
      SendMessage((HWND)handle(n), EM_SCROLLCARET, 0L, 0L);
    }
  }
}

static void setappend (Ihandle* n, char* v)
{
	if (n && (type(n)==MULTILINE_ || type(n)==TEXT_ || type(n)==LIST_))
	{
		int s1=0, s2=0;
		char* text;
		char* value;
    char* tmp = NULL;
    if(v)
      tmp = iupStrDup(v); /* Let's avoid memory problems with IupGetAttribute below */
    
    value = IupGetAttribute(n, IUP_VALUE);

		if (tmp)
			s1 = strlen(tmp);
		if (value)
			s2 = strlen(value);

		text = (char*) malloc( sizeof(char) * (s1 + s2 + 2) );

		if (value)
			strcpy(text, value);
		else
			text[0] = 0;

    if (type(n)==MULTILINE_)
      strcat(text, "\n");

		if (tmp)
		  strcat(text, tmp);

		IupStoreAttribute(n, IUP_VALUE, text);
    free(text);
    free(tmp);
	}
}

static void setreadonly(Ihandle* n, char* v)
{
  if (handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_) )
  {
    HWND hWnd = (HWND)handle(n);
    BOOL value = FALSE;
    if(v && iupStrEqualNoCase(v, "YES"))
      value = TRUE;

    if (type(n) == LIST_)
      hWnd = (HWND)IupGetAttribute(n, "_IUPWIN_EDITBOX");

    SendMessage(hWnd, EM_SETREADONLY, (WPARAM)value, 0);
  }
}

static void setinsert(Ihandle* n, char* v)
{
  if (handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_) )
  {
    int to_free = 0;
    HWND hWnd = (HWND)handle(n);
    char* str = iupwinTransMultiline(v,&to_free);

    if (type(n) == LIST_)
      hWnd = (HWND)IupGetAttribute(n, "_IUPWIN_EDITBOX");

    SendMessage(hWnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)str);

    if (to_free)
      free(str);
  }
}


/****************************************************
 *                       FONTS                      *
 ****************************************************/

static void setHFONT(Ihandle *h, HFONT font)
{
  assert(h);
  assert(handle(h));
  assert(font);

  IupSetAttribute(h, "HFONT", (char *) font);
  SendMessage(handle(h), WM_SETFONT, (WPARAM) font, MAKELPARAM(TRUE,0));
}

IwinFontInfo iupwin_fontinfo = { NULL, 0, 0 };

static void win_setfont(Ihandle *h, char *typeface, int height, BOOL is_bold,
                        BOOL is_italic, BOOL is_underline,
                        BOOL is_strikeout)
{
  HFONT new_font;
  
  if(!iupwin_fontinfo.fontlist)
  {
    iupwin_fontinfo.fontlist = (IwinFont *) malloc(2*sizeof(IwinFont));
    if(!iupwin_fontinfo.fontlist)
      return;

    iupwin_fontinfo.max_num_winfonts=2;
  }
  else  /* verifica se fonte ja existe */
  {
    int c;

    for(c=0; c < iupwin_fontinfo.num_winfonts; c++)
    {
      if(iupStrEqual(typeface, iupwin_fontinfo.fontlist[c].typeface) &&
         height == iupwin_fontinfo.fontlist[c].height &&
         is_bold == iupwin_fontinfo.fontlist[c].is_bold &&
         is_italic == iupwin_fontinfo.fontlist[c].is_italic &&
         is_underline == iupwin_fontinfo.fontlist[c].is_underline &&
         is_strikeout == iupwin_fontinfo.fontlist[c].is_strikeout)
      {
        setHFONT(h, iupwin_fontinfo.fontlist[c].font);
        return;
      }
    }
  }

  /* fonte nao existia, cria-a */

  if(iupwin_fontinfo.num_winfonts ==
    iupwin_fontinfo.max_num_winfonts) /* verifica se e' necessario 
                                      alocar mais memoria */
  {
    IwinFont *new_winfonts=NULL;

    /* se for, aloca o dobro da memoria ja' alocada */ 

    new_winfonts = (IwinFont *)
      malloc(2*iupwin_fontinfo.max_num_winfonts*sizeof(IwinFont));

    if(!new_winfonts)
      return;

    memcpy(new_winfonts, iupwin_fontinfo.fontlist,
      iupwin_fontinfo.num_winfonts*sizeof(IwinFont));

    iupwin_fontinfo.max_num_winfonts *= 2;
    free(iupwin_fontinfo.fontlist);

    iupwin_fontinfo.fontlist = new_winfonts;
  }

  {  /* cria fonte pedida */
    int height_pixels;
    
    {
      HDC dc = GetDC(handle(h));
      double res = (double)GetDeviceCaps(dc,LOGPIXELSY)/72.;
      height_pixels = -(int)(height*res + 0.5);
      ReleaseDC(handle(h),dc);
      assert(height_pixels != 0);
      if(height_pixels == 0)
        return;
    }

    new_font = CreateFont(height_pixels,
      0,0,0,
      (is_bold) ? FW_BOLD : FW_NORMAL,
      is_italic,
      is_underline,
      is_strikeout,
      DEFAULT_CHARSET,OUT_TT_PRECIS,
      CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
      FF_DONTCARE|DEFAULT_PITCH,typeface);

  }

  if(!new_font)
    return;
  
  strcpy(iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].typeface, typeface);
  iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].height = height;
  iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].is_bold = is_bold;
  iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].is_italic = is_italic;  
  iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].is_underline = is_underline;
  iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].is_strikeout = is_strikeout;
  iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].font = new_font;

  setHFONT(h, iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts].font);

  iupwin_fontinfo.num_winfonts++;
}

static void iupFontParse(char *v, char *fontname, int *height, int *bold, int *italic, int *underline, int *strikeout)
{
  int c;

  if (!v) return;

  if (v[0] == ':')  /* check if it has the typeface */
    v++;       /* jump separator */
  else
  {
    c = (int)strcspn(v, ":");      /* extract typeface */
    strncpy(fontname, v, c);
    fontname[c]='\0';
    v += c+1;  /* jump typeface and separator */
  }

  if (v[0] == ':')  /* check if it has attributes */
    v++;       /* jump separator */
  else
  {
    do /* extract style (bold/italic etc) */
    {
      char style[30];

      c = (int)strcspn(v, ":,");
      if (c == 0)
        break;

      strncpy(style, v, c);
      style[c] = '\0';

      if(!strcmp(style, "BOLD"))
        *bold = 1; 
      else if(!strcmp(style,"ITALIC"))
        *italic = 1; 
      else if(!strcmp(style,"UNDERLINE"))
        *underline = 1; 
      else if(!strcmp(style,"STRIKEOUT"))
        *strikeout = 1; 

      v += c; /* jump only the attribute */

      if(v[0] == ':')  /* terminou lista de atributos */
      {
        v++;
        break;
      }

      v++;   /* jump separator */
    } while (v[0]);
  }

  if (v[0] != 0)  /* check if it has the size */
  {
    /* extract height */
    sscanf(v, "%d", height);
  }
}

static void setwinfont(Ihandle* n, char *v)
{
  char typeface[50] = "";
  int height = 8;
  int is_bold = 0,
        is_italic = 0, 
        is_underline = 0,
        is_strikeout = 0;
  HWND hwnd = NULL;
    
  assert(n != NULL);
  if(n == NULL)
    return;
  
  hwnd = (HWND)handle(n);

  if (type(n) != BUTTON_    &&
      type(n) != CANVAS_    && type(n) != DIALOG_ && 
      type(n) != LABEL_     && type(n) != LIST_   &&
      type(n) != TEXT_      && type(n) != TOGGLE_ &&
      type(n) != MULTILINE_ && type(n) != FRAME_)
    return;

  /* parse the new attributes */
  iupFontParse(v, typeface, &height, &is_bold, &is_italic, &is_underline, &is_strikeout);

  if (hwnd)
    win_setfont(n, typeface, height, is_bold, is_italic, is_underline, is_strikeout);

  /* update both attributes */
  iupStoreEnv(n, "_IUPWIN_FONT", v);
  iupStoreEnv(n, "FONT",  v);
}

#define IUP_NUM_FONT 36

static struct
{
  char Iup_name[50];
  char font[70];
}
IupFonts[IUP_NUM_FONT] = 
{
  { IUP_HELVETICA_NORMAL_8, "Arial::8"},
  { IUP_HELVETICA_ITALIC_8, "Arial:ITALIC:8"},
  { IUP_HELVETICA_BOLD_8,   "Arial:BOLD:8"},
  { IUP_HELVETICA_NORMAL_10,"Arial::10" },
  { IUP_HELVETICA_ITALIC_10,"Arial:ITALIC:10"},
  { IUP_HELVETICA_BOLD_10,  "Arial:BOLD:10"},
  { IUP_HELVETICA_NORMAL_12,"Arial::12"},
  { IUP_HELVETICA_ITALIC_12,"Arial:ITALIC:12"},
  { IUP_HELVETICA_BOLD_12,  "Arial:BOLD:12"},
  { IUP_HELVETICA_NORMAL_14,"Arial::14"},
  { IUP_HELVETICA_ITALIC_14,"Arial:ITALIC:14"},
  { IUP_HELVETICA_BOLD_14,  "Arial:BOLD:14"},
  { IUP_COURIER_NORMAL_8,   "Courier New::8"},
  { IUP_COURIER_ITALIC_8,   "Courier New:ITALIC:8"},
  { IUP_COURIER_BOLD_8,     "Courier New:BOLD:8"},
  { IUP_COURIER_NORMAL_10,  "Courier New::10"},
  { IUP_COURIER_ITALIC_10,  "Courier New:ITALIC:10"},
  { IUP_COURIER_BOLD_10,    "Courier New:BOLD:10"},
  { IUP_COURIER_NORMAL_12,  "Courier New::12"},
  { IUP_COURIER_ITALIC_12,  "Courier New:ITALIC:12"},
  { IUP_COURIER_BOLD_12,    "Courier New:BOLD:12"},
  { IUP_COURIER_NORMAL_14,  "Courier New::14"},
  { IUP_COURIER_ITALIC_14,  "Courier New:ITALIC:14"},
  { IUP_COURIER_BOLD_14,    "Courier New:BOLD:14"},
  { IUP_TIMES_NORMAL_8,     "Times New Roman::8"},
  { IUP_TIMES_ITALIC_8,     "Times New Roman:ITALIC:8"},
  { IUP_TIMES_BOLD_8,       "Times New Roman:BOLD:8"},
  { IUP_TIMES_NORMAL_10,    "Times New Roman::10"},
  { IUP_TIMES_ITALIC_10,    "Times New Roman:ITALIC:10"},
  { IUP_TIMES_BOLD_10,      "Times New Roman:BOLD:10"},
  { IUP_TIMES_NORMAL_12,    "Times New Roman::12"},
  { IUP_TIMES_ITALIC_12,    "Times New Roman:ITALIC:12"},
  { IUP_TIMES_BOLD_12,      "Times New Roman:BOLD:12"},
  { IUP_TIMES_NORMAL_14,    "Times New Roman::14"},
  { IUP_TIMES_ITALIC_14,    "Times New Roman:ITALIC:14"},
  { IUP_TIMES_BOLD_14,      "Times New Roman:BOLD:14"}
};

char *IupUnMapFont(const char *driverfont)
{
  int c=0;

  for(c = 0; c < IUP_NUM_FONT; c++)
    if(iupStrEqual(driverfont, IupFonts[c].font))
      return IupFonts[c].Iup_name;

  return NULL; /* nome fornecido nao era de uma fonte iup */

}

char *IupMapFont(const char *iupfont)
{
  int c=0;

  for(c = 0; c < IUP_NUM_FONT; c++)
    if(iupStrEqual(iupfont, IupFonts[c].Iup_name))
      return IupFonts[c].font;

  return NULL; /* nome fornecido nao era de uma fonte iup */
}

static void setfont(Ihandle* n, char *v)
{
  if (v)
  {
    /* FONT can be a IUP Font or a Native font */
    char* winfont = IupMapFont(v);
    if (winfont)
      setwinfont(n, winfont);
    else
      setwinfont(n, v);
  }
  else
  {
    char *default_font = IupGetGlobal("DEFAULTFONT");
    setwinfont(n, default_font);
  }
}

/* called when a control is created */
void iupwinUpdateFont(Ihandle *h)
{
  char* winfont = iupGetEnv(h,"_IUPWIN_FONT");
  if (winfont)
    setwinfont(h, winfont);
  else
  {
    char* font = iupGetEnv(h,"FONT");
    setfont(h, font);
  }
}

/********************************************************/
/********************************************************/


/* Altera icone de um dialogo */

static void seticon(Ihandle* n, char *v)
{
  HWND hwnd=NULL;

  if (n == NULL)
    return;

  hwnd = (HWND)handle(n);
	 
  if (hwnd == NULL || type(n) != DIALOG_)
	 return;

  if (v == NULL)
  {
	  SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)NULL);
  }
  else
  {
    HICON icon = iupwinGetIcon(v, hwnd, IUP_ICON);
    if (icon == NULL)
      icon = LoadIcon(NULL, IDI_APPLICATION);

    SendMessage(hwnd, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM)icon);    
  }

  if (IsIconic(hwnd))
    RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_ERASE 
                                 | RDW_INVALIDATE | RDW_ERASENOW);
}

/*
 * Torna o dialogo indicado por dlg
 * uma janela TOPMOST ou uma janela
 * normal
 */

void settopmost(Ihandle *elem, char *value)
{
  HWND hwnd = NULL;

  /* parametro value nao e' usado */
  value = NULL;

  /* validacao de parametros */
  assert(elem != NULL);
  if(elem == NULL)
    return;

  /* verifica se e' um dialogo 
    (necessario devido `a heranca
    "forcada" em iupwinSet) */
  if(type(elem) != DIALOG_)
    return;

  hwnd = handle(elem);
  if(hwnd == NULL)
    return;
  
  if (iupCheck(elem, IUP_TOPMOST) == YES)
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
  else
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

  return;
}

static int rowcol2pos(Ihandle *n, int row, int col)
{
  int length=-1, pos=-1;
  assert(n);
  if(n==NULL) return 0;
  assert(row>=1 && col>=1);
  if(row<1 || col<1) return 0;
    
  pos = SendMessage((HWND)handle(n), EM_LINEINDEX,  (WPARAM)row-1, 0L);
  if(pos<0) /* Numero da linha é maior do que o maximo, ajustar para o maximo */
  {
    row = SendMessage((HWND)handle(n), EM_GETLINECOUNT,  (WPARAM)row-1, 0L);
    pos = SendMessage((HWND)handle(n), EM_LINEINDEX,     (WPARAM)row-1, 0L);
  }
  length = SendMessage((HWND)handle(n), EM_LINELENGTH, (WPARAM)pos,   0L);

  /* Ultimo elemento da linha vem antes do fim da linha */
  if (col > length)
      col = length + 1;

  pos = pos + col - 1;
  return pos;
}

static void setselection (Ihandle *n, char *v)
{
  int test=0;

  assert(n);
  if(n==NULL) 
    return;
  assert(v);
  if(v==NULL) 
    return;

  if (handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || type(n) == LIST_) )
  {
    if (type(n) == TEXT_ || type(n) == LIST_)
    {
      int start=1, end=1;

      if (iupStrToIntInt(v, &start, &end, ':')!=2) 
        return;

      if(start<1 || end<1) 
        return;

      if (type(n) == LIST_)
      {
        LPARAM wdPos = MAKELPARAM(start-1, end-1);
        SendMessage(handle(n), CB_SETEDITSEL, 0 , wdPos);
      }
      else
        SendMessage(handle(n), EM_SETSEL, (WPARAM)(start-1), (LPARAM)(end-1));

      SetFocus(handle(n));
    }
    else
    {
      int rowS=1, colS=1, rowE=1, colE=1, posS=1, posE=1;

      test = sscanf(v, "%d,%d:%d,%d", &rowS, &colS, &rowE, &colE);
      assert(test==4); /* Formato passado errado */
      if(test!=4) return;

      assert(rowS>=1 && colS>=1 && rowE>=1 && colE>=1);
      if(rowS<1 || colS<1 || rowE<1 || colE<1) 
        return;

      posS = rowcol2pos(n, rowS, colS);
      posE = rowcol2pos(n, rowE, colE);
      SendMessage(handle(n), EM_SETSEL, (WPARAM)posS, (LPARAM)posE);

      SetFocus(handle(n));
    }
  }
}

/********************************************************************
** Sets a previous selected text to a new value, replacing it.
********************************************************************/
static void setselectedtext(Ihandle *n, char *text)
{
  long style = 0;
  unsigned int i;

  if(!n || (type(n) != TEXT_ && type(n) != MULTILINE_ && type(n) != LIST_))
    return;

  /* Reads window style */
  style = GetWindowLong(handle(n), GWL_STYLE);
  if(style & ES_READONLY) /* readonly was set, overiding it */
    SendMessage(handle(n), EM_SETREADONLY, (WPARAM)(BOOL) FALSE, 0);

  for(i=0; i<strlen(text); i++) /* Simulates the user writing the text */
    SendMessage(handle(n), WM_CHAR,    (TCHAR) text[i], 0);

  /* readonly was set, and now should be set again */
  if(style & ES_READONLY) 
    SendMessage(handle(n), EM_SETREADONLY, (WPARAM)(BOOL) TRUE, 0);

  return;
}

/********************************************************************
** Resets dialog's size whenever the alignment changes (alignment 
** change should change element's position.)
********************************************************************/
static void setalignment(Ihandle *n, char *attr)
{
  if ((handle(n) != NULL) && (type(n) == HBOX_ || type(n) == VBOX_ || type(n) == ZBOX_))
  {
    IupRefresh(n);
  }
}

static void setbringfront(Ihandle *n, char *v)
{
  if (type(n) != DIALOG_ || handle(n) == NULL || !v)
    return;

  if(iupStrEqualNoCase(v, "YES"))
    SetForegroundWindow((HWND)handle(n));
}

static BOOL TrayMessage(HWND hWnd, DWORD dwMessage, HICON hIcon, PSTR pszTip)
{
  BOOL res;
  NOTIFYICONDATA tnd;
  memset(&tnd, 0, sizeof(NOTIFYICONDATA));

  tnd.cbSize	= sizeof(NOTIFYICONDATA);
  tnd.hWnd		= hWnd;
  tnd.uID			= ID_TRAYICON;

  if(dwMessage == NIM_ADD)
  {
    tnd.uFlags = NIF_MESSAGE;
    tnd.uCallbackMessage = WM_TRAY_NOTIFICATION;
  }
  else if(dwMessage == NIM_MODIFY)
  {
    if(hIcon)  
    {
      tnd.uFlags |= NIF_ICON;
      tnd.hIcon = hIcon;
    }
    
    if(pszTip) 
    {
      tnd.uFlags |= NIF_TIP;
      lstrcpyn(tnd.szTip, pszTip, sizeof(tnd.szTip));
    }
  }
  res = Shell_NotifyIcon(dwMessage, &tnd);
  assert(res == TRUE);
  return res;
}

static void setseparator(Ihandle *n, char *v)
{
  if(n && type(n) == LABEL_)
  {
    if (iupStrEqual(v, IUP_HORIZONTAL))
    {
      iupSetEnv(n, IUP_RASTERSIZE, "x2");
      iupSetEnv(n, IUP_EXPAND, IUP_HORIZONTAL);
    }
    else if (iupStrEqual(v, IUP_VERTICAL))
    {
      iupSetEnv(n, IUP_RASTERSIZE, "2x");
      iupSetEnv(n, IUP_EXPAND, IUP_VERTICAL);
    }
    else
    {
      iupSetEnv(n, IUP_RASTERSIZE, NULL);
      iupSetEnv(n, IUP_EXPAND, NULL);
    }
  }
}

static void sethidetaskbar(Ihandle *n, char *v)
{
  if(n && type(n) == DIALOG_ && handle(n))
  {
    HWND hwnd = handle(n);
    if(v && iupStrEqualNoCase(v, "YES"))
      SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_HIDEWINDOW);
    else
      SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
  }
}

static void settray(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == DIALOG_)
  {
    HWND hwnd = handle(n);
    if(v && iupStrEqualNoCase(v, IUP_YES))
      TrayMessage(hwnd, NIM_ADD, NULL, NULL);
    else
      TrayMessage(hwnd, NIM_DELETE, NULL, NULL);
  }
}
static void settraytip(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == DIALOG_)
    TrayMessage(handle(n), NIM_MODIFY, NULL, v);
}

static void settrayimage(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == DIALOG_)
  {
    HWND hwnd = handle(n);
    TrayMessage(hwnd, NIM_MODIFY, iupwinGetIcon(v, hwnd, IUP_ICON), NULL);
  }
}

static void setshowdropdown(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == LIST_ && v)
  {
    if (iupCheck(n, IUP_DROPDOWN)==YES)
    {
      BOOL val = iupStrEqualNoCase(v, IUP_YES)? TRUE: FALSE;
      SendMessage((HWND)handle(n), CB_SHOWDROPDOWN, val, 0); 
    }
  }
}

/* define this here, so we do not need to define _WIN32_WINNT=0x0500 */
#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#endif

#ifndef LWA_ALPHA
#define LWA_ALPHA               0x00000002
#endif

typedef BOOL (WINAPI*winSetLayeredWindowAttributes)(
    HWND hwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags);

static void setlayered(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == DIALOG_)
  {
    if (v && iupStrEqualNoCase(v, IUP_YES))
      SetWindowLong((HWND)handle(n), GWL_EXSTYLE, (LONG)GetWindowLongPtr((HWND)handle(n), GWL_EXSTYLE) | WS_EX_LAYERED);
    else
    {
      SetWindowLong((HWND)handle(n), GWL_EXSTYLE, (LONG)GetWindowLongPtr((HWND)handle(n), GWL_EXSTYLE) & ~WS_EX_LAYERED);
      RedrawWindow((HWND)handle(n), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }
  }
}

static void setlayeralpha(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == DIALOG_)
  {
    if (iupCheck(n, "LAYERED")==YES)
    {
      int alpha = atoi(v);
      static winSetLayeredWindowAttributes mySetLayeredWindowAttributes = NULL;
      if (!mySetLayeredWindowAttributes)
      {
        HMODULE hinstDll = LoadLibrary("user32.dll");
        if (hinstDll)
          mySetLayeredWindowAttributes = (winSetLayeredWindowAttributes)GetProcAddress(hinstDll, "SetLayeredWindowAttributes");
      }

      if (mySetLayeredWindowAttributes)
        mySetLayeredWindowAttributes((HWND)handle(n), 0, (BYTE)alpha, LWA_ALPHA);
    }
  }
}

static void setmdiarrange  (Ihandle *n, char *v)
{
  Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
  if (client)
  {
    UINT msg;
    WPARAM wp = 0;

    if (iupStrEqual(v, "TILEHORIZONTAL"))
    {
      msg = WM_MDITILE;
      wp = MDITILE_HORIZONTAL;
    }
    else if (iupStrEqual(v, "TILEVERTICAL"))
    {
      msg = WM_MDITILE;
      wp = MDITILE_VERTICAL;
    }
    else if (iupStrEqual(v, "CASCADE"))
      msg = WM_MDICASCADE;
    else
     msg = WM_MDIICONARRANGE;

    SendMessage((HWND)handle(client), msg, wp, 0);
  }
}

static void setmdiactivate(Ihandle *n, char *v)
{
  Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
  if (client)
  {
    Ihandle* child = IupGetHandle(v);
    if (child)
      SendMessage((HWND)handle(client), WM_MDIACTIVATE, (WPARAM)handle(child), 0);
    else
    {
      HWND hchild = (HWND)SendMessage((HWND)handle(client), WM_MDIGETACTIVE, 0, 0);
      if (iupStrEqual(v, "NEXT"))
        SendMessage((HWND)handle(client), WM_MDINEXT, (WPARAM)hchild, TRUE);
      else
        SendMessage((HWND)handle(client), WM_MDINEXT, (WPARAM)hchild, FALSE);
    }
  }
}

static void setmdicloseall(Ihandle *n, char *v)
{
  Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
  if (client)
    iupwinCloseChildren(client);
}

static void setzorder(Ihandle *n, char *v)
{
  if (iupStrEqualNoCase(v, "TOP"))
    SetWindowPos((HWND)handle(n), HWND_TOP, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
  else
    SetWindowPos((HWND)handle(n), HWND_BOTTOM, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

static void setclipboard(Ihandle *n, char *v)
{
  UINT msg = 0;

  if (iupStrEqualNoCase(v, "COPY"))
    msg = WM_COPY;
  else if (iupStrEqualNoCase(v, "CUT"))
    msg = WM_CUT;
  else if (iupStrEqualNoCase(v, "PASTE"))
    msg = WM_PASTE;

  if (msg)
    SendMessage((HWND)handle(n), msg, 0, 0);
}
