
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iupcbs.h"



const double kMaxMajorTickSizeInFontHeight = 0.5;  // not larger than half the font height
const double kMajorTickXInitialFac = 2.0;
const double kMajorTickYInitialFac = 3.0;
const double kRangeVerySmall = (double)1.0e-3;


bool iupPlot::CalculateXRange(double &outXMin, double &outXMax)  
{
  bool theFirst = true;
  outXMin = 0;
  outXMax = 0;

  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    iupPlotDataBase *theXData = mDataSetList[ds]->mDataX;
    if (theXData->GetCount() == 0)
      continue;
    
    double theXMin;
    double theXMax;
    if (!theXData->CalculateRange(theXMin, theXMax)) 
      return false;
    
    if (theFirst) 
    {
      outXMin = theXMin;
      outXMax = theXMax;
      theFirst = false;
    }
    if (theXMax>outXMax)
      outXMax = theXMax;
    if (theXMin<outXMin)
      outXMin = theXMin;
  }

  if (outXMin == 0 && outXMax == 0)
    return false;

  return true;
}

bool iupPlot::CalculateYRange(double &outYMin, double &outYMax)
{
  bool theFirst = true;
  outYMin = 0;
  outYMax = 0;

  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    iupPlotDataBase *theYData = mDataSetList[ds]->mDataY;

    double theYMin;
    double theYMax;
    if (!theYData->CalculateRange(theYMin, theYMax)) 
      return false;
    
    if (theFirst) 
    {
      outYMin = theYMin;
      outYMax = theYMax;
      theFirst = false;
    }
    if (theYMin<outYMin) 
      outYMin = theYMin;
    if (theYMax>outYMax)
      outYMax = theYMax;
  }

  //TODO YRange can be 0 why????
//  if (outYMin == 0 && outYMax == 0)
    //return false;

  return true;
}

bool iupPlot::CalculateAxisRanges() 
{
  if (mAxisX.mAutoScaleMin || mAxisX.mAutoScaleMax) 
  {
    double theXMin;
    double theXMax;

    CalculateXRange(theXMin, theXMax);

    if (mAxisX.mAutoScaleMin)
    {
      mAxisX.mMin = theXMin;
      if (mAxisX.mLogScale && (theXMin < kLogMinClipValue)) 
        mAxisX.mMin = kLogMinClipValue;
    }

    if (mAxisX.mAutoScaleMax) 
      mAxisX.mMax = theXMax;

    if (!mAxisX.mTickIter->AdjustRange(mAxisX.mMin, mAxisX.mMax)) 
      return false;
  }

  if (mAxisY.mAutoScaleMin || mAxisY.mAutoScaleMax) 
  {
    double theYMin;
    double theYMax;

    CalculateYRange(theYMin, theYMax);

    if (mAxisY.mAutoScaleMin) 
    {
      mAxisY.mMin = theYMin;
      if (mAxisY.mLogScale && (theYMin < kLogMinClipValue)) 
        mAxisY.mMin = kLogMinClipValue;
    }
    if (mAxisY.mAutoScaleMax)
      mAxisY.mMax = theYMax;

    if (!mAxisY.mTickIter->AdjustRange(mAxisY.mMin, mAxisY.mMax))
      return false;
  }

  return true;
}

bool iupPlot::CheckRange(const iupPlotAxis &inPlotAxis)
{
  if (inPlotAxis.mLogScale) 
  {
    if (inPlotAxis.mMin < kLogMinClipValue)
      return false;
  }
  return true;
}


bool iupPlot::CalculateXTransformation(const iupPlotRect &inRect)
{
  return mAxisX.mTrafo->Calculate(inRect.mX, inRect.mX + inRect.mWidth, mAxisX);
}

bool iupPlot::CalculateYTransformation(const iupPlotRect &inRect)
{
  return mAxisY.mTrafo->Calculate(inRect.mY, inRect.mY + inRect.mHeight, mAxisY);
}

bool iupPlot::CalculateTick(const iupPlotRect &inRect, cdCanvas* canvas)
{
  double theXRange = mAxisX.mMax - mAxisX.mMin;
  double theYRange = mAxisY.mMax - mAxisY.mMin;

  if (theXRange <= 0 || theYRange < 0)
    return false;

  if ((mAxisY.mMax != 0 && fabs(theYRange / mAxisY.mMax) < kRangeVerySmall) || theYRange == 0)
  {
    double delta = 0.1;
    if (mAxisY.mMax != 0)
      delta *= fabs(mAxisY.mMax);

    mAxisY.mMax += delta;
    mAxisY.mMin -= delta;
    theYRange = mAxisY.mMax - mAxisY.mMin;
  }

  int theXFontHeight;
  iupPlotSetFont(canvas, mAxisX.mTick.mFontStyle, mAxisX.mTick.mFontSize);
  cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);

  if (mAxisX.mTick.mAutoSpacing)
  {
    int theTextWidth;
    cdCanvasGetTextSize(canvas, "12345", &theTextWidth, NULL);

    double theDivGuess = inRect.mWidth / (kMajorTickXInitialFac*theTextWidth);
    if (!mAxisX.mTickIter->InitFromRanges(theXRange, inRect.mHeight, theDivGuess, mAxisX.mTick))
      return false;
  }

  int theYFontHeight;
  iupPlotSetFont(canvas, mAxisY.mTick.mFontStyle, mAxisY.mTick.mFontSize);
  cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);

  if (mAxisY.mTick.mAutoSpacing)
  {
    double theDivGuess = inRect.mHeight / (kMajorTickYInitialFac*theYFontHeight);
    if (!mAxisY.mTickIter->InitFromRanges(theYRange, inRect.mWidth, theDivGuess, mAxisY.mTick))
      return false;
  }

  SetTickSizes(theXFontHeight, mAxisX.mTick);
  SetTickSizes(theYFontHeight, mAxisY.mTick);

  return true;
}

void iupPlot::SetTickSizes(int inFontHeight, iupPlotTick &ioTick)
{
  if (ioTick.mAutoSize)
  {
    double theFac = kRelMinorTickSize / kRelMajorTickSize;
    int theMax = iupPlotRound(inFontHeight*kMaxMajorTickSizeInFontHeight);
    if (ioTick.mMajorSize > theMax)
      ioTick.mMajorSize = theMax;
    ioTick.mMinorSize = iupPlotRound(ioTick.mMajorSize*theFac);
  }
}
