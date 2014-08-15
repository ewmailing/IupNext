/** \file
 * \brief Motif Driver controls creation
 *
 * See Copyright Notice in iup.h
 *  */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>    /* not really needed */
#include <limits.h>    /* not really needed */
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/List.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>
#include <Xm/MenuShell.h>
#include <Xm/FileSB.h>
#include <Xm/ScrollBar.h>
#include <X11/StringDefs.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/CascadeB.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleB.h>
#include <Xm/BulletinB.h>  

#if (XmVERSION < 2)
#include "ComboBox1.h"
#else
#include <Xm/ComboBox.h>
#endif

#include "Canvas.h"

#include "iglobal.h"
#include "idrv.h"
#include "motif.h"

static Widget iupmot_parent_widget = NULL;
static Ihandle* iupmot_parent = NULL;

/* Use this to debug not defined callbacks when creating controls. */
/* #define CB_DEBUG */
#ifdef CB_DEBUG
static void myXtAddCallback (Widget w, String name, XtCallbackProc cb, XtPointer data)
{
  printf("XtAddCallback(%s)\n", name);
  XtAddCallback (w, name, cb, data);
}
#define XtAddCallback myXtAddCallback
#endif

#ifdef CB_DEBUG
static void myXtRemoveAllCallbacks (Widget w, String name)
{
  printf("XtRemoveAllCallbacks(%s)\n", name);
  XtRemoveAllCallbacks (w, name);
}
#define XtRemoveAllCallbacks myXtRemoveAllCallbacks
#endif

/*********************************/

static char *resourceStr (void)
{
 static int resourceid = -1;
 static char *resourcenames[100] = {
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
  "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
  "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
  "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
  "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
  "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
  "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
  "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
  "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
  "90", "91", "92", "93", "94", "95", "96", "97", "98", "99"
 };

 resourceid++;
 resourceid = resourceid % 100;
 return resourcenames[resourceid];
}

void iupmotCreateImage (Ihandle *n, Ihandle* prnt, int inactive)
{
  int width = IupGetInt(n,IUP_WIDTH);
  int height = IupGetInt(n,IUP_HEIGHT);
  unsigned char *data = (unsigned char*)image_data(n);
  Pixmap pxm, bgcolor = iupmot_background;
  unsigned int red, green, blue;
  int i, j;
  GC gc;
  Pixel table[256];
  char attr[6], *value;

  static struct { unsigned char r, g, b; } default_colors[] = {
    { 0,0,0 }, { 128,0,0 }, { 0,128,0 }, { 128,128,0 },    
    { 0,0,128 }, { 128,0,128 }, { 0,128,128 }, { 192,192,192 },    
    { 128,128,128 }, { 255,0,0 }, { 0,255,0 }, { 255,255,0 },
    { 0,0,255 }, { 255,0,255 }, { 0,255,255 }, { 255,255,255 } };

  memset(table, 0, sizeof(Pixel)*256);

  value = IupGetAttribute(prnt? prnt: n, IUP_BGCOLOR);
  if (value)
  {
    if (iupGetRGB(value, &red, &green, &blue))
      bgcolor = iupmot_GetPixel((unsigned char)red, (unsigned char)green, (unsigned char)blue);
  }

  for (i=0;i<16;i++)
  {
    sprintf( attr, "%d", i );
    value = IupGetAttribute(n, attr);

    if (value)
    {
      if(iupStrEqual(value,"BGCOLOR"))
        table[i] = bgcolor;
      else
      {
        if (!iupGetRGB(value, &red, &green, &blue))
          table[i] = iupmot_GetPixel(default_colors[i].r, default_colors[i].g, default_colors[i].b);
        else
          table[i] = iupmot_GetPixel((unsigned char)red, (unsigned char)green, (unsigned char)blue);
      }
    }
    else
    {
      table[i] = iupmot_GetPixel(default_colors[i].r, default_colors[i].g, default_colors[i].b);
    }
  }

  for (;i<256;i++)
  {
    sprintf( attr, "%d", i );
    value = IupGetAttribute(n, attr);
    if (!value)
      break;

    iupGetRGB(value, &red, &green, &blue);
    table[i] = iupmot_GetPixel((unsigned char)red, (unsigned char)green, (unsigned char)blue);
  }

  if (inactive)
  {
    Pixmap fginactive;
    unsigned char bgred, bggreen, bgblue,
                  red, green, blue;
    int total_colors = i;

    /* replace the non transparent colors by a darker version of the background color. */
    /* replace near white by the background color */
    /* replace similar colors to the background color by itself */

    iupmot_GetRGB(bgcolor, &bgred, &bggreen, &bgblue);
    fginactive = iupmot_GetPixel(bgred/2, bggreen/2, bgblue/2);

    for (i=0;i<total_colors;i++)
    {
      iupmot_GetRGB(table[i], &red, &green, &blue);

      if ((red>190 && green>190 && blue>190) ||
          ((abs(red-bgred) + abs(green-bggreen) + abs(blue-bgblue))) < 15 )
      {
        table[i] = bgcolor; 
      }
      else
      {
        table[i] = fginactive; 
      }
    }
  }

  pxm = XCreatePixmap(iupmot_display,
          RootWindow(iupmot_display,iupmot_screen),
          width, height, iupmot_depth);

  gc = XCreateGC(iupmot_display,pxm,0,NULL);
  for (i=0;i<height;i++)
  {
    for(j=0;j<width;j++)
    {
      XSetForeground(iupmot_display,gc,table[data[i*width+j]]);
      XDrawPoint(iupmot_display,pxm,gc,j,i);
    }
  }
  XFreeGC(iupmot_display,gc);

  if (inactive)
    iupSetEnv(n, "_IUPMOT_IMINACTIVE", (char*)pxm);
  else
    handle(n) = (Widget)pxm;
}

void iupmotCreateCursor (Ihandle *n)
{
  int width = IupGetInt(n,IUP_WIDTH);
  int height = IupGetInt(n,IUP_HEIGHT);
  int line_size = (width+7)/8;
  int size_bytes = line_size*height;
  char *data = (char*)image_data(n);
  char *sbits, *mbits, *sb, *mb;
  Pixmap source, mask;
  XColor fg, bg;
  unsigned int r, g, b;
  int i,j;

  char *hotspot = IupGetAttribute( n, IUP_HOTSPOT );
  int hx=0, hy=0;
  if (hotspot)
  {
    iupStrToIntInt(hotspot, &hx, &hy, ':');
    if (hx >= width)  hx = width-1;
    if (hy >= height) hy = height-1;
  }

  sbits = (char*)malloc(size_bytes*sizeof(char));
  if (!sbits) return;
  mbits = (char*)malloc(size_bytes*sizeof(char));
  if (!mbits) return;

  for (i=0; i<size_bytes; i++) { sbits[i]=0; mbits[i]=0; }

  sb = sbits;
  mb = mbits;
  for (i=0; i<height; i++)
  {
    for (j=0; j<width; j++)
    {
      int byte = j/8;
      int bit = j%8;
      int cor = (int)data[i*width+j];
      if (cor == 1)
        sb[byte] = sb[byte] | (1<<bit);
      if (cor != 0)
        mb[byte] = mb[byte] | (1<<bit);
    }

    sb += line_size;
    mb += line_size;
  }

  r = 255; g = 255; b = 255;
  iupGetRGB( IupGetAttribute(n, "1"), &r, &g, &b );
  fg.red   = (unsigned short)((r/255.0)*USHRT_MAX);
  fg.green = (unsigned short)((g/255.0)*USHRT_MAX);
  fg.blue  = (unsigned short)((b/255.0)*USHRT_MAX);
  fg.flags = DoRed | DoGreen | DoBlue;

  r = 0; g = 0; b = 0;
  iupGetRGB( IupGetAttribute(n, "2"), &r, &g, &b );
  bg.red   = (unsigned short)((r/255.0)*USHRT_MAX);
  bg.green = (unsigned short)((g/255.0)*USHRT_MAX);
  bg.blue  = (unsigned short)((b/255.0)*USHRT_MAX);
  bg.flags = DoRed | DoGreen | DoBlue;

  source = XCreateBitmapFromData( iupmot_display, 
             RootWindow(iupmot_display,iupmot_screen),
             sbits, width, height );
  mask   = XCreateBitmapFromData( iupmot_display, 
             RootWindow(iupmot_display,iupmot_screen),
             mbits, width, height );

  handle(n) = (Widget)XCreatePixmapCursor( iupmot_display,
                source, mask, &fg, &bg, hx, hy);
}

static XtArgVal getPixmap(Ihandle *n, char * attr, int inactive)
{
  Ihandle *bmp = IupGetHandle(iupGetEnv(n,attr));
  if (!bmp) 
  {
    /* not defined and it is inactive, use the IUP_IMAGE instead. */
    if (inactive)
    {
      bmp = IupGetHandle(iupGetEnv(n,IUP_IMAGE));
      if (!bmp)
        return XmUNSPECIFIED_PIXMAP;

      iupmotCreateImage(bmp, n, 1);

      return (XtArgVal)iupGetEnv(bmp,"_IUPMOT_IMINACTIVE");
    }
    else
      return XmUNSPECIFIED_PIXMAP;
  }
  else
  {
    if (handle(bmp) == NULL) 
      iupmotCreateImage(bmp, n, 0);

    return (XtArgVal)handle(bmp);
  }
}

static void createButton (Ihandle *n)
{
   Iwidgetdata *d = XtNew (Iwidgetdata);
   XtArgVal bmp = getPixmap(n,IUP_IMAGE, 0);
   XtArgVal bmpPress = getPixmap(n,IUP_IMPRESS, 0);
   XtArgVal bmpInactive = getPixmap(n,IUP_IMINACTIVE, 1);
   int drawImageInactive = (bmpInactive != XmUNSPECIFIED_PIXMAP);

   d->ihandle = n;
   d->data    = 0;

   handle(n) = XtVaCreateManagedWidget (resourceStr (),
     xmPushButtonWidgetClass, iupmot_parent_widget,
     XmNx, (XtArgVal) posx(n),
     XmNy, (XtArgVal) posy(n),
     XmNwidth, (XtArgVal) currentwidth(n),
     XmNheight, (XtArgVal) currentheight(n),
     XmNhighlightThickness, 1,
     XmNtraversalOn, True,
     XmNnavigationType, XmTAB_GROUP,
     XmNrecomputeSize, False,
     XmNmarginHeight, 0,
     XmNmarginWidth, 0,
     XmNlabelType, (bmp!=XmUNSPECIFIED_PIXMAP)?XmPIXMAP:XmSTRING,
     XmNlabelPixmap, bmp,
     XmNlabelInsensitivePixmap, drawImageInactive ? bmpInactive : bmp,
     XmNalignment, XmALIGNMENT_CENTER,
     XmNarmPixmap, bmpPress,
     XmNshadowThickness, ((bmpPress!=XmUNSPECIFIED_PIXMAP)||(iupCheck(n, "FLAT")==YES))?0:2,
     XmNuserData, d,
     NULL);
   XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);
   XtAddCallback ((Widget)handle(n), XmNactivateCallback, iupmotCBbutton, NULL);
   XtAddEventHandler((Widget)handle(n), ButtonPressMask | ButtonReleaseMask,
                     False, (XtEventHandler)iupmotCBbuttoncb, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), FocusChangeMask, False,
                     (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), KeyPressMask,    False,
                     (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);

   iupmotSetAttrs (n);
}

static Cardinal order(Widget w) { return 0; }

static void try24bitVisual (Visual** visual, int* depth, Colormap* cmap)
{
  XVisualInfo vinfo;

  if (XMatchVisualInfo(iupmot_display, XDefaultScreen(iupmot_display), 24, TrueColor,
                       &vinfo) != 0)
  {
    *visual = vinfo.visual;
    *depth = vinfo.depth;
    *cmap = XCreateColormap(iupmot_display,
               RootWindow(iupmot_display, XDefaultScreen(iupmot_display)),
               vinfo.visual, AllocNone);
  }
  else
  {
    *visual = iupmot_visual;
    *depth = iupmot_depth;
    *cmap = iupmot_color.colormap;
  }
}

void iupmotCreateCanvasWithClass( Ihandle* n, WidgetClass c, ... )
{
   Arg arg[30];
   int nargs = 0;
   Widget scroll;
   Widget sbh = 0;
   Widget sbv = 0;
   Iwidgetdata *d = XtNew( Iwidgetdata );

   d->ihandle = n;
   d->data    = 0;

   scroll = XtVaCreateManagedWidget( resourceStr(),
    xmFormWidgetClass, iupmot_parent_widget,
    XmNx, (XtArgVal) posx(n),
    XmNy, (XtArgVal) posy(n),
    XmNwidth, (XtArgVal) currentwidth(n),
    XmNheight, (XtArgVal) currentheight(n),
    XmNtraversalOn, True,
    XmNresizePolicy, XmRESIZE_NONE,
    XmNinsertPosition, order,
    NULL);
   XtAddEventHandler(scroll, SubstructureNotifyMask, False,
                     (XtEventHandler)iupmotCBcreatecanvas, (XtPointer)n);

   {
     String   name;
     XtArgVal value; 
     va_list  va;

     XtSetArg( arg[nargs], XmNmarginWidth, 0 ); nargs++;
     XtSetArg( arg[nargs], XmNmarginHeight, 0 ); nargs++;
     XtSetArg( arg[nargs], XmNresizePolicy, XmRESIZE_NONE ); nargs++;
     XtSetArg( arg[nargs], XmNuserData, d ); nargs++;

     va_start (va, c);
    
     while ( nargs<30 )
     {
       name=va_arg(va,String);
       if (!name) break;
       value = va_arg(va,XtArgVal);
       XtSetArg( arg[nargs], name, value ); 
       nargs++;
     }
    
     va_end (va);

   }

   handle(n) = XtCreateManagedWidget ("canvas", c, scroll, arg, nargs );

   XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);
   XtAddCallback( (Widget)handle(n), XmNexposeCallback, iupmotCBrepaint, NULL );
   XtAddCallback( (Widget)handle(n), XmNresizeCallback, iupmotCBresize,  NULL );
   XtAddCallback( (Widget)handle(n), XmNinputCallback,  iupmotCBinput,   NULL );

   XtAddEventHandler((Widget)handle(n), FocusChangeMask,   False,
                     (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), KeyPressMask,      False,
                     (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), KeyReleaseMask,    False,
                     (XtEventHandler)iupmotCBkeyrelease,   (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), EnterWindowMask,   False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask,   False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), PointerMotionMask, False,
                     (XtEventHandler)iupmotCBmotion,     (XtPointer)n);

   XtVaSetValues( (Widget) handle(n),
                  XmNtopAttachment, XmATTACH_FORM,
                  XmNleftAttachment, XmATTACH_FORM,
                  NULL);

   if (iupCheck(n,"SCROLLBAR")==YES ||
       iupCheck(n,IUP_SBH)==YES       || 
       iupStrEqual(IupGetAttribute(n,"SCROLLBAR"),IUP_HORIZONTAL))
   {
      sbh = XtVaCreateManagedWidget( "horizontal",
                xmScrollBarWidgetClass, scroll,
                XmNorientation, XmHORIZONTAL,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
                NULL );

      XtAddCallback( sbh, XmNvalueChangedCallback, iupmotCBscrollbar, (void*)IUP_SBPOSH );
      XtAddCallback( sbh, XmNdragCallback, iupmotCBscrollbar, (void*)IUP_SBDRAGH );
      XtAddCallback( sbh, XmNdecrementCallback, iupmotCBscrollbar, (void*)IUP_SBLEFT );
      XtAddCallback( sbh, XmNincrementCallback, iupmotCBscrollbar, (void*)IUP_SBRIGHT );
      XtAddCallback( sbh, XmNpageDecrementCallback, iupmotCBscrollbar, (void*)IUP_SBPGLEFT );
      XtAddCallback( sbh, XmNpageIncrementCallback, iupmotCBscrollbar, (void*)IUP_SBPGRIGHT );

      XtVaSetValues( (Widget) handle(n),
                     XmNbottomAttachment, XmATTACH_WIDGET,
                     XmNbottomOffset, 3,
                     XmNbottomWidget, sbh,
                     NULL);

      iupdrvSetAttribute(n, IUP_POSX, iupGetEnv(n, IUP_POSX ));
   }
   else
   {
      XtVaSetValues( handle(n), XmNbottomAttachment, XmATTACH_FORM, NULL);
   }

   if (iupCheck(n,"SCROLLBAR")==YES ||
       iupCheck(n,IUP_SBV)==YES       || 
       iupStrEqual(IupGetAttribute(n,"SCROLLBAR"),IUP_VERTICAL))
   {
      sbv = XtVaCreateManagedWidget( "vertical",
                xmScrollBarWidgetClass, scroll,
                XmNorientation, XmVERTICAL,
                XmNrightAttachment, XmATTACH_FORM,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                NULL );

      XtAddCallback( sbv, XmNvalueChangedCallback, iupmotCBscrollbar, (void*)IUP_SBPOSV );
      XtAddCallback( sbv, XmNdragCallback, iupmotCBscrollbar, (void*)IUP_SBDRAGV );
      XtAddCallback( sbv, XmNdecrementCallback, iupmotCBscrollbar, (void*)IUP_SBUP );
      XtAddCallback( sbv, XmNincrementCallback, iupmotCBscrollbar, (void*)IUP_SBDN );
      XtAddCallback( sbv, XmNpageDecrementCallback, iupmotCBscrollbar, (void*)IUP_SBPGUP );
      XtAddCallback( sbv, XmNpageIncrementCallback, iupmotCBscrollbar, (void*)IUP_SBPGDN );

      XtVaSetValues( (Widget) handle(n),
                     XmNrightAttachment, XmATTACH_WIDGET,
                     XmNrightOffset, 3,
                     XmNrightWidget, sbv,
                     NULL);

      iupdrvSetAttribute(n, IUP_POSY, iupGetEnv(n, IUP_POSY ));
   }
   else
   {
      XtVaSetValues( (Widget) handle(n), XmNrightAttachment, XmATTACH_FORM, NULL);
   }
 
   if (sbv && sbh)
   {
      Dimension x, y;
      XtVaGetValues( sbh, XmNheight, &y, NULL);
      XtVaGetValues( sbv, XmNwidth, &x, NULL);
      XtVaSetValues( sbh, XmNrightOffset, x+3, NULL);
      XtVaSetValues( sbv, XmNbottomOffset, y+3, NULL);
   }
   
   iupmotSetAttrs (n);
}

static void createCanvas (Ihandle *n)
{
  Visual*  visual;
  int      depth;
  Colormap cmap;

#if (XmVERSION >= 2)
   static char canvasTranslations[] = "Ctrl<Btn1Down>: DrawingAreaInput()"
                                      "                ManagerGadgetArm()";
   static XtTranslations CanvasTranslations;
   CanvasTranslations = XtParseTranslationTable(canvasTranslations);
#endif

  try24bitVisual(&visual, &depth, &cmap);
  iupmotCreateCanvasWithClass( n, xmCanvasWidgetClass, 
                                         XmNvisual, (XtArgVal)visual,
                                         XmNdepth, (XtArgVal)depth, 
                                         XmNcolormap, (XtArgVal)cmap, 
                                         NULL );

#if (XmVERSION >= 2)
   XtOverrideTranslations((Widget)handle(n), CanvasTranslations);
#endif
}

static void createDialog (Ihandle *n)
{
   Widget form, area;
   Iwidgetdata *d = XtNew (Iwidgetdata);
   Arg args[10];
   int i = 0;

   d->ihandle = n;
   d->data = HIDE;

#if _TEST_CODE_
   if (XmIsMotifWMRunning(iupmot_initialwidget))
#endif
   {
      int hints = 0;
      if (iupGetEnv(n, IUP_TITLE))
         hints |= MWM_DECOR_TITLE;
      if (iupCheck(n,IUP_MENUBOX))
         hints |= MWM_DECOR_MENU;
      if (iupCheck(n,IUP_MINBOX))
         hints |= MWM_DECOR_MINIMIZE;
      if (iupCheck(n,IUP_MAXBOX))
         hints |= MWM_DECOR_MAXIMIZE;
      if (iupCheck(n,IUP_RESIZE))
         hints |= MWM_DECOR_RESIZEH;
      if (iupCheck(n,IUP_BORDER))
         hints |= MWM_DECOR_BORDER;
      
      XtSetArg (args[i], XmNmwmDecorations, hints);  i++;
   }

   XtSetArg (args[i], XmNmappedWhenManaged, False);  i++;
   XtSetArg (args[i], XmNdeleteResponse, XmDO_NOTHING);  i++;
   XtSetArg (args[i], XmNvisual, iupmot_visual); i++;
   XtSetArg (args[i], XmNdepth, iupmot_depth); i++;
   XtSetArg (args[i], XmNcolormap, iupmot_color.colormap); i++;
   XtSetArg (args[i], XmNallowShellResize, True); i++;

   handle(n) = XtAppCreateShell (" ", "dialog",
               topLevelShellWidgetClass, iupmot_display,
               args, i);

   {
      static Atom wm_deletewindow = 0;
      if (wm_deletewindow == 0)
         wm_deletewindow = XmInternAtom(iupmot_display,
                              "WM_DELETE_WINDOW", False);
      XmAddWMProtocolCallback((Widget)handle(n), wm_deletewindow,
                           iupmotCBclose, (XtPointer)n);
   }

   form = XtVaCreateManagedWidget( "dialog_form",
                xmFormWidgetClass, (Widget)handle(n),
                XmNresizePolicy, XmRESIZE_NONE,
                NULL);

    area = (void*)XtVaCreateManagedWidget ("dialog_area",
        xmDrawingAreaWidgetClass, form,
        XmNx, (XtArgVal) 0,
        XmNy, (XtArgVal) 0,
        XmNwidth, (XtArgVal) currentwidth(n),
        XmNheight, (XtArgVal) currentheight(n),
        XmNmarginWidth, 0,
        XmNmarginHeight, 0,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNnoResize, iupCheck(n,IUP_RESIZE)? False: True,
        XmNrightAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        XmNuserData, d,
        XmNnavigationType, XmTAB_GROUP,
        NULL);

   XtAddCallback (area, XmNresizeCallback, iupmotCBdialog, NULL);

   XtAddEventHandler((Widget)handle(n), FocusChangeMask, False,
                     (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), KeyPressMask,    False,
                     (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), StructureNotifyMask, False,
                     (XtEventHandler)iupmotCBstructure, (XtPointer)n);

   iupmotSetAttrs (n);
}

static Ihandle* findFirstToggle(Ihandle *n)
{
   Ihandle* c;

   if (!n) return NULL;

  foreachchild(c, n)
  {
     if(type(c) == TOGGLE_)
       return c;
     else
     {
       Ihandle *tog = findFirstToggle(c);   
       if (tog) 
       return tog;
     }
  }
  return NULL;
}

static void createRadio (Ihandle *n)
{
   Ihandle *on;

   handle(n) = iupmot_parent_widget;

   iupdrvCreateObjects (child(n));

   iupmotSetAttrs(n);

   on = iupmotFindRadioOption(n);
   if (!on)
   {
      on = findFirstToggle(n);
      if (!on) return;
      IupSetAttribute(on, IUP_VALUE, IUP_ON);
   }
   iupSetEnv(n, IUP_VALUE, IupGetName(on));
}

static void createFrame (Ihandle *n)
{
  Iwidgetdata *d = XtNew (Iwidgetdata);
  char *title = iupGetEnv(n, IUP_TITLE);
  Widget *gad;
  int dx = 0, dy = 0;
 
  d->ihandle = n;
  d->data = malloc (sizeof (Widget) * 5);
  gad = (Widget*)d->data;
  if (!d->data)
    return;

  if (!title)
    gad[4] = NULL;
  else
  {
    int w, h;

    iupdrvStringSize (n, title, &w, &h);
    if (w > currentwidth(n)-15) w = currentwidth(n)-15;
    dy = h/2;
    dx = w + 10;

    gad[4] = XtVaCreateManagedWidget (resourceStr (),
      xmLabelWidgetClass, iupmot_parent_widget,
      XmNx, (XtArgVal) posx(n) + 5,
      XmNy, (XtArgVal) posy(n),
      XmNwidth, (XtArgVal) w+4,
      XmNheight, (XtArgVal) h,
      XmNalignment, XmALIGNMENT_BEGINNING,
      XmNrecomputeSize, False,
      NULL);
  }

  /*
          x                   x+w-2
                     3

y+dy      +--------------------+
          |+------------------+|
          ||                  ||
          ||                  ||
     0    ||                  ||   2
          ||                  ||
          ||                  ||
          ||                  ||
y+dy+h-2  |+------------------+|
          +--------------------+

                     1
  */

  gad[3] = XtVaCreateManagedWidget (resourceStr (),
    xmSeparatorWidgetClass, iupmot_parent_widget,
    XmNx, (XtArgVal) posx(n) + dx + 1,
    XmNy, (XtArgVal) posy(n) + dy,
    XmNwidth, (XtArgVal) currentwidth (n) - dx - 1,
    XmNseparatorType, XmSHADOW_ETCHED_IN,
    XmNorientation, XmHORIZONTAL,
    XmNmargin, 0,
    XmNuserData, d,
    NULL);

  gad[2] = XtVaCreateManagedWidget (resourceStr (),
    xmSeparatorWidgetClass, iupmot_parent_widget,
    XmNx, (XtArgVal) posx(n) + currentwidth (n) - 2,
    XmNy, (XtArgVal) posy(n) + dy + 2,
    XmNheight, (XtArgVal) currentheight (n) - dy - 3,
    XmNseparatorType, XmSHADOW_ETCHED_IN,
    XmNorientation, XmVERTICAL,
    XmNmargin, 0,
    XmNuserData, d,
    NULL);

  gad[1] = XtVaCreateManagedWidget (resourceStr (),
    xmSeparatorWidgetClass, iupmot_parent_widget,
    XmNx, (XtArgVal) posx(n) + 2,
    XmNy, (XtArgVal) posy(n) + currentheight (n) - 2,
    XmNwidth, (XtArgVal) currentwidth (n) - 2,
    XmNseparatorType, XmSHADOW_ETCHED_IN,
    XmNorientation, XmHORIZONTAL,
    XmNmargin, 0,
    XmNuserData, d,
    NULL);

  gad[0] = XtVaCreateManagedWidget (resourceStr (),
    xmSeparatorWidgetClass, iupmot_parent_widget,
    XmNx, (XtArgVal) posx(n),
    XmNy, (XtArgVal) posy(n) + dy,
    XmNheight, (XtArgVal) currentheight (n) - dy,
    XmNseparatorType, XmSHADOW_ETCHED_IN,
    XmNorientation, XmVERTICAL,
    XmNmargin, 0,
    XmNuserData, d,
    NULL);

  handle(n) = gad[0];

  iupmotSetAttrs(n);
}

static void createLabel (Ihandle *n)
{
  XtArgVal bmp = getPixmap(n,IUP_IMAGE, 0);
  int separator = 0;
  char* sep = IupGetAttribute(n, IUP_SEPARATOR);
  if (sep)
  {
    if (iupStrEqual(sep, IUP_HORIZONTAL))
      separator = 1;
    else if (iupStrEqual(sep, IUP_VERTICAL))
      separator = 2;
  }

  if (separator)
    handle(n) = XtVaCreateManagedWidget (resourceStr (),
      xmSeparatorWidgetClass, iupmot_parent_widget,
      XmNx, (XtArgVal) posx(n),
      XmNy, (XtArgVal) posy(n),
      XmNwidth,  (XtArgVal) currentwidth(n),
      XmNheight, (XtArgVal) currentheight(n),
      XmNorientation, (separator==1? XmHORIZONTAL: XmVERTICAL),
      NULL);
  else
    handle(n) = XtVaCreateManagedWidget (resourceStr (),
      xmLabelWidgetClass, iupmot_parent_widget,
      XmNx, (XtArgVal) posx(n),
      XmNy, (XtArgVal) posy(n),
      XmNwidth,  (XtArgVal) currentwidth(n),
      XmNheight, (XtArgVal) currentheight(n),
      XmNrecomputeSize, False,
      XmNalignment, XmALIGNMENT_BEGINNING,
      XmNmarginHeight, 0,
      XmNmarginWidth, 0,
      XmNlabelType, (bmp!=XmUNSPECIFIED_PIXMAP)?XmPIXMAP:XmSTRING,
      XmNlabelPixmap, bmp,
      NULL);

   XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);

   iupmotSetAttrs (n);
}

static void createList (Ihandle *n)
{
  Iwidgetdata *d = XtNew (Iwidgetdata);
  int isdropdown = iupCheck(n, IUP_DROPDOWN)==YES? 1: 0;
  int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;

  d->ihandle = n;
  d->data    = 0;

  if (isdropdown || haseditbox)
  {
    Widget cbedit;
    Widget cblist;

    handle (n) = XtVaCreateManagedWidget (resourceStr (),
      xmComboBoxWidgetClass, iupmot_parent_widget,
      XmNx, (XtArgVal)posx(n),
      XmNy, (XtArgVal)posy(n),
      XmNwidth, (XtArgVal)currentwidth(n),
      XmNhighlightThickness, 1,
      XmNuserData, d,
      XmNmarginWidth,  1,
      XmNmarginHeight,  1,
      XmNtraversalOn, True,
#if (XmVERSION < 2 )
      XmNheight, (XtArgVal) currentheight(n),
      XmNcursorPositionVisible, False,
      XmNlabelMarginHeight,	0,
      XmNdropDownOffset, 0,
      XmNshowLabel, False,
      XmNpersistentDropDown, False,
      XmNtwmHandlingOn, False,
      XmNeditable, False,
#else
      XmNcomboBoxType, (haseditbox? (isdropdown? XmDROP_DOWN_COMBO_BOX: XmCOMBO_BOX): XmDROP_DOWN_LIST),
      XmNpositionMode, XmONE_BASED,
      isdropdown? NULL: XmNheight, isdropdown? (XtArgVal)NULL: (XtArgVal)currentheight(n),
#endif
      NULL);

#if (XmVERSION < 2 )
    cbedit = XmComboBoxGetEditWidget(handle(n));
    cblist = XmComboBoxGetEditWidget(handle(n));
#else
    XtVaGetValues((Widget)handle(n), XmNtextField, &cbedit, NULL);
    XtVaGetValues((Widget)handle(n), XmNlist, &cblist, NULL);
#endif                

    XtAddEventHandler(cbedit, FocusChangeMask, False,
                      (XtEventHandler)iupmotCBfocus, (XtPointer)n);
    XtAddEventHandler(cblist, FocusChangeMask, False,
                      (XtEventHandler)iupmotCBfocus, (XtPointer)n);
    XtAddEventHandler(cbedit, KeyPressMask,    False,
                      (XtEventHandler)iupmotCBkeypress, (XtPointer)n);
                      

    XtAddCallback ((Widget)handle(n), XmNselectionCallback, iupmotCBlist, NULL);

    /* XtVaSetValues(cblist, XmNscrollBarDisplayPolicy, XmAS_NEEDED, NULL); */
    {
      int sbstyle = 1;  /* VERTICAL scroll by default */
      char* value = iupGetEnv(n, "SCROLLBAR");
      if (value)
      {
        if (iupStrEqualNoCase(value, "YES"))
          sbstyle = 3;
        else if (iupStrEqualNoCase(value, "HORIZONTAL"))
          sbstyle = 2;
        else if (iupStrEqualNoCase(value, "NO"))
          sbstyle = 0;
      }

      if (!(sbstyle&1)) /* hide vertical scrollbar */
      {
        Widget sb = NULL;
        XtVaGetValues(cblist, XmNverticalScrollBar, &sb, NULL);
        if (sb) XtUnmanageChild(sb);
      }

      if (!(sbstyle&2)) /* hide horizontal scrollbar */
      {
        Widget sb = NULL;
        XtVaGetValues(cblist, XmNhorizontalScrollBar, &sb, NULL);
        if (sb) XtUnmanageChild(sb);
      }
    }
    
    if (haseditbox)
    {
      IupSetAttribute(n, "_IUPMOT_EDITBOX", (char*)cbedit);
      XtVaSetValues (cbedit, XmNuserData, d, NULL);
      XtAddCallback (cbedit, XmNvalueChangedCallback, iupmotCBmask, NULL);
      XtAddCallback (cbedit, XmNmodifyVerifyCallback, iupmotCBtext, NULL);
      XtAddCallback (cbedit, XmNactivateCallback, iupmotCBtext, NULL);
      XtAddCallback (cbedit, XmNmotionVerifyCallback, iupmotCBcaret, NULL);
    }
  }
  else
  {
    Arg args[15];
    int nargs = 0;
    XtSetArg(args[nargs], XmNx, (XtArgVal) posx(n) ); nargs++;
    XtSetArg(args[nargs], XmNy, (XtArgVal) posy(n) ); nargs++;
    XtSetArg(args[nargs], XmNwidth, (XtArgVal) currentwidth(n) ); nargs++;
    XtSetArg(args[nargs], XmNheight, (XtArgVal) currentheight(n) ); nargs++;
    XtSetArg(args[nargs], XmNuserData, (XtArgVal) d ); nargs++;
    XtSetArg(args[nargs], XmNtraversalOn, (XtArgVal) True ); nargs++;
    XtSetArg(args[nargs], XmNshadowThickness, (XtArgVal) 2 ); nargs++;
    XtSetArg(args[nargs], XmNhighlightThickness, (XtArgVal) 1 ); nargs++;
    XtSetArg(args[nargs], XmNspacing, (XtArgVal)1 ); nargs++;
    /* XtSetArg(args[nargs], XmNscrollBarDisplayPolicy, XmAS_NEEDED ); nargs++; */
    handle(n) = XmCreateScrolledList(iupmot_parent_widget, resourceStr(), args, nargs );
    XtManageChild((Widget)handle(n));

    XtAddCallback ((Widget)handle(n), XmNbrowseSelectionCallback, iupmotCBlist, NULL);
    XtAddCallback ((Widget)handle(n), XmNextendedSelectionCallback, iupmotCBlist, NULL);

    XtAddEventHandler((Widget)handle(n), FocusChangeMask, False,
      (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
  }

  XtAddEventHandler((Widget)handle(n), KeyPressMask,    False,
    (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
  XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
    (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
  XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
    (XtEventHandler)iupmotCBenterleave, (XtPointer)n);

  XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);

  iupmotSetAttrs (n);

  if (!iupCheck(n, IUP_MULTIPLE) && !haseditbox)
  {
    int pos;
    char prev_list_pos[10];
    if (iupdrvGetAttribute(n, IUP_VALUE) == NULL)
      IupStoreAttribute(n, IUP_VALUE, "1");
    pos = IupGetInt(n, IUP_VALUE);
    sprintf(prev_list_pos, "%d", pos);
    IupStoreAttribute(n, "_IUPMOT_PREV_LIST_POS", prev_list_pos);
    if (isdropdown)
    {
#if (XmVERSION < 2 )
      XmComboBoxSelectPos((Widget)handle(n), pos, False);
#else
      XtVaSetValues((Widget)handle(n),
                    XmNselectedPosition, pos, NULL );
#endif
    }
    else
      XmListSelectPos((Widget)handle(n), pos, False);
  }
}

static void createText (Ihandle *n)
{
  Iwidgetdata *d = XtNew (Iwidgetdata);

  d->ihandle = n;
  d->data    = 0;

  handle(n) = XtVaCreateManagedWidget (resourceStr(),
    xmTextFieldWidgetClass,   iupmot_parent_widget,
    XmNx, (XtArgVal) posx(n),
    XmNy, (XtArgVal) posy(n),
    XmNwidth, (XtArgVal) currentwidth(n),
    XmNheight, (XtArgVal) currentheight(n),
    XmNtraversalOn, True,
    XmNverifyBell, False,
    XmNhighlightThickness, 1,
    XmNmarginWidth,  1,
    XmNmarginHeight, 1,
    XmNeditable, True,
    XmNeditMode, XmSINGLE_LINE_EDIT,         
    XmNbottomShadowColor, iupmot_shadow,
    XmNtopShadowColor, iupmot_highlight,
    XmNuserData, d,
    NULL);

  XtAddCallback ((Widget)handle(n), XmNvalueChangedCallback, iupmotCBmask, NULL);
  XtAddCallback ((Widget)handle(n), XmNmodifyVerifyCallback, iupmotCBtext, NULL);
  XtAddCallback ((Widget)handle(n), XmNactivateCallback, iupmotCBtext, NULL);
  XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);
  XtAddCallback ((Widget)handle(n), XmNmotionVerifyCallback, iupmotCBcaret, NULL);

  XtAddEventHandler((Widget)handle(n), FocusChangeMask, False,
    (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
  XtAddEventHandler((Widget)handle(n), KeyPressMask,    False,
    (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
  XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
    (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
  XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
    (XtEventHandler)iupmotCBenterleave, (XtPointer)n);

  iupmotSetAttrs (n);
}

static void createMultiline (Ihandle *n)
{
   Arg args[20];
   int nargs = 0;
   Iwidgetdata *d = XtNew (Iwidgetdata);

  int sbstyle = 3;  /* BOTH scrollbars by default */
  char* value = iupGetEnv(n, "SCROLLBAR");
  if (value)
  {
    if (iupStrEqualNoCase(value, "VERTICAL"))
      sbstyle = 1;
    else if (iupStrEqualNoCase(value, "HORIZONTAL"))
      sbstyle = 2;
    else if (iupStrEqualNoCase(value, "NO"))
      sbstyle = 0;
  }

   d->ihandle = n;
   d->data    = 0;

   XtSetArg(args[nargs], XmNx, (XtArgVal) posx(n) ); nargs++;
   XtSetArg(args[nargs], XmNy, (XtArgVal) posy(n) ); nargs++;
   XtSetArg(args[nargs], XmNwidth, (XtArgVal) currentwidth(n) ); nargs++;
   XtSetArg(args[nargs], XmNheight, (XtArgVal) currentheight(n) ); nargs++;
   XtSetArg(args[nargs], XmNuserData, (XtArgVal) d ); nargs++;
   XtSetArg(args[nargs], XmNshadowThickness, (XtArgVal) 2 ); nargs++;
   XtSetArg(args[nargs], XmNhighlightThickness, (XtArgVal) 1 ); nargs++;
   XtSetArg(args[nargs], XmNeditMode, XmMULTI_LINE_EDIT ); nargs++;
   XtSetArg(args[nargs], XmNspacing, (XtArgVal)1 ); nargs++;
   XtSetArg(args[nargs], XmNmarginWidth, (XtArgVal)1 ); nargs++;
   XtSetArg(args[nargs], XmNtopShadowPixmap, XmUNSPECIFIED_PIXMAP); nargs++;
   XtSetArg(args[nargs], XmNbottomShadowPixmap, XmUNSPECIFIED_PIXMAP); nargs++;
   XtSetArg(args[nargs], XmNbottomShadowColor, iupmot_shadow); nargs++;
   XtSetArg(args[nargs], XmNtopShadowColor, iupmot_highlight); nargs++;
   XtSetArg(args[nargs], XmNmarginHeight, (XtArgVal)1 ); nargs++;

   if (iupCheck(n, "WORDWRAP")==YES)
     { XtSetArg(args[nargs], XmNwordWrap, True); nargs++; }

   if (!(sbstyle&1)) /* hide vertical scrollbar */
     { XtSetArg(args[nargs], XmNscrollVertical, False ); nargs++; }

   if (!(sbstyle&2)) /* hide horizontal scrollbar */
     { XtSetArg(args[nargs], XmNscrollHorizontal, False ); nargs++; }

   handle(n) = XmCreateScrolledText(iupmot_parent_widget, resourceStr(), args, nargs );
   XtManageChild((Widget)handle(n));

   XtAddCallback ((Widget)handle(n), XmNmodifyVerifyCallback, iupmotCBtext, NULL);
   XtAddCallback ((Widget)handle(n), XmNactivateCallback, iupmotCBtext, NULL);
   XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);
   XtAddCallback ((Widget)handle(n), XmNmotionVerifyCallback, iupmotCBcaret, NULL);
   
   XtAddEventHandler((Widget)handle(n), FocusChangeMask, False,
                     (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), KeyPressMask,    False,
                     (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);

   /* XtVaSetValues((Widget)handle(n), XmNscrollBarDisplayPolicy, XmAS_NEEDED, NULL); */

   iupmotSetAttrs (n);
}

static void createToggle (Ihandle *n)
{
   Iwidgetdata* d=XtNew(Iwidgetdata);
   int in_radio = iupGetRadio(n)!=NULL;
   XtArgVal xpmRel = getPixmap(n,IUP_IMAGE, 0);
   XtArgVal xpmPress = getPixmap(n,IUP_IMPRESS, 0);
   XtArgVal xpmInactive = getPixmap(n,IUP_IMINACTIVE, 1);
   XmString label = XmStringCreateLocalized( " " );
   int drawImage = (xpmRel != XmUNSPECIFIED_PIXMAP);
   int drawImagePressed = (xpmPress != XmUNSPECIFIED_PIXMAP);
   int drawImageInactive = (xpmInactive != XmUNSPECIFIED_PIXMAP);

   d->ihandle = n;
   d->data    = 0;
 
   handle(n) = XtVaCreateManagedWidget (resourceStr (),
       xmToggleButtonWidgetClass,  iupmot_parent_widget,
       XmNlabelString, label,
       XmNx, (XtArgVal) posx(n),
       XmNy, (XtArgVal) posy(n),
       XmNwidth, (XtArgVal) currentwidth(n),
       XmNheight, (XtArgVal) currentheight(n),
       XmNalignment, drawImage ? XmALIGNMENT_CENTER : XmALIGNMENT_BEGINNING,
       XmNtraversalOn, True,
       XmNhighlightThickness, 1,
       XmNuserData, d,
       XmNindicatorType, in_radio? XmONE_OF_MANY : XmN_OF_MANY,
       XmNnavigationType, XmTAB_GROUP,
       XmNrecomputeSize, False,
       XmNlabelType, drawImage ? XmPIXMAP : XmSTRING,
       XmNlabelPixmap, xpmRel,
       XmNselectPixmap, drawImagePressed ? xpmPress : xpmRel,
       XmNlabelInsensitivePixmap, drawImageInactive ? xpmInactive : xpmRel,
       XmNselectInsensitivePixmap, drawImageInactive ? xpmInactive : xpmRel,
       XmNspacing, drawImage ? 0 : 4,
       XmNshadowThickness, drawImage ? 2 : 0,
#if (XmVERSION >= 2)
       XmNindicatorOn, drawImage ? XmINDICATOR_NONE : XmINDICATOR_FILL,
       XmNtoggleMode, (!drawImage && iupCheck(n, "3STATE")==YES)? XmTOGGLE_INDETERMINATE: XmTOGGLE_BOOLEAN,
#else
       XmNindicatorOn, drawImage ? False : True,
#endif
       NULL);

   XmStringFree( label );

   if (in_radio)
     XtAddCallback ((Widget)handle(n), XmNdisarmCallback, iupmotCBradio, NULL);
   else
     XtAddCallback ((Widget)handle(n), XmNdisarmCallback, iupmotCBtoggle, NULL);
   XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);

   XtAddEventHandler((Widget)handle(n), FocusChangeMask, False,
                     (XtEventHandler)iupmotCBfocus,      (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), KeyPressMask,    False,
                     (XtEventHandler)iupmotCBkeypress,   (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);

   iupmotSetAttrs (n);
}

static void createSeparator (Ihandle *n)
{
   Iwidgetdata *d = XtNew (Iwidgetdata);
   d->ihandle = n;
   d->data    = 0;
   handle(n) =  XtVaCreateManagedWidget (resourceStr (),
                xmSeparatorWidgetClass, iupmot_parent_widget,
                XmNuserData, d,
                NULL);
}

static const int CHECKSIZE = 10;

static void createItem(Ihandle *n)
{
   Arg arg[10];
   int nargs = 0;
   Pixel bgcolor;
   Iwidgetdata *d = XtNew (Iwidgetdata);
   d->ihandle = n;
   d->data    = 0;

   XtSetArg( arg[nargs], XmNtraversalOn,        True ); nargs++;
   XtSetArg( arg[nargs], XmNhighlightThickness, 1 ); nargs++;
   XtSetArg( arg[nargs], XmNindicatorSize,      CHECKSIZE ); nargs++;
   XtSetArg( arg[nargs], XmNrecomputeSize,      1 ); nargs++;
   XtSetArg( arg[nargs], XmNuserData,           d ); nargs++;

   bgcolor = iupmotGetColor( n, IupGetAttribute( n, IUP_BGCOLOR ) );
   if (bgcolor != -1) 
   {
      XtSetArg( arg[nargs], XmNbackground, bgcolor ); nargs++;
   }

   if (XtClass(XtParent(iupmot_parent_widget)) != xmFormWidgetClass)
   {
      XtSetArg( arg[nargs], XmNmarginRight, CHECKSIZE ); nargs++;

      handle(n) =  XtCreateManagedWidget (resourceStr(),
                                          xmToggleButtonWidgetClass, iupmot_parent_widget, arg, nargs );

      XtAddCallback ((Widget)handle(n), XmNvalueChangedCallback, iupmotCBmenu, NULL);
      XtAddCallback ((Widget)handle(n), XmNarmCallback, iupmotCBarmmenu, NULL);
   }
   else
   {
      handle(n) =  XtCreateManagedWidget (resourceStr(),
                                          xmCascadeButtonWidgetClass, iupmot_parent_widget, arg, nargs );

      XtAddCallback ((Widget)handle(n), XmNactivateCallback, iupmotCBmenu, NULL);
   }

   XtAddCallback ((Widget)handle(n), XmNhelpCallback, iupmotCBhelp, NULL);

   XtAddEventHandler((Widget)handle(n), EnterWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   XtAddEventHandler((Widget)handle(n), LeaveWindowMask, False,
                     (XtEventHandler)iupmotCBenterleave, (XtPointer)n);
   iupmotSetAttrs(n);
}

static void createSubmenu (Ihandle *n)
{
   Iwidgetdata *d = XtNew(Iwidgetdata);
   XmString label = XmStringCreateLocalized(IupGetAttribute(n,IUP_TITLE) );
   d->ihandle = n;
   d->data    = 0;
   handle(n) =  XtVaCreateManagedWidget (resourceStr (),
    xmCascadeButtonWidgetClass, iupmot_parent_widget,
    XmNtraversalOn, True,
    XmNhighlightThickness, 1,
    XmNspacing, 0,
    XmNrecomputeSize, True,
    XmNindicatorSize, CHECKSIZE,
    XmNuserData, d,
    XmNlabelString, label,
    NULL);
   XmStringFree( label );
   iupmotSetAttrs(n);
}

static void unmap_menu(Widget w, XtPointer p1, XtPointer p2)
{
  iupmotCBmenu(w, p1, p2);
  iupmot_exitmenuloop = 1;
}

static void createMenu(Ihandle *n)
{
   Iwidgetdata *d = XtNew ( Iwidgetdata );
   d->ihandle = n;
   d->data    = 0;

   if (!parent(n))
   {
      if (iupmot_parent_widget)
      {
         /* menu de barra */
         handle(n) =  XtVaCreateManagedWidget (resourceStr (),
            xmRowColumnWidgetClass, iupmot_parent_widget,
            XmNrowColumnType, XmMENU_BAR,
            XmNmarginHeight, 0,
            XmNmarginWidth, 0,
            XmNorientation, XmHORIZONTAL,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNuserData, d,
            NULL);
      }
      else
      {
         /* menu popup */
         Arg arg[6];
         int args = 0;
   
         XtSetArg (arg[args],XmNuserData,(XtArgVal)d); args++;
         XtSetArg (arg[args], XmNvisual, iupmot_visual); args++;
         XtSetArg (arg[args], XmNdepth, iupmot_depth); args++;
         XtSetArg(arg[args],XmNuserData,(XtArgVal)d); args++;
         /* _TESTCODE_     Lets make it work for all mouse buttons
         XtSetArg(arg[args],XmNwhichButton,Button1); args++;
         */
/* _TESTCODE_
         XtSetArg(arg[args],XmNmenuPost,"BSelect Press"); args++;
*/
         handle(n) = XmCreatePopupMenu(iupmot_initialwidget,resourceStr(),arg,args);
         XtAddCallback((Widget)handle(n), XmNmapCallback, iupmotCBmapmenu, IUP_OPEN_CB);
         XtAddCallback((Widget)handle(n), XmNunmapCallback, unmap_menu, "MENUCLOSE_CB");
      }
   }
   else
   {
      /* submenu */
      Arg arg[4];
      int args = 0;
   
      XtSetArg (arg[args],XmNuserData,(XtArgVal)d); args++;
      XtSetArg (arg[args], XmNvisual, iupmot_visual); args++;
      XtSetArg (arg[args], XmNdepth, iupmot_depth); args++;
      XtSetArg (arg[args], XmNcolormap, iupmot_color.colormap); args++;
      handle(n) = XmCreatePulldownMenu(iupmot_parent_widget,resourceStr(),arg,args);
      XtAddCallback((Widget)handle(n), XmNmapCallback, iupmotCBmapmenu, IUP_OPEN_CB);
      XtAddCallback((Widget)handle(n), XmNunmapCallback, iupmotCBmapmenu, "MENUCLOSE_CB");
      XtVaSetValues( (Widget)handle(parent(n)), XmNsubMenuId, (Widget)handle(n), NULL);
   }
   iupmotSetAttrs(n);
}

void iupmotSetParent( Widget w, Ihandle* h) 
{
  iupmot_parent_widget = w; 
  iupmot_parent = h; 
}

void iupdrvCreateObject(Ihandle *self, Ihandle *owner)
{
   Widget old_widget = iupmot_parent_widget;
   Ihandle *old_parent = iupmot_parent;

   iupmot_parent = owner;
   if (owner)
   {
      if (type(owner) == DIALOG_)
         iupmot_parent_widget = XtNameToWidget( (Widget)handle(owner), "*dialog_area");
      else if (type(owner) == CANVAS_)
      {
         parent(self) = owner;
         iupmot_parent_widget = XtParent((Widget)handle(owner));
      }
   }
   iupdrvCreateObjects(self);

   iupmot_parent_widget = old_widget;
   iupmot_parent = old_parent;
}

void iupdrvCreateObjects (Ihandle *n)
{
  int flag =0;

  if (type(n) != DIALOG_ && iupmot_parent_widget == NULL && 
      type(n) != IMAGE_ && type(n) != MENU_)
  {
    flag = 1;
    iupmot_parent = IupGetDialog(n);
    iupmot_parent_widget = (Widget)handle(iupmot_parent);
  }

  if (hclass(n))
    iupCpiMap(n, iupmot_parent);
  else
  {
    if (!handle(n)) iupdrvCreateNativeObject(n);
  }
  
  if (flag)
  {
    flag = 0;
    iupmot_parent_widget = NULL;
    iupmot_parent  = NULL;
  }
}

void iupdrvCreateNativeObject (Ihandle *n)
{
  Ihandle *c;

  if(type(n) == BUTTON_)
  {
    createButton (n);
  }
  else if(type(n) == CANVAS_)
  {
    createCanvas (n);
  }
  else if(type(n) == FILL_)
  {
    handle(n) = iupmot_parent_widget;
  }
  else if(type(n) == FRAME_)
  {
    createFrame (n);
    iupdrvCreateObjects (child(n));
  }
  else if(type(n) == ITEM_)
  {
    createItem (n);
  }
  else if(type(n) == LABEL_)
  {
    createLabel (n);
  }
  else if(type(n) == LIST_)
  {
    createList (n);
  }
  else if(type(n) == RADIO_)
  {
    createRadio (n);
  }
  else if(type(n) == TEXT_)
  {
    createText (n);
  }
  else if(type(n) == MULTILINE_)
  {
    createMultiline (n);
  }
  else if(type(n) == TOGGLE_)
  {
    createToggle (n);
  }
  else if(type(n) == SEPARATOR_)
  {
    createSeparator (n);
  }
  else if(type(n) == SUBMENU_)
  {
    createSubmenu (n);

    {
      Widget oldparent = iupmot_parent_widget;
      iupmot_parent_widget = (Widget)handle(n);

      iupdrvCreateObjects (child(n));

      iupmot_parent_widget = oldparent;
    }
  }
  else if(type(n) == MENU_)
  {
    createMenu (n);

    {
      Widget oldparent = iupmot_parent_widget;
      iupmot_parent_widget = (Widget)handle(n);

      foreachchild(c, n)
        iupdrvCreateObjects (c);

      iupmot_parent_widget = oldparent;
    }
  }
  else if(type(n) == VBOX_ || type(n) == HBOX_)
  {
    handle(n) = iupmot_parent_widget;
    foreachchild(c,n)
      iupdrvCreateObjects (c);
  }
  else if(type(n) == ZBOX_)
  {
    Iwidgetdata *d = XtNew(Iwidgetdata);
    Ihandle *v;
    d->ihandle = n;
    d->data    = 0;

    handle(n) = iupmot_parent_widget;

    v = iupmotGetZboxValue( n );
    foreachchild(c,n)
    {
      iupdrvCreateObjects(c);
      if (c!=v) IupSetAttribute(c,IUP_VISIBLE,IUP_NO);
    }
    IupSetAttribute(n, "_IUPMOT_PREVIOUS_ZBOX_VALUE", IupGetName(v));

  }
  else if(type(n) == DIALOG_)
  {
    createDialog (n);

    /* the parent of the controls will be the dialog area */
    iupmot_parent_widget = XtNameToWidget( (Widget)handle(n), "*dialog_area");
    iupmot_parent = n;

    iupdrvCreateObjects ( child(n) );

    iupmot_parent_widget = NULL;
    iupmot_parent = NULL;
  }
  else if(type(n) == IMAGE_)
  {
    iupmotCreateImage(n, NULL, 0);
  }

  iupmotUpdateFont(n);
}

static void removeCallbacks (Ihandle *n)
{
  Widget w = (Widget)handle(n);
  if(type(n) == DIALOG_)
  {
    XtRemoveAllCallbacks(XtNameToWidget(w, "*dialog_area"), XmNresizeCallback);
  }
  else if(type(n) == CANVAS_)
  {
    Widget sb;

    XtRemoveAllCallbacks(w, XmNhelpCallback);
    XtRemoveAllCallbacks(w, XmNexposeCallback);
    XtRemoveAllCallbacks(w, XmNresizeCallback);
    XtRemoveAllCallbacks(w, XmNinputCallback);
    if ((sb = XtNameToWidget(w, "vertical")) != NULL)
    {
      XtRemoveAllCallbacks(sb, XmNvalueChangedCallback);
      XtRemoveAllCallbacks(sb, XmNdragCallback);
      XtRemoveAllCallbacks(sb, XmNdecrementCallback);
      XtRemoveAllCallbacks(sb, XmNincrementCallback);
      XtRemoveAllCallbacks(sb, XmNpageDecrementCallback);
      XtRemoveAllCallbacks(sb, XmNpageIncrementCallback);
    }
    if ((sb = XtNameToWidget(w, "horizontal")) != NULL)
    {
      XtRemoveAllCallbacks(sb, XmNvalueChangedCallback);
      XtRemoveAllCallbacks(sb, XmNdragCallback);
      XtRemoveAllCallbacks(sb, XmNdecrementCallback);
      XtRemoveAllCallbacks(sb, XmNincrementCallback);
      XtRemoveAllCallbacks(sb, XmNpageDecrementCallback);
      XtRemoveAllCallbacks(sb, XmNpageIncrementCallback);
    }
  }
  else if(type(n) == ITEM_)
  {
    if (XtClass(XtParent(XtParent(w))) != xmFormWidgetClass) /* if iupmot_parent_widget is not the dialog */
    {
      XtRemoveAllCallbacks(w, XmNvalueChangedCallback);
      XtRemoveAllCallbacks(w, XmNarmCallback);
    }
    else
      XtRemoveAllCallbacks(w, XmNactivateCallback);

    XtRemoveAllCallbacks(w, XmNhelpCallback);
  }
  else if(type(n) == BUTTON_)
  {
    XtRemoveAllCallbacks(w, XmNactivateCallback);
    XtRemoveAllCallbacks(w, XmNhelpCallback);
  }
  else if(type(n) == TOGGLE_)
  {
    XtRemoveAllCallbacks(w, XmNdisarmCallback);
    XtRemoveAllCallbacks(w, XmNhelpCallback);
  }
  else if(type(n) == LIST_ )
  {
    int isdropdown = iupCheck(n, IUP_DROPDOWN)==YES? 1: 0;
    int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
    if (isdropdown || haseditbox)
    {
      XtRemoveAllCallbacks(w, XmNselectionCallback);

      if (haseditbox)
      {
        Widget cbedit = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
        XtRemoveAllCallbacks(cbedit, XmNvalueChangedCallback);
        XtRemoveAllCallbacks(cbedit, XmNmodifyVerifyCallback);
        XtRemoveAllCallbacks(cbedit, XmNactivateCallback);
        XtRemoveAllCallbacks(cbedit, XmNmotionVerifyCallback);
      }
    }
    else
    {
      XtRemoveAllCallbacks(w, XmNbrowseSelectionCallback);
      XtRemoveAllCallbacks(w, XmNmultipleSelectionCallback);
    }
    XtRemoveAllCallbacks(w, XmNhelpCallback);
  }
  else if(type(n) == TEXT_ || type(n) == MULTILINE_)
  {
    if(type(n) == TEXT_)
      XtRemoveAllCallbacks(w, XmNvalueChangedCallback);
    XtRemoveAllCallbacks(w, XmNmodifyVerifyCallback);
    XtRemoveAllCallbacks(w, XmNactivateCallback);
    XtRemoveAllCallbacks(w, XmNhelpCallback);
    XtRemoveAllCallbacks(w, XmNmotionVerifyCallback);
  }
}

static void removeEventHandlers (Ihandle *n)
{
  EventMask mask = NoEventMask;
  if(type(n) == DIALOG_)
  {
    mask |= FocusChangeMask | KeyPressMask | EnterWindowMask |
      LeaveWindowMask | StructureNotifyMask;
  }
  else if(type(n) == CANVAS_)
  {
    mask |= FocusChangeMask | KeyPressMask | EnterWindowMask |
      LeaveWindowMask | PointerMotionMask;
  }
  else if(type(n) == ITEM_ || type(n) == LABEL_)
  {
    mask |= EnterWindowMask | LeaveWindowMask;
  }
  else if(type(n) == BUTTON_ || type(n) == TOGGLE_ ||
          type(n) == MULTILINE_ ||  type(n) == TEXT_)
  {
    /* BUTTON, TOGGLE, TEXT e MULTILINE cadastram os mesmos event handlers */
    mask |= FocusChangeMask | KeyPressMask | EnterWindowMask |
      LeaveWindowMask;
  }
  else if(type(n) == LIST_ )
  {
    int isdropdown = iupCheck(n, IUP_DROPDOWN)==YES? 1: 0;
    int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
    if (isdropdown || haseditbox)
    {
      if (haseditbox)
      {
        Widget cbedit = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
        XtRemoveEventHandler(cbedit, FocusChangeMask, False,
          iupmotCBfocus, (XtPointer)n);
        XtRemoveEventHandler(cbedit, KeyPressMask, False,
          iupmotCBkeypress, (XtPointer)n);
      }
    }
    else
    {
      mask |= FocusChangeMask;
    }

    mask |= KeyPressMask | EnterWindowMask | LeaveWindowMask;
  }

  if ((mask & FocusChangeMask) != 0)
  {
    XtRemoveEventHandler((Widget)handle(n), FocusChangeMask, False,
      iupmotCBfocus, (XtPointer)n);
  }

  if ((mask & KeyPressMask) != 0)
  {
    XtRemoveEventHandler((Widget)handle(n), KeyPressMask, False,
      iupmotCBkeypress, (XtPointer)n);
  }

  if ((mask & EnterWindowMask) != 0)
  {
    XtRemoveEventHandler((Widget)handle(n), EnterWindowMask, False,
      iupmotCBenterleave, (XtPointer)n);
  }

  if ((mask & LeaveWindowMask) != 0)
  {
    XtRemoveEventHandler((Widget)handle(n), LeaveWindowMask, False,
      iupmotCBenterleave, (XtPointer)n);
  }

  if ((mask & PointerMotionMask) != 0)
  {
    XtRemoveEventHandler((Widget)handle(n), PointerMotionMask, False,
      iupmotCBmotion, (XtPointer)n);
  }

  if ((mask & StructureNotifyMask) != 0)
  {
    XtRemoveEventHandler((Widget)handle(n), StructureNotifyMask, False,
      iupmotCBstructure, (XtPointer)n);
  }
}

static void dissociate(Ihandle *n)
{
  /* esta funcao desassocia os elementos Motif 
  dos elementos iup.
  basicamente, libera toda area alocada pelo driver
  associada aos controles e zera o campo handle do iup
  e o atributo userData do motif  */

  int removeHandlers = 1;
  Iwidgetdata *wd = NULL;

  if (handle(n) == NULL) return;

  if(type(n) == DIALOG_)
  {
    Widget area = XtNameToWidget((Widget)handle(n), "*dialog_area");
    if (area) 
    {
      XtVaGetValues (area, XmNuserData, &wd, NULL);
      XtVaSetValues (area, XmNuserData, NULL, NULL);
      XtRemoveEventHandler((Widget)handle(n), StructureNotifyMask, False,
        iupmotCBstructure, (XtPointer)n);
    }
  }
  else if(type(n) == CANVAS_)
  {
    XtRemoveEventHandler(XtParent((Widget)handle(n)),
      SubstructureNotifyMask, False,
      iupmotCBcreatecanvas, (XtPointer)n);

    XtVaGetValues ((Widget)handle(n), XmNuserData, &wd, NULL);
    XtVaSetValues ((Widget)handle(n), XmNuserData, NULL, NULL);
  }
  else if(type(n) == SEPARATOR_ || type(n) == MENU_ || type(n) == ITEM_ ||
    type(n) == SUBMENU_   || type(n) == LABEL_|| type(n) == BUTTON_ ||
    type(n) == TOGGLE_    || type(n) == MULTILINE_ || type(n) == LIST_)
  {
    XtVaGetValues ((Widget)handle(n), XmNuserData, &wd, NULL);
    XtVaSetValues ((Widget)handle(n), XmNuserData, NULL, NULL);
    if(type(n) == LIST_ )
    {
      int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
      if (haseditbox)
      {
        Widget cbedit = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
        XtVaGetValues (cbedit, XmNuserData, &wd, NULL);
        XtVaSetValues (cbedit, XmNuserData, NULL, NULL);
      }
    }
  } 
  else if(type(n) == FRAME_)
  {
    Widget *gad = NULL;
    XtVaGetValues ((Widget)handle(n), XmNuserData, &wd, NULL);
    XtVaSetValues ((Widget)handle(n), XmNuserData, NULL, NULL);
    gad = (Widget *) wd->data;
    if (gad[4]) XtDestroyWidget(gad[4]);
    XtDestroyWidget(gad[3]); gad[3] = NULL;
    XtDestroyWidget(gad[2]); gad[2] = NULL;
    XtDestroyWidget(gad[1]); gad[1] = NULL;
    XtDestroyWidget(gad[0]); gad[0] = NULL;
    free(gad);
    gad = NULL;
  }
  else if(type(n) == TEXT_)
  {

    XtVaGetValues ((Widget)handle(n), XmNuserData, &wd, NULL);
    XtVaSetValues ((Widget)handle(n), XmNuserData, NULL, NULL);
    if (wd->data)
    {
      free(wd->data);
      wd->data = NULL;
    }
  }
  else if(type(n) == IMAGE_ || type(n) == KEYACTION_ ||
    type(n) == FILL_ || type(n) == RADIO_ || type(n) == HBOX_ ||
    type(n) == UNKNOWN_ || type(n) == ZBOX_ || type(n) == VBOX_ ||
    type(n) == USER_ || type(n) == CONTROL_)
  {
    removeHandlers = 0;
  }

  removeCallbacks(n);
  if (removeHandlers) removeEventHandlers(n);

  if (wd)
  {
    free(wd);
    wd = NULL;
  }
  handle(n) = NULL;
}

void iupdrvDestroyHandle(Ihandle *h)
{
  iupSetEnv(h, "_IUPMOT_DESTROY", "YES");

  /* esta funcao so' faz sentido para
  DIALOG, IMAGE, COLOR e MENU sem iupmot_parent_widget */
  if(type(h) == BUTTON_ || type(h) == ITEM_ || type(h) == LABEL_ ||
     type(h) == SEPARATOR_ || type(h) == SUBMENU_ ||
     type(h) == TEXT_ || type(h) == TOGGLE_)
  {
    Widget w = (Widget)handle(h);
    dissociate(h);
    XtDestroyWidget( w );
  }
  else if(type(h) == CANVAS_|| type(h) == MULTILINE_)
  {
    Widget w = (Widget)handle(h);
    dissociate(h);
    /* use iupmot_parent_widget because canvas is inserted in xmFormWidgetClass and 
       multiline is a child of a scrolled window. */
    /* FIXME: NOT WORKING for IupTabs. */
    if (!iupStrEqual(IupGetClassName(h), "tabs"))
      XtDestroyWidget( XtParent(w) );  
  }
  else if(type(h) == LIST_)
  {
    Widget w = (Widget)handle(h);
    dissociate(h);
    if (XtClass(w) == xmComboBoxWidgetClass)
    {
      int haseditbox = iupCheck(h, "EDITBOX")==YES? 1: 0;
      if (haseditbox)
        ; /* FIXME: XtDestroyWidget(w);  NOT WORKING. */
      else
        XtDestroyWidget( w );
    }
    else
      XtDestroyWidget( XtParent(w) );  /* When XmCreateScrolled* use iupmot_parent_widget */
  }
  else if(type(h) == FRAME_)
  {
    dissociate( h );  /* destroy done in dissociate */
  }
  else if(type(h) == DIALOG_)
  {
    Widget w = (Widget)handle(h);

    IupHide(h);

    {
      char *mname = IupGetAttribute(h, IUP_MENU);
      Ihandle *menu = mname? IupGetHandle(mname) : 0;
      if (menu) 
      {
        XtVaSetValues(XtNameToWidget(w, "*dialog_area"), XmNtopAttachment, XmATTACH_FORM, NULL);
        IupDestroy(menu);  
      }
    }

    dissociate(h);
    XtDestroyWidget(XtNameToWidget( w, "*dialog_area"));

    /* FIXME: has a leak here, otherwise crash */
    XtUnrealizeWidget( w );
  }
  else if(type(h) == IMAGE_)
  {
    Pixmap p = (Pixmap)handle(h);
    dissociate( h );
    if (iupGetEnv(h,"_IUPMOT_IMINACTIVE"))
      XFreePixmap(iupmot_display, (Pixmap)iupGetEnv(h,"_IUPMOT_IMINACTIVE"));
    XFreePixmap(iupmot_display, p);
  }
  else if(type(h) == MENU_)
  {
    Widget w = (Widget)handle(h);
    dissociate(h);
    if (!parent(h))
    {
      unsigned char menuType;
      XtVaGetValues(w, XmNrowColumnType, &menuType, NULL);
      if (menuType != XmMENU_POPUP)
        XtDestroyWidget(w);
      else
        XtDestroyWidget(XtParent(w));  /* XmCreatePulldownMenu() create the RowColumn widget as the child of a MenuShell. */
    }
  }
  else
  {
    dissociate(h);
  }
}

