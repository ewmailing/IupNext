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

#include "iup_assert.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iup_glcontrols.h"


int iupGLSubCanvasSetTransform(Ihandle* ih, Ihandle* gl_parent)
{
  int x = ih->x;
  int y = ih->y;
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int parent_x = iupAttribGetInt(ih->parent, "VIEWPORT_X");
  int parent_y = iupAttribGetInt(ih->parent, "VIEWPORT_Y");
  int parent_width = iupAttribGetInt(ih->parent, "VIEWPORT_WIDTH");
  int parent_height = iupAttribGetInt(ih->parent, "VIEWPORT_HEIGHT");

  /* crop to parent's rectangle */
  if (x < parent_x)
    x = parent_x;
  if (y < parent_y)
    y = parent_y;
  if (x + width > parent_x + parent_width)
    width = parent_x + parent_width - x;
  if (y + height > parent_y + parent_height)
    height = parent_y + parent_height - y;

  if (width < 0) width = 0;
  if (height < 0) height = 0;

  iupAttribSetInt(ih, "VIEWPORT_X", x);
  iupAttribSetInt(ih, "VIEWPORT_Y", y);
  iupAttribSetInt(ih, "VIEWPORT_WIDTH", width);
  iupAttribSetInt(ih, "VIEWPORT_HEIGHT", height);

  if (width == 0 || height == 0)
    return 0;

  /* y is at bottom and oriented bottom to top in OpenGL */
  y = y + ih->currentheight - 1;  /* move to bottom */
  y = gl_parent->currentheight - 1 - y; /* orient bottom to top */

  glViewport(x, y, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, 0, height, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.375, 0.375, 0.0);  /* render all primitives at integer positions */

  return 1;
}

void iupGLSubCanvasSaveState(Ihandle* gl_parent)
{
  char* saved = iupAttribGet(gl_parent, "_IUP_GLSUBCANVAS_SAVED");
  iupASSERT(saved == NULL);
  if (saved)
    return;

  /* save all changes, except transformation matrix */
  glPushAttrib(GL_ENABLE_BIT | GL_HINT_BIT | GL_VIEWPORT_BIT);

  /* save transformation matrix */
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  /* save transformation matrix */
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  /* alpha enabled */
  glEnable(GL_BLEND);     /* GL_COLOR_BUFFER_BIT or GL_ENABLE_BIT */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  /* image data alignment */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  /* not saved */
  /* disable Z axis */
  glDisable(GL_DEPTH_TEST);  /* GL_DEPTH_BUFFER_BIT or GL_ENABLE_BIT */
  /* anti-alias */
  glEnable(GL_POINT_SMOOTH);     /* GL_LINE_BIT or GL_ENABLE_BIT */
  glEnable(GL_LINE_SMOOTH);      /* GL_POINT_BIT or GL_ENABLE_BIT */
  glEnable(GL_POLYGON_SMOOTH);    /* GL_POLYGON_BIT or GL_ENABLE_BIT */
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);   /* GL_HINT_BIT */
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  iupAttribSet(gl_parent, "_IUP_GLSUBCANVAS_SAVED", "1");

  iupAttribSetInt(gl_parent, "VIEWPORT_X", 0);
  iupAttribSetInt(gl_parent, "VIEWPORT_Y", 0);
  iupAttribSetInt(gl_parent, "VIEWPORT_WIDTH", gl_parent->currentwidth);
  iupAttribSetInt(gl_parent, "VIEWPORT_HEIGHT", gl_parent->currentheight);
}

void iupGLSubCanvasRestoreState(Ihandle* gl_parent)
{
  char* saved = iupAttribGet(gl_parent, "_IUP_GLSUBCANVAS_SAVED");
  iupASSERT(saved != NULL);
  if (!saved)
    return;

  /* restore transformation matrix */
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  /* restore transformation matrix */
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  /* restore everything else */
  glPopAttrib();

  iupAttribSet(gl_parent, "_IUP_GLSUBCANVAS_SAVED", NULL);
}

void iupGLSubCanvasRedrawFront(Ihandle* ih)
{
  IFn cb = (IFn)IupGetCallback(ih, "GL_ACTION");
  if (cb && iupAttribGetInt(ih, "VISIBLE"))
  {
    Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
    IupGLMakeCurrent(gl_parent);
    glDrawBuffer(GL_FRONT);
    iupGLSubCanvasSaveState(gl_parent);
    if (iupGLSubCanvasSetTransform(ih, gl_parent))
      cb(ih);
    iupGLSubCanvasRestoreState(gl_parent);
    glDrawBuffer(GL_BACK);
  }
}

int iupGLSubCanvasRedraw(Ihandle* ih)
{
  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  if (iupAttribGetInt(ih, "REDRAWALL"))
    IupSetAttribute(gl_parent, "REDRAW", NULL);
  else
    iupGLSubCanvasRedrawFront(ih);
  return IUP_DEFAULT;  /* return IUP_DEFAULT so it can be used as a callback */
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
  iupClassRegisterCallback(ic, "GL_ENTERWINDOW_CB", "ii");
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
  iupClassRegisterAttribute(ic, "TIP", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TIPVISIBLE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  /* Common visual */
  iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERWIDTH", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, NULL, NULL, NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 225 245", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "PSCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "150 200 235", IUPAF_DEFAULT);  /* inheritable */

  iupClassRegisterAttribute(ic, "UNDERLINE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REDRAWFRONT", NULL, iGLSubCanvasSetRedrawFrontAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REDRAWALL", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "HIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PRESSED", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupGLFontInit();

  return ic;
}

Ihandle* IupGLSubCanvas(void)
{
  return IupCreate("glsubcanvas");
}

