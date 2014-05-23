/** \file
 * \brief GLSubCanvas Control.
 *
 * See Copyright Notice in "iup.h"
 */

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined (OSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupgl.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iup_glcontrols.h"


void iupGLSubCanvasSetTransform(Ihandle* ih, Ihandle* gl_parent)
{
  /* y is at bottom and oriented bottom to top in OpenGL */
  int y = ih->y + ih->currentheight - 1;  /* move to bottom */
  y = gl_parent->currentheight - 1 - y; /* orient bottom to top */
  glViewport(ih->x, y, ih->currentwidth, ih->currentheight);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, ih->currentwidth, 0, ih->currentheight, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.375, 0.375, 0.0);  /* render all primitives at integer positions */
}

void iupGLSubCanvasSaveState(void)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_POLYGON_STIPPLE);
}

void iupGLSubCanvasRestoreState(Ihandle* gl_parent)
{
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glViewport(0, 0, gl_parent->currentwidth, gl_parent->currentheight);

  glPopAttrib();
}

void iupGLSubCanvasRedrawFront(Ihandle* ih)
{
  IFn cb = (IFn)IupGetCallback(ih, "GL_ACTION");
  if (cb && iupAttribGetInt(ih, "VISIBLE"))
  {
    Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
    IupGLMakeCurrent(gl_parent);
    glDrawBuffer(GL_FRONT);
    iupGLSubCanvasSaveState();
    iupGLSubCanvasSetTransform(ih, gl_parent);
    cb(ih);
    iupGLSubCanvasRestoreState(gl_parent);
    glDrawBuffer(GL_BACK);
  }
}

int iupGLSubCanvasRestoreRedraw(Ihandle* ih)
{
  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  iupGLSubCanvasRestoreState(gl_parent);
  IupSetAttribute(gl_parent, "REDRAW", NULL);
  return IUP_DEFAULT;
}

static char* iGLSubCanvasGetCharSizeAttrib(Ihandle* ih)
{
  int charwidth, charheight;

  iupGLFontGetCharSize(ih, &charwidth, &charheight);
  if (charwidth == 0 || charheight == 0)
    return NULL;

  return iupStrReturnIntInt(charwidth, charheight, 'x');
}

static int iGLSubCanvasSetSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->userwidth = 0;
    ih->userheight = 0;
  }
  else
  {
    /* if not specified, the value is 0 */
    int w = 0, h = 0;
    int charwidth, charheight;
    iupGLFontGetCharSize(ih, &charwidth, &charheight);
    iupStrToIntInt(value, &w, &h, 'x');
    if (w < 0) w = 0;
    if (h < 0) h = 0;

    ih->userwidth = iupWIDTH2RASTER(w, charwidth);
    ih->userheight = iupHEIGHT2RASTER(h, charheight);
  }
  return 1;  /* always save in the hash table, so when FONT is changed SIZE can be updated */
}

static char* iGLSubCanvasGetSizeAttrib(Ihandle* ih)
{
  int charwidth, charheight, width, height;

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

  iupGLFontGetCharSize(ih, &charwidth, &charheight);
  if (charwidth == 0 || charheight == 0)
    return NULL;  /* if font failed get from the hash table */

  return iupStrReturnIntInt(iupRASTER2WIDTH(width, charwidth), iupRASTER2HEIGHT(height, charheight), 'x');
}

void iupGLSubCanvasUpdateSizeFromFont(Ihandle* ih)
{
  char* value = iupAttribGet(ih, "SIZE");
  if (!value)
    return;

  iGLSubCanvasSetSizeAttrib(ih, value);
}

static int iGLSubCanvasSetRedrawFrontAttrib(Ihandle* ih, const char* value)
{
  iupGLSubCanvasRedrawFront(ih);
  (void)value;
  return 0;
}

static void iGLSubCanvasSetZorder(Ihandle* parent, Ihandle* child, int top)
{
  Ihandle *c,
    *c_prev = NULL;

  if (top && child->brother == NULL)  /* already at the top */
    return;

  if (!top && parent->firstchild == child)  /* already at the bottom */
    return;

  for (c = parent->firstchild; c; c = c->brother)
  {
    if (c == child) /* Found the right child */
    {
      /* remove it from the hierarchy */
      if (c_prev == NULL)
        parent->firstchild = child->brother;
      else
        c_prev->brother = child->brother;

      child->brother = NULL;
    }

    c_prev = c;

    /* at last element */
    if (!c->brother)
    {
      if (top)
      {
        /* insert at last */
        c->brother = child;
      }
      else
      {
        /* insert at first */
        child->brother = parent->firstchild;
        parent->firstchild = child;
      }
    }
  }
}

static int iGLSubCanvasSetZorderAttrib(Ihandle* ih, const char* value)
{
  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  int top = 1;
  if (iupStrEqualNoCase(value, "BOTTOM"))
    top = 0;

  while (ih != gl_parent)
  {
    iGLSubCanvasSetZorder(ih->parent, ih, top);
    ih = ih->parent;
  }

return 0;
}


static int iGLSubCanvasMapMethod(Ihandle* ih)
{
  Ihandle* gl_parent;
  for (gl_parent = ih->parent; gl_parent; gl_parent = gl_parent->parent)
  {
    if (gl_parent->iclass->nativetype == IUP_TYPECANVAS)
      break;
  }

  /* must have a canvas parent, and it must be a glcanvasbox */
  if (!gl_parent || !iupClassMatch(gl_parent->iclass, "glcanvasbox"))
    return IUP_ERROR;

  iupAttribSet(ih, "GL_CANVAS", (char*)gl_parent);

  /* use the handle of the native parent */
  ih->handle = gl_parent->handle;
  return IUP_NOERROR;
}

static void iGLSubCanvasRelease(Iclass* ic)
{
  iupGLFontFinish();
  (void)ic;
}

Iclass* iupGLSubCanvasNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "glsubcanvas";
  ic->format = NULL;  /* no parameters */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupGLSubCanvasNewClass;
  ic->Release = iGLSubCanvasRelease;
  ic->Map = iGLSubCanvasMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "GL_ACTION", "");
  iupClassRegisterCallback(ic, "GL_BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "GL_ENTERWINDOW_CB", "");
  iupClassRegisterCallback(ic, "GL_LEAVEWINDOW_CB", "");
  iupClassRegisterCallback(ic, "GL_MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "GL_WHEEL_CB", "fiis");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* redefine common attributes */
  iupClassRegisterAttribute(ic, "SIZE", iGLSubCanvasGetSizeAttrib, iGLSubCanvasSetSizeAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CHARSIZE", iGLSubCanvasGetCharSizeAttrib, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iupGLSetStandardFontAttrib, IUPAF_SAMEASSYSTEM, "DEFAULTFONT", IUPAF_NO_SAVE | IUPAF_NOT_MAPPED);  /* use inheritance to retrieve standard fonts */

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* redefine native visual attributes */
  iupClassRegisterAttribute(ic, "VISIBLE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_SAVE | IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "ACTIVE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "ZORDER", NULL, iGLSubCanvasSetZorderAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

  /* Common visual */
  iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERWIDTH", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, NULL, NULL, NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 225 245", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "PRESSCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "150 200 235", IUPAF_DEFAULT);  /* inheritable */

  iupClassRegisterAttribute(ic, "REDRAWFRONT", NULL, iGLSubCanvasSetRedrawFrontAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

  iupGLFontInit();

  return ic;
}

Ihandle* IupGLSubCanvas(void)
{
  return IupCreate("glsubcanvas");
}

