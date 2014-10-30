
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iupcbs.h"



const double kMajorTickXInitialFac = 2.0;
const double kMajorTickYInitialFac = 3.0;
const double kRangeVerySmall = (double)1.0e-3;


void iupPlot::CalculateMargins(cdCanvas* canvas)
{
  if (mMarginAuto.mTop)
  {
    if (mTitle.GetText())
    {
      int theFontSize = mTitle.mFontSize;
      if (theFontSize == 0)
      {
        int size = IupGetInt(ih, "FONTSIZE");
        if (size > 0) size += 6;
        else size -= 8;
        theFontSize = size;
      }

      SetFont(canvas, mTitle.mFontStyle, theFontSize);

      int theTextHeight;
      cdCanvasGetTextSize(canvas, mTitle.GetText(), NULL, &theTextHeight);
      mMargin.mTop = theTextHeight + 5 + theTextHeight/2;
    }
    else
    {
      if (mAxisY.mShow && mAxisY.mShowArrow && !mAxisY.mReverse)
        mMargin.mTop = mAxisY.mTick.mMinorSize + 2 + 5;
      else
        mMargin.mTop = 0;
    }
  }

  if (mMarginAuto.mRight)
  {
    if (mAxisX.mShow && mAxisX.mShowArrow && !mAxisX.mReverse)
      mMargin.mRight = mAxisX.mTick.mMinorSize + 2 + 5;
    else
      mMargin.mRight = 0;
  }

  if (mMarginAuto.mBottom)
  {
    if (mAxisX.mShow && !mAxisX.mCrossOrigin)
    {
      mMargin.mBottom = mAxisX.mTick.mMajorSize;

      if (mAxisX.mTick.mShowNumber)
      {
        int theXTickFontWidth, theXTickFontHeight;
        cdCanvasGetTextSize(canvas, "1234567890.", &theXTickFontWidth, &theXTickFontHeight);
        theXTickFontWidth /= 11;
        if (mAxisX.mTick.mRotateNumber)
          mMargin.mBottom += theXTickFontWidth * iupPlotGetPrecisionNumChar(mAxisX.mTick.mFormatString, mAxisX.mMin, mAxisX.mMax);
        else
          mMargin.mBottom += theXTickFontHeight;  // skip number
      }

      if (mAxisX.GetLabel())
      {
        int theXFontHeight;
        SetFont(canvas, mAxisX.mFontStyle, mAxisX.mFontSize);
        cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);

        mMargin.mBottom += theXFontHeight + theXFontHeight / 10;
      }
    }
    else
      mMargin.mBottom = 0;
  }

  if (mMarginAuto.mLeft)
  {
    if (mAxisY.mShow && !mAxisY.mCrossOrigin)
    {
      mMargin.mLeft = mAxisY.mTick.mMajorSize;

      if (mAxisY.mTick.mShowNumber)
      {
        int theYTickFontWidth, theYTickFontHeight;
        cdCanvasGetTextSize(canvas, "1234567890.", &theYTickFontWidth, &theYTickFontHeight);
        theYTickFontWidth /= 11;
        if (mAxisY.mTick.mRotateNumber)
          mMargin.mLeft += theYTickFontWidth * iupPlotGetPrecisionNumChar(mAxisY.mTick.mFormatString, mAxisY.mMin, mAxisY.mMax);
        else
          mMargin.mLeft += theYTickFontHeight;  // skip number
      }

      if (mAxisY.GetLabel())
      {
        int theYFontHeight;
        SetFont(canvas, mAxisY.mFontStyle, mAxisY.mFontSize);
        cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);

        mMargin.mLeft += theYFontHeight + theYFontHeight / 10;
      }
    }
    else
      mMargin.mLeft = 0;
  }
}

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

void iupPlot::CalculateTickSize(cdCanvas* canvas, iupPlotTick &ioTick)
{
  if (ioTick.mAutoSize)
  {
    int theFontHeight;
    SetFont(canvas, ioTick.mFontStyle, ioTick.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theFontHeight, NULL, NULL);

    ioTick.mMajorSize = theFontHeight / 2;
    ioTick.mMinorSize = theFontHeight / 4;
  }
}

bool iupPlot::CalculateTickSpacing(const iupPlotRect &inRect, cdCanvas* canvas)
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

  if (mAxisX.mTick.mAutoSpacing)
  {
    int theXFontHeight;
    SetFont(canvas, mAxisX.mTick.mFontStyle, mAxisX.mTick.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);

    int theTextWidth;
    cdCanvasGetTextSize(canvas, "12345", &theTextWidth, NULL);

    double theDivGuess = inRect.mWidth / (kMajorTickXInitialFac*theTextWidth);
    if (!mAxisX.mTickIter->InitFromRanges(theXRange, theDivGuess, mAxisX.mTick))
      return false;
  }

  if (mAxisY.mTick.mAutoSpacing)
  {
    int theYFontHeight;
    SetFont(canvas, mAxisY.mTick.mFontStyle, mAxisY.mTick.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);

    double theDivGuess = inRect.mHeight / (kMajorTickYInitialFac*theYFontHeight);
    if (!mAxisY.mTickIter->InitFromRanges(theYRange, theDivGuess, mAxisY.mTick))
      return false;
  }

  return true;
}

