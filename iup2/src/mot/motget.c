/** \file
 * \brief Motif Driver functions to get attributes
 *
 * See Copyright Notice in iup.h
 *  */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <stdio.h>
#include <limits.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>

#if (XmVERSION < 2)
#include "ComboBox1.h"
#else
#include <Xm/ComboBox.h>
#endif

#include "iglobal.h"
#include "idrv.h"
#include "motif.h"

static char* get_active   (Ihandle *n);
static char* get_alignment(Ihandle *n);
static char* get_readonly (Ihandle *n);
static char* get_size     (Ihandle *n);
static char* get_x        (Ihandle *n);
static char* get_y        (Ihandle *n);
static char* get_rastersize(Ihandle *n);
static char* get_value    (Ihandle *n);
static char* get_caret    (Ihandle *n);
static char* get_nc       (Ihandle *n);
static char* get_visible  (Ihandle *n);
static char* get_multiple (Ihandle *n);
static char* get_conid    (Ihandle *n);
static char* get_wid      (Ihandle *n);
static char* get_display  (Ihandle *n);
static char* get_screen   (Ihandle *n);
static char* get_window   (Ihandle *n);
static char* get_font     (Ihandle *n);
static char* get_fgcolor  (Ihandle *n);
static char* get_bgcolor  (Ihandle *n);
static char* get_posx     (Ihandle *n);
static char* get_posy     (Ihandle *n);
static char* get_selectcolor(Ihandle *n);
static char* get_selection(Ihandle *n);
static char* get_selectedtext(Ihandle *n);
static char* get_truecolorcnv(Ihandle *n);

typedef char*(*get_fn)(Ihandle*);

static Itable *attrEnv = NULL;
static Itable *attrDefEnv = NULL;

static struct {
   char *name;
   get_fn get;
   char *def;
} attributes[] = {
   {IUP_ACTIVE,      get_active,       IUP_YES },
   {IUP_ALIGNMENT,   get_alignment,    NULL },
   {IUP_BGCOLOR,     get_bgcolor,      NULL },
   {IUP_BORDER,      NULL,             IUP_YES },
   {IUP_CARET,       get_caret,        NULL },
   {IUP_CONID,       get_conid,        NULL },
   {IUP_CURSOR,      NULL,             IUP_ARROW },
   {IUP_DX,          NULL,             "0.1" },
   {IUP_DY,          NULL,             "0.1" },
   {IUP_FGCOLOR,     get_fgcolor,      NULL },
   {IUP_FONT,        get_font,         NULL },
   {"_IUPMOT_FONT",    get_font,         NULL },
   {IUP_MAX,         NULL,             "1.0" },
   {IUP_MIN,         NULL,             "0.0" },
   {IUP_MULTIPLE,    get_multiple,     IUP_NO },
   {IUP_NC,          get_nc,           "32767" },
   {IUP_POSX,        get_posx,         "0.0" },
   {IUP_POSY,        get_posy,         "0.0" },
   {"DRAWSIZE",      get_rastersize,   NULL },
   {IUP_RASTERSIZE,  get_rastersize,   NULL },
   {"READONLY",      get_readonly,     IUP_NO },
   {IUP_SCROLLBAR,   NULL,             IUP_NO },
   {"SELECTCOLOR",   get_selectcolor,  NULL },
   {IUP_SELECTEDTEXT,get_selectedtext, NULL },
   {IUP_SELECTION,   get_selection,    NULL },
   {IUP_SIZE,        get_size,         NULL },
   {"TRUECOLORCANVAS", get_truecolorcnv,  NULL },
   {IUP_VALUE,       get_value,        NULL },
   {IUP_VISIBLE,     get_visible,      NULL },
   {IUP_WID,         get_wid,          NULL },
   {IUP_X,           get_x,            NULL },
   {"XDISPLAY",      get_display,      NULL },
   {IUP_XMAX,        NULL,             "1.0" },
   {IUP_XMIN,        NULL,             "0.0" },
   {"XSCREEN",       get_screen,       NULL },
   {"XWINDOW",       get_window,       NULL },
   {IUP_Y,           get_y,            NULL },
   {IUP_YMAX,        NULL,             "1.0" },
   {IUP_YMIN,        NULL,             "0.0" },
   {IUP_RESIZE,      NULL,             "YES" }
};

#define NATTRIBUTES (sizeof(attributes)/sizeof(attributes[0]))

static char *ALL_ = "ALL";
static char *QUERY_ = "QUERY";
static char *WIDGET_ = "WIDGET";
static char *NOTDIALOG_ = "NOTDIALOG";

/*
 * Defines wether or not the driver should store or read 
 * (depending on the element) the information from/to
 * the environment. Native elements will probably read
 * values directly from the interface.
 */

#define NSTORE 31

static struct {
  char* attr;
  char* type;
} store[NSTORE]; 


void iupmotGetInitAttTable (void)
{
  int i;

  if (attrEnv) return;

	store[0].attr = IUP_ACTIVE;
	store[0].type = WIDGET_;
	store[1].attr = IUP_FGCOLOR;
	store[1].type = WIDGET_;
	store[2].attr = IUP_BGCOLOR;
	store[2].type = WIDGET_;
	store[3].attr = IUP_WID;
	store[3].type = QUERY_;
	store[4].attr = IUP_X;
	store[4].type = QUERY_;
	store[5].attr = IUP_Y;
	store[5].type = QUERY_;
	store[6].attr = "XSCREEN";
	store[6].type = QUERY_;
	store[7].attr = "XWINDOW";
	store[7].type = QUERY_;
	store[8].attr = "XDISPLAY";
	store[8].type = QUERY_;
	store[9].attr = "SELECTCOLOR";
	store[9].type = TOGGLE_;
	store[10].attr = IUP_VALUE;
	store[10].type = TOGGLE_;
	store[11].attr = IUP_VALUE;
	store[11].type = MULTILINE_;
	store[12].attr = IUP_VALUE;
	store[12].type = TEXT_;
	store[13].attr = IUP_VALUE;
	store[13].type = ITEM_;
	store[14].attr = IUP_VALUE;
	store[14].type = LIST_;
	store[15].attr = IUP_VALUE;
	store[15].type = RADIO_;
	store[16].type = IUP_NC;
	store[16].attr = TEXT_;
	store[17].type = IUP_CARET;
	store[17].attr = TEXT_;
	store[18].type = IUP_CARET;
	store[18].attr = MULTILINE_;
	store[19].type = IUP_SELECTION;
	store[19].attr = TEXT_;
	store[20].type = IUP_SELECTION;
	store[20].attr = MULTILINE_;
	store[21].type = IUP_SELECTEDTEXT;
	store[21].attr = TEXT_;
	store[22].type = IUP_SELECTEDTEXT;
	store[22].attr = MULTILINE_;
	store[23].type = IUP_POSX;
	store[23].attr = CANVAS_;
	store[24].type = IUP_POSY;
	store[24].attr = CANVAS_;
	store[25].type = IUP_CONID;
	store[25].attr = CANVAS_;
	store[26].type = IUP_ALIGNMENT;
	store[26].attr = BUTTON_;
	store[27].type = IUP_MULTIPLE;
	store[27].attr = LIST_;
	store[28].type = IUP_ALIGNMENT;
	store[28].attr = LABEL_;
	store[29].type = IUP_ALIGNMENT;
	store[29].attr = TEXT_;
	store[30].type = IUP_ALIGNMENT;
	store[30].attr = MULTILINE_;

  attrEnv = iupTableCreate(IUPTABLE_STRINGINDEXED);
  attrDefEnv = iupTableCreate(IUPTABLE_STRINGINDEXED);
  for (i=0; i<NATTRIBUTES; i++)
  {
    iupTableSet(attrEnv, attributes[i].name, attributes[i].get, IUP_POINTER);
    iupTableSet(attrDefEnv, attributes[i].name, attributes[i].def, IUP_POINTER);
  }
}

char *iupdrvGetAttribute (Ihandle *n, const char *a)
{
   get_fn getfunc;
 
   if (n == NULL || handle(n) == NULL || a == NULL)
      return NULL;
 
   getfunc = (get_fn)iupTableGet(attrEnv, a);
 
   if (getfunc != NULL)
      return (getfunc)(n);
   return NULL;
}

char *iupdrvGetDefault (Ihandle *n, const char *a)
{
   char *def;

   if (n == NULL || a == NULL)
      return NULL;
 
   if (iupStrEqual(a, IUP_FONT) || iupStrEqual(a, "_IUPMOT_FONT"))
   {
	  return iupmot_fontname; 
   }
   else if (iupStrEqual(a, "EXPAND"))
   {
     if(type(n) == FILL_  || type(n) == CANVAS_ ||
				type(n) == FRAME_ || type(n) == HBOX_   ||
				type(n) == RADIO_ || type(n) == VBOX_   ||
				type(n) == ZBOX_)
       return IUP_YES;
     else
       return IUP_NO;
   }  

   def = (char *)iupTableGet(attrDefEnv, a);

   return def;
}

int iupdrvStoreAttribute (Ihandle *n, const char *a)
{
  int i;

  if (n == NULL || a == NULL || hclass(n))
    return 1;

  i = -1;
  for (i=0; i<NSTORE; i++)
  {
    char *type = store[i].type;
    if(type == ALL_ || type == QUERY_)
    {
      if (iupStrEqual(a, store[i].attr)) return 0;
      if (type(n) != DIALOG_ && iupStrEqual(a, store[i].attr)) return 0;
      if ((type(n)==MULTILINE_ || type(n)==CANVAS_ ||
            type(n)==BUTTON_    || type(n)==ITEM_ ||
            type(n)==LABEL_     || type(n)==TEXT_ ||
            type(n)==TOGGLE_    || type(n)==SEPARATOR_ ||
            type(n)==MENU_      || type(n)==SUBMENU_ ) &&
          iupStrEqual(a, store[i].attr))
        return 0;
    }
    else if(type == NOTDIALOG_)
    {
      if (type(n) != DIALOG_ && iupStrEqual(a, store[i].attr)) return 0;
      if ((type(n)==MULTILINE_ || type(n)==CANVAS_ ||
            type(n)==BUTTON_    || type(n)==ITEM_ ||
            type(n)==LABEL_     || type(n)==TEXT_ ||
            type(n)==TOGGLE_    || type(n)==SEPARATOR_ ||
            type(n)==MENU_      || type(n)==SUBMENU_ ) &&
          iupStrEqual(a, store[i].attr))
        return 0;
    }
    else if(type == WIDGET_)
    {
      if ((type(n)==MULTILINE_ || type(n)==CANVAS_ ||
            type(n)==BUTTON_    || type(n)==ITEM_ ||
            type(n)==LABEL_     || type(n)==TEXT_ ||
            type(n)==TOGGLE_    || type(n)==SEPARATOR_ ||
            type(n)==MENU_      || type(n)==SUBMENU_ ) &&
          iupStrEqual(a, store[i].attr))
        return 0;
    }
    else
    {
      if (store[i].type == type(n) && iupStrEqual(a, store[i].attr))
        return 0;
    }
  }
  return 1;
}

/* getBuffer:
   Esta funcao retorna um buffer para escrever 
   o resultado de um getAttr. 
   Tomar cuidado  para respeitar o
   tamanho maximo dos buffers (BUFFER_SIZE) */

#define BUFFER_SIZE 15
#define N_BUFFERS   10

static char* getBuffer(void)
{
   static char  buffer[N_BUFFERS][BUFFER_SIZE];
   static int   ibuffer = 0;

   ibuffer = (ibuffer+1)%N_BUFFERS;
   return buffer[ibuffer];
}

static char* get_alignment(Ihandle *n)
{
  if(n == NULL) return NULL;

  /* Hardcoded - can't be any diferent... */
  if (type(n) == TEXT_ || type(n) == MULTILINE_)
    return IUP_ALEFT;

  if (type(n) == BUTTON_)
    return IUP_ACENTER;

  /* Read values from the interface */
  if (type(n) == LABEL_)
  {
    unsigned char value;
    XtVaGetValues((Widget)handle(n), XmNalignment, &value, NULL);
    switch (value)
    {
      case XmALIGNMENT_CENTER:    return IUP_ACENTER;
      case XmALIGNMENT_BEGINNING: return IUP_ALEFT;
      case XmALIGNMENT_END:       return IUP_ARIGHT;
    }
  }
  
  return NULL;
}

static char* get_readonly (Ihandle *n)
{
  if (type(n) == TEXT_ || type(n) == MULTILINE_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
  {
    Boolean rw;
    Widget w = (Widget)handle(n);
    if (type(n) == LIST_)
      w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
    XtVaGetValues(w, XmNeditable, &rw, NULL);
    return rw ? IUP_NO : IUP_YES;
  }
  return NULL;
}

static char* get_active(Ihandle *n)
{
   int value = 0;
   Widget w[6];
 
   if (iupmotGetWidgetHandles(n, w) == 0) return NULL;
 
   if (type(n) == FRAME_)
   {
      if (w[4]) value = XtIsSensitive (w[4]);
      else
      {
        char *act = iupGetEnv( n, IUP_ACTIVE );
        if (!act || !iupStrEqual(act, IUP_NO)) value = 1;
      }
   }
   else
   {
      if (w[0]) value = XtIsSensitive (w[0]);
   }

   return value ? IUP_YES : IUP_NO;
}

static char* get_size(Ihandle *n)
{
   char *size = getBuffer();
   int charWidth, charHeight;
   Dimension width, height;
   Widget w[6];
 
   if (iupmotGetWidgetHandles(n, w) == 0) return NULL;

   iupdrvGetCharSize(n, &charWidth, &charHeight);

   if (type(n)==DIALOG_)
      XtVaGetValues(w[2], XmNwidth, &width, XmNheight, &height, NULL);
   else
      XtVaGetValues(w[0], XmNwidth, &width, XmNheight, &height, NULL);

   sprintf(size, "%dx%d", width*4/charWidth, height*8/charHeight);
   return size;
}

static char* get_rastersize(Ihandle *n)
{
   char *size = getBuffer();
   Dimension width, height;
   Widget w[6];
 
   if (iupmotGetWidgetHandles(n, w) == 0) return NULL;

   if (type(n)==DIALOG_)
      XtVaGetValues(w[2], XmNwidth, &width, XmNheight, &height, NULL);
   else
      XtVaGetValues(w[0], XmNwidth, &width, XmNheight, &height, NULL);

   sprintf(size, "%dx%d", width, height);
   return size;
}

static char* get_x(Ihandle *n)
{
   char *size = getBuffer();
   Position x=0;
   Widget w[6];
   Window child;
 
   if (iupmotGetWidgetHandles(n, w) == 0) return NULL;

   /* Translating to absolute screen coordinates */
   if(type(n) != DIALOG_)
   { 
     int newx, newy;
     XTranslateCoordinates(iupmot_display, XtWindow((Widget) handle(n)),
       RootWindow(iupmot_display, iupmot_screen), 0, 0, &newx, &newy, &child);
     x = newx;
   }
   else
     XtVaGetValues(w[0], XmNx, &x, NULL);

   sprintf(size, "%d", x);
   return size;
}

static char* get_y(Ihandle *n)
{
   char *size = getBuffer();
   Position y=0;
   Widget w[6];
   Window child;
 
   if (iupmotGetWidgetHandles(n, w) == 0) return NULL;

   if(type(n) != DIALOG_)
   {
     int newx=0, newy=0;
     /* Translating to absolute screen coordinates */
     XTranslateCoordinates(iupmot_display, XtWindow((Widget) w[0]),
       RootWindow(iupmot_display, iupmot_screen), 0, 0, &newx, &newy, &child);
     y = newy;
   }
   else
     XtVaGetValues(w[0], XmNy, &y, NULL);

   sprintf(size, "%d", y);
   return size;
}

static char* getComboValue(Ihandle *n)
{
   char *value;
   int pos = 1;

#if (XmVERSION < 2 )
   pos = XmComboBoxGetSelectedPos((Widget)handle(n));
   if (pos==0) pos=1;
#else
   XtVaGetValues((Widget)handle(n), XmNselectedPosition, &pos, NULL);
#endif

   value = getBuffer();
   sprintf(value, "%d", pos);
   return value;
}

static char* getListValue(Ihandle *n)
{
   static char *value = NULL;
   static int maxchar = 31;  /* qtos caracteres podem ser armazenados */
   int *pos, no;
   int multiple = iupStrEqualNoCase(get_multiple(n),IUP_YES);

   if ( !XmListGetSelectedPos( (Widget)handle(n), &pos, &no ))
      no = 0;

   if (value == NULL) value = (char *)malloc(maxchar+1);

   if (multiple)
   {
      int i, size = 0;
      XtVaGetValues((Widget)handle(n), XmNitemCount, &size, NULL);
      if (!size) return NULL;
      if (maxchar < size)
      {
        value = (char*)realloc( value, size+1 );
      }
      maxchar = size;

      for (i=0; i<size; i++) value[i]='-';
      value[size] = 0;
      for (i=0; i<no; i++)
      {
         value[pos[i]-1] = '+';
      }
   }
   else
   {
      if (no==0) return NULL;
      sprintf(value, "%d", pos[0] );
   }
   if (no) XtFree((XtPointer)pos);
   return value;
}

static char *iupmotGetMemory(const char *s)
{         
  int size;
  char* str;
  if (!s) return NULL;
  size = strlen(s);
  str = iupStrGetMemory(size);
  memcpy(str, s, size);
  XtFree((XtPointer)s);
  return str;
}

static char* get_value(Ihandle *n)
{
  if(type(n) == LIST_)
  {
    if (XtClass((Widget)handle(n)) == xmComboBoxWidgetClass)
    {
      int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
      if (haseditbox)
        return iupmotGetMemory(XmTextFieldGetString((Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX")));
      else
        return getComboValue(n);
    }
    else
      return getListValue(n);
  }
  else if(type(n) == TEXT_)
  {
    return iupmotGetMemory(XmTextFieldGetString((Widget)handle(n)));
  }
  else if(type(n) == MULTILINE_)
  {
    return iupmotGetMemory(XmTextGetString((Widget)handle(n)));
  }
  else if(type(n) == ITEM_)
  {
    unsigned char val;
    if (XtClass((Widget)handle(n))!=xmToggleButtonWidgetClass)
      return IUP_OFF;

    XtVaGetValues ((Widget)handle(n), XmNset, &val, NULL);
    if (val == True)
      return IUP_ON;
    else
      return IUP_OFF;
  }
  else if(type(n) == TOGGLE_)
  {
    unsigned char val = 0;
    XtVaGetValues ((Widget)handle(n), XmNset, &val, NULL);
    if (val == XmINDETERMINATE)
      return "NOTDEF";
    else if (val == XmSET)
      return IUP_ON;
    else
      return IUP_OFF;
  }
  else if(type(n) == RADIO_)
  {
    Ihandle *c=iupmotFindRadioOption (n);
    if (c == NULL)
      return NULL;
    else
      return IupGetName(c);
  }
  else if(type(n) == ZBOX_)
  {
    Ihandle *v = iupmotGetZboxValue(n);
    if (v) return IupGetName(v);
  }

  return NULL;
}

static char* get_selectedtext(Ihandle *n)
{
   if (type(n) == TEXT_)
     return iupmotGetMemory(XmTextFieldGetSelection( (Widget)handle(n) ) );
   else if (type(n) == MULTILINE_)
     return iupmotGetMemory(XmTextGetSelection( (Widget)handle(n) ) );
   else if (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES)
     return iupmotGetMemory(XmTextFieldGetSelection( (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX") ) );

   return NULL;
}

void iupmotLincol( char *str, long int pos, long int *lin, long int *col )
{
  *lin = 1;
  *col = 1;

  for (; pos > 0 ; str++, pos--, (*col)++)
  {
    if (*str == '\n')
    {
      (*lin)++;
      *col=0;
    }
  }
}

static char* get_selection(Ihandle *n)
{
   if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
     XmTextPosition left, right;
     Widget w = (Widget)handle(n);
     if (type(n) == LIST_)
       w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
     if (XmTextFieldGetSelectionPosition(w, &left, &right))
     {
        static char val[20];
        sprintf(val,"%ld:%ld",(long int)left+1, (long int)right+1-1);
        return val;
     }
   }
   else if (type(n) == MULTILINE_)
   {
     XmTextPosition left, right;
     if (XmTextGetSelectionPosition((Widget)handle(n), &left, &right))
     {
        static char val[40];
        long int l1, l2, r1, r2;
        char *str = XmTextGetString((Widget)handle(n));
        iupmotLincol( str, left,  &l1, &l2 );
        iupmotLincol( str, right, &r1, &r2 );
        sprintf(val,"%ld,%ld:%ld,%ld",l1, l2, r1, r2-1);
        XtFree(str);
        return val;
     }
   }
   return NULL;
}

static char* get_caret(Ihandle *n)
{
   if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
     static char val[10];
     XmTextPosition caret;
     Widget w = (Widget)handle(n);
     if (type(n) == LIST_)
       w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

     caret = XmTextGetInsertionPosition(w);
     
     sprintf(val,"%ld",(long int)caret+1);
     return val;
   }
   else if (type(n) == MULTILINE_)
   {
        char *val = getBuffer();
        long int linText, colText;
        XmTextPosition caret = XmTextGetInsertionPosition((Widget)handle(n));
        char *str = XmTextGetString((Widget)handle(n));
        iupmotLincol( str, caret, &linText, &colText );
        sprintf(val, "%ld,%ld", linText, colText);
        XtFree(str);
        return (val);
   }
   else return NULL;
}

static char* get_nc(Ihandle *n)
{
   if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
      char *nc = getBuffer();
      int max;
      Widget w = (Widget)handle(n);
      if (type(n) == LIST_)
        w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
      XtVaGetValues(w, XmNmaxLength, &max, NULL);
      sprintf(nc, "%d", max);
      return nc;
   }
   return NULL;
}

static char* get_visible(Ihandle *n)
{
   XWindowAttributes wa;
   if(n == NULL || handle(n) == NULL) return NULL;
   if (XtWindow((Widget)handle(n))==0) return NULL;
   /* For elements that are not native... */
   if (type(n)==RADIO_ || type(n)==VBOX_ || type(n)==HBOX_ || type(n)==ZBOX_ || type(n)==FRAME_)
   {
     char *vis = iupGetEnv(n, IUP_VISIBLE);
     /* If this element has no visibility set then the user has never changed it
      * so it must be visible. */
     if(vis == NULL)
       return IUP_YES;
     else 
       return vis;
   }
   XGetWindowAttributes(XtDisplay((Widget)handle(n)),XtWindow((Widget)handle(n)),&wa);
   if (wa.map_state!=IsUnmapped)
      return IUP_YES;
   else
      return IUP_NO;
}

static char* get_multiple(Ihandle *n)
{
   if (type(n) == LIST_)
   {
      if (XtClass((Widget)handle(n)) == xmComboBoxWidgetClass)
         return IUP_NO;
      else
      {
         unsigned char value;
         XtVaGetValues((Widget)handle(n), XmNselectionPolicy, &value, NULL);
         if (value == XmBROWSE_SELECT)
            return IUP_NO;
         else
            return IUP_YES;
      }
   }
   return NULL;
}

static char* get_conid(Ihandle *n)
{
   if (type(n) == CANVAS_ && XtWindow((Widget)handle(n))!=0)
   {
      static char conid[60];
      sprintf (conid, "%p %lu %d %d %d %d",
               XtDisplay((Widget)handle(n)),
               XtWindow((Widget)handle(n)),
               0, 0, currentwidth(n), currentheight(n));
      return conid;
   }
   return NULL;
}

static char* get_wid(Ihandle *n)
{
  return (char *) handle(n);
}

static char* get_display(Ihandle *n)
{
   return (char*)XtDisplay((Widget)handle(n));
}

static char* get_screen(Ihandle *n)
{
   return (char*)XtScreen((Widget)handle(n));
}

static char* get_window(Ihandle *n)
{
   return (char*)XtWindow((Widget)handle(n));
}

static char* get_truecolorcnv(Ihandle *n)
{
  static char truecolor[2];
  sprintf(truecolor, "%d", !iupmot_using8bitcanvas);
  return truecolor;
}

static char* get_selectcolor(Ihandle *n)
{
   if (handle(n) && XtClass(((Widget) handle(n)))==xmToggleButtonWidgetClass)
   {
     unsigned char r, g, b;
     Pixel pixel;
     char *cor = getBuffer();
     XtVaGetValues( (Widget) handle(n), XmNselectColor, &pixel, NULL); 
     iupmot_GetRGB(pixel, &r, &g, &b);
     sprintf(cor, "%d %d %d", r, g, b);
     return cor;
   }
   return NULL; 
}

static char* get_fgcolor(Ihandle *n)
{
   Widget w[6];
   XColor xcor;
   Colormap cmap;
   char *cor = getBuffer();
 
   if (iupmotGetWidgetHandles(n, w) == 0) return NULL;
 
   if (type(n) == DIALOG_)
   {
      XtVaGetValues(w[1], XmNcolormap, &cmap, XmNforeground, &xcor.pixel, NULL); 
   } 
   else
   {
      XtVaGetValues(w[0], XmNcolormap, &cmap, XmNforeground, &xcor.pixel, NULL); 
   }
   XQueryColor(iupmot_display,
               cmap,
               &xcor );
   sprintf(cor, "%d %d %d", xcor.red/257, xcor.green/257, xcor.blue/257);
   return cor;
}

static char* get_font(Ihandle *n)
{
  /* FONT and _IUPMOT_FONT return driver font name */
  return iupGetEnv(n, "_IUPMOT_FONT");
}

static char* get_bgcolor(Ihandle *n)
{
   Widget w[6];
   XColor xcor;
   Colormap cmap;
   char *cor = getBuffer();
 
  if (iupmotGetWidgetHandles(n, w) == 0) return NULL;
 
  if (type(n) == DIALOG_ || type(n) == MULTILINE_ ||
      type(n) == CANVAS_ || (type(n) == LIST_ && w[1]) )
  {
    XtVaGetValues(w[1], XmNcolormap, &cmap, XmNbackground, &xcor.pixel, NULL); 
  } 
  else
  {
    XtVaGetValues(w[0], XmNcolormap, &cmap, XmNbackground, &xcor.pixel, NULL); 
  }

  XQueryColor(iupmot_display,
               cmap,
               &xcor );
  sprintf(cor, "%d %d %d", xcor.red/257, xcor.green/257, xcor.blue/257);
  return cor;
}

/* see comment in motset.c - before calc_sbpos */

static char* get_posx(Ihandle *n)
{
   Widget sb;

   if (type(n) != CANVAS_) return NULL;

   sb = XtNameToWidget( XtParent((Widget)handle(n)), "horizontal" );

   if (sb) 
   {
      char *strpos = getBuffer();
      float pos = 0;
      int v;
      float max = IupGetFloat(n, IUP_XMAX);
      float min = IupGetFloat(n, IUP_XMIN);
      XtVaGetValues( sb, XmNvalue, &v, NULL );
      pos = ((max-min)/(float)INT_MAX) * v + min;
      sprintf(strpos, "%.3f", (double)pos);
      return strpos;
   }
   return NULL;
}

static char* get_posy(Ihandle *n)
{
   Widget sb;

   if (type(n) != CANVAS_) return NULL;

   sb = XtNameToWidget( XtParent((Widget)handle(n)), "vertical" );

   if (sb) 
   {
      char *strpos = getBuffer();
      float pos = 0;
      int v;
      float max = IupGetFloat(n, IUP_YMAX);
      float min = IupGetFloat(n, IUP_YMIN);
      XtVaGetValues( sb, XmNvalue, &v, NULL );
      pos = ((max-min)/(float)INT_MAX) * v + min;
      sprintf(strpos, "%.3f", (double)pos);
      return strpos;
   }
   return NULL;
}

Ihandle * iupmotGetZboxValue( Ihandle* n )
{
   if (type(n) == ZBOX_)
   {
      Ihandle *z, *c=IupGetHandle ((char*)iupTableGet(env(n),IUP_VALUE));
      if (c)
         foreachchild( z, n ) if (c==z) return c;
      return child(n);
   }
   return NULL;
}

