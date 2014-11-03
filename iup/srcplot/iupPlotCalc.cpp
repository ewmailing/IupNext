
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iupcbs.h"



const double kMajorTickXInitialFac = 2.0;
const double kMajorTickYInitialFac = 3.0;
const double kRangeVerySmall = (double)1.0e-3;


static int iPlotCountDigit(int inNum)
{
  int theCount = 0;
  while (inNum != 0)
  {
    inNum = inNum / 10;
    theCount++;
  }
  if (theCount == 0) theCount = 1;
  return theCount;
}

static int iPlotGetPrecisionNumChar(const char* inFormatString, double inMin, double inMax)
{
  int precision = 0;
  while (*inFormatString)
  {
    if (*inFormatString == '.')
      break;
    inFormatString++;
  }

  if (*inFormatString == '.')
  {
    inFormatString++;
    iupStrToInt(inFormatString, &precision);
  }

  int theMin = iupPlotRound(inMin);
  int theMax = iupPlotRound(inMax);
  int theNumDigitMin = iPlotCountDigit(theMin);
  int theNumDigitMax = iPlotCountDigit(theMax);
  if (theNumDigitMin > theNumDigitMax)
    precision += theNumDigitMin;
  else
    precision += theNumDigitMax;

  precision += 3;  // sign, decimal symbol, exp 

  return precision;
}

void iupPlot::GetTickNumberSize(const iupPlotAxis& inAxis, cdCanvas* canvas, int *outWitdh, int *outHeight) const
{
  int theTickFontWidth, theTickFontHeight;
  SetFont(canvas, inAxis.mTick.mFontStyle, inAxis.mTick.mFontSize);
  cdCanvasGetTextSize(canvas, "1234567890.", &theTickFontWidth, &theTickFontHeight);
  theTickFontWidth /= 11;
  if (outHeight) *outHeight = theTickFontHeight;
  if (outWitdh)  *outWitdh  = theTickFontWidth * iPlotGetPrecisionNumChar(inAxis.mTick.mFormatString, inAxis.mMin, inAxis.mMax);
}

void iupPlot::CalculateMargins(cdCanvas* canvas)
{
  mMargin.mLeft = 0;
  mMargin.mRight = 0;
  mMargin.mTop = 0;
  mMargin.mBottom = 0;

  if (mMarginAuto.mTop)
  {
    if (mTitle.GetText())
    {
      SetTitleFont(canvas);

      int theTextHeight;
      cdCanvasGetTextSize(canvas, mTitle.GetText(), NULL, &theTextHeight);
      mMargin.mTop += theTextHeight + 5 + theTextHeight/2;
    }
    else
    {
      if (mAxisY.mShow)
      {
        if (mAxisY.mShowArrow && !mAxisY.mReverse)
          mMargin.mTop += mAxisY.mTick.mMinorSize + 2 + 5;  // size of the arrow

        if (mAxisY.mTick.mShowNumber)
        {
          if (mAxisY.mTick.mRotateNumber)
          {
            int theYTickNumberHeight;
            GetTickNumberSize(mAxisY, canvas, NULL, &theYTickNumberHeight);
            mMargin.mBottom += theYTickNumberHeight / 2;
          }
          else
          {
            int theYTickNumberWidth;
            GetTickNumberSize(mAxisY, canvas, &theYTickNumberWidth, NULL);
            mMargin.mBottom += theYTickNumberWidth / 2;
          }
        }
      }
    }
  }

  if (mMarginAuto.mBottom)
  {
    if (mAxisX.mShow && !mAxisX.mCrossOrigin)
    {
      mMargin.mBottom += mAxisX.mTick.mMajorSize;

      if (mAxisX.mTick.mShowNumber)
      {
        if (mAxisX.mTick.mRotateNumber)
        {
          int theXTickNumberWidth;
          GetTickNumberSize(mAxisX, canvas, &theXTickNumberWidth, NULL);
          mMargin.mBottom += theXTickNumberWidth;
        }
        else
        {
          int theXTickNumberHeight;
          GetTickNumberSize(mAxisX, canvas, NULL, &theXTickNumberHeight);
          mMargin.mBottom += theXTickNumberHeight;
        }
      }

      if (mAxisX.GetLabel())
      {
        int theXFontHeight;
        SetFont(canvas, mAxisX.mFontStyle, mAxisX.mFontSize);
        cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);

        mMargin.mBottom += theXFontHeight + theXFontHeight / 10;
      }
    }

    if (mAxisY.mShow)
    {
      if (mAxisY.mTick.mShowNumber)
      {
        if (mAxisY.mTick.mRotateNumber)
        {
          int theYTickNumberHeight;
          GetTickNumberSize(mAxisY, canvas, NULL, &theYTickNumberHeight);
          mMargin.mBottom += theYTickNumberHeight/2;
        }
        else
        {
          int theYTickNumberWidth;
          GetTickNumberSize(mAxisY, canvas, &theYTickNumberWidth, NULL);
          mMargin.mBottom += theYTickNumberWidth/2;
        }
      }
    }
  }

  if (mMarginAuto.mRight)
  {
    if (mAxisX.mShow)
    {
      if (mAxisX.mShowArrow && !mAxisX.mReverse)
        mMargin.mRight += mAxisX.mTick.mMinorSize + 2 + 5;  // size of the arrow

      if (mAxisX.mTick.mShowNumber)
      {
        if (mAxisX.mTick.mRotateNumber)
        {
          int theXTickNumberHeight;
          GetTickNumberSize(mAxisX, canvas, NULL, &theXTickNumberHeight);
          mMargin.mRight += theXTickNumberHeight/2;
        }
        else
        {
          int theXTickNumberWidth;
          GetTickNumberSize(mAxisX, canvas, &theXTickNumberWidth, NULL);
          mMargin.mRight += theXTickNumberWidth/2;
        }
      }
    }
  }

  if (mMarginAuto.mLeft)
  {
    if (mAxisY.mShow && !mAxisY.mCrossOrigin)
    {
      mMargin.mLeft += mAxisY.mTick.mMajorSize;

      if (mAxisY.mTick.mShowNumber)
      {
        if (mAxisY.mTick.mRotateNumber)
        {
          int theYTickNumberHeight;
          GetTickNumberSize(mAxisY, canvas, NULL, &theYTickNumberHeight);
          mMargin.mLeft += theYTickNumberHeight;
        }
        else
        {
          int theYTickNumberWidth;
          GetTickNumberSize(mAxisY, canvas, &theYTickNumberWidth, NULL);
          mMargin.mLeft += theYTickNumberWidth;
        }
      }

      if (mAxisY.GetLabel())
      {
        int theYFontHeight;
        SetFont(canvas, mAxisY.mFontStyle, mAxisY.mFontSize);
        cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);

        mMargin.mLeft += theYFontHeight + theYFontHeight / 10;
      }
    }

    if (mAxisX.mShow)
    {
      if (mAxisX.mShowArrow && mAxisX.mReverse)
        mMargin.mLeft += mAxisX.mTick.mMinorSize + 2 + 5;  // size of the arrow

      if (mAxisX.mTick.mShowNumber)
      {
        if (mAxisX.mTick.mRotateNumber)
        {
          int theXTickNumberHeight;
          GetTickNumberSize(mAxisX, canvas, NULL, &theXTickNumberHeight);
          mMargin.mLeft += theXTickNumberHeight / 2;
        }
        else
        {
          int theXTickNumberWidth;
          GetTickNumberSize(mAxisX, canvas, &theXTickNumberWidth, NULL);
          mMargin.mLeft += theXTickNumberWidth / 2;
        }
      }
    }
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

bool iupPlot::CheckRange(const iupPlotAxis &inAxis)
{
  if (inAxis.mLogScale) 
  {
    if (inAxis.mMin < kLogMinClipValue)
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

