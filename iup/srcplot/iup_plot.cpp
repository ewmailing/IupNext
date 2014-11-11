/*
 * IupPlot element
 *
 */

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
#include "iup_plot_ctrl.h"



void iupPlotSetPlotCurrent(Ihandle* ih, int p)
{
  ih->data->current_plot_index = p;
  ih->data->current_plot = ih->data->plot_list[ih->data->current_plot_index];
}

void iupPlotRedraw(Ihandle* ih, int flush, int only_current, int reset_redraw)
{
  cdCanvasActivate(ih->data->cd_canvas);

  if (only_current)
  {
    if (reset_redraw)
      ih->data->current_plot->mRedraw = true;
    ih->data->current_plot->Render(ih->data->cd_canvas);
  }
  else
  {
    int old_current = ih->data->current_plot_index;
    for (int p = 0; p < ih->data->plot_list_count; p++)
    {
      // Set current because of the draw callbacks
      iupPlotSetPlotCurrent(ih, p);

      if (reset_redraw)
        ih->data->current_plot->mRedraw = true;
      ih->data->current_plot->Render(ih->data->cd_canvas);
    }
    iupPlotSetPlotCurrent(ih, old_current);
  }

  // Do the flush once
  if (flush)
  {
    cdCanvasFlush(ih->data->cd_canvas);

    if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
      IupGLSwapBuffers(ih);
  }
}

static int iPlotAction_CB(Ihandle* ih)
{
  // in the redraw callback
  int flush = 1,  // always flush
    only_current = 0,  // redraw all plots
    reset_redraw = 0; // render only if necessary

  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
  {
    IupGLMakeCurrent(ih);

    // in OpenGL mode must:
    flush = 1;  // always flush
    only_current = 0;  // redraw all plots
    reset_redraw = 1;  // always render
  }

  iupPlotRedraw(ih, flush, only_current, reset_redraw);

  return IUP_DEFAULT;
}

void iupPlotUpdateViewports(Ihandle* ih)
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

    double res = IupGetDouble(NULL, "SCREENDPI") / 25.4;
    cdCanvasSetfAttribute(ih->data->cd_canvas, "SIZE", "%dx%d %g", width, height, res);
  }

  iupPlotUpdateViewports(ih);
  return IUP_DEFAULT;
}

static void iPlotRedrawInteract(Ihandle *ih)
{
  // when interacting
  int flush = 0, // flush if necessary
    only_current = 1, // draw all plots if sync
    reset_redraw = 0;  // render only if necessary

  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
  {
    // in OpenGL mode must:
    flush = 1;  // always flush
    only_current = 0;  // redraw all plots
    reset_redraw = 1;  // always render
  }
  else
  {
    if (ih->data->current_plot->mRedraw)
      flush = 1;

    if (ih->data->sync_view)
      only_current = 0;
  }

  iupPlotRedraw(ih, flush, only_current, reset_redraw);
}

void iupPlotResetZoom(Ihandle *ih, int redraw)
{
  ih->data->current_plot->ResetZoom();

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
        ih->data->plot_list[p]->ResetZoom();
    }
  }

  if (redraw)
    iPlotRedrawInteract(ih);
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
      }
    }
  }

  iPlotRedrawInteract(ih);
}

static void iPlotZoom(Ihandle *ih, int x, int y, float delta)
{
  double rx, ry;
  ih->data->current_plot->TransformBack(x, y, rx, ry);

  if (delta > 0)
    ih->data->current_plot->ZoomIn(rx, ry);
  else
    ih->data->current_plot->ZoomOut(rx, ry);

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
      }
    }
  }

  iPlotRedrawInteract(ih);
}

static void iPlotZoomTo(Ihandle *ih, int x1, int y1, int x2, int y2)
{
  double rx1, ry1, rx2, ry2;
  ih->data->current_plot->TransformBack(x1, y1, rx1, ry1);
  ih->data->current_plot->TransformBack(x2, y2, rx2, ry2);

  ih->data->current_plot->ZoomTo(rx1, rx2, ry1, ry2);

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
      {
        ih->data->plot_list[p]->TransformBack(x1, y1, rx1, ry1);
        ih->data->plot_list[p]->TransformBack(x2, y2, rx2, ry2);

        ih->data->plot_list[p]->ZoomTo(rx1, rx2, ry1, ry2);
      }
    }
  }

  iPlotRedrawInteract(ih);
}

static void iPlotScroll(Ihandle *ih, float delta, bool full_page, bool vertical)
{
  ih->data->current_plot->Scroll(delta, full_page, vertical);

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
        ih->data->plot_list[p]->Scroll(delta, full_page, vertical);
    }
  }

  iPlotRedrawInteract(ih);
}

static void iPlotScrollTo(Ihandle *ih, int x, int y)
{
  double rx, ry;
  ih->data->current_plot->TransformBack(x, y, rx, ry);

  ih->data->current_plot->ScrollTo(rx, ry);

  if (ih->data->sync_view)
  {
    for (int p = 0; p<ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p] != ih->data->current_plot)
      {
        ih->data->plot_list[p]->TransformBack(x, y, rx, ry);
        ih->data->plot_list[p]->ScrollTo(rx, ry);
      }
    }
  }

  iPlotRedrawInteract(ih);
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

static int iPlotButton_CB(Ihandle* ih, int button, int press, int x, int y, char* status)
{
  int index = iPlotFindPlot(ih, x, y);
  if (index<0)
    return IUP_DEFAULT;

  iupPlotSetPlotCurrent(ih, index);

  x -= ih->data->current_plot->mViewport.mX;
  y -= ih->data->current_plot->mViewport.mY;

  IFniidds cb = (IFniidds)IupGetCallback(ih, "PLOTBUTTON_CB");
  if (cb) 
  {
    cdCanvasOrigin(ih->data->cd_canvas, ih->data->current_plot->mViewport.mX, ih->data->current_plot->mViewport.mY);

    double rx, ry;
    ih->data->current_plot->TransformBack(x, y, rx, ry);
    if (cb(ih, button, press, rx, ry, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (press)
  {
    ih->data->last_click_x = x;
    ih->data->last_click_y = y;
    ih->data->last_click_plot = index;

    if (button == IUP_BUTTON1)
    {
      if (!iup_iscontrol(status))
      {
        if (iup_isdouble(status))
          iupPlotResetZoom(ih, 1);
        else
          iPlotPanStart(ih);
      }
    }
    else if (button == IUP_BUTTON2)
    {
      if (!iup_iscontrol(status))
        iPlotScrollTo(ih, x, y);
    }
  }
  else
  {
    if (iup_iscontrol(status))
    {
      if (ih->data->last_click_x == x && ih->data->last_click_y == y && ih->data->last_click_plot == index)
      {
        float delta = 0;
        if (button == IUP_BUTTON1)
          delta = 1.0;
        else if (button == IUP_BUTTON3)
          delta = -1.0;

        if (delta)
          iPlotZoom(ih, x, y, delta);
      }
      else if (button == IUP_BUTTON1 && ih->data->last_click_x != x && ih->data->last_click_y != y && ih->data->last_click_plot == index)
      {
        iPlotZoomTo(ih, ih->data->last_click_x, ih->data->last_click_y, x, y);
      }
    }
  }

  return IUP_DEFAULT;
}

static int iPlotMotion_CB(Ihandle* ih, int x, int y, char *status)
{
  int index = iPlotFindPlot(ih, x, y);
  if (index<0)
    return IUP_DEFAULT;

  iupPlotSetPlotCurrent(ih, index);

  x -= ih->data->current_plot->mViewport.mX;
  y -= ih->data->current_plot->mViewport.mY;

  IFndds cb = (IFndds)IupGetCallback(ih, "PLOTMOTION_CB");
  if (cb) 
  {
    cdCanvasOrigin(ih->data->cd_canvas, ih->data->current_plot->mViewport.mX, ih->data->current_plot->mViewport.mY);

    double rx, ry;
    ih->data->current_plot->TransformBack(x, y, rx, ry);
    if (cb(ih, rx, ry, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (iup_isbutton1(status) && ih->data->last_click_plot == index)
  {
    if (iup_iscontrol(status)) // || iup_isshift(status))
    {
      ih->data->current_plot->mRedraw = true;
      ih->data->current_plot->mShowSelection = true;
      ih->data->current_plot->mSelection.mX = ih->data->last_click_x < x? ih->data->last_click_x: x;
      ih->data->current_plot->mSelection.mY = ih->data->last_click_y < y? ih->data->last_click_y: y;
      ih->data->current_plot->mSelection.mWidth = abs(ih->data->last_click_x - x) + 1;
      ih->data->current_plot->mSelection.mHeight = abs(ih->data->last_click_y - y) + 1;

      iPlotRedrawInteract(ih);

      ih->data->current_plot->mShowSelection = false;
      return IUP_DEFAULT;
    }
    else
    {
      if (ih->data->last_click_x != x || ih->data->last_click_y != y)
      {
        iPlotPan(ih, ih->data->last_click_x, ih->data->last_click_y, x, y);
        return IUP_DEFAULT;
      }
    }
  }

  int ds, sample;
  double rx, ry;
  const char* ds_name;
  const char* strX;
  if (ih->data->current_plot->FindDataSetSample(x, y, ds, ds_name, sample, rx, ry, strX))
  {
    if (ih->data->last_tip_ds != ds && ih->data->last_tip_sample != sample)
    {
      char str_y[20];
      sprintf(str_y, ih->data->current_plot->mAxisY.mTick.mFormatString, ry);
      if (strX)
        IupSetfAttribute(ih, "TIP", "%s [%d]=(%s, %s)", ds_name, sample, strX, str_y);
      else
      {
        char str_x[20];
        sprintf(str_x, ih->data->current_plot->mAxisX.mTick.mFormatString, rx);
        IupSetfAttribute(ih, "TIP", "%s [%d]=(%s, %s)", ds_name, sample, str_x, str_y);
      }

      IupSetAttribute(ih, "TIPVISIBLE", "Yes");

      ih->data->last_tip_ds = ds;
      ih->data->last_tip_sample = sample;
    }
  }
  else
  {
    if (ih->data->last_tip_ds != -1 && ih->data->last_tip_sample != -1)
    {
      ih->data->last_tip_ds = -1;
      ih->data->last_tip_sample = -1;
      IupSetAttribute(ih, "TIP", NULL);
      IupSetAttribute(ih, "TIPVISIBLE", "Yes");
    }
  }

  if (ih->data->show_cross_hair)
  {
    ih->data->current_plot->mRedraw = true;
    ih->data->current_plot->mCrossHair = true;
    ih->data->current_plot->mCrossHairX = x;

    iPlotRedrawInteract(ih);
  }

  return IUP_DEFAULT;
}

static int iPlotWheel_CB(Ihandle *ih, float delta, int x, int y, char* status)
{
  int index = iPlotFindPlot(ih, x, y);
  if (index<0)
    return IUP_DEFAULT;

  iupPlotSetPlotCurrent(ih, index);

  x -= ih->data->current_plot->mViewport.mX;
  y -= ih->data->current_plot->mViewport.mY;

  if (iup_iscontrol(status))
    iPlotZoom(ih, x, y, delta);
  else 
  {
    bool vertical = true;
    if (iup_isshift(status))
      vertical = false;

    iPlotScroll(ih, delta, false, vertical);
  }

  return IUP_DEFAULT;
}

static int iPlotKeyPress_CB(Ihandle* ih, int c, int press)
{
  if (!press)
    return IUP_DEFAULT;

  if (ih->data->graphics_mode == IUP_PLOT_OPENGL)
    IupGLMakeCurrent(ih);

  if (c == K_cH)
  {
    ih->data->show_cross_hair = !ih->data->show_cross_hair;

    for (int p = 0; p < ih->data->plot_list_count; p++)
    {
      if (ih->data->plot_list[p]->mCrossHair)
      {
        ih->data->plot_list[p]->mRedraw = true;
        ih->data->plot_list[p]->mCrossHair = false;
      }
    }

    if (!ih->data->show_cross_hair)  // was shown
      iPlotRedrawInteract(ih);
  }
  else if (c == K_plus)
  {
    int x = ih->data->current_plot->mViewport.mX + ih->data->current_plot->mViewport.mWidth/2;
    int y = ih->data->current_plot->mViewport.mY + ih->data->current_plot->mViewport.mHeight/2;
    iPlotZoom(ih, x, y, 1);
  }
  else if (c == K_minus)
  {
    int x = ih->data->current_plot->mViewport.mX + ih->data->current_plot->mViewport.mWidth / 2;
    int y = ih->data->current_plot->mViewport.mY + ih->data->current_plot->mViewport.mHeight / 2;
    iPlotZoom(ih, x, y, -1);
  }
  else if (c == K_LEFT || c == K_RIGHT)
  {
    float delta = 1.0f;
    if (c == K_LEFT) delta = -1.0f;
    iPlotScroll(ih, delta, false, false);
  }
  else if (c == K_UP || c == K_DOWN || c == K_PGUP || c == K_PGDN)
  {
    float delta = 1.0f;
    if (c == K_DOWN || c == K_PGDN) delta = -1.0f;

    bool full_page = false;
    if (c == K_PGUP || c == K_PGDN) full_page = true;

    iPlotScroll(ih, delta, full_page, true);
  }

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

  ih->data->current_plot->mRedraw = true;
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
  iupPlotUpdateViewports(ih);

  // when drawing to an external canvas
  int flush = 0, // never flush
    only_current = 0,  // draw all plots
    reset_redraw = 1;  // always render

  iupPlotRedraw(ih, flush, only_current, reset_redraw);

  ih->data->cd_canvas = old_cd_canvas;
  iupPlotUpdateViewports(ih);
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
    ih->data->plot_list[p]->mRedraw = true;

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

  ih->data->last_tip_ds = -1;
  ih->data->last_tip_sample = -1;

  /* Initializing object with no cd canvases */
  ih->data->plot_list[0] = new iupPlot(ih);
  ih->data->numcol = 1;
  ih->data->plot_list_count = 1;

  ih->data->current_plot = ih->data->plot_list[ih->data->current_plot_index];

  /* IupCanvas callbacks */
  IupSetCallback(ih, "ACTION",      (Icallback)iPlotAction_CB);
  IupSetCallback(ih, "RESIZE_CB",   (Icallback)iPlotResize_CB);
  IupSetCallback(ih, "BUTTON_CB",   (Icallback)iPlotButton_CB);
  IupSetCallback(ih, "MOTION_CB",   (Icallback)iPlotMotion_CB);
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

  iupPlotRegisterAttributes(ic);

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
