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


static int iGLFrameACTION(Ihandle* ih)
{
  char *image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  int active = iupAttribGetInt(ih, "ACTIVE");
  char* bcolor = iupAttribGetStr(ih, "BORDERCOLOR");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border_width = (int)ceil(bwidth);


  if (image || title)
  {
    char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
    int off = iupAttribGetInt(ih, "TITLEOFFSET");
    int natural_w = 0,
      natural_h = 0;
    iupGLIconGetNaturalSize(ih, image, title, &natural_w, &natural_h);
    if (natural_w > ih->currentwidth - 2 * border_width)
      natural_w = ih->currentwidth - 2 * border_width;

    /* draw frame border */
    iupGLDrawFrameRect(ih, 0, ih->currentwidth - 1, 0, ih->currentheight - 1, bwidth, bcolor, active, off, natural_w, natural_h);

    iupGLIconDraw(ih, off, 0,
                      natural_w, natural_h,
                      image, title, fgcolor, active);
  }
  else
  {
    char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");

    /* draw border - can still be disabled setting bwidth=0 */
    iupGLDrawRect(ih, 0, ih->currentwidth - 1, 0, ih->currentheight - 1, bwidth, bcolor, active, 0);

    /* draw background */
    iupGLDrawBox(ih, border_width, ih->currentwidth - 2 * border_width,
                     border_width, ih->currentheight - 2 * border_width, bgcolor);
  }

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

static void iGLFrameGetDecorSize(Ihandle* ih, int *width, int *height, int* title_width)
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
    if (title_width)
      *title_width = natural_w;
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
  int decorwidth, decorheight, title_width;
  Ihandle* child = ih->firstchild;

  iGLFrameGetDecorSize(ih, &decorwidth, &decorheight, &title_width);
  *w = decorwidth;  
  *h = decorheight;

  if (child)
  {
    /* update child natural size first */
    iupBaseComputeNaturalSize(child);

    *children_expand = child->expand;
    *w += iupMAX(child->naturalwidth, title_width);  /* make room for title always */
    *h += child->naturalheight;
  }
  else
    *w += title_width;
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
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "TITLEOFFSET", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_DEFAULT);  /* inheritable */

  /* replace default value */
  iupClassRegisterAttribute(ic, "PADDING", NULL, NULL, IUPAF_SAMEASSYSTEM, "2x0", IUPAF_DEFAULT);  /* inheritable */

  return ic;
}

Ihandle* IupGLFrame(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("glframe", children);
}
