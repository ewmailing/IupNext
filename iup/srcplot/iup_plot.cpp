/*
 * IupPlot element
 *
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
#include <math.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iup_plot.h"
#include "iupkey.h"
#include "iupgl.h"

#include <cd.h>
#include <cdgl.h>
#include <cdiup.h>

#include "iup_class.h"
#include "iup_register.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_assert.h"

#include "iupPlot.h"


#define IUP_PLOT_MAX_PLOTS 20
enum { IUP_PLOT_NATIVE, IUP_PLOT_IMAGERGB, IUP_PLOT_OPENGL };

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  iupPlot* plot_list[IUP_PLOT_MAX_PLOTS];
  int plot_list_count;

  iupPlot* current_plot;
  int current_plot_index;

  int numcol;
  int sync_view;

  cdCanvas* cd_canvas;
  int graphics_mode;

  int last_click_x, 
      last_click_y;
};

#ifndef M_E
#define M_E 2.71828182846
#endif

static int iPlotGetCDFontStyle(const char* value);

static char* iupStrReturnColor(long color)
{
  unsigned char r, g, b, a;
  cdDecodeColor(color, &r, &g, &b);
  a = cdDecodeAlpha(color);
  return iupStrReturnRGBA(r, g, b, a);
}

static int iupStrToColor(const char* str, long *color)
{
  unsigned char r, g, b, a;
  if (iupStrToRGBA(str, &r, &g, &b, &a))
  {
    *color = cdEncodeColor(r, g, b);
    *color = cdEncodeAlpha(*color, a);
    return 1;
  }
  return 0;
}

static void iPlotFlush(Ihandle* ih, cdCanvas* canvas)
{
  cdCanvasFlush(canvas);
  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
    IupGLSwapBuffers(ih);
}

static void iPlotSetPlotCurrent(Ihandle* ih, int p)
{
  ih->data->current_plot_index = p;
  ih->data->current_plot = ih->data->plot_list[ih->data->current_plot_index];
}

static void iPlotUpdateCanvasOrigin(iupPlot* plot, cdCanvas* canvas)
{
  if (plot->ih->data->graphics_mode == IUP_PLOT_OPENGL)
    IupGLMakeCurrent(plot->ih);

  cdCanvasActivate(canvas);
  cdCanvasOrigin(canvas, plot->mViewport.mX, plot->mViewport.mY);
}

static int iPlotRedraw_CB(Ihandle* ih)
{
  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
    IupGLMakeCurrent(ih);

  int old_current = ih->data->current_plot_index;
  for(int p=0; p<ih->data->plot_list_count; p++)
  {
    iPlotSetPlotCurrent(ih, p);
    ih->data->current_plot->mRedraw = 1;
    ih->data->current_plot->Render(ih->data->cd_canvas);
  }
  iPlotSetPlotCurrent(ih, old_current);

  // Do the flush once
  iPlotFlush(ih, ih->data->cd_canvas);

  return IUP_DEFAULT;
}

static void iPlotUpdateViewports(Ihandle* ih)
{
  int w, h;

  cdCanvasActivate(ih->data->cd_canvas);
  cdCanvasGetSize(ih->data->cd_canvas, &w, &h, NULL, NULL);

  int numcol = ih->data->numcol;
  if (numcol > ih->data->plot_list_count) numcol = ih->data->plot_list_count;
  int numlin = ih->data->plot_list_count / numcol;
  int pw = w / numcol;
  int ph = h / numlin;

  for(int p=0; p<ih->data->plot_list_count; p++)
  {
    int lin = p / numcol;
    int col = p % numcol;
    int px = col * pw;
    int py = lin * ph;

    ih->data->plot_list[p]->SetViewport(px, py, pw, ph);
  }
}

static int iPlotResize_CB(Ihandle* ih, int width, int height)
{
  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
  {
    IupGLMakeCurrent(ih);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.375, 0.375, 0.0);  /* render all primitives at integer positions */

    double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
    cdCanvasSetfAttribute(ih->data->cd_canvas, "SIZE", "%dx%d %g", width, height, res);
  }

  iPlotUpdateViewports(ih);
  return IUP_DEFAULT;
}

static void iPlotResetZoom(Ihandle *ih, int redraw)
{
  ih->data->current_plot->ResetZoom();
  bool flush = ih->data->current_plot->mRedraw;
  if (redraw) ih->data->current_plot->Render(ih->data->cd_canvas);

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
      {
        ih->data->plot_list[p]->ResetZoom();
        if (redraw) ih->data->plot_list[p]->Render(ih->data->cd_canvas);
      }
    }
  }

  if (redraw && flush)
    iPlotFlush(ih, ih->data->cd_canvas);
}

static void iPlotPanStart(Ihandle *ih)
{
  ih->data->current_plot->PanStart();

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
        ih->data->plot_list[p]->PanStart();
    }
  }
}

static void iPlotPan(Ihandle *ih, int x1, int y1, int x2, int y2)
{
  double rx1, ry1, rx2, ry2;
  ih->data->current_plot->TransformBack(x1, y1, rx1, ry1);
  ih->data->current_plot->TransformBack(x2, y2, rx2, ry2);

  double offsetX = rx2 - rx1;
  double offsetY = ry2 - ry1;

  ih->data->current_plot->Pan(offsetX, offsetY);
  bool flush = ih->data->current_plot->mRedraw;
  ih->data->current_plot->Render(ih->data->cd_canvas);

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
      {
        ih->data->plot_list[p]->TransformBack(x1, y1, rx1, ry1);
        ih->data->plot_list[p]->TransformBack(x2, y2, rx2, ry2);

        offsetX = rx2 - rx1;
        offsetY = ry2 - ry1;

        ih->data->plot_list[p]->Pan(offsetX, offsetY);
        ih->data->plot_list[p]->Render(ih->data->cd_canvas);
      }
    }
  }

  if (flush)
    iPlotFlush(ih, ih->data->cd_canvas);
}

static void iPlotZoom(Ihandle *ih, int x, int y, float delta)
{
  double rx, ry;
  ih->data->current_plot->TransformBack(x, y, rx, ry);

  if (delta > 0)
    ih->data->current_plot->ZoomIn(rx, ry);
  else
    ih->data->current_plot->ZoomOut(rx, ry);

  bool flush = ih->data->current_plot->mRedraw;
  ih->data->current_plot->Render(ih->data->cd_canvas);

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
      {
        ih->data->plot_list[p]->TransformBack(x, y, rx, ry);

        if (delta > 0)
          ih->data->plot_list[p]->ZoomIn(rx, ry);
        else
          ih->data->plot_list[p]->ZoomOut(rx, ry);

        ih->data->plot_list[p]->Render(ih->data->cd_canvas);  
      }
    }
  }

  if (flush)
    iPlotFlush(ih, ih->data->cd_canvas);
}

static void iPlotScroll(Ihandle *ih, float delta, bool vertical)
{
  ih->data->current_plot->Scroll(delta, false, vertical);
  bool flush = ih->data->current_plot->mRedraw;
  ih->data->current_plot->Render(ih->data->cd_canvas);  

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
      {
        ih->data->plot_list[p]->Scroll(delta, false, vertical);
        ih->data->plot_list[p]->Render(ih->data->cd_canvas);
      }
    }
  }

  if (flush)
    iPlotFlush(ih, ih->data->cd_canvas);
}

static int iPlotFindPlot(Ihandle* ih, int x, int &y)
{
  int w, h;

  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
    IupGLMakeCurrent(ih);

  cdCanvasActivate(ih->data->cd_canvas);
  cdCanvasGetSize(ih->data->cd_canvas, &w, &h, NULL, NULL);

  y = cdCanvasInvertYAxis(ih->data->cd_canvas, y);

  int numcol = ih->data->numcol;
  if (numcol > ih->data->plot_list_count) numcol = ih->data->plot_list_count;
  int numlin = ih->data->plot_list_count / numcol;
  int pw = w / numcol;
  int ph = h / numlin;

  int lin = y / ph;
  int col = x / pw;

  int index = lin * numcol + col;
  if (index < ih->data->plot_list_count)
    return index;

  return -1;
}

static int iPlotMouseButton_CB(Ihandle* ih, int button, int press, int x, int y, char* status)
{
  int index = iPlotFindPlot(ih, x, y);
  if (index<0)
    return IUP_DEFAULT;

  iPlotSetPlotCurrent(ih, index);

  x -= ih->data->current_plot->mViewport.mX;
  y -= ih->data->current_plot->mViewport.mY;

  IFniidds cb = (IFniidds)IupGetCallback(ih, "PLOTBUTTON_CB");
  if (cb) 
  {
    iPlotUpdateCanvasOrigin(ih->data->current_plot, ih->data->cd_canvas);

    double rx, ry;
    ih->data->current_plot->TransformBack(x, y, rx, ry);
    if (cb(ih, button, press, rx, ry, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (press)
  {
    ih->data->last_click_x = x;
    ih->data->last_click_y = y;

    if (button == IUP_BUTTON1)
    {
      if (iup_isdouble(status))
        iPlotResetZoom(ih, 1);
      else
        iPlotPanStart(ih);
    }
  }
  else
  {
    if (ih->data->last_click_x == x && ih->data->last_click_y == y && iup_iscontrol(status))
    {
      float delta = 0;
      if (button == IUP_BUTTON1)
        delta = 1.0;
      else if (button == IUP_BUTTON3)
        delta = -1.0;

      if (delta)
        iPlotZoom(ih, x, y, delta);
    }
  }

  return IUP_DEFAULT;
}

static int iPlotMouseMove_CB(Ihandle* ih, int x, int y, char *status)
{
  int index = iPlotFindPlot(ih, x, y);
  if (index<0)
    return IUP_DEFAULT;

  iPlotSetPlotCurrent(ih, index);

  x -= ih->data->current_plot->mViewport.mX;
  y -= ih->data->current_plot->mViewport.mY;

  IFndds cb = (IFndds)IupGetCallback(ih, "PLOTMOTION_CB");
  if (cb) 
  {
    iPlotUpdateCanvasOrigin(ih->data->current_plot, ih->data->cd_canvas);

    double rx, ry;
    ih->data->current_plot->TransformBack(x, y, rx, ry);
    if (cb(ih, rx, ry, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (iup_isbutton1(status) && (ih->data->last_click_x != x || ih->data->last_click_y != y))
    iPlotPan(ih, ih->data->last_click_x, ih->data->last_click_y, x, y);

  return IUP_DEFAULT;
}

static int iPlotWheel_CB(Ihandle *ih, float delta, int x, int y, char* status)
{
  int index = iPlotFindPlot(ih, x, y);
  if (index<0)
    return IUP_DEFAULT;

  iPlotSetPlotCurrent(ih, index);

  x -= ih->data->current_plot->mViewport.mX;
  y -= ih->data->current_plot->mViewport.mY;

  if (iup_iscontrol(status))
    iPlotZoom(ih, x, y, delta);
  else if (ih->data->current_plot->HasZoom())
  {
    bool vertical = true;
    if (iup_isshift(status))
      vertical = false;

    iPlotScroll(ih, delta, vertical);
  }

  return IUP_DEFAULT;
}

static int iPlotKeyPress_CB(Ihandle* ih, int c, int press)
{
  int old_current = ih->data->current_plot_index;
  for (int p = 0; p<ih->data->plot_list_count; p++)
  {
    iPlotSetPlotCurrent(ih, p);
    //TODO
    //    ih->data->current_plot->KeyPress(c, press);
  }
  iPlotSetPlotCurrent(ih, old_current);
  return IUP_DEFAULT;
} 


/************************************************************************************/


void IupPlotBegin(Ihandle* ih, int strXdata)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataBase* inXData = (iupPlotDataBase*)iupAttribGet(ih, "_IUP_PLOT_XDATA");
  iupPlotDataBase* inYData = (iupPlotDataBase*)iupAttribGet(ih, "_IUP_PLOT_YDATA");

  if (inXData) delete inXData;
  if (inYData) delete inYData;

  if (strXdata)
    inXData = (iupPlotDataBase*)(new iupPlotDataString());
  else
    inXData = (iupPlotDataBase*)(new iupPlotDataReal());

  inYData = (iupPlotDataBase*)new iupPlotDataReal();

  iupAttribSet(ih, "_IUP_PLOT_XDATA",    (char*)inXData);
  iupAttribSet(ih, "_IUP_PLOT_YDATA",    (char*)inYData);
}

void IupPlotAdd(Ihandle* ih, double x, double y)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataReal* inXData = (iupPlotDataReal*)iupAttribGet(ih, "_IUP_PLOT_XDATA");
  iupPlotDataReal* inYData = (iupPlotDataReal*)iupAttribGet(ih, "_IUP_PLOT_YDATA");

  if (!inYData || !inXData || inXData->IsString())
    return;

  inXData->AddItem(x);
  inYData->AddItem(y);
}

void IupPlotAddStr(Ihandle* ih, const char* x, double y)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataString *inXData = (iupPlotDataString*)iupAttribGet(ih, "_IUP_PLOT_XDATA");
  iupPlotDataReal *inYData = (iupPlotDataReal*)iupAttribGet(ih, "_IUP_PLOT_YDATA");

  if (!inYData || !inXData || !inXData->IsString())
    return;

  inXData->AddItem(x);
  inYData->AddItem(y);
}

void IupPlotInsertStr(Ihandle* ih, int inIndex, int inSampleIndex, const char* inX, double inY)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataString *theXData = (iupPlotDataString*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || !theXData->IsString())
    return;

  theXData->InsertItem(inSampleIndex, inX);
  theYData->InsertItem(inSampleIndex, inY);
}

void IupPlotInsert(Ihandle* ih, int inIndex, int inSampleIndex, double inX, double inY)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataReal *theXData = (iupPlotDataReal*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || theXData->IsString())
    return;

  theXData->InsertItem(inSampleIndex, inX);
  theYData->InsertItem(inSampleIndex, inY);
}

void IupPlotGetSample(Ihandle* ih, int inIndex, int inSampleIndex, double *x, double *y)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS ||
    !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataReal *theXData = (iupPlotDataReal*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || theXData->IsString())
    return;

  int theCount = theXData->GetCount();
  if (inSampleIndex < 0 || inSampleIndex >= theCount)
    return;

  if (x) *x = theXData->GetValue(inSampleIndex);
  if (y) *y = theYData->GetValue(inSampleIndex);
}

void IupPlotGetSampleStr(Ihandle* ih, int inIndex, int inSampleIndex, const char* *x, double *y)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS ||
    !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataString *theXData = (iupPlotDataString*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || !theXData->IsString())
    return;

  int theCount = theXData->GetCount();
  if (inSampleIndex < 0 || inSampleIndex >= theCount)
    return;

  if (x) *x = theXData->GetStrValue(inSampleIndex);
  if (y) *y = theYData->GetValue(inSampleIndex);
}

void IupPlotAddPoints(Ihandle* ih, int inIndex, double *x, double *y, int count)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataReal *theXData = (iupPlotDataReal*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || theXData->IsString())
    return;

  for (int i=0; i<count; i++)
  {
    theXData->AddItem(x[i]);
    theYData->AddItem(y[i]);
  }
}

void IupPlotAddStrPoints(Ihandle* ih, int inIndex, const char** x, double* y, int count)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataString *theXData = (iupPlotDataString*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || !theXData->IsString())
    return;

  for (int i=0; i<count; i++)
  {
    theXData->AddItem(x[i]);
    theYData->AddItem(y[i]);
  }
}

void IupPlotInsertStrPoints(Ihandle* ih, int inIndex, int inSampleIndex, const char** inX, double* inY, int count)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataString *theXData = (iupPlotDataString*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || !theXData->IsString())
    return;

  for (int i=0; i<count; i++)
  {
    theXData->InsertItem(inSampleIndex+i, inX[i]);
    theYData->InsertItem(inSampleIndex+i, inY[i]);
  }
}

void IupPlotInsertPoints(Ihandle* ih, int inIndex, int inSampleIndex, double *inX, double *inY, int count)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  iupPlotDataSet* theDataSet = ih->data->current_plot->mDataSetList[inIndex];
  iupPlotDataReal *theXData = (iupPlotDataReal*)theDataSet->mDataX;
  iupPlotDataReal *theYData = (iupPlotDataReal*)theDataSet->mDataY;

  if (!theYData || !theXData || theXData->IsString())
    return;

  for (int i=0; i<count; i++)
  {
    theXData->InsertItem(inSampleIndex+i, inX[i]);
    theYData->InsertItem(inSampleIndex+i, inY[i]);
  }
}

int IupPlotEnd(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return -1;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return -1;

  iupPlotDataBase* inXData = (iupPlotDataBase*)iupAttribGet(ih, "_IUP_PLOT_XDATA");
  iupPlotDataBase* inYData = (iupPlotDataBase*)iupAttribGet(ih, "_IUP_PLOT_YDATA");
  if (!inYData || !inXData)
    return -1;

  /* add to plot */
  ih->data->current_plot->AddDataSet(inXData, inYData);

  iupAttribSet(ih, "_IUP_PLOT_XDATA", NULL);
  iupAttribSet(ih, "_IUP_PLOT_YDATA", NULL);

  ih->data->current_plot->mRedraw = 1;
  return ih->data->current_plot->mCurrentDataSet;
}

void IupPlotTransform(Ihandle* ih, double x, double y, int *ix, int *iy)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  if (ix) *ix = iupRound(ih->data->current_plot->mAxisX.mTrafo->Transform(x));
  if (iy) *iy = iupRound(ih->data->current_plot->mAxisY.mTrafo->Transform(y));
}

void IupPlotTransformTo(Ihandle* ih, int x, int y, double *rx, double *ry)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  if (rx) *rx = ih->data->current_plot->mAxisX.mTrafo->TransformBack((double)x);
  if (ry) *ry = ih->data->current_plot->mAxisY.mTrafo->TransformBack((double)y);
}

void IupPlotPaintTo(Ihandle* ih, void* _cnv)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "plot"))
    return;

  if (!_cnv)
    return;

  cdCanvas *old_cd_canvas  = ih->data->cd_canvas;
  ih->data->cd_canvas = (cdCanvas*)_cnv;

  iPlotUpdateViewports(ih);

  int old_current = ih->data->current_plot_index;
  for (int p = 0; p<ih->data->plot_list_count; p++)
  {
    iPlotSetPlotCurrent(ih, p);
    ih->data->current_plot->Render(ih->data->cd_canvas);  
  }
  iPlotSetPlotCurrent(ih, old_current);

  ih->data->cd_canvas = old_cd_canvas;

  iPlotUpdateViewports(ih);
}


/************************************************************************************/


static int iPlotGetCDFontStyle(const char* value)
{
  if (!value)
    return -1;
  if (iupStrEqualNoCase(value, "PLAIN") || value[0] == 0)
    return CD_PLAIN;
  if (iupStrEqualNoCase(value, "BOLD"))
    return CD_BOLD;
  if (iupStrEqualNoCase(value, "ITALIC"))
    return CD_ITALIC;
  if (iupStrEqualNoCase(value, "BOLDITALIC") || 
      iupStrEqualNoCase(value, "BOLD ITALIC") ||
      iupStrEqualNoCase(value, "ITALIC BOLD"))
    return CD_BOLD_ITALIC;
  return -1;
}

static char* iPlotGetPlotFontSize(int size)
{
  if (size)
    return iupStrReturnInt(size);
  else
    return NULL;
}

static char* iPlotGetPlotFontStyle(int style)
{
  if (style >= CD_PLAIN && style <= CD_BOLD_ITALIC)
  {
    const char* style_str[4] = {"Plain", "Bold", "Italic", "Bold Italic"};
    return (char*)style_str[style];
  }
  else
    return NULL;
}

static char* iPlotGetPlotPenStyle(int style)
{
  if (style >= CD_CONTINUOUS && style <= CD_DASH_DOT_DOT)
  {
    const char* style_str[5] = {"CONTINUOUS", "DASHED", "DOTTED", "DASH_DOT", "DASH_DOT_DOT"};
    return (char*)style_str[style];
  }
  else
    return NULL;
}

static int iPlotGetCDPenStyle(const char* value)
{
  if (!value || iupStrEqualNoCase(value, "CONTINUOUS"))
    return CD_CONTINUOUS;
  else if (iupStrEqualNoCase(value, "DASHED"))
    return CD_DASHED;
  else if (iupStrEqualNoCase(value, "DOTTED"))
    return CD_DOTTED;
  else if (iupStrEqualNoCase(value, "DASH_DOT"))
    return CD_DASH_DOT;
  else if (iupStrEqualNoCase(value, "DASH_DOT_DOT"))
    return CD_DASH_DOT_DOT;
  else
    return CD_CONTINUOUS;
}
 
static char* iPlotGetPlotMarkStyle(int style)
{
  if (style >= CD_PLUS && style <= CD_HOLLOW_DIAMOND)
  {
    const char* style_str[9] = {"PLUS", "STAR", "CIRCLE", "X", "BOX", "DIAMOND", "HOLLOW_CIRCLE", "HOLLOW_BOX", "HOLLOW_DIAMOND"};
    return (char*)style_str[style];
  }
  else
    return NULL;
}

static int iPlotGetCDMarkStyle(const char* value)
{
  if (!value || iupStrEqualNoCase(value, "PLUS"))
    return CD_PLUS;
  else if (iupStrEqualNoCase(value, "STAR"))
    return CD_STAR;
  else if (iupStrEqualNoCase(value, "CIRCLE"))
    return CD_CIRCLE;
  else if (iupStrEqualNoCase(value, "X"))
    return CD_X;
  else if (iupStrEqualNoCase(value, "BOX"))
    return CD_BOX;
  else if (iupStrEqualNoCase(value, "DIAMOND"))
    return CD_DIAMOND;
  else if (iupStrEqualNoCase(value, "HOLLOW_CIRCLE"))
    return CD_HOLLOW_CIRCLE;
  else if (iupStrEqualNoCase(value, "HOLLOW_BOX"))
    return CD_HOLLOW_BOX;
  else if (iupStrEqualNoCase(value, "HOLLOW_DIAMOND"))
    return CD_HOLLOW_DIAMOND;
  else
    return CD_PLUS;
}


/************************************************************************************/


static int iPlotSetAntialiasAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    cdCanvasSetAttribute(ih->data->cd_canvas, "ANTIALIAS", "1");
  else
    cdCanvasSetAttribute(ih->data->cd_canvas, "ANTIALIAS", "0");
  return 0;
}

static char* iPlotGetAntialiasAttrib(Ihandle* ih)
{
  return cdCanvasGetAttribute(ih->data->cd_canvas, "ANTIALIAS");
}

static int iPlotSetRedrawAttrib(Ihandle* ih, const char* value)
{
  int flush = 1, 
    current = 0;

  if (iupStrEqualNoCase(value, "NOFLUSH"))
    flush = 0;
  else if (iupStrEqualNoCase(value, "CURRENT"))
  {
    flush = 0;
    current = 1;
  }

  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
    IupGLMakeCurrent(ih);

  if (current)
  {
    ih->data->current_plot->Render(ih->data->cd_canvas);  
  }
  else
  {
    int old_current = ih->data->current_plot_index;
    for (int p = 0; p < ih->data->plot_list_count; p++)
    {
      iPlotSetPlotCurrent(ih, p);
      ih->data->current_plot->Render(ih->data->cd_canvas);  
    }
    iPlotSetPlotCurrent(ih, old_current);
  }

  if (ih->data->plot_list_count == 1 &&
      ih->data->current_plot->mDataSetListCount == 0)
  {
    cdCanvasClear(ih->data->cd_canvas);
    flush = 1;
  }

  // Do the flush once
  if (flush)
    iPlotFlush(ih, ih->data->cd_canvas);

  (void)value;  /* not used */
  return 0;
}

static char* iPlotGetCountAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot->mDataSetListCount);
}

static int iPlotSetLegendAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->current_plot->mLegend.mShow = true;
  else 
    ih->data->current_plot->mLegend.mShow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetLegendAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean (ih->data->current_plot->mLegend.mShow); 
}

static int iPlotSetLegendPosAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "TOPLEFT"))
    ih->data->current_plot->mLegend.mPosition = IUP_PLOT_TOPLEFT;
  else if (iupStrEqualNoCase(value, "BOTTOMLEFT"))
    ih->data->current_plot->mLegend.mPosition = IUP_PLOT_BOTTOMLEFT;
  else if (iupStrEqualNoCase(value, "BOTTOMRIGHT"))
    ih->data->current_plot->mLegend.mPosition = IUP_PLOT_BOTTOMRIGHT;
  else if (iupStrEqualNoCase(value, "BOTTOMCENTER"))
    ih->data->current_plot->mLegend.mPosition = IUP_PLOT_BOTTOMCENTER;
  else
    ih->data->current_plot->mLegend.mPosition = IUP_PLOT_TOPRIGHT;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetLegendPosAttrib(Ihandle* ih)
{
  const char* legendpos_str[4] = {"TOPLEFT", "TOPRIGHT", "BOTTOMLEFT", "BOTTOMRIGHT"};
  return (char*)legendpos_str[ih->data->current_plot->mLegend.mPosition];
}

static int iPlotSetBackColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    ih->data->current_plot->mBackColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetBackColorAttrib(Ihandle* ih)
{
  return iupStrReturnColor(ih->data->current_plot->mBackColor);
}

static int iPlotSetBGColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      ih->data->plot_list[p]->mLegend.mBoxBackColor = color;
      ih->data->plot_list[p]->mBackColor = color;
      ih->data->plot_list[p]->mRedraw = 1;
    }
  }
  return 1;
}

static int iPlotSetFGColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      ih->data->plot_list[p]->mBox.mColor = color;
      ih->data->plot_list[p]->mAxisX.mColor = color;
      ih->data->plot_list[p]->mAxisY.mColor = color;
      ih->data->plot_list[p]->mLegend.mBoxColor = color;
      ih->data->plot_list[p]->mTitle.mColor = color;
      ih->data->plot_list[p]->mRedraw = 1;
    }
  }
  return 1;
}

static int iPlotSetStandardFontAttrib(Ihandle* ih, const char* value)
{
  iupdrvSetStandardFontAttrib(ih, value);

  int size = 0;
  int is_bold = 0,
    is_italic = 0,
    is_underline = 0,
    is_strikeout = 0;
  char typeface[1024];

  if (!value)
    return 0;

  if (!iupGetFontInfo(value, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return 0;

  int style = CD_PLAIN;
  if (is_bold) style |= CD_BOLD;
  if (is_italic) style |= CD_ITALIC;
  if (is_underline) style |= CD_UNDERLINE;
  if (is_strikeout) style |= CD_STRIKEOUT;

  for (int p = 0; p < ih->data->plot_list_count; p++)
  {
    ih->data->plot_list[p]->mDefaultFontSize = size;
    ih->data->plot_list[p]->mDefaultFontStyle = style;
    ih->data->plot_list[p]->mRedraw = 1;
  }

  return 1;
}

static int iPlotSetTitleColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    ih->data->current_plot->mTitle.mColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetTitleColorAttrib(Ihandle* ih)
{
  return iupStrReturnColor(ih->data->current_plot->mTitle.mColor);
}

static int iPlotSetTitleAttrib(Ihandle* ih, const char* value)
{
  ih->data->current_plot->mTitle.SetText(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetTitleAttrib(Ihandle* ih)
{
  return iupStrReturnStr(ih->data->current_plot->mTitle.GetText());
}

static int iPlotSetTitleFontSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mTitle.mFontSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetTitleFontSizeAttrib(Ihandle* ih)
{
  return iPlotGetPlotFontSize(ih->data->current_plot->mTitle.mFontSize);
}

static int iPlotSetTitleFontStyleAttrib(Ihandle* ih, const char* value)
{
  int style = iPlotGetCDFontStyle(value);
  if (style != -1)
  {
    ih->data->current_plot->mTitle.mFontStyle = style;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetTitleFontStyleAttrib(Ihandle* ih)
{
  return iPlotGetPlotFontStyle(ih->data->current_plot->mTitle.mFontStyle);
}

static int iPlotSetLegendFontSizeAttrib(Ihandle* ih, const char* value)
{
  int xx;
  if (!iupStrToInt(value, &xx))
    return 0;

  ih->data->current_plot->mLegend.mFontSize = xx;
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetLegendFontStyleAttrib(Ihandle* ih, const char* value)
{
  int style = iPlotGetCDFontStyle(value);
  if (style == -1)
    return 0;

  ih->data->current_plot->mLegend.mFontStyle = style;
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetLegendFontStyleAttrib(Ihandle* ih)
{
  return iPlotGetPlotFontStyle(ih->data->current_plot->mLegend.mFontStyle);
}

static char* iPlotGetLegendFontSizeAttrib(Ihandle* ih)
{
  return iPlotGetPlotFontSize(ih->data->current_plot->mLegend.mFontSize);
}

static int iPlotSetMarginLeftAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "AUTO"))
    ih->data->current_plot->mMarginAuto.mLeft = 1;

  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mMarginAuto.mLeft = 0;
    ih->data->current_plot->mMargin.mLeft = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetMarginRightAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "AUTO"))
    ih->data->current_plot->mMarginAuto.mRight = 1;

  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mMarginAuto.mRight = 0;
    ih->data->current_plot->mMargin.mRight = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetMarginTopAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "AUTO"))
    ih->data->current_plot->mMarginAuto.mTop = 1;

  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mMarginAuto.mTop = 0;
    ih->data->current_plot->mMargin.mTop = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetMarginBottomAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "AUTO"))
    ih->data->current_plot->mMarginAuto.mBottom = 1;

  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mMarginAuto.mBottom = 0;
    ih->data->current_plot->mMargin.mBottom = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetMarginLeftAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot->mMargin.mLeft);
}

static char* iPlotGetMarginRightAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot->mMargin.mRight);
}

static char* iPlotGetMarginTopAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot->mMargin.mTop);
}

static char* iPlotGetMarginBottomAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot->mMargin.mBottom);
}

static int iPlotSetGridColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    ih->data->current_plot->mGrid.mColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetGridColorAttrib(Ihandle* ih)
{
  return iupStrReturnColor(ih->data->current_plot->mGrid.mColor);
}

static int iPlotSetGridLineStyleAttrib(Ihandle* ih, const char* value)
{
  ih->data->current_plot->mGrid.mLineStyle = iPlotGetCDPenStyle(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetGridLineStyleAttrib(Ihandle* ih)
{
  return iPlotGetPlotPenStyle(ih->data->current_plot->mGrid.mLineStyle);
}

static int iPlotSetGridLineWidthAttrib(Ihandle* ih, const char* value)
{
  int ii;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
      return 0;

  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mGrid.mLineWidth = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetGridLineWidthAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
      return NULL;

  return iupStrReturnInt(ih->data->current_plot->mGrid.mLineWidth);
}

static int iPlotSetLegendBoxLineStyleAttrib(Ihandle* ih, const char* value)
{
  ih->data->current_plot->mLegend.mBoxLineStyle = iPlotGetCDPenStyle(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetLegendBoxLineStyleAttrib(Ihandle* ih)
{
  return iPlotGetPlotPenStyle(ih->data->current_plot->mLegend.mBoxLineStyle);
}

static int iPlotSetBoxLineStyleAttrib(Ihandle* ih, const char* value)
{
  ih->data->current_plot->mBox.mLineStyle = iPlotGetCDPenStyle(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetBoxColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    ih->data->current_plot->mBox.mColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetBoxColorAttrib(Ihandle* ih)
{
  return iupStrReturnColor(ih->data->current_plot->mBox.mColor);
}

static int iPlotSetBoxAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->current_plot->mBox.mShow = true;
  else
    ih->data->current_plot->mBox.mShow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetBoxAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->current_plot->mBox.mShow);
}

static int iPlotSetLegendBoxColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    ih->data->current_plot->mLegend.mBoxColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetLegendBoxBackColorAttrib(Ihandle* ih)
{
  return iupStrReturnColor(ih->data->current_plot->mLegend.mBoxBackColor);
}

static int iPlotSetLegendBoxBackColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    ih->data->current_plot->mLegend.mBoxBackColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetLegendBoxColorAttrib(Ihandle* ih)
{
  return iupStrReturnColor(ih->data->current_plot->mLegend.mBoxColor);
}

static int iPlotSetLegendBoxLineWidthAttrib(Ihandle* ih, const char* value)
{
  int ii;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
      return 0;

  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mLegend.mBoxLineWidth = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetLegendBoxLineWidthAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
      return NULL;

  return iupStrReturnInt(ih->data->current_plot->mLegend.mBoxLineWidth);
}

static char* iPlotGetBoxLineStyleAttrib(Ihandle* ih)
{
  return iPlotGetPlotPenStyle(ih->data->current_plot->mBox.mLineStyle);
}

static int iPlotSetBoxLineWidthAttrib(Ihandle* ih, const char* value)
{
  int ii;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
      return 0;

  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mBox.mLineWidth = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetBoxLineWidthAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
      return NULL;

  return iupStrReturnInt(ih->data->current_plot->mBox.mLineWidth);
}

static int iPlotSetGridAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "VERTICAL"))  /* vertical grid - X axis  */
  {
    ih->data->current_plot->mGrid.mXGridOn = true;
    ih->data->current_plot->mGrid.mYGridOn = false;
  }
  else if (iupStrEqualNoCase(value, "HORIZONTAL")) /* horizontal grid - Y axis */
  {
    ih->data->current_plot->mGrid.mYGridOn = true;
    ih->data->current_plot->mGrid.mXGridOn = false;
  }
  else if (iupStrEqualNoCase(value, "YES"))
  {
    ih->data->current_plot->mGrid.mXGridOn = true;
    ih->data->current_plot->mGrid.mYGridOn = true;
  }
  else
  {
    ih->data->current_plot->mGrid.mYGridOn = false;
    ih->data->current_plot->mGrid.mXGridOn = false;
  }

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetGridAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mGrid.mXGridOn && ih->data->current_plot->mGrid.mYGridOn)
    return "YES";
  else if (ih->data->current_plot->mGrid.mYGridOn)
    return "HORIZONTAL";
  else if (ih->data->current_plot->mGrid.mXGridOn)
    return "VERTICAL";
  else
    return "NO";
}

static int iPlotSetCurrentAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    int imax = ih->data->current_plot->mDataSetListCount;
    ih->data->current_plot->mCurrentDataSet = ( (ii>=0) && (ii<imax) ? ii : -1);
    ih->data->current_plot->mRedraw = 1;
  }
  else
  {
    ii = ih->data->current_plot->FindDataset(value);
    if (ii != -1)
    {
      int imax = ih->data->current_plot->mDataSetListCount;
      ih->data->current_plot->mCurrentDataSet = ( (ii>=0) && (ii<imax) ? ii : -1);
      ih->data->current_plot->mRedraw = 1;
    }
  }
  return 0;
}

static char* iPlotGetCurrentAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot->mCurrentDataSet);
}

static int iPlotSetPlotCurrentAttrib(Ihandle* ih, const char* value)
{
  int i;
  if (iupStrToInt(value, &i))
  {
    if (i>=0 && i<ih->data->plot_list_count)
      iPlotSetPlotCurrent(ih, i);
  }
  else
  {
    for (i=0; i<ih->data->plot_list_count; i++)
    {
      const char* title = ih->data->plot_list[i]->mTitle.GetText();
      if (iupStrEqual(title, value))
      {
        iPlotSetPlotCurrent(ih, i);
        return 0;
      }
    }
  }
  return 0;
}

static char* iPlotGetPlotCurrentAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->current_plot_index);
}

static char* iPlotGetPlotCountAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->plot_list_count);
}

static int iPlotSetPlotCountAttrib(Ihandle* ih, const char* value)
{
  int count;
  if (iupStrToInt(value, &count))
  {
    if (count>0 && count<IUP_PLOT_MAX_PLOTS)
    {
      if (count != ih->data->plot_list_count)
      {
        if (count < ih->data->plot_list_count)
        {
          // Remove at the end
          if (ih->data->current_plot_index >= count)
            iPlotSetPlotCurrent(ih, count-1);

          for (int i=count; i<ih->data->plot_list_count; i++)
          {
            delete ih->data->plot_list[i];
            ih->data->plot_list[i] = NULL;
          }
        }
        else
        {
          // Add at the end
          for (int i=ih->data->plot_list_count; i<count; i++)
            ih->data->plot_list[i] = new iupPlot(ih);
        }
      }

      ih->data->plot_list_count = count;

      iPlotUpdateViewports(ih);
    }
  }
  return 0;
}

static void iPlotPlotInsert(Ihandle* ih, int p)
{
  for (int i=ih->data->plot_list_count; i>p; i--)
    ih->data->plot_list[i] = ih->data->plot_list[i-1];

  ih->data->plot_list[p] = new iupPlot(ih);

  ih->data->plot_list_count++;

  iPlotUpdateViewports(ih);
}

static int iPlotSetPlotInsertAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    if (ih->data->plot_list_count<IUP_PLOT_MAX_PLOTS)
    {
      // Insert at the end (append)
      iPlotPlotInsert(ih, ih->data->plot_list_count);
      iPlotSetPlotCurrent(ih, ih->data->plot_list_count-1);
    }
  }
  else
  {
    // Insert before reference
    int i;
    if (iupStrToInt(value, &i))
    {
      if (i>=0 && i<ih->data->plot_list_count)
      {
        iPlotPlotInsert(ih, i);
        iPlotSetPlotCurrent(ih, ih->data->plot_list_count-1);
      }
    }
    else
    {
      for (i=0; i<ih->data->plot_list_count; i++)
      {
        const char* title = ih->data->plot_list[i]->mTitle.GetText();
        if (iupStrEqual(title, value))
        {
          iPlotPlotInsert(ih, i);
          iPlotSetPlotCurrent(ih, ih->data->plot_list_count-1);
          return 0;
        }
      }
    }
  }
  return 0;
}

static char* iPlotGetPlotNumColAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->numcol);
}

static int iPlotSetPlotNumColAttrib(Ihandle* ih, const char* value)
{
  int numcol;
  if (iupStrToInt(value, &numcol))
  {
    if (numcol > 0)
    {
      ih->data->numcol = numcol;

      iPlotUpdateViewports(ih);
    }
  }
  return 0;
}

static void iPlotPlotRemove(Ihandle* ih, int p)
{
  if (ih->data->current_plot_index == ih->data->plot_list_count-1)
    ih->data->current_plot_index--;

  delete ih->data->plot_list[p];

  for (int i=p; i<ih->data->plot_list_count; i++)
    ih->data->plot_list[i] = ih->data->plot_list[i+1];

  ih->data->plot_list[ih->data->plot_list_count-1] = NULL;

  ih->data->plot_list_count--;

  iPlotSetPlotCurrent(ih, ih->data->current_plot_index);

  iPlotUpdateViewports(ih);
}

static int iPlotSetPlotRemoveAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->plot_list_count == 1)
    return 0;

  if (!value)
  {
    iPlotPlotRemove(ih, ih->data->current_plot_index);
    return 0;
  }

  int i;
  if (iupStrToInt(value, &i))
  {
    if (i>=0 && i<ih->data->plot_list_count)
      iPlotPlotRemove(ih, i);
  }
  else
  {
    for (i=0; i<ih->data->plot_list_count; i++)
    {
      const char* title = ih->data->plot_list[i]->mTitle.GetText();
      if (iupStrEqual(title, value))
      {
        iPlotPlotRemove(ih, i);
        return 0;
      }
    }
  }
  return 0;
}

static int iPlotSetSyncViewAttrib(Ihandle* ih, const char* value)
{
  ih->data->sync_view = iupStrBoolean(value);
  return 0;
}

static char* iPlotGetSyncViewAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->sync_view);
}
                          
static int iPlotSetGraphicsModeAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle)  // Can be set only before map
    return 0;

  if (iupStrEqualNoCase(value, "OPENGL"))
  {
    ih->data->graphics_mode = IUP_PLOT_OPENGL;
    IupSetAttribute(ih, "BUFFER", "DOUBLE");
  }
  else if (iupStrEqualNoCase(value, "IMAGERGB"))
    ih->data->graphics_mode = IUP_PLOT_IMAGERGB;
  else
    ih->data->graphics_mode = IUP_PLOT_NATIVE;

  return 0;
}

static char* iPlotGetGraphicsModeAttrib(Ihandle* ih)
{
  char* graphics_mode_str[] = { "NATIVE", "IMAGERGB", "OPENGL" };
  return graphics_mode_str[ih->data->graphics_mode];
}


static char* iPlotGetCanvasAttrib(Ihandle* ih)
{
  return (char*)(ih->data->cd_canvas);
}

static void iPlotCheckCurrentDataSet(Ihandle* ih)
{
  int count = ih->data->current_plot->mDataSetListCount;
  if (ih->data->current_plot->mCurrentDataSet == count)
    ih->data->current_plot->mCurrentDataSet--;
}

static int iPlotSetRemoveAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->data->current_plot->RemoveDataset(ih->data->current_plot->mCurrentDataSet);
    ih->data->current_plot->mRedraw = 1;
    iPlotCheckCurrentDataSet(ih);
    return 0;
  }

  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->RemoveDataset(ii);
    ih->data->current_plot->mRedraw = 1;
    iPlotCheckCurrentDataSet(ih);
  }
  else
  {
    ii = ih->data->current_plot->FindDataset(value);
    if (ii != -1)
    {
      ih->data->current_plot->RemoveDataset(ii);
      ih->data->current_plot->mRedraw = 1;
      iPlotCheckCurrentDataSet(ih);
    }
  }
  return 0;
}

static int iPlotSetClearAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  ih->data->current_plot->RemoveAllDatasets();
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetDSLineStyleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;
  
  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  dataset->mLineStyle = iPlotGetCDPenStyle(value);

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetDSLineStyleAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];

  return iPlotGetPlotPenStyle(dataset->mLineStyle);
}

static int iPlotSetDSLineWidthAttrib(Ihandle* ih, const char* value)
{
  int ii;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;

  if (iupStrToInt(value, &ii))
  {
    iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
    dataset->mLineWidth = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetDSLineWidthAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  return iupStrReturnInt(dataset->mLineWidth);
}

static int iPlotSetDSMarkStyleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;
  
  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  dataset->mMarkStyle = iPlotGetCDMarkStyle(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetDSMarkStyleAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];

  return iPlotGetPlotMarkStyle(dataset->mMarkStyle);
}

static int iPlotSetDSMarkSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;
  
  if (iupStrToInt(value, &ii))
  {
    iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
    dataset->mMarkSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetDSMarkSizeAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  return iupStrReturnInt(dataset->mMarkSize);
}

static int iPlotSetDSNameAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;

  if (!value)  // can not be NULL
    return 0;
  
  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  dataset->SetName(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetDSNameAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  return iupStrReturnStr(dataset->GetName());
}

static int iPlotSetDSColorAttrib(Ihandle* ih, const char* value)
{
  long color;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;

  if (iupStrToColor(value, &color))
  {
    iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
    dataset->mColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetDSColorAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  return iupStrReturnColor(dataset->mColor);
}

static int iPlotSetDSModeAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;
  
  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];

  ih->data->current_plot->mAxisX.mDiscrete = false;

  if(iupStrEqualNoCase(value, "BAR"))
  {
    dataset->mMode = IUP_PLOT_BAR;
    ih->data->current_plot->mAxisX.mDiscrete = true;
  }
  else if (iupStrEqualNoCase(value, "AREA"))
    dataset->mMode = IUP_PLOT_AREA;
  else if (iupStrEqualNoCase(value, "MARK"))
    dataset->mMode = IUP_PLOT_MARK;
  else if (iupStrEqualNoCase(value, "MARKLINE"))
    dataset->mMode = IUP_PLOT_MARKLINE;
  else  /* LINE */
    dataset->mMode = IUP_PLOT_LINE;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetDSModeAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  const char* mode_str[] = { "LINE", "MARK", "MARKLINE", "AREA", "BAR" };

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  return (char*)mode_str[dataset->mMode];
}

static int iPlotSetDSRemoveAttrib(Ihandle* ih, const char* value)
{
  int ii;

  if (ih->data->current_plot->mCurrentDataSet <  0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return 0;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];

  if (iupStrToInt(value, &ii))
  {
    dataset->mDataX->RemoveItem(ii);
    dataset->mDataY->RemoveItem(ii);
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetDSCountAttrib(Ihandle* ih)
{
  if (ih->data->current_plot->mCurrentDataSet < 0 ||
      ih->data->current_plot->mCurrentDataSet >= ih->data->current_plot->mDataSetListCount)
    return NULL;

  iupPlotDataSet* dataset = ih->data->current_plot->mDataSetList[ih->data->current_plot->mCurrentDataSet];
  return iupStrReturnInt(dataset->mDataX->GetCount());
}

/* ========== */
/* axis props */
/* ========== */

static int iPlotSetAxisXLabelAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  axis->SetLabel(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYLabelAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  axis->SetLabel(value);
  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXLabelAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnStr(axis->GetLabel());
}

static char* iPlotGetAxisYLabelAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnStr(axis->GetLabel());
}

static int iPlotSetAxisXAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mShow = true;
  else
    axis->mShow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mShow = true;
  else
    axis->mShow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean(axis->mShow);
}

static char* iPlotGetAxisYAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean(axis->mShow);
}

static int iPlotSetAxisXLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mLabelCentered = true;
  else 
   axis->mLabelCentered = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mLabelCentered = true;
  else 
   axis->mLabelCentered = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXLabelCenteredAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean (axis->mLabelCentered); 
}

static char* iPlotGetAxisYLabelCenteredAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mLabelCentered); 
}

static int iPlotSetAxisXColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYColorAttrib(Ihandle* ih, const char* value)
{
  long color;
  if (iupStrToColor(value, &color))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mColor = color;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXColorAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnColor(axis->mColor);
}

static char* iPlotGetAxisYColorAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnColor(axis->mColor);
}

static int iPlotSetAxisXLineWidthAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mAxisX.mLineWidth = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXLineWidthAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnInt(axis->mLineWidth);
}

static int iPlotSetAxisYLineWidthAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    ih->data->current_plot->mAxisY.mLineWidth = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisYLineWidthAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnInt(axis->mLineWidth);
}

static int iPlotSetAxisXAutoMinAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  iPlotResetZoom(ih, 0);

  if (iupStrBoolean(value))
    axis->mAutoScaleMin = true;
  else 
    axis->mAutoScaleMin = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYAutoMinAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  iPlotResetZoom(ih, 0);

  if (iupStrBoolean(value))
    axis->mAutoScaleMin = true;
  else 
    axis->mAutoScaleMin = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXAutoMinAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean (axis->mAutoScaleMin); 
}

static char* iPlotGetAxisYAutoMinAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mAutoScaleMin); 
}

static int iPlotSetAxisXAutoMaxAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  iPlotResetZoom(ih, 0);

  if (iupStrBoolean(value))
    axis->mAutoScaleMax = true;
  else 
    axis->mAutoScaleMax = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYAutoMaxAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  iPlotResetZoom(ih, 0);

  if (iupStrBoolean(value))
    axis->mAutoScaleMax = true;
  else 
    axis->mAutoScaleMax = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXAutoMaxAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean (axis->mAutoScaleMax); 
}

static char* iPlotGetAxisYAutoMaxAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mAutoScaleMax); 
}

static int iPlotSetAxisXMinAttrib(Ihandle* ih, const char* value)
{
  double xx;
  if (iupStrToDouble(value, &xx))
  {
    iPlotResetZoom(ih, 0);

    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mMin = xx;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYMinAttrib(Ihandle* ih, const char* value)
{
  double xx;
  if (iupStrToDouble(value, &xx))
  {
    iPlotResetZoom(ih, 0);

    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mMin = xx;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXMinAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnDouble(axis->mMin);
}

static char* iPlotGetAxisYMinAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnDouble(axis->mMin);
}

static int iPlotSetAxisXMaxAttrib(Ihandle* ih, const char* value)
{
  double xx;
  if (iupStrToDouble(value, &xx))
  {
    iPlotResetZoom(ih, 0);

    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mMax = xx;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYMaxAttrib(Ihandle* ih, const char* value)
{
  double xx;
  if (iupStrToDouble(value, &xx))
  {
    iPlotResetZoom(ih, 0);

    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mMax = xx;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXMaxAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnDouble(axis->mMax);
}

static char* iPlotGetAxisYMaxAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnDouble(axis->mMax);
}

static int iPlotSetAxisXReverseAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mReverse = true;
  else
    axis->mReverse = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYReverseAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mReverse = true;
  else 
    axis->mReverse = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXReverseAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnBoolean(axis->mReverse);
}

static char* iPlotGetAxisYReverseAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnBoolean(axis->mReverse);
}

static int iPlotSetAxisXCrossOriginAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mCrossOrigin = true;
  else 
    axis->mCrossOrigin = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYCrossOriginAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mCrossOrigin = true;
  else 
    axis->mCrossOrigin = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXCrossOriginAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean (axis->mCrossOrigin); 
}

static char* iPlotGetAxisYCrossOriginAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mCrossOrigin); 
}

static int iPlotSetAxisXScaleAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if(iupStrEqualNoCase(value, "LIN"))
  {
    axis->mLogScale = false;
  }
  else if(iupStrEqualNoCase(value, "LOG10"))
  {
    axis->mLogScale = true;
    axis->mLogBase  = 10.0;
  }
  else if(iupStrEqualNoCase(value, "LOG2"))
  {
    axis->mLogScale = true;
    axis->mLogBase  = 2.0;
  }
  else
  {
    axis->mLogScale = true;
    axis->mLogBase  = (double)M_E;
  }

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYScaleAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if(iupStrEqualNoCase(value, "LIN"))
  {
    axis->mLogScale = false;
  }
  else if(iupStrEqualNoCase(value, "LOG10"))
  {
    axis->mLogScale = true;
    axis->mLogBase  = 10.0;
  }
  else if(iupStrEqualNoCase(value, "LOG2"))
  {
    axis->mLogScale = true;
    axis->mLogBase  = 2.0;
  }
  else
  {
    axis->mLogScale = true;
    axis->mLogBase  = (double)M_E;
  }

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXScaleAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (axis->mLogScale)
  {
    if (axis->mLogBase == 10.0)
      return "LOG10";
    else if (axis->mLogBase == 2.0)
      return "LOG2";
    else
      return "LOGN";
  }
  else
    return "LIN";
}

static char* iPlotGetAxisYScaleAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (axis->mLogScale)
  {
    if (axis->mLogBase == 10.0)
      return "LOG10";
    else if (axis->mLogBase == 2.0)
      return "LOG2";
    else
      return "LOGN";
  }
  else
    return "LIN";
}

static int iPlotSetAxisXFontSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mFontSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYFontSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mFontSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXFontSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iPlotGetPlotFontSize(axis->mFontSize);
}

static char* iPlotGetAxisYFontSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iPlotGetPlotFontSize(axis->mFontSize);
}

static int iPlotSetAxisXFontStyleAttrib(Ihandle* ih, const char* value)
{
  int style = iPlotGetCDFontStyle(value);
  if (style != -1)
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mFontStyle = style;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYFontStyleAttrib(Ihandle* ih, const char* value)
{
  int style = iPlotGetCDFontStyle(value);
  if (style != -1)
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mFontStyle = style;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXFontStyleAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iPlotGetPlotFontStyle(axis->mFontStyle);
}

static char* iPlotGetAxisYFontStyleAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iPlotGetPlotFontStyle(axis->mFontStyle);
}

static int iPlotSetAxisXArrowAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mShowArrow = true;
  else
    axis->mShowArrow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXArrowAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean(axis->mShowArrow);
}

static int iPlotSetAxisYArrowAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mShowArrow = true;
  else
    axis->mShowArrow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisYArrowAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean(axis->mShowArrow);
}

static int iPlotSetAxisXAutoTickSizeAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mTick.mAutoSize = true;
  else 
    axis->mTick.mAutoSize = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYAutoTickSizeAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mTick.mAutoSize = true;
  else 
    axis->mTick.mAutoSize = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXAutoTickSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean (axis->mTick.mAutoSize); 
}

static char* iPlotGetAxisYAutoTickSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mTick.mAutoSize); 
}

static int iPlotSetAxisXTickSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mTick.mMinorSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYTickSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mTick.mMinorSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXTickSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnInt(axis->mTick.mMinorSize);
}

static char* iPlotGetAxisYTickSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnInt(axis->mTick.mMinorSize);
}

static int iPlotSetAxisXTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mTick.mMajorSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mTick.mMajorSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXTickMajorSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnInt(axis->mTick.mMajorSize);
}

static char* iPlotGetAxisYTickMajorSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnInt(axis->mTick.mMajorSize);
}

static int iPlotSetAxisXTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mTick.mFontSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mTick.mFontSize = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXTickFontSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iPlotGetPlotFontSize(axis->mTick.mFontSize);
}

static char* iPlotGetAxisYTickFontSizeAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iPlotGetPlotFontSize(axis->mTick.mFontSize);
}

static int iPlotSetAxisXTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  int style = iPlotGetCDFontStyle(value);
  if (style != -1)
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mTick.mFontStyle = style;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  int style = iPlotGetCDFontStyle(value);
  if (style != -1)
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mTick.mFontStyle = style;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXTickFontStyleAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iPlotGetPlotFontSize(axis->mTick.mFontStyle);
}

static char* iPlotGetAxisYTickFontStyleAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iPlotGetPlotFontSize(axis->mTick.mFontStyle);
}

static int iPlotSetAxisXTickFormatAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (value && value[0]!=0)
    strcpy(axis->mTick.mFormatString, value);
  else
    strcpy(axis->mTick.mFormatString, "%.0f");

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYTickFormatAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (value && value[0]!=0)
    strcpy(axis->mTick.mFormatString, value);
  else
    strcpy(axis->mTick.mFormatString, "%.0f");

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXTickFormatAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnStr(axis->mTick.mFormatString);
}

static char* iPlotGetAxisYTickFormatAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnStr(axis->mTick.mFormatString);
}

static int iPlotSetAxisXTickAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mTick.mShow = true;
  else 
    axis->mTick.mShow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXTickAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean(axis->mTick.mShow);
}

static int iPlotSetAxisYTickAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mTick.mShow = true;
  else 
    axis->mTick.mShow = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisYTickAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mTick.mShow); 
}

static int iPlotSetAxisXTickNumberAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mTick.mShowNumber = true;
  else
    axis->mTick.mShowNumber = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYTickNumberAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mTick.mShowNumber = true;
  else
    axis->mTick.mShowNumber = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXTickNumberAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean(axis->mTick.mShowNumber);
}

static char* iPlotGetAxisYTickNumberAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean(axis->mTick.mShowNumber);
}

static int iPlotSetAxisXTickRotateNumberAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mTick.mRotateNumber = true;
  else
    axis->mTick.mRotateNumber = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYTickRotateNumberAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mTick.mRotateNumber = true;
  else
    axis->mTick.mRotateNumber = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXTickRotateNumberAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  return iupStrReturnBoolean(axis->mTick.mRotateNumber);
}

static char* iPlotGetAxisYTickRotateNumberAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean(axis->mTick.mRotateNumber);
}

static int iPlotSetAxisXTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  double xx;
  if (iupStrToDouble(value, &xx))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mTick.mMajorSpan = xx;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  double xx;
  if (iupStrToDouble(value, &xx))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mTick.mMajorSpan = xx;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXTickMajorSpanAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnDouble(axis->mTick.mMajorSpan);
}

static char* iPlotGetAxisYTickMajorSpanAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnDouble(axis->mTick.mMajorSpan);
}

static int iPlotSetAxisXTickDivisionAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
    axis->mTick.mMinorDivision = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static int iPlotSetAxisYTickDivisionAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
    axis->mTick.mMinorDivision = ii;
    ih->data->current_plot->mRedraw = 1;
  }
  return 0;
}

static char* iPlotGetAxisXTickDivisionAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnInt(axis->mTick.mMinorDivision);
}

static char* iPlotGetAxisYTickDivisionAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;
  return iupStrReturnInt(axis->mTick.mMinorDivision);
}

static int iPlotSetAxisXAutoTickAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;

  if (iupStrBoolean(value))
    axis->mTick.mAutoSpacing = true;
  else 
    axis->mTick.mAutoSpacing = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static int iPlotSetAxisYAutoTickAttrib(Ihandle* ih, const char* value)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  if (iupStrBoolean(value))
    axis->mTick.mAutoSpacing = true;
  else 
    axis->mTick.mAutoSpacing = false;

  ih->data->current_plot->mRedraw = 1;
  return 0;
}

static char* iPlotGetAxisXAutoTickAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisX;
  return iupStrReturnBoolean (axis->mTick.mAutoSpacing); 
}

static char* iPlotGetAxisYAutoTickAttrib(Ihandle* ih)
{
  iupPlotAxis* axis = &ih->data->current_plot->mAxisY;

  return iupStrReturnBoolean (axis->mTick.mAutoSpacing); 
}


/************************************************************************************/


static int iPlotMapMethod(Ihandle* ih)
{
  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
  {
    IupGLMakeCurrent(ih);
    double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
    ih->data->cd_canvas = cdCreateCanvasf(CD_GL, "%dx%d %g", ih->currentwidth, ih->currentheight, res);
  }
  else if (ih->data->graphics_mode == IUP_PLOT_IMAGERGB)
    ih->data->cd_canvas = cdCreateCanvas(CD_IUPDBUFFERRGB, ih);
  else
    ih->data->cd_canvas = cdCreateCanvas(CD_IUPDBUFFER, ih);
  if (!ih->data->cd_canvas)
    return IUP_ERROR;

  for(int p=0; p<ih->data->plot_list_count; p++)
    ih->data->plot_list[p]->mRedraw = 1;

  return IUP_NOERROR;
}

static void iPlotUnMapMethod(Ihandle* ih)
{
  if (ih->data->cd_canvas != NULL)
  {
    cdKillCanvas(ih->data->cd_canvas);
    ih->data->cd_canvas = NULL;
  }
}

static void iPlotDestroyMethod(Ihandle* ih)
{
  for(int p=0; p<ih->data->plot_list_count; p++)
    delete ih->data->plot_list[p];
}

static int iPlotCreateMethod(Ihandle* ih, void **params)
{
  (void)params;

  /* free the data alocated by IupCanvas */
  free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  /* Initializing object with no cd canvases */
  ih->data->plot_list[0] = new iupPlot(ih);
  ih->data->numcol = 1;
  ih->data->plot_list_count = 1;

  ih->data->current_plot = ih->data->plot_list[ih->data->current_plot_index];

  /* IupCanvas callbacks */
  IupSetCallback(ih, "ACTION",      (Icallback)iPlotRedraw_CB);
  IupSetCallback(ih, "RESIZE_CB",   (Icallback)iPlotResize_CB);
  IupSetCallback(ih, "BUTTON_CB",   (Icallback)iPlotMouseButton_CB);
  IupSetCallback(ih, "MOTION_CB",   (Icallback)iPlotMouseMove_CB);
  IupSetCallback(ih, "KEYPRESS_CB", (Icallback)iPlotKeyPress_CB);
  IupSetCallback(ih, "WHEEL_CB",    (Icallback)iPlotWheel_CB);

  return IUP_NOERROR;
}

static Iclass* iPlotNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glcanvas"));

  ic->name = (char*)"plot";
  ic->format = NULL;  /* none */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iPlotNewClass;
  ic->Create  = iPlotCreateMethod;
  ic->Destroy = iPlotDestroyMethod;
  ic->Map     = iPlotMapMethod;
  ic->UnMap   = iPlotUnMapMethod;

  /* IupPlot Callbacks */
  iupClassRegisterCallback(ic, "POSTDRAW_CB", "C");
  iupClassRegisterCallback(ic, "PREDRAW_CB", "C");
  iupClassRegisterCallback(ic, "PLOTMOTION_CB", "dds");
  iupClassRegisterCallback(ic, "PLOTBUTTON_CB", "iidds");

  //iupClassRegisterCallback(ic, "SELECT_CB", "iiffi");
  //iupClassRegisterCallback(ic, "SELECTBEGIN_CB", "");
  //iupClassRegisterCallback(ic, "SELECTEND_CB", "");

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iPlotSetBGColorAttrib, IUPAF_SAMEASSYSTEM, "255 255 255", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iPlotSetFGColorAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iPlotSetStandardFontAttrib, IUPAF_SAMEASSYSTEM, "DEFAULTFONT", IUPAF_NO_SAVE | IUPAF_NOT_MAPPED);

  /* IupPlot only */

  iupClassRegisterAttribute(ic, "ANTIALIAS", iPlotGetAntialiasAttrib, iPlotSetAntialiasAttrib, IUPAF_SAMEASSYSTEM, "No", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REDRAW", NULL, iPlotSetRedrawAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SYNCVIEW", iPlotGetSyncViewAttrib, iPlotSetSyncViewAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CANVAS", iPlotGetCanvasAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRAPHICSMODE", iPlotGetGraphicsModeAttrib, iPlotSetGraphicsModeAttrib, IUPAF_SAMEASSYSTEM, "NATIVE", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "MARGINLEFT", iPlotGetMarginLeftAttrib, iPlotSetMarginLeftAttrib, IUPAF_SAMEASSYSTEM, "AUTO", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARGINRIGHT", iPlotGetMarginRightAttrib, iPlotSetMarginRightAttrib, IUPAF_SAMEASSYSTEM, "AUTO", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARGINTOP", iPlotGetMarginTopAttrib, iPlotSetMarginTopAttrib, IUPAF_SAMEASSYSTEM, "AUTO", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARGINBOTTOM", iPlotGetMarginBottomAttrib, iPlotSetMarginBottomAttrib, IUPAF_SAMEASSYSTEM, "AUTO", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BACKCOLOR", iPlotGetBackColorAttrib, iPlotSetBackColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TITLE", iPlotGetTitleAttrib, iPlotSetTitleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLECOLOR", iPlotGetTitleColorAttrib, iPlotSetTitleColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEFONTSIZE", iPlotGetTitleFontSizeAttrib, iPlotSetTitleFontSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEFONTSTYLE", iPlotGetTitleFontStyleAttrib, iPlotSetTitleFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "LEGEND", iPlotGetLegendAttrib, iPlotSetLegendAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "LEGENDSHOW", iPlotGetLegendAttrib, iPlotSetLegendAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDPOS", iPlotGetLegendPosAttrib, iPlotSetLegendPosAttrib, IUPAF_SAMEASSYSTEM, "TOPRIGHT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDFONTSIZE", iPlotGetLegendFontSizeAttrib, iPlotSetLegendFontSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDFONTSTYLE", iPlotGetLegendFontStyleAttrib, iPlotSetLegendFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDBOXCOLOR", iPlotGetLegendBoxColorAttrib, iPlotSetLegendBoxColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDBOXBACKCOLOR", iPlotGetLegendBoxBackColorAttrib, iPlotSetLegendBoxBackColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDBOXLINESTYLE", iPlotGetLegendBoxLineStyleAttrib, iPlotSetLegendBoxLineStyleAttrib, IUPAF_SAMEASSYSTEM, "CONTINUOUS", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDBOXLINEWIDTH", iPlotGetLegendBoxLineWidthAttrib, iPlotSetLegendBoxLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "GRIDCOLOR", iPlotGetGridColorAttrib, iPlotSetGridColorAttrib, IUPAF_SAMEASSYSTEM, "200 200 200", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRID", iPlotGetGridAttrib, iPlotSetGridAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRIDLINESTYLE", iPlotGetGridLineStyleAttrib, iPlotSetGridLineStyleAttrib, IUPAF_SAMEASSYSTEM, "CONTINUOUS", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRIDLINEWIDTH", iPlotGetGridLineWidthAttrib, iPlotSetGridLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BOX", iPlotGetBoxAttrib, iPlotSetBoxAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BOXCOLOR", iPlotGetBoxColorAttrib, iPlotSetBoxColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BOXLINESTYLE", iPlotGetBoxLineStyleAttrib, iPlotSetBoxLineStyleAttrib, IUPAF_SAMEASSYSTEM, "CONTINUOUS", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BOXLINEWIDTH", iPlotGetBoxLineWidthAttrib, iPlotSetBoxLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DS_LINESTYLE", iPlotGetDSLineStyleAttrib, iPlotSetDSLineStyleAttrib, IUPAF_SAMEASSYSTEM, "CONTINUOUS", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_LINEWIDTH", iPlotGetDSLineWidthAttrib, iPlotSetDSLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MARKSTYLE", iPlotGetDSMarkStyleAttrib, iPlotSetDSMarkStyleAttrib, IUPAF_SAMEASSYSTEM, "X", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MARKSIZE", iPlotGetDSMarkSizeAttrib, iPlotSetDSMarkSizeAttrib, IUPAF_SAMEASSYSTEM, "7", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_NAME", iPlotGetDSNameAttrib, iPlotSetDSNameAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
    iupClassRegisterAttribute(ic, "DS_LEGEND", iPlotGetDSNameAttrib, iPlotSetDSNameAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_COLOR", iPlotGetDSColorAttrib, iPlotSetDSColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MODE", iPlotGetDSModeAttrib, iPlotSetDSModeAttrib, IUPAF_SAMEASSYSTEM, "LINE", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_REMOVE", NULL, iPlotSetDSRemoveAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_COUNT", iPlotGetDSCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_X", iPlotGetAxisXAttrib, iPlotSetAxisXAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_Y", iPlotGetAxisYAttrib, iPlotSetAxisYAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XLABEL", iPlotGetAxisXLabelAttrib, iPlotSetAxisXLabelAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLABEL", iPlotGetAxisYLabelAttrib, iPlotSetAxisYLabelAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XLABELCENTERED", iPlotGetAxisXLabelCenteredAttrib, iPlotSetAxisXLabelCenteredAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLABELCENTERED", iPlotGetAxisYLabelCenteredAttrib, iPlotSetAxisYLabelCenteredAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XFONTSIZE", iPlotGetAxisXFontSizeAttrib, iPlotSetAxisXFontSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YFONTSIZE", iPlotGetAxisYFontSizeAttrib, iPlotSetAxisYFontSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XFONTSTYLE", iPlotGetAxisXFontStyleAttrib, iPlotSetAxisXFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YFONTSTYLE", iPlotGetAxisYFontStyleAttrib, iPlotSetAxisYFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XLINEWIDTH", iPlotGetAxisXLineWidthAttrib, iPlotSetAxisXLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLINEWIDTH", iPlotGetAxisYLineWidthAttrib, iPlotSetAxisYLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XCOLOR", iPlotGetAxisXColorAttrib, iPlotSetAxisXColorAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YCOLOR", iPlotGetAxisYColorAttrib, iPlotSetAxisYColorAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XAUTOMIN", iPlotGetAxisXAutoMinAttrib, iPlotSetAxisXAutoMinAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YAUTOMIN", iPlotGetAxisYAutoMinAttrib, iPlotSetAxisYAutoMinAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XAUTOMAX", iPlotGetAxisXAutoMaxAttrib, iPlotSetAxisXAutoMaxAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YAUTOMAX", iPlotGetAxisYAutoMaxAttrib, iPlotSetAxisYAutoMaxAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XMIN", iPlotGetAxisXMinAttrib, iPlotSetAxisXMinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YMIN", iPlotGetAxisYMinAttrib, iPlotSetAxisYMinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XMAX", iPlotGetAxisXMaxAttrib, iPlotSetAxisXMaxAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YMAX", iPlotGetAxisYMaxAttrib, iPlotSetAxisYMaxAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XREVERSE", iPlotGetAxisXReverseAttrib, iPlotSetAxisXReverseAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YREVERSE", iPlotGetAxisYReverseAttrib, iPlotSetAxisYReverseAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XCROSSORIGIN", iPlotGetAxisXCrossOriginAttrib, iPlotSetAxisXCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YCROSSORIGIN", iPlotGetAxisYCrossOriginAttrib, iPlotSetAxisYCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XSCALE", iPlotGetAxisXScaleAttrib, iPlotSetAxisXScaleAttrib, IUPAF_SAMEASSYSTEM, "LIN", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YSCALE", iPlotGetAxisYScaleAttrib, iPlotSetAxisYScaleAttrib, IUPAF_SAMEASSYSTEM, "LIN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XARROW", iPlotGetAxisXArrowAttrib, iPlotSetAxisXArrowAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YARROW", iPlotGetAxisYArrowAttrib, iPlotSetAxisYArrowAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XTICK", iPlotGetAxisXTickAttrib, iPlotSetAxisXTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICK", iPlotGetAxisYTickAttrib, iPlotSetAxisYTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XTICKSIZEAUTO", iPlotGetAxisXAutoTickSizeAttrib, iPlotSetAxisXAutoTickSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKSIZEAUTO", iPlotGetAxisYAutoTickSizeAttrib, iPlotSetAxisYAutoTickSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_XAUTOTICKSIZE", iPlotGetAxisXAutoTickSizeAttrib, iPlotSetAxisXAutoTickSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_YAUTOTICKSIZE", iPlotGetAxisYAutoTickSizeAttrib, iPlotSetAxisYAutoTickSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMINORSIZE", iPlotGetAxisXTickSizeAttrib, iPlotSetAxisXTickSizeAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMINORSIZE", iPlotGetAxisYTickSizeAttrib, iPlotSetAxisYTickSizeAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_XTICKSIZE", iPlotGetAxisXTickSizeAttrib, iPlotSetAxisXTickSizeAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_YTICKSIZE", iPlotGetAxisYTickSizeAttrib, iPlotSetAxisYTickSizeAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMAJORSIZE", iPlotGetAxisXTickMajorSizeAttrib, iPlotSetAxisXTickMajorSizeAttrib, IUPAF_SAMEASSYSTEM, "8", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMAJORSIZE", iPlotGetAxisYTickMajorSizeAttrib, iPlotSetAxisYTickMajorSizeAttrib, IUPAF_SAMEASSYSTEM, "8", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XTICKAUTO", iPlotGetAxisXAutoTickAttrib, iPlotSetAxisXAutoTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKAUTO", iPlotGetAxisYAutoTickAttrib, iPlotSetAxisYAutoTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_XAUTOTICK", iPlotGetAxisXAutoTickAttrib, iPlotSetAxisXAutoTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_YAUTOTICK", iPlotGetAxisYAutoTickAttrib, iPlotSetAxisYAutoTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMAJORSPAN", iPlotGetAxisXTickMajorSpanAttrib, iPlotSetAxisXTickMajorSpanAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMAJORSPAN", iPlotGetAxisYTickMajorSpanAttrib, iPlotSetAxisYTickMajorSpanAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMINORDIVISION", iPlotGetAxisXTickDivisionAttrib, iPlotSetAxisXTickDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMINORDIVISION", iPlotGetAxisYTickDivisionAttrib, iPlotSetAxisYTickDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_XTICKDIVISION", iPlotGetAxisXTickDivisionAttrib, iPlotSetAxisXTickDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
      iupClassRegisterAttribute(ic, "AXS_YTICKDIVISION", iPlotGetAxisYTickDivisionAttrib, iPlotSetAxisYTickDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XTICKNUMBER", iPlotGetAxisXTickNumberAttrib, iPlotSetAxisXTickNumberAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKNUMBER", iPlotGetAxisYTickNumberAttrib, iPlotSetAxisYTickNumberAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKROTATENUMBER", iPlotGetAxisXTickRotateNumberAttrib, iPlotSetAxisXTickRotateNumberAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKROTATENUMBER", iPlotGetAxisYTickRotateNumberAttrib, iPlotSetAxisYTickRotateNumberAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKFORMAT", iPlotGetAxisXTickFormatAttrib, iPlotSetAxisXTickFormatAttrib, IUPAF_SAMEASSYSTEM, "%.0f", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKFORMAT", iPlotGetAxisYTickFormatAttrib, iPlotSetAxisYTickFormatAttrib, IUPAF_SAMEASSYSTEM, "%.0f", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKFONTSIZE", iPlotGetAxisXTickFontSizeAttrib, iPlotSetAxisXTickFontSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKFONTSIZE", iPlotGetAxisYTickFontSizeAttrib, iPlotSetAxisYTickFontSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKFONTSTYLE", iPlotGetAxisXTickFontStyleAttrib, iPlotSetAxisXTickFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKFONTSTYLE", iPlotGetAxisYTickFontStyleAttrib, iPlotSetAxisYTickFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "REMOVE", NULL, iPlotSetRemoveAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLEAR", NULL, iPlotSetClearAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COUNT", iPlotGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CURRENT", iPlotGetCurrentAttrib, iPlotSetCurrentAttrib, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "PLOT_NUMCOL", iPlotGetPlotNumColAttrib, iPlotSetPlotNumColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PLOT_CURRENT", iPlotGetPlotCurrentAttrib, iPlotSetPlotCurrentAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PLOT_COUNT", iPlotGetPlotCountAttrib, iPlotSetPlotCountAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PLOT_REMOVE", NULL, iPlotSetPlotRemoveAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PLOT_INSERT", NULL, iPlotSetPlotInsertAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupPlot(void)
{
  return IupCreate("plot");
}

void IupPlotOpen(void)
{
  IupGLCanvasOpen();

  if (!IupGetGlobal("_IUP_PLOT_OPEN"))
  {
    iupRegisterClass(iPlotNewClass());
    IupSetGlobal("_IUP_PLOT_OPEN", "1");
  }
}

/* TODO
DS_MODES  STEM  STEP  
          BARHORIZONTAL
          CHART
*/
