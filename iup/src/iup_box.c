/** \file
 * \brief Base for box Controls.
 *
 * See Copyright Notice in iup.ih
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_box.h"


void iupBoxNormalizeSize(Ihandle *ih, int children_natural_maxwidth, int children_natural_maxheight)
{
  Ihandle* child;
  enum{NORMALIZE_NONE, NORMALIZE_WIDTH, NORMALIZE_HEIGHT};
  int normalize = NORMALIZE_NONE;

  char* value = iupAttribGetStr(ih, "NORMALIZESIZE");
  if (!value)
    return;

  if (iupStrEqualNoCase(value, "HORIZONTAL"))
    normalize = NORMALIZE_WIDTH;
  else if (iupStrEqualNoCase(value, "VERTICAL"))
    normalize = NORMALIZE_HEIGHT;
  else if (iupStrEqualNoCase(value, "BOTH"))
    normalize = NORMALIZE_WIDTH|NORMALIZE_HEIGHT;
  else 
    return;

  /* reset the natural width and/or height */
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (normalize & NORMALIZE_WIDTH)
      child->naturalwidth = children_natural_maxwidth;
    if (normalize & NORMALIZE_HEIGHT)
      child->naturalheight = children_natural_maxheight;
  }
}

static int iBoxCreateMethod(Ihandle* ih, void** params)
{
  ih->data = iupALLOCCTRLDATA();

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    while (*iparams) 
    {
      IupAppend(ih, *iparams);
      iparams++;
    }
  }

  return IUP_NOERROR;
}

static int iBoxMapMethod(Ihandle* ih)
{
  ih->handle = (InativeHandle*)-1; /* fake value just to indicate that it is already mapped */
  return IUP_NOERROR;
}

static char* iBoxGetClientSizeAttrib(Ihandle* ih)
{
  char* str;
  int width, height;

  if (ih->handle)
  {
    width = ih->currentwidth;
    height = ih->currentheight;
  }
  else
  {
    width = ih->userwidth;
    height = ih->userheight;
  }

  if (!width && !height)
    return NULL;

  str = iupStrGetMemory(50);

  width -= 2*ih->data->margin_x;
  height -= 2*ih->data->margin_y;

  sprintf(str, "%dx%d", width, height);
  return str;
}

static int iBoxSetCGapAttrib(Ihandle* ih, const char* value)
{
  int cgap;
  iupStrToInt(value, &cgap);
  if (iupStrEqual(ih->iclass->name, "vbox"))
  {
    int charheight;
    iupdrvFontGetCharSize(ih, NULL, &charheight);
    ih->data->gap = iupHEIGHT2RASTER(cgap, charheight);
  }
  else
  {
    int charwidth;
    iupdrvFontGetCharSize(ih, &charwidth, NULL);
    ih->data->gap = iupWIDTH2RASTER(cgap, charwidth);
  }
  return 0;
}

static char* iBoxGetCGapAttrib(Ihandle* ih)
{
  char *str = iupStrGetMemory(50);
  if (iupStrEqual(ih->iclass->name, "vbox"))
  {
    int charheight;
    iupdrvFontGetCharSize(ih, NULL, &charheight);
    sprintf(str, "%d", iupRASTER2HEIGHT(ih->data->gap, charheight));
  }
  else
  {
    int charwidth;
    iupdrvFontGetCharSize(ih, &charwidth, NULL);
    sprintf(str, "%d", iupRASTER2WIDTH(ih->data->gap, charwidth));
  }
  return str;
}

static int iBoxSetGapAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->gap);
  return 0;
}

static char* iBoxGetGapAttrib(Ihandle* ih)
{
  char *str = iupStrGetMemory(50);
  sprintf(str, "%d", ih->data->gap);
  return str;
}

static int iBoxSetCMarginAttrib(Ihandle* ih, const char* value)
{
  int cmargin_x=-1, cmargin_y=-1;
  int charwidth, charheight;
  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  iupStrToIntInt(value, &cmargin_x, &cmargin_y, 'x');
  if (cmargin_x!=-1)
    ih->data->margin_x = iupHEIGHT2RASTER(cmargin_x, charheight);
  if (cmargin_y!=-1)
    ih->data->margin_x = iupWIDTH2RASTER(cmargin_y, charwidth);
  return 0;
}

static char* iBoxGetCMarginAttrib(Ihandle* ih)
{
  char *str = iupStrGetMemory(50);
  int charwidth, charheight;
  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  sprintf(str, "%dx%d", iupRASTER2WIDTH(ih->data->margin_x, charwidth), iupRASTER2HEIGHT(ih->data->margin_y, charheight));
  return str;
}

static int iBoxSetMarginAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->margin_x, &ih->data->margin_y, 'x');
  return 0;
}

static char* iBoxGetMarginAttrib(Ihandle* ih)
{
  char *str = iupStrGetMemory(50);
  sprintf(str, "%dx%d", ih->data->margin_x, ih->data->margin_y);
  return str;
}


/******************************************************************************/


Iclass* iupBoxClassBase(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->format = "g"; /* array of Ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDMANY;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = iBoxCreateMethod;
  ic->Map = iBoxMapMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, "YES", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iBoxGetClientSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* boxes only */
  iupClassRegisterAttribute(ic, "GAP", iBoxGetGapAttrib, iBoxSetGapAttrib, "0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "CGAP", iBoxGetCGapAttrib, iBoxSetCGapAttrib, "0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "MARGIN", iBoxGetMarginAttrib, iBoxSetMarginAttrib, "0x0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "CMARGIN", iBoxGetCMarginAttrib, iBoxSetCMarginAttrib, "0x0", IUP_NOT_MAPPED, IUP_INHERIT);

  return ic;
}
