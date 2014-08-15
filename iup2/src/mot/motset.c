/** \file
 * \brief Motif Driver set for attributes
 *
 * See Copyright Notice in iup.h
 *  */

#if defined(__STDC__) && defined(ULTRIX)
#define X$GBLS(x)  #x
#endif

#include <assert.h>
#include <string.h>   
#include <stdio.h>
#include <limits.h>   
#include <stdlib.h>   
#include <time.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/List.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/LabelG.h>
#include <X11/cursorfont.h>
#include <Xm/MwmUtil.h> 

#if (XmVERSION < 2)
#include "ComboBox1.h"
#else
#include <Xm/ComboBox.h>
#endif

#include "iglobal.h"
#include "idrv.h"
#include "imask.h"
#include "iupkey.h"
#include "motif.h"

static void set_alignment  (Ihandle* n, char* v);
static void set_mask       (Ihandle* n, char* v);
static void set_multiple   (Ihandle* n, char* v);
static void set_key        (Ihandle* n, char* v);
static void set_image      (Ihandle* n, char* v);
static void set_impress    (Ihandle* n, char* v);
static void set_iminactive (Ihandle* n, char* v);
static void set_nc         (Ihandle* n, char* v);
static void set_border     (Ihandle* n, char* v);
static void set_icon       (Ihandle* n, char* v);
static void set_active     (Ihandle* n, char* v);
static void set_readonly   (Ihandle* n, char* v);
static void set_fgcolor    (Ihandle* n, char* v);
static void set_bgcolor    (Ihandle* n, char* v);
static void set_size       (Ihandle* n, char* v);
static void set_rastersize (Ihandle* n, char* v);
static void set_title      (Ihandle* n, char* v);
static void set_posx       (Ihandle* n, char* v);
static void set_posy       (Ihandle* n, char* v);
static void set_cursor     (Ihandle* n, char* v);
static void set_visible    (Ihandle* n, char* v);
static void set_elemvisible(Ihandle* n, char* v);
static void set_menu       (Ihandle* n, char* v);
static void set_append     (Ihandle* n, char* v);
static void set_insert     (Ihandle* n, char* v);
static void set_caret      (Ihandle* n, char* v);
static void set_selectcolor(Ihandle* n, char* v);
static void set_selection  (Ihandle* n, char* v);
static void set_value      (Ihandle* n, char* v);
static void set_font       (Ihandle* n, char* v);
static void set_selectedtext(Ihandle* n, char* v);
static void set_visibleitems(Ihandle* n, char* v);
static void set_defaultenter(Ihandle* n, char* v);
static void set_parentdialog(Ihandle* n, char* v);
static void set_fullscreen  (Ihandle* n, char* v);
static void set_showdropdown(Ihandle *n, char *v);
static void set_separator   (Ihandle *n, char *v);
static void set_zorder      (Ihandle *n, char *v);
static void setclipboard   (Ihandle *n, char *v);

static int  canBeVisible( Ihandle* n );
static void set_option     (Ihandle* n, int op,  const char* v);
static void setTreeVisibility (Ihandle* n, char* v);

typedef void (*set_fn)(Ihandle* n, char* v);

static Itable *attrEnv = NULL;

static struct table
{
   char *name;
   set_fn set;
}attributes[] = { 
   {IUP_ACTIVE,        set_active},
   {IUP_ALIGNMENT,     set_alignment},
   {IUP_APPEND,        set_append},
   {IUP_BGCOLOR,       set_bgcolor},
   {IUP_BORDER,        set_border},
   {IUP_CARET,         set_caret},
   {IUP_CURSOR,        set_cursor},
   {IUP_DEFAULTENTER,  set_defaultenter},
   {IUP_FGCOLOR,       set_fgcolor},
   {IUP_FONT,          set_font},
   {IUP_ICON,          set_icon},
   {IUP_IMAGE,         set_image},
   {IUP_IMINACTIVE,    set_iminactive},
   {IUP_IMPRESS,       set_impress},
   {IUP_INSERT,        set_insert},
   {IUP_KEY,           set_key},
   {"MASK",            set_mask},
   {IUP_MENU,          set_menu},
   {"_IUPMOT_FONT",      set_font},
   {IUP_MULTIPLE,      set_multiple}, /* multiple tem que ficar antes de value */
   {IUP_NC,            set_nc},
   {IUP_PARENTDIALOG,  set_parentdialog},
   {IUP_POSX,          set_posx},
   {IUP_POSY,          set_posy},
   {IUP_RASTERSIZE,    set_rastersize},
   {"READONLY",        set_readonly},
   {"SELECTCOLOR",     set_selectcolor},
   {IUP_SELECTION,     set_selection},
   {IUP_SELECTEDTEXT,  set_selectedtext},
   {IUP_SIZE,          set_size},
   {IUP_TITLE,         set_title},
   {IUP_VALUE,         set_value},
   {IUP_VISIBLE,       set_visible},
   {IUP_FULLSCREEN,    set_fullscreen},
   {IUP_SEPARATOR,     set_separator},
   {"SHOWDROPDOWN",    set_showdropdown},
   {"ZORDER",          set_zorder},
   {IUP_VISIBLE_ITEMS, set_visibleitems},
   {"CLIPBOARD",       setclipboard}
};

#define IUP_NUM_FONT 36
static struct
{
  char Iup_name[50];
  char font[70];
}
IupFonts[IUP_NUM_FONT] = 
{
    { IUP_HELVETICA_NORMAL_8, "-*-helvetica-medium-r-*-*-8-*"},
    { IUP_HELVETICA_ITALIC_8, "-*-helvetica-medium-o-*-*-8-*"},
    { IUP_HELVETICA_BOLD_8,   "-*-helvetica-bold-r-*-*-8-*"},
    { IUP_HELVETICA_NORMAL_10,"-*-helvetica-medium-r-*-*-10-*" },
    { IUP_HELVETICA_ITALIC_10,"-*-helvetica-medium-o-*-*-10-*"},
    { IUP_HELVETICA_BOLD_10,  "-*-helvetica-bold-r-*-*-10-*"},
    { IUP_HELVETICA_NORMAL_12,"-*-helvetica-medium-r-*-*-12-*"},
    { IUP_HELVETICA_ITALIC_12,"-*-helvetica-medium-o-*-*-12-*"},
    { IUP_HELVETICA_BOLD_12,  "-*-helvetica-bold-r-*-*-12-*"},
    { IUP_HELVETICA_NORMAL_14,"-*-helvetica-medium-r-*-*-14-*"},
    { IUP_HELVETICA_ITALIC_14,"-*-helvetica-medium-o-*-*-14-*"},
    { IUP_HELVETICA_BOLD_14,  "-*-helvetica-bold-r-*-*-14-*"},
    { IUP_COURIER_NORMAL_8,   "-*-courier-medium-r-*-*-8-*"},
    { IUP_COURIER_ITALIC_8,   "-*-courier-medium-o-*-*-8-*"},
    { IUP_COURIER_BOLD_8,     "-*-courier-bold-r-*-*-8-*"},
    { IUP_COURIER_NORMAL_10,  "-*-courier-medium-r-*-*-10-*"},
    { IUP_COURIER_ITALIC_10,  "-*-courier-medium-o-*-*-10-*"},
    { IUP_COURIER_BOLD_10,    "-*-courier-bold-r-*-*-10-*"},
    { IUP_COURIER_NORMAL_12,  "-*-courier-medium-r-*-*-12-*"},
    { IUP_COURIER_ITALIC_12,  "-*-courier-medium-o-*-*-12-*"},
    { IUP_COURIER_BOLD_12,    "-*-courier-bold-r-*-*-12-*"},
    { IUP_COURIER_NORMAL_14,  "-*-courier-medium-r-*-*-14-*"},
    { IUP_COURIER_ITALIC_14,  "-*-courier-medium-o-*-*-14-*"},
    { IUP_COURIER_BOLD_14,    "-*-courier-bold-r-*-*-14-*"},
    { IUP_TIMES_NORMAL_8,     "-*-times-medium-r-*-*-8-*"},
    { IUP_TIMES_ITALIC_8,     "-*-times-medium-i-*-*-8-*"},
    { IUP_TIMES_BOLD_8,       "-*-times-bold-r-*-*-8-*"},
    { IUP_TIMES_NORMAL_10,    "-*-times-medium-r-*-*-10-*"},
    { IUP_TIMES_ITALIC_10,    "-*-times-medium-i-*-*-10-*"},
    { IUP_TIMES_BOLD_10,      "-*-times-bold-r-*-*-10-*"},
    { IUP_TIMES_NORMAL_12,    "-*-times-medium-r-*-*-12-*"},
    { IUP_TIMES_ITALIC_12,    "-*-times-medium-i-*-*-12-*"},
    { IUP_TIMES_BOLD_12,      "-*-times-bold-r-*-*-12-*"},
    { IUP_TIMES_NORMAL_14,    "-*-times-medium-r-*-*-14-*"},
    { IUP_TIMES_ITALIC_14,    "-*-times-medium-i-*-*-14-*"},
    { IUP_TIMES_BOLD_14,      "-*-times-bold-r-*-*-14-*"},
};

static int parse_iup_font(const char *v, int* font)
{
  const char *p = v;
  int s = 0, len = strlen(v);

  if (!p) return 0;

  switch (p[0])
  {
  case 'T' : font[0]=0; s = 6; break; 
  case 'H' : font[0]=1; s = 10; break; 
  case 'C' : font[0]=2; s = 8; break; 
  default: return 0;
  }

  if (s < len)
    p += s;
  else
    return 0;

  switch (p[0])
  {
  case 'N' : font[1]=0; s = 7; break;
  case 'I' : font[1]=1; s = 7; break;
  case 'B' : font[1]=2; s = 5; break;
  default: return 0;
  }

  if (s < len)
    p += s;
  else
    return 0;

  font[2] = atoi(p);
  if (font[2] <= 0)
    return 0;

  return 1;
}

char *IupMapFont(const char *iupfont)
{
  static char motif_font[100];
  static const char* family[3] = { "times", "helvetica", "courier" };
  static const char* type[3] = { "medium-r", "medium-o", "bold-r" };
  int font[3]; /* family-index, type-index, size */
  
  if (!parse_iup_font(iupfont, font)) return NULL;

  sprintf(motif_font, "-*-%s-%s-*-*-*-%d-*-*-*-*-*-*", family[font[0]], type[font[1]], font[2]*10);

  if (font[0]==0 && font[1]==1) /* times-medium-o*/
    strstr(motif_font, "-o-")[1] = 'i';

  return motif_font;
}

char *IupUnMapFont(const char *driverfont)
{
  int c=0;
  
  for(c = 0; c < IUP_NUM_FONT; c++)
  if(iupStrEqual(driverfont, IupFonts[c].font))
      return IupFonts[c].Iup_name;

  return NULL; /* font was not an IUP font */
}

#define NATTRIBUTES (sizeof(attributes)/sizeof(attributes[0])) 

void iupmotSetInitAttTable (void)
{
  int i;
  if (attrEnv) return;
  attrEnv = iupTableCreate(IUPTABLE_STRINGINDEXED);
  for (i=0; i<NATTRIBUTES; i++)
  {
    iupTableSet(attrEnv, attributes[i].name, attributes[i].set, IUP_POINTER);
  }
}

/* Esta funcao retorna o(s) Widget(s) associado com um elemento iup. */
int iupmotGetWidgetHandles(Ihandle *n, Widget* w)
{
   int i=0;
   if(type(n) == DIALOG_)
   {
         w[i++] = (Widget)handle(n);
         w[i++] = XtNameToWidget((Widget)handle(n),"dialog_form");
         w[i++] = XtNameToWidget((Widget)handle(n),"*dialog_area");
   }
   else if(type(n) == LIST_)
   {
         if ( XtClass((Widget)handle(n)) == xmComboBoxWidgetClass )
         {
            w[i++] = (Widget)handle(n);
#if (XmVERSION < 2)
            w[i++] = (Widget)XmComboBoxGetListWidget(handle(n));
            w[i++] = (Widget)XmComboBoxGetEditWidget(handle(n));
#else
            XtVaGetValues((Widget)handle(n), XmNlist, &w[i], NULL); i++;
            XtVaGetValues((Widget)handle(n), XmNtextField, &w[i], NULL); i++;
#endif
         }
         else
         {
            Widget sbh = XtNameToWidget( XtParent((Widget)handle(n)), "horizontal" );
            Widget sbv = XtNameToWidget( XtParent((Widget)handle(n)), "vertical" );
            w[i++] = XtParent((Widget)handle(n));
            w[i++] = (Widget)handle(n);
            if (sbh) w[i++] = sbh;
            if (sbv) w[i++] = sbv;
         }
   }
   else if(type(n) == MULTILINE_ || type(n) == CANVAS_)
   {
            Widget sbh = XtNameToWidget( XtParent((Widget)handle(n)), "horizontal" );
            Widget sbv = XtNameToWidget( XtParent((Widget)handle(n)), "vertical" );
            w[i++] = XtParent((Widget)handle(n));
            w[i++] = (Widget)handle(n);
            if (sbh) w[i++] = sbh;
            if (sbv) w[i++] = sbv;
   }
   else if(type(n) == BUTTON_ || type(n) == ITEM_ || type(n) == LABEL_ ||
           type(n) == TEXT_ || type(n) == TOGGLE_ || type(n) ==  SEPARATOR_ ||
           type(n) == MENU_ || type(n) == SUBMENU_)
   {
         w[i++] = (Widget)handle(n);
   }
   else if(type(n) == FRAME_)
   {
         Iwidgetdata *d;
         Widget *gad;

         XtVaGetValues ((Widget)handle(n), XmNuserData, &d, NULL);
         gad = (Widget*)d->data;
         w[i++] = gad[0];
         w[i++] = gad[1];
         w[i++] = gad[2];
         w[i++] = gad[3];
         w[i++] = gad[4];
   }
   w[i] = 0;
   return i;
}

void iupdrvSetAttribute (Ihandle* n, const char* a, const char* v)
{
   Ihandle *c;
   set_fn setfunc;

   if (n == NULL || handle(n) == NULL) return;

   setfunc = (set_fn)iupTableGet(attrEnv, a);

   if (setfunc == NULL)
   {  
      int op;
      if (sscanf (a, "%d", &op) != 1)
         return;
      set_option (n, op, v); /* Sets the value of a List element (1,2,etc) */
      return;
   }
   else
      (setfunc)(n, (char*)v); /* Calls apropriate function to set atribute */

   /* Non-inherited attributes. */
   if (  !iupIsInheritable(a) ||
         iupStrEqualNoCase (a, IUP_VISIBLE)) /*Visibility is dealt in set_visible*/
     return;
   
   foreachchild(c,n)
      if (iupTableGet (env(c), a) == NULL)
         iupdrvSetAttribute(c, a, v);
}


/* Update on the IUP_VISIBLE inside the environment
** is done on IupSetAttribute. 
*/
static void set_visible(Ihandle *n, char *v)
{
  assert(n);
  if(!n) return;
  
  /* User cannot make visible an element that is inside
   * a hidden ZBOX */
  if (!v || (iupStrEqual(v, IUP_YES) && !canBeVisible(n)))
    return;

  if(type(n) == DIALOG_)
  {
      set_elemvisible(n, v);
  }
  else if(type(n) == HBOX_ || type(n) == VBOX_ || 
          type(n) == RADIO_ || type(n) == FRAME_)
  {
      setTreeVisibility(n, v);
  }
  else if(type(n) == ZBOX_)
  {
        Ihandle *c = iupmotGetZboxValue( n );
        /* Setting ZBOX visible element */
        if (c != NULL)
        {
          iupSetEnv(c, IUP_VISIBLE, IUP_YES);
          iupdrvSetAttribute(c, IUP_VISIBLE, v);
        }
  }
  else
    set_elemvisible(n, v);
}

Pixel iupmotGetColor(Ihandle *n, char* color)
{
  unsigned int r, g, b;

  if (!color) return (Pixel)-1;

  if (!iupGetRGB(color, &r, &g, &b))
    return (Pixel)-1;

  if (n && type(n) == CANVAS_)
  {
    if (!iupmot_using8bitcanvas)
    {
      XColor cor;
      Colormap cmap;
      cor.red   = (unsigned short)((r/255.0)*USHRT_MAX);
      cor.green = (unsigned short)((g/255.0)*USHRT_MAX);
      cor.blue  = (unsigned short)((b/255.0)*USHRT_MAX);
      cor.flags = DoRed | DoGreen | DoBlue;
      XtVaGetValues( handle(n), XmNcolormap, &cmap, NULL );
      if ( XAllocColor(iupmot_display,cmap,&cor)==0 ) return (Pixel)-1;
      return cor.pixel;
    }
  }

  return iupmot_GetPixel((unsigned char)r, (unsigned char)g, (unsigned char)b);
}

static void set_fgcolor (Ihandle* n, char* v)
{
   Widget w[6];
   Pixel foreground;
   int i=0;

   if (iupmotGetWidgetHandles(n, w) == 0) return;

   if (XmIsGadget(w[0])) return;

   foreground = iupmotGetColor(n, v);
   if (foreground == -1) return;

   if (type(n) == DIALOG_) i++;
   while (w[i]) XtVaSetValues (w[i++], XmNforeground, foreground, NULL);
}

static void set_selectcolor(Ihandle *n, char *v)
{
   if (handle(n) && XtClass(handle(n))==xmToggleButtonWidgetClass)
   {
     Pixel color = iupmotGetColor(n, v);
     if (color == -1) return;
     XtVaSetValues( handle(n), XmNselectColor, color, NULL );
   }
}

static void set_bgcolor (Ihandle* n, char* v)
{
   Widget w[6];
   Colormap cmap;
   XtVarArgsList list;
   Pixel background, top, bottom, arm;

   if (iupmotGetWidgetHandles(n, w) == 0) return;

   if (XmIsGadget(w[0])) return;

   background = iupmotGetColor (n, v);
   if (background == -1) return;

   if (type(n) == CANVAS_)
     XtVaGetValues( handle(n), XmNcolormap, &cmap, NULL );
   else
     cmap = iupmot_color.colormap;

   XmGetColors(ScreenOfDisplay(iupmot_display,iupmot_screen),
               cmap,
               background, NULL, &top, &bottom, &arm );
   list = XtVaCreateArgsList(NULL, XmNbackground, background,
                                   XmNtopShadowColor, top,
                                   XmNbottomShadowColor, bottom,
                                   NULL );

   if (type(n) == BUTTON_)
      XtVaSetValues (w[0], XtVaNestedList, list, XmNarmColor, arm, NULL);
   else
   {
      if (type(n)==CANVAS_ || type(n)==MULTILINE_ ||
           (type(n)==LIST_ &&
             XtClass((Widget)handle(n)) != xmComboBoxWidgetClass))
         XtVaSetValues (w[1], XtVaNestedList, list, NULL);
      else
      {
         int i=0;
         if (type(n) == DIALOG_) i++;
         while (w[i]) 
            XtVaSetValues (w[i++], XtVaNestedList, list, NULL);
      }
   }
   XtFree(list);
}

static void set_alignment (Ihandle *n, char *v)
{
  if (type(n) == LABEL_)
  {
    unsigned char value;

    if (!v) value = XmALIGNMENT_BEGINNING;
    else if (iupStrEqualNoCase(v,IUP_ACENTER)) value = XmALIGNMENT_CENTER;
    else if (iupStrEqualNoCase(v,IUP_ARIGHT))  value = XmALIGNMENT_END;
    else if (iupStrEqualNoCase(v,IUP_ALEFT))   value = XmALIGNMENT_BEGINNING;
    else return;
    XtVaSetValues ((Widget)handle(n), XmNalignment, value, NULL);
  }
      
  if(type(n) == HBOX_ || type(n) == VBOX_ || type(n) == ZBOX_)
  {
    /* Reseting dialog's size so that alignment change will take effect. */
    char *value = NULL;
    Ihandle *dlg = IupGetDialog(n);
    value = IupGetAttribute(dlg, IUP_RASTERSIZE);
    if(value) value = (char*) iupStrDup(value);
    IupStoreAttribute(dlg, IUP_RASTERSIZE, value);
    if(value) free(value);
  }
}

static void set_readonly (Ihandle *n, char *v)
{
  if (type(n) == TEXT_ || type(n) == MULTILINE_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
  {
    int value;
    Widget w = (Widget)handle(n);
    if (type(n) == LIST_)
      w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

    if (!v) value = TRUE;
    else if (iupStrEqualNoCase(v,IUP_YES)) value = TRUE;
    else if (iupStrEqualNoCase(v,IUP_NO))  value = FALSE;
    else return;

    if (value)
      XtVaSetValues (w, XmNeditable, False, NULL);
    else
      XtVaSetValues (w, XmNeditable, True, NULL);
  }
}

static void set_active (Ihandle* n, char* v)
{
   int value;
   Widget w[6];

   if (iupmotGetWidgetHandles(n, w) == 0) return;

   if (!v) value = TRUE;
   else if (iupStrEqualNoCase(v,IUP_YES)) value = TRUE;
   else if (iupStrEqualNoCase(v,IUP_NO))  value = FALSE;
   else return;

   if (type(n) == FRAME_)
   {
     if (w[4]) XtSetSensitive (w[4], value);
   }
   else if (type(n) == MULTILINE_)
   {
     XtSetSensitive (w[1], value);
   }
   else
   {
     if (type(n) != MENU_ && w[0]) XtSetSensitive (w[0], value);
   }
}

static void set_title   (Ihandle* n, char* v)
{
   if (!v) v = "";

   if(type(n) == DIALOG_)
   {
         XtVaSetValues((Widget)handle(n), XmNtitle, v, XmNiconName, v, NULL );
   }
   else if(type(n) == BUTTON_ || type(n) == TOGGLE_ || type(n) == SUBMENU_)
   {
         XmString str = XmStringCreateLocalized( v );
         XtVaSetValues ((Widget)handle(n), XmNlabelString, str, NULL);
         XmStringFree( str );
   }
   else if(type(n) == LABEL_)
   {
#if (XmVERSION < 2)
         XmString str = XmStringCreateLtoR(v, XmSTRING_DEFAULT_CHARSET);
#else
         XmString str = XmStringGenerate((XtPointer)v, NULL, XmCHARSET_TEXT, NULL);
#endif
         XtVaSetValues ((Widget)handle(n), XmNlabelString, str, NULL);
         XmStringFree( str );
   }
   else if(type(n) == ITEM_)
   {
        char *p = strchr( v, '\t' );
        if (!p)
        {
          XmString str = XmStringCreateLocalized( v );
          XtVaSetValues ((Widget)handle(n), XmNlabelString, str, NULL);
          XmStringFree( str );
        }
        else
        {
          XmString lbl, acc;
          *p=0;
          lbl = XmStringCreateLocalized( v );
          acc = XmStringCreateLocalized( p+1 );
          XtVaSetValues ((Widget)handle(n), XmNlabelString, lbl, NULL);
          XtVaSetValues ((Widget)handle(n), XmNacceleratorText, acc, NULL);
          XmStringFree( lbl );
          XmStringFree( acc );
          *p = '\t';
        }
   }
   else if(type(n) == FRAME_)
   {
         XmString str;
         Iwidgetdata *d;
         Widget *gad;

         XtVaGetValues ((Widget)handle(n), XmNuserData, &d, NULL);
         gad = (Widget*)d->data;

         if (!v)
         {
            if (gad[4]) XtDestroyWidget(gad[4]);
            return;
         }

         str = XmStringCreateLocalized( v );
         if (gad[4])
         {
            XtVaSetValues (gad[4], 
              XmNlabelString, str,
                          NULL);
         }
         else
         {
            gad[4] = XtVaCreateManagedWidget ("tt",
                  xmLabelWidgetClass, XtParent(gad[0]),
                  XmNlabelString, str,
                  XmNx, (XtArgVal) posx(n) + 10,
                  XmNy, (XtArgVal) posy(n),
                  XmNrecomputeSize, False,
                  NULL);
         }
         XmStringFree( str );
   }
}

static void set_font(Ihandle* n, char* v)
{
   XmFontList font;
   Widget w[6], wf = 0;

   if (v)
   {
    char *motif_font = IupMapFont(v);
    if (motif_font != NULL) 
      v = motif_font;
   }

   iupStoreEnv(n, IUP_FONT, v);
   iupStoreEnv(n, "_IUPMOT_FONT", v);

   if (iupmotGetWidgetHandles(n, w) == 0) return;

   if (XmIsGadget(w[0])) return;

   if(type(n) == LIST_)
   {
     if ( XtClass((Widget)handle(n)) == xmComboBoxWidgetClass )
       wf = w[0];
     else
       wf = w[1];
   }
   else if(type(n) == MULTILINE_)
     wf = w[1];
   else if(type(n) == BUTTON_ || type(n) == SUBMENU_ || type(n) == ITEM_ ||
           type(n) == LABEL_  || type(n) == TEXT_ || type(n) ==  TOGGLE_)
     wf = w[0];
   else if(type(n) == FRAME_)
     wf = w[4];

   if (!wf) return;
   font = iupmoGetFontList(v);
   if (!font) return;
   XtVaSetValues(wf, XmNfontList, font, NULL);
}

void iupmotUpdateFont(Ihandle *h)
{
  char* font = iupGetEnv(h,"_IUPMOT_FONT");
  if (font)
    set_font(h, font);
  else
  {
    font = iupGetEnv(h,"FONT");
    if (font)
      set_font(h, font);
  }
}

static void set_image (Ihandle* n, char* v)
{
  if (!v) return;

  if(type(n) == TOGGLE_ || type(n) == LABEL_ || type(n) == BUTTON_)
  {
    if (!v)
      XtVaSetValues ((Widget)handle(n), XmNlabelType, XmSTRING, NULL );
    else
    {
      XtArgVal pix = XmUNSPECIFIED_PIXMAP;
      Ihandle *inactive = IupGetAttributeHandle(n, "IMINACTIVE");
      Ihandle *img = IupGetHandle(v);
      if (!img)
        return;

      if (handle(img)==NULL) 
        iupmotCreateImage(img, n, 0);
      pix = (XtArgVal)handle(img);

      XtVaSetValues ((Widget)handle(n), 
        XmNlabelType, XmPIXMAP, 
        XmNlabelPixmap, pix,
        NULL);

      if (!inactive)
      {
        if (iupGetEnv(img,"_IUPMOT_IMINACTIVE")==NULL) 
          iupmotCreateImage(img, n, 1);
        pix = (XtArgVal)iupGetEnv(img,"_IUPMOT_IMINACTIVE");

        XtVaSetValues ((Widget)handle(n), 
          XmNlabelType, XmPIXMAP,
          XmNlabelInsensitivePixmap, pix,
          NULL);
      }
    }
  }
}

static void set_impress (Ihandle* n, char* v)
{
   if (!v) return;

   if(type(n) == BUTTON_)
   {
        XtArgVal pix = XmUNSPECIFIED_PIXMAP;
        Ihandle *img = IupGetHandle(v);
        if (img)
        {
           if (handle(img)==NULL) iupmotCreateImage(img, n, 0);
           if (handle(img)!=NULL) pix = (XtArgVal)handle(img);
        }
        XtVaSetValues ((Widget)handle(n), 
                       XmNlabelType, XmPIXMAP,
                       XmNarmPixmap, pix,
                       XmNshadowThickness, pix==XmUNSPECIFIED_PIXMAP?2:0,
                       NULL);
   }
}

static void set_iminactive (Ihandle* n, char* v)
{
   if (!v) return;

   if(type(n) == TOGGLE_ || type(n) == BUTTON_ || type(n) == LABEL_)
   {
        XtArgVal pix = XmUNSPECIFIED_PIXMAP;
        Ihandle *img = IupGetHandle(v);
        if (img)
        {
           if (handle(img)==NULL) iupmotCreateImage(img, n, 0);
           if (handle(img)!=NULL) pix = (XtArgVal)handle(img);
        }
        XtVaSetValues ((Widget)handle(n), 
                       XmNlabelType, XmPIXMAP,
                       XmNlabelInsensitivePixmap, pix,
                       NULL);
   }
}


/********************************************************************
* Sets options (list element only)
* n -> list handle
* op -> Option number
* v -> value
********************************************************************/
static void set_option (Ihandle *n, int op, const char *v)
{
   if (handle(n)  &&  type(n) == LIST_  &&  op > 0)
   {
      int isCombo = XtClass((Widget)handle(n)) == xmComboBoxWidgetClass;
      int size;
      XtVaGetValues((Widget)handle(n), XmNitemCount, &size, NULL);

      if(isCombo) 
        XtRemoveCallback((Widget)handle(n), XmNselectionCallback, iupmotCBlist, NULL);

      if (v) /* If any value */
      {
         XmString str = NULL;
         if (op > size+1) return; /* Value out of list size */
         if (op <= size)          /* Value inside list      */
         {
            XmString newItems[1];
            str = XmStringCreateLocalized((String)v);
            newItems[0] = str;
            if ( isCombo ) /* Dropdown */
            {
#if (XmVERSION < 2)
               XmComboBoxReplaceItemsPos((Widget)handle(n), newItems, 1, op);
#else
               XmComboBoxDeletePos((Widget)handle(n), op);
               XmComboBoxAddItem((Widget)handle(n), str, op, 0);
#endif
            }
            else
            {
               XmListReplaceItemsPos((Widget)handle(n), newItems, 1, op);
            }
            XmStringFree(str);
         }
         else /* op = size+1, add elements inside environment */
         {
            static char nstr[10];
            char *val;
            int i = op;
            while (1) /* Add elements that were stored inside the environment */
            {
               sprintf(nstr, "%d", i);
               val = iupGetEnv(n,nstr);
               if (!val) break;
               str = XmStringCreateLocalized(val);
               if ( isCombo ) /* Dropdown */
#if (XmVERSION < 2)
                  XmComboBoxAddItem ((Widget)handle(n), str, i);
#else
                  XmComboBoxAddItem ((Widget)handle(n), str, i, 0);
#endif
               else
                  XmListAddItem((Widget)handle(n), str, i);
               XmStringFree(str);
               i++;
               size++;
            }
         }
      }
      else /* Sets NULL value (remove from list) */
      {
         int i;
         if (op > size) return;
         /* Sets end of list by deleting all previous items */
         for (i=op; i<=size; i++)
         {
            if ( isCombo )
               XmComboBoxDeletePos ((Widget)handle(n), op);
            else
               XmListDeletePos ((Widget)handle(n), 0); /* deletes last item */
         }

         if (isCombo)
         {
            /* trick to clear the text box of the dropdown after deleting the last item */
            static XmString nullstr = 0;
            if (!nullstr) nullstr = XmStringCreateSimple(" ");
#if (XmVERSION < 2)
            XmComboBoxAddItem ((Widget)handle(n), nullstr, 1);
            XmComboBoxSelectPos((Widget)handle(n), 1, False );
            XmComboBoxDeletePos ((Widget)handle(n), 1);           
#else
            XmComboBoxAddItem ((Widget)handle(n), nullstr, 1, 0);
            XmComboBoxSetItem ((Widget)handle(n), nullstr);
            XmComboBoxDeletePos ((Widget)handle(n), 1);           
#endif
         }
      }
   
     if(isCombo) 
       XtAddCallback((Widget)handle(n), XmNselectionCallback, iupmotCBlist, NULL);
     else
       XmListUpdateSelectedList((Widget)handle(n));

     if (iupCheck(n, IUP_MULTIPLE)==YES)
     {
       /* if the list is changed, update the current state */
       iupStoreEnv(n, "_IUPMOT_LISTOLDVALUE", IupGetAttribute(n, IUP_VALUE));
     }
   }
}

static void common_setsize (Ihandle* n, char* v)
{
   if (type(n) == DIALOG_)
   {
     Widget dialog = XtNameToWidget( (Widget)handle(n), "*dialog_area" );
     XtRemoveCallback (dialog, XmNresizeCallback, iupmotCBdialog, NULL);
     if (iupSetSize (n) != IUP_ERROR)
     {
       iupdrvResizeObjects (n);
     }
     XtAddCallback (dialog, XmNresizeCallback, iupmotCBdialog, NULL);
   }
}

static void set_rastersize (Ihandle* n, char* v)
{
  if (type(n) == DIALOG_) iupSetEnv(n, IUP_SIZE, NULL);
  common_setsize(n, v);
}

static void set_size (Ihandle* n, char* v)
{
  if (type(n) == DIALOG_) iupSetEnv(n, IUP_RASTERSIZE, NULL);
  common_setsize(n, v);
}

static void set_multiple (Ihandle* n, char* v)
{
   if (type(n) != LIST_) return;
   {
      unsigned char value = XmBROWSE_SELECT;
      if (iupStrEqualNoCase(v,IUP_YES)) value = XmEXTENDED_SELECT; 
      XtVaSetValues ((Widget)handle(n), XmNselectionPolicy, value, NULL);
   }
}

static void set_append (Ihandle* n, char* v)
{
   if (type(n) == MULTILINE_ || type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
    Widget w = (Widget)handle(n);
    if (type(n) == LIST_)
      w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

    if (type(n) == MULTILINE_)
    {
        XmTextPosition last = XmTextGetLastPosition(w);
        XmTextInsert(w, last, "\n");
        XmTextInsert(w, last+1, v);
    }
    else
    {
        XmTextPosition last = XmTextFieldGetLastPosition(w);
        XmTextFieldInsert(w, last, v);
    }

      /* if text exceeds max number of characters, deletes the exceeding text */      
      if(strlen(IupGetAttribute(n, IUP_VALUE)) + strlen(v) > atoi(IupGetAttribute(n, IUP_NC)))
      {
          XmTextReplace(w,
            atoi(IupGetAttribute(n, IUP_NC)),
            strlen(IupGetAttribute(n, IUP_VALUE)) + strlen(v),
            NULL);
      }
   }
}

static void set_insert (Ihandle* n, char* v)
{
   if (type(n) == MULTILINE_)
   {
      XmTextPosition caret = XmTextGetInsertionPosition((Widget)handle(n));
      XmTextInsert((Widget)handle(n), caret, v);

      /*if text exceeds max number of characters, deletes the exceeding text */
      if(strlen(IupGetAttribute(n, IUP_VALUE)) + strlen(v) > atoi(IupGetAttribute(n, IUP_NC))) 
      {
         XmTextReplace((Widget)handle(n),
           atoi(IupGetAttribute(n, IUP_NC)),
           strlen(IupGetAttribute(n, IUP_VALUE)) + strlen(v),
           NULL);
      }
   }
   else if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
     XmTextPosition caret;
      Widget w = (Widget)handle(n);
      if (type(n) == LIST_)
        w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

      caret = XmTextFieldGetInsertionPosition(w);
      XmTextFieldInsert(w, caret, v);

      /* if text exceeds max number of characters, deletes the exceeding text */      
      if(strlen(IupGetAttribute(n, IUP_VALUE)) + strlen(v) > atoi(IupGetAttribute(n, IUP_NC)))
      {
         XmTextReplace(w,
           atoi(IupGetAttribute(n, IUP_NC)),
           strlen(IupGetAttribute(n, IUP_VALUE)) + strlen(v),
           NULL);
      }
   }
}

static XmTextPosition pos( char *str, long int lin, long int col )
{
  int caret = 0, linText=1, colText=1;
  for (; str[caret] ; caret++, colText++)
  {
    if (str[caret] == '\n')
    {
      linText++;
      colText = 0;
      if (lin < linText)
        break;
    }
    if ((lin <= linText) && (col <= colText))
      break;
  }
  return caret;
}

static void set_selectedtext (Ihandle* n, char* v)
{
  assert( n && v );
  if (!n || !v)
    return;
  if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
  {
    XmTextPosition pos;
    Boolean rw;
    Widget w = (Widget)handle(n);
    if (type(n) == LIST_)
      w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

    XtVaGetValues(w, XmNeditable, &rw, NULL);

    if (!rw)
      XtVaSetValues(w, XmNeditable, True, NULL);

    XmTextFieldRemove(w);
    pos = XmTextFieldGetInsertionPosition(w);
    XmTextFieldInsert(w, pos, v);

    if (!rw)
      XtVaSetValues(w, XmNeditable, False, NULL);
  }
  else if (type(n) == MULTILINE_)
  {
    Widget w = (Widget) handle(n);
    XmTextPosition pos;
    Boolean rw;

    XtVaGetValues(w, XmNeditable, &rw, NULL);

    if (!rw)
      XtVaSetValues(w, XmNeditable, True, NULL);

    XmTextRemove(w);
    pos = XmTextGetInsertionPosition(w);
    XmTextInsert(w, pos, v);

    if (!rw)
      XtVaSetValues(w, XmNeditable, False, NULL);
  }
}

static void set_selection (Ihandle* n, char* v)
{
   if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
      int start=1, end = 1;

      Widget w = (Widget)handle(n);
      if (type(n) == LIST_)
        w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

      if (iupStrToIntInt(v,&start,&end,':')!=2) 
        return;

      if(start<1 || end<1) 
        return;

      if (start > end)
      {
        long int t = start;
        start = end;
        end = t;
      }

      XmTextFieldSetSelection(w,
        (XmTextPosition)(start-1), (XmTextPosition)(end-1+1), 0 );
   }
   else if (type(n) == MULTILINE_)
   {
      long int l1, c1, l2, c2;
      char *str = XmTextGetString((Widget)handle(n));
      sscanf(v, "%ld,%ld:%ld,%ld", &l1, &c1, &l2, &c2);
      XmTextSetSelection((Widget)handle(n), 
          pos( str, l1, c1 ), pos( str, l2, c2 )+1, 0 ); 
      XtFree( str );
   }
}

static void set_caret (Ihandle* n, char* v)
{
   if (!v) return;

   if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
      long int caret = 0;
      Widget w = (Widget)handle(n);
      if (type(n) == LIST_)
        w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
      sscanf(v,"%ld",&caret);

      XmTextSetInsertionPosition(w, (XmTextPosition)(caret-1));
   }
   else if (type(n) == MULTILINE_)
   {
        XmTextPosition caret = 0;
        int lin=0, col=0, linText = 1, colText = 1;
        char *str;

        iupStrToIntInt(v, &lin, &col, ',');
        str = XmTextGetString((Widget)handle(n));
        for (; str[caret] ; caret++, colText++)
        {
                if (str[caret] == '\n')
                        {
                        linText++;
                        colText = 0;
                        if (lin < linText)
                                break;
                        }
                if ((lin <= linText) && (col <= colText))
                        break;
        }
        XtFree( str );
        XmTextSetInsertionPosition((Widget)handle(n), caret);
   }
}

/*
  Scrollbars no motif, sao definidas por minimum,
  maximum, slidersize, increment e pageincrement, todos inteiros.
  No caso do IUP, slidersize = pageincrement.
  Foram fixados os valores de minimum e maximum em
  0 e INT_MAX, respectivamente.

          INT_MAX
  f(x) = --------- * (POS - MIN)
          MAX-MIN

  Remember that:  MIN<=POS<=MAX-D
*/

static void calc_sbpos(double min, double max, double d, double pos, int *slider, int *value, int *increment)
{
  double ratio = ((double)INT_MAX)/(max-min);

  *value  = (int)((pos-min)*ratio);

  *slider = (int)(d*ratio);
  if (*slider > INT_MAX) *slider = INT_MAX;
  if (*slider < 1) *slider = 1;

  *increment = (*slider)/10;
  if (*increment < 1) *increment = 1;

  if (*value < 0) *value = 0;
  if (*value > (INT_MAX - *slider)) *value = INT_MAX - *slider;
}

static void set_posx (Ihandle* n, char* v)
{
   char *par;

   if ( type(n) != CANVAS_ || !v ) return;

   par = IupGetAttribute(n, IUP_SCROLLBAR);
   if ((iupStrEqual(par,IUP_YES))           ||
       (iupStrEqual(par,IUP_HORIZONTAL))    ||
       (iupCheck(n,IUP_SBH)==YES)       ||
       (iupCheck(n,IUP_HORIZONTAL)==YES))
   {
      float min, max, pos, d;
      int slider, value, increment, minimum=0, maximum=INT_MAX;
      Widget sb = XtNameToWidget( XtParent((Widget)handle(n)), "horizontal" );

      if (!sb) return;

      pos = 0;
      if (v) pos = (float)atof(v);
      max = IupGetFloat(n, IUP_XMAX);
      min = IupGetFloat(n, IUP_XMIN);
      d   = IupGetFloat(n, IUP_DX);

      calc_sbpos(min, max, d, pos, &slider, &value, &increment);

      XtVaSetValues( sb,
                     XmNminimum,       minimum,
                     XmNmaximum,       maximum,
                     XmNvalue,         value,
                     XmNincrement,     increment,
                     XmNpageIncrement, slider,
                     XmNsliderSize,    slider,
                     NULL );
   }
}

static void set_posy (Ihandle* n, char* v)
{
   char *par;

   if ( type(n) != CANVAS_ || !v ) return;

   par = IupGetAttribute(n, IUP_SCROLLBAR);
   if ((iupStrEqual(par,IUP_YES))           ||
       (iupStrEqual(par,IUP_VERTICAL))  ||
       (iupCheck(n,IUP_SBV)==YES)       ||
       (iupCheck(n,IUP_VERTICAL)==YES))
   {
      float min, max, pos, d;
      int slider, value, increment, minimum=0, maximum=INT_MAX;
      Widget sb = XtNameToWidget( XtParent((Widget)handle(n)), "vertical" );

      if (!sb) return;

      pos = 0;
      if (v) pos = (float)atof(v);
      max = IupGetFloat(n, IUP_YMAX);
      min = IupGetFloat(n, IUP_YMIN);
      d   = IupGetFloat(n, IUP_DY);

      calc_sbpos(min, max, d, pos, &slider, &value, &increment);

      XtVaSetValues( sb,
                     XmNminimum,       minimum,
                     XmNmaximum,       maximum,
                     XmNvalue,         value,
                     XmNincrement,     increment,
                     XmNpageIncrement, slider,
                     XmNsliderSize,    slider,
                     NULL );
   }
}

static void set_icon (Ihandle* n, char* v)
{
   if ( type(n) == DIALOG_ )
   {
      XtArgVal pix = 0;
      Ihandle *img = IupGetHandle(v);

      if (img)
      {
         if (handle(img)==NULL) iupmotCreateImage(img, n, 0);
         if (handle(img)!=NULL) pix = (XtArgVal)(Pixmap)handle(img);
      }

      XtVaSetValues ((Widget)handle(n), 
                     XmNiconPixmap, pix,
                     NULL);
   }
}

static void set_parentdialog (Ihandle* n, char* v)
{
   if ( type(n) == DIALOG_ && handle(n) && XtWindow(handle(n)))
   {
      Ihandle *parent = IupGetHandle(v);

      if (parent && type(parent)==DIALOG_ &&
          handle(parent) && XtWindow(handle(parent)))
      {
         XSetTransientForHint( iupmot_display,
           XtWindow(handle(n)), 
           XtWindow(handle(parent)));
      }
      else
      {
        static Atom wm_transient_for = 0;
        if (wm_transient_for == 0)
           wm_transient_for = XmInternAtom(iupmot_display,
                              "WM_TRANSIENT_FOR", False);
        XDeleteProperty( iupmot_display, XtWindow(handle(n)), wm_transient_for );
      }
   }
}

static void set_mask(Ihandle* n, char* v)
{
   if (type(n) == TEXT_)
   {
      Iwidgetdata *d = NULL;
      ITextInfo *info = NULL;
      ImaskElem *mask;

      XtVaGetValues ((Widget)handle(n), XmNuserData, &d, NULL);

      if (!d) return;

      if (d->data) info = (ITextInfo*)(d->data); 

      mask = iupCreateMask(v);

      if (mask)
      {
         char *val;
         char  fill[30];
         if (!info) info = (ITextInfo*)malloc(sizeof(ITextInfo));
         if (!info)
         {
            d->data = 0;
            return;
         }
         val = XmTextFieldGetString((Widget)handle(n));
         info->mask = mask;
         info->status = iupCheckMask(mask, val, fill);
         if (info->status == 0) 
         {
            info->status = iupCheckMask(mask, "", fill);
            XtVaSetValues((Widget)handle(n), XmNvalue, fill, NULL);
         }
         else if (fill[0])
         {
            XmTextFieldInsert((Widget)handle(n),
                              XmTextFieldGetLastPosition((Widget)handle(n)),
                              fill);
         }
         XtFree(val);
         d->data = info;
      }
      else
      {
         if (info) free(info);
         d->data = 0;
      }
   }
}

static void set_nc(Ihandle* n, char* v)
{
   if (!v) return;

   if (type (n) == TEXT_ || type(n) == MULTILINE_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
   {
      int max;
      Widget w = (Widget)handle(n);
      if (type(n) == LIST_)
        w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");
      sscanf( v, "%d", &max );
      XtVaSetValues(w,XmNmaxLength, max, NULL);
   }
}

static void set_border(Ihandle* n, char* v)
{
   if (type (n) == TEXT_ || type(n) == MULTILINE_)
   {
      int b = 2;
      if (v && iupStrEqual(v,IUP_NO)) b=0;
      XtVaSetValues((Widget)handle(n),XmNshadowThickness, b, NULL);
   }
}

static void set_key   (Ihandle* n, char* v)
{
   if (type(n)==ITEM_ || 
       type(n)==SUBMENU_ ||
       type(n)==BUTTON_ ||
       type(n)==TOGGLE_ )
   {
      int key = iupmotKeyGetMenuKey(v);
      if (key) XtVaSetValues((Widget)handle(n), XmNmnemonic, key, NULL);
   }
}

/********************************************************************
 * Sets visibility of given element including its child.
 *
 * Visible elements become invisible when parent is made 
 * invisible (but they store their visibility status).
 *
 ********************************************************************/
static void setTreeVisibility (Ihandle* n, char* v)
{
  Ihandle *c;
  char *attr;
 
  if (!handle(n)) return;

  /* This attr is taken when this function is called recursively 
   * (the first time this function is called this attribute 
   * will be the same as v because caller has set the
   * environment already) */
  attr = iupGetEnv(n, IUP_VISIBLE);

  /* If environment says it should be invisible, then let it be 
   * (so parents made visible do not interfere with children's
   * visibility).
   */
  if (iupStrEqualNoCase(v, IUP_YES) && attr && iupStrEqualNoCase(attr, IUP_NO))
  {
    set_elemvisible(n,IUP_NO);
    /* Must return to protect elements inside VBOX and HBOX 
     * so that even if elements are visible, they should not 
     * be because it's parent isn't. */
    return;
  }
  else
    set_elemvisible(n,v);
  
  foreachchild(c,n)
    setTreeVisibility(c,v);
}

static int isChild( Ihandle* parenth, Ihandle* h )
{
  if (!h) return 0;
  if (h == parenth) return 1;
  return isChild( parenth, parent(h) );
}

static int canBeVisibleRec( Ihandle* n, Ihandle* elem )
{
  if (!parent(n)) return 1;
  if (type(parent(n)) == ZBOX_)
  {
    Ihandle* v = iupmotGetZboxValue( parent(n) );
    if (!isChild(v, elem)) return 0; 
  }
  return canBeVisibleRec( parent(n), elem );
}

/*
 * Returns wether or not the element is inside a ZBOX (can
 * be visible.) If it is not inside a ZBOX then it can be drawn. 
 *
 * If it is inside a ZBOX then it must be inside the visible 
 * ZBOX or else it cannot be drawn.
 *
 * This may be adapted to work also with a HBOX or ZBOX (this
 * was not done because it would decrease performace).
 */
static int canBeVisible( Ihandle* n )
{
  return canBeVisibleRec( n, n );
}

/* Makes visible each element individually. Some elements don't need
 * to appear (proper treatment to those elements are given on previous 
 * functions).
 */
static void set_elemvisible (Ihandle* n, char* v)
{
  int value;

  if (!v) value = TRUE;
  else if (iupStrEqualNoCase(v,IUP_YES)) value = TRUE;
  else if (iupStrEqualNoCase(v,IUP_NO))  value = FALSE;
  else return;

  if(type(n) == DIALOG_)
  {
    if (value)
      iupmotShowDialog(n);
    else
      iupmotHideDialog(n);
  }
  else
  {
    Widget w[6];
    int i=0;

    if (iupmotGetWidgetHandles(n, w) == 0) return;

    while(w[i])
      XtSetMappedWhenManaged(w[i++], value);
  }
}

static void set_visibleitems (Ihandle* n, char* v)
{
  int size;
  if (type(n) != LIST_) return;

  if ( XtClass((Widget)handle(n)) == xmComboBoxWidgetClass )
  {
    sscanf(v, "%d", &size);
    if (size <= 0) return;
    XtVaSetValues((Widget)handle(n), XmNvisibleItemCount, size, NULL);
  }
}

static void set_menu (Ihandle* n, char* v)
{
  if (type(n) != DIALOG_) return;

  {
    Widget dialog = XtNameToWidget( (Widget)handle(n), "*dialog_area" );
    Ihandle *c = IupGetHandle (v);
    if (c && type(c)==MENU_)
    {
      iupmotSetParent(XtNameToWidget((Widget)handle(n),"*dialog_form"), NULL);
      iupdrvCreateObjects(c);
      iupmotSetParent(NULL, NULL);
      if (XtParent((Widget)handle(c)) == XtParent(dialog))
      {
        Ihandle *menu_old = IupGetHandle (IupGetAttribute(n,"_IUPMOT_PREVIOUS_MENU"));
        if ((menu_old != NULL) &&
            (c != menu_old) &&
            (XtParent((Widget)handle(menu_old)) == XtParent(dialog)) &&
            (type(menu_old) == MENU_) )
        {
          XtUnmapWidget(handle(menu_old));
        }

        if (XtIsRealized(handle(c)))
          XtMapWidget(handle(c));

        XtVaSetValues( dialog, XmNtopAttachment, XmATTACH_WIDGET,
          XmNtopWidget,     (Widget)handle(c),
          NULL );
        IupStoreAttribute( n, "_IUPMOT_PREVIOUS_MENU", v );
      }
      else
        fprintf(stderr, "IUP warning: menu %s already in use\n", v);
    }
    else
    {
      XtVaSetValues( dialog, XmNtopAttachment, XmATTACH_FORM, NULL );
      IupSetAttribute( n, "_IUPMOT_PREVIOUS_MENU", NULL );
    }
  }
}

static void set_cursor   (Ihandle* n, char* v)
{
   static struct {
      char * iupname;
      int    xname;
      Cursor cur;
   } table[] = {
      { "NONE",      0,                      0 }, 
      { "ARROW",     XC_left_ptr,            0 },
      { "BUSY",      XC_watch,               0 },
      { "CROSS",     XC_cross,               0 },
      { "HAND",      XC_hand2,               0 },
      { "IUP",       XC_pirate,              0 },
      { "MOVE",      XC_fleur,               0 },
      { "PEN",       XC_pencil,              0 },
      { "RESIZE_N",  XC_top_side,            0 },
      { "RESIZE_S",  XC_bottom_side,         0 },
      { "RESIZE_W",  XC_left_side,           0 },
      { "RESIZE_E",  XC_right_side,          0 },
      { "RESIZE_NE", XC_top_right_corner,    0 },
      { "RESIZE_SE", XC_bottom_right_corner, 0 },
      { "RESIZE_NW", XC_top_left_corner,     0 },
      { "RESIZE_SW", XC_bottom_left_corner,  0 },
      { "RESIZE_C",  XC_sb_h_double_arrow,   0 },
      { "TEXT",      XC_xterm,               0 } };

   int i;

   if (!n || !handle(n) || type(n)==SUBMENU_ ||
       (type(n)==MENU_ && parent(n)) ||
       XmIsGadget((Widget)handle(n)) ||
       !XtWindow((Widget)handle(n)) ) 
       return;

   if (!v) 
     v = table[1].iupname; /* default is arrow */

   for (i=0; i<sizeof(table)/sizeof(table[0]); i++)
   {
      if (iupStrEqualNoCase(v, table[i].iupname)) 
      {
         if (table[i].cur == 0)
         {
            if (i)
               table[i].cur = XCreateFontCursor(iupmot_display, table[i].xname );
            else
            {
               static XColor cursor_color={0L,0,0,0,0,0};
               static char bitsnull[1]={0x00};
               static Pixmap pixmapnull;

               pixmapnull = XCreateBitmapFromData (iupmot_display,
                                                   XtWindow((Widget)handle(n)),
                                                   bitsnull,
                                                   1,1);

               table[i].cur = XCreatePixmapCursor (iupmot_display,
                                             pixmapnull,
                                             pixmapnull,
                                             &cursor_color,
                                             &cursor_color,
                                             0,0);
            }
         }

         XDefineCursor(iupmot_display, XtWindow((Widget)handle(n)), table[i].cur);
         return;  
      }
   }

   {
     /* if not pre-defined, try an image */
      Ihandle *curimg = IupGetHandle(v);
      if (curimg)
      {
         if (handle(curimg)==NULL) 
           iupmotCreateCursor(curimg);
         if (handle(curimg)!=NULL)
            XDefineCursor(iupmot_display, XtWindow((Widget)handle(n)), (Cursor)handle(curimg));
      }
      else
      {
        /* if not image, try other pre-defined Motif cursors */
        if (sscanf(v, "%d", &i) == 1)
        {
          Cursor cur = XCreateFontCursor(iupmot_display, i );
          if (cur)
            XDefineCursor(iupmot_display, XtWindow((Widget)handle(n)), cur);
        }
      }
   }
}

static void setradiovalue (Ihandle *n, char *v)
{
   Ihandle *c, *t;

   c = t = IupGetHandle(v);
   if ( !c || type(c)!=TOGGLE_) return;

   while (c!=NULL && c!=n) c=parent(c);

   if (!c) return;

   IupSetAttribute( t, IUP_VALUE, IUP_ON );
}

static void set_value (Ihandle* n, char* v)
{
   if(type(n) == LIST_)
   {
         int op, size;
         int isCombo = (XtClass((Widget)handle(n))==xmComboBoxWidgetClass);
         int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;

         if (!v) return;

         XtVaGetValues((Widget)handle(n), XmNitemCount, &size, NULL);

         if (haseditbox)
         {
           Widget w = (Widget)IupGetAttribute(n, "_IUPMOT_EDITBOX");

           if (!v)
             XmTextFieldSetString( w, "" );
           else
             XmTextFieldSetString( w, v );
         }
         else if (!isCombo && iupCheck(n,IUP_MULTIPLE) == YES)
         {
           int i, len = strlen(v);
           XmListDeselectAllItems( (Widget)handle(n) );
           XtVaSetValues((Widget)handle(n), XmNselectionPolicy, XmMULTIPLE_SELECT, NULL);
           for (i=0; i<len; i++)
           {
             if (v[i] == '+')
               XmListSelectPos((Widget)handle(n), i+1, False );
           }
           XtVaSetValues((Widget)handle(n), XmNselectionPolicy, XmEXTENDED_SELECT, 
                                            XmNselectionMode, XmNORMAL_MODE, NULL);  /* must also restore this */
           iupStoreEnv(n, "_IUPMOT_LISTOLDVALUE", v);
         }
         else if (sscanf (v, "%d", &op) == 1)
         {
            if (op <= size)
           {
               char *lastpos = iupGetEnv(n, "_IUPMOT_PREV_LIST_POS");
               if (lastpos != NULL) 
               {
                 char *v = (char*) malloc(sizeof(char)*(strlen(lastpos)+1));
                 sprintf(v, "%d", op);
                 iupStoreEnv(n, "_IUPMOT_PREV_LIST_POS", v);
                 free(v);
               }
               if (isCombo)
               {
#if (XmVERSION < 2)
                  XmComboBoxSelectPos((Widget)handle(n), op, False );
#else
                  XtRemoveCallback((Widget)handle(n), XmNselectionCallback, iupmotCBlist, NULL);
                  XtVaSetValues((Widget)handle(n), XmNselectedPosition, op, NULL);
                  XtAddCallback((Widget)handle(n), XmNselectionCallback, iupmotCBlist, NULL);
#endif
               }
               else
               {
                  XmListSelectPos((Widget)handle(n), op, FALSE);
               }
            }
         }
   }
   else if(type(n) == MULTILINE_)
   {
         XtRemoveCallback ((Widget)handle(n), XmNmodifyVerifyCallback,
                           iupmotCBtext, NULL);
         XmTextSetString ((Widget)handle(n), v ? v : "" );
         XtAddCallback ((Widget)handle(n), XmNmodifyVerifyCallback,
                        iupmotCBtext, NULL);
   }
   else if(type(n) == TEXT_)
   {
         if (!v)
         {   
#if (XmVERSION == 1) && (XmREVISION < 2)
           XtVaSetValues( (Widget)handle(n), XmNvalue, "", NULL );
#else
           XmTextFieldSetString( (Widget)handle(n), "" );
#endif

         }
         else
         {
           /* verificar mascara */
           Iwidgetdata *wd = 0;
           ITextInfo *ti = 0;
           XtVaGetValues((Widget)handle(n), XmNuserData, &wd, NULL);
           if (wd) ti = (ITextInfo*)(wd->data);
           if (!ti || iupCheckMask(ti->mask, v, 0)) 
           {
#if (XmVERSION == 1) && (XmREVISION < 2)
             if (v[0] == 0)
               XtVaSetValues( (Widget)handle(n), XmNvalue, "", NULL );
             else
#endif
               XmTextFieldSetString( (Widget)handle(n), v );
           }
         }
   } 
   else if(type(n) == RADIO_)
   {
         if (v == NULL) return;
         setradiovalue(n,v);
   }
   else if(type(n) == TOGGLE_)
   {
         Ihandle *p;
         if (!v) v = IUP_OFF;
         for (p=parent(n); (type(p) != DIALOG_) && (type(p)!=RADIO_); p=parent(p))
          ;
         if (type(p) == RADIO_)
         {
            if (iupStrEqualNoCase(v,IUP_ON))
            {
               Ihandle *c = iupmotFindRadioOption(p);
               if (c)
               {
                  XtRemoveCallback ((Widget)handle(c),
                                    XmNdisarmCallback,
                                    iupmotCBradio, NULL); 
                  XtVaSetValues ((Widget)handle(c), XmNset, 0, NULL);
                  XtAddCallback ((Widget)handle(c), XmNdisarmCallback,
                                 iupmotCBradio, NULL); 
               }
               XtRemoveCallback ((Widget)handle(n), XmNdisarmCallback,
                                 iupmotCBradio, NULL); 
               XtVaSetValues ((Widget)handle(n), XmNset, 1, NULL);
               XtAddCallback ((Widget)handle(n), XmNdisarmCallback,
                              iupmotCBradio, NULL); 
            }
         }
         else
         {
            int check = iupStrEqualNoCase(v,IUP_ON)? XmSET: iupStrEqualNoCase(v,"NOTDEF")? XmINDETERMINATE: XmUNSET;
            XtVaSetValues ((Widget)handle(n), XmNset, check, NULL);
         }
   }
   else if(type(n) == ITEM_)
   {
           if (!v) v = IUP_OFF;
           if (XtClass((Widget)handle(n))==xmToggleButtonWidgetClass)
              /* is in main menu */
              XmToggleButtonSetState((Widget)handle(n),iupStrEqualNoCase(v,IUP_ON),0);
   }
   else if(type(n) == ZBOX_)
   {
          Ihandle *z, *nc;
          char *ov = IupGetAttribute(n, "_IUPMOT_PREVIOUS_ZBOX_VALUE");
          /* Acha o value anterior */
          Ihandle *oc = IupGetHandle(ov);
          if (oc == NULL)
             oc = child(n);
          else
          {
             foreachchild(z,n) if (oc==z) break;
             oc = z;
          }
          
          /* Found previouse value (oc). Now set it no invisible */
          if (oc != NULL) 
          {
            iupSetEnv(oc, IUP_VISIBLE, IUP_NO);
            set_visible(oc, IUP_NO);
          }

          nc = v ? IupGetHandle(v) : child(n);
          foreachchild( z, n )
             if (z==nc)
             {
                /* Test if ZBOX itself is visible... */
                char *vis = iupGetEnv(n,IUP_VISIBLE);
                if (!vis || !iupStrEqual(vis, IUP_NO))
                {
                   if (handle(nc)==NULL) IupMap(nc);
                   IupSetAttribute(nc, IUP_VISIBLE, IUP_YES);
                }
                else /* If we don't set it, we will loose info on ZBOX's VALUE */
                {
                   iupSetEnv(nc, IUP_VISIBLE, IUP_YES);
                }
            }
         IupSetAttribute(n, "_IUPMOT_PREVIOUS_ZBOX_VALUE", v);
   }
}

static void set_defaultenter (Ihandle* n, char* v)
{
  if (type(n) == DIALOG_)
  {
    char *ov = IupGetAttribute(n, "_IUPMOT_PREVIOUS_DEFAULT_ENTER");
    Ihandle *old = IupGetHandle( ov );
    Ihandle *new = IupGetHandle( v );

    if (old == new) return;
    if (old && type(old) == BUTTON_ && IupGetDialog(old) == n && handle(old))
    {
      /* apaga feedback do antigo default */
    }
    if (new && type(new) == BUTTON_ && IupGetDialog(new) == n && handle(new))
    {
      /* desenha feedback no novo default */
    }
    IupSetAttribute( n, "_IUPMOT_PREVIOUS_DEFAULT_ENTER", v );
  }
}

void iupmotSetAttrs (Ihandle *n)
{
   int i;
   int inc = iupmot_incallback;
   iupmot_incallback = 1;

   /* procura opcoes da lista */

   if (type(n) == LIST_ && env(n))
   {
      char *val;
      val = iupGetEnv(n,"1");
      if (val) set_option (n, 1, val);
   }

   for (i = 0; i < NATTRIBUTES; i++)
   {
      char *a = attributes[i].name;
      char *val;

      if ( (iupStrEqualNoCase (a, IUP_SIZE)) || /* estes atributos nao sao herdados */
           (iupStrEqualNoCase (a, IUP_VALUE)) ||
           (iupStrEqualNoCase (a, IUP_TITLE)) )
      {
         if (!env(n)) continue;
         val = (char*)iupTableGet(env(n),a);
      }
      else
      {
         val = iupGetEnv(n, a);
      }
      if (val) (*attributes[i].set)(n, val);
   }
  iupmot_incallback = inc;
}

/* implemented in "utilsx11.c" */
void wxSetFullScreenStateX11(Display* disp, Window root, Window wnd, int full);

static void set_fullscreen(Ihandle *n, char *v)
{
  if(n && type(n) == DIALOG_ && handle(n) && v)
  {
    if(iupStrEqual(v, IUP_YES))
    {
#ifdef Linux  /* this is only working in OpenMotif */
      Window root = RootWindow(iupmot_display, iupmot_screen);
      Window wnd = XtWindow((Widget)handle(n));
      wxSetFullScreenStateX11(iupmot_display, root, wnd, 1);
#else
      Display *display = XtDisplay(handle(n));
      int screen = DefaultScreen(display);
      int w = DisplayWidth(display, screen);
      int h = DisplayHeight(display, screen);
      Widget dlg_area;
      int decor;

      XtVaGetValues(handle(n), XmNmwmDecorations, &decor,NULL);
      decor &= ~MWM_DECOR_BORDER;
      XtVaSetValues(handle(n), XmNmwmDecorations, decor, NULL);
 
      XtVaSetValues(handle(n), XmNmwmFunctions, decor, NULL);

      dlg_area = XtNameToWidget((Widget)handle(n), "*dialog_area");
      XtVaSetValues (XtParent(dlg_area),
                              XmNresizePolicy, XmRESIZE_ANY,
                              NULL);

      XtVaSetValues(dlg_area, XmNresizePolicy, XmRESIZE_ANY,
                              XmNwidth,  (XtArgVal) w,
                              XmNheight, (XtArgVal) h, 
                              XmNx,      (XtArgVal) 0,
                              XmNy,      (XtArgVal) 0,
                              NULL);
                              
      XtVaSetValues (XtParent(dlg_area),
                              XmNresizePolicy, XmRESIZE_NONE, NULL);

      XtVaSetValues (dlg_area, XmNresizePolicy, XmRESIZE_NONE, NULL);
#endif
    }
    else
    {
#ifdef Linux
      Window root = RootWindow(iupmot_display, iupmot_screen);
      Window wnd = XtWindow((Widget)handle(n));
      wxSetFullScreenStateX11(iupmot_display, root, wnd, 0);
#else
      int decor;
      IupSetAttribute(n, IUP_SIZE, NULL);
      XtVaGetValues(handle(n), XmNmwmDecorations, &decor,NULL);
      decor |= MWM_DECOR_BORDER;
      XtVaSetValues(handle(n), XmNmwmDecorations, decor, NULL);
      XtVaSetValues (handle(n), XmNmwmFunctions,
        MWM_DECOR_BORDER  | MWM_FUNC_ALL      | MWM_FUNC_CLOSE |
        MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE | MWM_DECOR_TITLE, NULL);
#endif
    }
  }
}

static void set_separator(Ihandle *n, char *v)
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

static void set_showdropdown(Ihandle *n, char *v)
{
  if(n && handle(n) && type(n) == LIST_ && v)
  {
    if (iupCheck(n, IUP_DROPDOWN)==YES)
    {
      int val = iupStrEqualNoCase(v, IUP_YES)? 1: 0;
      if (val)
      {
        XButtonEvent ev;
        memset(&ev, 0, sizeof(XButtonEvent));
        ev.type = ButtonPress;
        ev.display = XtDisplay((Widget)handle(n));
        ev.send_event = True;
        ev.root = RootWindow(iupmot_display, iupmot_screen);
        ev.time = clock()*CLOCKS_PER_SEC;
        ev.window = XtWindow((Widget)handle(n));
        ev.state = Button1Mask;
        ev.button = Button1;
        ev.same_screen = True;
        XtCallActionProc((Widget)handle(n), "CBDropDownList", (XEvent*)&ev, 0, 0 ); 
      }
      else
        XtCallActionProc((Widget)handle(n), "CBDisarm", 0, 0, 0 );
    }
  }
}

static void set_zorder(Ihandle *n, char *v)
{
  if (iupStrEqualNoCase(v, "TOP"))
    XRaiseWindow(iupmot_display, XtWindow((Widget)handle(n)));
  else
    XLowerWindow(iupmot_display, XtWindow((Widget)handle(n)));
}

static void setclipboard(Ihandle *n, char *v)
{
  if (n && handle(n) && (type(n) == TEXT_ || type(n) == MULTILINE_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES)) && v)
  {
    if (iupStrEqualNoCase(v, "COPY"))
      XmTextCopy((Widget)handle(n), CurrentTime);
    else if (iupStrEqualNoCase(v, "CUT"))
      XmTextCut((Widget)handle(n), CurrentTime); 
    else if (iupStrEqualNoCase(v, "PASTE"))
      XmTextPaste((Widget)handle(n));
  }
}
