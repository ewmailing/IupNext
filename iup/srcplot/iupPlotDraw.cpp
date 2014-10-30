
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iupcbs.h"


static inline void iPlotSetLine(cdCanvas* canvas, int inLineStyle, int inLineWidth)
{
  cdCanvasLineStyle(canvas, inLineStyle);
  cdCanvasLineWidth(canvas, inLineWidth);
}

static inline void iPlotSetMark(cdCanvas* canvas, int inMarkStyle, int inMarkSize)
{
  cdCanvasMarkType(canvas, inMarkStyle);
  cdCanvasMarkSize(canvas, inMarkSize);
}

static inline void iPlotDrawText(cdCanvas* canvas, double inX, double inY, int inAlignment, const char* inString)
{
  cdCanvasTextAlignment(canvas, inAlignment);
  cdfCanvasText(canvas, inX, inY, inString);
}

static inline void iPlotDrawRotatedText(cdCanvas* canvas, double inX, double inY, double inDegrees, int inAlignment, const char *inString)
{
  cdCanvasTextAlignment(canvas, inAlignment);
  double theOldOrientation = cdCanvasTextOrientation(canvas, inDegrees);
  cdfCanvasText(canvas, inX, inY, inString);
  cdCanvasTextOrientation(canvas, theOldOrientation);
}

static inline void iPlotDrawRect(cdCanvas* canvas, double inX, double inY, double inW, double inH)
{
  cdfCanvasRect(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

static inline void iPlotDrawBox(cdCanvas* canvas, double inX, double inY, double inW, double inH)
{
  cdfCanvasBox(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

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

int iupPlotGetPrecisionNumChar(const char* inFormatString, double inMin, double inMax)
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

void iupPlot::DrawBox(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mBox.mShow)
  {
    cdCanvasSetForeground(canvas, mBox.mColor);
    iPlotSetLine(canvas, mBox.mLineStyle, mBox.mLineWidth);

    iPlotDrawRect(canvas, inRect.mX, inRect.mY, inRect.mWidth, inRect.mHeight);
  }
}

bool iupPlot::DrawXGrid(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mGrid.mXGridOn) 
  {
    if (!mAxisX.mTickIter->Init())
      return false;

    double theX;
    bool theIsMajorTick;

    cdCanvasSetForeground(canvas, mGrid.mColor);
    iPlotSetLine(canvas, mGrid.mLineStyle, mGrid.mLineWidth);

    while (mAxisX.mTickIter->GetNextTick(theX, theIsMajorTick, NULL))
    {          
      if (theIsMajorTick) 
      {
        double theScreenX = mAxisX.mTrafo->Transform(theX);
        cdfCanvasLine(canvas, theScreenX, inRect.mY, theScreenX, inRect.mY + inRect.mHeight-1);
      }
    }
  }

  return true;
}

bool iupPlot::DrawYGrid(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mGrid.mYGridOn)
  {
    if (!mAxisY.mTickIter->Init())
      return false;

    double theY;
    bool theIsMajorTick;
    iupPlotRect theTickRect;

    cdCanvasSetForeground(canvas, mGrid.mColor);
    iPlotSetLine(canvas, mGrid.mLineStyle, mGrid.mLineWidth);

    while (mAxisY.mTickIter->GetNextTick(theY, theIsMajorTick, NULL))
    {            
      if (theIsMajorTick) 
      {
        double theScreenY = mAxisY.mTrafo->Transform(theY);
        cdfCanvasLine(canvas, inRect.mX, theScreenY, inRect.mX + inRect.mWidth - 1, theScreenY);
      }
    }
  }

  return true;
}

static void iPlotFillArrow(cdCanvas* canvas, int inX1, int inY1, int inX2, int inY2, int inX3, int inY3)
{
  cdCanvasBegin(canvas, CD_FILL);
  cdCanvasVertex(canvas, inX1, inY1);
  cdCanvasVertex(canvas, inX2, inY2);
  cdCanvasVertex(canvas, inX3, inY3);
  cdCanvasEnd(canvas);
}

static void iPlotDrawArrow(cdCanvas* canvas, double inX, double inY, int inVertical, int inDirection, int inSize)
{
  int x = iupPlotRound(inX);
  int y = iupPlotRound(inY);
  inSize += 2; // to avoid too small sizes
  int size2 = iupPlotRound(inSize*0.7);
  if (inVertical)
  {
    int y1 = y;
    y += inDirection*inSize;
    cdCanvasLine(canvas, x, y, x, y1);

    int y2 = y - inDirection*size2;
    iPlotFillArrow(canvas, x, y, x - size2, y2, x + size2, y2);
  }
  else
  {
    int x1 = x;
    x += inDirection*inSize;
    cdCanvasLine(canvas, x, y, x1, y);

    int x2 = x - inDirection*size2;
    iPlotFillArrow(canvas, x, y, x2, y - size2, x2, y + size2);
  }
}

bool iupPlot::DrawXAxis(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (!mAxisX.mShow)
    return true;

  cdCanvasSetForeground(canvas, mAxisX.mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, mAxisX.mLineWidth);

  double theTargetY = 0;
  if (!mAxisX.mCrossOrigin) 
  {
    if (!mAxisY.mReverse) 
      theTargetY = mAxisY.mMin;
    else 
      theTargetY = mAxisY.mMax;
  }

  double theScreenY  = mAxisY.mTrafo->Transform(theTargetY);
  double theScreenX1 = inRect.mX;
  double theScreenX2 = theScreenX1 + inRect.mWidth;

  cdfCanvasLine(canvas, theScreenX1, theScreenY, theScreenX2, theScreenY);

  if (mAxisX.mShowArrow)
  {
    if (!mAxisX.mReverse)
      iPlotDrawArrow(canvas, theScreenX2, theScreenY, 0, 1, mAxisX.mTick.mMinorSize);
    else
      iPlotDrawArrow(canvas, theScreenX1, theScreenY, 0, -1, mAxisX.mTick.mMinorSize);
  }

  if (mAxisX.mTick.mShow) 
  {
    if (!mAxisX.mTickIter->Init())
      return false;

    double theX;
    bool theIsMajorTick;
    char theFormatString[30];

    if (mAxisX.mTick.mShowNumber)
      SetFont(canvas, mAxisX.mTick.mFontStyle, mAxisX.mTick.mFontSize);

    while (mAxisX.mTickIter->GetNextTick(theX, theIsMajorTick, theFormatString))
    {
      if (!DrawXTick(theX, theScreenY, theIsMajorTick, theFormatString, canvas))
        return false;
    }

    theScreenY -= mAxisX.mTick.mMajorSize;  // skip major tick
    if (mAxisX.mTick.mShowNumber)
    {
      int theXTickFontWidth, theXTickFontHeight;
      cdCanvasGetTextSize(canvas, "1234567890.", &theXTickFontWidth, &theXTickFontHeight);
      theXTickFontWidth /= 11;
      if (mAxisX.mTick.mRotateNumber)
        theScreenY -= theXTickFontWidth * iupPlotGetPrecisionNumChar(theFormatString, mAxisX.mMin, mAxisX.mMax);  // skip number
      else
        theScreenY -= theXTickFontHeight;  // skip number
    }
  }

  if (mAxisX.GetLabel()) 
  {
    int theXFontHeight;
    SetFont(canvas, mAxisX.mFontStyle, mAxisX.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);

    theScreenY -= theXFontHeight / 10;  // spacing

    if (mAxisX.mLabelCentered)
    {
      double theScreenX = theScreenX1 + inRect.mWidth / 2;
      iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH, mAxisX.GetLabel());
    }
    else
    {
      double theScreenX = theScreenX2;
      iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH_EAST, mAxisX.GetLabel());
    }
  }

  return true;
}

bool iupPlot::DrawXTick(double inX, double inScreenY, bool inMajor, const char*inFormatString, cdCanvas* canvas) const
{
  int theTickSize;
  double theScreenX = mAxisX.mTrafo->Transform(inX);
  if (inMajor) 
  {
    theTickSize = mAxisX.mTick.mMajorSize;

    if (mAxisX.mTick.mShowNumber)
    {
      char theBuf[128];
      sprintf(theBuf, inFormatString, inX);
      double theScreenY = inScreenY - theTickSize - mAxisX.mTick.mMinorSize;  // Use minor size as spacing
      if (mAxisY.mTick.mRotateNumber)
        iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_EAST, theBuf);
      else
        iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH, theBuf);
    }
  }
  else 
    theTickSize = mAxisX.mTick.mMinorSize;

  cdfCanvasLine(canvas, theScreenX, inScreenY, theScreenX, inScreenY - theTickSize);
  return true;
}

bool iupPlot::DrawYAxis(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (!mAxisY.mShow)
    return true;

  cdCanvasSetForeground(canvas, mAxisY.mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, mAxisY.mLineWidth);

  double theTargetX = 0;
  if (!mAxisY.mCrossOrigin) 
  {
    if (!mAxisX.mReverse) 
      theTargetX = mAxisX.mMin;
    else
      theTargetX = mAxisX.mMax;
  }
  if (mAxisX.mDiscrete)
    theTargetX -= 0.5;

  double theScreenX  = mAxisX.mTrafo->Transform(theTargetX);
  double theScreenY1 = inRect.mY;
  double theScreenY2 = theScreenY1 + inRect.mHeight;

  cdfCanvasLine(canvas, theScreenX, theScreenY1, theScreenX, theScreenY2);

  if (mAxisX.mShowArrow)
  {
    if (!mAxisY.mReverse)
      iPlotDrawArrow(canvas, theScreenX, theScreenY2, 1, 1, mAxisY.mTick.mMinorSize);
    else
      iPlotDrawArrow(canvas, theScreenX, theScreenY1, 1, -1, mAxisY.mTick.mMinorSize);
  }

  if (mAxisY.mTick.mShow) 
  {
    if (!mAxisY.mTickIter->Init())
      return false;

    double theY;
    bool theIsMajorTick;
    char theFormatString[30];

    if (mAxisY.mTick.mShowNumber)
      SetFont(canvas, mAxisY.mTick.mFontStyle, mAxisY.mTick.mFontSize);

    while (mAxisY.mTickIter->GetNextTick(theY, theIsMajorTick, theFormatString))
    {
      if (!DrawYTick(theY, theScreenX, theIsMajorTick, theFormatString, canvas))
        return false;
    }

    theScreenX -= mAxisY.mTick.mMajorSize;  // skip major tick
    if (mAxisY.mTick.mShowNumber)
    {
      int theYTickFontWidth, theYTickFontHeight;
      cdCanvasGetTextSize(canvas, "1234567890.", &theYTickFontWidth, &theYTickFontHeight);
      theYTickFontWidth /= 11;
      if (mAxisY.mTick.mRotateNumber)
        theScreenX -= theYTickFontHeight;  // skip number
      else
        theScreenX -= theYTickFontWidth * iupPlotGetPrecisionNumChar(theFormatString, mAxisY.mMin, mAxisY.mMax);  // skip number
    }
  }

  if (mAxisY.GetLabel()) 
  {
    int theYFontHeight;
    SetFont(canvas, mAxisY.mFontStyle, mAxisY.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);

    theScreenX -= theYFontHeight / 10;  // spacing

    if (mAxisY.mLabelCentered)
    {
      double theScreenY = theScreenY1 + inRect.mHeight / 2;
      iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH, mAxisY.GetLabel());
    }
    else
    {
      double theScreenY = theScreenY2;
      iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH_EAST, mAxisY.GetLabel());
    }
  }

  return true;
}

bool iupPlot::DrawYTick(double inY, double inScreenX, bool inMajor, const char*inFormatString, cdCanvas* canvas) const
{
  int theTickSize;
  double theScreenY = mAxisY.mTrafo->Transform(inY);
  if (inMajor) 
  {
    theTickSize = mAxisY.mTick.mMajorSize;

    if (mAxisY.mTick.mShowNumber)
    {
      char theBuf[128];
      sprintf(theBuf, inFormatString, inY);
      double theScreenX = inScreenX - theTickSize - mAxisY.mTick.mMinorSize;  // Use minor size as spacing
      if (mAxisY.mTick.mRotateNumber)
        iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH, theBuf);
      else
        iPlotDrawText(canvas, theScreenX, theScreenY, CD_EAST, theBuf);
    }
  }
  else 
    theTickSize = mAxisY.mTick.mMinorSize;

  cdfCanvasLine(canvas, inScreenX, theScreenY, inScreenX - theTickSize, theScreenY);  
  return true;
}

void iupPlot::DrawPlotTitle(cdCanvas* canvas) const
{
  if (mTitle.GetText()) 
  {
    cdCanvasSetForeground(canvas, mTitle.mColor);

    int theFontSize = mTitle.mFontSize;
    if (theFontSize == 0)
    {
      int size = IupGetInt(ih, "FONTSIZE");
      if (size > 0) size += 6;
      else size -= 8;
      theFontSize = size;
    }

    SetFont(canvas, mTitle.mFontStyle, theFontSize);

    // do not depend on theMargin
    int theX = mViewport.mX + mViewport.mWidth / 2;
    int theY = mViewport.mY + mViewport.mHeight - 5;  // add small spacing

    cdCanvasTextAlignment(canvas, CD_NORTH);
    cdCanvasText(canvas, theX, theY, mTitle.GetText());
  }
}

void iupPlot::DrawPlotBackground(cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mBackColor);
  cdCanvasBox(canvas, mViewport.mX, mViewport.mX + mViewport.mWidth - 1,
                      mViewport.mY, mViewport.mY + mViewport.mHeight - 1);
}

bool iupPlot::DrawLegend (const iupPlotRect &inRect, cdCanvas* canvas) const 
{
  if (mLegend.mShow)
  {
    int ds;
    int theFontHeight;

    SetFont(canvas, mLegend.mFontStyle, mLegend.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theFontHeight, NULL, NULL);

    int theMargin = theFontHeight / 2;
    if (mLegend.mPosition == IUP_PLOT_BOTTOMCENTER)
      theMargin = 0;
    int theTotalHeight = mDataSetListCount*theFontHeight + 2 * theMargin;
    int theMarkSpace = 0, theLineSpace = 0;

    int theWidth, theMaxWidth = 0;
    for (ds = 0; ds < mDataSetListCount; ds++)
    {
      iupPlotDataSet* dataset = mDataSetList[ds];

      cdCanvasGetTextSize(canvas, dataset->GetName(), &theWidth, NULL);

      if (dataset->mMode == IUP_PLOT_MARK || dataset->mMode == IUP_PLOT_MARKLINE)
      {
        if (dataset->mMarkSize + 6 > theMarkSpace)
          theMarkSpace = dataset->mMarkSize + 6;
        theWidth += dataset->mMarkSize + 6;
      }
      if (dataset->mMode != IUP_PLOT_MARK)
      {
        theWidth += 10;
        theLineSpace = 10;
      }

      if (theWidth > theMaxWidth)
        theMaxWidth = theWidth;
    }

    if (theMaxWidth == 0)
      return false;

    theMaxWidth += 2 * theMargin;

    int theScreenX = inRect.mX;
    int theScreenY = inRect.mY;

    switch (mLegend.mPosition)
    {
    case IUP_PLOT_TOPLEFT:
      theScreenX += 2;
      theScreenY += inRect.mHeight - theTotalHeight - 2;
      break;
    case IUP_PLOT_BOTTOMLEFT:
      theScreenX += 2;
      theScreenY += 2;
      break;
    case IUP_PLOT_BOTTOMRIGHT:
      theScreenX += inRect.mWidth - theMaxWidth - 2;
      theScreenY += 2;
      break;
    case IUP_PLOT_BOTTOMCENTER:
      theScreenX += (inRect.mWidth - theMaxWidth) / 2;
      theScreenY = theFontHeight / 4;
      break;
    default: // IUP_PLOT_TOPRIGHT
      theScreenX += inRect.mWidth - theMaxWidth - 2;
      theScreenY += inRect.mHeight - theTotalHeight - 2;
      break;
    }

    cdCanvasClipArea(canvas, theScreenX, theScreenX + theMaxWidth - 1, 
                             theScreenY, theScreenY + theTotalHeight - 1);

    cdCanvasSetForeground(canvas, mLegend.mBoxColor);
    iPlotSetLine(canvas, mLegend.mBoxLineStyle, mLegend.mBoxLineWidth);
    iPlotDrawRect(canvas, theScreenX, theScreenY, theMaxWidth, theTotalHeight);
    cdCanvasSetForeground(canvas, mLegend.mBoxBackColor);
    iPlotDrawBox(canvas, theScreenX+1, theScreenY+1, theMaxWidth-2, theTotalHeight-2);

    for (ds = 0; ds < mDataSetListCount; ds++)
    {
      iupPlotDataSet* dataset = mDataSetList[ds];

      cdCanvasSetForeground(canvas, dataset->mColor);

      int theLegendX = theScreenX + theMargin;
      int theLegendY = theScreenY + (mDataSetListCount-1 - ds)*theFontHeight + theMargin;

      theLegendY += theFontHeight / 2;

      if (dataset->mMode == IUP_PLOT_MARK || dataset->mMode == IUP_PLOT_MARKLINE)
      {
        iPlotSetMark(canvas, dataset->mMarkStyle, dataset->mMarkSize);
        cdCanvasMark(canvas, theLegendX + theMarkSpace / 2, theLegendY - theFontHeight/8);
      }
      if (dataset->mMode != IUP_PLOT_MARK)
      {
        iPlotSetLine(canvas, dataset->mLineStyle, dataset->mLineWidth);
        cdCanvasLine(canvas, theLegendX + theMarkSpace, theLegendY - theFontHeight / 8,
                             theLegendX + theMarkSpace + theLineSpace - 3, theLegendY - theFontHeight / 8);
      }

      iPlotDrawText(canvas, theLegendX + theMarkSpace + theLineSpace, theLegendY, CD_WEST, dataset->GetName());
    }
  }

  return true;
}

void iupPlot::DrawDataLine(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const 
{
  cdCanvasBegin(canvas, CD_OPEN_LINES);

  for (int i = 0; i < inCount; i++) 
  {
    double theX = inXData.GetValue(i);
    double theY = inYData.GetValue(i);
    double theScreenX = mAxisX.mTrafo->Transform(theX);
    double theScreenY = mAxisY.mTrafo->Transform(theY);

    cdfCanvasVertex(canvas, theScreenX, theScreenY);
  }

  cdCanvasEnd(canvas);
}

void iupPlot::DrawDataMark(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const 
{
  for (int i = 0; i < inCount; i++) 
  {
    double theX = inXData.GetValue(i);
    double theY = inYData.GetValue(i);
    double theScreenX = mAxisX.mTrafo->Transform(theX);
    double theScreenY = mAxisY.mTrafo->Transform(theY);

    cdCanvasMark(canvas, iupPlotRound(theScreenX), iupPlotRound(theScreenY));
  }
}

void iupPlot::DrawDataMarkLine(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const 
{
  cdCanvasBegin(canvas, CD_OPEN_LINES);

  for (int i = 0; i < inCount; i++) 
  {
    double theX = inXData.GetValue(i);
    double theY = inYData.GetValue(i);
    double theScreenX = mAxisX.mTrafo->Transform(theX);
    double theScreenY = mAxisY.mTrafo->Transform(theY);

    cdCanvasMark(canvas, iupPlotRound(theScreenX), iupPlotRound(theScreenY));
    cdfCanvasVertex(canvas, theScreenX, theScreenY);
  }

  cdCanvasEnd(canvas);
}

void iupPlot::DrawDataArea(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const 
{
  cdCanvasBegin(canvas, CD_FILL);

  for (int i = 0; i < inCount; i++) 
  {
    double theX = inXData.GetValue(i);
    double theY = inYData.GetValue(i);
    double theScreenX = mAxisX.mTrafo->Transform(theX);
    double theScreenY = mAxisY.mTrafo->Transform(theY);

    if (i == 0) 
    {
      double theScreenY0 = mAxisY.mTrafo->Transform(0);
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);
    }

    cdfCanvasVertex(canvas, theScreenX, theScreenY);

    if (i == inCount-1)
    {
      double theScreenY0 = mAxisY.mTrafo->Transform(0);
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);
    }
  }

  cdCanvasEnd(canvas);
}

void iupPlot::DrawDataBar(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const 
{
  double theScreenY0 = mAxisY.mTrafo->Transform (0);

  double theMinX = inXData.GetValue(0);
  double theScreenMinX = mAxisX.mTrafo->Transform(theMinX);
  double theMaxX = inXData.GetValue(inCount-1);
  double theScreenMaxX = mAxisX.mTrafo->Transform(theMaxX);

  double theBarWidth = (theScreenMaxX - theScreenMinX + 1) / inCount;
  theBarWidth *= 0.9;

  for (int i = 0; i < inCount; i++) 
  {
    double theX = inXData.GetValue(i);
    double theY = inYData.GetValue(i);
    double theScreenX = mAxisX.mTrafo->Transform(theX);
    double theScreenY = mAxisY.mTrafo->Transform(theY);

    double theBarX = theScreenX - theBarWidth / 2;
    double theBarHeight = theScreenY - theScreenY0;

    iPlotDrawBox(canvas, theBarX, theScreenY0, theBarWidth, theBarHeight);
  }
}

bool iupPlot::DrawPlot(int inIndex, cdCanvas* canvas) const 
{  
  iupPlotDataSet* dataset = mDataSetList[inIndex];

  iupPlotDataBase *theXData = dataset->mDataX;
  iupPlotDataBase *theYData = dataset->mDataY;

  int theXCount = theXData->GetCount();
  int theYCount = theXData->GetCount();

  if ((theXCount == 0) || (theYCount == 0))
    return false;

  if (theXCount != theYCount)
    return false;

  cdCanvasSetForeground(canvas, dataset->mColor);
  iPlotSetLine(canvas, dataset->mLineStyle, dataset->mLineWidth);
  iPlotSetMark(canvas, dataset->mMarkStyle, dataset->mMarkSize);

  switch (dataset->mMode)
  {
  case IUP_PLOT_LINE:
    DrawDataLine(*theXData, *theYData, theXCount, canvas);
    break;
  case IUP_PLOT_MARK:
    DrawDataMark(*theXData, *theYData, theXCount, canvas);
    break;
  case IUP_PLOT_MARKLINE:
    DrawDataMarkLine(*theXData, *theYData, theXCount, canvas);
    break;
  case IUP_PLOT_AREA:
    DrawDataArea(*theXData, *theYData, theXCount, canvas);
    break;
  case IUP_PLOT_BAR:
    DrawDataBar(*theXData, *theYData, theXCount, canvas);
    break;
  }

  return true;
}

