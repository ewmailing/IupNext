#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupgl.h"
#include "cd.h"
#include "cdiup.h"
#include "cdgl.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#define CANVAS_SIZE "1100x600"

static void fill_charac(char *charac)
{
  int c, l = 0, off = 0;
  for (c=32; c<=255; c++)
  {
    off = (c-32)*3 + l;
    charac[off+0] = ' ';
    if (c==127 || c==129 || c==141 ||
        c==143 || c==144 || c==152 || c==157)
      charac[off+1] = ' ';
    else
      charac[off+1] = (char)c;
    charac[off+2] = ' ';

    if ((c-32 + 1)%16==0)
    {
      charac[off+3] = '\n';
      l++;
    }
  }

  charac[off+3] = 0;
//  charac[off+4] = 0;
}

void UpdateCoordSys(void)
{
  int w, h;
  sscanf(CANVAS_SIZE, "%dx%d", &w, &h);

  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, (GLdouble)w, 0, (GLdouble)h);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

static int redraw_cb(Ihandle *ih)
{
  char charac[10240];
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

  fill_charac(charac);

  if (IupGetAttribute(ih, "_APP_CDCANVAS_GL"))
  {
    IupGLMakeCurrent(ih);
    UpdateCoordSys();
  }

  cdCanvasActivate(cdcanvas);
  cdCanvasClear(cdcanvas);
  if (IupGetAttribute(ih, "_APP_CDCANVAS_VECTOR"))
    cdCanvasVectorText(cdcanvas, 0, 0, charac);
  else
    cdCanvasText(cdcanvas, 0, 0, charac);
  cdCanvasFlush(cdcanvas);

  if (IupGetAttribute(ih, "_APP_CDCANVAS_GL"))
    IupGLSwapBuffers(ih);

  return IUP_DEFAULT;
}

static int map_cb(Ihandle *ih)
{
  cdCanvas *cdcanvas;
  int size_px;

  cdcanvas = cdCreateCanvas(CD_IUP, ih);
  if (!cdcanvas)
    return IUP_DEFAULT;

  IupSetAttribute(ih, "_APP_CDCANVAS", (char*)cdcanvas);

  cdCanvasBackground(cdcanvas, CD_WHITE);
  cdCanvasForeground(cdcanvas, CD_BLACK);
  cdCanvasFont(cdcanvas, "Courier", CD_PLAIN, 14);
  cdCanvasTextAlignment(cdcanvas, CD_SOUTH_WEST);

  cdCanvasMM2Pixel(cdcanvas, 0, (14 / CD_MM2PT), NULL, &size_px);
  cdCanvasVectorCharSize(cdcanvas, size_px);

  return IUP_DEFAULT;
}

static int gl_map_cb(Ihandle *ih)
{
  cdCanvas *cdcanvas;
  double res;

  IupGLMakeCurrent(ih);
  res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
  cdcanvas = cdCreateCanvasf(CD_GL, "%s %g", CANVAS_SIZE, res);
  if (!cdcanvas)
    return IUP_DEFAULT;

  IupSetAttribute(ih, "_APP_CDCANVAS", (char*)cdcanvas);
  IupSetAttribute(ih, "_APP_CDCANVAS_GL", "1");

  cdCanvasBackground(cdcanvas, CD_WHITE);
  cdCanvasForeground(cdcanvas, CD_BLACK);
  cdCanvasFont(cdcanvas, "Courier", CD_PLAIN, 14);
  cdCanvasTextAlignment(cdcanvas, CD_SOUTH_WEST);

  return IUP_DEFAULT;
}

static int gl_resize_cb(Ihandle* ih)
{
  int canvas_width, canvas_height;
  double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

  IupGetIntInt(ih, "DRAWSIZE", &canvas_width, &canvas_height);

  IupGLMakeCurrent(ih);
  cdCanvasSetfAttribute(cdcanvas, "SIZE", "%dx%d %g", canvas_width, canvas_height, res);

  return IUP_DEFAULT;
}

static int unmap_cb(Ihandle *ih)
{
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

  if (cdcanvas)
    cdKillCanvas(cdcanvas);

  return IUP_DEFAULT;
}

static int list_cb (Ihandle *h, char *t, int o, int selected)
{
  if (selected == 1)
  {
    Ihandle* zbox = IupGetHandle ("zbox");
    IupSetAttribute (zbox, "VALUE", t);
  }
  
  return IUP_DEFAULT;
}

void CharacTest(void)
{
  char charac[10240];
  Ihandle *dlg, *frm, *zbox, *label, *cd_canvas, *cd_canvas_gl, *cd_canvas_vector, *list;

  IupGLCanvasOpen();

  fill_charac(charac);
  
  label = IupLabel(NULL);
  cd_canvas = IupCanvas(NULL);
  cd_canvas_vector = IupCanvas(NULL);
  cd_canvas_gl = IupGLCanvas(NULL);

  IupSetAttribute(label, "EXPAND", "YES");
  IupStoreAttribute(label, "TITLE", charac);
  IupSetAttribute(label, "RASTERSIZE", CANVAS_SIZE);
  IupSetAttribute(label, "ALIGNMENT", "ALEFT:ABOTTOM");
  label = IupBackgroundBox(label);
  IupStoreAttribute(label, "BGCOLOR", "255 255 255");
  IupStoreAttribute(label, "BORDER", "Yes");

  IupSetAttribute(cd_canvas, "RASTERSIZE", CANVAS_SIZE);
  IupSetCallback(cd_canvas, "ACTION",  (Icallback)redraw_cb);
  IupSetCallback(cd_canvas, "MAP_CB",  (Icallback)map_cb);
  IupSetCallback(cd_canvas, "UNMAP_CB",  (Icallback)unmap_cb);

  IupSetAttribute(cd_canvas_vector, "RASTERSIZE", CANVAS_SIZE);
  IupSetCallback(cd_canvas_vector, "ACTION", (Icallback)redraw_cb);
  IupSetCallback(cd_canvas_vector, "MAP_CB", (Icallback)map_cb);
  IupSetCallback(cd_canvas_vector, "UNMAP_CB", (Icallback)unmap_cb);
  IupSetAttribute(cd_canvas_vector, "_APP_CDCANVAS_VECTOR", "1");

  IupSetAttribute(cd_canvas_gl, "RASTERSIZE", CANVAS_SIZE);
  IupSetAttribute(cd_canvas_gl, "BUFFER", "DOUBLE");
  IupSetCallback(cd_canvas_gl, "ACTION",  (Icallback)redraw_cb);
  IupSetCallback(cd_canvas_gl, "MAP_CB",  (Icallback)gl_map_cb);
  IupSetCallback(cd_canvas_gl, "UNMAP_CB",  (Icallback)unmap_cb);
  IupSetCallback(cd_canvas_gl, "RESIZE_CB", (Icallback)gl_resize_cb);

  zbox = IupZbox(label, cd_canvas, cd_canvas_gl, cd_canvas_vector, NULL);

  IupSetHandle("zbox", zbox);
  IupSetHandle("label", label);
  IupSetHandle("cd_canvas", cd_canvas);
  IupSetHandle("cd_canvas_gl", cd_canvas_gl);
  IupSetHandle("cd_canvas_vector", cd_canvas_vector);

  IupSetAttribute (zbox, "ALIGNMENT", "ACENTER");
  IupSetAttribute (zbox, "VALUE", "label");

  frm = IupFrame(
    IupHbox
    (
      list = IupList(NULL),
      NULL
    )
  ),

  dlg = IupDialog
  (
    IupVbox
    (
      frm,
      zbox,
      NULL
    )
  );

  IupSetAttributes (list, "1=label, 2=cd_canvas, 3=cd_canvas_gl, 4=cd_canvas_vector, VALUE=1");
  IupSetCallback (list, "ACTION", (Icallback) list_cb);

  IupSetAttribute (frm, "TITLE", "Select:");
  IupSetAttributes (dlg, "MARGIN=10x10, GAP=10, TITLE = \"Character Example\"");
  IupSetAttribute(dlg, "FONT", "Courier, 14");

  IupShowXY (dlg, IUP_CENTER, IUP_CENTER );
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);
  IupGLCanvasOpen();

  CharacTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
