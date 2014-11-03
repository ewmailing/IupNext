/* Single plot class.
   Multiple plots can be used in the same IupPlot element.

   This is a re-write of the PPlot library,
   directly using CD for drawing and IUP callback for interaction.
*/

#include "iup.h"

#include "iup_array.h"
#include "iup_str.h"

#include <cd.h>

#ifndef __IUPPLOT_H__
#define __IUPPLOT_H__


const double kFloatSmall = 1e-20;
const double kLogMinClipValue = 1e-10;  // pragmatism to avoid problems with small values in log plot

int iupPlotCalcPrecision(double inValue);

inline int iupPlotRound(double inFloat)
{
  return (int)(inFloat>0 ? inFloat + 0.5 : inFloat - 0.5);
}

inline double iupPlotLog(double inFloat, double inBase)
{
  const double kLogMin = 1e-10;  // min argument for log10 function
  if (inFloat<kLogMin) inFloat = kLogMin;
  return log10(inFloat) / log10(inBase);
}

inline double iupPlotExp(double inFloat, double inBase)
{
  const double kExpMax = 1e10;  // max argument for pow10 function
  if (inFloat>kExpMax) inFloat = kExpMax;
  return pow(inBase, inFloat);
}

class iupPlotRect 
{
public:
  iupPlotRect() :mX(0), mY(0), mWidth(0), mHeight(0){}
  iupPlotRect(int inX, int inY, int inWidth, int inHeight)
    : mX(inX), mY(inY), mWidth(inWidth), mHeight(inHeight) {}

  int mX;
  int mY;
  int mWidth;
  int mHeight;
};

class iupPlotMargin 
{
public:
  iupPlotMargin() :mLeft(0), mRight(0), mTop(0), mBottom(0){}
  iupPlotMargin(int inLeft, int inRight, int inTop, int inBottom)
    : mLeft(inLeft), mRight(inRight), mTop(inTop), mBottom(inBottom) {}

  int mLeft;
  int mRight;
  int mTop;
  int mBottom;
};

class iupPlotAxis;

class iupPlotTrafoBase
{
public:
  virtual ~iupPlotTrafoBase() {}
  virtual double Transform(double inValue) const = 0;
  virtual double TransformBack(double inValue) const = 0;
  virtual bool Calculate(int inBegin, int inEnd, const iupPlotAxis& inAxis) = 0;
};

class iupPlotTrafoLinear : public iupPlotTrafoBase
{
public:
  iupPlotTrafoLinear() :mOffset(0), mSlope(0) {}
  double Transform(double inValue) const;
  double TransformBack(double inValue) const;

  bool Calculate(int inBegin, int inEnd, const iupPlotAxis& inAxis);

  double mOffset;
  double mSlope;
};

class iupPlotTrafoLog : public iupPlotTrafoBase 
{
public:
  iupPlotTrafoLog() :mOffset(0), mSlope(0), mBase(10) {}
  double Transform(double inValue) const;
  double TransformBack(double inValue) const;

  bool Calculate(int inBegin, int inEnd, const iupPlotAxis& inAxis);

  double mOffset;
  double mSlope;
  double mBase;
};

class iupPlotDataBase
{
public:
  iupPlotDataBase(int inSize) :mIsString(false) { mArray = iupArrayCreate(20, inSize); }
  virtual ~iupPlotDataBase() { iupArrayDestroy(mArray); }

  bool IsString() const { return mIsString; }
  int GetCount() const { return iupArrayCount(mArray); }

  virtual bool CalculateRange(double &outMin, double &outMax) = 0;
  virtual double GetValue(int inIndex) const = 0;

  void RemoveItem(int inIndex) { iupArrayRemove(mArray, inIndex, 1); }

protected:
  Iarray* mArray;
  bool mIsString;
};

class iupPlotDataString : public iupPlotDataBase
{
public:
  iupPlotDataString() :iupPlotDataBase(sizeof(char*)) { mIsString = true; }
  ~iupPlotDataString(); 

  char** GetStrData() const { return (char**)iupArrayGetData(mArray); }
  char*  GetStrValue(int inIndex) const { if (inIndex < 0 || inIndex >= iupArrayCount(mArray)) return NULL; char** data = (char**)iupArrayGetData(mArray); return data[inIndex]; }
  double GetValue(int inIndex) const { return inIndex; }

  void AddItem(const char *inString) { char** data = (char**)iupArrayInc(mArray); data[iupArrayCount(mArray)-1] = iupStrDup(inString); }
  void InsertItem(int inIndex, const char *inString) { char** data = (char**)iupArrayInsert(mArray, inIndex, 1); data[inIndex] = iupStrDup(inString); }

  bool CalculateRange(double &outMin, double &outMax);
};

class iupPlotDataReal : public iupPlotDataBase
{
public:
  iupPlotDataReal() :iupPlotDataBase(sizeof(double)) { }

  double* GetRealData() const { return (double*)iupArrayGetData(mArray); }
  double GetValue(int inIndex) const { double* data = (double*)iupArrayGetData(mArray); return data[inIndex]; }

  void AddItem(double inReal) { double* data = (double*)iupArrayInc(mArray); data[iupArrayCount(mArray) - 1] = inReal; }
  void InsertItem(int inIndex, double inReal) { double* data = (double*)iupArrayInsert(mArray, inIndex, 1); data[inIndex] = inReal; }

  bool CalculateRange(double &outMin, double &outMax);
};

enum iupPlotMode { IUP_PLOT_LINE, IUP_PLOT_MARK, IUP_PLOT_MARKLINE, IUP_PLOT_AREA, IUP_PLOT_BAR };

class iupPlotDataSet
{
public:
  iupPlotDataSet(iupPlotDataBase* inDataX, iupPlotDataBase* inDataY, const char* inName, long inColor);
  ~iupPlotDataSet();

  void SetName(const char* inName) { if (mName) free(mName); mName = iupStrDup(inName); }
  const char* GetName() {return mName; }

  long mColor;
  iupPlotMode mMode;
  int mLineStyle;
  int mLineWidth;
  int mMarkStyle;
  int mMarkSize;

  iupPlotDataBase* mDataX;
  iupPlotDataBase* mDataY;

protected:
  char* mName;
};

class iupPlotAxis;
class iupPlotTick;

class iupPlotTickIterBase 
{
public:
  iupPlotTickIterBase() :mAxis(NULL){}
  virtual ~iupPlotTickIterBase() {}
  virtual bool Init() = 0;
  virtual bool GetNextTick(double &outTick, bool &outIsMajorTick, char* outFormatString) = 0;

  virtual bool CalculateSpacing(double inParRange, double inDivGuess, iupPlotTick &outTickInfo) const = 0;
  virtual bool AdjustRange(double &, double &) const { return true; };
  void SetAxis(const iupPlotAxis *inAxis) { mAxis = inAxis; };

protected:
  const iupPlotAxis *mAxis;
};

class iupPlotTickIterLinear : public iupPlotTickIterBase 
{
public:
  iupPlotTickIterLinear() :mCurrentTick(0), mDelta(0){}
  virtual bool Init();
  virtual bool GetNextTick(double &outTick, bool &outIsMajorTick, char* outFormatString);
  bool CalculateSpacing(double inParRange, double inDivGuess, iupPlotTick &outTickInfo) const;
protected:
  double mCurrentTick;
  long mCount;
  double mDelta;
};

class iupPlotTickIterLog : public iupPlotTickIterBase 
{
public:
  iupPlotTickIterLog() :mCurrentTick(0), mDelta(0){}
  virtual bool Init();
  virtual bool GetNextTick(double &outTick, bool &outIsMajorTick, char* outFormatString);

  bool CalculateSpacing(double inParRange, double inDivGuess, iupPlotTick &outTickInfo) const;
  virtual bool AdjustRange(double &ioMin, double &ioMax) const;
  double RoundUp(double inFloat) const;
  double RoundDown(double inFloat) const;

protected:
  double mCurrentTick;
  long mCount;
  double mDelta;
};

class iupPlotTickIterNamed : public iupPlotTickIterLinear 
{
public:
  iupPlotTickIterNamed(){}
  void SetStringList(const iupPlotDataString* inStringData) { mStringData = inStringData; };

  //  virtual bool Init ();
  virtual bool GetNextTick(double &outTick, bool &outIsMajorTick, char* outFormatString);
  bool CalculateSpacing(double inParRange, double inDivGuess, iupPlotTick &outTickInfo) const;
protected:
  const iupPlotDataString* mStringData;
};

class iupPlotTick 
{
public:
  iupPlotTick() 
    :mAutoSpacing(true), mAutoSize(true), mMinorDivision(1), mShowNumber(true),
    mMajorSpan(1), mMajorSize(1), mMinorSize(1), mShow(true),
    mFontSize(0), mFontStyle(-1), mRotateNumber(false)
  { strcpy(mFormatString, "%.0f"); }

  bool mShow;

  bool mShowNumber;
  bool mRotateNumber;
  char mFormatString[30];
  int mFontSize;
  int mFontStyle;

  bool mAutoSpacing;
  int mMinorDivision;
  double mMajorSpan; // in plot units

  bool mAutoSize;
  int mMajorSize;
  int mMinorSize;
};

class iupPlotAxis
{
public:
  iupPlotAxis() 
    : mShow(true), mMin(0), mMax(0), mAutoScaleMin(true), mAutoScaleMax(true),
      mReverse(false), mLogScale(false), mCrossOrigin(false), mColor(CD_BLACK),
      mMaxDecades(-1), mLogBase(10), mLabelCentered(false),
      mDiscrete(false), mLabel(NULL), mShowArrow(true), mLineWidth(1),
      mFontSize(0), mFontStyle(-1) {}
  ~iupPlotAxis() { SetLabel(NULL); }

  void SetLabel(const char* inLabel) { if (mLabel) free(mLabel); mLabel = iupStrDup(inLabel); }
  const char* GetLabel() const { return mLabel; }

  void Init();
  void SetNamedTickIter(const iupPlotDataString *inStringXData);

  bool mShow;
  long mColor;
  double mMin;
  double mMax;
  bool mAutoScaleMin;
  bool mAutoScaleMax;
  bool mReverse;
  bool mCrossOrigin;
  bool mShowArrow;

  int mFontSize;
  int mFontStyle;
  bool mLabelCentered;

  bool mLogScale;
  int mMaxDecades;// property for auto logscale
  double mLogBase;
  bool mDiscrete;

  int mLineWidth;

  iupPlotTick mTick;

  iupPlotTrafoBase *mTrafo;
  iupPlotTickIterBase *mTickIter;

protected:
  char* mLabel;

  iupPlotTrafoLinear mLinTrafo;
  iupPlotTrafoLog mLogTrafo;

  iupPlotTickIterLinear mLinTickIter;
  iupPlotTickIterLog mLogTickIter;
  iupPlotTickIterNamed mNamedTickIter;
};

class iupPlotGrid 
{
public:
  iupPlotGrid()
    : mXGridOn(false), mYGridOn(false), mColor(cdEncodeColor(200, 200, 200)), 
      mLineStyle(CD_CONTINUOUS), mLineWidth(1) {}

  bool mXGridOn;
  bool mYGridOn;
  long mColor;
  int  mLineStyle;
  int  mLineWidth;
};

class iupPlotBox
{
public:
  iupPlotBox()
    : mShow(false), mColor(CD_BLACK),
      mLineStyle(CD_CONTINUOUS), mLineWidth(1) {}

  bool mShow;
  long mColor;
  int  mLineStyle;
  int  mLineWidth;
};

enum iupPlotLegendPosition { IUP_PLOT_TOPLEFT, IUP_PLOT_TOPRIGHT, IUP_PLOT_BOTTOMLEFT, IUP_PLOT_BOTTOMRIGHT, IUP_PLOT_BOTTOMCENTER };

class iupPlotLegend
{
public:
  iupPlotLegend() 
    : mShow(false), mFontSize(0), mFontStyle(-1), mPosition(IUP_PLOT_TOPRIGHT),
      mBoxLineStyle(CD_CONTINUOUS), mBoxLineWidth(1), mBoxColor(CD_BLACK) 
  { mBoxBackColor = cdEncodeAlpha(CD_WHITE, 0); }

  bool mShow;
  iupPlotLegendPosition mPosition;
  int mFontSize;
  int mFontStyle;

  long mBoxColor;
  long mBoxBackColor;
  int  mBoxLineStyle;
  int  mBoxLineWidth;
};

class iupPlotTitle
{
public:
  iupPlotTitle() 
    : mColor(CD_BLACK), mText(NULL), mFontSize(0), mFontStyle(-1) {}
  ~iupPlotTitle() { SetText(NULL); }

  void SetText(const char* inValue) { if (mText) free(mText); if (inValue) mText = iupStrDup(inValue); }
  const char* GetText() const { return mText; }

  long mColor;
  int mFontSize;
  int mFontStyle;

protected:
  char* mText;
};

#define IUP_PLOT_MAX_DS 20

class iupPlot
{
public:
  iupPlot(Ihandle* ih);
  ~iupPlot();

  void SetViewport(int x, int y, int w, int h);
  bool Render(cdCanvas* canvas);
  void Configure();
  void SetFont(cdCanvas* canvas, int inFontStyle, int inFontSize) const;

  void DrawPlotTitle(cdCanvas* canvas) const;
    void SetTitleFont(cdCanvas* canvas) const;
  void DrawPlotBackground(cdCanvas* canvas) const;
  void DrawBox(const iupPlotRect &inRect, cdCanvas* canvas) const;
  bool DrawXGrid(const iupPlotRect &inRect, cdCanvas* canvas) const;
  bool DrawYGrid(const iupPlotRect &inRect, cdCanvas* canvas) const;
  bool DrawXAxis(const iupPlotRect &inRect, cdCanvas* canvas) const;
  bool DrawYAxis(const iupPlotRect &inRect, cdCanvas* canvas) const;
    bool DrawXTick(double inX, double inScreenY, bool inMajor, const char* inFormatString, cdCanvas* canvas) const;
    bool DrawYTick(double inY, double inScreenX, bool inMajor, const char* inFormatString, cdCanvas* canvas) const;
  bool DrawLegend(const iupPlotRect &inRect, cdCanvas* canvas) const;
  bool DrawPlot(int inIndex, cdCanvas* canvas) const;
    void DrawDataLine(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const;
    void DrawDataMark(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const;
    void DrawDataMarkLine(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const;
    void DrawDataArea(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const;
    void DrawDataBar(const iupPlotDataBase &inXData, const iupPlotDataBase &inYData, int inCount, cdCanvas* canvas) const;

  void CalculateMargins(cdCanvas* canvas);
  bool CalculateAxisRanges();
    bool CalculateXRange(double &outXMin, double &outXMax);
    bool CalculateYRange(double &outYMin, double &outYMax);
  bool CheckRange(const iupPlotAxis &inAxis);
  bool CalculateYTransformation(const iupPlotRect &inRect);
  bool CalculateXTransformation(const iupPlotRect &inRect);
    bool CalculateLinTransformation(int inBegin, int inEnd, const iupPlotAxis& inAxis, iupPlotTrafoLinear* outTrafo);
    bool CalculateLogTransformation(int inBegin, int inEnd, const iupPlotAxis& inAxis, iupPlotTrafoLog* outTrafo);
  bool CalculateTickSpacing(const iupPlotRect &inRect, cdCanvas* canvas);
  void CalculateTickSize(cdCanvas* canvas, iupPlotTick &ioTick);
    void GetTickNumberSize(const iupPlotAxis& inAxis, cdCanvas* canvas, int *outWitdh, int *outHeight) const;

  void AddDataSet(iupPlotDataBase* inDataX, iupPlotDataBase* inDataY);
  void RemoveDataset(int inIndex);
  int FindDataset(const char* inName);
  void RemoveAllDatasets();
  long GetNextDatasetColor();

  Ihandle* ih;
  bool mRedraw;
  iupPlotRect mViewport;
  int mDefaultFontSize;
  int mDefaultFontStyle;

  iupPlotMargin mMargin, mMarginAuto;
  long mBackColor;
  iupPlotGrid mGrid;
  iupPlotAxis mAxisX;
  iupPlotAxis mAxisY;
  iupPlotBox mBox;
  iupPlotLegend mLegend;
  iupPlotTitle mTitle;

  iupPlotDataSet* mDataSetList[IUP_PLOT_MAX_DS];
  int mDataSetListCount;
  int mCurrentDataSet;
};

#endif
