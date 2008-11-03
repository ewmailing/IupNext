/** \file
 * \brief IupColorDlg pre-defined dialog control
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"
#include "iupcb.h"
#include "iupmask.h"
#include "iupcontrols.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>
#include <cdirgb.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_strmessage.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"
#include "iup_register.h"
#include "iup_image.h"
#include "iup_colorhsi.h"


const char* default_colortable_cells[20] = {
       {"0 0 0"}, {"255 255 255"}, {"127 127 127"},     {"255 0 0"},
  {"160 32 240"},     {"0 0 255"}, {"173 216 230"},     {"0 255 0"},
   {"255 255 0"},   {"255 165 0"}, {"230 230 250"},   {"165 42 42"},
  {"139 105 20"},  {"30 144 255"}, {"255 192 203"}, {"144 238 144"},
    {"26 26 26"},    {"77 77 77"}, {"191 191 191"}, {"229 229 229"}
};

typedef struct _IcolorDlgData
{
  int status;

  long color;

  float hue, saturation, intensity;
  unsigned char red, green, blue, alpha;

  Ihandle *red_txt, *green_txt, *blue_txt, *alpha_txt;
  Ihandle *hue_txt, *intensity_txt, *saturation_txt;
  Ihandle *color_browser, *color_cnv, *colorhex_txt;
  Ihandle *colortable_cbar, *alpha_val;
  Ihandle *ok_bt, *cancel_bt, *help_bt;

  cdCanvas* color_cdcanvas, *color_cddbuffer;
} IcolorDlgData;


static void iColorBrowserDlgColorCnvRepaint(IcolorDlgData* colordlg_data)
{
  if (!colordlg_data->color_cddbuffer)
    return;

  ///* DST = White background */
  //r = iupALPHABLEND(colordlg_data->red,   255, colordlg_data->alpha);
  //g = iupALPHABLEND(colordlg_data->green, 255, colordlg_data->alpha);
  //b = iupALPHABLEND(colordlg_data->blue,  255, colordlg_data->alpha);

  //colordlg_data->color = cdEncodeColor(r,g,b);

  cdCanvasBackground(colordlg_data->color_cddbuffer, colordlg_data->color);
  cdCanvasClear(colordlg_data->color_cddbuffer);

  cdCanvasFlush(colordlg_data->color_cddbuffer);
}

/*********************************\
* Converts HSI to RGB color model *
\*********************************/
static void iColorBrowserDlgHSI2RGB(IcolorDlgData* colordlg_data)
{
  iupColorHSI2RGB(colordlg_data->hue, colordlg_data->saturation, colordlg_data->intensity,
                  &colordlg_data->red, &colordlg_data->green, &colordlg_data->blue);
}

/*********************************\
* Converts RGB to HSI color model *
\*********************************/
static void iColorBrowserDlgRGB2HSI(IcolorDlgData* colordlg_data)
{
  iupColorRGB2HSI(colordlg_data->red, colordlg_data->green, colordlg_data->blue,
                  &(colordlg_data->hue), &(colordlg_data->saturation), &(colordlg_data->intensity));
}

/*********************************\
* Converts RGB to HEX color name *
\*********************************/
static void iColorBrowserDlgHex_TXT_Update(IcolorDlgData* colordlg_data)
{
  IupSetfAttribute(colordlg_data->colorhex_txt, "VALUE", "%.2X%.2X%.2X", colordlg_data->red, colordlg_data->green, colordlg_data->blue);
}

/*************************************************\
* Updates text fields with the current HSI values *
\*************************************************/
static void iColorBrowserDlgHSI_TXT_Update(IcolorDlgData* colordlg_data)
{
  if (IupGetInt(colordlg_data->hue_txt, "VALUE") != (int) colordlg_data->hue)
    IupSetfAttribute(colordlg_data->hue_txt, "VALUE", "%d", (int) colordlg_data->hue);
  if (IupGetFloat(colordlg_data->saturation_txt, "VALUE") != colordlg_data->saturation)
    IupSetfAttribute(colordlg_data->saturation_txt, "VALUE", "%d", (int) (colordlg_data->saturation * 100));
  if (IupGetFloat(colordlg_data->intensity_txt, "VALUE") != colordlg_data->intensity)
    IupSetfAttribute(colordlg_data->intensity_txt, "VALUE", "%d", (int) (colordlg_data->intensity * 100));
}

/*************************************************\
* Updates text fields with the current RGB values *
\*************************************************/
static void iColorBrowserDlgRGB_TXT_Update(IcolorDlgData* colordlg_data)
{
  if (IupGetInt(colordlg_data->red_txt, "VALUE") != (int) colordlg_data->red)
    IupSetfAttribute(colordlg_data->red_txt, "VALUE", "%d", (int) colordlg_data->red);
  if (IupGetInt(colordlg_data->green_txt, "VALUE") != (int) colordlg_data->green)
    IupSetfAttribute(colordlg_data->green_txt, "VALUE", "%d", (int) colordlg_data->green);
  if (IupGetInt(colordlg_data->blue_txt, "VALUE") != (int) colordlg_data->blue)
    IupSetfAttribute(colordlg_data->blue_txt, "VALUE", "%d", (int) colordlg_data->blue);
}

/*****************************************\
* Sets the RGB color in the Color browser *
\*****************************************/
static void iColorBrowserDlgView_Update(IcolorDlgData* colordlg_data)
{
  IupSetfAttribute(colordlg_data->color_browser, "RGB", "%3d %3d %3d", colordlg_data->red, colordlg_data->green, colordlg_data->blue);
  colordlg_data->color = cdEncodeColor(colordlg_data->red, colordlg_data->green, colordlg_data->blue);
  iColorBrowserDlgColorCnvRepaint(colordlg_data);
}

/***********************************************\
* Initializes the default values to the element *
\***********************************************/
static void iColorBrowserDlgInit_Defaults(IcolorDlgData* colordlg_data)
{
  IupSetAttribute(colordlg_data->color_browser, "RGB", "0 0 0");

  IupSetAttribute(colordlg_data->red_txt,   "VALUE", "0");
  IupSetAttribute(colordlg_data->green_txt, "VALUE", "0");
  IupSetAttribute(colordlg_data->blue_txt,  "VALUE", "0");
  
  IupSetAttribute(colordlg_data->hue_txt,        "VALUE", "0");
  IupSetAttribute(colordlg_data->saturation_txt, "VALUE", "0");
  IupSetAttribute(colordlg_data->intensity_txt,  "VALUE", "0");

  IupSetAttribute(colordlg_data->colorhex_txt,  "VALUE", "000000");

  IupSetAttribute(colordlg_data->alpha_val, "VALUE", "255");
  IupSetAttribute(colordlg_data->alpha_txt, "VALUE", "255");

  iColorBrowserDlgRGB2HSI(colordlg_data);
}

/*******************************************\
* Sets an attribute and returns the element *
\*******************************************/
static Ihandle* iColorBrowserDlgSetAttribute(Ihandle* ih, const char* a, const char* v)
{
  IupSetAttribute(ih, a, v);
  return ih;
}


/**************************************************************************************************************/
/*                                 Internal Callbacks                                                         */
/**************************************************************************************************************/


static int iColorBrowserDlgButtonOK_CB(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  colordlg_data->status = 1;
  return IUP_CLOSE;
}

static int iColorBrowserDlgButtonCancel_CB(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  colordlg_data->status = 0;
  return IUP_CLOSE;
}

static int iColorBrowserDlgButtonHelp_CB(Ihandle* ih)
{
  Icallback cb = IupGetCallback(ih, "HELP_CB");
  if (cb && cb(ih) == IUP_CLOSE)
  {
    IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
    colordlg_data->status = 0;
    return IUP_CLOSE;
  }
  return IUP_DEFAULT;
}

static int iColorBrowserDlgColorCnvRepaint_CB(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  if (!colordlg_data->color_cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(colordlg_data->color_cddbuffer);

  iColorBrowserDlgColorCnvRepaint(colordlg_data);

  return IUP_DEFAULT;
}

static int iColorBrowserDlgRGBKillfocus_CB(Ihandle* ih) 
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  colordlg_data->red   = (unsigned char)IupGetInt(colordlg_data->red_txt, "VALUE");
  colordlg_data->green = (unsigned char)IupGetInt(colordlg_data->green_txt, "VALUE");
  colordlg_data->blue  = (unsigned char)IupGetInt(colordlg_data->blue_txt, "VALUE");

  iColorBrowserDlgRGB2HSI(colordlg_data);
  iColorBrowserDlgHex_TXT_Update(colordlg_data);
  iColorBrowserDlgHSI_TXT_Update(colordlg_data);
  iColorBrowserDlgView_Update(colordlg_data);

  return IUP_DEFAULT;
}

static int iColorBrowserDlgHSIKillfocus_CB(Ihandle* ih) 
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  colordlg_data->hue        = IupGetFloat(colordlg_data->hue_txt, "VALUE");
  colordlg_data->saturation = IupGetFloat(colordlg_data->saturation_txt, "VALUE")/100;
  colordlg_data->intensity  = IupGetFloat(colordlg_data->intensity_txt, "VALUE")/100;
  
  iColorBrowserDlgHSI2RGB(colordlg_data);
  iColorBrowserDlgHex_TXT_Update(colordlg_data);
  iColorBrowserDlgRGB_TXT_Update(colordlg_data);
  iColorBrowserDlgView_Update(colordlg_data);
  
  return IUP_DEFAULT;
}

static int iColorBrowserDlgHexDisplayKillfocus_CB(Ihandle* ih) 
{
  unsigned int ri, gi, bi;
  
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  ri = colordlg_data->red;
  gi = colordlg_data->green;
  bi = colordlg_data->blue;

  sscanf(IupGetAttribute(colordlg_data->colorhex_txt, "VALUE"), "%2X%2X%2X", &ri, &gi, &bi);

  colordlg_data->red   = (unsigned char)ri;
  colordlg_data->green = (unsigned char)gi;
  colordlg_data->blue  = (unsigned char)bi;

  iColorBrowserDlgRGB2HSI(colordlg_data);
  iColorBrowserDlgHSI_TXT_Update(colordlg_data);
  iColorBrowserDlgRGB_TXT_Update(colordlg_data);
  iColorBrowserDlgView_Update(colordlg_data);

  return IUP_DEFAULT;
}

static int iColorBrowserDlgColorSelDrag_CB(Ihandle* ih, unsigned char r, unsigned char g, unsigned char b)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  colordlg_data->red   = r;
  colordlg_data->green = g;
  colordlg_data->blue  = b;

  iColorBrowserDlgRGB2HSI(colordlg_data);
  iColorBrowserDlgHex_TXT_Update(colordlg_data);
  iColorBrowserDlgHSI_TXT_Update(colordlg_data);
  iColorBrowserDlgRGB_TXT_Update(colordlg_data);

  colordlg_data->color = cdEncodeColor(colordlg_data->red,colordlg_data->green,colordlg_data->blue);

  iColorBrowserDlgColorCnvRepaint(colordlg_data);

  return IUP_DEFAULT;
}

static int iColorBrowserDlgAlphaVal_CB(Ihandle* ih, double val)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  colordlg_data->alpha = (unsigned char)val;
  IupSetfAttribute(colordlg_data->alpha_txt, "VALUE", "%d", (int)colordlg_data->alpha);

  iColorBrowserDlgColorCnvRepaint(colordlg_data);

  return IUP_DEFAULT;  
}

static int iColorBrowserDlgAlphaKillfocus_CB(Ihandle* ih) 
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  colordlg_data->alpha = (unsigned char)IupGetInt(colordlg_data->alpha_txt, "VALUE");
  IupSetfAttribute(colordlg_data->alpha_val, "VALUE", "%d", (int)colordlg_data->alpha);

  iColorBrowserDlgColorCnvRepaint(colordlg_data);

  return IUP_DEFAULT;
}

static int iColorBrowserDlgColorTableSelect_CB(Ihandle* ih, int cell, int type)
{
  char* str = iupStrGetMemory(30);

  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  sprintf(str, "CELL%d", cell);
  iupStrToRGB(IupGetAttribute(ih, str), &colordlg_data->red, &colordlg_data->green, &colordlg_data->blue);

  iColorBrowserDlgRGB2HSI(colordlg_data);
  iColorBrowserDlgHex_TXT_Update(colordlg_data);
  iColorBrowserDlgHSI_TXT_Update(colordlg_data);
  iColorBrowserDlgRGB_TXT_Update(colordlg_data);
  iColorBrowserDlgView_Update(colordlg_data);

  (void)type;
  return IUP_DEFAULT;
}

static int iColorBrowserDlgColorCnvMap_CB(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  /* Create Canvas */
  colordlg_data->color_cdcanvas = cdCreateCanvas(CD_IUP, colordlg_data->color_cnv);

  if (!colordlg_data->color_cdcanvas)
    return IUP_DEFAULT;

  /* this can fail if canvas size is zero */
  colordlg_data->color_cddbuffer = cdCreateCanvas(CD_DBUFFERRGB, colordlg_data->color_cdcanvas);
  return IUP_DEFAULT;
}

static int iColorBrowserDlgColorCnvUnMap_CB(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  if (colordlg_data->color_cddbuffer)
    cdKillCanvas(colordlg_data->color_cddbuffer);

  if (colordlg_data->color_cdcanvas)
    cdKillCanvas(colordlg_data->color_cdcanvas);

  return IUP_DEFAULT;
}


/**************************************************************************************************************/
/*                                     Attributes                                                             */
/**************************************************************************************************************/


static char* iColorBrowserDlgGetStatusAttrib(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  if (colordlg_data->status) 
    return "1";
  else 
    return NULL;
}

static int iColorBrowserDlgSetAlphaAttrib(Ihandle* ih, const char* value)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  int alpha;
  if (iupStrToInt(value, &alpha))
  {
    colordlg_data->alpha = (unsigned char)alpha;
    IupSetfAttribute(colordlg_data->alpha_txt, "VALUE", "%d", (int)colordlg_data->alpha);
    IupSetfAttribute(colordlg_data->alpha_val, "VALUE", "%d", (int)colordlg_data->alpha);
    iColorBrowserDlgColorCnvRepaint(colordlg_data);
  }
 
  return 1;
}

static char* iColorBrowserDlgGetAlphaAttrib(Ihandle* ih)
{
  char* buffer = iupStrGetMemory(100);
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  sprintf(buffer, "%d", (int)colordlg_data->alpha);
  return buffer;
}

static int iColorBrowserDlgSetValueAttrib(Ihandle* ih, const char* value)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  if (iupStrToRGB(value, &colordlg_data->red, &colordlg_data->green, &colordlg_data->blue))
  {
    iColorBrowserDlgRGB2HSI(colordlg_data);
    iColorBrowserDlgHex_TXT_Update(colordlg_data);
    iColorBrowserDlgHSI_TXT_Update(colordlg_data);
    iColorBrowserDlgView_Update(colordlg_data);
  }
 
  return 0;
}

static char* iColorBrowserDlgGetValueAttrib(Ihandle* ih)
{
  char* buffer = iupStrGetMemory(100);
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  sprintf(buffer, "%d %d %d", (int)colordlg_data->red, (int)colordlg_data->green, (int)colordlg_data->blue);
  return buffer;
}

static char* iColorBrowserDlgGetColorTableAttrib(Ihandle* ih)
{
  int i, inc, off = 0;
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  char* color_str, attrib_str[30];
  char* str = iupStrGetMemory(300);
  for (i=0; i < 20; i++)
  {
    sprintf(attrib_str, "CELL%d", i);
    color_str = IupGetAttribute(colordlg_data->colortable_cbar, attrib_str);
    inc = strlen(color_str);
    memcpy(str+off, color_str, inc);
    off += inc;
  }
  str[off-1] = 0; /* remove last separator */
  return str;
}

static int iColorBrowserDlgSetColorTableAttrib(Ihandle* ih, const char* value)
{
  int i = 0;
  unsigned char r, g, b;
  char str[30];
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  while (value && *value && i < 20)
  {
    if (!iupStrToRGB(value, &r, &g, &b))
      return 0;

    sprintf(str, "CELL%d", i);
    IupSetfAttribute(colordlg_data->colortable_cbar, str, "%d %d %d", (int)r, (int)g, (int)b);

    value = strchr(value, ';');
    if (value) value++;
    i++;
  }

  return 1;
}


/**************************************************************************************************************/
/*                                     Methods                                                                */
/**************************************************************************************************************/

static int iColorBrowserDlgMapMethod(Ihandle* ih)
{
  char* value;
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");

  value = iupAttribGetStr(ih, "ALPHA");
  if (!value)
  {
    Ihandle* alpha_box = IupGetParent(colordlg_data->alpha_val);
    IupSetAttribute(alpha_box, "VISIBLE", "NO");
  }

  value = iupAttribGetStr(ih, "COLORTABLE");

  return IUP_NOERROR;
}

static void iColorBrowserDlgDestroyMethod(Ihandle* ih)
{
  IcolorDlgData* colordlg_data = (IcolorDlgData*)iupAttribGetStrInherit(ih, "_IUP_GC_DATA");
  free(colordlg_data);
}

static int iColorBrowserDlgCreateMethod(Ihandle* ih, void** params)
{
  Ihandle *rgb_vb, *hsi_vb, *clr_vb;
  Ihandle *box1, *box2, *col1, *col2, *col3, *col4;
  char* str = iupStrGetMemory(100);
  int i;

  IcolorDlgData* colordlg_data = (IcolorDlgData*)malloc(sizeof(IcolorDlgData));
  memset(colordlg_data, 0, sizeof(IcolorDlgData));
  iupAttribSetStr(ih, "_IUP_GC_DATA", (char*)colordlg_data);

  /* ======================================================================= */
  /* BUTTONS   ============================================================= */
  /* ======================================================================= */
  colordlg_data->ok_bt = IupButton("OK", NULL);                      /* Ok Button */
  IupSetAttribute(colordlg_data->ok_bt, "SIZE",   "32");      /* 12 Characters */
  IupSetCallback (colordlg_data->ok_bt, "ACTION", (Icallback)iColorBrowserDlgButtonOK_CB);
  IupSetAttributeHandle(ih, "DEFAULTENTER", colordlg_data->ok_bt);

  colordlg_data->cancel_bt = IupButton(iupStrMessageGet("IUP_CANCEL"), NULL);          /* Cancel Button */
  IupSetAttribute(colordlg_data->cancel_bt, "SIZE",   "32");  
  IupSetCallback (colordlg_data->cancel_bt, "ACTION", (Icallback)iColorBrowserDlgButtonCancel_CB);
  IupSetAttributeHandle(ih, "DEFAULTESC", colordlg_data->cancel_bt);

  colordlg_data->help_bt = IupButton(iupStrMessageGet("IUP_HELP"), NULL);            /* Help Button */
  IupSetAttribute(colordlg_data->help_bt, "SIZE",   "32");
  IupSetCallback (colordlg_data->help_bt, "ACTION", (Icallback)iColorBrowserDlgButtonHelp_CB);

  /* ======================================================================= */
  /* COLOR BROWSER   ======================================================= */
  /* ======================================================================= */
  colordlg_data->color_browser = IupColorBrowser();
  IupSetCallback(colordlg_data->color_browser, "DRAG_CB",   (Icallback)iColorBrowserDlgColorSelDrag_CB);
  IupSetCallback(colordlg_data->color_browser, "CHANGE_CB", (Icallback)iColorBrowserDlgColorSelDrag_CB);

  /* ======================================================================= */
  /* COLOR TABLE   ==-====================================================== */
  /* ======================================================================= */
  colordlg_data->colortable_cbar = IupColorbar();
  IupSetAttribute(colordlg_data->colortable_cbar, "ORIENTATION", "HORIZONTAL");
  IupSetAttribute(colordlg_data->colortable_cbar, "NUM_PARTS", "2");  
  IupSetAttribute(colordlg_data->colortable_cbar, "NUM_CELLS", "20");
  IupSetAttribute(colordlg_data->colortable_cbar, "SHOW_PREVIEW", "NO");
  for(i = 0; i < 20; i++)
  {
    sprintf(str, "CELL%d", i);
    IupSetAttribute(colordlg_data->colortable_cbar, str, default_colortable_cells[i]);
  }
  IupSetCallback (colordlg_data->colortable_cbar, "SELECT_CB",   (Icallback)iColorBrowserDlgColorTableSelect_CB);

  /* ======================================================================= */
  /* COLOR   ======================+++++++++================================ */
  /* ======================================================================= */
  colordlg_data->color_cnv = IupCanvas(NULL);  /* Canvas of the color */
  IupSetAttribute(colordlg_data->color_cnv, "RASTERSIZE", "61x25");
  IupSetAttribute(colordlg_data->color_cnv, "BORDER", "YES");
  IupSetAttribute(colordlg_data->color_cnv, "EXPAND", "NO");
  IupSetCallback (colordlg_data->color_cnv, "ACTION", (Icallback)iColorBrowserDlgColorCnvRepaint_CB);
  IupSetCallback (colordlg_data->color_cnv, "MAP_CB", (Icallback)iColorBrowserDlgColorCnvMap_CB);
  IupSetCallback (colordlg_data->color_cnv, "UNMAP_CB", (Icallback)iColorBrowserDlgColorCnvUnMap_CB);

  colordlg_data->colorhex_txt = IupText(NULL);      /* Hex of the color */
  IupSetAttribute(colordlg_data->colorhex_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->colorhex_txt, "VALUE", "000000");
  IupSetCallback (colordlg_data->colorhex_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgHexDisplayKillfocus_CB);

  /* ======================================================================= */
  /* ALPHA TRANSPARENCY   ================================================== */
  /* ======================================================================= */
  colordlg_data->alpha_val = IupVal("HORIZONTAL");
  IupSetAttribute(colordlg_data->alpha_val, "MIN", "0");
  IupSetAttribute(colordlg_data->alpha_val, "MAX", "255");
  IupSetAttribute(colordlg_data->alpha_val, "VALUE", "255");
  IupSetAttribute(colordlg_data->alpha_val, "RASTERSIZE", "176");
  IupSetCallback (colordlg_data->alpha_val, "MOUSEMOVE_CB", (Icallback)iColorBrowserDlgAlphaVal_CB);
  IupSetCallback (colordlg_data->alpha_val, "BUTTON_PRESS_CB", (Icallback)iColorBrowserDlgAlphaVal_CB);
  IupSetCallback (colordlg_data->alpha_val, "BUTTON_RELEASE_CB", (Icallback)iColorBrowserDlgAlphaVal_CB);

  /* ======================================================================= */
  /* RGB TEXT FIELDS   ===================================================== */
  /* ======================================================================= */
  colordlg_data->red_txt = IupText(NULL);                            /* Red value */
  IupSetAttribute(colordlg_data->red_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->red_txt, "SPIN", "YES");
  IupSetAttribute(colordlg_data->red_txt, "SPINMIN", "0");
  IupSetAttribute(colordlg_data->red_txt, "SPINMAX", "255");
  IupSetAttribute(colordlg_data->red_txt, "SPININC", "1");
  IupSetCallback (colordlg_data->red_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgRGBKillfocus_CB);
  iupmaskSet(colordlg_data->red_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  colordlg_data->green_txt = IupText(NULL);                        /* Green value */
  IupSetAttribute(colordlg_data->green_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->green_txt, "SPIN", "YES");
  IupSetAttribute(colordlg_data->green_txt, "SPINMIN", "0");
  IupSetAttribute(colordlg_data->green_txt, "SPINMAX", "255");
  IupSetAttribute(colordlg_data->green_txt, "SPININC", "1");
  IupSetCallback (colordlg_data->green_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgRGBKillfocus_CB);
  iupmaskSet(colordlg_data->green_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  colordlg_data->blue_txt = IupText(NULL);                          /* Blue value */
  IupSetAttribute(colordlg_data->blue_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->blue_txt, "SPIN", "YES");
  IupSetAttribute(colordlg_data->blue_txt, "SPINMIN", "0");
  IupSetAttribute(colordlg_data->blue_txt, "SPINMAX", "255");
  IupSetAttribute(colordlg_data->blue_txt, "SPININC", "1");
  IupSetCallback (colordlg_data->blue_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgRGBKillfocus_CB);
  iupmaskSet(colordlg_data->blue_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  colordlg_data->alpha_txt = IupText(NULL);                        /* Alpha value */
  IupSetAttribute(colordlg_data->alpha_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->alpha_txt, "VALUE", "255");
  IupSetCallback (colordlg_data->alpha_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgAlphaKillfocus_CB);
  iupmaskSet(colordlg_data->alpha_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  /* ======================================================================= */
  /* HSI TEXT FIELDS   ===================================================== */
  /* ======================================================================= */
  colordlg_data->hue_txt = IupText(NULL);                            /* Hue value */
  IupSetAttribute(colordlg_data->hue_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->hue_txt, "SPIN", "YES");
  IupSetAttribute(colordlg_data->hue_txt, "SPINMIN", "0");
  IupSetAttribute(colordlg_data->hue_txt, "SPINMAX", "360");
  IupSetAttribute(colordlg_data->hue_txt, "SPININC", "1");
  IupSetCallback(colordlg_data->hue_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgHSIKillfocus_CB);
  iupmaskSet(colordlg_data->hue_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  colordlg_data->saturation_txt = IupText(NULL);              /* Saturation value */
  IupSetAttribute(colordlg_data->saturation_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->saturation_txt, "SPIN", "YES");
  IupSetAttribute(colordlg_data->saturation_txt, "SPINMIN", "0");
  IupSetAttribute(colordlg_data->saturation_txt, "SPINMAX", "100");
  IupSetAttribute(colordlg_data->saturation_txt, "SPININC", "1");
  IupSetCallback(colordlg_data->saturation_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgHSIKillfocus_CB);
  iupmaskSet(colordlg_data->saturation_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  colordlg_data->intensity_txt = IupText(NULL);                /* Intensity value */
  IupSetAttribute(colordlg_data->intensity_txt, "SIZE", "32");
  IupSetAttribute(colordlg_data->intensity_txt, "SPIN", "YES");
  IupSetAttribute(colordlg_data->intensity_txt, "SPINMIN", "0");
  IupSetAttribute(colordlg_data->intensity_txt, "SPINMAX", "100");
  IupSetAttribute(colordlg_data->intensity_txt, "SPININC", "1");
  IupSetCallback(colordlg_data->intensity_txt, "KILLFOCUS_CB", (Icallback)iColorBrowserDlgHSIKillfocus_CB);
  iupmaskSet(colordlg_data->intensity_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  /* =================== */
  /* 1st line = Controls */
  /* =================== */
  box1 = IupHbox(NULL);

  col1 = IupVbox(IupHbox(IupFill(), colordlg_data->color_browser,  IupFill(), NULL), 
                 IupHbox(IupFill(), colordlg_data->color_cnv, IupFill(), NULL), 
                 NULL);
  IupAppend(box1, col1);

  hsi_vb = IupVbox(IupHbox(iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_HUE")), "SIZE", "32"),
                           colordlg_data->hue_txt, IupFill(), NULL),
                   IupHbox(iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_SATURATION")), "SIZE", "32"),
                           colordlg_data->saturation_txt, IupFill(), NULL),
                   IupHbox(iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_INTENSITY")), "SIZE", "32"),
                           colordlg_data->intensity_txt, IupFill(), NULL),
                   NULL);

  rgb_vb = IupVbox(IupHbox(IupFill(), iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_RED")), "SIZE", "32"),
                           colordlg_data->red_txt, NULL),
                   IupHbox(IupFill(), iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_GREEN")), "SIZE", "32"),
                           colordlg_data->green_txt, NULL), 
                   IupHbox(IupFill(), iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_BLUE")), "SIZE", "32"),
                           colordlg_data->blue_txt, NULL), 
                   NULL);
  
  clr_vb = IupVbox(IupHbox(IupFill(), iColorBrowserDlgSetAttribute(IupLabel(NULL), "SEPARATOR", "HORIZONTAL"),
                           IupFill(), NULL),
                   IupHbox(iColorBrowserDlgSetAttribute(IupLabel(iupStrMessageGet("IUP_OPACITY")), "SIZE", "35"),
                           colordlg_data->alpha_val, colordlg_data->alpha_txt, IupFill(), NULL),
                   IupHbox(IupLabel("Hex"), colordlg_data->colorhex_txt, IupFill(), NULL), 
                   IupHbox(IupFill(), IupLabel(iupStrMessageGet("IUP_PALETTE")), colordlg_data->colortable_cbar, NULL), 
                   NULL);

  col2 = IupVbox(IupHbox(hsi_vb, rgb_vb, NULL), IupHbox(clr_vb, NULL), NULL);
  IupAppend(box1, col2);

  /* ================== */
  /* 2nd line = Buttons */
  /* ================== */
  box2 = IupHbox(NULL);

  col3 = IupVbox(IupHbox(colordlg_data->help_bt, IupFill(), NULL), NULL);
  IupAppend(box2, col3);

  col4 = IupVbox(IupHbox(IupFill(), colordlg_data->cancel_bt, colordlg_data->ok_bt, NULL), NULL);
  IupAppend(box2, col4);

  IupAppend(ih, IupVbox(box1, box2, NULL));

  iColorBrowserDlgInit_Defaults(colordlg_data);

  (void)params;
  return IUP_NOERROR;
}

Iclass* iupColorBrowserDlgGetClass(void)
{
  Iclass* ic = iupClassNew(iupDialogGetClass());

  ic->Create = iColorBrowserDlgCreateMethod;
  ic->Destroy = iColorBrowserDlgDestroyMethod;
  ic->Map = iColorBrowserDlgMapMethod;

  ic->name = "colordlg";   /* this will hide the GTK and Windows implementations */
  ic->nativetype = IUP_TYPEDIALOG;
  ic->is_interactive = 1;
  ic->childtype = IUP_CHILD_ONE;

  iupClassRegisterAttribute(ic, "COLORTABLE", iColorBrowserDlgGetColorTableAttrib, iColorBrowserDlgSetColorTableAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STATUS", iColorBrowserDlgGetStatusAttrib, NULL, "YES", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iColorBrowserDlgGetValueAttrib, iColorBrowserDlgSetValueAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ALPHA", iColorBrowserDlgGetAlphaAttrib, iColorBrowserDlgSetAlphaAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}

/*
desenho do alpha
SHOWHEX

mudar atualizacao para ACTION em vez de focus
      verificar atualizacao do SPIN

K_ANY GTK
*/
