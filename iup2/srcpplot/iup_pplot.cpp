/*
 * IupPPlot component
 *
 * Description : A component, derived from PPlot and IUP canvas
 *      Remark : Depend on libs IUP, CD, IUPCD
 */


#ifdef _MSC_VER
#pragma warning(disable: 4100)
#pragma warning(disable: 4512)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "iup.h"
#include "iupcpi.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>
#include <cdirgb.h>
#include <cdgdiplus.h>

#include "iupcompat.h"
#include "iup_pplot.h"

#include "PPlot.h"
#include "PPlotInteraction.h"
#include "iup_pplot.hpp"

static char* PPLOTOBJ_ = "_IUP_PPLOT_OBJ";  // use preffix to make it an internal attribute

#ifndef M_E
#define M_E 2.71828182846
#endif


/*********************************************************************************
                                    IUP CANVAS CALLBACKS
*********************************************************************************/


/*
  callback: forward paint request to PPlot object
*/
static int PPlotPaint_cb(Ihandle* self)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->Draw(0);  /* full redraw only if nothing changed */
  return IUP_DEFAULT;
}


/*
  callback: forward resize request to PPlot object
*/
static int PPlotResize_cb(Ihandle* self)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->Resize();
  return IUP_DEFAULT;
}


/*
  callback: forward mouse button events to PPlot object
*/
static int PPlotMouseButton_cb(Ihandle* self, 
                                   int btn, int stat,
                                   int x, int y,
                                   char* r)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->MouseButton(btn, stat, x, y, r);

  return IUP_DEFAULT;
}


/*
  callback: forward mouse button events to PPlot object
*/
static int PPlotMouseMove_cb(Ihandle* self, int x, int y)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->MouseMove(x, y);

  return IUP_DEFAULT;
}


/*
  callback: forward kyeboard events to PPlot object
*/
static int PPlotKeyPress_cb(Ihandle* self, int c, int press)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->KeyPress(c, press);

  return IUP_DEFAULT;
} 


/*********************************************************************************
                                    IUP CPI METHODS
*********************************************************************************/


/*
  perform creation of the PPlot object
*/
static Ihandle * PPlotCreate(Iclass* ic)
{
  Ihandle *hnd;
  PPainterIup *plt;

  hnd = IupCanvas(NULL);

  plt = new PPainterIup(hnd);
  iupSetEnv(hnd, PPLOTOBJ_, (char *)plt);
  /* custom callbacks */
  IupSetCallback(hnd, "ACTION", (Icallback)PPlotPaint_cb);
  IupSetCallback(hnd, "RESIZE_CB", (Icallback)PPlotResize_cb);
  IupSetCallback(hnd, "BUTTON_CB", (Icallback)PPlotMouseButton_cb);
  IupSetCallback(hnd, "MOTION_CB", (Icallback)PPlotMouseMove_cb);
  IupSetCallback(hnd, "KEYPRESS_CB", (Icallback)PPlotKeyPress_cb);

  return hnd;
}


/*
  perform destruction of the PPlot object
*/
static void PPlotDestroy(Ihandle* self)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;

  if (plt) delete plt;  
  iupSetEnv(self, PPLOTOBJ_, (char *)NULL); 
}


/*
  forward get/set to the PPlot object
*/
static void PPlotSetAttribute(Ihandle* self, char* attr, char* value)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->SetAttribute(attr, value);
}

static char* PPlotGetAttribute(Ihandle* self, char* attr)
{
  char *pch;
  PPainterIup *plt;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    return (plt->GetAttribute(attr));
  else
    return NULL; 
}


/*********************************************************************************
                                    USER LEVEL
*********************************************************************************/


void IupPPlotOpen(void)
{
  Iclass *new_class = iupCpiCreateNewClass("pplot", NULL);  // no parameters in creation (used by LED)

  iupCpiSetClassMethod(new_class, ICPI_CREATE, (Imethod)PPlotCreate);
  iupCpiSetClassMethod(new_class, ICPI_DESTROY, (Imethod)PPlotDestroy);
  iupCpiSetClassMethod(new_class, ICPI_SETATTR, (Imethod)PPlotSetAttribute);
  iupCpiSetClassMethod(new_class, ICPI_GETATTR, (Imethod)PPlotGetAttribute);

}

/*
  user level call: create control
*/
Ihandle *IupPPlot(void)
{
  return IupCreate("pplot");
}

static int get_CdFontSize(char* value);
static int get_CdFontStyle(char* value);

/*
  user level call: add dataset to plot
*/
void IupPPlotBegin(Ihandle *self, int strXdata)
{
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    PlotDataBase *inXData = (PlotDataBase*)iupGetEnv(self, "_IUP_PLOT_XDATA");
    PlotDataBase *inYData = (PlotDataBase*)iupGetEnv(self, "_IUP_PLOT_YDATA");
    if (inXData) delete inXData;
    if (inYData) delete inYData;

    if (strXdata)
      inXData = (PlotDataBase *)(new StringData());
    else
      inXData = (PlotDataBase *)(new PlotData());
    inYData = (PlotDataBase *)new PlotData();

    iupSetEnv(self, "_IUP_PLOT_XDATA", (char*)inXData);
    iupSetEnv(self, "_IUP_PLOT_YDATA", (char*)inYData);
    iupSetEnv(self, "_IUP_PLOT_STRXDATA", (char*)(strXdata? "1": "0"));
  }
}

void IupPPlotAdd(Ihandle *self, float x, float y)
{
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    PlotData *inXData = (PlotData*)iupGetEnv(self, "_IUP_PLOT_XDATA");
    PlotData *inYData = (PlotData*)iupGetEnv(self, "_IUP_PLOT_YDATA");
    int strXdata = iupGetEnvInt(self, "_IUP_PLOT_STRXDATA");
    if (!inYData || !inXData || strXdata)
      return;

    inXData->push_back(x);
    inYData->push_back(y);
  }
}

void IupPPlotAddStr(Ihandle *self, const char* x, float y)
{
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    StringData *inXData = (StringData*)iupGetEnv(self, "_IUP_PLOT_XDATA");
    PlotData *inYData = (PlotData*)iupGetEnv(self, "_IUP_PLOT_YDATA");
    int strXdata = iupGetEnvInt(self, "_IUP_PLOT_STRXDATA");
    if (!inYData || !inXData || !strXdata)
      return;

    inXData->AddItem(x);
    inYData->push_back(y);
  }
}

void IupPPlotInsertStr(Ihandle *self, int inIndex, int inSampleIndex, const char* inX, float inY)
{
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    PlotDataBase *theXDataBase = plt->_plot.mPlotDataContainer.GetXData(inIndex);
    PlotDataBase *theYDataBase = plt->_plot.mPlotDataContainer.GetYData(inIndex);
    StringData *theXData = (StringData*)theXDataBase;
    PlotData *theYData = (PlotData *)theYDataBase;
    theXData->InsertItem(inSampleIndex, inX);
    theYData->insert(theYData->begin()+inSampleIndex, inY);
  }
}

void IupPPlotInsert(Ihandle *self, int inIndex, int inSampleIndex, float inX, float inY)
{
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    PlotDataBase *theXDataBase = plt->_plot.mPlotDataContainer.GetXData(inIndex);
    PlotDataBase *theYDataBase = plt->_plot.mPlotDataContainer.GetYData(inIndex);
    PlotData *theXData = (PlotData*)theXDataBase;
    PlotData *theYData = (PlotData*)theYDataBase;
    theXData->insert(theXData->begin()+inSampleIndex, inX);
    theYData->insert(theYData->begin()+inSampleIndex, inY);
  }
}

int IupPPlotEnd(Ihandle *self)
{
  int iret = -1;
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    PlotDataBase *inXData = (PlotDataBase*)iupGetEnv(self, "_IUP_PLOT_XDATA");
    PlotDataBase *inYData = (PlotDataBase*)iupGetEnv(self, "_IUP_PLOT_YDATA");
    if (!inYData || !inXData)
      return iret;

    /* add to plot */
    iret = plt->_plot.mPlotDataContainer.AddXYPlot(inXData, inYData);
    plt->_currentDataSetIndex = iret;

    LegendData* legend = plt->_plot.mPlotDataContainer.GetLegendData(iret);
    legend->mStyle.mFontStyle = get_CdFontStyle(IupGetAttribute(self, "LEGENDFONTSTYLE"));
    legend->mStyle.mFontSize = get_CdFontSize(IupGetAttribute(self, "LEGENDFONTSIZE"));

    iupSetEnv(self, "_IUP_PLOT_XDATA", NULL);
    iupSetEnv(self, "_IUP_PLOT_YDATA", NULL);

    plt->_redraw = 1;
  }
  return iret;
}


void IupPPlotTransform(Ihandle* self, float x, float y, int *ix, int *iy)
{
  PPainterIup * plt = (PPainterIup *)iupGetEnv(self, PPLOTOBJ_);
  if (plt) 
  {
    if (ix) *ix = plt->_plot.Round(plt->_plot.mXTrafo->Transform(x));
    if (iy) *iy = plt->_plot.Round(plt->_plot.mYTrafo->Transform(y));
  }
}

/*
  user level call: plot on the given device
*/
void IupPPlotPaintTo(Ihandle *self, void* _cnv)
{
  char *pch;
  PPainterIup *plt;
  cdCanvas *cnv = (cdCanvas *)_cnv;

  pch = iupGetEnv(self, PPLOTOBJ_);
  plt = (PPainterIup *)pch;
  if (plt)
    plt->DrawTo(cnv);
}


/* --------------------------------------------------------------------
                      class implementation
   -------------------------------------------------------------------- */

typedef int (*Drawcb)(Ihandle*, cdCanvas*);
typedef int (*Deletecb)(Ihandle*, int, int, float, float);
typedef int (*Selectcb)(Ihandle*, int, int, float, float, int);
typedef int (*Editcb)(Ihandle*, int, int, float, float, float*, float*);

PostPainterCallbackIup::PostPainterCallbackIup (PPlot &inPPlot, Ihandle* inHandle):
  _handle(inHandle)
{
  inPPlot.mPostDrawerList.push_back (this);
}

bool PostPainterCallbackIup::Draw(Painter &inPainter)
{
  Drawcb cb = (Drawcb)IupGetCallback(_handle, "POSTDRAW_CB");
  if (cb)
  {
    PPainterIup* iupPainter = (PPainterIup*)(&inPainter);
    cb(_handle, iupPainter->_canvas_db);
  }

  return true;
}

PrePainterCallbackIup::PrePainterCallbackIup (PPlot &inPPlot, Ihandle* inHandle):
  _handle(inHandle)
{
  inPPlot.mPreDrawerList.push_back (this);
}

bool PrePainterCallbackIup::Draw(Painter &inPainter)
{
  Drawcb cb = (Drawcb)IupGetCallback(_handle, "PREDRAW_CB");
  if (cb)
  {
    PPainterIup* iupPainter = (PPainterIup*)(&inPainter);
    cb(_handle, iupPainter->_canvas_db);
  }

  return true;
}

bool PDeleteInteractionIup::DeleteNotify(int inIndex, int inSampleIndex, PlotDataBase *inXData, PlotDataBase *inYData)
{
  Deletecb cb = (Deletecb)IupGetCallback(_handle, "DELETE_CB");
  if (cb)
  {
    if (inIndex == -1)
    {
      Icallback cbb = IupGetCallback(_handle, "DELETEBEGIN_CB");
      if (cbb && cbb(_handle) == IUP_IGNORE)
        return false;
    }
    else if (inIndex == -2)
    {
      Icallback cbb = IupGetCallback(_handle, "DELETEEND_CB");
      if (cbb)
        cbb(_handle);
    }
    else
    {
      float theX = inXData->GetValue(inSampleIndex);
      float theY = inYData->GetValue(inSampleIndex);
      int ret = cb(_handle, inIndex, inSampleIndex, theX, theY);
      if (ret == IUP_IGNORE)
        return false;
    }
  }

  return true;
}

bool PSelectionInteractionIup::SelectNotify(int inIndex, int inSampleIndex, PlotDataBase *inXData, PlotDataBase *inYData, bool inSelect)
{
  Selectcb cb = (Selectcb)IupGetCallback(_handle, "SELECT_CB");
  if (cb)
  {
    if (inIndex == -1)
    {
      Icallback cbb = IupGetCallback(_handle, "SELECTBEGIN_CB");
      if (cbb && cbb(_handle) == IUP_IGNORE)
        return false;
    }
    else if (inIndex == -2)
    {
      Icallback cbb = IupGetCallback(_handle, "SELECTEND_CB");
      if (cbb)
        cbb(_handle);
    }
    else
    {
      float theX = inXData->GetValue(inSampleIndex);
      float theY = inYData->GetValue(inSampleIndex);
      int ret = cb(_handle, inIndex, inSampleIndex, theX, theY, (int)inSelect);
      if (ret == IUP_IGNORE)
        return false;
    }
  }

  return true;
}

bool PEditInteractionIup::Impl_HandleKeyEvent (const PKeyEvent &inEvent) {

  if (inEvent.IsArrowDown () || inEvent.IsArrowUp () ||
      inEvent.IsArrowLeft () || inEvent.IsArrowRight ()) {
    return true;
  }
  return false;
};

bool PEditInteractionIup::Impl_Calculate (Painter &inPainter, PPlot& inPPlot) {

  PlotDataContainer &theContainer = inPPlot.mPlotDataContainer;
  long thePlotCount = theContainer.GetPlotCount ();

  if (!EditNotify(-1, 0, 0, 0, NULL, NULL))
    return false;

  for (long theI=0;theI<thePlotCount;theI++) {
    PlotDataBase *theXData = theContainer.GetXData (theI);
    PlotDataBase *theYData = theContainer.GetYData (theI);
    PlotDataSelection *thePlotDataSelection = theContainer.GetPlotDataSelection (theI);

  if (mKeyEvent.IsArrowDown () || mKeyEvent.IsArrowUp () ||
      mKeyEvent.IsArrowLeft () || mKeyEvent.IsArrowRight ()) {
      HandleCursorKey (thePlotDataSelection, theXData, theYData, theI);
    }
  }

  EditNotify(-2, 0, 0, 0, NULL, NULL);

  return true;
}

void PEditInteractionIup::HandleCursorKey (const PlotDataSelection *inPlotDataSelection, PlotDataBase *inXData, PlotDataBase *inYData, int inIndex) {
  float theXDelta = 0; // pixels
  if (mKeyEvent.IsArrowLeft () || mKeyEvent.IsArrowRight ())
  {
    theXDelta = 1;
    if (mKeyEvent.IsArrowLeft ()) {
      theXDelta *= -1;
    }
    if (mKeyEvent.IsOnlyControlKeyDown ()) {
      theXDelta *= 10;
    }
  }

  float theYDelta = 0; // pixels
  if (mKeyEvent.IsArrowDown () || mKeyEvent.IsArrowUp ())
  {
    theYDelta = 1;
    if (mKeyEvent.IsArrowDown ()) {
      theYDelta *= -1;
    }
    if (mKeyEvent.IsOnlyControlKeyDown ()) {
      theYDelta *= 10;
    }
  }

  for (int theI=0;theI<inYData->GetSize ();theI++) {
    if (inPlotDataSelection->IsSelected (theI)) {

      float theX = inXData->GetValue(theI);
      float newX = theX;
      if (theXDelta)
      {
        float theXPixels = mPPlot.mXTrafo->Transform(theX);
        theXPixels += theXDelta;
        newX = mPPlot.mXTrafo->TransformBack(theXPixels);
      }

      float theY = inYData->GetValue(theI);
      float newY = theY;
      if (theYDelta)
      {
        float theYPixels = mPPlot.mYTrafo->Transform(theY);
        theYPixels -= theYDelta;  // in pixels Y is descending
        newY = mPPlot.mYTrafo->TransformBack(theYPixels);
      }

      if (!EditNotify(inIndex, theI, theX, theY, &newX, &newY))
        return;

      if (inXData->IsString())
      {
        StringData *theXData = (StringData*)(inXData);
        PlotData *theYData = (PlotData*)(inYData);
        theXData->mRealPlotData[theI] = newX;
        (*theYData)[theI] = newY;
      }
      else
      {
        PlotData *theXData = (PlotData*)(inXData);
        PlotData *theYData = (PlotData*)(inYData);
        (*theXData)[theI] = newX;
        (*theYData)[theI] = newY;
      }
    }
  }
}

bool PEditInteractionIup::EditNotify(int inIndex, int inSampleIndex, float inX, float inY, float *inNewX, float *inNewY)
{
  Editcb cb = (Editcb)IupGetCallback(_handle, "EDIT_CB");
  if (cb)
  {
    if (inIndex == -1)
    {
      Icallback cbb = IupGetCallback(_handle, "EDITBEGIN_CB");
      if (cbb && cbb(_handle) == IUP_IGNORE)
        return false;
    }
    else if (inIndex == -2)
    {
      Icallback cbb = IupGetCallback(_handle, "EDITEND_CB");
      if (cbb)
        cbb(_handle);
    }
    else
    {
      int ret = cb(_handle, inIndex, inSampleIndex, inX, inY, inNewX, inNewY);
      if (ret == IUP_IGNORE)
        return false;
    }
  }

  return true;
}

InteractionContainerIup::InteractionContainerIup(PPlot &inPPlot, Ihandle* inHandle):
  mZoomInteraction (inPPlot),
  mSelectionInteraction (inPPlot, inHandle),
  mEditInteraction (inPPlot, inHandle),
  mDeleteInteraction (inPPlot, inHandle),
  mCrosshairInteraction (inPPlot),
  mPostPainterCallback(inPPlot, inHandle),
  mPrePainterCallback(inPPlot, inHandle)
{
  AddInteraction (mZoomInteraction);
  AddInteraction (mSelectionInteraction);
  AddInteraction (mEditInteraction);
  AddInteraction (mDeleteInteraction);
  AddInteraction (mCrosshairInteraction);
}

PPainterIup::PPainterIup(Ihandle *hnd) : 
  Painter(),
  _handle(hnd),
  _canvas_iup(NULL),
  _canvas_db(NULL),
  _mouseDown(0),
  _currentDataSetIndex(-1),
  _redraw(1)
{
  _plot.mShowLegend = false; // change default to hidden
  _plot.mPlotBackground.mTransparent = false;  // always draw the background
  _plot.mMargins.mLeft = 15;
  _plot.mMargins.mBottom = 15;
  _plot.mMargins.mTop = 30;
  _plot.mMargins.mRight = 15;
  _plot.mXAxisSetup.mTickInfo.mTickDivision = 5;
  _plot.mYAxisSetup.mTickInfo.mTickDivision = 5;
  _plot.mXAxisSetup.mTickInfo.mMinorTickScreenSize = 5;
  _plot.mYAxisSetup.mTickInfo.mMinorTickScreenSize = 5;
  _plot.mXAxisSetup.mTickInfo.mMajorTickScreenSize = 8;
  _plot.mYAxisSetup.mTickInfo.mMajorTickScreenSize = 8;

  _InteractionContainer = new InteractionContainerIup(_plot, _handle);

} /* c-tor */


PPainterIup::~PPainterIup()
{
  if (_canvas_db != NULL) cdKillCanvas(_canvas_db);
  delete _InteractionContainer;
} /* d-tor */

static int get_CdFontSize(char* value)
{
  if (value)
  {
    int ii;
    sscanf(value, "%d", &ii);
    return ii;
  }
  else
    return 0;
}

static int get_CdFontStyle(char* value)
{
  if (!value)
    return -1;
  if (iupStrEqualNoCase(value, "PLAIN"))
    return CD_PLAIN;
  if (iupStrEqualNoCase(value, "BOLD"))
    return CD_BOLD;
  if (iupStrEqualNoCase(value, "ITALIC"))
    return CD_ITALIC;
  if (iupStrEqualNoCase(value, "BOLDITALIC"))
    return CD_BOLD_ITALIC;
  return -1;
}

static char* get_PlotFontSize(int size)
{
  if (size)
  {
    static char buffer[50];
    sprintf(buffer, "%d", size);
    return buffer;
  }
  else
    return NULL;
}

static char* get_PlotFontStyle(int style)
{
  if (style >= CD_PLAIN && style <= CD_BOLD_ITALIC)
  {
    char* style_str[4] = {"PLAIN", "BOLD", "ITALIC", "BOLDITALIC"};
    return style_str[style];
  }
  else
    return NULL;
}

static char* get_PlotPenStyle(int style)
{
  if (style >= CD_CONTINUOUS && style <= CD_DASH_DOT_DOT)
  {
    char* style_str[5] = {"CONTINUOUS", "DASHED", "DOTTED", "DASH_DOT", "DASH_DOT_DOT"};
    return style_str[style];
  }
  else
    return NULL;
}

static int get_CdPenStyle(char* value)
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
 
static char* get_PlotMarkStyle(int style)
{
  if (style >= CD_PLUS && style <= CD_HOLLOW_DIAMOND)
  {
    char* style_str[9] = {"PLUS", "STAR", "CIRCLE", "X", "BOX", "DIAMOND", "HOLLOW_CIRCLE", "HOLLOW_BOX", "HOLLOW_DIAMOND"};
    return style_str[style];
  }
  else
    return NULL;
}

static int get_CdMarkStyle(char* value)
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

class MarkDataDrawer: public LineDataDrawer {
 public:
   MarkDataDrawer (bool inDrawLine) { mDrawLine = inDrawLine; mDrawPoint = true; mMode = inDrawLine? "MARKLINE": "MARK"; };
   virtual bool DrawPoint (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const;
};

bool MarkDataDrawer::DrawPoint (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const {
  PPainterIup* painter = (PPainterIup*)&inPainter;
  cdCanvasMark(painter->_canvas_db, inScreenX, cdCanvasInvertYAxis(painter->_canvas_db, inScreenY));
  return true;
}

static void RemoveSample(PPlot& inPPlot, int inIndex, int inSampleIndex)
{
  PlotDataBase *theXDataBase = inPPlot.mPlotDataContainer.GetXData(inIndex);
  PlotDataBase *theYDataBase = inPPlot.mPlotDataContainer.GetYData(inIndex);
  if (theXDataBase->IsString())
  {
    StringData *theXData = (StringData *)theXDataBase;
    PlotData *theYData = (PlotData *)theYDataBase;
    theXData->mRealPlotData.erase(theXData->mRealPlotData.begin()+inSampleIndex);
    theXData->mStringData.erase(theXData->mStringData.begin()+inSampleIndex);
    theYData->erase(theYData->begin()+inSampleIndex);
  }
  else
  {
    PlotData *theXData = (PlotData *)theXDataBase;
    PlotData *theYData = (PlotData *)theYDataBase;
    theXData->erase(theXData->begin()+inSampleIndex);
    theYData->erase(theYData->begin()+inSampleIndex);
  }
}

void PPainterIup::SetAttribute(char* attr, char* value)
{
  int ii, imax;
  int rr, gg, bb;
  float xx;

  if (!attr) return;

  /******************************/
  /*     general attributes     */
  /******************************/
  if ( iupStrEqual(attr, "REDRAW") ) { /* refresh plot window */
    Draw(1);  /* force a full redraw here */
    return; // do not set redraw=1
  }
  else if ( iupStrEqual(attr, "LEGENDSHOW") ) { /* legend box visibility */
    if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
       _plot.mShowLegend = true;
    else 
       _plot.mShowLegend = false;
  }
  else if ( iupStrEqual(attr, "LEGENDPOS") ) { /* legend box position */
    if (iupStrEqualNoCase(value, "TOPLEFT"))
      _plot.mLegendPos = PPLOT_TOPLEFT;
    if (iupStrEqualNoCase(value, "BOTTOMLEFT"))
      _plot.mLegendPos = PPLOT_BOTTOMLEFT;
    if (iupStrEqualNoCase(value, "BOTTOMRIGHT"))
      _plot.mLegendPos = PPLOT_BOTTOMRIGHT;
    else
      _plot.mLegendPos = PPLOT_TOPRIGHT;
  }
  else if ( iupStrEqual(attr, "FONT") ) { /* global font */
    if (_canvas_db) 
    {
      cdCanvasNativeFont(_canvas_db, value);
      cdCanvasGetFont(_canvas_db, NULL, &_FontStyle, &_FontSize);
    }
  }
  else if ( iupStrEqual(attr, "BGCOLOR") ) { /* background color */
    sscanf(value, "%d %d %d", &rr, &gg, &bb);
    _plot.mPlotBackground.mPlotRegionBackColor = PColor(rr, gg, bb);
  }
  else if ( iupStrEqual(attr, "FGCOLOR") ) { /* title color */
    sscanf(value, "%d %d %d", &rr, &gg, &bb);
    _plot.mPlotBackground.mTitleColor = PColor(rr, gg, bb);
  }
  else if ( iupStrEqual(attr, "TITLE") ) { /* plot title */
    if (value && value[0]!=0)
       _plot.mPlotBackground.mTitle = value;
    else
       _plot.mPlotBackground.mTitle.resize(0);
  }
  else if ( iupStrEqual(attr, "TITLEFONTSIZE") ) { /* plot title font size */
    _plot.mPlotBackground.mStyle.mFontSize = get_CdFontSize(value);
  }
  else if ( iupStrEqual(attr, "TITLEFONTSTYLE") ) { /* plot title font style */
    _plot.mPlotBackground.mStyle.mFontStyle = get_CdFontStyle(value);
  }
  else if ( iupStrEqual(attr, "LEGENDFONTSIZE") ) { /* legend font size */
    for (ii=0; ii<_plot.mPlotDataContainer.GetPlotCount(); ii++)
    {
      LegendData* legend = _plot.mPlotDataContainer.GetLegendData(ii);
      legend->mStyle.mFontSize = get_CdFontSize(value);
    }
  }
  else if ( iupStrEqual(attr, "LEGENDFONTSTYLE") ) { /* legend font style */
    for (ii=0; ii<_plot.mPlotDataContainer.GetPlotCount(); ii++)
    {
      LegendData* legend = _plot.mPlotDataContainer.GetLegendData(ii);
      legend->mStyle.mFontStyle = get_CdFontStyle(value);
    }
  }
  else if ( iupStrEqualPartial(attr, "MARGIN") ) { /* plot margins */
    sscanf(value, "%d", &ii);
    attr += 6; /* strlen("MARGIN") */
    if ( iupStrEqual(attr, "LEFT") ) 
      _plot.mMargins.mLeft = ii;
    else if ( iupStrEqual(attr, "BOTTOM") ) 
      _plot.mMargins.mBottom = ii;
    else if ( iupStrEqual(attr, "TOP") ) 
      _plot.mMargins.mTop = ii;
    else if ( iupStrEqual(attr, "RIGHT") ) 
      _plot.mMargins.mRight = ii;
  }
  else if ( iupStrEqual(attr, "GRIDCOLOR") ) { /* plot grid color */
    sscanf(value, "%d %d %d", &rr, &gg, &bb);
    _plot.mGridInfo.mGridColor = PColor(rr, gg, bb);
  }
  else if ( iupStrEqual(attr, "GRIDLINESTYLE") ) { /* plot grid line style */
    _plot.mGridInfo.mStyle.mPenStyle = get_CdPenStyle(value);
  }
  else if ( iupStrEqual(attr, "GRID") ) { /* grid  */
    if (iupStrEqualNoCase(value, "VERTICAL")) { /* vertical grid - X axis  */
      _plot.mGridInfo.mXGridOn = true;
      _plot.mGridInfo.mYGridOn = false;
    }
    else if (iupStrEqualNoCase(value, "HORIZONTAL")) { /* horizontal grid - Y axis */
      _plot.mGridInfo.mYGridOn = true;
      _plot.mGridInfo.mXGridOn = false;
    }
    else if (iupStrEqualNoCase(value, "YES")) {
      _plot.mGridInfo.mXGridOn = true;
      _plot.mGridInfo.mYGridOn = true;
    }
    else {
      _plot.mGridInfo.mYGridOn = false;
      _plot.mGridInfo.mXGridOn = false;
    }
  }
  else if ( iupStrEqual(attr, "CURRENT") ) { /* current dataset index */
    sscanf(value, "%d", &ii);
    imax = _plot.mPlotDataContainer.GetPlotCount();
    _currentDataSetIndex = ( (ii>=0)&&(ii<imax) ? ii : -1);
  }
  else if ( iupStrEqual(attr, "REMOVE") ) { /* remove a dataset */
    sscanf(value, "%d", &ii);
    _plot.mPlotDataContainer.RemoveElement(ii);
  }
  else if ( iupStrEqual(attr, "CLEAR") ) {  /* remove all dataset */
    _plot.mPlotDataContainer.ClearData();
  }
  /******************************/
  /* current dataset attributes */
  /******************************/
  else if ( iupStrEqualPartial(attr, "DS_") ) { /* current plot */
    attr += 3; // strlen("DS_")
    if (_currentDataSetIndex<0 || _currentDataSetIndex>=_plot.mPlotDataContainer.GetPlotCount()) return;

    if ( iupStrEqual(attr, "LINESTYLE") ) { /* current plot line style */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      drawer->mStyle.mPenStyle = get_CdPenStyle(value);
    }
    else if ( iupStrEqual(attr, "LINEWIDTH") ) { /* current plot line width */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      sscanf(value, "%d", &ii);
      drawer->mStyle.mPenWidth = ii;
    }
    else if ( iupStrEqual(attr, "MARKSTYLE") ) { /* current plot mark style */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      drawer->mStyle.mMarkStyle = get_CdMarkStyle(value);
    }
    else if ( iupStrEqual(attr, "MARKSIZE") ) { /* current plot mark size */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      sscanf(value, "%d", &ii);
      drawer->mStyle.mMarkSize = ii;
    }
    else if ( iupStrEqual(attr, "LEGEND") ) { /* current dataset legend */
      LegendData* legend = _plot.mPlotDataContainer.GetLegendData(_currentDataSetIndex);
      if (value) legend->mName = value;
      else legend->mName.resize(0);
    }
    else if ( iupStrEqual(attr, "COLOR") ) { /* current dataset line and legend color */
      LegendData* legend = _plot.mPlotDataContainer.GetLegendData(_currentDataSetIndex);
      sscanf(value, "%d %d %d", &rr, &gg, &bb);
      legend->mColor = PColor(rr, gg, bb);
    }
    else if ( iupStrEqual(attr, "SHOWVALUES") ) { /* show values */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         drawer->mShowValues = true;
      else 
         drawer->mShowValues = false;
    }
    else if ( iupStrEqual(attr, "MODE") ) { /* current dataset drawing mode */
      DataDrawerBase *theDataDrawer = NULL;
      _plot.mXAxisSetup.mDiscrete = false;
      if (iupStrEqualNoCase(value, "BAR"))
      {
        theDataDrawer = new BarDataDrawer();
        _plot.mXAxisSetup.mDiscrete = true;
      }
      else if (iupStrEqualNoCase(value, "MARK"))
        theDataDrawer = new MarkDataDrawer(0);
      else if (iupStrEqualNoCase(value, "MARKLINE"))
        theDataDrawer = new MarkDataDrawer(1);
      else  // LINE
        theDataDrawer = new LineDataDrawer();

      _plot.mPlotDataContainer.SetDataDrawer(_currentDataSetIndex, theDataDrawer);
    }
    else if ( iupStrEqual(attr, "EDIT") ) { /* allows selection and editing */
      PlotDataSelection* dataselect = _plot.mPlotDataContainer.GetPlotDataSelection(_currentDataSetIndex);
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
        dataselect->resize(_plot.mPlotDataContainer.GetConstYData(_currentDataSetIndex)->GetSize());
      else
        dataselect->clear();
    }
    else if ( iupStrEqual(attr, "REMOVE") ) { /* remove a sample */
      sscanf(value, "%d", &ii);
      RemoveSample(_plot, _currentDataSetIndex, ii);
    }
  }
  /******************************/
  /*        axis props          */
  /******************************/
  else if ( iupStrEqualPartial(attr, "AXS_") ) { /* axis */
    attr += 4; // strlen("AXS_")
    AxisSetup* axis = NULL;
    if (*attr == 'X')
      axis = &_plot.mXAxisSetup;
    else if (*attr == 'Y')
      axis = &_plot.mYAxisSetup;
    else
      return;
    attr++;

    if ( iupStrEqual(attr, "LABEL") ) { /* axis title */
      if (value)
         axis->mLabel = value;
      else
         axis->mLabel = "";
    }
    else if ( iupStrEqual(attr, "LABELCENTERED") ) { /* axis title */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mLabelCentered = true;
      else 
         axis->mLabelCentered = false;
    }
    else if ( iupStrEqual(attr, "COLOR") ) { /* axis, ticks and label color */
      sscanf(value, "%d %d %d", &rr, &gg, &bb);
      axis->mColor = PColor(rr, gg, bb);
    }
    else if ( iupStrEqual(attr, "AUTOMIN") ) { /* autoscaling */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mAutoScaleMin = true;
      else 
         axis->mAutoScaleMin = false;
    }
    else if ( iupStrEqual(attr, "AUTOMAX") ) { /* autoscaling */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mAutoScaleMax = true;
      else 
         axis->mAutoScaleMax = false;
    }
    else if ( iupStrEqual(attr, "MIN") ) { /* min visible val */
      sscanf(value, "%f", &xx);
      axis->mMin = xx;
    }
    else if ( iupStrEqual(attr, "MAX") ) { /* max visible val */
      sscanf(value, "%f", &xx);
      axis->mMax = xx;
    }
    else if ( iupStrEqual(attr, "REVERSE") ) { /* values from left/top to right/bottom */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mAscending = false; // inverted
      else 
         axis->mAscending = true;
    }
    else if ( iupStrEqual(attr, "CROSSORIGIN") ) { /* axis mode */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mCrossOrigin = true;
      else 
         axis->mCrossOrigin = false;
    }
    else if ( iupStrEqual(attr, "SCALE") ) { /* log/lin scale */
      if (iupStrEqualNoCase(value, "LIN"))
         axis->mLogScale = false;
      else if ( iupStrEqualNoCase(value, "LOG10") ) {
         axis->mLogScale = true;
         axis->mLogBase = 10.0;
      }
      else if ( iupStrEqualNoCase(value, "LOG2") ) {
         axis->mLogScale = true;
         axis->mLogBase = 2.0;
      }
      else {
         axis->mLogScale = true;
         axis->mLogBase = (float)M_E;
      }
    }
    else if ( iupStrEqual(attr, "FONTSIZE") ) { /* axis label font size */
      axis->mStyle.mFontSize = get_CdFontSize(value);
    }
    else if ( iupStrEqual(attr, "FONTSTYLE") ) { /* axis label font style */
      axis->mStyle.mFontStyle = get_CdFontStyle(value);
    }
    else if ( iupStrEqual(attr, "AUTOTICK") ) { /* auto tick spacing */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mTickInfo.mAutoTick = true;
      else 
         axis->mTickInfo.mAutoTick = false;
    }
    else if ( iupStrEqual(attr, "TICKDIVISION") ) { /* number of ticks between major ticks */
      sscanf(value, "%d", &ii);
      axis->mTickInfo.mTickDivision = ii;
    }
    else if ( iupStrEqual(attr, "TICKMAJORSPAN") ) { /* major tick spacing */
      sscanf(value, "%f", &xx);
      axis->mTickInfo.mMajorTickSpan = xx;
    }
    else if ( iupStrEqual(attr, "AUTOTICKSIZE") ) { /* automatic tick size */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
         axis->mTickInfo.mAutoTickSize = true;
      else 
         axis->mTickInfo.mAutoTickSize = false;
    }
    else if ( iupStrEqual(attr, "TICKSIZE") ) { /* size of ticks (in pixels) */
      sscanf(value, "%d", &ii);
      axis->mTickInfo.mMinorTickScreenSize = ii;
    }
    else if ( iupStrEqual(attr, "TICKMAJORSIZE") ) { /* size of major ticks (in pixels) */
      sscanf(value, "%d", &ii);
      axis->mTickInfo.mMajorTickScreenSize = ii;
    }
    else if ( iupStrEqual(attr, "TICKFONTSIZE") ) { /* axis ticks font size */
      axis->mTickInfo.mStyle.mFontSize = get_CdFontSize(value);
    }
    else if ( iupStrEqual(attr, "TICKFONTSTYLE") ) { /* axis ticks number font style */
      axis->mTickInfo.mStyle.mFontStyle = get_CdFontStyle(value);
    }
    else if ( iupStrEqual(attr, "TICKFORMAT") ) { /* axis ticks number format */
      if (value && value[0]!=0)
        axis->mTickInfo.mFormatString = value;
      else
        axis->mTickInfo.mFormatString = "%.0f";
    }
    else if ( iupStrEqual(attr, "TICK") ) { /* axis ticks  */
      if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
        axis->mTickInfo.mTicksOn = true;
      else 
        axis->mTickInfo.mTicksOn = false;
    }
  }
  /******************************/
  /*     other attributes       */
  /******************************/
  else {
    iupCpiDefaultSetAttr(_handle, attr, value);
    return; // do not set redraw=1
  }

  _redraw = 1;
} /* SetAttribute */


char * PPainterIup::GetAttribute(char *attr)
{
  if (!attr) return NULL;

  /******************************/
  /*      general attributes    */
  /******************************/
  if ( iupStrEqual(attr, "LEGENDSHOW") ) { /* legend box visibility */
    if (_plot.mShowLegend)
      return "YES";
    else
      return "NO";
  }
  else if ( iupStrEqual(attr, "LEGENDPOS") ) { /* legend box position */
    char* legendpos_str[4] = {"TOPLEFT", "TOPRIGHT", "BOTTOMLEFT", "BOTTOMRIGHT"};
    return legendpos_str[_plot.mLegendPos];
  }
  else if ( iupStrEqual(attr, "BGCOLOR") ) { /* background color */
    sprintf(_att_buffer, "%d %d %d",
            _plot.mPlotBackground.mPlotRegionBackColor.mR,
            _plot.mPlotBackground.mPlotRegionBackColor.mG,
            _plot.mPlotBackground.mPlotRegionBackColor.mB);
    return _att_buffer;
  }
  else if ( iupStrEqual(attr, "FGCOLOR") ) { /* title color */
    sprintf(_att_buffer, "%d %d %d",
            _plot.mPlotBackground.mTitleColor.mR,
            _plot.mPlotBackground.mTitleColor.mG,
            _plot.mPlotBackground.mTitleColor.mB);
    return _att_buffer;
  }
  else if ( iupStrEqual(attr, "TITLE") ) { /* plot title */
    strncpy(_att_buffer, _plot.mPlotBackground.mTitle.c_str(), 127);
    _att_buffer[127]='\0';
    return _att_buffer;
  }
  else if ( iupStrEqual(attr, "TITLEFONTSIZE") ) { /* plot title font size */
    return get_PlotFontSize(_plot.mPlotBackground.mStyle.mFontSize);
  }
  else if ( iupStrEqual(attr, "TITLEFONTSTYLE") ) { /* plot title font style */
    return get_PlotFontStyle(_plot.mPlotBackground.mStyle.mFontStyle);
  }
  // LEGENDFONTSIZE and LEGENDFONTSTYLE returns the hash table value
  else if ( iupStrEqualPartial(attr, "MARGIN") ) { /* plot margins */
    int ii, 
        len = 6; /* strlen("MARGIN") */
    if ( iupStrEqual(attr+len, "LEFT") ) 
      ii = _plot.mMargins.mLeft;
    else if ( iupStrEqual(attr+len, "BOTTOM") ) 
      ii = _plot.mMargins.mBottom;
    else if ( iupStrEqual(attr+len, "TOP") ) 
      ii = _plot.mMargins.mTop;
    else if ( iupStrEqual(attr+len, "RIGHT") ) 
      ii = _plot.mMargins.mRight;
    else
      return NULL;

    sprintf(_att_buffer, "%d", ii);
    return _att_buffer;
  }
  else if ( iupStrEqual(attr, "GRIDCOLOR") ) { /* plot grid color */
    sprintf(_att_buffer, "%d %d %d",
            _plot.mGridInfo.mGridColor.mR,
            _plot.mGridInfo.mGridColor.mG,
            _plot.mGridInfo.mGridColor.mB);
    return _att_buffer;
  }
  else if ( iupStrEqual(attr, "GRIDLINESTYLE") ) { /* grid line style */
    return get_PlotPenStyle(_plot.mGridInfo.mStyle.mPenStyle);
  }
  else if ( iupStrEqual(attr, "GRID") ) { /* grid  */
    if (_plot.mGridInfo.mXGridOn && _plot.mGridInfo.mYGridOn)
      return "YES";
    else if (_plot.mGridInfo.mYGridOn)
      return "HORIZONTAL";
    else if (_plot.mGridInfo.mXGridOn)
      return "VERTICAL";
    else
      return "NO";
  }
  else if ( iupStrEqual(attr, "COUNT") ) { /* total number of datasets */
    sprintf(_att_buffer, "%d", _plot.mPlotDataContainer.GetPlotCount());
    return _att_buffer;
  }
  else if ( iupStrEqual(attr, "CURRENT") ) { /* current dataset index */
    sprintf(_att_buffer, "%d", _currentDataSetIndex);
    return _att_buffer;
  }
  /******************************/
  /* current dataset attributes */
  /******************************/
  else if ( iupStrEqualPartial(attr, "DS_") ) { /* current plot */
    attr += 3; // strlen("DS_")
    if (_currentDataSetIndex<0 || _currentDataSetIndex>=_plot.mPlotDataContainer.GetPlotCount()) return NULL;

    if ( iupStrEqual(attr, "LINESTYLE") ) { /* current plot line style */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      return get_PlotPenStyle(drawer->mStyle.mPenStyle);
    }
    else if ( iupStrEqual(attr, "LINEWIDTH") ) { /* current plot line width */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      sprintf(_att_buffer, "%d", drawer->mStyle.mPenWidth);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "MARKSTYLE") ) { /* current plot mark style */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      return get_PlotMarkStyle(drawer->mStyle.mMarkStyle);
    }
    else if ( iupStrEqual(attr, "MARKSIZE") ) { /* current plot mark size */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      sprintf(_att_buffer, "%d", drawer->mStyle.mMarkSize);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "LEGEND") ) { /* current dataset legend */
      LegendData* legend = _plot.mPlotDataContainer.GetLegendData(_currentDataSetIndex);
      strncpy(_att_buffer, legend->mName.c_str(), 127);
      _att_buffer[127]='\0';
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "SHOWVALUES") ) { /* show values */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      if (drawer->mShowValues)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "MODE") ) { /* current dataset drawing mode */
      DataDrawerBase* drawer = _plot.mPlotDataContainer.GetDataDrawer(_currentDataSetIndex);
      return (char*)drawer->mMode;
    }
    else if ( iupStrEqual(attr, "EDIT") ) { /* allows selection and editing */
      PlotDataSelection* dataselect = _plot.mPlotDataContainer.GetPlotDataSelection(_currentDataSetIndex);
      if (dataselect->empty())
        return "NO";
      else
        return "YES";
    }
    else if ( iupStrEqual(attr, "COLOR") ) { /* current dataset line and legend color */
      LegendData* legend = _plot.mPlotDataContainer.GetLegendData(_currentDataSetIndex);
      sprintf(_att_buffer, "%d %d %d", legend->mColor.mR,
              legend->mColor.mG, legend->mColor.mB);
      return _att_buffer;
    }
  }
  /******************************/
  /*         axis props         */
  /******************************/
  else if ( iupStrEqualPartial(attr, "AXS_") ) { /* axis */
    attr += 4; // strlen("AXS_")
    AxisSetup* axis = NULL;
    if (*attr == 'X')
      axis = &_plot.mXAxisSetup;
    else if (*attr == 'Y')
      axis = &_plot.mYAxisSetup;
    else
      return NULL;
    attr++;

    if ( iupStrEqual(attr, "LABEL") ) { /* axis title */
      strncpy(_att_buffer, axis->mLabel.c_str(), 127);
      _att_buffer[127]='\0';
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "LABELCENTERED") ) { /* axis title position */
      if (axis->mLabelCentered)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "COLOR") ) { /* axis, ticks and label color */
      sprintf(_att_buffer, "%d %d %d",
              axis->mColor.mR,
              axis->mColor.mG,
              axis->mColor.mB);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "AUTOMIN") ) { /* autoscaling */
      if (axis->mAutoScaleMin)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "AUTOMAX") ) { /* autoscaling */
      if (axis->mAutoScaleMax)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "MIN") ) { /* min visible val */
      sprintf(_att_buffer, "%g", axis->mMin);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "MAX") ) { /* max visible val */
      sprintf(_att_buffer, "%g", axis->mMax);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "REVERSE") ) { /* values from left/top to right/bottom */
      if (axis->mAscending)
        return "NO";    // inverted
      else
        return "YES";
    }
    else if ( iupStrEqual(attr, "CROSSORIGIN") ) { /* axis crossing the origin */
      if (axis->mCrossOrigin)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "SCALE") ) { /* log/lin scale */
      if (axis->mLogScale) {
        if (axis->mLogBase == 10.0)
          strcpy(_att_buffer, "LOG10");
        else if (axis->mLogBase == 2.0)
          strcpy(_att_buffer, "LOG2");
        else
          strcpy(_att_buffer, "LOGN");
      }
      else
        strcpy(_att_buffer, "LIN");
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "FONTSIZE") ) { /* axis label font size */
      return get_PlotFontSize(axis->mStyle.mFontSize);
    }
    else if ( iupStrEqual(attr, "FONTSTYLE") ) { /* axis label font style */
      return get_PlotFontStyle(axis->mStyle.mFontStyle);
    }
    else if ( iupStrEqual(attr, "AUTOTICK") ) { /* auto tick spacing */
      if (axis->mTickInfo.mAutoTick)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "TICKDIVISION") ) { /* number of ticks between major ticks */
      sprintf(_att_buffer, "%d", axis->mTickInfo.mTickDivision);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "TICKMAJORSPAN") ) { /* major tick spacing */
      sprintf(_att_buffer, "%g", axis->mTickInfo.mMajorTickSpan);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "TICKFONTSIZE") ) { /* axis ticks font size */
      return get_PlotFontSize(axis->mTickInfo.mStyle.mFontSize);
    }
    else if ( iupStrEqual(attr, "TICKFONTSTYLE") ) { /* axis ticks font style */
      return get_PlotFontStyle(axis->mTickInfo.mStyle.mFontStyle);
    }
    else if ( iupStrEqual(attr, "AUTOTICKSIZE") ) { /* automatic tick size */
      if (axis->mTickInfo.mAutoTickSize)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "TICKSIZE") ) { /* size of ticks (in pixels) */
      sprintf(_att_buffer, "%d", axis->mTickInfo.mMinorTickScreenSize);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "TICKMAJORSIZE") ) { /* size of major ticks (in pixels) */
      sprintf(_att_buffer, "%d", axis->mTickInfo.mMajorTickScreenSize);
      return _att_buffer;
    }
    else if ( iupStrEqual(attr, "TICK") ) { /* axis ticks */
      if (axis->mTickInfo.mTicksOn)
        return "YES";
      else
        return "NO";
    }
    else if ( iupStrEqual(attr, "TICKFORMAT") ) { /* axis ticks number format */
      strncpy(_att_buffer, axis->mTickInfo.mFormatString.c_str(), 127);
      _att_buffer[127]='\0';
      return _att_buffer;
    }
  }
  /******************************/
  /*     other attributes       */
  /******************************/

  return iupCpiDefaultGetAttr(_handle, attr);
} /* GetAttribute */


void PPainterIup::MouseButton(int btn, int stat, int x, int y, char *r)
{
  PMouseEvent theEvent;
  int theModifierKeys = 0;

  theEvent.mX = x;
  theEvent.mY = y;
  
  if (btn == IUP_BUTTON1) {
    theEvent.mType = ( stat!=0 ? (PMouseEvent::kDown) : (PMouseEvent::kUp) );
    _mouseDown = ( stat!=0 ? 1 : 0 );
  }
  else return;

  _mouse_ALT = 0;
  _mouse_SHIFT = 0;
  _mouse_CTRL = 0;

  /* TODO: Alt to be implemented here in IUP 3.0 
     BUT: PPlot is not using it, so it is not a problem.
  */

  if (0) {  /* signal Alt */
    theModifierKeys = (theModifierKeys | PMouseEvent::kAlt);
    _mouse_ALT = 1;
  }
  if (iscontrol(r)) { /* signal Ctrl */
    theModifierKeys = (theModifierKeys | PMouseEvent::kControl);
    _mouse_SHIFT = 1;
  }
  if (isshift(r)) { /* signal Shift */
    theModifierKeys = (theModifierKeys | PMouseEvent::kShift);
    _mouse_CTRL = 1;
  }
  theEvent.SetModifierKeys (theModifierKeys);

  if ( _InteractionContainer->HandleMouseEvent(theEvent) ) {
    this->Draw(1);
  } 
  else {
    /* ignore the event */
  }
} /* MouseButton */


void PPainterIup::MouseMove(int x, int y)
{
  PMouseEvent theEvent;
  int theModifierKeys = 0;

  if (!_mouseDown ) return;

  theEvent.mX = x;
  theEvent.mY = y;

  theEvent.mType = PMouseEvent::kMove;
  if (_mouse_ALT) {  /* signal Alt */
    theModifierKeys = (theModifierKeys | PMouseEvent::kAlt);
  }
  if (_mouse_SHIFT) { /* signal Shift */
    theModifierKeys = (theModifierKeys | PMouseEvent::kControl);
  }
  if (_mouse_CTRL) { /* signal Ctrl */
    theModifierKeys = (theModifierKeys | PMouseEvent::kShift);
  }
  theEvent.SetModifierKeys (theModifierKeys);

  if ( _InteractionContainer->HandleMouseEvent(theEvent) ) {
    this->Draw(1);
  } 
  else {
    /* ignore the event */
  }
} /* MouseMove */


void PPainterIup::KeyPress(int c, int press)
{
  int theModifierKeys = 0;
  int theRepeatCount = 0;
  PKeyEvent::EKey theKeyCode = PKeyEvent::kNone;
  char theChar = 0;

  if (!press) return;

  switch (c) {
    case K_cX: // CTRL + X
      theModifierKeys = PMouseEvent::kControl;
      theKeyCode = PKeyEvent::kChar;
      theChar = 'x';
    break;
    case K_cY: // CTRL + Y
      theModifierKeys = PMouseEvent::kControl;
      theKeyCode = PKeyEvent::kChar;
      theChar = 'y';
    break;
    case K_cR: // CTRL + R
      theModifierKeys = PMouseEvent::kControl;
      theKeyCode = PKeyEvent::kChar;
      theChar = 'r';
    break;
    case K_cUP: // CTRL + Arrow
      theModifierKeys = PMouseEvent::kControl;
    case K_UP:  // Arrow
      theKeyCode = PKeyEvent::kArrowUp;
    break;
    case K_cDOWN: // CTRL + Arrow
      theModifierKeys = PMouseEvent::kControl;
    case K_DOWN:  // Arrow
      theKeyCode = PKeyEvent::kArrowDown;
    break;
    case K_cLEFT: // CTRL + Arrow
      theModifierKeys = PMouseEvent::kControl;
    case K_LEFT:  // Arrow
      theKeyCode = PKeyEvent::kArrowLeft;
    break;
    case K_cRIGHT: // CTRL + Arrow
      theModifierKeys = PMouseEvent::kControl;
    case K_RIGHT:  // Arrow
      theKeyCode = PKeyEvent::kArrowRight;
    break;
    case K_cDEL: // CTRL + Arrow
      theModifierKeys = PMouseEvent::kControl;
    case K_DEL:  // Arrow
      theKeyCode = PKeyEvent::kDelete;
    break;
  }

  PKeyEvent theEvent (theKeyCode, theRepeatCount, theModifierKeys, theChar);

  if ( _InteractionContainer->HandleKeyEvent(theEvent) ) {
    this->Draw(1);
  } 
  else {
    /* ignore the event */
  }
} /* KeyPress */


void PPainterIup::CreateCanvas()
{
  int old_gdi = 0;

  if (IupGetInt(_handle, "USE_GDI+"))
    old_gdi = cdUseContextPlus(1);

  if (!_canvas_iup)
    _canvas_iup = cdCreateCanvas(CD_IUP, _handle);
  if (!_canvas_iup) return;

  if (IupGetInt(_handle, "USE_IMAGERGB"))
    _canvas_db = cdCreateCanvas(CD_DBUFFERRGB, _canvas_iup);
  else
    _canvas_db = cdCreateCanvas(CD_DBUFFER, _canvas_iup);

  if (!_canvas_db) return;

  if (IupGetInt(_handle, "USE_GDI+"))
    cdUseContextPlus(old_gdi);

  // Update the CD font from the FONT attribute
  cdCanvasNativeFont(_canvas_db, IupGetAttribute(_handle, "FONT"));
  cdCanvasGetFont(_canvas_db, NULL, &_FontStyle, &_FontSize);

  _redraw = 1;
}

void PPainterIup::Draw(int force)
{
  if (!_canvas_db)
    CreateCanvas();

  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasActivate(_canvas_db);

  if (force || _redraw)
  {
    cdCanvasClear(_canvas_db);
    _plot.Draw(*this);
    _redraw = 0;
  }

  cdCanvasFlush(_canvas_db);
} /* Draw */


void PPainterIup::Resize()
{
  if (!_canvas_db)
    CreateCanvas();

  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasActivate(_canvas_db);
  _redraw = 1;
} /* Resize */


/* send plot to some other device */ 
void PPainterIup::DrawTo(cdCanvas *usrCnv)
{
  cdCanvas *old_canvas_db = _canvas_db;
  cdCanvas *old_canvas_iup = _canvas_iup;
  _canvas_iup = _canvas_db = usrCnv;

  assert(_canvas_db);
  if (!_canvas_db) return;

  // Update the CD font from the FONT attribute
  cdCanvasNativeFont(_canvas_db, IupGetAttribute(_handle, "FONT"));
  cdCanvasGetFont(_canvas_db, NULL, &_FontStyle, &_FontSize);

  Draw(1);

  _canvas_db = old_canvas_db;
  _canvas_iup = old_canvas_iup;
}


void PPainterIup::FillArrow(int inX1, int inY1, int inX2, int inY2, int inX3, int inY3)
{
  assert(_canvas_db);
  if (!_canvas_db) return;
  cdCanvasBegin(_canvas_db, CD_FILL);
  cdCanvasVertex(_canvas_db, inX1, cdCanvasInvertYAxis(_canvas_db, inY1));
  cdCanvasVertex(_canvas_db, inX2, cdCanvasInvertYAxis(_canvas_db, inY2));
  cdCanvasVertex(_canvas_db, inX3, cdCanvasInvertYAxis(_canvas_db, inY3));
  cdCanvasEnd(_canvas_db);
}

void PPainterIup::DrawLine(float inX1, float inY1, float inX2, float inY2)
{
  assert(_canvas_db);
  if (!_canvas_db) return;
  cdfCanvasLine(_canvas_db, inX1, cdfCanvasInvertYAxis(_canvas_db, inY1), inX2, cdfCanvasInvertYAxis(_canvas_db, inY2));
} /* DrawLine */


void PPainterIup::FillRect(int inX, int inY, int inW, int inH)
{
  assert(_canvas_db);
  if (!_canvas_db) return;
  cdCanvasBox(_canvas_db, inX, inX+inW, cdCanvasInvertYAxis(_canvas_db, inY), cdCanvasInvertYAxis(_canvas_db, inY+inH-1));
} /* FillRect */


void PPainterIup::InvertRect(int inX, int inY, int inW, int inH)
{
  long cprev;

  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasWriteMode(_canvas_db, CD_XOR);
  cprev = cdCanvasForeground(_canvas_db, CD_WHITE);
  cdCanvasRect(_canvas_db, inX, inX+inW-1, cdCanvasInvertYAxis(_canvas_db, inY), cdCanvasInvertYAxis(_canvas_db, inY+inH-1));
  cdCanvasWriteMode(_canvas_db, CD_REPLACE);
  cdCanvasForeground(_canvas_db, cprev);
} /* InvertRect */


void PPainterIup::SetClipRect(int inX, int inY, int inW, int inH)
{
  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasClipArea(_canvas_db, inX, inX+inW-1, cdCanvasInvertYAxis(_canvas_db, inY), cdCanvasInvertYAxis(_canvas_db, inY+inH-1));
  cdCanvasClip(_canvas_db, CD_CLIPAREA);
} /* SetClipRect */


long PPainterIup::GetWidth() const
{
  int iret;

  assert(_canvas_db);
  if (!_canvas_db) return 0;

  cdCanvasGetSize(_canvas_db, &iret, NULL, NULL, NULL);

  return (long)iret;
} /* GetWidth */


long PPainterIup::GetHeight() const
{
  int iret;

  assert(_canvas_db);
  if (!_canvas_db) return 0;

  cdCanvasGetSize(_canvas_db, NULL, &iret, NULL, NULL);

  return (long)iret;
} /* GetHeight */


void PPainterIup::SetLineColor(int inR, int inG, int inB)
{
  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasForeground(_canvas_db, cdEncodeColor((unsigned char)inR,
                                               (unsigned char)inG,
                                               (unsigned char)inB));
} /* SetLineColor */


void PPainterIup::SetFillColor(int inR, int inG, int inB)
{
  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasForeground(_canvas_db, cdEncodeColor((unsigned char)inR,
                                               (unsigned char)inG,
                                               (unsigned char)inB));
} /* SetFillColor */


long PPainterIup::CalculateTextDrawSize(const char *inString)
{
  int iw;

  assert(_canvas_db);
  if (!_canvas_db) return 0;

  cdCanvasGetTextSize(_canvas_db, const_cast<char *>(inString), &iw, NULL);

  return iw;
} /* CalculateTextDrawSize */


long PPainterIup::GetFontHeight() const
{
  int ih;

  assert(_canvas_db);
  if (!_canvas_db) return 0;

  cdCanvasGetFontDim(_canvas_db, NULL, &ih, NULL, NULL);

  return ih;
} /* GetFontHeight */


// this call leave all the hard job of alignment on painter side 
void PPainterIup::DrawText(int inX, int inY, short align, const char *inString)
{
  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasTextAlignment(_canvas_db, align);
  cdCanvasText(_canvas_db, inX, cdCanvasInvertYAxis(_canvas_db, inY), const_cast<char *>(inString));
} /* DrawText */

void PPainterIup::DrawRotatedText(int inX, int inY, float inDegrees,
                               short align, const char *inString)
{
  double aprev;

  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasTextAlignment(_canvas_db, align);
  aprev = cdCanvasTextOrientation(_canvas_db, -inDegrees);
  cdCanvasText(_canvas_db, inX, cdCanvasInvertYAxis(_canvas_db, inY), const_cast<char *>(inString));
  cdCanvasTextOrientation(_canvas_db, aprev);
} /* DrawRotatedText */

void PPainterIup::SetStyle(const PStyle &inStyle)
{
  assert(_canvas_db);
  if (!_canvas_db) return;

  cdCanvasLineWidth(_canvas_db, inStyle.mPenWidth);
  cdCanvasLineStyle(_canvas_db, inStyle.mPenStyle);

  int style;
  if (inStyle.mFontStyle == -1) style = _FontStyle;       // if not defined gets the global value
  else                          style = inStyle.mFontStyle;

  int size;
  if (inStyle.mFontSize == 0) size = _FontSize;           // if not defined gets the global value
  else                        size = inStyle.mFontSize;
  
  cdCanvasFont(_canvas_db, NULL, style, size);

  cdCanvasMarkType(_canvas_db, inStyle.mMarkStyle);
  cdCanvasMarkSize(_canvas_db, inStyle.mMarkSize);
}

