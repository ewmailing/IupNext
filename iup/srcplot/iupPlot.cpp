
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iupcbs.h"



double iupPlotTrafoLinear::Transform(double inValue) const 
{
  return inValue * mSlope + mOffset;
}

double iupPlotTrafoLinear::TransformBack(double inValue) const 
{
  if (mSlope != 0)
    return (inValue - mOffset) / mSlope;
  else
    return 0;
}

bool iupPlotTrafoLinear::Calculate(int inBegin, int inEnd, const iupPlotAxis& inAxis)
{
  double theDataRange = inAxis.mMax - inAxis.mMin;
  if (theDataRange < kFloatSmall)
    return false;

  double theMin = inAxis.mMin;
  if (inAxis.mDiscrete)
  {
    theDataRange++;
    theMin -= 0.5;
  }

  double theTargetRange = inEnd - inBegin;
  double theScale = theTargetRange / theDataRange;

  if (!inAxis.mReverse)
    mOffset = inBegin - theMin * theScale;
  else
    mOffset = inEnd + theMin * theScale;

  mSlope = theScale;
  if (inAxis.mReverse)
    mSlope *= -1;

  return true;
}


/************************************************************************************************/


double iupPlotTrafoLog::Transform(double inValue) const
{
  if (inValue<kLogMinClipValue) inValue = kLogMinClipValue;
  return iupPlotLog(inValue, mBase)*mSlope + mOffset;
}

double iupPlotTrafoLog::TransformBack(double inValue) const 
{
  if (mSlope != 0)
    return iupPlotExp((inValue - mOffset) / mSlope, mBase);
  else
    return 0;
}

bool iupPlotTrafoLog::Calculate(int inBegin, int inEnd, const iupPlotAxis& inAxis)
{
  double theBase = inAxis.mLogBase;
  double theDataRange = iupPlotLog(inAxis.mMax, theBase) - iupPlotLog(inAxis.mMin, theBase);
  if (theDataRange < kFloatSmall)
    return false;

  double theTargetRange = inEnd - inBegin;
  double theScale = theTargetRange / theDataRange;

  if (!inAxis.mReverse)
    mOffset = inBegin - iupPlotLog(inAxis.mMin, theBase) * theScale;
  else
    mOffset = inEnd + iupPlotLog(inAxis.mMin, theBase) * theScale;

  mBase = theBase;

  mSlope = theScale;
  if (inAxis.mReverse)
    mSlope *= -1;

  return true;
}


/************************************************************************************************/


iupPlotDataString::~iupPlotDataString()
{
  int theCount = iupArrayCount(mArray);
  char** theData = (char**)iupArrayGetData(mArray);
  for (int i = 0; i < theCount; i++) 
    free(theData[i]);
}

bool iupPlotDataString::CalculateRange(double &outMin, double &outMax)
{
  int theCount = iupArrayCount(mArray);
  if (theCount > 0)
  {
    outMin = 0;
    outMax = theCount-1;
    return true;
  }

  return false;
}


/************************************************************************************************/


bool iupPlotDataReal::CalculateRange(double &outMin, double &outMax)
{
  int theCount = iupArrayCount(mArray);
  if (theCount > 0)
  {
    double* theData = (double*)iupArrayGetData(mArray);
    outMax = outMin = theData[0];
    for (int i = 1; i < theCount; i++)
    {
      if (theData[i] > outMax)
        outMax = theData[i];
      if (theData[i] < outMin)
        outMin = theData[i];
    }
    return true;
  }

  return false;
}


/************************************************************************************************/


iupPlotDataSet::iupPlotDataSet(iupPlotDataBase* inDataX, iupPlotDataBase* inDataY, const char* inLegend, long inColor)
  :mColor(inColor), mLineStyle(CD_CONTINUOUS), mLineWidth(1), mMarkStyle(CD_X), mMarkSize(7),
  mDataX(inDataX), mDataY(inDataY), mMode(IUP_PLOT_LINE), mName(NULL)
{
  SetName(inLegend);
}

iupPlotDataSet::~iupPlotDataSet()
{
  SetName(NULL);
  delete[] mDataX;
  delete[] mDataY;
}


/************************************************************************************************/


void iupPlotAxis::Init()
{
  if (mLogScale)
  {
    mTickIter = &mLogTickIter;
    mTrafo = &mLogTrafo;
  }
  else
  {
    mTickIter = &mLinTickIter;
    mTrafo = &mLinTrafo;
  }

  mTickIter->SetAxis(this);
}

void iupPlotAxis::SetNamedTickIter(const iupPlotDataString *inStringData)
{
  mTickIter = &mNamedTickIter;
  mTickIter->SetAxis(this);
  mNamedTickIter.SetStringList(inStringData);
}

void iupPlotAxis::CheckZoomOutLimit(double inRange)
{
  if (mMin < mNoZoomMin)
  {
    mMin = mNoZoomMin;
    mMax = mMin + inRange;
    if (mMax > mNoZoomMax)
      mMax = mNoZoomMax;
  }

  if (mMax > mNoZoomMax)
  {
    mMax = mNoZoomMax;
    mMin = mMax - inRange;
    if (mMin < mNoZoomMin)
      mMin = mNoZoomMin;
  }
}

void iupPlotAxis::InitZoom()
{
  if (!mHasZoom)
  {
    mHasZoom = true;
    mAutoScaleMin = false;
    mAutoScaleMax = false;

    mNoZoomMin = mMin;
    mNoZoomMax = mMax;
    mNoZoomAutoScaleMin = mAutoScaleMin;
    mNoZoomAutoScaleMax = mAutoScaleMax;
  }
}

bool iupPlotAxis::ResetZoom()
{
  if (mHasZoom)
  {
    mHasZoom = false;

    mMin = mNoZoomMin;
    mMax = mNoZoomMax;
    mAutoScaleMin = mNoZoomAutoScaleMin;
    mAutoScaleMax = mNoZoomAutoScaleMax;
    return true;
  }

  return false;
}

bool iupPlotAxis::ZoomOut(double inCenter)
{
  if (inCenter < mMin || inCenter > mMax)
    return false;

  if (!mHasZoom)
    return false;

  double theRange = mMax - mMin;
  double theNewRange = theRange * 1.1; // 10%
  double theFactor = (inCenter - mMin) / theRange;
  double theOffset = (theNewRange - theRange);

  mMin -= theOffset*theFactor;
  mMax += theOffset*(1.0 - theFactor);

  CheckZoomOutLimit(theNewRange);

  if (mMin == mNoZoomMin && mMax == mNoZoomMax)
    ResetZoom();

  return true;
}

bool iupPlotAxis::ZoomIn(double inCenter)
{
  if (inCenter < mMin || inCenter > mMax)
    return false;

  InitZoom();

  double theRange = mMax - mMin;
  double theNewRange = theRange * 0.9; // 10%
  double theFactor = (inCenter - mMin) / theRange;
  double theOffset = (theRange - theNewRange);

  mMin += theOffset*theFactor;
  mMax -= theOffset*(1.0 - theFactor);

  // Check limits only in ZoomOut and Pan
  return true;
}

bool iupPlotAxis::ZoomTo(double inMin, double inMax)
{
  InitZoom();

  if (inMin > inMax) { double tmp = inMin; inMin = inMax; inMax = tmp; }

  if (inMin < mNoZoomMin || inMin > mNoZoomMax ||
      inMax < mNoZoomMin || inMax > mNoZoomMax)
  {
    if (mMin == mNoZoomMin && mMax == mNoZoomMax)
      ResetZoom();

    return false;
  }

  mMin = inMin;
  mMax = inMax;

  if (mMin == mNoZoomMin && mMax == mNoZoomMax)
    ResetZoom();

  return true;
}

bool iupPlotAxis::Pan(double inOffset)
{
  if (!mHasZoom)
    return false;

  double theRange = mMax - mMin;

  mMin = mPanMin - inOffset;
  mMax = mMin + theRange;

  CheckZoomOutLimit(theRange);
  return true;
}

bool iupPlotAxis::Scroll(double inDelta, bool inFullPage)
{
  if (!mHasZoom)
    return false;

  double theRange = mMax - mMin;

  double thePage;
  if (inFullPage)
    thePage = theRange;
  else
    thePage = theRange / 10.0;

  double theOffset = thePage * inDelta;

  mMin += theOffset;
  mMax += theOffset;

  CheckZoomOutLimit(theRange);
  return true;
}

bool iupPlotAxis::ScrollTo(double inMin)
{
  if (inMin < mNoZoomMin || inMin > mNoZoomMax)
    return false;

  if (!mHasZoom)
    return false;

  double theRange = mMax - mMin;

  mMin = inMin;
  mMax = mMin + theRange;

  CheckZoomOutLimit(theRange);
  return true;
}


/************************************************************************************************/


iupPlot::iupPlot(Ihandle* _ih)
  :ih(_ih), mCurrentDataSet(-1), mRedraw(true), mDataSetListCount(0), mCrossHair(false),
  mBackColor(CD_WHITE), mMarginAuto(1, 1, 1, 1), mDefaultFontSize(0), mDefaultFontStyle(-1)
{
}

iupPlot::~iupPlot()
{
  RemoveAllDataSets();
}

void iupPlot::SetViewport(int x, int y, int w, int h)
{
  mViewport.mX = x;
  mViewport.mY = y;
  mViewport.mWidth = w;
  mViewport.mHeight = h;
  mRedraw = true;
}

void iupPlot::SetFont(cdCanvas* canvas, int inFontStyle, int inFontSize) const
{
  if (inFontStyle == -1) inFontStyle = mDefaultFontStyle;
  if (inFontSize == 0) inFontSize = mDefaultFontSize;
  cdCanvasFont(canvas, NULL, inFontStyle, inFontSize);
}

static long iPlotGetDefaultColor(int index)
{
  switch (index)
  {
  case 0: return cdEncodeColor(255, 0, 0);
  case 1: return cdEncodeColor(0, 255, 0);
  case 2: return cdEncodeColor(0, 0, 255);
  case 3: return cdEncodeColor(0, 255, 255);
  case 4: return cdEncodeColor(255, 0, 255);
  case 5: return cdEncodeColor(255, 255, 0);
  case 6: return cdEncodeColor(128, 0, 0);
  case 7: return cdEncodeColor(0, 128, 0);
  case 8: return cdEncodeColor(0, 0, 128);
  case 9: return cdEncodeColor(0, 128, 128);
  case 10: return cdEncodeColor(128, 0, 128);
  case 11: return cdEncodeColor(128, 128, 0);
  default: return cdEncodeColor(0, 0, 0);  // the last must be always black
  }
}

long iupPlot::GetNextDataSetColor()
{
  int def_color = 0, i = 0;
  long theColor;

  do
  {
    theColor = iPlotGetDefaultColor(def_color);

    for (i = 0; i<mDataSetListCount; i++)
    {
      // already used, get another
      long theDataSetColor = cdEncodeAlpha(mDataSetList[i]->mColor, 255);
      if (theDataSetColor == theColor)
          break;
    }

    // not found, use it
    if (i == mDataSetListCount)
      break;

    def_color++;
  } while (def_color<12);

  return theColor;
}

void iupPlot::AddDataSet(iupPlotDataBase* inDataX, iupPlotDataBase* inDataY)
{
  if (mDataSetListCount < IUP_PLOT_MAX_DS)
  {
    long theColor = GetNextDataSetColor();

    mCurrentDataSet = mDataSetListCount;
    mDataSetListCount++;

    char theLegend[30];
    sprintf(theLegend, "plot %d", mCurrentDataSet);

    mDataSetList[mCurrentDataSet] = new iupPlotDataSet(inDataX, inDataY, theLegend, theColor);
  }
}

void iupPlot::RemoveDataSet(int inIndex)
{
  if (mCurrentDataSet == mDataSetListCount - 1)
    mCurrentDataSet--;

  delete mDataSetList[inIndex];

  for (int i = inIndex; i<mDataSetListCount; i++)
    mDataSetList[i] = mDataSetList[i + 1];

  mDataSetList[mDataSetListCount - 1] = NULL;

  mDataSetListCount--;
}

int iupPlot::FindDataSet(const char* inName)
{
  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    if (iupStrEqualNoCase(mDataSetList[ds]->GetName(), inName))
      return ds;
  }
  return -1;
}

void iupPlot::RemoveAllDataSets()
{
  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    delete mDataSetList[ds];
  }
}

bool iupPlot::FindSample(const iupPlotDataBase *inXData, const iupPlotDataBase *inYData, double inX, double inY, double tolX, double tolY, 
                         int &outSample, double &outX, double &outY) const
{
  int theCount = inXData->GetCount();
  for (int i = 0; i < theCount; i++)
  {
    outX = inXData->GetValue(i);
    outY = inYData->GetValue(i);

    if (fabs(outX - inX) < tolX && 
        fabs(outY - inY) < tolY)
    {
      outSample = i;
      return true;
    }
  }

  return false;
}

bool iupPlot::FindDataSetSample(int inX, int inY, int &outIndex, const char* &outName, int &outSample, double &outX, double &outY, const char* &outStrX) const
{
  double theX = mAxisX.mTrafo->TransformBack((double)inX);
  double theY = mAxisY.mTrafo->TransformBack((double)inY);
  double tolX = (fabs(mAxisX.mMax - mAxisX.mMin) / mViewport.mWidth) * 5.0;  // 5 pixels tolerance
  double tolY = (fabs(mAxisY.mMax - mAxisY.mMin) / mViewport.mHeight) * 5.0;

  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    iupPlotDataSet* dataset = mDataSetList[ds];

    iupPlotDataBase *theXData = dataset->mDataX;
    iupPlotDataBase *theYData = dataset->mDataY;

    if (FindSample(theXData, theYData, theX, theY, tolX, tolY, outSample, outX, outY))
    {
      if (theXData->IsString())
      {
        const iupPlotDataString *theStringXData = (const iupPlotDataString *)(theXData);
        outStrX = theStringXData->GetStrValue(outSample);
      }
      else
        outStrX = NULL;

      outIndex = ds;
      outName = dataset->GetName();
      return true;
    }
  }
  return false;
}

void iupPlot::Configure()
{
  mAxisX.Init();
  mAxisY.Init();

  if (mAxisX.mLogScale)
    mAxisY.mCrossOrigin = false;  // change at the other axis
  else
  {
    const iupPlotDataBase *theXData = mDataSetList[0]->mDataX;
    if (theXData->IsString())
    {
      const iupPlotDataString *theStringXData = (const iupPlotDataString *)(theXData);
      mAxisX.SetNamedTickIter(theStringXData);
    }
  }

  if (mAxisY.mLogScale)
    mAxisX.mCrossOrigin = false;   // change at the other axis
}

bool iupPlot::Render(cdCanvas* canvas)
{
  if (!mRedraw)
    return true;

  cdCanvasOrigin(canvas, mViewport.mX, mViewport.mY);

  // draw entire background, including the margins
  DrawPlotBackground(canvas);

  if (!mDataSetListCount)
    return true;

  cdCanvasNativeFont(canvas, IupGetAttribute(ih, "FONT"));

  Configure();

  if (!CalculateAxisRanges())
    return false;

  if (!CheckRange(mAxisX))
    return false;

  if (!CheckRange(mAxisY))
    return false;

  CalculateTickSize(canvas, mAxisX.mTick);
  CalculateTickSize(canvas, mAxisY.mTick);

  CalculateMargins(canvas);

  iupPlotRect theRect;
  theRect.mX = mMargin.mLeft;
  theRect.mY = mMargin.mBottom;
  theRect.mWidth = mViewport.mWidth - mMargin.mLeft - mMargin.mRight;
  theRect.mHeight = mViewport.mHeight - mMargin.mTop - mMargin.mBottom;

  if (!CalculateTickSpacing(theRect, canvas))
    return false;

  if (!CalculateXTransformation(theRect))
    return false;

  if (!CalculateYTransformation(theRect))
    return false;

  cdCanvasClip(canvas, CD_CLIPAREA);
  cdCanvasClipArea(canvas, mViewport.mX, mViewport.mX + mViewport.mWidth - 1,
                           mViewport.mY, mViewport.mY + mViewport.mHeight - 1);

  IFnC pre_cb = (IFnC)IupGetCallback(ih, "PREDRAW_CB");
  if (pre_cb)
    pre_cb(ih, canvas);

  if (!DrawXGrid(theRect, canvas))
    return false;

  if (!DrawYGrid(theRect, canvas))
    return false;

  if (!DrawXAxis(theRect, canvas))
    return false;

  if (!DrawYAxis(theRect, canvas))
    return false;

  if (mBox.mShow)
    DrawBox(theRect, canvas);

  // clip the plotregion while drawing plots
  cdCanvasClipArea(canvas, theRect.mX, theRect.mX + theRect.mWidth - 1, 
                           theRect.mY, theRect.mY + theRect.mHeight - 1);

  for (int ds = 0; ds < mDataSetListCount; ds++) 
  {
    if (!DrawPlot(ds, canvas))
      return false;
  }

  if (mCrossHair)
    DrawCrossHair(theRect, canvas);

  if (mShowSelection)
    DrawSelection(canvas);

  if (!DrawLegend(theRect, canvas))
    return false;

  cdCanvasClipArea(canvas, mViewport.mX, mViewport.mX + mViewport.mWidth - 1, 
                           mViewport.mY, mViewport.mY + mViewport.mHeight - 1);

  DrawPlotTitle(canvas);

  IFnC post_cb = (IFnC)IupGetCallback(ih, "POSTDRAW_CB");
  if (post_cb)
    post_cb(ih, canvas);

  mRedraw = false;
  return true;
}

