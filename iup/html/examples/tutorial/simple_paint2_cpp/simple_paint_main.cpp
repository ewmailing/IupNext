
#include "simple_paint.h"
#include "simple_paint_util.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <im_process.h>
#include <cdirgb.h>
#include <cdiup.h>
#include <cdprint.h>
#include <iup_config.h>
#include <iupim.h>

//#define USE_OPENGL 1
//#define USE_CONTEXTPLUS 1

#ifdef USE_OPENGL
#include <iupgl.h>
#include <cdgl.h>
#endif

#if _MSC_VER < 1800 /* vc12 (2013) */
#define DEFINE_ROUND
#endif

#ifdef DEFINE_ROUND
static double round(double x)
{
  return (int)(x>0 ? x + 0.5 : x - 0.5);
}
#endif


/********************************** Images *****************************************/


static Ihandle* load_image_PaintZoomGrid(void)
{
  unsigned char imgdata[] = {
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 110, 155, 223, 255, 255, 255, 255, 0, 255, 255, 255, 0, 106, 151, 219, 255, 255, 255, 255, 0, 255, 255, 255, 0, 95, 142, 210, 255, 255, 255, 255, 0, 255, 255, 255, 0, 84, 129, 201, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 110, 155, 223, 255, 255, 255, 255, 0, 255, 255, 255, 0, 102, 147, 217, 255, 255, 255, 255, 0, 255, 255, 255, 0, 92, 137, 207, 255, 255, 255, 255, 0, 255, 255, 255, 0, 80, 125, 197, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 110, 155, 223, 255, 110, 155, 223, 255, 107, 152, 222, 255, 104, 151, 219, 255, 103, 148, 216, 255, 100, 145, 213, 255, 97, 142, 210, 255, 93, 138, 206, 255, 88, 133, 203, 255, 84, 129, 201, 255, 80, 125, 197, 255, 76, 121, 193, 255, 72, 117, 189, 255, 68, 113, 183, 255, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 106, 149, 219, 255, 255, 255, 255, 0, 255, 255, 255, 0, 97, 142, 210, 255, 255, 255, 255, 0, 255, 255, 255, 0, 84, 131, 199, 255, 255, 255, 255, 0, 255, 255, 255, 0, 72, 117, 187, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 103, 148, 216, 255, 255, 255, 255, 0, 255, 255, 255, 0, 92, 137, 207, 255, 255, 255, 255, 0, 255, 255, 255, 0, 81, 125, 196, 255, 255, 255, 255, 0, 255, 255, 255, 0, 68, 113, 185, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 104, 151, 219, 255, 104, 148, 216, 255, 100, 145, 213, 255, 97, 142, 210, 255, 92, 137, 207, 255, 88, 135, 203, 255, 84, 129, 199, 255, 80, 125, 195, 255, 76, 121, 193, 255, 70, 117, 189, 255, 68, 112, 183, 255, 62, 109, 181, 255, 60, 105, 177, 255, 57, 102, 174, 255, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 97, 142, 210, 255, 255, 255, 255, 0, 255, 255, 255, 0, 84, 129, 199, 255, 255, 255, 255, 0, 255, 255, 255, 0, 72, 117, 187, 255, 255, 255, 255, 0, 255, 255, 255, 0, 60, 105, 177, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 93, 138, 206, 255, 255, 255, 255, 0, 255, 255, 255, 0, 80, 125, 195, 255, 255, 255, 255, 0, 255, 255, 255, 0, 66, 113, 185, 255, 255, 255, 255, 0, 255, 255, 255, 0, 56, 103, 173, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 95, 140, 212, 255, 92, 137, 207, 255, 88, 133, 203, 255, 84, 129, 199, 255, 80, 125, 195, 255, 76, 121, 193, 255, 72, 117, 187, 255, 68, 113, 185, 255, 64, 107, 181, 255, 59, 105, 178, 255, 55, 101, 174, 255, 53, 98, 170, 255, 49, 95, 168, 255, 48, 91, 165, 255, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 84, 131, 199, 255, 255, 255, 255, 0, 255, 255, 255, 0, 70, 117, 189, 255, 255, 255, 255, 0, 255, 255, 255, 0, 60, 105, 177, 255, 255, 255, 255, 0, 255, 255, 255, 0, 49, 95, 168, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 82, 125, 195, 255, 255, 255, 255, 0, 255, 255, 255, 0, 68, 113, 185, 255, 255, 255, 255, 0, 255, 255, 255, 0, 57, 102, 174, 255, 255, 255, 255, 0, 255, 255, 255, 0, 46, 92, 165, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 84, 129, 199, 255, 80, 125, 195, 255, 76, 121, 191, 255, 72, 117, 189, 255, 66, 113, 185, 255, 64, 109, 181, 255, 60, 105, 177, 255, 55, 102, 174, 255, 52, 98, 171, 255, 49, 95, 168, 255, 46, 91, 166, 255, 44, 90, 163, 255, 42, 88, 161, 255, 42, 88, 161, 255, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 72, 117, 187, 255, 255, 255, 255, 0, 255, 255, 255, 0, 60, 107, 177, 255, 255, 255, 255, 0, 255, 255, 255, 0, 50, 95, 167, 255, 255, 255, 255, 0, 255, 255, 255, 0, 42, 88, 161, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 68, 113, 185, 255, 255, 255, 255, 0, 255, 255, 255, 0, 56, 101, 173, 255, 255, 255, 255, 0, 255, 255, 255, 0, 46, 92, 165, 255, 255, 255, 255, 0, 255, 255, 255, 0, 42, 88, 161, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0,
    255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0 };

  Ihandle* image = IupImageRGBA(16, 16, imgdata);
  return image;
}


/*********************************** Utilities Methods **************************************/


void SimplePaint::UpdateZoom(double zoom_index)
{
  Ihandle* zoom_lbl = IupGetDialogChild(dlg, "ZOOMLABEL");
  double zoom_factor = pow(2, zoom_index);
  IupSetStrf(zoom_lbl, "TITLE", "%.0f%%", floor(zoom_factor * 100));
  if (file.GetImage())
  {
    float old_center_x, old_center_y;
    int view_width = (int)(zoom_factor * file.GetImage()->width);
    int view_height = (int)(zoom_factor * file.GetImage()->height);

    scroll_calc_center(canvas, &old_center_x, &old_center_y);

    scroll_update(canvas, view_width, view_height);

    scroll_center(canvas, old_center_x, old_center_y);
  }
  IupUpdate(canvas);
}

void SimplePaint::UpdateImage(imImage* new_image, bool update_size)
{
  file.SetImage(new_image);

  if (update_size)
  {
    Ihandle* size_lbl = IupGetDialogChild(dlg, "SIZELABEL");
    Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
    double zoom_index = IupGetDouble(zoom_val, "VALUE");
    IupSetfAttribute(size_lbl, "TITLE", "%d x %d px", file.GetImage()->width, file.GetImage()->height);
    UpdateZoom(zoom_index);
  }
  else
    IupUpdate(canvas);
}

void SimplePaint::UpdateFile()
{
  Ihandle* size_lbl = IupGetDialogChild(dlg, "SIZELABEL");
  Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");

  if (file.GetFilename())
    IupSetfAttribute(dlg, "TITLE", "%s - Simple Paint", str_filetitle(file.GetFilename()));
  else
    IupSetAttribute(dlg, "TITLE", "Untitled - Simple Paint");

  IupSetfAttribute(size_lbl, "TITLE", "%d x %d px", file.GetImage()->width, file.GetImage()->height);

  IupSetDouble(zoom_val, "VALUE", 0);
  UpdateZoom(0);
}

void SimplePaint::CheckNewFile()
{
  if (!file.GetImage())
  {
    int width = IupConfigGetVariableIntDef(config, "NewImage", "Width", 640);
    int height = IupConfigGetVariableIntDef(config, "NewImage", "Height", 480);

    if (file.New(width, height))
      UpdateFile();
  }
}

void SimplePaint::OpenFile(const char* filename)
{
  if (file.Open(filename))
  {
    UpdateFile();

    IupConfigRecentUpdate(config, filename);
  }
}

void SimplePaint::ToggleBarVisibility(Ihandle* item, Ihandle* bar)
{
  if (IupGetInt(item, "VALUE"))
  {
    IupSetAttribute(bar, "FLOATING", "YES");
    IupSetAttribute(bar, "VISIBLE", "NO");
    IupSetAttribute(item, "VALUE", "OFF");
  }
  else
  {
    IupSetAttribute(bar, "FLOATING", "NO");
    IupSetAttribute(bar, "VISIBLE", "YES");
    IupSetAttribute(item, "VALUE", "ON");
  }

  IupRefresh(bar);  /* refresh the dialog layout */
}

void SimplePaint::SelectFile(bool is_open)
{
  const char* dir = IupConfigGetVariableStr(config, "MainWindow", "LastDirectory");

  Ihandle* filedlg = IupFileDlg();
  if (is_open)
    IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  else
  {
    IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
    IupSetStrAttribute(filedlg, "FILE", file.GetFilename());
  }
  IupSetAttribute(filedlg, "EXTFILTER", "Image Files|*.bmp;*.jpg;*.png;*.tif;*.tga|All Files|*.*|");
  IupSetStrAttribute(filedlg, "DIRECTORY", dir);
  IupSetAttributeHandle(filedlg, "PARENTDIALOG", dlg);

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    if (is_open)
      OpenFile(filename);
    else
    {
      if (file.SaveAsFile(filename))
      {
        IupSetfAttribute(dlg, "TITLE", "%s - Simple Paint", str_filetitle(filename));
        IupConfigRecentUpdate(config, filename);
      }
    }

    dir = IupGetAttribute(filedlg, "DIRECTORY");
    IupConfigSetVariableStr(config, "MainWindow", "LastDirectory", dir);
  }

  IupDestroy(filedlg);
}

double SimplePaint::ViewZoomRect(int *_x, int *_y, int *_view_width, int *_view_height)
{
  int x, y, canvas_width, canvas_height;
  int view_width, view_height;
  Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
  double zoom_index = IupGetDouble(zoom_val, "VALUE");
  double zoom_factor = pow(2, zoom_index);

  float posy = IupGetFloat(canvas, "POSY");
  float posx = IupGetFloat(canvas, "POSX");

  IupGetIntInt(canvas, "DRAWSIZE", &canvas_width, &canvas_height);

  view_width = (int)(zoom_factor * file.GetImage()->width);
  view_height = (int)(zoom_factor * file.GetImage()->height);

  if (canvas_width < view_width)
    x = (int)floor(-posx*view_width);
  else
    x = (canvas_width - view_width) / 2;

  if (canvas_height < view_height)
  {
    /* posy is top-bottom, CD is bottom-top.
    invert posy reference (YMAX-DY - POSY) */
    float dy = IupGetFloat(canvas, "DY");
    posy = 1.0f - dy - posy;
    y = (int)floor(-posy*view_height);
  }
  else
    y = (canvas_height - view_height) / 2;

  *_x = x;
  *_y = y;
  *_view_width = view_width;
  *_view_height = view_height;

  return zoom_factor;
}

void SimplePaint::DrawPencil(int start_x, int start_y, int end_x, int end_y)
{
  double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
  unsigned char** data = (unsigned char**)file.GetImage()->data;

  int line_width = toolbox.LineWidth();
  long color = toolbox.Color();

  /* do not use line style here */
  cdCanvas* rgb_canvas = cdCreateCanvasf(CD_IMAGERGB, "%dx%d %p %p %p -r%g", file.GetImage()->width, file.GetImage()->height, data[0], data[1], data[2], res);
  cdCanvasForeground(rgb_canvas, color);
  cdCanvasLineWidth(rgb_canvas, line_width);
  cdCanvasLine(rgb_canvas, start_x, start_y, end_x, end_y);
  cdKillCanvas(rgb_canvas);
}

void SimplePaint::DrawToolOverlay(cdCanvas* cnv, int start_x, int start_y, int end_x, int end_y)
{
  SimplePaintToolbox::Tool tool_index = toolbox.ToolIndex();
  int line_width = toolbox.LineWidth();
  int line_style = toolbox.LineStyle();
  long color = toolbox.Color();

  cdCanvasForeground(cnv, color);
  cdCanvasLineWidth(cnv, line_width);
  if (line_width == 1)
    cdCanvasLineStyle(cnv, line_style);

  if (tool_index == SimplePaintToolbox::TOOL_LINE)
    cdCanvasLine(cnv, start_x, start_y, end_x, end_y);
  else if (tool_index == SimplePaintToolbox::TOOL_RECT)
    cdCanvasRect(cnv, start_x, end_x, start_y, end_y);
  else if (tool_index == SimplePaintToolbox::TOOL_BOX)
    cdCanvasBox(cnv, start_x, end_x, start_y, end_y);
  else if (tool_index == SimplePaintToolbox::TOOL_ELLIPSE)
    cdCanvasArc(cnv, (end_x + start_x) / 2, (end_y + start_y) / 2, abs(end_x - start_x), abs(end_y - start_y), 0, 360);
  else if (tool_index == SimplePaintToolbox::TOOL_OVAL)
    cdCanvasSector(cnv, (end_x + start_x) / 2, (end_y + start_y) / 2, abs(end_x - start_x), abs(end_y - start_y), 0, 360);
  else if (tool_index == SimplePaintToolbox::TOOL_TEXT)
  {
    cdCanvasTextAlignment(cnv, CD_SOUTH_WEST);
    cdCanvasNativeFont(cnv, toolbox.Font());
    cdCanvasText(cnv, end_x, end_y, toolbox.Text());
  }
}


/********************************** Callbacks Methods *****************************************/


int SimplePaint::CanvasActionCallback(Ihandle*)
{
  unsigned int ri, gi, bi;
  const char* background = IupConfigGetVariableStrDef(config, "MainWindow", "Background", "208 208 208");

#ifdef USE_OPENGL
  IupGLMakeCurrent(canvas);
#endif
  cdCanvasActivate(cd_canvas);

  /* draw the background */
  sscanf(background, "%u %u %u", &ri, &gi, &bi);
  cdCanvasBackground(cd_canvas, cdEncodeColor((unsigned char)ri, (unsigned char)gi, (unsigned char)bi));
  cdCanvasClear(cd_canvas);

  /* draw the image at the center of the canvas */
  if (file.GetImage())
  {
    int x, y, view_width, view_height;
    ViewZoomRect(&x, &y, &view_width, &view_height);

    /* black line around the image */
    cdCanvasForeground(cd_canvas, CD_BLACK);
    cdCanvasLineWidth(cd_canvas, 1);
    cdCanvasLineStyle(cd_canvas, CD_CONTINUOUS);
    cdCanvasRect(cd_canvas, x - 1, x + view_width, y - 1, y + view_height);

    /* Some CD drivers have interpolation options for image zoom */
    /* we force NEAREST so we can see the pixel boundary in zoom in */
    /* an alternative would be to set BILINEAR when zoom out */
    cdCanvasSetAttribute(cd_canvas, "IMGINTERP", (char*)"NEAREST");  /* affects only drivers that have this attribute */
    imcdCanvasPutImage(cd_canvas, file.GetImage(), x, y, view_width, view_height, 0, 0, 0, 0);

    if (IupConfigGetVariableInt(config, "MainWindow", "ZoomGrid"))
    {
      Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
      double zoom_index = IupGetDouble(zoom_val, "VALUE");
      if (zoom_index > 1)
      {
        int ix, iy;
        double zoom_factor = pow(2, zoom_index);

        cdCanvasForeground(cd_canvas, CD_GRAY);

        for (ix = 0; ix < file.GetImage()->width; ix++)
        {
          int gx = (int)(ix * zoom_factor);
          cdCanvasLine(cd_canvas, gx + x, y, gx + x, y + view_height);
        }
        for (iy = 0; iy < file.GetImage()->height; iy++)
        {
          int gy = (int)(iy * zoom_factor);
          cdCanvasLine(cd_canvas, x, gy + y, x + view_width, gy + y);
        }
      }
    }

    if (interact.overlay)
    {
      double scale_x = (double)view_width / (double)file.GetImage()->width;
      double scale_y = (double)view_height / (double)file.GetImage()->height;

      /* offset and scale drawing in screen to match the image */
      if (scale_x > 1 || scale_y > 1)
        cdCanvasTransformTranslate(cd_canvas, x + scale_x / 2, y + scale_y / 2);  /* also draw at the center of the pixel when zoom in */
      else
        cdCanvasTransformTranslate(cd_canvas, x, y);
      cdCanvasTransformScale(cd_canvas, scale_x, scale_y);

      DrawToolOverlay(cd_canvas, interact.start_x, interact.start_y, interact.end_x, interact.end_y);

      cdCanvasTransform(cd_canvas, NULL);
    }
  }

  cdCanvasFlush(cd_canvas);

#ifdef USE_OPENGL
  IupGLSwapBuffers(canvas);
#endif
  return IUP_DEFAULT;
}

int SimplePaint::CanvasMapCallback(Ihandle* canvas)
{
#ifdef USE_OPENGL
  double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
  IupGLMakeCurrent(canvas);
  cd_canvas = cdCreateCanvasf(CD_GL, "10x10 %g", res);
#else
#ifdef USE_CONTEXTPLUS
  cdUseContextPlus(1);
#endif
  cd_canvas = cdCreateCanvas(CD_IUPDBUFFER, canvas);
#ifdef USE_CONTEXTPLUS
  cdUseContextPlus(0);
#endif
#endif
  return IUP_DEFAULT;
}

int SimplePaint::CanvasUnmapCallback(Ihandle*)
{
  cdKillCanvas(cd_canvas);
  return IUP_DEFAULT;
}

int SimplePaint::CanvasResizeCallback(Ihandle* canvas)
{
  if (file.GetImage())
  {
    Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
    double zoom_index = IupGetDouble(zoom_val, "VALUE");
    double zoom_factor = pow(2, zoom_index);
    float old_center_x, old_center_y;

    int view_width = (int)(zoom_factor * file.GetImage()->width);
    int view_height = (int)(zoom_factor * file.GetImage()->height);

    scroll_calc_center(canvas, &old_center_x, &old_center_y);

    scroll_update(canvas, view_width, view_height);

    scroll_center(canvas, old_center_x, old_center_y);
  }

#ifdef USE_OPENGL
  {
    int canvas_width, canvas_height;
    double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
    IupGetIntInt(canvas, "DRAWSIZE", &canvas_width, &canvas_height);

    IupGLMakeCurrent(canvas);
    cdCanvasSetfAttribute(cd_canvas, "SIZE", "%dx%d %g", canvas_width, canvas_height, res);
  }
#endif
  return IUP_DEFAULT;
}

int SimplePaint::CanvasWheelCallback(Ihandle* canvas, float delta, int, int, char*)
{
  if (IupGetInt(NULL, "CONTROLKEY"))
  {
    if (delta < 0)
      ItemZoomoutActionCallback(canvas);
    else
      ItemZoominActionCallback(canvas);
  }
  else
  {
    float posy = IupGetFloat(canvas, "POSY");
    posy -= delta * IupGetFloat(canvas, "DY") / 10.0f;
    IupSetFloat(canvas, "POSY", posy);
    IupUpdate(canvas);
  }
  return IUP_DEFAULT;
}

int SimplePaint::CanvasButtonCallback(Ihandle* canvas, int button, int pressed, int x, int y)
{
  if (file.GetImage())
  {
    int cursor_x = x, cursor_y = y;
    int view_x, view_y, view_width, view_height;
    double zoom_factor = ViewZoomRect(&view_x, &view_y, &view_width, &view_height);

    /* y is top-down in IUP */
    int canvas_height = IupGetInt2(canvas, "DRAWSIZE");
    y = canvas_height - y - 1;

    /* inside image area */
    if (x > view_x && y > view_y && x < view_x + view_width && y < view_y + view_height)
    {
      view_zoom_offset(view_x, view_y, file.GetImage()->width, file.GetImage()->height, zoom_factor, &x, &y);

      if (button == IUP_BUTTON1)
      {
        if (pressed)
        {
          interact.start_x = x;
          interact.start_y = y;
          interact.start_cursor_x = cursor_x;
          interact.start_cursor_y = cursor_y;
        }
        else
        {
          SimplePaintToolbox::Tool tool_index = toolbox.ToolIndex();

          if (tool_index == SimplePaintToolbox::TOOL_COLORPICKER)
          {
            unsigned char** data = (unsigned char**)file.GetImage()->data;
            unsigned char r, g, b;
            int offset;

            offset = y * file.GetImage()->width + x;
            r = data[0][offset];
            g = data[1][offset];
            b = data[2][offset];

            toolbox.SetColor(cdEncodeColor(r, g, b));
          }
          else if (tool_index == SimplePaintToolbox::TOOL_PENCIL)
          {
            DrawPencil(interact.start_x, interact.start_y, x, y);

            file.dirty = true;

            IupUpdate(canvas);

            interact.start_x = x;
            interact.start_y = y;
          }
          else if (tool_index >= SimplePaintToolbox::TOOL_LINE && tool_index <= SimplePaintToolbox::TOOL_TEXT)  /* All Shapes */
          {
            if (interact.overlay)
            {
              double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
              unsigned char** data = (unsigned char**)file.GetImage()->data;
  
              cdCanvas* rgb_canvas = cdCreateCanvasf(CD_IMAGERGB, "%dx%d %p %p %p -r%g", file.GetImage()->width, file.GetImage()->height, data[0], data[1], data[2], res);

              DrawToolOverlay(rgb_canvas, interact.start_x, interact.start_y, x, y);

              cdKillCanvas(rgb_canvas);

              interact.overlay = false;
              file.dirty = true;

              IupUpdate(canvas);
            }
          }
          else if (tool_index == SimplePaintToolbox::TOOL_FILLCOLOR)
          {
            double tol_percent = toolbox.FillTol();
            long color = toolbox.Color();

            image_flood_fill(file.GetImage(), x, y, color, tol_percent);
            file.dirty = true;

            IupUpdate(canvas);
          }
        }
      }
      else if (button == IUP_BUTTON3)
      {
        if (!pressed)
        {
          SimplePaintToolbox::Tool tool_index = toolbox.ToolIndex();
          if (tool_index == SimplePaintToolbox::TOOL_TEXT)
            toolbox.ToolGetText();
        }
      }
    }
  }

  return IUP_DEFAULT;
}

int SimplePaint::CanvasMotionCallback(Ihandle* canvas, int x, int y, char *status)
{
  SimplePaintToolbox::Tool tool_index = toolbox.ToolIndex();
  if (tool_index == SimplePaintToolbox::TOOL_POINTER)
  {
    char* cursor = IupGetAttribute(canvas, "CURSOR");
    if (strcmp(cursor, "ARROW") != 0)
      IupSetAttribute(canvas, "CURSOR", "ARROW");
  }
  else
  {
    char* cursor = IupGetAttribute(canvas, "CURSOR");
    if (strcmp(cursor, "CROSS") != 0)
      IupSetAttribute(canvas, "CURSOR", "CROSS");
  }

  if (file.GetImage())
  {
    int cursor_x = x, cursor_y = y;
    int view_x, view_y, view_width, view_height;
    double zoom_factor = ViewZoomRect(&view_x, &view_y, &view_width, &view_height);

    /* y is top-down in IUP */
    int canvas_height = IupGetInt2(canvas, "DRAWSIZE");
    y = canvas_height - y - 1;

    /* inside image area */
    if (x > view_x && y > view_y && x < view_x + view_width && y < view_y + view_height)
    {
      Ihandle* status_lbl = IupGetDialogChild(dlg, "STATUSLABEL");
      unsigned char** data = (unsigned char**)file.GetImage()->data;
      unsigned char r, g, b;
      int offset;

      view_zoom_offset(view_x, view_y, file.GetImage()->width, file.GetImage()->height, zoom_factor, &x, &y);

      offset = y * file.GetImage()->width + x; 
      r = data[0][offset];
      g = data[1][offset];
      b = data[2][offset];

      IupSetStrf(status_lbl, "TITLE", "(%4d, %4d) = %3d %3d %3d", x, y, (int)r, (int)g, (int)b);

      if (iup_isbutton1(status)) /* button1 is pressed */
      {
        if (tool_index == SimplePaintToolbox::TOOL_POINTER)
        {
          int canvas_width = IupGetInt(canvas, "DRAWSIZE");

          scroll_move(canvas, canvas_width, canvas_height, cursor_x - interact.start_cursor_x, cursor_y - interact.start_cursor_y, view_width, view_height);

          interact.start_cursor_x = cursor_x;
          interact.start_cursor_y = cursor_y;
        }
        else if (tool_index == SimplePaintToolbox::TOOL_PENCIL)
        {
          DrawPencil(interact.start_x, interact.start_y, x, y);

          file.dirty = true;

          IupUpdate(canvas);

          interact.start_x = x;
          interact.start_y = y;
        }
        else if (tool_index >= SimplePaintToolbox::TOOL_LINE && tool_index <= SimplePaintToolbox::TOOL_TEXT)  /* All Shapes */
        {
          interact.end_x = x;
          interact.end_y = y;
          interact.overlay = true;
          IupUpdate(canvas);
        }
      }
    }
  }

  return IUP_DEFAULT;
}

int SimplePaint::ZoomValueChangedCallback(Ihandle* val)
{
  double zoom_index = IupGetDouble(val, "VALUE");
  UpdateZoom(zoom_index);
  return IUP_DEFAULT;
}

int SimplePaint::DropfilesCallback(Ihandle*, char* filename)
{
  if (file.SaveCheck())
    OpenFile(filename);

  return IUP_DEFAULT;
}

int SimplePaint::FileMenuOpenCallback(Ihandle*)
{
  Ihandle* item_save = IupGetDialogChild(dlg, "ITEM_SAVE");
  if (file.dirty)
    IupSetAttribute(item_save, "ACTIVE", "YES");
  else
    IupSetAttribute(item_save, "ACTIVE", "NO");

  Ihandle* item_revert = IupGetDialogChild(dlg, "ITEM_REVERT");
  if (file.dirty && file.GetFilename())
    IupSetAttribute(item_revert, "ACTIVE", "YES");
  else
    IupSetAttribute(item_revert, "ACTIVE", "NO");
  return IUP_DEFAULT;
}

int SimplePaint::EditMenuOpenCallback(Ihandle*)
{
  Ihandle *clipboard = IupClipboard(); 

  Ihandle *item_paste = IupGetDialogChild(dlg, "ITEM_PASTE");

  if (!IupGetInt(clipboard, "IMAGEAVAILABLE"))
    IupSetAttribute(item_paste, "ACTIVE", "NO");
  else
    IupSetAttribute(item_paste, "ACTIVE", "YES");

  IupDestroy(clipboard);
  return IUP_DEFAULT;
}

int SimplePaint::ConfigRecentCallback(Ihandle* ih)
{
  if (file.SaveCheck())
  {
    char* filename = IupGetAttribute(ih, "TITLE");
    OpenFile(filename);
  }
  return IUP_DEFAULT;
}

int SimplePaint::DialogMoveCallback(Ihandle* dlg, int x, int y)
{
  int old_x = IupGetInt(dlg, "_OLD_X");
  int old_y = IupGetInt(dlg, "_OLD_Y");

  if (old_x == x && old_y == y)
    return IUP_DEFAULT;

  toolbox.MoveDialog(x - old_x, y - old_y);

  IupSetInt(dlg, "_OLD_X", x);
  IupSetInt(dlg, "_OLD_Y", y);

  return IUP_DEFAULT;
}


/********************************* SimplePaint Menu Item Callbacks Methods **************************/


int SimplePaint::ItemNewActionCallback(Ihandle*)
{
  if (file.SaveCheck())
  {
    int width = IupConfigGetVariableIntDef(config, "NewImage", "Width", 640);
    int height = IupConfigGetVariableIntDef(config, "NewImage", "Height", 480);

    if (IupGetParam("New Image", NULL, NULL, "Width: %i[1,]\nHeight: %i[1,]\n", &width, &height, NULL))
    {
      IupConfigSetVariableInt(config, "NewImage", "Width", width);
      IupConfigSetVariableInt(config, "NewImage", "Height", height);

      file.New(width, height);

      UpdateFile();
    }
  }

  return IUP_DEFAULT;
}

int SimplePaint::ItemOpenActionCallback(Ihandle*)
{
  if (!file.SaveCheck())
    return IUP_DEFAULT;

  SelectFile(true);
  return IUP_DEFAULT;
}

int SimplePaint::ItemSaveasActionCallback(Ihandle*)
{
  SelectFile(false);
  return IUP_DEFAULT;
}

int SimplePaint::ItemSaveActionCallback(Ihandle* item_save)
{
  if (!file.GetFilename())
    ItemSaveasActionCallback(item_save);
  else   
  {
    /* test again because in can be called using the hot key */
    if (file.dirty)
      file.SaveFile();
  }
  return IUP_DEFAULT;
}

int SimplePaint::ItemRevertActionCallback(Ihandle*)
{
  OpenFile(file.GetFilename());
  return IUP_DEFAULT;
}

int SimplePaint::ItemPagesetupActionCallback(Ihandle*)
{
  int margin_width = IupConfigGetVariableIntDef(config, "Print", "MarginWidth", 20);
  int margin_height = IupConfigGetVariableIntDef(config, "Print", "MarginHeight", 20);

  if (IupGetParam("Page Setup", NULL, NULL, "Margin Width (mm): %i[1,]\nMargin Height (mm): %i[1,]\n", &margin_width, &margin_height, NULL))
  {
    IupConfigSetVariableInt(config, "Print", "MarginWidth", margin_width);
    IupConfigSetVariableInt(config, "Print", "MarginHeight", margin_height);
  }

  return IUP_DEFAULT;
}

int SimplePaint::ItemPrintActionCallback(Ihandle*)
{
  char* title = IupGetAttribute(dlg, "TITLE");

  cdCanvas* print_canvas = cdCreateCanvasf(CD_PRINTER, "%s -d", title);
  if (!print_canvas)
    return IUP_DEFAULT;

  /* do NOT draw the background, use the paper color */

  /* draw the image at the center of the canvas */
  if (file.GetImage())
  {
    int x, y, canvas_width, canvas_height, view_width, view_height;
    double canvas_width_mm, canvas_height_mm;
    int margin_width = IupConfigGetVariableIntDef(config, "Print", "MarginWidth", 20);
    int margin_height = IupConfigGetVariableIntDef(config, "Print", "MarginHeight", 20);

    cdCanvasGetSize(print_canvas, &canvas_width, &canvas_height, &canvas_width_mm, &canvas_height_mm);

    /* convert to pixels */
    margin_width = (int)((margin_width * canvas_width) / canvas_width_mm);
    margin_height = (int)((margin_height * canvas_height) / canvas_height_mm);

    view_fit_rect(canvas_width - 2 * margin_width, canvas_height - 2 * margin_height, 
                  file.GetImage()->width, file.GetImage()->height, 
                  &view_width, &view_height);

    x = (canvas_width - view_width) / 2;
    y = (canvas_height - view_height) / 2;

    imcdCanvasPutImage(print_canvas, file.GetImage(), x, y, view_width, view_height, 0, 0, 0, 0);
  }

  cdKillCanvas(print_canvas);
  return IUP_DEFAULT;
}

int SimplePaint::ItemExitActionCallback(Ihandle*)
{
  if (!file.SaveCheck())
    return IUP_IGNORE;  /* to abort the CLOSE_CB callback normal processing */

  toolbox.HideDialog();

  file.Close();

  IupConfigDialogClosed(config, dlg, "MainWindow");
  IupConfigSave(config);
  IupDestroy(config);
  return IUP_CLOSE;
}

int SimplePaint::ItemCopyActionCallback(Ihandle*)
{
  Ihandle *clipboard = IupClipboard();
  IupSetAttribute(clipboard, "NATIVEIMAGE", NULL); /* clear clipboard first */
  IupSetAttribute(clipboard, "NATIVEIMAGE", (char*)IupGetImageNativeHandle(file.GetImage()));
  IupDestroy(clipboard);
  return IUP_DEFAULT;
}

int SimplePaint::ItemPasteActionCallback(Ihandle*)
{
  if (file.SaveCheck())
  {
    Ihandle *clipboard = IupClipboard();
    imImage* new_image = IupGetNativeHandleImage(IupGetAttribute(clipboard, "NATIVEIMAGE"));
    IupDestroy(clipboard);

    if (!new_image)
    {
      show_error("Invalid Clipboard Data", 1);
      return IUP_DEFAULT;
    }

    file.New(new_image);

    UpdateFile();
  }
  return IUP_DEFAULT;
}

int SimplePaint::ItemBackgroundActionCallback(Ihandle*)
{
  Ihandle* colordlg = IupColorDlg();
  const char* background = IupConfigGetVariableStrDef(config, "MainWindow", "Background", "255 255 255");
  IupSetStrAttribute(colordlg, "VALUE", background);
  IupSetAttributeHandle(colordlg, "PARENTDIALOG", dlg);

  IupPopup(colordlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  if (IupGetInt(colordlg, "STATUS") == 1)
  {
    background = IupGetAttribute(colordlg, "VALUE");
    IupConfigSetVariableStr(config, "MainWindow", "Background", background);

    IupUpdate(canvas);
  }

  IupDestroy(colordlg);
  return IUP_DEFAULT;
}

int SimplePaint::ItemZoomoutActionCallback(Ihandle*)
{
  Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
  double zoom_index = IupGetDouble(zoom_val, "VALUE");
  zoom_index--;
  if (zoom_index < -6)
    zoom_index = -6;
  IupSetDouble(zoom_val, "VALUE", round(zoom_index));  /* fixed increments when using buttons */

  UpdateZoom(zoom_index);
  return IUP_DEFAULT;
}

int SimplePaint::ItemZoominActionCallback(Ihandle*)
{
  Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
  double zoom_index = IupGetDouble(zoom_val, "VALUE");
  zoom_index++;
  if (zoom_index > 6)
    zoom_index = 6;
  IupSetDouble(zoom_val, "VALUE", round(zoom_index));  /* fixed increments when using buttons */

  UpdateZoom(zoom_index);
  return IUP_DEFAULT;
}

int SimplePaint::ItemActualsizeActionCallback(Ihandle*)
{
  Ihandle* zoom_val = IupGetDialogChild(dlg, "ZOOMVAL");
  IupSetDouble(zoom_val, "VALUE", 0);
  UpdateZoom(0);
  return IUP_DEFAULT;
}

int SimplePaint::ItemZoomgridActionCallback(Ihandle*)
{
  Ihandle* item_zoomgrid = IupGetDialogChild(dlg, "ZOOMGRID");

  if (IupGetInt(item_zoomgrid, "VALUE"))
    IupSetAttribute(item_zoomgrid, "VALUE", "OFF");
  else
    IupSetAttribute(item_zoomgrid, "VALUE", "ON");

  IupConfigSetVariableStr(config, "MainWindow", "ZoomGrid", IupGetAttribute(item_zoomgrid, "VALUE"));

  IupUpdate(canvas);
  return IUP_DEFAULT;
}

int SimplePaint::ItemToolbarActionCallback(Ihandle* item_toolbar)
{
  Ihandle* toolbar = IupGetDialogChild(item_toolbar, "TOOLBAR");

  ToggleBarVisibility(item_toolbar, toolbar);

  IupConfigSetVariableStr(config, "MainWindow", "Toolbar", IupGetAttribute(item_toolbar, "VALUE"));
  return IUP_DEFAULT;
}

int SimplePaint::ItemToolboxActionCallback(Ihandle* item_toolbox)
{
  if (toolbox.HideDialog())
    IupSetAttribute(item_toolbox, "VALUE", "OFF");
  else
  {
    IupSetAttribute(item_toolbox, "VALUE", "ON");
    toolbox.ShowDialog();
  }

  IupConfigSetVariableStr(config, "MainWindow", "Toolbox", IupGetAttribute(item_toolbox, "VALUE"));
  return IUP_DEFAULT;
}

int SimplePaint::ItemStatusbarActionCallback(Ihandle* item_statusbar)
{
  Ihandle* statusbar = IupGetDialogChild(item_statusbar, "STATUSBAR");

  ToggleBarVisibility(item_statusbar, statusbar);

  IupConfigSetVariableStr(config, "MainWindow", "Statusbar", IupGetAttribute(item_statusbar, "VALUE"));
  return IUP_DEFAULT;
}

int SimplePaint::ItemResizeActionCallback(Ihandle*)
{
  int height = file.GetImage()->height,
    width = file.GetImage()->width;
  int quality = IupConfigGetVariableIntDef(config, "Image", "ResizeQuality", 1);  /* medium default */

  if (!IupGetParam("Resize", NULL, NULL,
    "Width: %i[1,]\n"
    "Height: %i[1,]\n"
    "Quality: %l|low|medium|high|\n",
    &width, &height, &quality, NULL))
    return IUP_DEFAULT;

  IupConfigSetVariableInt(config, "Image", "ResizeQuality", quality);

  imImage* new_image = imImageCreateBased(file.GetImage(), width, height, -1, -1);
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  if (quality == 2)
    quality = 3; /* interpolation order can be 0, 1, and 3 */

  imProcessResize(file.GetImage(), new_image, quality);

  UpdateImage(new_image, true);   /* update size */

  return IUP_DEFAULT;
}

int SimplePaint::ItemMirrorActionCallback(Ihandle*)
{
  imImage* new_image = imImageClone(file.GetImage());
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  imProcessMirror(file.GetImage(), new_image);

  UpdateImage(new_image);

  return IUP_DEFAULT;
}

int SimplePaint::ItemFlipActionCallback(Ihandle*)
{
  imImage* new_image = imImageClone(file.GetImage());
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  imProcessFlip(file.GetImage(), new_image);

  UpdateImage(new_image);

  return IUP_DEFAULT;
}

int SimplePaint::ItemRotate180ActionCallback(Ihandle*)
{
  imImage* new_image = imImageClone(file.GetImage());
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  imProcessRotate180(file.GetImage(), new_image);

  UpdateImage(new_image);

  return IUP_DEFAULT;
}

int SimplePaint::ItemRotate90cwActionCallback(Ihandle*)
{
  imImage* new_image = imImageCreateBased(file.GetImage(), file.GetImage()->height, file.GetImage()->width, -1, -1);
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  imProcessRotate90(file.GetImage(), new_image, 1);

  UpdateImage(new_image, true);   /* update size */

  return IUP_DEFAULT;
}

int SimplePaint::ItemRotate90ccwActionCallback(Ihandle*)
{
  imImage* new_image = imImageCreateBased(file.GetImage(), file.GetImage()->height, file.GetImage()->width, -1, -1);
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  imProcessRotate90(file.GetImage(), new_image, -1);

  UpdateImage(new_image, true);   /* update size */

  return IUP_DEFAULT;
}

int SimplePaint::ItemNegativeActionCallback(Ihandle*)
{
  imImage* new_image = imImageClone(file.GetImage());
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  imProcessNegative(file.GetImage(), new_image);

  UpdateImage(new_image);

  return IUP_DEFAULT;
}

static int brightcont_param_cb(Ihandle* dialog, int param_index, void* user_data)
{
  Ihandle* canvas = (Ihandle*)user_data;

  if (param_index == 0 || param_index == 1)
  {
    float param[2] = { 0, 0 };
    imImage* image = (imImage*)IupGetAttribute(canvas, "ORIGINAL_IMAGE");
    imImage* new_image = (imImage*)IupGetAttribute(canvas, "NEW_IMAGE");
    SimplePaintFile* file = (SimplePaintFile*)IupGetAttribute(canvas, "IMAGE_FILE");
    Ihandle* brightness_shift_param = (Ihandle*)IupGetAttribute(dialog, "PARAM0");
    Ihandle* contrast_factor_param = (Ihandle*)IupGetAttribute(dialog, "PARAM1");
    param[0] = IupGetFloat(brightness_shift_param, "VALUE");
    param[1] = IupGetFloat(contrast_factor_param, "VALUE");

    imProcessToneGamut(image, new_image, IM_GAMUT_BRIGHTCONT, param);

    file->SetImage(new_image, false);

    IupUpdate(canvas);
  }
  else if (param_index != IUP_GETPARAM_INIT)
  {
    /* restore original configuration */
    SimplePaintFile* file = (SimplePaintFile*)IupGetAttribute(canvas, "IMAGE_FILE");
    imImage* image = (imImage*)IupGetAttribute(canvas, "ORIGINAL_IMAGE");

    file->SetImage(image, false);

    IupSetAttribute(canvas, "ORIGINAL_IMAGE", NULL);
    IupSetAttribute(canvas, "NEW_IMAGE", NULL);
    IupSetAttribute(canvas, "IMAGE_FILE", NULL);

    if (param_index == IUP_GETPARAM_BUTTON2)  /* cancel */
      IupUpdate(canvas);
  }

  return 1;
}

int SimplePaint::ItemBrightcontActionCallback(Ihandle*)
{
  imImage* new_image = imImageClone(file.GetImage());
  if (!new_image)
  {
    show_file_error(IM_ERR_MEM);
    return IUP_DEFAULT;
  }

  IupSetAttribute(canvas, "ORIGINAL_IMAGE", (char*)file.GetImage());
  IupSetAttribute(canvas, "NEW_IMAGE", (char*)new_image);
  IupSetAttribute(canvas, "IMAGE_FILE", (char*)&file);

  float param[2] = { 0, 0 };

  if (!IupGetParam("Brightness and Contrast", brightcont_param_cb, canvas,
                   "Brightness Shift: %r[-100,100]\n"
                   "Contrast Factor: %r[-100,100]\n",
                   &param[0], &param[1], NULL))
  {
    imImageDestroy(new_image);
    return IUP_DEFAULT;
  }

  /* since the image was already processed in preview we don't need to process it again,
     but leave it here to illustrate the logic */
  imProcessToneGamut(file.GetImage(), new_image, IM_GAMUT_BRIGHTCONT, param);

  UpdateImage(new_image);

  return IUP_DEFAULT;
}

int SimplePaint::ItemHelpActionCallback(Ihandle*)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

int SimplePaint::ItemAboutActionCallback(Ihandle*)
{
  IupMessage("About", "   Simple Paint\n\nAutors:\n   Gustavo Lyrio\n   Antonio Scuri");
  return IUP_DEFAULT;
}


/********************************** Main *****************************************/


Ihandle* SimplePaint::CreateMainMenu()
{
  Ihandle *menu, *sub_menu_file;
  Ihandle *file_menu, *item_exit, *item_new, *item_open, *item_save, *item_saveas, *item_revert;
  Ihandle *sub_menu_edit, *edit_menu, *item_copy, *item_paste, *item_print, *item_pagesetup;
  Ihandle *sub_menu_help, *help_menu, *item_help, *item_about;
  Ihandle *sub_menu_view, *view_menu, *item_toolbar, *item_statusbar;
  Ihandle *item_zoomin, *item_zoomout, *item_actualsize;
  Ihandle *recent_menu, *item_background, *item_toolbox, *item_zoomgrid;
  Ihandle *sub_menu_image, *image_menu;

  item_new = IupItem("&New\tCtrl+N", NULL);
  IupSetAttribute(item_new, "IMAGE", "IUP_FileNew");
  IUP_CLASS_SETCALLBACK(item_new, "ACTION", ItemNewActionCallback);

  item_open = IupItem("&Open...\tCtrl+O", NULL);
  IupSetAttribute(item_open, "IMAGE", "IUP_FileOpen");
  IUP_CLASS_SETCALLBACK(item_open, "ACTION", ItemOpenActionCallback);

  item_save = IupItem("&Save\tCtrl+S", NULL);
  IupSetAttribute(item_save, "NAME", "ITEM_SAVE");
  IupSetAttribute(item_save, "IMAGE", "IUP_FileSave");
  IUP_CLASS_SETCALLBACK(item_save, "ACTION", ItemSaveActionCallback);

  item_saveas = IupItem("Save &As...", NULL);
  IupSetAttribute(item_saveas, "NAME", "ITEM_SAVEAS");
  IUP_CLASS_SETCALLBACK(item_saveas, "ACTION", ItemSaveasActionCallback);

  item_revert = IupItem("&Revert", NULL);
  IupSetAttribute(item_revert, "NAME", "ITEM_REVERT");
  IUP_CLASS_SETCALLBACK(item_revert, "ACTION", ItemRevertActionCallback);

  item_pagesetup = IupItem("Page Set&up...", NULL);
  IUP_CLASS_SETCALLBACK(item_pagesetup, "ACTION", ItemPagesetupActionCallback);

  item_print = IupItem("&Print...\tCtrl+P", NULL);
  IUP_CLASS_SETCALLBACK(item_print, "ACTION", ItemPrintActionCallback);

  item_exit = IupItem("E&xit", NULL);
  IUP_CLASS_SETCALLBACK(item_exit, "ACTION", ItemExitActionCallback);

  item_copy = IupItem("&Copy\tCtrl+C", NULL);
  IupSetAttribute(item_copy, "NAME", "ITEM_COPY");
  IupSetAttribute(item_copy, "IMAGE", "IUP_EditCopy");
  IUP_CLASS_SETCALLBACK(item_copy, "ACTION", ItemCopyActionCallback);

  item_paste = IupItem("&Paste\tCtrl+V", NULL);
  IupSetAttribute(item_paste, "NAME", "ITEM_PASTE");
  IupSetAttribute(item_paste, "IMAGE", "IUP_EditPaste");
  IUP_CLASS_SETCALLBACK(item_paste, "ACTION", ItemPasteActionCallback);

  item_zoomin = IupItem("Zoom &In\tCtrl++", NULL);
  IupSetAttribute(item_zoomin, "IMAGE", "IUP_ZoomIn");
  IUP_CLASS_SETCALLBACK(item_zoomin, "ACTION", ItemZoominActionCallback);

  item_zoomout = IupItem("Zoom &Out\tCtrl+-", NULL);
  IupSetAttribute(item_zoomout, "IMAGE", "IUP_ZoomOut");
  IUP_CLASS_SETCALLBACK(item_zoomout, "ACTION", ItemZoomoutActionCallback);

  item_actualsize = IupItem("&Actual Size\tCtrl+0", NULL);
  IupSetAttribute(item_actualsize, "IMAGE", "IUP_ZoomActualSize");
  IUP_CLASS_SETCALLBACK(item_actualsize, "ACTION", ItemActualsizeActionCallback);

  item_zoomgrid = IupItem("&Zoom Grid", NULL);
  IUP_CLASS_SETCALLBACK(item_zoomgrid, "ACTION", ItemZoomgridActionCallback);
  IupSetAttribute(item_zoomgrid, "NAME", "ZOOMGRID");
  IupSetAttribute(item_zoomgrid, "VALUE", "ON");  /* default is ON */

  item_background = IupItem("&Background...", NULL);
  IUP_CLASS_SETCALLBACK(item_background, "ACTION", ItemBackgroundActionCallback);

  item_toolbar = IupItem("&Toobar", NULL);
  IUP_CLASS_SETCALLBACK(item_toolbar, "ACTION", ItemToolbarActionCallback);
  IupSetAttribute(item_toolbar, "VALUE", "ON");   /* default is ON */

  item_toolbox = IupItem("&Toobox", NULL);
  IUP_CLASS_SETCALLBACK(item_toolbox, "ACTION", ItemToolboxActionCallback);
  IupSetAttribute(item_toolbox, "NAME", "TOOLBOXMENU");
  IupSetAttribute(item_toolbox, "VALUE", "ON");   /* default is ON */

  item_statusbar = IupItem("&Statusbar", NULL);
  IUP_CLASS_SETCALLBACK(item_statusbar, "ACTION", ItemStatusbarActionCallback);
  IupSetAttribute(item_statusbar, "VALUE", "ON");  /* default is ON */

  item_help = IupItem("&Help...", NULL);
  IUP_CLASS_SETCALLBACK(item_help, "ACTION", ItemHelpActionCallback);

  item_about = IupItem("&About...", NULL);
  IUP_CLASS_SETCALLBACK(item_about, "ACTION", ItemAboutActionCallback);

  recent_menu = IupMenu(NULL);

  file_menu = IupMenu(
    item_new,
    item_open,
    item_save,
    item_saveas,
    item_revert,
    IupSeparator(),
    item_pagesetup,
    item_print,
    IupSeparator(),
    IupSubmenu("Recent &Files", recent_menu),
    IupSeparator(),
    item_exit,
    NULL);
  edit_menu = IupMenu(
    item_copy,
    item_paste,
    NULL);
  view_menu = IupMenu(
    item_zoomin,
    item_zoomout,
    item_actualsize,
    item_zoomgrid,
    IupSeparator(),
    item_background,
    IupSeparator(),
    item_toolbar,
    item_toolbox,
    item_statusbar,
    NULL);
  image_menu = IupMenu(
    IupSetCallbacks(IupItem("&Resize...", NULL), "ACTION", CB_ItemResizeActionCallback, NULL),
    IupSetCallbacks(IupItem("&Mirror", NULL), "ACTION", CB_ItemMirrorActionCallback, NULL),
    IupSetCallbacks(IupItem("&Flip", NULL), "ACTION", CB_ItemFlipActionCallback, NULL),
    IupSetCallbacks(IupItem("&Rotate 180º", NULL), "ACTION", CB_ItemRotate180ActionCallback, NULL),
    IupSetCallbacks(IupItem("&Rotate +90º (clock-wise)", NULL), "ACTION", CB_ItemRotate90cwActionCallback, NULL),
    IupSetCallbacks(IupItem("&Rotate -90º (counter-clock)", NULL), "ACTION", CB_ItemRotate90ccwActionCallback, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("&Negative", NULL), "ACTION", CB_ItemNegativeActionCallback, NULL),
    IupSetCallbacks(IupItem("&Brightness and Contrast...", NULL), "ACTION", CB_ItemBrightcontActionCallback, NULL),
    NULL);
  help_menu = IupMenu(
    item_help,
    item_about,
    NULL);
  
  IUP_CLASS_SETCALLBACK(file_menu, "OPEN_CB", FileMenuOpenCallback);
  IUP_CLASS_SETCALLBACK(edit_menu, "OPEN_CB", EditMenuOpenCallback);

  sub_menu_file = IupSubmenu("&File", file_menu);
  sub_menu_edit = IupSubmenu("&Edit", edit_menu);
  sub_menu_view = IupSubmenu("&View", view_menu);
  sub_menu_image = IupSubmenu("&Image", image_menu);
  sub_menu_help = IupSubmenu("&Help", help_menu);

  menu = IupMenu(
    sub_menu_file,
    sub_menu_edit,
    sub_menu_view,
    sub_menu_image,
    sub_menu_help,
    NULL);

  /* Initialize variables from the configuration file */

  IupConfigRecentInit(config, recent_menu, CB_ConfigRecentCallback, 10);

  if (!IupConfigGetVariableIntDef(config, "MainWindow", "ZoomGrid", 1))
    IupSetAttribute(item_zoomgrid, "VALUE", "OFF");

  if (!IupConfigGetVariableIntDef(config, "MainWindow", "Toolbar", 1))
    IupSetAttribute(item_toolbar, "VALUE", "OFF");

  if (!IupConfigGetVariableIntDef(config, "MainWindow", "Toolbox", 1))
    IupSetAttribute(item_toolbox, "VALUE", "OFF");

  if (!IupConfigGetVariableIntDef(config, "MainWindow", "Statusbar", 1))
    IupSetAttribute(item_statusbar, "VALUE", "OFF");

  return menu;
}

Ihandle* SimplePaint::CreateToolbar()
{
  Ihandle *toolbar;
  Ihandle *btn_copy, *btn_paste, *btn_new, *btn_open, *btn_save, *btn_zoomgrid;

  IupSetHandle("PaintZoomGrid", load_image_PaintZoomGrid());

  btn_new = IupButton(NULL, NULL);
  IupSetAttribute(btn_new, "IMAGE", "IUP_FileNew");
  IupSetAttribute(btn_new, "FLAT", "Yes");
  IUP_CLASS_SETCALLBACK(btn_new, "ACTION", ItemNewActionCallback);
  IupSetAttribute(btn_new, "TIP", "New (Ctrl+N)");
  IupSetAttribute(btn_new, "CANFOCUS", "No");

  btn_open = IupButton(NULL, NULL);
  IupSetAttribute(btn_open, "IMAGE", "IUP_FileOpen");
  IupSetAttribute(btn_open, "FLAT", "Yes");
  IUP_CLASS_SETCALLBACK(btn_open, "ACTION", ItemOpenActionCallback);
  IupSetAttribute(btn_open, "TIP", "Open (Ctrl+O)");
  IupSetAttribute(btn_open, "CANFOCUS", "No");

  btn_save = IupButton(NULL, NULL);
  IupSetAttribute(btn_save, "IMAGE", "IUP_FileSave");
  IupSetAttribute(btn_save, "FLAT", "Yes");
  IUP_CLASS_SETCALLBACK(btn_save, "ACTION", ItemSaveActionCallback);
  IupSetAttribute(btn_save, "TIP", "Save (Ctrl+S)");
  IupSetAttribute(btn_save, "CANFOCUS", "No");

  btn_copy = IupButton(NULL, NULL);
  IupSetAttribute(btn_copy, "IMAGE", "IUP_EditCopy");
  IupSetAttribute(btn_copy, "FLAT", "Yes");
  IUP_CLASS_SETCALLBACK(btn_copy, "ACTION", ItemCopyActionCallback);
  IupSetAttribute(btn_copy, "TIP", "Copy (Ctrl+C)");
  IupSetAttribute(btn_copy, "CANFOCUS", "No");

  btn_paste = IupButton(NULL, NULL);
  IupSetAttribute(btn_paste, "IMAGE", "IUP_EditPaste");
  IupSetAttribute(btn_paste, "FLAT", "Yes");
  IUP_CLASS_SETCALLBACK(btn_paste, "ACTION", ItemPasteActionCallback);
  IupSetAttribute(btn_paste, "TIP", "Paste (Ctrl+V)");
  IupSetAttribute(btn_paste, "CANFOCUS", "No");

  btn_zoomgrid = IupButton(NULL, NULL);
  IupSetAttribute(btn_zoomgrid, "IMAGE", "PaintZoomGrid");
  IupSetAttribute(btn_zoomgrid, "FLAT", "Yes");
  IUP_CLASS_SETCALLBACK(btn_zoomgrid, "ACTION", ItemZoomgridActionCallback);
  IupSetAttribute(btn_zoomgrid, "TIP", "Zoom Grid");
  IupSetAttribute(btn_paste, "CANFOCUS", "No");

  toolbar = IupHbox(
    btn_new,
    btn_open,
    btn_save,
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"),
    btn_copy,
    btn_paste,
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"),
    btn_zoomgrid,
    NULL);
  IupSetAttribute(toolbar, "MARGIN", "5x5");
  IupSetAttribute(toolbar, "GAP", "2");
  IupSetAttribute(toolbar, "NAME", "TOOLBAR");

  /* Initialize variables from the configuration file */

  if (!IupConfigGetVariableIntDef(config, "MainWindow", "Toolbar", 1))
  {
    IupSetAttribute(toolbar, "FLOATING", "YES");
    IupSetAttribute(toolbar, "VISIBLE", "NO");
  }

  return toolbar;
}

Ihandle* SimplePaint::CreateStatusbar()
{
  Ihandle *statusbar;

  statusbar = IupHbox(
    IupSetAttributes(IupLabel("(0, 0) = 0   0   0"), "EXPAND=HORIZONTAL, PADDING=10x5, NAME=STATUSLABEL"),
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"),
    IupSetAttributes(IupLabel("0 x 0"), "SIZE=70x, PADDING=10x5, NAME=SIZELABEL, ALIGNMENT=ACENTER"),
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"),
    IupSetAttributes(IupLabel("100%"), "SIZE=30x, PADDING=10x5, NAME=ZOOMLABEL, ALIGNMENT=ARIGHT"),
    IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "IMAGE=IUP_ZoomOut, FLAT=Yes, TIP=\"Zoom Out (Ctrl+-)\""), "ACTION", CB_ItemZoomoutActionCallback, NULL),
    IupSetCallbacks(IupSetAttributes(IupVal(NULL), "VALUE=0, MIN=-6, MAX=6, RASTERSIZE=150x25, NAME=ZOOMVAL"), "VALUECHANGED_CB", CB_ZoomValueChangedCallback, NULL),
    IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "IMAGE=IUP_ZoomIn, FLAT=Yes, TIP=\"Zoom In (Ctrl++)\""), "ACTION", CB_ItemZoominActionCallback, NULL),
    IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "IMAGE=IUP_ZoomActualSize, FLAT=Yes, TIP=\"Actual Size (Ctrl+0)\""), "ACTION", CB_ItemActualsizeActionCallback, NULL),
    NULL);
  IupSetAttribute(statusbar, "NAME", "STATUSBAR");
  IupSetAttribute(statusbar, "ALIGNMENT", "ACENTER");

  /* Initialize variables from the configuration file */

  if (!IupConfigGetVariableIntDef(config, "MainWindow", "Statusbar", 1))
  {
    IupSetAttribute(statusbar, "FLOATING", "YES");
    IupSetAttribute(statusbar, "VISIBLE", "NO");
  }

  return statusbar;
}

void SimplePaint::CreateMainDialog()
{
  Ihandle *vbox;

#ifdef USE_OPENGL
  canvas = IupGLCanvas(NULL);
  IupSetAttribute(canvas, "BUFFER", "DOUBLE");
#else
  canvas = IupCanvas(NULL);
#endif
  IupSetAttribute(canvas, "SCROLLBAR", "Yes");
  IUP_CLASS_SETCALLBACK(canvas, "ACTION", CanvasActionCallback);
  IUP_CLASS_SETCALLBACK(canvas, "MAP_CB", CanvasMapCallback);
  IUP_CLASS_SETCALLBACK(canvas, "UNMAP_CB", CanvasUnmapCallback);
  IUP_CLASS_SETCALLBACK(canvas, "WHEEL_CB", CanvasWheelCallback);
  IUP_CLASS_SETCALLBACK(canvas, "RESIZE_CB", CanvasResizeCallback);
  IUP_CLASS_SETCALLBACK(canvas, "MOTION_CB", CanvasMotionCallback);
  IUP_CLASS_SETCALLBACK(canvas, "BUTTON_CB", CanvasButtonCallback);
  IUP_CLASS_SETCALLBACK(canvas, "DROPFILES_CB", DropfilesCallback);

  vbox = IupVbox(
    CreateToolbar(),
    canvas,
    CreateStatusbar(),
    NULL);

  dlg = IupDialog(vbox);
  IupSetAttributeHandle(dlg, "MENU", CreateMainMenu());
  IupSetAttribute(dlg, "SIZE", "HALFxHALF");
  IUP_CLASS_SETCALLBACK(dlg, "CLOSE_CB", ItemExitActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "DROPFILES_CB", DropfilesCallback);
  IUP_CLASS_SETCALLBACK(dlg, "MOVE_CB", DialogMoveCallback);

  IUP_CLASS_SETCALLBACK(dlg, "K_cN", ItemNewActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cO", ItemOpenActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cS", ItemSaveActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cV", ItemPasteActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cC", ItemCopyActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cP", ItemPrintActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cMinus", ItemZoomoutActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cPlus", ItemZoominActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_cEqual", ItemZoominActionCallback);
  IUP_CLASS_SETCALLBACK(dlg, "K_c0", ItemActualsizeActionCallback);

  /* parent for pre-defined dialogs in closed functions (IupMessage and IupAlarm) */
  IupSetAttributeHandle(NULL, "PARENTDIALOG", dlg);
}

SimplePaint::SimplePaint()
  :cd_canvas(NULL), toolbox()
{
  interact.overlay = false;

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "simple_paint");
  IupConfigLoad(config);

  CreateMainDialog();

  IUP_CLASS_INITCALLBACK(dlg, SimplePaint);

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, dlg, "MainWindow");

  /* create and show the toolbox */
  toolbox.CreateDialog(canvas, config);
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupImageLibOpen();
#ifdef USE_OPENGL
  IupGLCanvasOpen();
#endif
#ifdef USE_CONTEXTPLUS
  cdInitContextPlus();
#endif

  SimplePaint app;

  /* open a file from the command line (allow file association in Windows) */
  if (argc > 1 && argv[1])
  {
    const char* filename = argv[1];
    app.OpenFile(filename);
  }

  /* initialize the current file, if not already loaded */
  app.CheckNewFile();

  IupMainLoop();

  IupClose();
  return EXIT_SUCCESS;
}

/* Possible Enhancements:
- Save last used toolbox options in configuration file?
- Hide/show toolbox options according to selected tool
- Capture image from Camera using IM
- Undo/Redo
- Secondary color for drawing of shapes with both outline and filled at the same time
- Alpha for colors
- Area Selection
*/
