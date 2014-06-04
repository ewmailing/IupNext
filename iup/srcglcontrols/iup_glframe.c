/** \file
 * \brief Frame Control.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_register.h"
#include "iup_layout.h"

#include "iup_glcontrols.h"


#if 0
static void winFrameDrawItem(Ihandle* ih, DRAWITEMSTRUCT *drawitem)
{
  iupwinBitmapDC bmpDC;
  HDC hDC = iupwinDrawCreateBitmapDC(&bmpDC, drawitem->hDC, 0, 0, drawitem->rcItem.right - drawitem->rcItem.left,
    drawitem->rcItem.bottom - drawitem->rcItem.top);

  iupwinDrawParentBackground(ih, hDC, &drawitem->rcItem);

  if (iupAttribGet(ih, "_IUPFRAME_HAS_TITLE"))
  {
    int x, y;
    HFONT hOldFont, hFont = (HFONT)iupwinGetHFontAttrib(ih);
    int txt_height = iGLFrameGetTitleHeight(ih);
    COLORREF fgcolor;
    SIZE size;

    char* title = iupAttribGet(ih, "TITLE");
    if (!title) title = "";

    x = drawitem->rcItem.left + 7;
    y = drawitem->rcItem.top;

    hOldFont = SelectObject(hDC, hFont);
    {
      TCHAR* str = iupwinStrToSystem(title);
      int len = lstrlen(str);
      GetTextExtentPoint32(hDC, str, len, &size);
    }
    ExcludeClipRect(hDC, x - 2, y, x + size.cx + 2, y + size.cy);

    drawitem->rcItem.top += txt_height / 2;
    if (iupwin_comctl32ver6)
      iupwinDrawThemeFrameBorder(ih->handle, hDC, &drawitem->rcItem, drawitem->itemState);
    else
      DrawEdge(hDC, &drawitem->rcItem, EDGE_ETCHED, BF_RECT);

    SelectClipRgn(hDC, NULL);

    if (drawitem->itemState & ODS_DISABLED)
      fgcolor = GetSysColor(COLOR_GRAYTEXT);
    else
    {
      unsigned char r, g, b;
      char* color = iupAttribGetInherit(ih, "FGCOLOR");
      if (!color)
      {
        if (!iupwinDrawGetThemeFrameFgColor(ih->handle, &fgcolor))
          fgcolor = 0;  /* black */
      }
      else
      {
        if (iupStrToRGB(color, &r, &g, &b))
          fgcolor = RGB(r, g, b);
        else
          fgcolor = 0;  /* black */
      }
    }

    winFrameDrawText(hDC, title, x, y, fgcolor);

    SelectObject(hDC, hOldFont);
  }
  else
  {
    char* value = iupAttribGetStr(ih, "SUNKEN");
    if (iupStrBoolean(value))
      DrawEdge(hDC, &drawitem->rcItem, EDGE_SUNKEN, BF_RECT);
    else
      DrawEdge(hDC, &drawitem->rcItem, EDGE_ETCHED, BF_RECT);

    if (iupAttribGet(ih, "_IUPFRAME_HAS_BGCOLOR"))
    {
      unsigned char r = 0, g = 0, b = 0;
      char* color = iupAttribGetStr(ih, "BGCOLOR");
      iupStrToRGB(color, &r, &g, &b);
      SetDCBrushColor(hDC, RGB(r, g, b));
      InflateRect(&drawitem->rcItem, -2, -2);
      FillRect(hDC, &drawitem->rcItem, (HBRUSH)GetStockObject(DC_BRUSH));
    }
  }

  iupwinDrawDestroyBitmapDC(&bmpDC);
}
#endif

static int iGLFrameACTION(Ihandle* ih)
{
  char *image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  int active = iupAttribGetInt(ih, "ACTIVE");
  char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
  char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border_width = (int)ceil(bwidth);

  /* draw border - can still be disabled setting bwidth=0 */
  char* bcolor = iupAttribGetStr(ih, "BORDERCOLOR");
  iupGLDrawRect(ih, 0, ih->currentwidth - 1, 0, ih->currentheight - 1, bwidth, bcolor, active, 0);

  /* draw background */
  iupGLDrawBox(ih, border_width, ih->currentwidth - 2 * border_width,
                   border_width, ih->currentheight - 2 * border_width, bgcolor);

  iupGLIconDraw(ih, border_width, border_width,
    ih->currentwidth - 2 * border_width, ih->currentheight - 2 * border_width,
    image, title, fgcolor, active);

  return IUP_DEFAULT;
}

static void iGLFrameGetDecorOffset(Ihandle* ih, int *dx, int *dy)
{
  char* image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border_width = (int)ceil(bwidth);
  *dx = border_width;
  *dy = 2 * border_width;

  if (image || title)
  {
    int natural_w = 0,
        natural_h = 0;
    iupGLIconGetNaturalSize(ih, image, title, &natural_w, &natural_h);

    (*dy) += natural_h;
  }
}

static void iGLFrameGetDecorSize(Ihandle* ih, int *width, int *height, int* full_width)
{
  char* image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border_width = (int)ceil(bwidth);
  *width = 2 * border_width;
  *height = 2 * border_width;

  if (image || title)
  {
    int natural_w = 0,
        natural_h = 0;
    iupGLIconGetNaturalSize(ih, image, title, &natural_w, &natural_h);

    (*height) += natural_h;
    if (full_width)
      *full_width = *width + natural_w;
  }
}

static char* iGLFrameGetClientSizeAttrib(Ihandle* ih)
{
  int width, height, decorwidth, decorheight;
  width = ih->currentwidth;
  height = ih->currentheight;
  iGLFrameGetDecorSize(ih, &decorwidth, &decorheight, NULL);
  width -= decorwidth;
  height -= decorheight;
  if (width < 0) width = 0;
  if (height < 0) height = 0;
  return iupStrReturnIntInt(width, height, 'x');
}

static char* iGLFrameGetClientOffsetAttrib(Ihandle* ih)
{
  int dx, dy;
  iGLFrameGetDecorOffset(ih, &dx, &dy);
  return iupStrReturnIntInt(dx, dy, 'x');
}

static void iGLFrameComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int decorwidth, decorheight, full_width;
  Ihandle* child = ih->firstchild;

  iGLFrameGetDecorSize(ih, &decorwidth, &decorheight, &full_width);
  *w = full_width;  /* make room for title always */
  *h = decorheight;

  if (child)
  {
    /* update child natural size first */
    iupBaseComputeNaturalSize(child);

    *children_expand = child->expand;
    *w += child->naturalwidth;
    *h += child->naturalheight;
  }
}

static void iGLFrameSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  if (ih->firstchild)
  {
    int width, height, decorwidth, decorheight;

    iGLFrameGetDecorSize(ih, &decorwidth, &decorheight, NULL);

    width = ih->currentwidth - decorwidth;
    height = ih->currentheight - decorheight;
    if (width < 0) width = 0;
    if (height < 0) height = 0;

    iupBaseSetCurrentSize(ih->firstchild, width, height, shrink);
  }
}

static void iGLFrameSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  if (ih->firstchild)
  {
    int dx, dy;
    iGLFrameGetDecorOffset(ih, &dx, &dy);

    iupBaseSetPosition(ih->firstchild, x + dx, y + dy);
  }
}

static int iGLFrameCreateMethod(Ihandle* ih, void** params)
{
  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  IupSetCallback(ih, "GL_ACTION", iGLFrameACTION);

  return IUP_NOERROR;
}


/******************************************************************************/


Iclass* iupGLFrameNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glsubcanvas"));

  ic->name = "glframe";
  ic->format = "h"; /* one ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDMANY+1;   /* one child */
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupGLFrameNewClass;
  ic->Create = iGLFrameCreateMethod;

  ic->ComputeNaturalSize = iGLFrameComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iGLFrameSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iGLFrameSetChildrenPositionMethod;

  /* Base Container */
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iGLFrameGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iGLFrameGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* Visual */
  iupClassRegisterAttribute(ic, "IMAGE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEPRESS", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEINACTIVE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, "200 225 245", NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "TXTCOLOR", NULL, NULL, "0 0 0", NULL, IUPAF_DEFAULT);  /* inheritable */

  return ic;
}

Ihandle* IupGLFrame(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("glframe", children);
}
