
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

bool iupPlotTrafoLinear::Calculate(int inBegin, int inEnd, const iupPlotAxis& inPlotAxis)
{
  double theDataRange = inPlotAxis.mMax - inPlotAxis.mMin;
  if (theDataRange < kFloatSmall)
    return false;

  double theMin = inPlotAxis.mMin;
  if (inPlotAxis.mDiscrete)
  {
    theDataRange++;
    theMin -= 0.5;
  }

  double theTargetRange = inEnd - inBegin;
  double theScale = theTargetRange / theDataRange;

  if (!inPlotAxis.mReverse)
    mOffset = inBegin - theMin * theScale;
  else
    mOffset = inEnd + theMin * theScale;

  mSlope = theScale;
  if (inPlotAxis.mReverse)
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

bool iupPlotTrafoLog::Calculate(int inBegin, int inEnd, const iupPlotAxis& inPlotAxis)
{
  double theBase = inPlotAxis.mLogBase;
  double theDataRange = iupPlotLog(inPlotAxis.mMax, theBase) - iupPlotLog(inPlotAxis.mMin, theBase);
  if (theDataRange < kFloatSmall)
    return false;

  double theTargetRange = inEnd - inBegin;
  double theScale = theTargetRange / theDataRange;

  if (!inPlotAxis.mReverse)
    mOffset = inBegin - iupPlotLog(inPlotAxis.mMin, theBase) * theScale;
  else
    mOffset = inEnd + iupPlotLog(inPlotAxis.mMin, theBase) * theScale;

  mBase = theBase;

  mSlope = theScale;
  if (inPlotAxis.mReverse)
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
  mNamedTickIter.SetStringList(inStringData);
}


/************************************************************************************************/


iupPlot::iupPlot(Ihandle* _ih)
  :ih(_ih), mCurrentDataSet(-1), mRedraw(true), mDataSetListCount(0), mBackColor(CD_WHITE)
{
}

iupPlot::~iupPlot()
{
  RemoveAllDatasets();
}

void iupPlot::SetViewport(int x, int y, int w, int h)
{
  mViewport.mX = x;
  mViewport.mY = y;
  mViewport.mWidth = w;
  mViewport.mHeight = h;
  mRedraw = true;
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

long iupPlot::GetNextDatasetColor()
{
  int def_color = 0, i = 0;
  long theColor;

  do
  {
    theColor = iPlotGetDefaultColor(def_color);

    for (i = 0; i<mDataSetListCount; i++)
    {
      // already used, get another
      if (mDataSetList[i]->mColor == theColor)
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
    long theColor = GetNextDatasetColor();

    mCurrentDataSet = mDataSetListCount;
    mDataSetListCount++;

    char theLegend[30];
    sprintf(theLegend, "plot %d", mCurrentDataSet);

    mDataSetList[mCurrentDataSet] = new iupPlotDataSet(inDataX, inDataY, theLegend, theColor);
  }
}

void iupPlot::RemoveDataset(int inIndex)
{
  if (mCurrentDataSet == mDataSetListCount - 1)
    mCurrentDataSet--;

  delete mDataSetList[inIndex];

  for (int i = inIndex; i<mDataSetListCount; i++)
    mDataSetList[i] = mDataSetList[i + 1];

  mDataSetList[mDataSetListCount - 1] = NULL;

  mDataSetListCount--;
}

int iupPlot::FindDataset(const char* inName)
{
  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    if (iupStrEqualNoCase(mDataSetList[ds]->GetName(), inName))
      return ds;
  }
  return -1;
}

void iupPlot::RemoveAllDatasets()
{
  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    delete mDataSetList[ds];
  }
}

bool iupPlot::Render(cdCanvas* canvas)
{
  iupPlotRect theRect;
  theRect.mX = mMargin.mLeft;
  theRect.mY = mMargin.mTop;
  theRect.mWidth = mViewport.mWidth - mMargin.mLeft - mMargin.mRight;
  theRect.mHeight = mViewport.mHeight - mMargin.mTop - mMargin.mBottom;

  if (!mDataSetListCount)
    return true;

  cdCanvasNativeFont(canvas, IupGetAttribute(ih, "FONT"));

  Configure();

  if (!CalculateAxisRanges())
    return false;

  if (!this->CheckRange(mAxisX))
    return false;

  if (!this->CheckRange(mAxisY))
    return false;

  if (!CalculateTick(theRect, canvas))
    return false;

  if (!CalculateXTransformation(theRect))
    return false;

  if (!CalculateYTransformation(theRect))
    return false;

  cdCanvasClip(canvas, CD_CLIPAREA);
  cdCanvasClipArea(canvas, mViewport.mX, mViewport.mX + mViewport.mWidth - 1,
                           mViewport.mY, mViewport.mY + mViewport.mHeight - 1);

  // draw entire background, including the margins
  DrawPlotBackground(canvas);

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

  DrawBox(theRect, canvas);

  // clip the plotregion while drawing plots
  cdCanvasClipArea(canvas, theRect.mX, theRect.mX + theRect.mWidth - 1, 
                           theRect.mY, theRect.mY + theRect.mHeight - 1);

  for (int ds = 0; ds < mDataSetListCount; ds++) 
  {
    if (!DrawPlot(ds, canvas))
      return false;
  }

  if (!DrawLegend(theRect, canvas))
    return false;

  cdCanvasClipArea(canvas, mViewport.mX, mViewport.mX + mViewport.mWidth - 1, 
                           mViewport.mY, mViewport.mY + mViewport.mHeight - 1);

  DrawPlotTitle(canvas);

  IFnC post_cb = (IFnC)IupGetCallback(ih, "POSTDRAW_CB");
  if (post_cb)
    post_cb(ih, canvas);

  return true;
}

void iupPlot::Configure()
{
  mAxisX.Init();
  mAxisY.Init();

  if (mAxisX.mLogScale)
    mAxisY.mCrossOrigin = false;  // change at the other axis
  else
  {
    const iupPlotDataBase *theGlue = mDataSetList[0]->mDataX;
    if (theGlue->IsString())
    {
      const iupPlotDataString *theStringXData = (const iupPlotDataString *)(theGlue);
      mAxisX.SetNamedTickIter(theStringXData);
    }
  }

  if (mAxisY.mLogScale)
    mAxisX.mCrossOrigin = false;   // change at the other axis
}

