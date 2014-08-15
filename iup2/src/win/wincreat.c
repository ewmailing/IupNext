/** \file
 * \brief Windows Driver Controls creation
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>    /* sprintf */
#include <windows.h>
#include <math.h>
#include <assert.h>

#include "iglobal.h"
#include "idrv.h"
#include "win.h"
#include "wintips.h"
#include "wincreat.h"
#include "winproc.h"
#include "winhandle.h"
#include "winicon.h"
#include "winbutton.h"
#include "wintoggle.h"
#include "winframe.h"

#ifndef WS_EX_COMPOSITED
#define WS_EX_COMPOSITED        0x02000000L
#endif

/* global variables */
HINSTANCE iupwin_hinstance = NULL;

/* local variables */
static int iupwin_comctl32ver6 = 0;
static HWND iupwin_parent_hwnd = NULL;
static Ihandle* iupwin_parent = NULL;

/* prototipos */
static void assignidIhandle(Ihandle *h);

#define MENUINITIALNUMBER 10 /* Start with 10 because then we know that lower numbers
                             ** are not iupwin_menus (necessary because of DEFAULTENTER and
                             ** DEFAULTESC. */
#define NUM_MAX_MENUS 60000
static  Ihandle* iupwin_menus[NUM_MAX_MENUS];

#define INITIALNUMBER 50
static  unsigned int iupwin_child_id = INITIALNUMBER; /* control identifier */

#define IUPWIN_SINGLELINE 0
#define IUPWIN_MULTILINE  1


int iupwinUseComCtl32Ver6(void)
{
  return iupwin_comctl32ver6;
}

void iupwinShowLastError(void)
{
  int error = GetLastError();
  if (error)
  {
    LPVOID lpMsgBuf;
    FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      error,
      0,
      (LPTSTR) &lpMsgBuf,
      0,
      NULL 
    );
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
    LocalFree( lpMsgBuf );
  }
}

static void sethandle( Ihandle* h, HWND hwnd )
{
  assert(hwnd);
#if _TEST_CODE_ 
  if(hwnd == NULL)
    iupwinShowLastError();
#endif
  handle(h) = hwnd;
}

static int iupEnvCheck(Ihandle *n, char *a)
{
 char* v = iupGetEnv (n, a);
 if (v == NULL)
   v = iupdrvGetDefault(n, a);
 if (v == NULL)
  return NOTDEF;
 else if (iupStrEqualNoCase(v, IUP_NO) || iupStrEqualNoCase(v, IUP_OFF))
  return NO;
 else if (iupStrEqualNoCase(v, IUP_YES) || iupStrEqualNoCase(v, IUP_ON))
  return YES;
 return NOTDEF;
}

static char* GetDialogClass(Ihandle* h, int mdi)
{
  static int dialogcount = 0;
  char* name = (char*) malloc(sizeof(char)*30);
  WNDCLASS wndclass;
  ZeroMemory(&wndclass, sizeof(WNDCLASS));
  
  /* each IupDialog has a different class 
     so we can change cursor and background */

  if (mdi == 2)
    sprintf( name, "IUPChildDlg%i", dialogcount );
  else if (mdi == 1)
    sprintf( name, "IUPFrameDlg%i", dialogcount );
  else
    sprintf( name, "IUPDlg%i", dialogcount );
  dialogcount++;

  wndclass.hInstance      = iupwin_hinstance;
  wndclass.lpszClassName  = name;
  wndclass.lpfnWndProc    = (WNDPROC)iupwinDialogProc;
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.style          = 0; 
  wndclass.hbrBackground  = (HBRUSH)(atoi(IupGetGlobal("_IUPWIN_DLGBGCOLOR"))+1);

  if (iupCheck(h,"SAVEUNDER"))  /* default = YES */
    wndclass.style |= CS_SAVEBITS;

  if (iupCheck(h, IUP_CONTROL) == YES) /* default = NO */
    wndclass.style |=  CS_HREDRAW | CS_VREDRAW;
    
  RegisterClass(&wndclass);

  return name;
}

static char* GetCanvasClass(void)
{
  static int canvascount = 0;
  char* name = (char*) malloc(sizeof(char)*20);
  WNDCLASS wndclass;
  ZeroMemory(&wndclass, sizeof(WNDCLASS));

  sprintf( name, "IUPCnv%i", canvascount );
  canvascount ++;

  /* using CS_OWNDC will minimize the work of cdActivate in the CD library */

  wndclass.hInstance      = iupwin_hinstance;
  wndclass.lpszClassName  = name;
  wndclass.lpfnWndProc    = (WNDPROC)iupwinCanvasProc;
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.style          = CS_DBLCLKS | CS_OWNDC;
  wndclass.hbrBackground  = NULL;  /* remove the background to optimize redraw */

  RegisterClass(&wndclass);

  return name;
}

static void InitColors(Ihandle *n, RGBQUAD* colors, int pal_count, int *transp_index)
{
  int i;
  char *value;
  char attr[6];

  static struct{unsigned char r, g, b;} default_colors[16] = {
    { 0,0,0 },     
    { 128,0,0 },
    { 0,128,0 },    
    { 128,128,0 },    
    { 0,0,128 },    
    { 128,0,128 },    
    { 0,128,128 },    
    { 192,192,192 },    
    { 128,128,128 },    
    { 255,0,0 },
    { 0,255,0 },
    { 255,255,0 },
    { 0,0,255 },
    { 255,0,255 },
    { 0,255,255 },
    { 255,255,255 }};

  *transp_index = -1;

  for (i=0;i<16;i++)
  {
    sprintf( attr, "%d", i );
    value = IupGetAttribute(n, attr);

    if(value != NULL)
    {
      unsigned int red, green, blue;

      if(iupStrEqual(value,"BGCOLOR"))
      {
        /* retrieve the background color */
        value = IupGetAttribute(n, IUP_BGCOLOR);
        if (*transp_index == -1) *transp_index = i;
      }

      if (!iupGetRGB(value, &red, &green, &blue))
      {
        colors[i].rgbRed   = default_colors[i].r;
        colors[i].rgbGreen = default_colors[i].g;
        colors[i].rgbBlue  = default_colors[i].b;
      }
      else
      {
        colors[i].rgbRed   = (BYTE)red;
        colors[i].rgbGreen = (BYTE)green;
        colors[i].rgbBlue  = (BYTE)blue;
      }
    }
    else
    {
      colors[i].rgbRed   = default_colors[i].r;
      colors[i].rgbGreen = default_colors[i].g;
      colors[i].rgbBlue  = default_colors[i].b;
    }

    colors[i].rgbReserved = 0;
  }

  for (;i<pal_count;i++)
  {
    unsigned int red = 0, green = 0, blue = 0;
    sprintf( attr, "%d", i );
    value = IupGetAttribute(n, attr);

    iupGetRGB(value, &red, &green, &blue);
    colors[i].rgbRed   = (BYTE)red;
    colors[i].rgbGreen = (BYTE)green;
    colors[i].rgbBlue  = (BYTE)blue;
  }

  /* If cursor and no transparency defined, assume 0 is transparent. */
  if(IupGetAttribute(n, IUP_HOTSPOT) != NULL && *transp_index == -1)
  {
    *transp_index = 0;
    colors[0].rgbRed   = 0;
    colors[0].rgbGreen = 0;
    colors[0].rgbBlue  = 0;
  }

  IupSetfAttribute(n, "_IUPWIN_TRANSP_INDEX", "%d", *transp_index);
}

static void CreateImage (Ihandle *n)
{
  int i,j;
  int width = IupGetInt(n,IUP_WIDTH);
  int height = IupGetInt(n,IUP_HEIGHT);
  int line_size, mask_line;
  unsigned char *data = (unsigned char*)image_data(n);
  int transp_index;

  IwinBitmap* bmp;
  BYTE* aImage;
  BYTE* aMask;
  int pal_count = 16;
  int bpp = 4;

  bmp  = (IwinBitmap*) malloc(sizeof(IwinBitmap));
  handle(n) = bmp;

  for (i=16;i<256;i++)
  {
    char attr[6];
    sprintf( attr, "%d", i );
    if (!IupGetAttribute(n, attr))
      break;
    pal_count++;
  }

  if (pal_count > 16)
    bpp = 8;

  bmp->bmpinfo = malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*pal_count);

  {
    BITMAPINFOHEADER* bmpheader = (BITMAPINFOHEADER*)bmp->bmpinfo;
    memset(bmpheader, 0, sizeof(BITMAPINFOHEADER));
    bmpheader->biSize = sizeof(BITMAPINFOHEADER);
    bmpheader->biWidth  = width;
    bmpheader->biHeight = height;
    bmpheader->biPlanes = 1;
    bmpheader->biBitCount = (WORD)bpp;
    bmpheader->biCompression = BI_RGB;
    bmpheader->biClrUsed = pal_count;
  }

  InitColors(n, (RGBQUAD*)((BYTE*)bmp->bmpinfo + sizeof(BITMAPINFOHEADER)), pal_count, &transp_index);

  line_size = ((width * bpp + 31) / 32) * 4;   /* DWORD aligned, 4 bytes boundary in a N bpp image */
  mask_line = ((width * 1 + 15) / 16) * 2;     /* WORD aligned, 2 bytes boundary in a 1 bpp image */

  bmp->bitmap = malloc(height * line_size);
  bmp->bitmask = malloc(height * mask_line);

  memset(bmp->bitmap, 0, height * line_size);
  memset(bmp->bitmask, 0, height * mask_line);

  aImage = (BYTE*)bmp->bitmap;   /* windows bitmaps are bottom up */
  aMask = (BYTE*)bmp->bitmask;  

  aMask += (height-1)*mask_line; /* mask is top down */
  data += (height-1)*width;      /* iupimage is top down */

  /* for cursors and icons */
  /* destination = (destination AND bitmask) XOR bitmap */

  for (i=0;i<height;i++)
  {
    for(j=0;j<width;j++)
    {
      unsigned char pixel = data[j];

      /* seta bit na mascara */
      if(transp_index != -1 && transp_index == pixel)
      {
        aMask[j/8] |= 1 << (7 - (j % 8)); /* set mask bit */
      }
      
      if (bpp == 4)
      {
        if (j % 2)
          aImage[j/2] = (unsigned char)((0x0F & pixel) | (0xF0 & aImage[j/2])); /* second 4 bits low */
        else
          aImage[j/2] = (unsigned char)(0xF0 & (pixel << 4)); /* first 4 bits high */
      }
      else
        aImage[j] = pixel;
    }

    aImage += line_size;
    aMask -= mask_line;
    data -= width;
  }
}

static void changeProc(Ihandle *n, WNDPROC new_proc, char* name)
{
  HWND hwnd = handle(n);
  IupSetAttribute(n, name, (char*)GetWindowLongPtr (hwnd, GWLP_WNDPROC));
  SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)new_proc);
}

static void CreateButton (Ihandle *n, int x, int y)
{
 Ihandle* image      = IupGetAttributeHandle( n, IUP_IMAGE);
 Ihandle* iminactive = IupGetAttributeHandle( n, IUP_IMINACTIVE );
 Ihandle* impress    = IupGetAttributeHandle( n, IUP_IMPRESS );

 if (image)
 {
   if ( image && handle(image)==NULL)
     iupdrvCreateObjects( image );
 }

 if (iminactive)
 {
   if (iminactive && handle(iminactive)==NULL)
     iupdrvCreateObjects( iminactive );
 }

 if (impress)
 {
   if (impress && handle(impress)==NULL)
     iupdrvCreateObjects( impress );
 }

 /* Buttons with the BS_PUSHBUTTON, BS_DEFPUSHBUTTON, or BS_PUSHLIKE styles
    do not use the returned brush in WM_CTLCOLORBTN. 
    Buttons with these styles are always drawn with the default system colors.
    The BS_FLAT style does not completely remove the borders.
 */

 sethandle(n, CreateWindow ("BUTTON",   /* class */
        (LPCSTR)IupGetAttribute(n,IUP_TITLE),      /* window name */
        WS_CHILD | WS_TABSTOP | BS_NOTIFY |
        (iupCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0)    |
        BS_OWNERDRAW,
        x,                   /* x-position */
        y,                   /* y-position */
        currentwidth(n),     /* width */
        currentheight(n),    /* height */
        iupwin_parent_hwnd,    /* window parent */
        (HMENU)++iupwin_child_id,/* Identifier */
        iupwin_hinstance,           /* instance of app. */
        NULL) );               /* no creation parameters */

 iupwinHandleAdd(handle(n), n);

 /*redefine a callback de mensagens do botao*/
 changeProc(n, iupwinButtonProc, "_IUPWIN_BUTTONOLDPROC__");

 if (iupEnvCheck(n, IUP_ACTIVE) == NO)
  EnableWindow ((HWND)handle(n), FALSE);
}

void iupwinSetHScrollInfo( Ihandle* h )
{
  float posx=0.0F, xmin=0.0F, xmax=1.0F;
  float ratio;
  unsigned short x;
  float dx;
  SCROLLINFO scrollinfo;

   if (IupGetAttribute(h,IUP_XMIN))
      xmin = IupGetFloat(h,IUP_XMIN);

   if (IupGetAttribute(h,IUP_XMAX))
      xmax = IupGetFloat(h,IUP_XMAX);

   if (IupGetAttribute(h,IUP_POSX))
      posx = IupGetFloat(h,IUP_POSX);

   ratio = (posx-xmin)/(xmax-xmin);
   x = (unsigned short) (HORZ_SCROLLBAR_SIZE * ratio);
  if ( x < 1 )
    x = 1;

  dx = IupGetFloat(h,IUP_DX);
  if (dx == 0.0F)
    dx = 1.0F;
  scrollinfo.cbSize = sizeof(SCROLLINFO);
  scrollinfo.fMask = SIF_ALL;
  scrollinfo.nPage = (int) ((dx/(xmax-xmin))*HORZ_SCROLLBAR_SIZE);

  if (scrollinfo.nPage < 1)
    scrollinfo.nPage = 1;
  else if (scrollinfo.nPage > HORZ_SCROLLBAR_SIZE)
    {
       x = 1;
    scrollinfo.nPage = HORZ_SCROLLBAR_SIZE;
    }

    scrollinfo.nPos = x;
    scrollinfo.nMax = HORZ_SCROLLBAR_SIZE;
    scrollinfo.nMin = 1;

  SetScrollInfo((HWND)handle(h), SB_HORZ, &scrollinfo, TRUE);
}

void iupwinSetVScrollInfo( Ihandle* h )
{
  float posy=0.0F, ymin=0.0F, ymax=1.0F;
  float ratio;
  unsigned short y;

  if (IupGetAttribute(h,IUP_YMIN))
    ymin = IupGetFloat(h,IUP_YMIN);

  if (IupGetAttribute(h,IUP_YMAX))
    ymax = IupGetFloat(h,IUP_YMAX);

  if (IupGetAttribute(h,IUP_POSY))
    posy = IupGetFloat(h,IUP_POSY);

  ratio = (posy-ymin)/(ymax-ymin);
  y = (unsigned short) (VERT_SCROLLBAR_SIZE * ratio);
  if ( y < 1 )
    y = 1;

  {
    float dy = IupGetFloat(h,IUP_DY);
    SCROLLINFO scrollinfo;
    if (dy == 0.0F)
      dy = 1.0F;
    scrollinfo.cbSize = sizeof(SCROLLINFO);
    scrollinfo.fMask = SIF_ALL;
    scrollinfo.nPage = (int) ((dy/(ymax-ymin))*VERT_SCROLLBAR_SIZE);

    if (scrollinfo.nPage < 1)
      scrollinfo.nPage = 1;
    else if (scrollinfo.nPage > VERT_SCROLLBAR_SIZE)
      {
         scrollinfo.nPage = VERT_SCROLLBAR_SIZE;
         y = 1;
      }

      scrollinfo.nPos = y;
      scrollinfo.nMax = VERT_SCROLLBAR_SIZE;
      scrollinfo.nMin = 1;

    SetScrollInfo((HWND)handle(h), SB_VERT, &scrollinfo, TRUE);
  }
}

static void enableDragDrop(Ihandle *n)
{
  char *teste;
  teste = IupGetAttribute(n,IUP_DROPFILES_CB);
  if (teste!=NULL)
    DragAcceptFiles((HWND)handle(n), TRUE );
  else 
    DragAcceptFiles((HWND)handle(n), FALSE );
}

static void CreateCanvas (Ihandle *n, int x, int y)
{
 CLIENTCREATESTRUCT clientstruct;
 char buffer[30];
 void* clientdata = NULL;
 char* classname = NULL;
 
  DWORD sbstyle = 0L;  /* NO scroll by default */
  char* value = iupGetEnv(n, "SCROLLBAR");
  if (value)
  {
    if (iupStrEqualNoCase(value, "YES"))
      sbstyle = WS_VSCROLL | WS_HSCROLL;
    else if (iupStrEqualNoCase(value, "HORIZONTAL"))
      sbstyle = WS_HSCROLL;
    else if (iupStrEqualNoCase(value, "VERTICAL"))
      sbstyle = WS_VSCROLL;
  }

 if (iupCheck(n, "MDICLIENT")==YES)
 {
   iupSetEnv(n, "CLIPCHILDREN", "YES");
   iupSetEnv(n, "BORDER", "NO");
   classname = iupStrDup("mdiclient");
   sbstyle = WS_VSCROLL  | WS_HSCROLL; 
   iupSetEnv(IupGetDialog(n), "_IUPWIN_MDICLIENT", (char*)n);
   clientdata = &clientstruct;
   {
     Ihandle *winmenu = IupGetAttributeHandle(n, "MDIMENU");
     clientstruct.hWindowMenu = winmenu? handle(winmenu): NULL;
     clientstruct.idFirstChild = NUM_MAX_MENUS+1;
   }
 }
 else 
 {
   if (handle(n))
   {
     GetClassName( (HWND)handle(n), buffer, 30 );
     classname = iupStrDup(buffer);
   }
   else
     classname = GetCanvasClass();
 }

 sethandle(n,CreateWindow(classname,   /* Canvas class */
        "canvas",            /* window name */
        (iupCheck(n,IUP_BORDER)? WS_BORDER : 0) |
        (iupCheck(n,IUP_VISIBLE)? WS_VISIBLE : 0) |
        (iupCheck(n,IUP_CLIPCHILDREN)? WS_CLIPCHILDREN : 0) |  /* because of IupTabs and similars */
        sbstyle | WS_TABSTOP | WS_CHILD,    /* window style */
        x,                   /* x-position */
        y,                   /* y-position */
        currentwidth(n),     /* width */
        currentheight(n),    /* height */
        iupwin_parent_hwnd,    /* window parent */
        (HMENU)++iupwin_child_id,           /* Identifier */
        iupwin_hinstance,           /* instance of app. */
        clientdata));               /* no creation parameters */

 if(classname)
    free(classname);

 iupwinHandleAdd(handle(n), n);

 iupwinSetCursor (n);

 if (sbstyle & WS_HSCROLL)
   iupwinSetHScrollInfo(n);
 else
   IupSetAttribute(n,IUP_POSX,"0.0");

 if (sbstyle & WS_VSCROLL)
   iupwinSetVScrollInfo(n);
 else
   IupSetAttribute(n,IUP_POSY,"0.0");

 if (iupEnvCheck(n, IUP_ACTIVE) == NO)
   EnableWindow ((HWND)handle(n), FALSE);

 enableDragDrop(n);
}

static void CreateDlg (Ihandle *n)
{
 HMENU menu = iupwinGetMenu(n);
 DWORD dialstyle = 0, dialexstyle = 0;
 int mdi = 0;
 char *classname;
 HWND hwnd_owner=NULL;
 int titlebar  = 0,
     hasborder = 0;

 if(iupGetEnv(n, IUP_TITLE))
   titlebar = 1;

 if (iupCheck(n,IUP_MAXBOX))
 {
   dialstyle |= WS_MAXIMIZEBOX;
   titlebar = 1;
 }

 if (iupCheck(n,IUP_MINBOX))
 {
   dialstyle |= WS_MINIMIZEBOX;
   titlebar = 1;
 }

 if (iupCheck(n,IUP_MENUBOX))
 {
   dialstyle |= WS_SYSMENU;
   titlebar = 1;
 }

 if (iupCheck(n,IUP_RESIZE))
   dialstyle |= WS_THICKFRAME;

 if (!iupCheck(n,IUP_ACTIVE))
   dialstyle |= WS_DISABLED;

 if(iupCheck(n, IUP_BORDER) || titlebar==1)
   hasborder = 1;

 if (iupCheck(n,"MDICHILD")==YES)
 {
   static int mdi_child_id = NUM_MAX_MENUS+1;
   char name[50];
   Ihandle *client = IupGetAttributeHandle(n, "MDICLIENT");
   iupSetEnv(n, "_IUPWIN_MDICLIENT", (char*)client);

   mdi = 2;

   hwnd_owner = handle(client);

   dialstyle |= WS_CHILD;
   if(titlebar == 1)
     dialstyle |= WS_CAPTION;
   else if (hasborder == 1)
     dialstyle |= WS_BORDER;
   dialexstyle |= WS_EX_MDICHILD;

   sprintf(name, "mdichild%d", mdi_child_id - (NUM_MAX_MENUS+1));
   IupSetHandle(name, n);

   menu = (HMENU)mdi_child_id;
   mdi_child_id++;
 }
 else
 {
   Ihandle *parent = IupGetAttributeHandle(n, IUP_PARENTDIALOG);
   if(parent != NULL && type(parent) == DIALOG_)
   {
     if(handle(parent) != NULL)
       hwnd_owner = handle(parent);
     else
     {
       IupMap(parent);
       hwnd_owner = handle(parent);
     }
     
     dialstyle |= WS_POPUP;

     if(titlebar == 1)
       dialstyle |= WS_CAPTION;
     else if (hasborder == 1)
       dialstyle |= WS_BORDER;
   }
   else
   {
     if (titlebar == 1)
       dialstyle |= WS_OVERLAPPED;
     else 
     {
       if(hasborder == 1)
         dialstyle |= WS_POPUP | WS_BORDER;
       else
         dialstyle |= WS_POPUP;
     }
   }

   if (iupGetEnv(n, "MDIMENU"))
     mdi = 1;
 }

 if(hwnd_owner == NULL) /* if user hasn't set IUP_PARENTDIALOG */
  hwnd_owner = (HWND) IupGetAttribute(n, "NATIVEPARENT");

 if(iupCheck(n, IUP_TOOLBOX)==YES && hwnd_owner!=NULL)
   dialexstyle |= WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;

 if(iupCheck(n, IUP_CONTROL)==YES && hwnd_owner!=NULL) {
   dialexstyle |= WS_EX_CONTROLPARENT;
   dialstyle = WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN;
 }

 if(iupCheck(n, "COMPOSITED")==YES)
   dialexstyle |= WS_EX_COMPOSITED;

 if(iupCheck(n, "HELPBUTTON")==YES)
   dialexstyle |= WS_EX_CONTEXTHELP;

 classname = GetDialogClass(n, mdi);

 if(iupCheck(n, IUP_CLIPCHILDREN))
   dialstyle = dialstyle | WS_CLIPCHILDREN; /* Clips children. Must warn them to repaint */
 
 handle(n) = (void *) CreateWindowEx (dialexstyle /* extended styles */, 
        classname,  /* class */
        titlebar? iupGetEnv(n, IUP_TITLE) : NULL, /* title */
        dialstyle |              /* style */
        WS_CLIPSIBLINGS,
        CW_USEDEFAULT,           /* x-position */
        CW_USEDEFAULT,           /* y-position */
        currentwidth(n)+iupwinDialogDecorX(n),
        currentheight(n)+iupwinDialogDecorY(n),
        hwnd_owner,                /* owner window */
        menu,              /* Menu */
        iupwin_hinstance,           /* instance of app. */
        NULL);               /* no creation parameters */

 if (mdi == 2) /* hides the mdi child */
  ShowWindow(handle(n), SW_HIDE);

 free(classname);
 
 iupwinHandleAdd(handle(n), n);

 enableDragDrop(n);

 iupwinSetBgColor (n);
 iupwinSetCursor (n);
 iupwinUpdateAttr(n, "TRAY");
 iupwinUpdateAttr(n, "TRAYTIP");
 iupwinUpdateAttr(n, "TRAYIMAGE");
 iupwinUpdateAttr(n, "LAYERED");
 iupwinUpdateAttr(n, "LAYERALPHA");
 iupwinUpdateAttr(n, "TOPMOST"); 
 iupwinUpdateAttr(n, "ICON"); 
}

static void CreateFrame (Ihandle *n, int x, int y)
{
 if (iupGetEnv(n, IUP_TITLE))
  sethandle(n, CreateWindow ("BUTTON",   /* window class */
        (LPCSTR)iupGetEnv(n, IUP_TITLE),  /* window name */
        WS_CHILD | 
        (iupCheck(n,IUP_VISIBLE)? WS_VISIBLE : 0) |
        BS_GROUPBOX,             /* window style */
        x,                       /* x-position */
        y,                       /* y-position */
        currentwidth(n),         /* width */
        currentheight(n),        /* height */
        iupwin_parent_hwnd,        /* window parent */
        (HMENU)++iupwin_child_id,        /* Identifier */
        iupwin_hinstance,               /* instance of app. */
        NULL));                   /* no creation parameters */
 else
  sethandle(n, CreateWindow ("STATIC",   /* window class */
        NULL,  /* window name */
        WS_CHILD |
        (iupCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0) |
        (iupCheck(n,"SUNKEN")==YES? SS_SUNKEN : SS_ETCHEDFRAME), /* window style */
        x,                       /* x-position */
        y,                       /* y-position */
        currentwidth(n),         /* width */
        currentheight(n),        /* height */
        iupwin_parent_hwnd,        /* window parent */
        (HMENU)++iupwin_child_id,        /* Identifier */
        iupwin_hinstance,               /* instance of app. */
        NULL));                   /* no creation parameters */

 iupwinHandleAdd(handle(n), n);

 changeProc(n, iupwinFrameProc, "_IUPWIN_FRAMEOLDPROC__");

 if (iupEnvCheck(n, IUP_ACTIVE) == NO)
  EnableWindow ((HWND)handle(n), FALSE);
}

static long getalignment(Ihandle *n)
{
  int edit_align[] = {ES_LEFT, ES_CENTER, ES_RIGHT}; /* edit/multiline */
  int label_align[] = {SS_LEFT, SS_CENTER, SS_RIGHT}; /* label */
  int* win_align = type(n)==LABEL_? label_align: edit_align;
  long alignment = win_align[0];
  char *alig = IupGetAttribute(n, IUP_ALIGNMENT);

  if(!alig)
    return alignment;

  if(iupStrEqual(alig, IUP_ARIGHT))
    alignment = win_align[2];
  else if(iupStrEqual(alig, IUP_ALEFT))
    alignment = win_align[0];
  else if(iupStrEqual(alig, IUP_ACENTER))
    alignment = win_align[1];

  return alignment;
}

static void CreateLabel (Ihandle *n, int x, int y)
{
 int separator = 0;
 Ihandle* image = IupGetAttributeHandle(n, IUP_IMAGE);
 long alignment = getalignment(n);

/* the owner draw implementation is only necessary because of the background color */

 if (image)
 {
   if (image && handle(image)==NULL)
     iupdrvCreateObjects( image );
 }

 {
    char* sep = IupGetAttribute(n, IUP_SEPARATOR);
    if (sep)
    {
      if (iupStrEqual(sep, IUP_HORIZONTAL))
        separator = 1;
      else if (iupStrEqual(sep, IUP_VERTICAL))
        separator = 2;
    }
 }

 if (separator)
  sethandle(n, CreateWindow ( "STATIC",   /* window class */
          NULL,   /* window name */
          (separator == 1? SS_ETCHEDHORZ: SS_ETCHEDVERT) |
          (iupCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0) |
          WS_CHILD,             /* window style */
          x,                   /* x-position */
          y,                   /* y-position */
          currentwidth(n),     /* width */
          currentheight(n),    /* height */
          iupwin_parent_hwnd,    /* window parent */
          (HMENU)++iupwin_child_id,    /* Identifier */
          iupwin_hinstance,           /* instance of app. */
          NULL));               /* no creation parameters */
 else
  sethandle(n, CreateWindow ( "STATIC",   /* window class */
          (LPCSTR)IupGetAttribute(n,IUP_TITLE),   /* window name */
          (image?SS_OWNERDRAW:alignment) |   /* set alignment only for non owner draw */
          (iupCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0) |
          WS_CHILD,             /* window style */
          x,                   /* x-position */
          y,                   /* y-position */
          currentwidth(n),     /* width */
          currentheight(n),    /* height */
          iupwin_parent_hwnd,    /* window parent */
          (HMENU)++iupwin_child_id,    /* Identifier */
          iupwin_hinstance,           /* instance of app. */
          NULL));               /* no creation parameters */

 iupwinHandleAdd(handle(n), n);

 changeProc(n, iupwinButtonProc, "_IUPWIN_BUTTONOLDPROC__");

 if (iupEnvCheck(n, IUP_ACTIVE) == NO)
   EnableWindow ((HWND)handle(n), FALSE);
}

static void CreateList (Ihandle *n, int x, int y)
{
  int iscombobox;
  int isdropdown = iupCheck(n, IUP_DROPDOWN)==YES? 1: 0;
  int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;

  DWORD sbstyle = WS_VSCROLL;  /* VERTICAL scroll by default */
  char* value = iupGetEnv(n, "SCROLLBAR");
  if (value)
  {
    if (iupStrEqualNoCase(value, "YES"))
      sbstyle = WS_VSCROLL | WS_HSCROLL;
    else if (iupStrEqualNoCase(value, "HORIZONTAL"))
      sbstyle = WS_HSCROLL;
    else if (iupStrEqualNoCase(value, "NO"))
      sbstyle = 0;

    /* for the future LBS_DISABLENOSCROLL CBS_DISABLENOSCROLL */
  }

  if (isdropdown || haseditbox)
  {
    sethandle(n, CreateWindowEx (WS_EX_CLIENTEDGE, "COMBOBOX",   /* window class */
      "iupcombo",            /* window name */
      (iupEnvCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0) |  CBS_NOINTEGRALHEIGHT |
      (haseditbox? CBS_AUTOHSCROLL: 0) |
      (haseditbox? (isdropdown? CBS_DROPDOWN: CBS_SIMPLE): CBS_DROPDOWNLIST) | 
      WS_TABSTOP | WS_CHILD | sbstyle,    /* window style */
      x,                   /* x-position */
      y,                   /* y-position */
      CW_USEDEFAULT,       /* width */
      CW_USEDEFAULT,       /* height */
      iupwin_parent_hwnd,       /* window parent */
      (HMENU)++iupwin_child_id,    /* Identifier */
      iupwin_hinstance,           /* instance of app. */
      NULL));               /* no creation parameters */

  }
  else
  {
    sethandle(n, CreateWindowEx (WS_EX_CLIENTEDGE, /* extended style */
      "LISTBOX",   /* window class */
      "iuplistbox",            /* window name */
      (iupCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0) | LBS_NOINTEGRALHEIGHT |
      (iupCheck(n, IUP_MULTIPLE)==YES ? LBS_EXTENDEDSEL : 0) |
      LBS_NOTIFY |
      WS_TABSTOP | WS_CHILD | sbstyle,  /* window style */
      x,                   /* x-position */
      y,                   /* y-position */
      currentwidth(n),     /* width */
      currentheight(n),    /* height */
      iupwin_parent_hwnd,    /* window parent */
      (HMENU)++iupwin_child_id,           /* Identifier */
      iupwin_hinstance,           /* instance of app. */
      NULL));               /* no creation parameters */
  }

  iupwinHandleAdd(handle(n), n);

  iscombobox = isdropdown|haseditbox;

  {  /* insere strings na lista */
    char  op[5];
    char *v;
    int max_w = 0, i=1, dropexpand = iupCheck(n, "DROPEXPAND")!=NO;
    UINT addstring = iscombobox? CB_ADDSTRING : LB_ADDSTRING;

    if (isdropdown && dropexpand)
      iupwinUpdateFont(n);

    while(1)
    {
      sprintf (op, "%d", i);
      v = iupGetEnv (n,op);
      if (v == NULL)
        break;
      SendMessage ((HWND)handle(n), addstring, 0, (LPARAM)v);

      if (isdropdown && dropexpand)
      {
        int w, h;
        iupdrvStringSize(n, v, &w, &h);
        if (w>max_w)
          max_w = w;
      }

      i++;
    }

    if (isdropdown && dropexpand)
    {
      int sb = iupdrvGetScrollSize()+6; /* +borders */;
      SendMessage((HWND)handle(n), CB_SETDROPPEDWIDTH, max_w+sb, 0);
    }
  }

  {  /* inicializa opcoes da lista */
    char* v = iupGetEnv (n, IUP_VALUE);
    UINT setcursel = iscombobox? CB_SETCURSEL : LB_SETCURSEL;
    unsigned  i;
    if (v)
    {
      if (haseditbox)
      {
        SendMessage ((HWND)handle(n), WM_SETTEXT, 0, (LPARAM)v);
      }
      else if (iupCheck(n, IUP_MULTIPLE)==YES)
      {
        for (i=0; i < strlen(v); ++i)
          if (v[i] == '+')
            SendMessage ((HWND)handle(n), LB_SETSEL, 1, i);
        iupStoreEnv(n, "_IUPWIN_LISTSEL", v);
      }
      else
      {
        sscanf (v, "%d", &i);
        SendMessage ((HWND)handle(n), setcursel, i-1, 0L);
        iupStoreEnv(n, "_IUPWIN_LISTSEL", v);
      }
    }
    else if (isdropdown)
      SendMessage ((HWND)handle(n), setcursel, 0, 0L);
  }

  if (iupEnvCheck(n, IUP_ACTIVE) == NO)
    EnableWindow ((HWND)handle(n), FALSE);

  if (haseditbox)
  {
    char* readonly;
    COMBOBOXINFO boxinfo;
    char* strMaxChar = iupGetEnv( n, IUP_NC );
    if(strMaxChar)
    {
      int maxchar = atoi(strMaxChar);
      SendMessage( (HWND)handle(n), CB_LIMITTEXT, maxchar, 0L );
    }

    ZeroMemory(&boxinfo, sizeof(COMBOBOXINFO));
    boxinfo.cbSize = sizeof(COMBOBOXINFO);
    GetComboBoxInfo((HWND)handle(n), &boxinfo);
    iupwinHandleAdd(boxinfo.hwndItem, n);
    IupSetAttribute(n, "_IUPWIN_EDITBOX", (char*)boxinfo.hwndItem);

    IupSetAttribute(n, "_IUPWIN_TEXTOLDPROC__", (char*)GetWindowLongPtr(boxinfo.hwndItem, GWLP_WNDPROC));
    SetWindowLongPtr (boxinfo.hwndItem, GWLP_WNDPROC, (LONG_PTR)iupwinEditProc);

    readonly = iupGetEnv( n, IUP_READONLY );
    if(readonly && iupStrEqual(readonly, "YES"))
      SendMessage(boxinfo.hwndItem, EM_SETREADONLY, (WPARAM)TRUE, 0);
  }

  if (isdropdown || haseditbox)
  {
    if (iupwin_comctl32ver6)
    {
      /* Setting combobox's own window proc. Necessary because it does not paint its
        * own background. Should always be done after setting the Ihandle, so that the
        * value can be retrieved. Occours only in WinXP.
        */

      COMBOBOXINFO boxinfo;

      ZeroMemory(&boxinfo, sizeof(COMBOBOXINFO));
      boxinfo.cbSize = sizeof(COMBOBOXINFO);
      GetComboBoxInfo((HWND)handle(n), &boxinfo);
      iupwinHandleAdd(boxinfo.hwndList, n);
      IupSetAttribute(n, "_IUPWIN_COMBOBOX", (char*)boxinfo.hwndList);

      IupSetAttribute(n, "_IUPWIN_COMBOLISTOLDPROC__", (char*)GetWindowLongPtr(boxinfo.hwndList, GWLP_WNDPROC));
      SetWindowLongPtr (boxinfo.hwndList, GWLP_WNDPROC, (LONG_PTR)iupwinComboListProc);
    }
  }
}

char* iupwinMenuLabel( Ihandle* h, char* label )
{
  char* keyattr = IupGetAttribute( h, IUP_KEY );

  if (keyattr)
  {
    int keycode;
    int size;
    int i, j;
    char *newlabel = (char*)malloc(sizeof(char)*strlen(label)+3);

    keycode = iupKeyDecode(keyattr);
    size = strlen(label) + 1;
    for (i=0, j=0; i < size; i++, j++)
    {
      if (label[i]==keycode)
      {
        newlabel[j] = '&';
        j++;
        strcpy(newlabel + j, label + i);
        break;
      }
      newlabel[j] = label[i];
    }

    IupStoreAttribute(h, "_IUPWIN_MENULABEL", newlabel);
    free(newlabel);
    return IupGetAttribute(h, "_IUPWIN_MENULABEL");
  }
  else
  {
    IupStoreAttribute(h, "_IUPWIN_MENULABEL", label);
    return IupGetAttribute(h, "_IUPWIN_MENULABEL");
  }
}

static HMENU iupwinFillMenu (Ihandle* n)
{
 HMENU m = (HMENU) handle(n);
 Ihandle *c;

 assert(m);
 assignidIhandle(n);

 foreachchild (c,n)
 {
  assignidIhandle(c);
  if(type(c) == ITEM_)
  {
    Ihandle* himage;
    char *value;
    handle(c) = (void *) m;
    assert(number(c)>0);

    AppendMenu (m, MF_STRING, number(c), iupwinMenuLabel(c, IupGetAttribute(c, IUP_TITLE)));

    himage = IupGetAttributeHandle(c, IUP_IMAGE);
    if (himage)
    {
      MENUITEMINFO iteminfo;
      HBITMAP hbm, hbmpress = NULL;
      HDC hDC;             
      Ihandle* himpress;

      himpress = IupGetAttributeHandle(c, IUP_IMPRESS);

      hDC = GetDC(NULL);
      hbm = iupwinCreateBitmap(himage, hDC, c);
      if (himpress)
        hbmpress = iupwinCreateBitmap(himpress, hDC, c);
      ReleaseDC(NULL, hDC);

      iteminfo.cbSize = sizeof(MENUITEMINFO);
      iteminfo.fMask = MIIM_CHECKMARKS;
      iteminfo.hbmpUnchecked = hbm;     /* will be deleted by the system */
      iteminfo.hbmpChecked = hbmpress;  

      SetMenuItemInfo(m, number(c), FALSE, &iteminfo);
    }

    if (iupEnvCheck(c,IUP_ACTIVE) == NO)
      EnableMenuItem (m, number(c), MF_GRAYED);
    else
      EnableMenuItem (m, number(c), MF_ENABLED);

    value = iupGetEnv(c,IUP_VALUE);
    assert(number(c)>0);
    if (value && iupStrEqual(value,IUP_ON))
      CheckMenuItem ((HMENU)handle(c), number(c), MF_CHECKED | MF_BYCOMMAND);

  }
  else if(type(c) == MENU_ || type(c) == SUBMENU_)
  {
    int value = 0;
    BOOL test;
    handle(c) = iupwinCreatePopupMenu(child(c));

    if (iupEnvCheck(c,IUP_ACTIVE) == NO)
      value = MF_GRAYED;
     
    test = AppendMenu (m, MF_POPUP | value, 
      (UINT_PTR) handle(c), 
      (LPCSTR)iupwinMenuLabel(c, IupGetAttribute(c,IUP_TITLE)));

    assert(test != 0);
  }
  else if(type(c) == SEPARATOR_)
    AppendMenu (m, MF_SEPARATOR, number(c), NULL);
 }
 return m;
}

HMENU iupwinCreatePopupMenu (Ihandle* n)
{
 handle(n) = (void*)CreatePopupMenu ();
 assert(handle(n));

 iupwinFillMenu( n );

 return (HMENU) handle(n);
}

static HMENU iupwinCreateMenu (Ihandle* n)
{
  handle(n) = (void*)CreateMenu ();
  assert(handle(n));

  iupwinFillMenu( n );

  return (HMENU) handle(n);
}

static void CreateTypein (Ihandle* n, int x, int y, int type)
{
  DWORD style = WS_TABSTOP | WS_CHILD;
  int to_free = 0;
  char *v = iupwinTransMultiline(iupGetEnv (n, "VALUE"), &to_free);
  char* tabsize = NULL;

  if (!v)
    v = "";

  if (type==IUPWIN_MULTILINE)
  {
    DWORD sbstyle = WS_VSCROLL | WS_HSCROLL;  /* both scrolls by default */
    char* value = iupGetEnv(n, "SCROLLBAR");
    if (value)
    {
      if (iupStrEqualNoCase(value, "VERTICAL"))
        sbstyle = WS_VSCROLL;
      else if (iupStrEqualNoCase(value, "HORIZONTAL"))
        sbstyle = WS_HSCROLL;
      else if (iupStrEqualNoCase(value, "NO"))
        sbstyle = 0;
    }

    if (iupCheck(n, "WORDWRAP")!=YES)
      style |= ES_AUTOHSCROLL;   /* do not specify ES_AUTOHSCROLL, the control automatically wraps words */
    else
      sbstyle &= ~WS_HSCROLL;    /* must remove the horizontal scroolbar */

    style |= sbstyle;
    style |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
  }
  else
    style |= ES_AUTOHSCROLL;

  style |= getalignment(n);

  if (iupCheck(n,IUP_VISIBLE))
    style |= WS_VISIBLE;

  if (iupCheck(n,IUP_READONLY)==YES)
    style |= ES_READONLY;

  if (iupCheck(n,"PASSWORD")==YES)
    style |= ES_PASSWORD;

  sethandle(n, CreateWindowEx (WS_EX_CLIENTEDGE,
    "EDIT",     /* window class */
    (LPCSTR)v,  /* window name */
    style,      /* window style */
    x,                   /* x-position */
    y,                   /* y-position */
    currentwidth(n),     /* width */
    currentheight(n),    /* height */
    iupwin_parent_hwnd,    /* window parent */
    (HMENU)++iupwin_child_id,    /* Identifier */
    iupwin_hinstance,           /* instance of app. */
    NULL));               /* no creation parameters */

  iupwinHandleAdd(handle(n), n);

  /*redefine a callback de mensagens do controle de edicao*/
  changeProc(n, iupwinEditProc, "_IUPWIN_TEXTOLDPROC__");

  {
    int maxchar = 0; /* maximum limit */
    char* strMaxChar = iupGetEnv(n, IUP_NC);
    if (strMaxChar)
      maxchar = atoi(strMaxChar);
    SendMessage( (HWND)handle(n), EM_LIMITTEXT, maxchar, 0L );
  }

  if (iupEnvCheck(n, IUP_ACTIVE) == NO)
    EnableWindow ((HWND)handle(n), FALSE);

  if (type(n) == MULTILINE_)
  {
    tabsize = iupGetEnv(n, "TABSIZE");
    if (tabsize)
      iupdrvSetAttribute(n, "TABSIZE", tabsize);
  }

  if (to_free)
    free(v);
}

static void CreateToggle (Ihandle *n, int x, int y)
{     
 Ihandle* image      = IupGetAttributeHandle(n, IUP_IMAGE);
 Ihandle* iminactive = IupGetAttributeHandle(n, IUP_IMINACTIVE);
 Ihandle* impress    = IupGetAttributeHandle(n, IUP_IMPRESS);
 long  btstyle = 0;
 Ihandle* radio = iupGetRadio(n);

 btstyle |= WS_CHILD | BS_NOTIFY |
           (iupCheck(n,IUP_VISIBLE) ? WS_VISIBLE : 0);

 if (radio)
 {
   btstyle |= BS_AUTORADIOBUTTON;
   if (!iupGetEnv(radio,"_IUPWIN_LASTTOGGLE"))
   {
     /* this is the first toggle in the radio */
     iupSetEnv(radio,"_IUPWIN_LASTTOGGLE", (char*)n);

     btstyle |= WS_GROUP; /* this is specified only for the first toggle in the radio. But necessary. */
                          /* this affects keyboard navigation in the dialog for the arrow keys */
                          /* it will form a group up to the next Radio. This is weird. */
   }
 }
 else
 {
   if (!image && iupCheck(n, "3STATE")==YES)
     btstyle |= BS_AUTO3STATE;
   else
     btstyle |= BS_AUTOCHECKBOX;

   /* Do not set TABSTOP for toggles in a radio, 
      we will set it later. */
   btstyle |= WS_TABSTOP;
 }

/* The BGCOLOR for toggles has the same problem of buttons and labels,
   but here we opt to not implement an OWNERDRAW and keep the problem. */

 if (image)
 {
   btstyle |= BS_BITMAP | BS_PUSHLIKE;
   if ( image && handle(image)==NULL)
     iupdrvCreateObjects(image);
 }

 if (iminactive)
 {
   if (iminactive && handle(iminactive)==NULL)
     iupdrvCreateObjects(iminactive);
 }

 if (impress)
 {
   if (impress && handle(impress)==NULL)
     iupdrvCreateObjects(impress);
 }

 sethandle(n, CreateWindow ("BUTTON",   /* window class */
        (LPCSTR)IupGetAttribute(n,IUP_TITLE),      /* window name */
        btstyle,             /* windows style */
        x,                   /* x-position */
        y,                   /* y-position */
        currentwidth(n),     /* width */
        currentheight(n),    /* height */
        iupwin_parent_hwnd,    /* window parent */
        (HMENU)++iupwin_child_id,    /* Identifier */
        iupwin_hinstance,           /* instance of app. */
        NULL));               /* no creation parameters */

 iupwinHandleAdd(handle(n), n);

 /* We only need to change procedure if image is wanted */
 if(image != NULL && !iupwinUseComCtl32Ver6())
   changeProc(n, iupwinToggleProc, "_IUPWIN_TOGGLEOLDPROC__");

 iupwinUpdateAttr(n, IUP_VALUE);
 iupwinUpdateAttr(n, IUP_IMAGE);
 iupwinUpdateAttr(n, IUP_ACTIVE);
}

HMENU iupwinGetMenu (Ihandle* n)
{
 Ihandle* menu = IupGetHandle(iupGetEnv (n, IUP_MENU));
 if (menu == NULL)
  return NULL;

 IupSetAttribute( menu, "_IUPWIN_DIALOG", (char*)n );
 parent(menu) = n;
 iupdrvCreateObjects (menu);
 return (HMENU) handle(menu);
}

void iupdrvCreateObject( Ihandle* self, Ihandle* parent )
{
   HWND oldhwnd = iupwin_parent_hwnd;
   Ihandle* olddlg = iupwin_parent;

   iupwin_parent = parent;
   if (parent)
       iupwin_parent_hwnd = (HWND)handle(parent);
   iupdrvCreateObjects(self);

   iupwin_parent_hwnd = oldhwnd;
   iupwin_parent = olddlg;
}

void iupdrvCreateObjects (Ihandle *n)
{
  int flag =0;

  if (type(n) != DIALOG_ && iupwin_parent_hwnd == NULL && 
      type(n) != IMAGE_ && type(n) != MENU_)
  {
    flag = 1;
    iupwin_parent = IupGetDialog(n);
    iupwin_parent_hwnd = (HWND)handle(iupwin_parent);
  }

  if (hclass(n))
    iupCpiMap(n, iupwin_parent);
  else
  {
    if (!handle(n)) iupdrvCreateNativeObject(n);
  }
  
  if (flag)
  {
    flag = 0;
    iupwin_parent_hwnd = NULL;
    iupwin_parent  = NULL;
  }
}

void iupdrvCreateNativeObject (Ihandle *n)
{
  Ihandle *c;

  if(type(n) == IMAGE_)
    CreateImage(n);
  else if(type(n) == BUTTON_)
    CreateButton (n, posx(n), posy(n));
  else if(type(n) == CANVAS_)
    CreateCanvas (n, posx(n), posy(n));
  else if(type(n) == DIALOG_)
  {
    CreateDlg(n);

    iupwin_parent_hwnd = (HWND) handle(n);
    iupwin_parent  = n;

    if (child(n)) iupdrvCreateObjects (child(n));

    iupwin_parent_hwnd = NULL;
    iupwin_parent  = NULL;
  }
  else if(type(n) == FILL_)
  {
    handle(n) = (void*)iupwin_parent_hwnd;
  }
  else if(type(n) == FRAME_)
  {
    CreateFrame (n, posx(n), posy(n));
    iupdrvCreateObjects (child(n));
  }
  else if(type(n) == HBOX_ || type(n) == VBOX_)
  {
    handle(n) = (void *) iupwin_parent_hwnd;
    foreachchild(c,n)
      iupdrvCreateObjects (c);
  }
  else if(type(n) == LABEL_)
    CreateLabel (n, posx(n), posy(n));
  else if(type(n) == LIST_)
    CreateList (n, posx(n), posy(n));
  else if(type(n) == MENU_)
    iupwinCreateMenu (n);
  else if(type(n) == RADIO_)
  {
    handle(n) = (void *) iupwin_parent_hwnd;
    iupSetEnv(n, "_IUPWIN_LASTTOGGLE", NULL);
    iupdrvCreateObjects (child(n));

    {
      Ihandle* last_tg = (Ihandle*)iupGetEnv(n, "_IUPWIN_LASTTOGGLE");
      if (last_tg) /* not empty radio */
      {
        int check = IupGetInt(last_tg, "VALUE");
        if (!check) /* last toggle must be ON */
          IupSetAttribute(last_tg, "VALUE", "ON");
      }
    }
  }
  else if(type(n) == MULTILINE_)
    CreateTypein (n, posx(n), posy(n), IUPWIN_MULTILINE);
  else if(type(n) == TEXT_)
    CreateTypein (n, posx(n), posy(n), IUPWIN_SINGLELINE);
  else if(type(n) == TOGGLE_)
    CreateToggle (n, posx(n), posy(n));
  else if(type(n) == ZBOX_)
  {
    Ihandle *v = NULL;
    char *value = IupGetAttribute (n,IUP_VALUE);
    if(value == NULL && child(n)) 
    {
      /* se value nao tiver sido setado, usa nome do
        primeiro elemento como value default */
      value = IupGetName(child(n));

      assert(value != NULL); /* todos os filhos da ZBOX devem ter nomes */
      if(value == NULL)
        return;
    }

    v = IupGetHandle(value);

    handle(n) = (void *) iupwin_parent_hwnd;
    
    iupStoreEnv(n, "_IUPWIN_ZBOX_OLDVALUE", value);
    foreachchild(c,n)
    {
      if (c!=v) 
      {
        iupSetEnv(c,IUP_VISIBLE,IUP_NO);  /* make remaining planes invisible */
        iupdrvCreateObjects (c);
      }
      else
        iupdrvCreateObjects (c);
    }
  }

  iupwinUpdateFont(n);
  iupwinTipsUpdate(n);

  if(type(n) == DIALOG_)
    iupwinUpdateAttr(n, "FULLSCREEN"); /* must be updated after updating the FONT */
}

/******************************************************************** 
** Assignes a child window number to an Ihandle. Necessary for
** menu items. Assigns the first array position not being used
** by the application.
********************************************************************/
static void assignidIhandle(Ihandle *h)
{
  int i=MENUINITIALNUMBER;
  while(1)
  {
    if(!iupwin_menus[i])
    {
      iupwin_menus[i] = h;
      number(h) = i;
      break;
    }
    else
    {
      i++;
      assert(i<NUM_MAX_MENUS); /* Array size is over */
    }
  }
}

/******************************************************************** 
** Removes an id Ihandle association (used by delete item).
********************************************************************/
void iupwinCleanidIhandle(int id)
{
  assert(id>=MENUINITIALNUMBER&&id<NUM_MAX_MENUS);
  if (id<MENUINITIALNUMBER||id>=NUM_MAX_MENUS)
    return;
  assert(iupwin_menus[id]); /* Must clean an already defined Ihandle */
  iupwin_menus[id] = NULL;
}

/******************************************************************** 
** Returns a menu Ihandle from a menu window id.
********************************************************************/
Ihandle *iupwinGetMenuIhandle(int id)
{
/*  assert(id>=MENUINITIALNUMBER&&id<NUM_MAX_MENUS); */
  if (id<MENUINITIALNUMBER||id>=NUM_MAX_MENUS)
    return NULL;
  assert(iupwin_menus[id]);
  return iupwin_menus[id];
}

/******************************************************************** 
** Initializes this module (called from IupOpen).
********************************************************************/

#define PACKVERSION(major,minor) MAKELONG(minor,major)
typedef struct _DLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                   
    DWORD dwMinorVersion;                   
    DWORD dwBuildNumber;                    
    DWORD dwPlatformID;                     
} DLLVERSIONINFO;
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

static DWORD GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    /* For security purposes, LoadLibrary should be provided with a 
       fully-qualified path to the DLL. The lpszDllName variable should be
       tested to ensure that it is a fully qualified path before it is used. */
    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, 
                          "DllGetVersion");

        /* Because some DLLs might not implement this function, you
        must test for it explicitly. Depending on the particular 
        DLL, the lack of a DllGetVersion function can be a useful
        indicator of the version. */

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = pDllGetVersion(&dvi);

            if(SUCCEEDED(hr))
            {
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

void iupwinCreatInit(void)
{
  memset(iupwin_menus, 0, NUM_MAX_MENUS*sizeof(Ihandle*));
  iupwin_comctl32ver6 = (GetDllVersion(TEXT("comctl32.dll")) >= 0x060000)? 1: 0;
}
