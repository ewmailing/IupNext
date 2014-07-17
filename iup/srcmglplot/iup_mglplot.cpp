/*
* IupMglPlot component
*
* Description : A component, derived from MathGL and IupGLCanvas
*      Remark : Depend on libs IUP, IUPGL, OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <float.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"
#include "iupgl.h"

#include "iup_mglplot.h"

#include "iup_assert.h"
#include "iup_class.h"
#include "iup_register.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_array.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#pragma warning(push, 0)
#include "mgl2/mgl.h"
#include "mgl2/eval.h"
#pragma warning(pop)


enum {IUP_MGLPLOT_BOTTOMLEFT, IUP_MGLPLOT_BOTTOMRIGHT, IUP_MGLPLOT_TOPLEFT, IUP_MGLPLOT_TOPRIGHT};
enum {IUP_MGLPLOT_INHERIT, IUP_MGLPLOT_PLAIN, IUP_MGLPLOT_BOLD, IUP_MGLPLOT_ITALIC, IUP_MGLPLOT_BOLD_ITALIC};


typedef struct _IdataSet
{
  char dsLineStyle;
  int dsLineWidth;
  bool dsShowMarks;
  char dsMarkStyle;
  double dsMarkSize;
  bool dsShowValues;
  char* dsMode;
  char* dsLegend;
  mglColor dsColor;  // should never has NAN values

  int dsDim;        /* Dimension of the data: 1D, 2D or 3D */
  Iarray* dsNames;  /* optional names used in ticks when in 1D */
  mglData* dsX;
  mglData* dsY;
  mglData* dsZ;
  int dsCount;
} IdataSet;

typedef struct _Iaxis
{
  mglColor axColor;
  double axOrigin;
  const char* axScale;
  bool axShow, axShowArrow;

  int axLabelPos;
  int axLabelFontStyle;
  double axLabelFontSizeFactor;
  bool axLabelRotation;

  int axTickFontStyle;
  double axTickFontSizeFactor;
  bool axTickShowValues, axTickShow, 
       axTickAutoSpace, axTickAutoSize;
  double axTickMinorSizeFactor, axTickMajorSize;          
  int axTickMinorDivision;
  double axTickMajorSpan;
  bool axTickValuesRotation;

  bool axReverse;
  double axMax, axMin;
  bool axAutoScaleMax, axAutoScaleMin;
} Iaxis;

struct _IcontrolData
{
  iupCanvas canvas;     /* from IupCanvas (must reserve it)  */

  mglGraph* mgl;

  /* Control */
  int w, h;
  double dpi;
  bool redraw;
  bool opengl;

  /* Obtained from FONT */
  char FontDef[32];     
  double FontSizeDef;
  int FontStyleDef;
  
  /* Global */
  bool transparent;
  double alpha;
  mglColor bgColor, fgColor;

  /* Title */
  mglColor titleColor;
  int titleFontStyle;
  double titleFontSizeFactor;

  /* Axes */
  Iaxis axisX, axisY, axisZ;

  /* Interaction */
  double last_x, last_y;  // Last Mouse Position
  double x1, x2, y1, y2;  // Zoom+Pan
  double rotX, rotZ, rotY;  // Rotate (angle in degrees)

  /* Box */
  bool Box, boxTicks;
  mglColor boxColor;

  /* Legend */
  bool legendShow, legendBox;
  mglColor legendColor;
  int legendPosition;
  int legendFontStyle;
  double legendFontSizeFactor;

  /* Grid */
  mglColor gridColor;
  const char* gridShow;
  char gridLineStyle;

  /* Dataset */
  int dataSetCurrent;
  int dataSetCount, dataSetMaxCount;
  IdataSet* dataSet;
};

/* Callbacks function pointer typedefs. */
// TODO
// typedef int (*IFniidd)(Ihandle*, int, int, double, double); /* delete_cb */
// typedef int (*IFniiddi)(Ihandle*, int, int, double, double, int); /* select_cb */
// typedef int (*IFniidddd)(Ihandle*, int, int, double, double, double*, double*); /* edit_cb */

/******************************************************************************
 Useful Functions
******************************************************************************/

// Quantize 0-1 values into 0-255.
inline unsigned char iQuant(const double& value)
{
  if (value >= 1.0) return 255;
  if (value <= 0.0) return 0;
  return (unsigned char)(value*256.0);
}                               

// Reconstruct 0-255 values into 0-1.
inline double iRecon(const unsigned char& value)
{
  if (value <= 0) return 0.0;
  if (value >= 255) return 1.0;
  return ((double)value + 0.5f)/256.0;
}

inline void iSwap(double& a, double& b)
{
  double tmp = a;
  a = b;
  b = tmp;
}

static char* iMglPlotDefaultLegend(int ds)
{
  char legend[50];
  sprintf(legend, "plot %d", ds);
  return iupStrDup(legend);
}

static void iMglPlotSetDsColorDefault(mglColor &color, int ds_index)
{
  switch(ds_index)
  {
    case 0: color.Set(1, 0, 0); break;
    case 1: color.Set(0, 0, 1); break;
    case 2: color.Set(0, 1, 0); break;
    case 3: color.Set(0, 1, 1); break;
    case 4: color.Set(1, 0, 1); break;
    case 5: color.Set(1, 1, 0); break;
    default: color.Set(0, 0, 0); break;
  }
}

static void iMglPlotResetDataSet(IdataSet* ds, int ds_index)
{
  // Can NOT use memset here

  ds->dsLineStyle  = '-';  // SOLID/CONTINUOUS
  ds->dsLineWidth  = 1;
  ds->dsMarkStyle  = 'x';  // "X"
  ds->dsMarkSize   = 0.02f;
  ds->dsShowMarks = false;
  ds->dsShowValues = false;
  ds->dsMode = iupStrDup("LINE");
  ds->dsLegend = iMglPlotDefaultLegend(ds_index);

  iMglPlotSetDsColorDefault(ds->dsColor, ds_index);
}

static void iMglPlotResetInteraction(Ihandle *ih)
{
  ih->data->x1 = 0.0;  ih->data->y1 = 0.0;
  ih->data->x2 = 1.0;  ih->data->y2 = 1.0;

  ih->data->last_x = 0.0;
  ih->data->last_y = 0.0;

  ih->data->rotX = 0.0;   ih->data->rotZ = 0.0;   ih->data->rotY = 0.0;
}

static void iMglPlotResetAxis(Iaxis& axis)
{
  memset(&axis, 0, sizeof(Iaxis));

  axis.axLabelFontSizeFactor = 1;
  axis.axTickFontSizeFactor = 0.8f;

  axis.axColor.Set(NAN, NAN, NAN);

  axis.axAutoScaleMin = true;
  axis.axAutoScaleMax = true;
  axis.axMin = -1;
  axis.axMax = 1;
  axis.axShow = true;
  axis.axShowArrow = true;
  axis.axLabelRotation = true;
  axis.axOrigin = NAN;

  axis.axTickShow = true;
  axis.axTickShowValues = true;
  axis.axTickValuesRotation = true;
  axis.axTickAutoSpace = true;
  axis.axTickAutoSize = true;
  axis.axTickMinorSizeFactor = 0.6f;
  axis.axTickMajorSize = 0.1f;
  axis.axTickMinorDivision = 5;
  axis.axTickMajorSpan = -5;
}

static void iMglPlotReset(Ihandle* ih)
{
  // Can NOT use memset here

  ih->data->legendPosition = IUP_MGLPLOT_TOPRIGHT;
  ih->data->legendBox = true;
  ih->data->legendShow = false;
  ih->data->Box = false;
  ih->data->boxTicks = true;
  ih->data->gridLineStyle = '-';
  ih->data->gridShow = NULL;

  ih->data->alpha = 0.5f;
  ih->data->transparent = false;

  ih->data->titleFontStyle = IUP_MGLPLOT_INHERIT;
  ih->data->legendFontStyle = IUP_MGLPLOT_INHERIT;
  ih->data->legendFontSizeFactor = 0.8f;
  ih->data->titleFontSizeFactor = 1.4f;

  ih->data->bgColor.Set(1, 1, 1);
  ih->data->fgColor.Set(0, 0, 0);
  ih->data->gridColor.Set((float)iRecon(200), (float)iRecon(200), (float)iRecon(200));
  ih->data->titleColor.Set(NAN, NAN, NAN);
  ih->data->legendColor.Set(NAN, NAN, NAN);
  ih->data->boxColor.Set(NAN, NAN, NAN);

  iMglPlotResetAxis(ih->data->axisX);
  iMglPlotResetAxis(ih->data->axisY);
  iMglPlotResetAxis(ih->data->axisZ);

  /* Interaction default values */
  iMglPlotResetInteraction(ih);
}

static bool iMglPlotIsPlanarOrVolumetricData(IdataSet* ds)
{
  return (ds->dsDim == 1 && (ds->dsX->ny>1 || ds->dsX->nz>1));
}

static bool iMglPlotIsPlanarData(IdataSet* ds)
{
  return (ds->dsDim == 1 && ds->dsX->ny>1 && ds->dsX->nz==1);
}

static bool iMglPlotIsVolumetricData(IdataSet* ds)
{
  return (ds->dsDim == 1 && ds->dsX->nz>1);
}

static bool iMglPlotIsView3D(Ihandle* ih)
{
  int i;
  for (i=0; i< ih->data->dataSetCount; i++)
  {
    IdataSet* ds = &ih->data->dataSet[i];
    if ((ds->dsDim == 1 && (ds->dsX->ny>1 || ds->dsX->nz>1)) ||  // Planar or Volumetric
        ds->dsDim == 3)  // 3D data
    {
      if (!iupStrEqualNoCase(ds->dsMode, "PLANAR_DENSITY") &&
          !iupStrEqualNoCase(ds->dsMode, "PLANAR_GRADIENTLINES"))
        return true;
    }
  }

  return false;
}

static char* iMglPlotGetTexFontName(const char* typeface)
{
  if (iupStrEqualNoCase(typeface, "sans") ||
      iupStrEqualNoCase(typeface, "helvetica") ||
      iupStrEqualNoCase(typeface, "arial"))
    return "heros";
  if (iupStrEqualNoCase(typeface, "monospace") ||
      iupStrEqualNoCase(typeface, "courier") ||
      iupStrEqualNoCase(typeface, "courier new"))
    return "cursor";
  if (iupStrEqualNoCase(typeface, "serif") ||
      iupStrEqualNoCase(typeface, "times") ||
      iupStrEqualNoCase(typeface, "times new roman"))
    return "termes";
  return NULL;
}

static void iMglPlotConfigFontDef(Ihandle* ih, mglGraph *gr)
{
  int size = 0, i = 0;
  int is_bold = 0,
    is_italic = 0, 
    is_underline = 0,
    is_strikeout = 0;
  char typeface[1024];
  
  const char* font = iupGetFontAttrib(ih);
  if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return;

  if (is_bold && is_italic)
  {
    ih->data->FontStyleDef = IUP_MGLPLOT_BOLD_ITALIC;
    ih->data->FontDef[i++] = 'b';
    ih->data->FontDef[i++] = 'i';
  }
  else if (is_bold)
  {
    ih->data->FontStyleDef = IUP_MGLPLOT_BOLD;
    ih->data->FontDef[i++] = 'b';
  }
  else if (is_italic)
  {
    ih->data->FontStyleDef = IUP_MGLPLOT_ITALIC;
    ih->data->FontDef[i++] = 'i';
  }
  else
  {
    ih->data->FontStyleDef = IUP_MGLPLOT_PLAIN;
    ih->data->FontDef[i++] = 'r';
  }

  if (is_underline)
    ih->data->FontDef[i++] = 'u';

  if (is_strikeout)
    ih->data->FontDef[i++] = 'o';

  ih->data->FontDef[i] = 0;

  if (size < 0) 
    size = -size;
  else 
    size = (int)((size*ih->data->dpi)/72.0);   //from points to pixels

  //IMPORTANT: 
  //  Magic factor for acceptable size. 
  //  Don't know why it works, but we obtain good results.
  ih->data->FontSizeDef = ((double)size/(double)ih->data->h)*ih->data->dpi;

  char *path = getenv("IUP_MGLFONTS");
  if (!path) 
    path = IupGetGlobal("MGLFONTS");

  if (!iupStrEqualNoCase(typeface, iupAttribGetStr(ih, "_IUP_MGL_FONTNAME")) ||
      !iupStrEqualNoCase(path, iupAttribGetStr(ih, "_IUP_MGL_FONTPATH")))
  {
    const char* fontname = iMglPlotGetTexFontName(typeface);
    if (!fontname)
      fontname = typeface;
    gr->LoadFont(fontname, path);  // Load MathGL VFM fonts

    iupAttribSetStr(ih, "_IUP_MGL_FONTNAME", typeface);
    iupAttribSetStr(ih, "_IUP_MGL_FONTPATH", path);
  }
}

static void iMglPlotConfigFont(Ihandle* ih, mglGraph *gr, int fontstyle, double fontsizefactor)
{
  int i=0;
  char fnt[32];

  if (fontstyle==IUP_MGLPLOT_INHERIT)
    fontstyle = ih->data->FontStyleDef;

  if (fontstyle==IUP_MGLPLOT_PLAIN)
    fnt[i++] = 'r';
  else if (fontstyle==IUP_MGLPLOT_BOLD)
    fnt[i++] = 'b';
  else if (fontstyle==IUP_MGLPLOT_ITALIC)
    fnt[i++] = 'i';
  else if (fontstyle==IUP_MGLPLOT_BOLD_ITALIC)
  {
    fnt[i++] = 'b';
    fnt[i++] = 'i';
  }

  fnt[i] = 0;

  if (i!=0)
    gr->SetFontDef(fnt);
  else
    gr->SetFontDef(ih->data->FontDef);

  gr->SetFontSize(fontsizefactor*ih->data->FontSizeDef);
}

static mglColor iMglPlotGetColorDefault(Ihandle* ih, mglColor color)
{
  if (mgl_isnan(color.r) || mgl_isnan(color.g) || mgl_isnan(color.b))
    color = ih->data->fgColor;
  return color;
}

static double iMglPlotGetAttribDoubleNAN(Ihandle* ih, const char* name)
{
  double val = NAN;
  iupStrToDouble(iupAttribGet(ih, name), &val);
  return val;
}

static bool iMglPlotHasx10(double min, double max)
{
  if (fabs(max - min)<0.01*fabs(min))
  {
    double v = fabs(max - min);
    if (v>10000.f || v<1e-4f)
      return true;
  }
  else
  {
    double v = fabs(max)>fabs(min) ? fabs(max) : fabs(min);
    if (v>10000.f || v<1e-4f)
      return true;
  }
  return false;
}

static void iMglPlotConfigColor(Ihandle* ih, mglGraph *gr, mglColor color)
{
//  Finally, you can specify RGB or RGBA values of a color using format ‘{ xRRGGBB }’ or
//    ‘{ xRRGGBBAA }’ correspondingly.For example, ‘{ xFF9966 }’ give you melone color.
  //color = iMglPlotGetColorDefault(ih, color);
}

static char* iMglPlotConfigPen(char* pen, char line_style, int line_width)
{
  *pen++ = line_style;

  if (line_width < 1) line_width = 1;
  if (line_width > 9) line_width = 9;
  *pen++ = (char)('0' + line_width);

  *pen = 0;
  return pen;
}

static char iMglPlotFindColor(const mglColor& c1)
{
  int i;
  char id = 0;
  mreal old_diff = -1, diff, rr, bb, gg;

  /* Not the best solution, 
     but the only one possible witht he current MglPlot limitations */

	for(i=0; mglColorIds[i].id; i++)
  {
    const mglColor& c2 = mglColorIds[i].col;
    rr = c1.r-c2.r;
    gg = c1.g-c2.g;
    bb = c1.b-c2.b;
    diff = rr*rr + gg*gg + bb*bb;
    if (old_diff==-1 ||
        diff < old_diff)
    {
      id = mglColorIds[i].id;
      old_diff = diff;
    }
  }

  return id;
}

static void iMglPlotConfigDataSetColor(IdataSet* ds, char* style)
{
  style += strlen(style); // Skip previous configuration

  char id = iMglPlotFindColor(ds->dsColor);
  if (id)
    *style++ = id;

  *style = 0;
}

static void iMglPlotConfigDataSetLineMark(IdataSet* ds, mglGraph *gr, char* style)
{
  style += strlen(style); // Skip previous configuration
  style = iMglPlotConfigPen(style, ds->dsLineStyle, ds->dsLineWidth);

  if (ds->dsShowMarks)
  {
    // Fix mark codes
    if (ds->dsMarkStyle == 'O' || ds->dsMarkStyle == 'S' || ds->dsMarkStyle == 'D' ||
        ds->dsMarkStyle == '^' || ds->dsMarkStyle == 'V' || ds->dsMarkStyle == '<' || ds->dsMarkStyle == '>')
    {
      *style++ = '#';   // Conflict with datagrid, can not use these marks
      *style++ = (char)tolower(ds->dsMarkStyle);
    }
    else
      *style++ = ds->dsMarkStyle;

    gr->SetMarkSize(ds->dsMarkSize);
  }

  *style = 0;
}

static void iMglPlotConfigAxisTicks(Ihandle* ih, mglGraph *gr, char dir, const Iaxis& axis, double min, double max)
{
  if (!axis.axTickShow)
  {
    gr->SetTicks(dir, 0, 0);
    return;
  }

  //Set Scale only if ticks are enabled
  if (axis.axScale)  // Logarithm Scale
  {
    if (dir == 'x')
      gr->SetFunc(axis.axScale, 0, 0, 0);
    if (dir == 'y')
      gr->SetFunc(0, axis.axScale, 0, 0);
    if (dir == 'z')
      gr->SetFunc(0, 0, axis.axScale, 0);

    gr->SetTicks(dir, 0, 0);  // Disable normal ticks, logarithm ticks will be done
  }
  else  
  {
    // step for axis ticks (if positive) or 
    // it’s number on the axis range (if negative).
    double step = -5;
    int numsub = 0;  // Number of axis submarks

    if (axis.axTickAutoSpace)
    {
      // Copied from MathGL AdjustTicks internal function
      // TODO: improve autoticks computation
      mreal n, d = fabs(max-min);
      n = floor(log10(d));  
      d = floor(d*pow(10,-n));
      step = pow(10,n);
      if (d<4)
      {
        step *= 0.5f;
        numsub = 4;
      }
      else if(d<7)
        numsub = 4;
      else
      {
        step *= 2;
        numsub = 3;
      }
    }
    else
    {
      numsub = axis.axTickMinorDivision-1;
      step = axis.axTickMajorSpan;
    }

    gr->SetTicks(dir, step, numsub);
  }

  // Setting the template, switches off automatic ticks tuning
  char attrib[16] = "AXS_?TICKFORMAT";
  attrib[4] = dir;
  iupStrUpper(attrib, attrib);
  char* format = iupAttribGetStr(ih, attrib);
  if (format && axis.axTickShowValues)
  {
    if (dir == 'x') gr->SetTickTempl('x', format);
    if (dir == 'y') gr->SetTickTempl('y', format);
    if (dir == 'z') gr->SetTickTempl('z', format);
    if (dir == 'c') gr->SetTickTempl('c', format);
  }
}

static void iMglPlotConfigScale(Iaxis& axis, double& min, double& max)
{
  if (!axis.axAutoScaleMax)
    max = axis.axMax;
  else
    axis.axMax = max;  // Update the attribute value
  if (!axis.axAutoScaleMin)
    min = axis.axMin;
  else
    axis.axMin = min;  // Update the attribute value

  if (axis.axScale)  // Logarithm Scale
  {
    if (axis.axMin < 0)
    {
      axis.axMin = 0;
      min = 0;
    }
    if (axis.axMax < 0)
    {
      axis.axMax = 0;
      max = 0;
    }
  }

  if (axis.axReverse)
    iSwap(min, max);
}

static void iMglPlotFindMinMaxValues(mglData& ds_data, bool add, double& min, double& max)
{
  int i, count = ds_data.nx*ds_data.ny*ds_data.nz;
  double* data = ds_data.a;

  for (i = 1; i < count; i++, data++)
  {
    if (mgl_isnan(*data))
      continue;

    if (!add)  // first valid found
    {
      min = *data;
      max = *data;
      add = true;
    }
    else
    {
      min = min<*data ? min: *data;
      max = max>*data ? max: *data;
    }
  }
}

static void iMglPlotConfigAxesRange(Ihandle* ih, mglGraph *gr)
{
  int i;
  mglPoint Min(ih->data->axisX.axMin, ih->data->axisY.axMin, ih->data->axisZ.axMin);
  mglPoint Max(ih->data->axisX.axMax, ih->data->axisY.axMax, ih->data->axisZ.axMax);

  if (ih->data->axisX.axAutoScaleMax || ih->data->axisX.axAutoScaleMin ||
      ih->data->axisY.axAutoScaleMax || ih->data->axisY.axAutoScaleMin ||
      ih->data->axisZ.axAutoScaleMax || ih->data->axisZ.axAutoScaleMin)
  {
    for(i = 0; i < ih->data->dataSetCount; i++)
    {
      IdataSet* ds = &ih->data->dataSet[i];
      bool add = i==0? false: true;
      mglPoint oldMin = Min;
      mglPoint oldMax = Max;

      if (iupStrEqualNoCase(ds->dsMode, "RADAR"))
        add = false;

      if (ds->dsDim == 1)
      {
        if (ds->dsX->ny>1 || ds->dsX->nz>1)  /* Planar or Volumetric data */
        {
          if (ih->data->axisX.axAutoScaleMax || ih->data->axisX.axAutoScaleMin)
          {
            Min.x = -1.0;
            Max.x = +1.0;
          }
          if (ih->data->axisY.axAutoScaleMax || ih->data->axisY.axAutoScaleMin)
          {
            Min.y = -1.0;
            Max.y = +1.0;
          }
          if (ih->data->axisZ.axAutoScaleMax || ih->data->axisZ.axAutoScaleMin)
          {
            Min.z = -1.0;
            Max.z = +1.0;
          }
        }
        else /* 1D Linear data */
        {
          /* the data will be plotted as Y, X will be 0,1,2,3,... */
          if (ih->data->axisX.axAutoScaleMax || ih->data->axisX.axAutoScaleMin)
          {
            Min.x = 0;
            double ds_max = (mreal)(ds->dsCount-1);
            Max.x = i==0? ds_max: (ds_max>Max.x? ds_max: Max.x);
          }
          if (ih->data->axisY.axAutoScaleMax || ih->data->axisY.axAutoScaleMin)
            iMglPlotFindMinMaxValues(*ds->dsX, add, Min.y, Max.y);

          if (iupStrEqualNoCase(ds->dsMode, "BARHORIZONTAL"))
          {
            iSwap(Min.x, Min.y);
            iSwap(Max.x, Max.y);
          }
          else if (iupStrEqualNoCase(ds->dsMode, "RADAR"))
          {
            double r = iupAttribGetDouble(ih, "RADARSHIFT");   // Default -1
          	if(r<0)	r = (Min.y<0)? -Min.y:0;
            double rmax = fabs(Max.y)>fabs(Min.y)?fabs(Max.y):fabs(Min.y);
            double min = -(r + rmax);
            double max =  (r + rmax);
            if (i==0)
            {
              Min.y = Min.x = min;
              Max.y = Max.x = max;
            }
            else
            {
              if (min < oldMin.x)
                Min.y = Min.x = min;
              else
                Min.y = Min.x = oldMin.x;
              if (max > oldMax.x)
                Max.y = Max.x = max;
              else
                Max.y = Max.x = oldMax.x;
            }
            oldMin = Min;
            oldMax = Max;
          }
          else if (iupStrEqualNoCase(ds->dsMode, "CHART") && iupAttribGetBoolean(ih, "PIECHART"))
          {
            double rmax = fabs(Max.y)>fabs(Min.y)?fabs(Max.y):fabs(Min.y);
            Min.y = Min.x = -rmax;
            Max.y = Max.x =  rmax;
          }
        }
      }
      else if (ds->dsDim == 2)  /* 2D Linear data */
      {
        if (ih->data->axisX.axAutoScaleMax || ih->data->axisX.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsX, add, Min.x, Max.x);
        if (ih->data->axisY.axAutoScaleMax || ih->data->axisY.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsY, add, Min.y, Max.y);
      }
      else if (ds->dsDim == 3)  /* 2D Linear data */
      {
        if (ih->data->axisX.axAutoScaleMax || ih->data->axisX.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsX, add, Min.x, Max.x);
        if (ih->data->axisY.axAutoScaleMax || ih->data->axisY.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsY, add, Min.y, Max.y);
        if (ih->data->axisZ.axAutoScaleMax || ih->data->axisZ.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsZ, add, Min.z, Max.z);
      }
    }
  }

  iMglPlotConfigScale(ih->data->axisX, Min.x, Max.x);
  iMglPlotConfigScale(ih->data->axisY, Min.y, Max.y);
  iMglPlotConfigScale(ih->data->axisZ, Min.z, Max.z);

  gr->SetRange('x', ih->data->axisX.axMin, ih->data->axisX.axMax);
  gr->SetRange('y', ih->data->axisY.axMin, ih->data->axisY.axMax);
  gr->SetRange('z', ih->data->axisZ.axMin, ih->data->axisZ.axMax);

  iMglPlotConfigAxisTicks(ih, gr, 'x', ih->data->axisX, ih->data->axisX.axMin, ih->data->axisX.axMax);
  iMglPlotConfigAxisTicks(ih, gr, 'y', ih->data->axisY, ih->data->axisY.axMin, ih->data->axisY.axMax);
  iMglPlotConfigAxisTicks(ih, gr, 'z', ih->data->axisZ, ih->data->axisZ.axMin, ih->data->axisZ.axMax);


  // Color Range
  double Cmin, Cmax;
  char* coloraxisticks = iupAttribGetStr(ih, "COLORBARAXISTICKS");
  char* value = iupAttribGetStr(ih, "COLORBARRANGE");
  if (!value || iupStrToDoubleDouble(value, &Cmin, &Cmax, ':') != 2) 
  {
    if (coloraxisticks && tolower(*coloraxisticks) == 'x') 
    {
      Cmin = ih->data->axisX.axMin;
      Cmax = ih->data->axisX.axMax;
    }
    else if (coloraxisticks && tolower(*coloraxisticks) == 'y') 
    {
      Cmin = ih->data->axisY.axMin;
      Cmax = ih->data->axisY.axMax;
    }
    else 
    {
      // By default set CMin-Max to Z Min-Max
      Cmin = ih->data->axisZ.axMin;
      Cmax = ih->data->axisZ.axMax;
    }
  }
  gr->SetRange('c', Cmin, Cmax);

  if (coloraxisticks && tolower(*coloraxisticks) == 'x') 
    iMglPlotConfigAxisTicks(ih, gr, 'c', ih->data->axisX, Cmin, Cmax);
  else if (coloraxisticks && tolower(*coloraxisticks) == 'y') 
    iMglPlotConfigAxisTicks(ih, gr, 'c', ih->data->axisY, Cmin, Cmax);
  else 
    iMglPlotConfigAxisTicks(ih, gr, 'c', ih->data->axisZ, Cmin, Cmax);


  // Origin
  gr->SetOrigin(ih->data->axisX.axOrigin, ih->data->axisY.axOrigin, ih->data->axisZ.axOrigin);
}

static void iMglPlotDrawAxisLabel(Ihandle* ih, mglGraph *gr, char dir, Iaxis& axis)
{
  char attrib[11] = "AXS_?LABEL";
  attrib[4] = dir;
  iupStrUpper(attrib, attrib);
  char* label = iupAttribGetStr(ih, attrib);
  if (label)
  {
    gr->SetRotatedText(axis.axLabelRotation);

    iMglPlotConfigFont(ih, gr, axis.axLabelFontStyle, axis.axLabelFontSizeFactor);
    
    // TODO sometimes the label gets too close to the ticks
    gr->Label(dir, label, (mreal)axis.axLabelPos, "");  

    gr->SetRotatedText(false);
  }
}

static void iMglPlotConfigAxisTicksVal(Ihandle* ih, mglGraph *gr, bool set)
{
  IdataSet* ds = &ih->data->dataSet[0];  // Allow names only for the first dataset
  if (ds->dsNames)
  {
    // TODO: TicksVal should follow ticks spacing configuration
    if (set)
    {
      char** dsNames = (char**)iupArrayGetData(ds->dsNames);
      int count = iupArrayCount(ds->dsNames);
      mglData val(count);
      int i, total_size = 0;
      for (i = 0; i < count; i++)
      {
        total_size += (int)strlen(dsNames[i]) + 1;
        val.a[i] = (mreal)i;
      }
      char* labels = new char[total_size + 1];
      labels[0] = 0;
      for (i = 0; i < count; i++)
      {
        strcat(labels, dsNames[i]);
        strcat(labels, "\n");
      }
      gr->SetTicksVal('x', val, labels);
      delete[] labels;
    }
    else
      gr->SetTicksVal('x', (wchar_t*)NULL, false);
  }
}

static void iMglPlotConfigAxisTicksLen(mglGraph *gr, Iaxis& axis)
{
  if (!axis.axTickAutoSize)
  {
    //TODO: documentation says negative len puts tic outside the bounding box, 
    //      but it is NOT working
    mreal stt = 1.0/(axis.axTickMinorSizeFactor*axis.axTickMinorSizeFactor) - 1.0;
    gr->SetTickLen(axis.axTickMajorSize, stt);
  }
  else
  {
    mreal stt = 1.0/(0.6f*0.6f) - 1.0;
    gr->SetTickLen(0.1f, stt);
  }
}

static void iMglPlotDrawAxis(Ihandle* ih, mglGraph *gr, char dir, Iaxis& axis)
{
  iMglPlotConfigColor(ih, gr, axis.axColor);
  iMglPlotConfigFont(ih, gr, axis.axTickFontStyle, axis.axTickFontSizeFactor);

  // Must be set here, because there is only one for all the axis.
  iMglPlotConfigAxisTicksLen(gr, axis);

  // Must be after configuring the ticks
  if (dir == 'x')
    iMglPlotConfigAxisTicksVal(ih, gr, true);

  // Configure ticks values rotation along axis
  gr->SetRotatedText(axis.axTickValuesRotation);

  // Draw the axis, ticks and ticks values
  int i = 0;
  char style[10];
  style[i++]=dir;
  if (!axis.axTickShowValues)
    style[i++]='_';
  if (axis.axShowArrow)
    style[i++]='T';
  gr->Axis(style);  

  iMglPlotDrawAxisLabel(ih, gr, dir, axis);

  // Reset to default
  if (dir == 'x')
    iMglPlotConfigAxisTicksVal(ih, gr, false);

  gr->SetRotatedText(false);
}

static void iMglPlotDrawAxes(Ihandle* ih, mglGraph *gr)
{
  // Draw axes lines, ticks and ticks labels
  if(ih->data->axisX.axShow)  
    iMglPlotDrawAxis(ih, gr, 'x', ih->data->axisX);
  if(ih->data->axisY.axShow)  
    iMglPlotDrawAxis(ih, gr, 'y', ih->data->axisY);
  if(ih->data->axisZ.axShow && iMglPlotIsView3D(ih))
    iMglPlotDrawAxis(ih, gr, 'z', ih->data->axisZ);

  // Reset to default
  gr->SetFunc(NULL, NULL, NULL);

  if (iupAttribGetBoolean(ih, "COLORBAR"))
  {
    char sch[10] = "";
    sch[0] = '>';  // RIGHT
    char* value = iupAttribGetStr(ih, "COLORBARPOS");
    if (iupStrEqualNoCase(value, "LEFT"))	sch[0] = '<';
    else if (iupStrEqualNoCase(value, "TOP"))	sch[0] = '^';
    else if (iupStrEqualNoCase(value, "BOTTOM"))	sch[0] = '_';
    sch[1] = 0;

    if (sch[0] == '>' || sch[0] == '<')
    {
      iMglPlotConfigColor(ih, gr, ih->data->axisX.axColor);
      iMglPlotConfigFont(ih, gr, ih->data->axisX.axTickFontStyle, ih->data->axisX.axTickFontSizeFactor);
    }
    else
    {
      iMglPlotConfigColor(ih, gr, ih->data->axisY.axColor);
      iMglPlotConfigFont(ih, gr, ih->data->axisY.axTickFontStyle, ih->data->axisY.axTickFontSizeFactor);
    }

    gr->Colorbar(sch);
  }
}

static void iMglPlotDrawGrid(Ihandle* ih, mglGraph *gr)
{
  char pen[10], grid[10];
  iMglPlotConfigColor(ih, gr, ih->data->gridColor);
  iMglPlotConfigPen(pen, ih->data->gridLineStyle, 1);
  iupStrLower(grid, ih->data->gridShow);
  gr->Grid(grid, pen);
}

static void iMglPlotDrawBox(Ihandle* ih, mglGraph *gr)
{
  char style[50] = "";
  iMglPlotConfigColor(ih, gr, ih->data->boxColor);
  gr->Box(style, ih->data->boxTicks);
}

static char* iMglPlotMakeFormatString(double inValue, double range) 
{
  if (inValue<0)
    inValue = range/(-inValue);

  if (inValue > (double)1.0e4 || inValue < (double)1.0e-3)
    return "%.1e";
  else 
  {
    int thePrecision = 0;
    if (inValue<1) 
    {
      double theSpan = inValue;
      while (theSpan<1 && thePrecision<10) 
      {
        thePrecision++;
        theSpan *= 10;
      }
      if (thePrecision<10)
        thePrecision++;
    }

    switch (thePrecision)
    {
    case 1: return "%.1f";
    case 2: return "%.2f";
    case 3: return "%.3f";
    case 4: return "%.4f";
    case 5: return "%.5f";
    case 6: return "%.6f";
    case 7: return "%.7f";
    case 8: return "%.8f";
    case 9: return "%.9f";
    default: return "%.0";
    }
  }
}

static void iMglPlotDrawValues(Ihandle* ih, IdataSet* ds, mglGraph *gr)
{
  int i;
  char text[256];
  char format[256];
  mglPoint p;
  char* xformat = iupAttribGetStr(ih, "AXS_XTICKFORMAT");
  char* yformat = iupAttribGetStr(ih, "AXS_YTICKFORMAT");
  char* zformat = iupAttribGetStr(ih, "AXS_ZTICKFORMAT");
  if (!xformat) xformat = iMglPlotMakeFormatString(ih->data->axisX.axTickMajorSpan, ih->data->axisX.axMax - ih->data->axisX.axMin);
  if (!yformat) yformat = iMglPlotMakeFormatString(ih->data->axisY.axTickMajorSpan, ih->data->axisY.axMax - ih->data->axisY.axMin);
  if (!zformat) zformat = iMglPlotMakeFormatString(ih->data->axisZ.axTickMajorSpan, ih->data->axisZ.axMax - ih->data->axisZ.axMin);

  iMglPlotConfigColor(ih, gr, ds->dsColor);
  iMglPlotConfigFont(ih, gr, ih->data->legendFontStyle, ih->data->legendFontSizeFactor);

  double* dsXPoints = ds->dsX->a;
  if (ds->dsDim == 3)
  {
    double* dsYPoints = ds->dsY->a;
    double* dsZPoints = ds->dsZ->a;
    mglPoint d = mglPoint(ih->data->axisX.axMax-ih->data->axisX.axMin, ih->data->axisY.axMax-ih->data->axisY.axMin, ih->data->axisZ.axMax-ih->data->axisZ.axMin);

    sprintf(format, "(%s, %s, %s)", xformat, yformat, zformat);

    for(i = 0; i < ds->dsCount; i++)
    {
      sprintf(text, format, dsXPoints[i], dsYPoints[i], dsZPoints[i]);
      p = mglPoint(dsXPoints[i], dsYPoints[i], dsZPoints[i]);
      
      gr->Puts(p, d, text, 0, -1);
    }
  }
  else if (ds->dsDim == 2)
  {
    double* dsYPoints = ds->dsY->a;
    mglPoint d = mglPoint(ih->data->axisX.axMax-ih->data->axisX.axMin, ih->data->axisY.axMax-ih->data->axisY.axMin);

    sprintf(format, "(%s, %s)", xformat, yformat);

    for(i = 0; i < ds->dsCount; i++)
    {
      sprintf(text, format, dsXPoints[i], dsYPoints[i]);
      p = mglPoint(dsXPoints[i], dsYPoints[i]);
      
      gr->Puts(p, d, text, 0, -1);
    }
  }
  else
  {
    sprintf(format, "(%%d, %s)", xformat);
    mglPoint d = mglPoint(ih->data->axisX.axMax-ih->data->axisX.axMin, ih->data->axisY.axMax-ih->data->axisY.axMin);

    for(i = 0; i < ds->dsCount; i++)
    {
      sprintf(text, format, i, dsXPoints[i]);
      p = mglPoint((double)i, dsXPoints[i]);
      
      gr->Puts(p, d, text, 0, -1);
    }
  }
}

static void iMglPlotConfigDataGrid(mglGraph *gr, IdataSet* ds, char* style)
{ 
  // Plot affected by SelectPen
  iMglPlotConfigDataSetLineMark(ds, gr, style);

  strcat(style, "#"); // grid
}

static void iMglPlotConfigColorScheme(Ihandle* ih, char* style)
{
  char* value = iupAttribGetStr(ih, "COLORSCHEME");
  if (value)
  {
    strcpy(style, value);
    strcat(style, ":");
  }
}

static void iMglPlotDrawVolumetricData(Ihandle* ih, mglGraph *gr, IdataSet* ds)
{               
  char style[64] = "";
  char* value;

  // All plots here are affected by SetScheme
  iMglPlotConfigColorScheme(ih, style);

  int nx = (int)ds->dsX->nx;
  int ny = (int)ds->dsX->ny;
  int nz = (int)ds->dsX->nz;
  mglData xx(nx), yy(ny), zz(nz);
  xx.Fill(-1.0, +1.0);
  yy.Fill(-1.0, +1.0);
  zz.Fill(-1.0, +1.0);

  if (iupStrEqualNoCase(ds->dsMode, "VOLUME_ISOSURFACE"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))   // Default false
      iMglPlotConfigDataGrid(gr, ds, style);  // Here means wire

    value = iupAttribGet(ih, "ISOVALUE");
    if (value)
    {
      double isovalue;
      if (iupStrToDouble(value, &isovalue))
        gr->Surf3(isovalue, xx, yy, zz, *ds->dsX, style);   // only 1 isosurface
    }
    else
    {
      int isocount = iupAttribGetInt(ih, "ISOCOUNT");  //Default 3
      char opt[100];
      sprintf(opt, "value %d;", isocount);
      gr->Surf3(xx, yy, zz, *ds->dsX, style, opt); // plots N isosurfaces, from Cmin to Cmax
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_DENSITY"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))  // Default false
      iMglPlotConfigDataGrid(gr, ds, style);  // Here means wire

    char* slicedir = iupAttribGetStr(ih, "SLICEDIR"); //Default "XYZ"
    int project = iupAttribGetBoolean(ih, "PROJECT");  //Default false
    if (project)
    {
      double valx = iMglPlotGetAttribDoubleNAN(ih, "PROJECTVALUEX");
      double valy = iMglPlotGetAttribDoubleNAN(ih, "PROJECTVALUEY");
      double valz = iMglPlotGetAttribDoubleNAN(ih, "PROJECTVALUEZ");
      if (tolower(*slicedir)=='x') { gr->DensX(ds->dsX->Sum("x"), style, valx); slicedir++; }
      if (tolower(*slicedir)=='y') { gr->DensY(ds->dsX->Sum("y"), style, valy); slicedir++; }
      if (tolower(*slicedir)=='z') { gr->DensZ(ds->dsX->Sum("z"), style, valz); slicedir++; }
    }
    else
    {
      double slicex = iupAttribGetDouble(ih, "SLICEX");  //Default -1 (central)
      double slicey = iupAttribGetDouble(ih, "SLICEY");  //Default -1 (central)
      double slicez = iupAttribGetDouble(ih, "SLICEZ");  //Default -1 (central)
      if (tolower(*slicedir) == 'x') { strcat(style, "x");  gr->Dens3(xx, yy, zz, *ds->dsX, style, slicex); slicedir++; }
      if (tolower(*slicedir) == 'y') { strcat(style, "y");  gr->Dens3(xx, yy, zz, *ds->dsX, style, slicey); slicedir++; }
      if (tolower(*slicedir) == 'z') { strcat(style, "z");  gr->Dens3(xx, yy, zz, *ds->dsX, style, slicez); slicedir++; }
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_CONTOUR"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))
      iMglPlotConfigDataGrid(gr, ds, style);  // Here means wire

    int contourcount = iupAttribGetInt(ih, "CONTOURCOUNT");  //Default 7, plots N countours, from Cmin to Cmax
    char opt[100];
    sprintf(opt, "value %d;", contourcount);

    int countourfilled = iupAttribGetBoolean(ih, "CONTOURFILLED");  //Default false
    char* slicedir = iupAttribGetStr(ih, "SLICEDIR"); //Default "XYZ"
    int project = iupAttribGetBoolean(ih, "PROJECT");  //Default false
    if (project)
    {
      double valx = iMglPlotGetAttribDoubleNAN(ih, "PROJECTVALUEX");
      double valy = iMglPlotGetAttribDoubleNAN(ih, "PROJECTVALUEY");
      double valz = iMglPlotGetAttribDoubleNAN(ih, "PROJECTVALUEZ");
      if (countourfilled)
      {
        if (tolower(*slicedir) == 'x') { gr->ContFX(ds->dsX->Sum("x"), style, valx, opt); slicedir++; }
        if (tolower(*slicedir) == 'y') { gr->ContFY(ds->dsX->Sum("y"), style, valy, opt); slicedir++; }
        if (tolower(*slicedir) == 'z') { gr->ContFZ(ds->dsX->Sum("z"), style, valz, opt); slicedir++; }
      }
      else
      {
        if (tolower(*slicedir) == 'x') { gr->ContX(ds->dsX->Sum("x"), style, valx, opt); slicedir++; }
        if (tolower(*slicedir) == 'y') { gr->ContY(ds->dsX->Sum("y"), style, valy, opt); slicedir++; }
        if (tolower(*slicedir) == 'z') { gr->ContZ(ds->dsX->Sum("z"), style, valz, opt); slicedir++; }
      }
    }
    else
    {
      double slicex = iupAttribGetDouble(ih, "SLICEX");  //Default -1 (central)
      double slicey = iupAttribGetDouble(ih, "SLICEY");  //Default -1 (central)
      double slicez = iupAttribGetDouble(ih, "SLICEZ");  //Default -1 (central)

      if (countourfilled)
      {
        if (tolower(*slicedir) == 'x') { strcat(style, "x");  gr->ContF3(xx, yy, zz, *ds->dsX, style, slicex, opt); slicedir++; }
        if (tolower(*slicedir) == 'y') { strcat(style, "y");  gr->ContF3(xx, yy, zz, *ds->dsX, style, slicey, opt); slicedir++; }
        if (tolower(*slicedir) == 'z') { strcat(style, "z");  gr->ContF3(xx, yy, zz, *ds->dsX, style, slicez, opt); slicedir++; }
      }
      else
      {
        // Plot affected by SelectPen
        iMglPlotConfigDataSetLineMark(ds, gr, style);

        if (tolower(*slicedir) == 'x') { strcat(style, "x");  gr->Cont3(xx, yy, zz, *ds->dsX, style, slicex, opt); slicedir++; }
        if (tolower(*slicedir) == 'y') { strcat(style, "y");  gr->Cont3(xx, yy, zz, *ds->dsX, style, slicey, opt); slicedir++; }
        if (tolower(*slicedir) == 'z') { strcat(style, "z");  gr->Cont3(xx, yy, zz, *ds->dsX, style, slicez, opt); slicedir++; }
      }
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_CLOUD"))
  {
    int low = iupAttribGetBoolean(ih, "CLOUDLOW");  //Default false
    if (low)
      strcat(style, ".");

    gr->Cloud(xx, yy, zz, *ds->dsX, style, "-1");   // Use AlphaDef
  }
}

static void iMglPlotDrawPlanarData(Ihandle* ih, mglGraph *gr, IdataSet* ds)
{               
  char style[64] = "";

  // All plots here are affected by SetScheme
  iMglPlotConfigColorScheme(ih, style);

  int nx = (int)ds->dsX->nx;
  int ny = (int)ds->dsX->ny;
  mglData xx(nx), yy(ny);
  xx.Fill(-1.0, +1.0);
  yy.Fill(-1.0, +1.0);

  if (iupStrEqualNoCase(ds->dsMode, "PLANAR_MESH"))
  {
    // Plot affected by SelectPen
    iMglPlotConfigDataSetLineMark(ds, gr, style);

    gr->Mesh(xx, yy, *ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_FALL"))
  {
    // Plot affected by SelectPen
    iMglPlotConfigDataSetLineMark(ds, gr, style);

    char* falldir = iupAttribGetStr(ih, "DIR"); //Default "Y"
    if (tolower(*falldir) == 'x')
      { style[0] = 'x'; style[1] = 0; }

    gr->Fall(xx, yy, *ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_BELT"))
  {
    char* beltdir = iupAttribGetStr(ih, "DIR"); //Default "Y"
    if (tolower(*beltdir) == 'x')
      { style[0] = 'x'; style[1] = 0; }

    gr->Belt(xx, yy, *ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_SURFACE"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))   // Default false
      iMglPlotConfigDataGrid(gr, ds, style);

    gr->Surf(xx, yy, *ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_BOXES"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))   // Default false
      iMglPlotConfigDataGrid(gr, ds, style);  // Here means box lines

    gr->Boxs(xx, yy, *ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_TILE"))
  {
    gr->Tile(xx, yy, *ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_DENSITY"))
  {
    gr->Dens(xx, yy, *ds->dsX, style, "");
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_CONTOUR"))
  {
    int contourcount = iupAttribGetInt(ih, "CONTOURCOUNT");  //Default 7, plots N countours, from Cmin to Cmax
    int countourfilled = iupAttribGetBoolean(ih, "CONTOURFILLED");  //Default false

    char opt[100];
    sprintf(opt, "value %d;", contourcount);

    if (countourfilled)
      gr->ContF(xx, yy, *ds->dsX, style, opt);
    else
    {
      // Plot affected by SelectPen
      iMglPlotConfigDataSetLineMark(ds, gr, style);

      char* countourlabels = iupAttribGetStr(ih, "CONTOURLABELS");  //Default NULL
      if (iupStrEqualNoCase(countourlabels, "BELLOW"))
        strcat(style, "t");
      else if (iupStrEqualNoCase(countourlabels, "ABOVE"))
        strcat(style, "T");

      gr->Cont(xx, yy, *ds->dsX, style, opt);
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_AXIALCONTOUR"))
  {
    int axialcount = iupAttribGetInt(ih, "AXIALCOUNT");  //Default 3, plots N countours, from Cmin to Cmax
    char opt[100];
    sprintf(opt, "value %d;", axialcount);
    gr->Axial(xx, yy, *ds->dsX, style, opt);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_GRADIENTLINES"))
  {
    int gradlinescount = iupAttribGetInt(ih, "GRADLINESCOUNT");  //Default 5
    char opt[100];
    sprintf(opt, "value %d;", gradlinescount);
    gr->Grad(xx, yy, *ds->dsX, style, opt);
  }
}

static void iMglPlotDrawLinearData(Ihandle* ih, mglGraph *gr, IdataSet* ds)
{               
  char style[64] = "";

  iMglPlotConfigColor(ih, gr, ds->dsColor);

  int nx = (int)ds->dsX->nx;
  mglData xx(nx);
  xx.Fill(0,(double)nx-1);

  // All plots here are affected by SelectPen
  // All plots here are affected by SetPalette

  // TODO Workaround, we changed SetPal for this to work
  //gr->SetPalNum(1);
  //gr->SetPalColor(0, ds->dsColor.r, ds->dsColor.g, ds->dsColor.b); // No problem using it here because 
                                                                   // dsColor should never has NAN values

  if (iupStrEqualNoCase(ds->dsMode, "LINE") ||
      iupStrEqualNoCase(ds->dsMode, "MARKLINE") ||
      iupStrEqualNoCase(ds->dsMode, "MARK"))
  {
    if (!iupStrEqualNoCase(ds->dsMode, "LINE"))  // same as setting marks=yes
      ds->dsShowMarks = true;
    if (iupStrEqualNoCase(ds->dsMode, "MARK"))   // Same as setting linestyle=none
      ds->dsLineStyle = ' '; // No line

    // Plot affected by SelectPen
    iMglPlotConfigDataSetLineMark(ds, gr, style);

    if (ds->dsDim == 3)
      gr->Plot(*ds->dsX, *ds->dsY, *ds->dsZ, style);
    else if (ds->dsDim == 2)
      gr->Plot(*ds->dsX, *ds->dsY, style, 0);
    else
      gr->Plot(xx, *ds->dsX, style, 0);  // At Z=0
  }
  else if (iupStrEqualNoCase(ds->dsMode, "RADAR"))
  {
    // Plot affected by SelectPen
    iMglPlotConfigDataSetLineMark(ds, gr, style);

    double radarshift = iupAttribGetDouble(ih, "RADARSHIFT");   // Default -1
    char* opt = iupStrReturnDouble(radarshift);

    if (iupAttribGetBoolean(ih, "DATAGRID"))  //Default false
      iMglPlotConfigDataGrid(gr, ds, style);

    gr->Radar(*ds->dsX, style, opt);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "AREA"))
  {
    // NOT affected by SelectPen

    if (ds->dsDim == 3)
      gr->Area(*ds->dsX, *ds->dsY, *ds->dsZ, style);
    else if (ds->dsDim == 2)
      gr->Area(*ds->dsX, *ds->dsY, style, 0);
    else
      gr->Area(xx, *ds->dsX, style, 0);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "BAR"))
  {
    // NOT affected by SelectPen
    double barwidth = iupAttribGetDouble(ih, "BARWIDTH");   // Default 0.7
    gr->SetBarWidth(barwidth);

    if (iupAttribGetBoolean(ih, "DATAGRID"))  //Default false
      iMglPlotConfigDataGrid(gr, ds, style);

    // Each bar could has a different color using a scheme
    // But we can NOT use scheme here because we changed SetPal
    //iMglPlotConfigColorScheme(ih, style);

    // To avoid hole bars clipped when touching the bounding box
    gr->SetCut(false); 

    if (ds->dsDim == 3)
      gr->Bars(*ds->dsX, *ds->dsY, *ds->dsZ, style);
    else if (ds->dsDim == 2)
      gr->Bars(*ds->dsX, *ds->dsY, style, 0);
    else
      gr->Bars(xx, *ds->dsX, style, 0);

    gr->SetCut(true); 
  }
  else if (iupStrEqualNoCase(ds->dsMode, "BARHORIZONTAL"))
  {
    // NOT affected by SelectPen
    double barwidth = iupAttribGetDouble(ih, "BARWIDTH");   // Default 0.7
    gr->SetBarWidth(barwidth);

    // Each bar could has a different color using a scheme
    // But we can NOT use scheme here because we changed SetPal
    //iMglPlotConfigColorScheme(ih, style);

    if (iupAttribGetBoolean(ih, "DATAGRID"))  //Default false
      iMglPlotConfigDataGrid(gr, ds, style);

    // To avoid hole bars clipped when touching the bounding box
    gr->SetCut(false); 

    if (ds->dsDim == 2)
      gr->Barh(*ds->dsY, *ds->dsX, style, 0);
    else
      gr->Barh(xx, *ds->dsX, style, 0);

    gr->SetCut(true); 
  }
  else if (iupStrEqualNoCase(ds->dsMode, "STEP"))
  {
    // Plot affected by SelectPen
    iMglPlotConfigDataSetLineMark(ds, gr, style);

    if (ds->dsDim == 3)
      gr->Step(*ds->dsX, *ds->dsY, *ds->dsZ, style);
    else if (ds->dsDim == 2)
      gr->Step(*ds->dsX, *ds->dsY, style, 0);
    else
      gr->Step(xx, *ds->dsX, style, 0);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "STEM"))
  {
    // Plot affected by SelectPen
    iMglPlotConfigDataSetLineMark(ds, gr, style);

    if (ds->dsDim == 3)
      gr->Stem(*ds->dsX, *ds->dsY, *ds->dsZ, style);
    else if (ds->dsDim == 2)
      gr->Stem(*ds->dsX, *ds->dsY, style, 0);
    else
      gr->Stem(xx, *ds->dsX, style, 0);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "CHART"))
  {
    // NOT affected by SelectPen

    // Chart does not use SetPalette,
    // so we use the scheme attribute as a list of colors.
    iMglPlotConfigColorScheme(ih, style);

    if (iupAttribGetBoolean(ih, "DATAGRID"))  //Default false
      iMglPlotConfigDataGrid(gr, ds, style);

    int piechart = iupAttribGetBoolean(ih, "PIECHART");  //Default false
    if (piechart)
    {
      //TODO: this equation works only if x in the interval [-1, 1]
      gr->SetFunc("(y+1)/2*cos(pi*x)", "(y+1)/2*sin(pi*x)");

      // This box has to be here, so it will be affected by the transformation
      if (ih->data->Box)
        iMglPlotDrawBox(ih, gr);

      gr->Chart(*ds->dsX, style);
      gr->SetFunc(NULL, NULL);
    }
    else
      gr->Chart(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "CRUST"))
  {
    if (ds->dsDim == 3)
    {
      // NOT affected by SelectPen
      // NOT affected by SetPalette

      // Affected by SetScheme
      iMglPlotConfigColorScheme(ih, style);

      if (iupAttribGetBoolean(ih, "DATAGRID"))  //Default false
        iMglPlotConfigDataGrid(gr, ds, style);

      gr->Crust(*ds->dsX, *ds->dsY, *ds->dsZ, style, "");
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "DOTS"))
  {
    if (ds->dsDim == 3)
    {
      // NOT affected by SelectPen
      // NOT affected by SetPalette

      // Affected by SetScheme
      iMglPlotConfigColorScheme(ih, style);

      gr->Dots(*ds->dsX, *ds->dsY, *ds->dsZ, style);
    }
  }
}

static void iMglPlotDrawData(Ihandle* ih, mglGraph *gr)
{
  int i;
  for(i = 0; i < ih->data->dataSetCount; i++)
  {
    IdataSet* ds = &ih->data->dataSet[i];

    if (iMglPlotIsVolumetricData(ds))
      iMglPlotDrawVolumetricData(ih, gr, ds);
    else if (iMglPlotIsPlanarData(ds))
      iMglPlotDrawPlanarData(ih, gr, ds);
    else
      iMglPlotDrawLinearData(ih, gr, ds);

    if(ds->dsShowValues && !iMglPlotIsPlanarOrVolumetricData(ds))
      iMglPlotDrawValues(ih, ds, gr);  /* Print values near the samples */
  }
}

static void iMglPlotDrawLegend(Ihandle* ih, mglGraph *gr)
{
  int i;
  char style[64] = "";

  gr->ClearLegend();

  for(i = 0; i < ih->data->dataSetCount; i++)
  {
    IdataSet* ds = &ih->data->dataSet[i];

    style[0] = 0;
    iMglPlotConfigDataSetLineMark(ds, gr, style);
    iMglPlotConfigDataSetColor(ds, style);
    gr->AddLegend(ds->dsLegend, style);
  }

  style[0] = 0;
  if (ih->data->legendBox)
  {
    style[0] = '#';
    style[1] = 0;
  }

  // Draw legend of accumulated strings
  iMglPlotConfigColor(ih, gr, ih->data->legendColor);
  iMglPlotConfigFont(ih, gr, ih->data->legendFontStyle, ih->data->legendFontSizeFactor);
  gr->Legend(ih->data->legendPosition, style, "");
}

static void iMglPlotDrawTitle(Ihandle* ih, mglGraph *gr, const char* title)
{
  iMglPlotConfigColor(ih, gr, ih->data->titleColor);
  iMglPlotConfigFont(ih, gr, ih->data->titleFontStyle, ih->data->titleFontSizeFactor);

  gr->Title(title, NULL, -1);
}

static void iMglPlotConfigPlotArea(Ihandle* ih, mglGraph *gr)
{
  // The default adds spaces at left, top, right and bottom
  double x1 = 0, x2 = 1.0, y1 = 0, y2 = 1.0;

  // So we remove the spaces if not used

  // Left
  if (!mgl_isnan(ih->data->axisY.axOrigin))
    x1 = -0.16f;  // Axis is crossed
  else if (!iupAttribGetStr(ih, "AXS_YLABEL"))  // no label
  {
    if (!ih->data->axisY.axTickShowValues)
      x1 = -0.20f;  // not crossed, no label, no ticks values
    else if (ih->data->axisY.axTickValuesRotation)
      x1 = -0.15f;  // not crossed, no label, with vertical ticks values
    else
      x1 = -0.10f;  // not crossed, no label, with horizontal ticks values
  }
  else if (ih->data->axisY.axLabelRotation)
    x1 = -0.10f; // not crossed and with vertical label
  else
    x1 = -0.01f; // not crossed and with horizontal label

  // Right
  if (iMglPlotHasx10(ih->data->axisX.axMin, ih->data->axisX.axMax))  // X Axis Min-Max
    x2 = 1.05f;  // with x10 notation
  else
    x2 = 1.15f;  // no notation

  // Bottom
  if (!mgl_isnan(ih->data->axisX.axOrigin))
    y1 = -0.20f;  // Axis is crossed
  else if (!iupAttribGetStr(ih, "AXS_XLABEL"))
  {
    if (!ih->data->axisX.axTickShowValues)
      y1 = -0.20f;  // not crossed, no label, no ticks values
    else
      y1 = -0.15f;  // not crossed, no label, with ticks values
  }
  else
    y1 = -0.10f;  // not crossed and with label

  // Top
  if (!iupAttribGetStr(ih, "TITLE"))
    y2 = 1.15f;  // no title
  else
    y2 = 1.05f;  // with title

  // Colorbar
  if (iupAttribGetBoolean(ih, "COLORBAR"))
  {
    char* value = iupAttribGetStr(ih, "COLORBARPOS");
    if (iupStrEqualNoCase(value, "LEFT"))
      x1 = 0.05f;  // includes also space for axis
    else if (iupStrEqualNoCase(value, "TOP"))
      y2 = 0.9f;   // includes also space for title
    else if (iupStrEqualNoCase(value, "BOTTOM"))
      y1 = 0.05f;  // includes also space for axis
    else // RIGHT
      x2 = 0.9f;   // includes also space for notation
  }

  // Only one plot using all viewport
  gr->InPlot(x1, x2, y1, y2, false);
}

static void iMglPlotConfigView(Ihandle* ih, mglGraph *gr)
{
  iMglPlotConfigPlotArea(ih, gr);

  // Transparency
  gr->Alpha(true);  // Necessary so Anti-alias can work.
  if (ih->data->transparent)
    gr->SetAlphaDef(ih->data->alpha);  // Should be used only if transparent is enabled
  else
    gr->SetAlphaDef(1.0);

  // Zoom and Pan
  gr->Zoom(ih->data->x1, ih->data->y1, ih->data->x2, ih->data->y2);

  if (iMglPlotIsView3D(ih))
  {
    if (iupAttribGetBoolean(ih, "LIGHT"))  /* Default: FALSE */
      gr->Light(true);
    else
      gr->Light(false);

    // Notice that rotZ is the second parameter
    gr->Rotate(ih->data->rotX, ih->data->rotZ, ih->data->rotY);  

    if (ih->data->rotX != 0 || ih->data->rotY != 0 || ih->data->rotZ)
    {
      // Add more space when rotating a 3D graph
      //TODO
      //((mglCanvas*)gr)->GetPlotFactor()
      //gr->SetPlotFactor(pf+0.3);  
    }
  }
}

static void iMglPlotDrawPlot(Ihandle* ih, mglGraph *gr)
{
  // Since this function will be used to draw on screen and
  // on metafile and bitmaps, all mglGraph control must be done here
  // and can NOT be done inside the attribute methods

  gr->SetWarn(0, "");

  gr->DefaultPlotParam();

  /* Clear */
  gr->Clf(ih->data->bgColor.r, ih->data->bgColor.g, ih->data->bgColor.b);

  iMglPlotConfigView(ih, gr);

  iMglPlotConfigFontDef(ih, gr);

  iMglPlotConfigAxesRange(ih, gr);

  iupAttribSet(ih, "_IUP_MGLPLOT_GRAPH", (char*)gr);

  IFn cb = IupGetCallback(ih, "PREDRAW_CB");
  if (cb)
    cb(ih);

  iMglPlotDrawAxes(ih, gr);

  if(ih->data->gridShow)
    iMglPlotDrawGrid(ih, gr);

  int piechart = iupAttribGetBoolean(ih, "PIECHART");  //Default false
  if (ih->data->Box && !piechart)
    iMglPlotDrawBox(ih, gr);

  iMglPlotDrawData(ih, gr);

  if(ih->data->legendShow)
    iMglPlotDrawLegend(ih, gr);

  char* value = iupAttribGetStr(ih, "TITLE");
  if (value)
    iMglPlotDrawTitle(ih, gr, value);

  cb = IupGetCallback(ih, "POSTDRAW_CB");
  if (cb)
    cb(ih);

  iupAttribSet(ih, "_IUP_MGLPLOT_GRAPH", NULL);
}

static void iMglPlotRepaint(Ihandle* ih, int force, int flush)
{
  if (!IupGLIsCurrent(ih))
    force = 1;

  IupGLMakeCurrent(ih);

  mglGraph* gr = ih->data->mgl;

  if (force || ih->data->redraw)
  {
    /* update render */
    iMglPlotDrawPlot(ih, gr);  /* Draw the graphics plot */

    gr->Finish();
    ih->data->redraw = false;
  }

  if (flush)
  {
    if (!ih->data->opengl)
    {
      const unsigned char *rgb = gr->GetRGB();
	    if (rgb)
        glDrawPixels(ih->data->w, ih->data->h, GL_RGB, GL_UNSIGNED_BYTE, rgb);
    }

    IupGLSwapBuffers(ih);
  }
}


/******************************************************************************
 Attribute Methods
******************************************************************************/

static int iMglPlotSetColor(Ihandle* ih, const char* value, mglColor& color)
{
  unsigned char rr, gg, bb;

  if (!value)
  {
    color.Set(NAN, NAN, NAN);
    ih->data->redraw = true;
  }
  else if (iupStrToRGB(value, &rr, &gg, &bb))
  {
    color.Set((float)iRecon(rr), (float)iRecon(gg), (float)iRecon(bb));
    ih->data->redraw = true;
  }

  return 0;
}

static char* iMglPlotReturnColor(const mglColor& color)
{
  return iupStrReturnRGB(iQuant(color.r), iQuant(color.g), iQuant(color.b));
}

static int iMglPlotSetBoolean(Ihandle* ih, const char* value, bool& num)
{
  bool old_num = num;

  if (iupStrBoolean(value))
    num = true;
  else 
    num = false;

  if (old_num != num)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetInt(Ihandle* ih, const char* value, int& num)
{
  int old_num = num;

  iupStrToInt(value, &num);

  if (old_num != num)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetDouble(Ihandle* ih, const char* value, double& num)
{
  double old_num = num;

  iupStrToDouble(value, &num);

  if (old_num != num)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetResetAttrib(Ihandle* ih, const char* value)
{
  (void)value;  /* not used */
  iMglPlotReset(ih);

  // Some attributes are stored in the hash table
  iupAttribSet(ih, "TITLE", NULL);

  iupAttribSet(ih, "DATAGRID", NULL);
  iupAttribSet(ih, "GRADLINESCOUNT", NULL);
  iupAttribSet(ih, "AXIALCOUNT", NULL);
  iupAttribSet(ih, "CONTOURFILLED", NULL);
  iupAttribSet(ih, "CONTOURLABELS", NULL);
  iupAttribSet(ih, "CONTOURCOUNT", NULL);
  iupAttribSet(ih, "DIR", NULL);
  iupAttribSet(ih, "CLOUDLOW", NULL);
  iupAttribSet(ih, "SLICEX", NULL);
  iupAttribSet(ih, "SLICEY", NULL);
  iupAttribSet(ih, "SLICEZ", NULL);
  iupAttribSet(ih, "SLICEDIR", NULL);
  iupAttribSet(ih, "PROJECTVALUEX", NULL);
  iupAttribSet(ih, "PROJECTVALUEY", NULL);
  iupAttribSet(ih, "PROJECTVALUEZ", NULL);
  iupAttribSet(ih, "PROJECT", NULL);
  iupAttribSet(ih, "ISOCOUNT", NULL);
  iupAttribSet(ih, "BARWIDTH", NULL);
  iupAttribSet(ih, "RADARSHIFT", NULL);
  iupAttribSet(ih, "PIECHART", NULL);
  iupAttribSet(ih, "CRUSTRADIUS", NULL);

  iupAttribSet(ih, "LIGHT", NULL);
  iupAttribSet(ih, "COLORSCHEME", NULL);

  iupAttribSet(ih, "COLORBAR", NULL);
  iupAttribSet(ih, "COLORBARPOS", NULL);
  iupAttribSet(ih, "COLORBARRANGE", NULL);
  iupAttribSet(ih, "COLORBARAXISTICKS", NULL);

  iupAttribSet(ih, "AXS_XLABEL", NULL);
  iupAttribSet(ih, "AXS_YLABEL", NULL);
  iupAttribSet(ih, "AXS_ZLABEL", NULL);
  iupAttribSet(ih, "AXS_XTICKFORMAT", NULL);
  iupAttribSet(ih, "AXS_YTICKFORMAT", NULL);
  iupAttribSet(ih, "AXS_ZTICKFORMAT", NULL);

  ih->data->redraw = true;
  return 0;
}

static int iMglPlotSetRedrawAttrib(Ihandle* ih, const char* value)
{
  (void)value;  /* not used */
  iMglPlotRepaint(ih, 1, 1);    /* force a full redraw here */
  return 0;
}

static int iMglPlotSetBGColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->bgColor);
}

static char* iMglPlotGetBGColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->bgColor);
}

static int iMglPlotSetFGColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->fgColor);
}

static char* iMglPlotGetFGColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->fgColor);
}

static int iMglPlotSetTitleFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->titleFontSizeFactor);
}

static char* iMglPlotGetTitleFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->titleFontSizeFactor);
}

static int iMglPlotSetFontStyle(Ihandle* ih, const char* value, int& fontstyle)
{
  int old_fontstyle = fontstyle;

  if (!value)
    fontstyle = IUP_MGLPLOT_INHERIT;
  else if (iupStrEqualNoCase(value, "PLAIN"))
    fontstyle = IUP_MGLPLOT_PLAIN;
  else if (iupStrEqualNoCase(value, "BOLD"))
    fontstyle = IUP_MGLPLOT_BOLD;
  else if (iupStrEqualNoCase(value, "ITALIC"))
    fontstyle = IUP_MGLPLOT_ITALIC;
  else if (iupStrEqualNoCase(value, "BOLDITALIC"))
    fontstyle = IUP_MGLPLOT_BOLD_ITALIC;

  if (old_fontstyle != fontstyle)
    ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetFontStyle(int fontstyle)
{
  if (fontstyle != IUP_MGLPLOT_INHERIT)
    return iupStrReturnInt(fontstyle);
  else
    return NULL;
}

static int iMglPlotSetTitleFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->titleFontStyle);
}

static char* iMglPlotGetTitleFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->titleFontStyle);
}

static int iMglPlotSetTitleColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->titleColor);
}

static char* iMglPlotGetTitleColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->titleColor);
}

static int iMglPlotSetLegendFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->legendFontSizeFactor);
}

static char* iMglPlotGetLegendFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->legendFontStyle);
}

static char* iMglPlotGetLegendFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->legendFontSizeFactor);
}

static int iMglPlotSetLegendFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->legendFontStyle);
}

static int iMglPlotSetLegendShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->legendShow);
}

static char* iMglPlotGetLegendShowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->legendShow);
}

static int iMglPlotSetLegendBoxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->legendBox);
}

static char* iMglPlotGetLegendBoxAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->legendBox);
}

static int iMglPlotSetLegendPosAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "TOPLEFT"))
    ih->data->legendPosition = IUP_MGLPLOT_TOPLEFT;
  else if (iupStrEqualNoCase(value, "BOTTOMLEFT"))
    ih->data->legendPosition = IUP_MGLPLOT_BOTTOMLEFT;
  else if (iupStrEqualNoCase(value, "BOTTOMRIGHT"))
    ih->data->legendPosition = IUP_MGLPLOT_BOTTOMRIGHT;
  else
    ih->data->legendPosition = IUP_MGLPLOT_TOPRIGHT;

  ih->data->redraw = true;
  return 0;
}

static char* iMglPlotGetLegendPosAttrib(Ihandle* ih)
{
  char* legendpos_str[4] = {"BOTTOMLEFT", "BOTTOMRIGHT", "TOPLEFT", "TOPRIGHT"};
  return legendpos_str[ih->data->legendPosition];
}

static int iMglPlotSetLegendColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->legendColor);
}

static char* iMglPlotGetLegendColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->legendColor);
}

static int iMglPlotSetGridColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->gridColor);
}

static char* iMglPlotGetGridColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->gridColor);
}

static int iMglPlotSetBoxColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->boxColor);
}

static char* iMglPlotGetBoxColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->boxColor);
}

static int iMglPlotSetLineStyle(Ihandle* ih, const char* value, char &linestyle)
{
  char old_linestyle = linestyle;

  if (!value || iupStrEqualNoCase(value, "CONTINUOUS") || iupStrEqualNoCase(value, "SOLID"))
    linestyle = '-';
  else if (iupStrEqualNoCase(value, "LONGDASHED"))
    linestyle = '|';
  else if (iupStrEqualNoCase(value, "DASHED"))
    linestyle = ';';
  else if (iupStrEqualNoCase(value, "SMALLDASHED"))
    linestyle = '=';
  else if (iupStrEqualNoCase(value, "DOTTED"))
    linestyle = ':';
  else if (iupStrEqualNoCase(value, "DASH_DOT"))
    linestyle = 'j';
  else if (iupStrEqualNoCase(value, "SMALLDASH_DOT") || 
           iupStrEqualNoCase(value, "DASH_DOT_DOT")) // for IUP compatibility
    linestyle = 'i';
  else if (iupStrEqualNoCase(value, "NONE"))
    linestyle = ' ';
  else
    linestyle = '-';  /* reset to default */

  if (old_linestyle != linestyle)
    ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetLineStyle(char linestyle)
{
  switch(linestyle)
  {
    case ' ': return "NONE";
    case '|': return "LONGDASHED";
    case ';': return "DASHED";
    case '=': return "SMALLDASHED";
    case ':': return "DOTTED";
    case 'j': return "DASH_DOT";
    case 'i': return "SMALLDASH_DOT";
    default:  return "CONTINUOUS";
  }
}

static int iMglPlotSetGridLineStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetLineStyle(ih, value, ih->data->gridLineStyle);
}

static char* iMglPlotGetGridLineStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetLineStyle(ih->data->gridLineStyle);
}

static int iMglPlotSetGridAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "VERTICAL") || iupStrEqualNoCase(value, "X"))
    ih->data->gridShow = "X";
  else if (iupStrEqualNoCase(value, "HORIZONTAL") || iupStrEqualNoCase(value, "Y"))
    ih->data->gridShow = "Y";
  else if (iupStrEqualNoCase(value, "Z"))
    ih->data->gridShow = "Z";
  else if (iupStrEqualNoCase(value, "XY"))
    ih->data->gridShow = "XY";
  else if (iupStrEqualNoCase(value, "XZ"))
    ih->data->gridShow = "XZ";
  else if (iupStrEqualNoCase(value, "YZ"))
    ih->data->gridShow = "YZ";
  else if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "XYZ"))
    ih->data->gridShow = "XYZ";
  else                                
    ih->data->gridShow = NULL;

  ih->data->redraw = true;
  return 0;
}

static char* iMglPlotGetGridAttrib(Ihandle* ih)
{
  if (ih->data->gridShow)
    return (char*)ih->data->gridShow;
  else
    return "NO";
}

static char* iMglPlotGetCountAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->dataSetCount);
}

static int iMglPlotSetCurrentAttrib(Ihandle* ih, const char* value)
{
  int ii;

  ih->data->dataSetCurrent = -1;

  if (iupStrToInt(value, &ii))
  {
    if (ii>=0 && ii<ih->data->dataSetCount)
    {
      ih->data->dataSetCurrent = ii;
      ih->data->redraw = true;
    }
  }

  return 0;
}

static char* iMglPlotGetCurrentAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->dataSetCurrent);
}

static void iMglPlotRemoveDataSet(IdataSet* ds)
{
  free(ds->dsLegend);
  free(ds->dsMode);

  if (ds->dsNames)
  {
    int j;
    char** dsNames = (char**)iupArrayGetData(ds->dsNames);
    int count = iupArrayCount(ds->dsNames);
    for (j=0; j<count; j++)
      free(dsNames[j]);

    iupArrayDestroy(ds->dsNames);
  }

  if (ds->dsX)
    delete ds->dsX;
  if (ds->dsY)
    delete ds->dsY;
  if (ds->dsZ)
    delete ds->dsZ;

  memset(ds, 0, sizeof(IdataSet));
}

static int iMglPlotSetRemoveAttrib(Ihandle* ih, const char* value)
{
  int ii;
  if (iupStrToInt(value, &ii))
  {
    int i;

    if(ii > ih->data->dataSetCount || ii < 0)
      return 0;

    iMglPlotRemoveDataSet(&ih->data->dataSet[ii]);

    for(i = ii; i < ih->data->dataSetCount-1; i++)
      ih->data->dataSet[i] = ih->data->dataSet[i+1];

    memset(&ih->data->dataSet[ih->data->dataSetCount-1], 0, sizeof(IdataSet));

    ih->data->dataSetCount--;

    if (ih->data->dataSetCurrent > ih->data->dataSetCount-1)
      ih->data->dataSetCurrent = ih->data->dataSetCount-1;

    ih->data->redraw = true;
  }
  return 0;
}

static int iMglPlotSetClearAttrib(Ihandle* ih, const char* value)
{
  int i;
  for(i = 0; i < ih->data->dataSetCount; i++)
    iMglPlotRemoveDataSet(&ih->data->dataSet[i]);
  ih->data->dataSetCount = 0;
  ih->data->dataSetCurrent = -1;
  ih->data->redraw = true;
  (void)value;
  return 0;
}

static int iMglPlotSetDSRearrangeAttrib(Ihandle* ih, const char* value)
{
  int i;
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];

  // Must be Planar data
  if (ds->dsY || ds->dsZ || ds->dsX->nz != 1)
    return 0;
  if (ds->dsX->ny == 1)
    return 0;

  // Now nx!=1 and ny!=1, so rearrange data from ds->dsX to ds->dsY
  if (ds->dsX->ny == 2 || ds->dsX->ny == 3)
  {
    ds->dsCount = ds->dsX->nx;
    ds->dsDim = 2;
    ds->dsY = new mglData(ds->dsCount);
    mglData* dsX = new mglData(ds->dsCount);
    double* ax = ds->dsX->a;
    double* ay = ax + ds->dsCount;
    double* az = NULL;
    double* x = dsX->a;
    double* y = ds->dsY->a;
    double* z = NULL;
    if (ds->dsX->ny == 3)
    {
      ds->dsZ = new mglData(ds->dsCount);
      z = ds->dsZ->a;
      az = ay + ds->dsCount;
      ds->dsDim = 3;
    }

    memcpy(x, ax, ds->dsCount*sizeof(double));
    memcpy(y, ay, ds->dsCount*sizeof(double));
    if (z) memcpy(z, az, ds->dsCount*sizeof(double));

    delete ds->dsX;
    ds->dsX = dsX;
  }
  else if (ds->dsX->nx == 2 || ds->dsX->nx == 3)
  {
    ds->dsCount = ds->dsX->ny;
    ds->dsDim = 2;
    ds->dsY = new mglData(ds->dsCount);
    mglData* dsX = new mglData(ds->dsCount);
    double* a = ds->dsX->a;
    double* x = dsX->a;
    double* y = ds->dsY->a;
    double* z = NULL;
    if (ds->dsX->nx == 3)
    {
      ds->dsZ = new mglData(ds->dsCount);
      z = ds->dsZ->a;
      ds->dsDim = 3;
    }

    for (i=0; i<ds->dsCount; i++)
    {
      *x++ = *a++;
      *y++ = *a++;
      if (z) *z++ = *a++;
    }

    delete ds->dsX;
    ds->dsX = dsX;
  }
  else if (ds->dsX->nx == 1)
  {
    ds->dsCount = ds->dsX->ny;
    ds->dsX->nx = ds->dsCount;
    ds->dsX->ny = 1;
  }

  (void)value;
  return 0;
}

static int iMglPlotSetDSSplitAttrib(Ihandle* ih, const char* value)
{
  int old_current;
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  
  // Must be Planar data
  if (ds->dsY || ds->dsZ || ds->dsX->nz != 1)
    return 0;
  if (ds->dsX->ny == 1)
    return 0;

  old_current = ih->data->dataSetCurrent;

  // Now nx!=1 and ny!=1, so rearrange data from ds->dsX to ds->dsY
  if (ds->dsX->ny == 2 || ds->dsX->ny == 3)
  {
    ds->dsCount = ds->dsX->nx;

    double* ax = ds->dsX->a;
    double* ay = ax + ds->dsCount;

    int dsy = IupMglPlotNewDataSet(ih, 1);
    IupMglPlotSet1D(ih, dsy, NULL, ay, ds->dsCount);

    if (ds->dsX->ny == 3)
    {
      double* az = ay + ds->dsCount;
      int dsz = IupMglPlotNewDataSet(ih, 1);
      IupMglPlotSet1D(ih, dsz, NULL, az, ds->dsCount);
    }

    ds->dsDim = 1;
    ds->dsX->ny = 1;
  }

  ih->data->dataSetCurrent = old_current;

  (void)value;
  return 0;
}

static int iMglPlotSetDSLineStyleAttrib(Ihandle* ih, const char* value)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetLineStyle(ih, value, ds->dsLineStyle);
}

static char* iMglPlotGetDSLineStyleAttrib(Ihandle* ih)
{
  IdataSet* ds;
  if (ih->data->dataSetCurrent==-1)
    return NULL;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotGetLineStyle(ds->dsLineStyle);
}

static int iMglPlotSetDSLineWidthAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetInt(ih, value, ds->dsLineWidth);
}

static char* iMglPlotGetDSLineWidthAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;
  else
  {
    IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
    return iupStrReturnInt(ds->dsLineWidth);
  }
}

static int iMglPlotSetMarkStyle(Ihandle* ih, const char* value, char& markstyle)
{
  char old_markstyle = markstyle;

  if (!value || iupStrEqualNoCase(value, "X"))
    markstyle = 'x';
  else if (iupStrEqualNoCase(value, "STAR"))
    markstyle = '*';
  else if (iupStrEqualNoCase(value, "CIRCLE"))
    markstyle = 'O';  // In fact "#o", we will fix it later in iMglPlotConfigDataSetLineMark
  else if (iupStrEqualNoCase(value, "PLUS"))
    markstyle = '+';
  else if (iupStrEqualNoCase(value, "BOX"))
    markstyle = 'S';  // In fact "#s", we will fix it later
  else if (iupStrEqualNoCase(value, "DIAMOND"))
    markstyle = 'D';  // In fact "#d", we will fix it later
  else if (iupStrEqualNoCase(value, "HOLLOW_CIRCLE"))
    markstyle = 'o';
  else if (iupStrEqualNoCase(value, "HOLLOW_BOX"))
    markstyle = 's';
  else if (iupStrEqualNoCase(value, "HOLLOW_DIAMOND"))
    markstyle = 'd';
  else if (iupStrEqualNoCase(value, "DOT"))
    markstyle = '.';
  else if (iupStrEqualNoCase(value, "ARROW_UP"))
    markstyle = '^';  // In fact "#^", we will fix it later
  else if (iupStrEqualNoCase(value, "ARROW_DOWN"))
    markstyle = 'V';  // In fact "#V", we will fix it later
  else if (iupStrEqualNoCase(value, "ARROW_LEFT"))
    markstyle = '<';  // In fact "#<", we will fix it later
  else if (iupStrEqualNoCase(value, "ARROW_RIGHT"))
    markstyle = '>';  // In fact "#>", we will fix it later
  else
    markstyle = 'x';  /* reset to default */
  
  if (old_markstyle != markstyle)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetDSMarkStyleAttrib(Ihandle* ih, const char* value)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetMarkStyle(ih, value, ds->dsMarkStyle);
}

static char* iMglPlotGetDSMarkStyleAttrib(Ihandle* ih)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return NULL;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];

  switch(ds->dsMarkStyle)
  {
    case '*': return "STAR";
    case 'O': return "CIRCLE";
    case 'x': return "X";
    case 'S': return "BOX";
    case 'D': return "DIAMOND";
    case 'o': return "HOLLOW_CIRCLE";
    case 's': return "HOLLOW_BOX";
    case 'd': return "HOLLOW_DIAMOND";
    case '^': return "ARROW_UP";
    case 'V': return "ARROW_DOWN";
    case '<': return "ARROW_LEFT";
    case '>': return "ARROW_RIGHT";
    case '.': return "DOT";
    default: return "PLUS";
  }
}

static int iMglPlotSetDSMarkSizeAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetDouble(ih, value, ds->dsMarkSize);
}

static char* iMglPlotGetDSMarkSizeAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;
  else
  {
    IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
    return iupStrReturnDouble(ds->dsMarkSize);
  }
}

static int iMglPlotSetDSColorAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  iMglPlotSetColor(ih, value, ds->dsColor);

  if (mgl_isnan(ds->dsColor.r) || mgl_isnan(ds->dsColor.g) || mgl_isnan(ds->dsColor.b))
    iMglPlotSetDsColorDefault(ds->dsColor, ih->data->dataSetCurrent);

  return 0;
}

static char* iMglPlotGetDSColorAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotReturnColor(ds->dsColor);
}

static int iMglPlotSetDSModeAttrib(Ihandle* ih, const char* value)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];

  free(ds->dsMode);
  if (value)
    ds->dsMode = iupStrDup(value);
  else
    ds->dsMode = iupStrDup("LINE");

  ih->data->redraw = true;
  return 0;
}

static char* iMglPlotGetDSModeAttrib(Ihandle* ih)
{
  IdataSet* ds;
  if (ih->data->dataSetCurrent==-1)
    return NULL;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return ds->dsMode;
}

static int iMglPlotSetDSLegendAttrib(Ihandle* ih, const char* value)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];

  free(ds->dsLegend);

  if (value)
    ds->dsLegend = iupStrDup(value);
  else
    ds->dsLegend = iMglPlotDefaultLegend(ih->data->dataSetCurrent);

  ih->data->redraw = true;
  return 0;
}

static char* iMglPlotGetDSLegendAttrib(Ihandle* ih)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return NULL;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return ds->dsLegend;
}

static int iMglPlotSetDSShowValuesAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetBoolean(ih, value, ds->dsShowValues);
}

static char* iMglPlotGetDSShowValuesAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iupStrReturnBoolean(ds->dsShowValues);
}

static int iMglPlotSetDSShowMarksAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetBoolean(ih, value, ds->dsShowMarks);
}

static char* iMglPlotGetDSShowMarksAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iupStrReturnBoolean(ds->dsShowMarks);
}

static int iMglPlotSetDSRemoveAttrib(Ihandle* ih, const char* value)
{
  IdataSet* ds;
  int index, remove_count=1;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  if (iMglPlotIsPlanarOrVolumetricData(ds))
    return 0;

  if (iupStrToIntInt(value, &index, &remove_count, ':'))
  {
    if(index < 0 || index+remove_count > ds->dsCount)
      return 0;

    if (index+remove_count < ds->dsCount)  /* if equal, remove at the end, no need to move data */
    {
      memmove(ds->dsX->a + index, ds->dsX->a + (index + remove_count), sizeof(double)*(ds->dsCount - (index + remove_count)));
      if (ds->dsY)
        memmove(ds->dsY->a + index, ds->dsY->a + (index + remove_count), sizeof(double)*(ds->dsCount - (index + remove_count)));
      if (ds->dsZ)
        memmove(ds->dsZ->a + index, ds->dsZ->a + (index + remove_count), sizeof(double)*(ds->dsCount - (index + remove_count)));
    }
    ds->dsX->Crop(ds->dsCount, 1, 'x');
    if (ds->dsY)
      ds->dsY->Crop(ds->dsCount, 1, 'y');
    if (ds->dsZ)
      ds->dsZ->Crop(ds->dsCount, 1, 'z');

    ds->dsCount -= remove_count;

    if (ds->dsNames)
    {
      char** dsNames = (char**)iupArrayGetData(ds->dsNames);
      free(dsNames[index]);
      iupArrayRemove(ds->dsNames, index, remove_count);
    }

    ih->data->redraw = true;
  }
  return 0;
}

static char* iMglPlotGetDSCountAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;
  else
  {
    IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
    return iupStrReturnInt(ds->dsCount);
  }
}

static char* iMglPlotGetDSDimAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;
  else
  {
    IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
    if (iMglPlotIsPlanarOrVolumetricData(ds))
      return iupStrReturnStrf("%dx%dx%d", (int)ds->dsX->nx, (int)ds->dsX->ny, (int)ds->dsX->nz);
    else
      return iupStrReturnInt(ds->dsDim);
  }
}

static int iMglPlotSetBoxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->Box);
}

static char* iMglPlotGetBoxAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->Box);
}

static int iMglPlotSetBoxTicksAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->boxTicks);
}

static char* iMglPlotGetBoxTicksAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->boxTicks);
}

static int iMglPlotSetAxisLabelPosition(Ihandle* ih, const char* value, int& pos)
{
  int old_pos = pos;

  if (iupStrEqualNoCase(value, "CENTER"))
    pos = 0;
  else if (iupStrEqualNoCase(value, "MIN"))
    pos = -1;
  else if (iupStrEqualNoCase(value, "MAX"))
    pos = 1;

  if (old_pos != pos)
    ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetAxisLabelPosition(int pos)
{
  if (pos == 0)
    return "CENTER";
  if (pos == 1)
    return "MAX";
  if (pos == -1)
    return "MIN";
  return NULL;
}

static int iMglPlotSetAxisXLabelPositionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisLabelPosition(ih, value, ih->data->axisX.axLabelPos);
}

static int iMglPlotSetAxisYLabelPositionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisLabelPosition(ih, value, ih->data->axisY.axLabelPos);
}

static int iMglPlotSetAxisZLabelPositionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisLabelPosition(ih, value, ih->data->axisZ.axLabelPos);
}

static char* iMglPlotGetAxisXLabelPositionAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisLabelPosition(ih->data->axisX.axLabelPos);
}

static char* iMglPlotGetAxisYLabelPositionAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisLabelPosition(ih->data->axisY.axLabelPos);
}

static char* iMglPlotGetAxisZLabelPositionAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisLabelPosition(ih->data->axisZ.axLabelPos);
}

static int iMglPlotSetAxisXLabelRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axLabelRotation);
}

static int iMglPlotSetAxisYLabelRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axLabelRotation);
}

static int iMglPlotSetAxisZLabelRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axLabelRotation);
}

static char* iMglPlotGetAxisXLabelRotationAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axLabelRotation);
}

static char* iMglPlotGetAxisYLabelRotationAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axLabelRotation);
}

static char* iMglPlotGetAxisZLabelRotationAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axLabelRotation);
}

static int iMglPlotSetAxisXLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    return iMglPlotSetAxisLabelPosition(ih, "CENTER", ih->data->axisX.axLabelPos);
  else 
    return iMglPlotSetAxisLabelPosition(ih, "MAX", ih->data->axisX.axLabelPos);
}

static int iMglPlotSetAxisYLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    return iMglPlotSetAxisLabelPosition(ih, "CENTER", ih->data->axisY.axLabelPos);
  else 
    return iMglPlotSetAxisLabelPosition(ih, "MAX", ih->data->axisY.axLabelPos);
}

static int iMglPlotSetAxisZLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    return iMglPlotSetAxisLabelPosition(ih, "CENTER", ih->data->axisZ.axLabelPos);
  else 
    return iMglPlotSetAxisLabelPosition(ih, "MAX", ih->data->axisZ.axLabelPos);
}

static char* iMglPlotGetAxisXLabelCenteredAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axLabelPos==0? true: false);
}

static char* iMglPlotGetAxisYLabelCenteredAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axLabelPos==0? true: false);
}

static char* iMglPlotGetAxisZLabelCenteredAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axLabelPos==0? true: false);
}

static int iMglPlotSetAxisXColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->axisX.axColor);
}

static int iMglPlotSetAxisYColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->axisY.axColor);
}

static int iMglPlotSetAxisZColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->axisZ.axColor);
}

static char* iMglPlotGetAxisXColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->axisX.axColor);
}

static char* iMglPlotGetAxisYColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->axisY.axColor);
}

static char* iMglPlotGetAxisZColorAttrib(Ihandle* ih)
{
  return iMglPlotReturnColor(ih->data->axisZ.axColor);
}

static int iMglPlotSetAxisXFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axLabelFontSizeFactor);
}

static int iMglPlotSetAxisYFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axLabelFontSizeFactor);
}

static int iMglPlotSetAxisZFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axLabelFontSizeFactor);
}

static char* iMglPlotGetAxisXFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axLabelFontSizeFactor);
}

static char* iMglPlotGetAxisYFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axLabelFontSizeFactor);
}

static char* iMglPlotGetAxisZFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axLabelFontSizeFactor);
}

static int iMglPlotSetAxisXFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->axisX.axLabelFontStyle);
}

static int iMglPlotSetAxisYFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->axisY.axLabelFontStyle);
}

static int iMglPlotSetAxisZFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->axisZ.axLabelFontStyle);
}

static char* iMglPlotGetAxisXFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->axisX.axLabelFontStyle);
}

static char* iMglPlotGetAxisYFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->axisY.axLabelFontStyle);
}

static char* iMglPlotGetAxisZFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->axisZ.axLabelFontStyle);
}

static int iMglPlotSetAxisXTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axTickFontSizeFactor);
}

static int iMglPlotSetAxisYTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axTickFontSizeFactor);
}

static int iMglPlotSetAxisZTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axTickFontSizeFactor);
}

static char* iMglPlotGetAxisXTickFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axTickFontSizeFactor);
}

static char* iMglPlotGetAxisYTickFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axTickFontSizeFactor);
}

static char* iMglPlotGetAxisZTickFontSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axTickFontSizeFactor);
}

static int iMglPlotSetAxisXTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->axisX.axTickFontStyle);
}

static int iMglPlotSetAxisYTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->axisY.axTickFontStyle);
}

static int iMglPlotSetAxisZTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->axisZ.axTickFontStyle);
}

static char* iMglPlotGetAxisXTickFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->axisX.axTickFontStyle);
}

static char* iMglPlotGetAxisYTickFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->axisY.axTickFontStyle);
}

static char* iMglPlotGetAxisZTickFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->axisZ.axTickFontStyle);
}

static int iMglPlotSetAxisXTickMinorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axTickMinorSizeFactor);
}

static int iMglPlotSetAxisYTickMinorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axTickMinorSizeFactor);
}

static int iMglPlotSetAxisZTickMinorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axTickMinorSizeFactor);
}

static char* iMglPlotGetAxisXTickMinorSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axTickMinorSizeFactor);
}

static char* iMglPlotGetAxisYTickMinorSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axTickMinorSizeFactor);
}

static char* iMglPlotGetAxisZTickMinorSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axTickMinorSizeFactor);
}

static int iMglPlotSetAxisXTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axTickMajorSize);
}

static int iMglPlotSetAxisYTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axTickMajorSize);
}

static int iMglPlotSetAxisZTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axTickMajorSize);
}

static char* iMglPlotGetAxisXTickMajorSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axTickMajorSize);
}

static char* iMglPlotGetAxisYTickMajorSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axTickMajorSize);
}

static char* iMglPlotGetAxisZTickMajorSizeAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axTickMajorSize);
}

static int iMglPlotSetAxisXTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axTickMajorSpan);
}

static int iMglPlotSetAxisYTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axTickMajorSpan);
}

static int iMglPlotSetAxisZTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axTickMajorSpan);
}

static char* iMglPlotGetAxisXTickMajorSpanAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axTickMajorSpan);
}

static char* iMglPlotGetAxisYTickMajorSpanAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axTickMajorSpan);
}

static char* iMglPlotGetAxisZTickMajorSpanAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axTickMajorSpan);
}

static int iMglPlotSetAxisScale(Ihandle* ih, const char* value, const char** scale, char dir)
{
  if(iupStrEqualNoCase(value, "LIN"))
    *scale = NULL;
  else if(iupStrEqualNoCase(value, "LOG10"))
    *scale = (dir=='x')? "lg(x)": (dir=='y'? "lg(y)": "lg(z)");
/* TODO else if(iupStrEqualNoCase(value, "LOG2"))   // NOT supported yet
    *scale = (dir=='x')? "log(x, 2)": (dir=='y'? "log(y, 2)": "log(z, 2)");
  else if(iupStrEqualNoCase(value, "LOGN"))
    *scale = (dir=='x')? "ln(x)": (dir=='y'? "ln(y)": "ln(z)"); */

  ih->data->redraw = true;
  return 0;
}

static int iMglPlotSetAxisXScaleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisScale(ih, value, &(ih->data->axisX.axScale), 'x');
}

static int iMglPlotSetAxisYScaleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisScale(ih, value, &(ih->data->axisY.axScale), 'y');
}

static int iMglPlotSetAxisZScaleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisScale(ih, value, &(ih->data->axisZ.axScale), 'z');
}

static char* iMglPlotGetAxisScale(const char* scale)
{
  if (!scale)
    return "LIN";
  else if (strstr(scale, "lg"))
    return "LOG10";
/* TODO else if (strstr(scale, "log"))  // NOT supported yet
    return "LOG2";
  else if (strstr(scale, "ln"))
    return "LOGN"; */
  return NULL;
}

static char* iMglPlotGetAxisXScaleAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisScale(ih->data->axisX.axScale);
}

static char* iMglPlotGetAxisYScaleAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisScale(ih->data->axisY.axScale);
}

static char* iMglPlotGetAxisZScaleAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisScale(ih->data->axisZ.axScale);
}

static int iMglPlotSetAxisXReverseAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axReverse);
}

static int iMglPlotSetAxisYReverseAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axReverse);
}

static int iMglPlotSetAxisZReverseAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axReverse);
}

static char* iMglPlotGetAxisXReverseAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axReverse);
}

static char* iMglPlotGetAxisYReverseAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axReverse);
}

static char* iMglPlotGetAxisZReverseAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axReverse);
}

static int iMglPlotSetAxisXShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axShow);
}

static int iMglPlotSetAxisYShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axShow);
}

static int iMglPlotSetAxisZShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axShow);
}

static char* iMglPlotGetAxisXShowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axShow);
}

static char* iMglPlotGetAxisYShowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axShow);
}

static char* iMglPlotGetAxisZShowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axShow);
}

static int iMglPlotSetAxisXShowArrowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axShowArrow);
}

static int iMglPlotSetAxisYShowArrowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axShowArrow);
}

static int iMglPlotSetAxisZShowArrowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axShowArrow);
}

static char* iMglPlotGetAxisXShowArrowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axShowArrow);
}

static char* iMglPlotGetAxisYShowArrowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axShowArrow);
}

static char* iMglPlotGetAxisZShowArrowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axShowArrow);
}

static int iMglPlotSetAxisXTickShowValuesAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axTickShowValues);
}

static int iMglPlotSetAxisYTickShowValuesAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axTickShowValues);
}

static int iMglPlotSetAxisZTickShowValuesAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axTickShowValues);
}

static char* iMglPlotGetAxisXTickShowValuesAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axTickShowValues);
}

static char* iMglPlotGetAxisYTickShowValuesAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axTickShowValues);
}

static char* iMglPlotGetAxisZTickShowValuesAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axTickShowValues);
}

static int iMglPlotSetAxisXTickAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axTickShow);
}

static int iMglPlotSetAxisYTickAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axTickShow);
}

static int iMglPlotSetAxisZTickAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axTickShow);
}

static char* iMglPlotGetAxisXTickAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axTickShow);
}

static char* iMglPlotGetAxisYTickAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axTickShow);
}

static char* iMglPlotGetAxisZTickAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axTickShow);
}

static int iMglPlotSetAxisXTickAutoSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axTickAutoSize);
}

static int iMglPlotSetAxisYTickAutoSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axTickAutoSize);
}

static int iMglPlotSetAxisZTickAutoSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axTickAutoSize);
}

static char* iMglPlotGetAxisXTickAutoSizeAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axTickAutoSize);
}

static char* iMglPlotGetAxisYTickAutoSizeAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axTickAutoSize);
}

static char* iMglPlotGetAxisZTickAutoSizeAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axTickAutoSize);
}

static int iMglPlotSetAxisXTickAutoAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axTickAutoSpace);
}

static int iMglPlotSetAxisYTickAutoAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axTickAutoSpace);
}

static int iMglPlotSetAxisZTickAutoAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axTickAutoSpace);
}

static char* iMglPlotGetAxisXTickAutoAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axTickAutoSpace);
}

static char* iMglPlotGetAxisYTickAutoAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axTickAutoSpace);
}

static char* iMglPlotGetAxisZTickAutoAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axTickAutoSpace);
}

static int iMglPlotSetAxisXTickValuesRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axTickValuesRotation);
}

static int iMglPlotSetAxisYTickValuesRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axTickValuesRotation);
}

static int iMglPlotSetAxisZTickValuesRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axTickValuesRotation);
}

static char* iMglPlotGetAxisXTickValuesRotationAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axTickValuesRotation);
}

static char* iMglPlotGetAxisYTickValuesRotationAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axTickValuesRotation);
}

static char* iMglPlotGetAxisZTickValuesRotationAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axTickValuesRotation);
}

static int iMglPlotSetAxisXTickMinorDivisionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetInt(ih, value, ih->data->axisX.axTickMinorDivision);
}

static int iMglPlotSetAxisYTickMinorDivisionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetInt(ih, value, ih->data->axisY.axTickMinorDivision);
}

static int iMglPlotSetAxisZTickMinorDivisionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetInt(ih, value, ih->data->axisZ.axTickMinorDivision);
}

static char* iMglPlotGetAxisXTickMinorDivisionAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->axisX.axTickMinorDivision);
}

static char* iMglPlotGetAxisYTickMinorDivisionAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->axisY.axTickMinorDivision);
}

static char* iMglPlotGetAxisZTickMinorDivisionAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->axisZ.axTickMinorDivision);
}

static int iMglPlotSetAxisXAutoMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axAutoScaleMin);
}

static int iMglPlotSetAxisYAutoMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axAutoScaleMin);
}

static int iMglPlotSetAxisZAutoMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axAutoScaleMin);
}

static char* iMglPlotGetAxisXAutoMinAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axAutoScaleMin);
}

static char* iMglPlotGetAxisYAutoMinAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axAutoScaleMin);
}

static char* iMglPlotGetAxisZAutoMinAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axAutoScaleMin);
}

static int iMglPlotSetAxisXAutoMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisX.axAutoScaleMax);
}

static int iMglPlotSetAxisYAutoMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisY.axAutoScaleMax);
}

static int iMglPlotSetAxisZAutoMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->axisZ.axAutoScaleMax);
}

static char* iMglPlotGetAxisXAutoMaxAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisX.axAutoScaleMax);
}

static char* iMglPlotGetAxisYAutoMaxAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisY.axAutoScaleMax);
}

static char* iMglPlotGetAxisZAutoMaxAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->axisZ.axAutoScaleMax);
}

static int iMglPlotSetAxisXMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axMin);
}

static int iMglPlotSetAxisYMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axMin);
}

static int iMglPlotSetAxisZMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axMin);
}

static char* iMglPlotGetAxisXMinAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axMin);
}

static char* iMglPlotGetAxisYMinAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axMin);
}

static char* iMglPlotGetAxisZMinAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axMin);
}

static int iMglPlotSetAxisXMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisX.axMax);
}

static int iMglPlotSetAxisYMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisY.axMax);
}

static int iMglPlotSetAxisZMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->axisZ.axMax);
}

static char* iMglPlotGetAxisXMaxAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisX.axMax);
}

static char* iMglPlotGetAxisYMaxAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisY.axMax);
}

static char* iMglPlotGetAxisZMaxAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->axisZ.axMax);
}

static int iMglPlotSetAxisCrossOrigin(Ihandle* ih, const char* value, double& origin)
{
  double old_origin = origin;

  if (iupStrBoolean(value))
    origin = 0;
  else
    origin = NAN;

  if (old_origin != origin)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetAxisXCrossOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisCrossOrigin(ih, value, ih->data->axisX.axOrigin);
}

static int iMglPlotSetAxisYCrossOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisCrossOrigin(ih, value, ih->data->axisY.axOrigin);
}

static int iMglPlotSetAxisZCrossOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisCrossOrigin(ih, value, ih->data->axisZ.axOrigin);
}

static char* iMglPlotGetAxisXCrossOrigin(double origin)
{
  if (origin==0)
    return "YES";
  else if (mgl_isnan(origin))
    return "NO";
  else
    return NULL;
}

static char* iMglPlotGetAxisXCrossOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisXCrossOrigin(ih->data->axisX.axOrigin);
}

static char* iMglPlotGetAxisYCrossOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisXCrossOrigin(ih->data->axisY.axOrigin);
}

static char* iMglPlotGetAxisZCrossOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisXCrossOrigin(ih->data->axisZ.axOrigin);
}

static int iMglPlotSetAxisOrigin(Ihandle* ih, const char* value, double& num)
{
  double old_num = num;

  if (!value)
    num = NAN;
  else
    iupStrToDouble(value, &num);

  if (old_num != num)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetAxisXOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisOrigin(ih, value, ih->data->axisX.axOrigin);
}

static int iMglPlotSetAxisYOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisOrigin(ih, value, ih->data->axisY.axOrigin);
}

static int iMglPlotSetAxisZOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisOrigin(ih, value, ih->data->axisZ.axOrigin);
}

static char* iMglPlotGetAxisOrigin(double Origin)
{
  if (!mgl_isnan(Origin))
    return iupStrReturnDouble(Origin);
  else
    return NULL;
}

static char* iMglPlotGetAxisXOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisOrigin(ih->data->axisX.axOrigin);
}

static char* iMglPlotGetAxisYOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisOrigin(ih->data->axisY.axOrigin);
}

static char* iMglPlotGetAxisZOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisOrigin(ih->data->axisZ.axOrigin);
}

static int iMglPlotSetAlphaAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetDouble(ih, value, ih->data->alpha);
}

static char* iMglPlotGetAlphaAttrib(Ihandle* ih)
{
  return iupStrReturnDouble(ih->data->alpha);
}

static int iMglPlotSetTransparentAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->transparent);
}

static char* iMglPlotGetTransparentAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->transparent);
}

static void iMglPlotInitOpenGL2D(void)
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(-1.0, 0, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  /* data alignment is 1 */
  glPixelZoom(1.0, -1.0);  // vertical flip image
  glRasterPos2f(0, 1.0);
}

static int iMglPlotSetOpenGLAttrib(Ihandle* ih, const char* value)
{
  bool old_opengl = ih->data->opengl;
  iMglPlotSetBoolean(ih, value, ih->data->opengl);

  if (old_opengl != ih->data->opengl)
  {
    // No need to reset attrib since mgl will be used only in ACTION
    delete ih->data->mgl;
    ih->data->redraw = 1;

    if (ih->data->opengl)
      ih->data->mgl = new mglGraph(1, ih->data->w, ih->data->h);
    else
    {
      if (ih->handle)
      {
        IupGLMakeCurrent(ih);
        iMglPlotInitOpenGL2D();
      }
      ih->data->mgl = new mglGraph(0, ih->data->w, ih->data->h);
    }
  }

  return 0;
}

static char* iMglPlotGetOpenGLAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->opengl);
}

static int iMglPlotSetAntialiasAttrib(Ihandle* ih, const char* value)
{
  if (!ih->data->opengl)
    return 0;

  IupGLMakeCurrent(ih);

  if (iupStrBoolean(value))
  {
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  }
  else
  {
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
  }

  return 0;
}

static char* iMglPlotGetAntialiasAttrib(Ihandle* ih)
{
  if (!ih->data->opengl)
    return NULL;

  IupGLMakeCurrent(ih);
  return iupStrReturnBoolean(glIsEnabled(GL_LINE_SMOOTH)==GL_TRUE);
}

static char* iMglPlotGetErrorMessageAttrib(Ihandle* ih)
{
  char* err = (char*)ih->data->mgl->Message();
  if (err && err[0] != 0)
    return err;
  else
    return NULL;
}

static int iMglPlotSetZoomAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->data->x1 = 0;
    ih->data->y1 = 0;
    ih->data->x2 = 1.0;
    ih->data->y2 = 1.0;
  }
  else
  {
    sscanf(value, "%f:%f:%f:%f", &(ih->data->x1), &(ih->data->y1), &(ih->data->x2), &(ih->data->y2));

    if (ih->data->x1 < 0) ih->data->x1 = 0;
    if (ih->data->y1 < 0) ih->data->y1 = 0;
    if (ih->data->x2 > 1.0) ih->data->x2 = 1.0;
    if (ih->data->y2 > 1.0) ih->data->y2 = 1.0;
  }

  ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetZoomAttrib(Ihandle* ih)
{
  return iupStrReturnStrf("%.9f,%.9f:%.9f,%.9f", ih->data->x1, ih->data->y1, ih->data->x2, ih->data->y2);
}

static int iMglPlotSetRotateAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->data->rotX = 0;
    ih->data->rotY = 0;
    ih->data->rotZ = 0;
  }
  else
    sscanf(value, "%f:%f:%f", &(ih->data->rotX), &(ih->data->rotY), &(ih->data->rotZ));

  ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetRotateAttrib(Ihandle* ih)
{
  return iupStrReturnStrf("%.9f:%.9f:%.9f", ih->data->rotX, ih->data->rotY, ih->data->rotZ);
}


/******************************************************************************
Additional Functions
******************************************************************************/


int IupMglPlotNewDataSet(Ihandle *ih, int dim)
{
  int ds_index;
  IdataSet* ds;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return -1;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return -1;

  /* Increment the number of datasets */
  ih->data->dataSetCount++;

  // Allocate memory if necessary
  if(ih->data->dataSetCount == ih->data->dataSetMaxCount)
  {
    ih->data->dataSetMaxCount += ih->data->dataSetMaxCount;
    ih->data->dataSet = (IdataSet*)realloc(ih->data->dataSet, ih->data->dataSetMaxCount*sizeof(IdataSet));
    memset(ih->data->dataSet+ih->data->dataSetCount-1, 0, sizeof(IdataSet)*(ih->data->dataSetMaxCount - (ih->data->dataSetCount-1)));
  }

  ds_index = ih->data->dataSetCount-1;
  ds = &ih->data->dataSet[ds_index];

  ds->dsDim = dim; 
  ds->dsX = new mglData();
  if (dim > 1)
    ds->dsY = new mglData();
  if (dim > 2)
    ds->dsZ = new mglData();
  ds->dsCount = 0;

  /* Initialize the default values */
  iMglPlotResetDataSet(ds, ds_index);

  ih->data->redraw = true;

  ih->data->dataSetCurrent = ds_index;
  return ih->data->dataSetCurrent;
}

void IupMglPlotBegin(Ihandle* ih, int dim)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  Iarray* inXData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_XDATA");
  Iarray* inYData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_YDATA");
  Iarray* inZData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_ZDATA");
  Iarray* inNames = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_NAMES");

  if (inXData) { iupArrayDestroy(inXData); inXData = NULL;}
  if (inYData) { iupArrayDestroy(inYData); inYData = NULL;}
  if (inZData) { iupArrayDestroy(inZData); inZData = NULL;}
  if (inNames) iupArrayDestroy(inNames);

  inXData =  iupArrayCreate(10, sizeof(double));
  if (dim>1)
    inYData =  iupArrayCreate(10, sizeof(double));
  if (dim>2)
    inZData =  iupArrayCreate(10, sizeof(double));

  iupAttribSet(ih, "_IUP_MGLPLOT_XDATA", (char*)inXData);
  iupAttribSet(ih, "_IUP_MGLPLOT_YDATA", (char*)inYData);
  iupAttribSet(ih, "_IUP_MGLPLOT_ZDATA", (char*)inZData);
  iupAttribSet(ih, "_IUP_MGLPLOT_NAMES", NULL);
}

int IupMglPlotEnd(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return -1;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "mglplot"))
    return -1;

  Iarray* inXData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_XDATA");
  Iarray* inYData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_YDATA");
  Iarray* inZData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_ZDATA");
  Iarray* inNames = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_NAMES");
  int dim = 0;
  if (inXData) dim = 1;
  if (inYData) dim = 2;
  if (inZData) dim = 3;
  if (dim == 0)
    return -1;

  // Actually add the dataset only at the End
  int ds_index = IupMglPlotNewDataSet(ih, dim);
  if (dim==1)
  {
    char** names = NULL;
    if (inNames)
      names = (char**)iupArrayGetData(inNames);
    double* x = (double*)iupArrayGetData(inXData);
    int count = iupArrayCount(inXData);
    IupMglPlotSet1D(ih, ds_index, (const char**)names, x, count);
  }
  else if (dim==2)
  {
    double* x = (double*)iupArrayGetData(inXData);
    double* y = (double*)iupArrayGetData(inYData);
    int count = iupArrayCount(inXData);
    IupMglPlotSet2D(ih, ds_index, x, y, count);
  }
  else if (dim==3)
  {
    double* x = (double*)iupArrayGetData(inXData);
    double* y = (double*)iupArrayGetData(inYData);
    double* z = (double*)iupArrayGetData(inZData);
    int count = iupArrayCount(inXData);
    IupMglPlotSet3D(ih, ds_index, x, y, z, count);
  }

  if (inXData) iupArrayDestroy(inXData);
  if (inYData) iupArrayDestroy(inYData);
  if (inZData) iupArrayDestroy(inZData);
  if (inNames) iupArrayDestroy(inNames);
  iupAttribSet(ih, "_IUP_MGLPLOT_XDATA", NULL);
  iupAttribSet(ih, "_IUP_MGLPLOT_YDATA", NULL);
  iupAttribSet(ih, "_IUP_MGLPLOT_ZDATA", NULL);
  iupAttribSet(ih, "_IUP_MGLPLOT_NAMES", NULL);

  return ds_index;
}

void IupMglPlotAdd1D(Ihandle* ih, const char* inName, double inX)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  Iarray* inXData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_XDATA");
  Iarray* inYData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_YDATA");
  Iarray* inZData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_ZDATA");
  int dim = 0;
  if (inXData) dim = 1;
  if (inYData) dim = 2;
  if (inZData) dim = 3;
  if (dim < 1)
    return;

  int sample_index = iupArrayCount(inXData);  /* get before incrementing the array */

  if (inName)
  {
    Iarray* inNames = NULL;
    if (sample_index == 0)
    {
      if (inNames) iupArrayDestroy(inNames);
      inNames =  iupArrayCreate(10, sizeof(char*));
      iupAttribSet(ih, "_IUP_MGLPLOT_NAMES", (char*)inNames);
    }
    else
      inNames = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_NAMES");

    if (inNames)
    {
      char** names = (char**)iupArrayInc(inNames);
      names[sample_index] = iupStrDup(inName);
    }
  }

  double* x = (double*)iupArrayInc(inXData);
  x[sample_index] = inX;
}

void IupMglPlotAdd2D(Ihandle* ih, double inX, double inY)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  Iarray* inXData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_XDATA");
  Iarray* inYData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_YDATA");
  Iarray* inZData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_ZDATA");
  int dim = 0;
  if (inXData) dim = 1;
  if (inYData) dim = 2;
  if (inZData) dim = 3;
  if (dim < 2)
    return;

  int sample_index = iupArrayCount(inXData);  /* get before incrementing the array */

  double* x = (double*)iupArrayInc(inXData);
  double* y = (double*)iupArrayInc(inYData);
  x[sample_index] = inX;
  y[sample_index] = inY;
}

void IupMglPlotAdd3D(Ihandle *ih, double inX, double inY, double inZ)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  Iarray* inXData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_XDATA");
  Iarray* inYData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_YDATA");
  Iarray* inZData = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_ZDATA");
  int dim = 0;
  if (inXData) dim = 1;
  if (inYData) dim = 2;
  if (inZData) dim = 3;
  if (dim < 3)
    return;

  int sample_index = iupArrayCount(inXData);  /* get before incrementing the array */

  double* x = (double*)iupArrayInc(inXData);
  double* y = (double*)iupArrayInc(inYData);
  double* z = (double*)iupArrayInc(inZData);
  x[sample_index] = inX;
  y[sample_index] = inY;
  z[sample_index] = inZ;
}

void IupMglPlotInsert1D(Ihandle* ih, int inIndex, int inSampleIndex, const char** inNames, const double* inX, int inCount)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || inCount<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];

  if(inSampleIndex > ds->dsCount || inSampleIndex < 0)
    return;

  ds->dsCount += inCount;
  ds->dsX->Extend(ds->dsCount);
  if (inSampleIndex < ds->dsCount-1)  // insert in the middle, open space first
    memmove(ds->dsX->a + inSampleIndex + inCount, ds->dsX->a + inSampleIndex, inCount*sizeof(double));
  memcpy(ds->dsX->a + inSampleIndex, inX, inCount*sizeof(double));

  if (inNames && ds->dsNames && inIndex==0)  // Allow names only for the first dataset
  {
    char** dsNames = (char**)iupArrayInsert(ds->dsNames, inSampleIndex, inCount);
    for (int i = 0; i < inCount; i++)
    {
      dsNames[inSampleIndex] = iupStrDup(inNames[i]!=NULL? inNames[i]: "");  
      inSampleIndex++;
    }
  }
}

void IupMglPlotInsert2D(Ihandle* ih, int inIndex, int inSampleIndex, const double *inX, const double *inY, int inCount)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || inCount<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];
  if (!ds->dsY)
    return;

  if(inSampleIndex > ds->dsCount || inSampleIndex < 0)
    return;

  ds->dsCount += inCount;
  ds->dsX->Extend(ds->dsCount);
  ds->dsY->Extend(ds->dsCount);
  if (inSampleIndex < ds->dsCount-1)  // insert in the middle, open space first
  {
    memmove(ds->dsX->a + inSampleIndex + inCount, ds->dsX->a + inSampleIndex, inCount*sizeof(double));
    memmove(ds->dsY->a + inSampleIndex + inCount, ds->dsY->a + inSampleIndex, inCount*sizeof(double));
  }
  memcpy(ds->dsX->a + inSampleIndex, inX, inCount*sizeof(double));
  memcpy(ds->dsY->a + inSampleIndex, inY, inCount*sizeof(double));
}

void IupMglPlotInsert3D(Ihandle* ih, int inIndex, int inSampleIndex, const double* inX, const double* inY, const double* inZ, int inCount)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || inCount<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];
  if (!ds->dsY || !ds->dsZ)
    return;

  if(inSampleIndex > ds->dsCount || inSampleIndex < 0)
    return;

  ds->dsCount += inCount;
  ds->dsX->Extend(ds->dsCount);
  ds->dsY->Extend(ds->dsCount);
  ds->dsZ->Extend(ds->dsCount);
  if (inSampleIndex < ds->dsCount-1)  // insert in the middle, open space first
  {
    memmove(ds->dsX->a + inSampleIndex + inCount, ds->dsX->a + inSampleIndex, inCount*sizeof(double));
    memmove(ds->dsY->a + inSampleIndex + inCount, ds->dsY->a + inSampleIndex, inCount*sizeof(double));
    memmove(ds->dsZ->a + inSampleIndex + inCount, ds->dsZ->a + inSampleIndex, inCount*sizeof(double));
  }
  memcpy(ds->dsX->a + inSampleIndex, inX, inCount*sizeof(double));
  memcpy(ds->dsY->a + inSampleIndex, inY, inCount*sizeof(double));
  memcpy(ds->dsZ->a + inSampleIndex, inZ, inCount*sizeof(double));
}

void IupMglPlotSet1D(Ihandle* ih, int inIndex, const char** inNames, const double* inX, int inCount)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || inCount<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];

  if (inNames && inIndex==0)   // Allow names only for the first dataset
  {
    char** dsNames;
    int j;

    if (!ds->dsNames)
    {
      ds->dsNames = iupArrayCreate(10, sizeof(char*));
      dsNames = (char**)iupArrayAdd(ds->dsNames, inCount);
    }
    else
    {
      dsNames = (char**)iupArrayGetData(ds->dsNames);
      int count = iupArrayCount(ds->dsNames);
      for (j=0; j<count; j++)
        free(dsNames[j]);

      if (count < inCount)
        dsNames = (char**)iupArrayAdd(ds->dsNames, inCount - count);
      else if (count > inCount)
        iupArrayRemove(ds->dsNames, inCount, count - inCount);
    }

    for (j = 0; j < inCount; j++)
    {
      dsNames[j] = iupStrDup(inNames[j]!=NULL? inNames[j]: "");  
    }
  }

  ds->dsX->Set(inX, inCount);
  ds->dsCount = inCount;

  ih->data->redraw = true;
}

void IupMglPlotSet2D(Ihandle* ih, int inIndex, const double *inX, const double *inY, int inCount)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || inCount<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];
  if (!ds->dsY)
    return;

  ds->dsX->Set(inX, inCount);
  ds->dsY->Set(inY, inCount);
  ds->dsCount = inCount;

  ih->data->redraw = true;
}

void IupMglPlotSet3D(Ihandle* ih, int inIndex, const double* inX, const double* inY, const double* inZ, int inCount)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || inCount<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];
  if (!ds->dsY || !ds->dsZ)
    return;

  ds->dsX->Set(inX, inCount);
  ds->dsY->Set(inY, inCount);
  ds->dsZ->Set(inZ, inCount);
  ds->dsCount = inCount;

  ih->data->redraw = true;
}

void IupMglPlotSetData(Ihandle* ih, int inIndex, const double* data, int count_x, int count_y, int count_z)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 ||
     count_x<=0 || count_y<=0 || count_z<=0)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];
  ds->dsX->Set(data, count_x, count_y, count_z);
  ds->dsCount = count_x*count_y*count_z;

  ih->data->redraw = true;
}

void IupMglPlotLoadData(Ihandle* ih, int inIndex, const char* filename, int count_x, int count_y, int count_z)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 ||
     !filename)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];
  if (count_x==0 || count_y==0 || count_z==0)
    ds->dsX->Read(filename);
  else
    ds->dsX->Read(filename, count_x, count_y, count_z);
  ds->dsCount = ds->dsX->nx*ds->dsX->ny*ds->dsX->nz;

  ih->data->redraw = true;
}

void IupMglPlotSetFormula(Ihandle* ih, int inIndex, const char* formulaX, const char* formulaY, const char* formulaZ, int count)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 || !formulaX)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];

  if (ds->dsDim >= 2 && !formulaY)
    return;
  if (ds->dsDim == 3 && !formulaZ)
    return;

  count = count>0? count: ds->dsCount;
  if (count != ds->dsCount)
  {
    ds->dsCount = count;
    ds->dsX->Create(count);
    if (ds->dsY) ds->dsY->Create(count);
    if (ds->dsZ) ds->dsZ->Create(count);
  }

  ds->dsX->Modify(formulaX);
  if (ds->dsY) ds->dsY->Modify(formulaY);
  if (ds->dsZ) ds->dsZ->Modify(formulaZ);

  ih->data->redraw = true;
}

void IupMglPlotSetFromFormula(Ihandle* ih, int inIndex, const char* formula, int count_x, int count_y, int count_z)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if(inIndex > (ih->data->dataSetCount-1) || inIndex < 0 ||
     !formula)
    return;

  IdataSet* ds = &ih->data->dataSet[inIndex];

  int nx = count_x>0? count_x: ds->dsX->nx;
  int ny = count_y>0? count_y: ds->dsX->ny;
  int nz = count_z>0? count_z: ds->dsX->nz;
  if (nx != ds->dsX->nx || ny != ds->dsX->ny || nz != ds->dsX->nz)
    ds->dsX->Create(nx, ny, nz);

  ds->dsX->Modify(formula);
  ds->dsCount = ds->dsX->nx*ds->dsX->ny*ds->dsX->nz;

  ih->data->redraw = true;
}

void IupMglPlotTransform(Ihandle* ih, double x, double y, double z, int *ix, int *iy)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "mglplot"))
    return;

  mglPoint p = ih->data->mgl->CalcScr(mglPoint(x, y, z));

  if (ix) *ix = (int)p.x;
  if (iy) *iy = (int)p.y;
}

void IupMglPlotTransformXYZ(Ihandle* ih, int ix, int iy, double *x, double *y, double *z)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "mglplot"))
    return;

  mglPoint p = ih->data->mgl->CalcXYZ(ix, iy);
  if (x) *x = p.x;
  if (y) *y = p.y;
  if (z) *z = p.z;
}

void IupMglPlotPaintTo(Ihandle* ih, const char* format, int w, int h, double dpi, void *data)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if (!format || !data)
    return;

  int old_w = ih->data->w;
  int old_h = ih->data->h;
  double old_dpi = ih->data->dpi;

  if (dpi == 0) dpi = old_dpi;
  if (w == 0) w = old_w;
  if (h == 0) h = old_h;

  ih->data->w = w;
  ih->data->h = h;
  ih->data->dpi = dpi;

  if (*format == 'E') //EPS
  {
    char* filename = (char*)data;

    mglGraph* gr = new mglGraph(0, w, h);
    iMglPlotDrawPlot(ih, gr);
    gr->WriteEPS(filename, "IupMglPlot");

    delete gr;
  }
  else if (*format == 'S') //SVG
  {
    char* filename = (char*)data;

    mglGraph* gr = new mglGraph(0, w, h);
    iMglPlotDrawPlot(ih, gr);
    gr->WriteSVG(filename, "IupMglPlot");

    delete gr;
  }
  else if (*format == 'R') //RGB
  {
    int alpha = 0;
    const unsigned char *input_bits;
    unsigned char* output_bits = (unsigned char*)data;
    if (*(format+3) == 'A')
      alpha = 1;

    mglGraph* gr = new mglGraph(0, w, h);
    iMglPlotDrawPlot(ih, gr);
    if (alpha)
      input_bits = gr->GetRGBA();
    else
      input_bits = gr->GetRGB();
    memcpy(output_bits, input_bits, w*h*(alpha? 4:3));

    delete gr;
  }

  ih->data->w = old_w;
  ih->data->h = old_h;
  ih->data->dpi = old_dpi;
}

void IupMglPlotDrawMark(Ihandle* ih, double x, double y, double z)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  mglGraph* gr = (mglGraph*)iupAttribGet(ih, "_IUP_MGLPLOT_GRAPH");
  if (!gr)
    return;

  char* value = iupAttribGetStr(ih, "DRAWCOLOR");
  mglColor markColor;
  if (!value) markColor = ih->data->fgColor;
  else iMglPlotSetColor(ih, value, markColor);
  iMglPlotConfigColor(ih, gr, markColor);

  value = iupAttribGetStr(ih, "DRAWMARKSTYLE");
  char markstyle = 'x';
  iMglPlotSetMarkStyle(ih, value, markstyle);

  value = iupAttribGetStr(ih, "DRAWMARKSIZE");
  double marksize = 0.02f;
  iMglPlotSetDouble(ih, value, marksize);
  gr->SetMarkSize(marksize);

  gr->Mark(mglPoint(x, y, z), &markstyle);
}

void IupMglPlotDrawLine(Ihandle* ih, double x1, double y1, double z1, double x2, double y2, double z2)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  mglGraph* gr = (mglGraph*)iupAttribGet(ih, "_IUP_MGLPLOT_GRAPH");
  if (!gr)
    return;

  char* value = iupAttribGetStr(ih, "DRAWCOLOR");
  mglColor lineColor;
  if (!value) lineColor = ih->data->fgColor;
  else iMglPlotSetColor(ih, value, lineColor);

  value = iupAttribGetStr(ih, "DRAWLINESTYLE");
  char linestyle = '-';
  iMglPlotSetLineStyle(ih, value, linestyle);

  value = iupAttribGetStr(ih, "DRAWLINEWIDTH");
  int linewidth = 0;
  iupStrToInt(value, &linewidth);
  if (linewidth<=0) linewidth = 1;

  char pen[10];
  iMglPlotConfigColor(ih, gr, lineColor);
  iMglPlotConfigPen(pen, linestyle, linewidth);
  gr->Line(mglPoint(x1, y1, z1), mglPoint(x2, y2, z2), pen);
}

void IupMglPlotDrawText(Ihandle* ih, const char* text, double x, double y, double z)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  mglGraph* gr = (mglGraph*)iupAttribGet(ih, "_IUP_MGLPLOT_GRAPH");
  if (!gr)
    return;

  char* value = iupAttribGetStr(ih, "DRAWCOLOR");
  mglColor textColor;
  if (!value) textColor = ih->data->fgColor;
  else iMglPlotSetColor(ih, value, textColor);

  value = iupAttribGetStr(ih, "DRAWFONTSTYLE");
  int fontstyle = IUP_MGLPLOT_INHERIT;
  iMglPlotSetFontStyle(ih, value, fontstyle);

  value = iupAttribGetStr(ih, "DRAWFONTSIZE");
  double fontsize = 1.0;
  iMglPlotSetDouble(ih, value, fontsize);

  char* style = iupAttribGetStr(ih, "DRAWTEXTALIGN");
  if (style)
  {
    if (iupStrEqualNoCase(style, "LEFT"))
      style = "L";
    else if (iupStrEqualNoCase(style, "CENTER"))
      style = "C";
    else if (iupStrEqualNoCase(style, "RIGHT"))
      style = "R";
    else
      style = NULL;
  }

  iMglPlotConfigColor(ih, gr, textColor);
  iMglPlotConfigFont(ih, gr, fontstyle, fontsize);
  
  gr->Puts(mglPoint(x, y, z), text, style, -1);
}


/******************************************************************************
 Canvas Callbacks
******************************************************************************/


static void iMglPlotZoom(Ihandle *ih, double factor)
{
  double rh = ih->data->y2 - ih->data->y1;
  double ry = (rh * factor) / (double)ih->data->h;
  double rw = ih->data->x2 - ih->data->x1;
  double rx = (rw * factor) / (double)ih->data->w;
  ih->data->y1 += ry;
  ih->data->y2 -= ry;
  ih->data->x1 += rx;
  ih->data->x2 -= rx;

  //TODO: temporary-commented while adjusting sizes
  //if (ih->data->x1 < 0) ih->data->x1 = 0;
  //if (ih->data->y1 < 0) ih->data->y1 = 0;
  //if (ih->data->x2 > 1.0) ih->data->x2 = 1.0;
  //if (ih->data->y2 > 1.0) ih->data->y2 = 1.0;
}

static void iMglPlotPanY(Ihandle *ih, double yoffset)
{
  double rh = ih->data->y2 - ih->data->y1;
  double ry = (rh * yoffset) / (double)ih->data->h;
  if (ih->data->y1 + ry < 0) ry = -ih->data->y1;
  if (ih->data->y2 + ry > 1.0) ry = 1.0 - ih->data->y2;
  ih->data->y1 += ry;
  ih->data->y2 += ry;
}

static void iMglPlotPanX(Ihandle *ih, double xoffset)
{
  double rw = ih->data->x2 - ih->data->x1;
  double rx = (rw * xoffset) / (double)ih->data->w;
  if (ih->data->x1 + rx < 0) rx = -ih->data->x1;
  if (ih->data->x2 + rx > 1.0) rx = 1.0 - ih->data->x2;
  ih->data->x1 += rx;
  ih->data->x2 += rx;
}

static void iMglPlotRotate(double &angle, double delta)
{
  angle += delta;

  // Keep between -360:+360 just to return a nice value in the getattrib method
  angle = fmod(angle, 360.0);
}

static int iMglPlotResize_CB(Ihandle* ih, int width, int height)
{
  IupGLMakeCurrent(ih);
  glViewport(0, 0, width, height);

  if (!ih->data->opengl)
    iMglPlotInitOpenGL2D();

  ih->data->redraw = true;
  ih->data->w = width;
  ih->data->h = height;
  ih->data->dpi = IupGetFloat(NULL, "SCREENDPI");

  ih->data->mgl->SetSize(width, height);

  return IUP_DEFAULT;
}

static int iMglPlotRedraw_CB(Ihandle* ih)
{
  iMglPlotRepaint(ih, 0, 1);  /* full redraw only if nothing changed */
  return IUP_DEFAULT;
}

static int iMglPlotMouseButton_CB(Ihandle* ih, int b, int press, int x, int y, char* status)
{
  if (press)  /* Any Button pressed */
  {
    /* Initial (x,y) */
    ih->data->last_x = (double)x;
    ih->data->last_y = (double)y;
  }

  if (iup_isdouble(status))  /* Double-click: restore interaction default values */
  {
    iMglPlotResetInteraction(ih);
    iMglPlotRepaint(ih, 1, 1);
  }

  (void)b;
  return IUP_DEFAULT;
}

static int iMglPlotMouseMove_CB(Ihandle* ih, int x, int y, char *status)
{
  double cur_x = (double)x;
  double cur_y = (double)y;

  if (iup_isbutton3(status))
  {
    // This is the same computation done in MathGL widgets
    double ff = 240.0 / sqrt(double(ih->data->w * ih->data->h));
    double deltaZ = (ih->data->last_x - cur_x) * ff;
    double deltaX = (ih->data->last_y - cur_y) * ff;

    iMglPlotRotate(ih->data->rotX, deltaX);
    iMglPlotRotate(ih->data->rotZ, deltaZ);

    iMglPlotRepaint(ih, 1, 1);
  }
  else if(iup_isbutton1(status))
  {
    if (iup_iscontrol(status))
    {
      /* Zoom with vertical movement */
      double factor = 10.0 * (ih->data->last_y - cur_y);
      iMglPlotZoom(ih, factor);
    }
    else
    {
      /* Pan */
      double xoffset = ih->data->last_x - cur_x;
      double yoffset = cur_y - ih->data->last_y;  /* Inverted because Y in IUP is top-down */

      iMglPlotPanX(ih, xoffset);
      iMglPlotPanY(ih, yoffset);
    }

    iMglPlotRepaint(ih, 1, 1);
  }

  ih->data->last_x = cur_x;
  ih->data->last_y = cur_y;

  return IUP_DEFAULT;
}

static int iMglPlotWheel_CB(Ihandle* ih, double delta)
{
  if(delta > 0)  /* Zoom In */
  {
    iMglPlotZoom(ih, 50.0);
    iMglPlotRepaint(ih, 1, 1);
  }
  else if(delta < 0)  /* Zoom Out */
  {
    iMglPlotZoom(ih, -50.0);
    iMglPlotRepaint(ih, 1, 1);
  }

  return IUP_DEFAULT;
}

static int iMglPlotKeyPress_CB(Ihandle* ih, int c, int press)
{
  if(!press)
    return IUP_DEFAULT;

  switch(c)
  {
  /* Restore interaction default values */
  case K_HOME:
    iMglPlotResetInteraction(ih);
    break;
  /* Pan */
  case K_cUP:
  case K_UP:
    if (c == K_cUP)
      iMglPlotPanY(ih, -10.0);
    else
      iMglPlotPanY(ih, -1.0);
    break;
  case K_cDOWN:
  case K_DOWN:
    if (c == K_cDOWN)
      iMglPlotPanY(ih, 10.0);
    else
      iMglPlotPanY(ih, 1.0);
    break;
  case K_cLEFT:
  case K_LEFT:
    if (c == K_cLEFT)
      iMglPlotPanX(ih, 10.0);
    else
      iMglPlotPanX(ih, 1.0);
    break;
  case K_cRIGHT:
  case K_RIGHT:
    if (c == K_cRIGHT)
      iMglPlotPanX(ih, -10.0);
    else
      iMglPlotPanX(ih, -1.0);
    break;
  /* Zoom */
  case K_plus:
    iMglPlotZoom(ih, 10.0);
    break;
  case K_minus:
    iMglPlotZoom(ih, -10.0);
    break;
  /* Rotation */
  case K_A: case K_a:
    iMglPlotRotate(ih->data->rotY, +1.0);     // 1 degree
    break;
  case K_D: case K_d:
    iMglPlotRotate(ih->data->rotY, -1.0);
    break;
  case K_W: case K_w:
    iMglPlotRotate(ih->data->rotX, +1.0);
    break;
  case K_S: case K_s:
    iMglPlotRotate(ih->data->rotX, -1.0);
    break;
  case K_E: case K_e:
    iMglPlotRotate(ih->data->rotZ, +1.0);
    break;
  case K_Q: case K_q:
    iMglPlotRotate(ih->data->rotZ, -1.0);
    break;
  default:
    return IUP_DEFAULT;
  }

  iMglPlotRepaint(ih, 1, 1);
  return IUP_DEFAULT;
} 

/******************************************************************************
  Class Methods
******************************************************************************/

static void iMglPlotDestroyMethod(Ihandle* ih)
{
  int i;
  
  /* PLOT End for the current stream */
  for(i = 0; i < ih->data->dataSetCount; i++)
    iMglPlotRemoveDataSet(&ih->data->dataSet[i]);

  delete ih->data->mgl;
}

static int iMglPlotCreateMethod(Ihandle* ih, void **params)
{
  (void)params;

  /* free the data allocated by IupCanvas */
  free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  ih->data->dataSetMaxCount = 5;
  ih->data->dataSet = (IdataSet*)malloc(sizeof(IdataSet)*ih->data->dataSetMaxCount);
  memset(ih->data->dataSet, 0, sizeof(IdataSet)*ih->data->dataSetMaxCount);
  ih->data->dataSetCurrent = -1;

  /* IupCanvas callbacks */
  IupSetCallback(ih, "RESIZE_CB",   (Icallback)iMglPlotResize_CB);
  IupSetCallback(ih, "ACTION",      (Icallback)iMglPlotRedraw_CB);
  IupSetCallback(ih, "BUTTON_CB",   (Icallback)iMglPlotMouseButton_CB);
  IupSetCallback(ih, "WHEEL_CB",    (Icallback)iMglPlotWheel_CB);
  IupSetCallback(ih, "KEYPRESS_CB", (Icallback)iMglPlotKeyPress_CB);
  IupSetCallback(ih, "MOTION_CB",   (Icallback)iMglPlotMouseMove_CB);

  IupSetAttribute(ih, "BUFFER", "DOUBLE");

  ih->data->redraw = true;
  ih->data->w = 1;
  ih->data->h = 1;
  ih->data->mgl = new mglGraph(0, ih->data->w, ih->data->h);

  // Default values
  iMglPlotReset(ih);

  return IUP_NOERROR;
}

static Iclass* iMglPlotNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glcanvas"));

  ic->name = "mglplot";
  ic->format = NULL;  /* none */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New     = iMglPlotNewClass;
  ic->Create  = iMglPlotCreateMethod;
  ic->Destroy = iMglPlotDestroyMethod;

   /* IupPPlot Callbacks */
   iupClassRegisterCallback(ic, "POSTDRAW_CB", "");
   iupClassRegisterCallback(ic, "PREDRAW_CB", "");
  // TODO
//   iupClassRegisterCallback(ic, "DELETE_CB", "iiff");
//   iupClassRegisterCallback(ic, "DELETEBEGIN_CB", "");
//   iupClassRegisterCallback(ic, "DELETEEND_CB", "");
//   iupClassRegisterCallback(ic, "SELECT_CB", "iiffi");
//   iupClassRegisterCallback(ic, "SELECTBEGIN_CB", "");
//   iupClassRegisterCallback(ic, "SELECTEND_CB", "");
//   iupClassRegisterCallback(ic, "EDIT_CB", "iiffff");
//   iupClassRegisterCallback(ic, "EDITBEGIN_CB", "");
//   iupClassRegisterCallback(ic, "EDITEND_CB", "");

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", iMglPlotGetBGColorAttrib, iMglPlotSetBGColorAttrib, "255 255 255", NULL, IUPAF_NOT_MAPPED);   /* overwrite canvas implementation, set a system default to force a new default */
  iupClassRegisterAttribute(ic, "FGCOLOR", iMglPlotGetFGColorAttrib, iMglPlotSetFGColorAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED);

  /* IupMglPlot only */
  iupClassRegisterAttribute(ic, "REDRAW", NULL, iMglPlotSetRedrawAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ALPHA", iMglPlotGetAlphaAttrib, iMglPlotSetAlphaAttrib, IUPAF_SAMEASSYSTEM, "0.5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TRANSPARENT", iMglPlotGetTransparentAttrib, iMglPlotSetTransparentAttrib, IUPAF_SAMEASSYSTEM, "No", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "OPENGL", iMglPlotGetOpenGLAttrib, iMglPlotSetOpenGLAttrib, IUPAF_SAMEASSYSTEM, "No", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ANTIALIAS", iMglPlotGetAntialiasAttrib, iMglPlotSetAntialiasAttrib, "Yes", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RESET", NULL, iMglPlotSetResetAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ERRORMESSAGE", iMglPlotGetErrorMessageAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TITLE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLECOLOR", iMglPlotGetTitleColorAttrib, iMglPlotSetTitleColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "TITLEFONTSIZE", iMglPlotGetTitleFontSizeAttrib, iMglPlotSetTitleFontSizeAttrib, IUPAF_SAMEASSYSTEM, "1.6", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEFONTSTYLE", iMglPlotGetTitleFontStyleAttrib, iMglPlotSetTitleFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "LEGEND", iMglPlotGetLegendShowAttrib, iMglPlotSetLegendShowAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDSHOW", iMglPlotGetLegendShowAttrib, iMglPlotSetLegendShowAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT); // for IupPPlot compatibility
  iupClassRegisterAttribute(ic, "LEGENDBOX", iMglPlotGetLegendBoxAttrib, iMglPlotSetLegendBoxAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDPOS", iMglPlotGetLegendPosAttrib, iMglPlotSetLegendPosAttrib, IUPAF_SAMEASSYSTEM, "TOPRIGHT", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDFONTSIZE", iMglPlotGetLegendFontSizeAttrib, iMglPlotSetLegendFontSizeAttrib, IUPAF_SAMEASSYSTEM, "0.8", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDFONTSTYLE", iMglPlotGetLegendFontStyleAttrib, iMglPlotSetLegendFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LEGENDCOLOR", iMglPlotGetLegendColorAttrib, iMglPlotSetLegendColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "GRIDLINESTYLE", iMglPlotGetGridLineStyleAttrib, iMglPlotSetGridLineStyleAttrib, IUPAF_SAMEASSYSTEM, "CONTINUOUS", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRIDCOLOR", iMglPlotGetGridColorAttrib, iMglPlotSetGridColorAttrib, IUPAF_SAMEASSYSTEM, "200 200 200", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRID", iMglPlotGetGridAttrib, iMglPlotSetGridAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "REMOVE", NULL, iMglPlotSetRemoveAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLEAR", NULL, iMglPlotSetClearAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COUNT", iMglPlotGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CURRENT", iMglPlotGetCurrentAttrib, iMglPlotSetCurrentAttrib, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DS_LINESTYLE", iMglPlotGetDSLineStyleAttrib, iMglPlotSetDSLineStyleAttrib, IUPAF_SAMEASSYSTEM, "CONTINUOUS", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_LINEWIDTH", iMglPlotGetDSLineWidthAttrib, iMglPlotSetDSLineWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MARKSTYLE", iMglPlotGetDSMarkStyleAttrib, iMglPlotSetDSMarkStyleAttrib, IUPAF_SAMEASSYSTEM, "X", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MARKSIZE", iMglPlotGetDSMarkSizeAttrib, iMglPlotSetDSMarkSizeAttrib, IUPAF_SAMEASSYSTEM, "0.02", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_SHOWMARKS", iMglPlotGetDSShowMarksAttrib, iMglPlotSetDSShowMarksAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_LEGEND", iMglPlotGetDSLegendAttrib, iMglPlotSetDSLegendAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_COLOR", iMglPlotGetDSColorAttrib, iMglPlotSetDSColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_SHOWVALUES", iMglPlotGetDSShowValuesAttrib, iMglPlotSetDSShowValuesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MODE", iMglPlotGetDSModeAttrib, iMglPlotSetDSModeAttrib, IUPAF_SAMEASSYSTEM, "LINE", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "DS_EDIT", iMglPlotGetDSEditAttrib, iMglPlotSetDSEditAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_REMOVE", NULL, iMglPlotSetDSRemoveAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_COUNT", iMglPlotGetDSCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_DIMENSION", iMglPlotGetDSDimAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_REARRANGE", NULL, iMglPlotSetDSRearrangeAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_SPLIT", NULL, iMglPlotSetDSSplitAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DATAGRID", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRADLINESCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXIALCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "3", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONTOURFILLED", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONTOURLABELS", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONTOURCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "7", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DIR", NULL, NULL, IUPAF_SAMEASSYSTEM, "Y", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLOUDLOW", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEX", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEY", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEZ", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEDIR", NULL, NULL, IUPAF_SAMEASSYSTEM, "XYZ", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECTVALUEX", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECTVALUEY", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECTVALUEZ", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECT", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ISOCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "3", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BARWIDTH", NULL, NULL, IUPAF_SAMEASSYSTEM, "0.7", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RADARSHIFT", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PIECHART", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "LIGHT", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COLORSCHEME", NULL, NULL, IUPAF_SAMEASSYSTEM, "BbcyrR", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "COLORBAR", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COLORBARPOS", NULL, NULL, IUPAF_SAMEASSYSTEM, "RIGHT", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COLORBARRANGE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COLORBARAXISTICKS", NULL, NULL, IUPAF_SAMEASSYSTEM, "Z", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BOX", iMglPlotGetBoxAttrib, iMglPlotSetBoxAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BOXTICKS", iMglPlotGetBoxTicksAttrib, iMglPlotSetBoxTicksAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BOXCOLOR", iMglPlotGetBoxColorAttrib, iMglPlotSetBoxColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XLABEL", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLABEL", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZLABEL", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XLABELCENTERED", iMglPlotGetAxisXLabelCenteredAttrib, iMglPlotSetAxisXLabelCenteredAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLABELCENTERED", iMglPlotGetAxisYLabelCenteredAttrib, iMglPlotSetAxisYLabelCenteredAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZLABELCENTERED", iMglPlotGetAxisZLabelCenteredAttrib, iMglPlotSetAxisZLabelCenteredAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XLABELPOSITION", iMglPlotGetAxisXLabelPositionAttrib, iMglPlotSetAxisXLabelPositionAttrib, IUPAF_SAMEASSYSTEM, "CENTER", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLABELPOSITION", iMglPlotGetAxisYLabelPositionAttrib, iMglPlotSetAxisYLabelPositionAttrib, IUPAF_SAMEASSYSTEM, "CENTER", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZLABELPOSITION", iMglPlotGetAxisZLabelPositionAttrib, iMglPlotSetAxisZLabelPositionAttrib, IUPAF_SAMEASSYSTEM, "CENTER", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XLABELROTATION", iMglPlotGetAxisXLabelRotationAttrib, iMglPlotSetAxisXLabelRotationAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YLABELROTATION", iMglPlotGetAxisYLabelRotationAttrib, iMglPlotSetAxisYLabelRotationAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZLABELROTATION", iMglPlotGetAxisZLabelRotationAttrib, iMglPlotSetAxisZLabelRotationAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XCOLOR", iMglPlotGetAxisXColorAttrib, iMglPlotSetAxisXColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YCOLOR", iMglPlotGetAxisYColorAttrib, iMglPlotSetAxisYColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZCOLOR", iMglPlotGetAxisZColorAttrib, iMglPlotSetAxisZColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XAUTOMIN", iMglPlotGetAxisXAutoMinAttrib, iMglPlotSetAxisXAutoMinAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YAUTOMIN", iMglPlotGetAxisYAutoMinAttrib, iMglPlotSetAxisYAutoMinAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZAUTOMIN", iMglPlotGetAxisZAutoMinAttrib, iMglPlotSetAxisZAutoMinAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XAUTOMAX", iMglPlotGetAxisXAutoMaxAttrib, iMglPlotSetAxisXAutoMaxAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YAUTOMAX", iMglPlotGetAxisYAutoMaxAttrib, iMglPlotSetAxisYAutoMaxAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZAUTOMAX", iMglPlotGetAxisZAutoMaxAttrib, iMglPlotSetAxisZAutoMaxAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XMIN", iMglPlotGetAxisXMinAttrib, iMglPlotSetAxisXMinAttrib, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YMIN", iMglPlotGetAxisYMinAttrib, iMglPlotSetAxisYMinAttrib, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZMIN", iMglPlotGetAxisZMinAttrib, iMglPlotSetAxisZMinAttrib, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XMAX", iMglPlotGetAxisXMaxAttrib, iMglPlotSetAxisXMaxAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YMAX", iMglPlotGetAxisYMaxAttrib, iMglPlotSetAxisYMaxAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZMAX", iMglPlotGetAxisZMaxAttrib, iMglPlotSetAxisZMaxAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XREVERSE", iMglPlotGetAxisXReverseAttrib, iMglPlotSetAxisXReverseAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YREVERSE", iMglPlotGetAxisYReverseAttrib, iMglPlotSetAxisYReverseAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZREVERSE", iMglPlotGetAxisZReverseAttrib, iMglPlotSetAxisZReverseAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_X", iMglPlotGetAxisXShowAttrib, iMglPlotSetAxisXShowAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_Y", iMglPlotGetAxisYShowAttrib, iMglPlotSetAxisYShowAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_Z", iMglPlotGetAxisZShowAttrib, iMglPlotSetAxisZShowAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XARROW", iMglPlotGetAxisXShowArrowAttrib, iMglPlotSetAxisXShowArrowAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YARROW", iMglPlotGetAxisYShowArrowAttrib, iMglPlotSetAxisYShowArrowAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZARROW", iMglPlotGetAxisZShowArrowAttrib, iMglPlotSetAxisZShowArrowAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XCROSSORIGIN", iMglPlotGetAxisXCrossOriginAttrib, iMglPlotSetAxisXCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YCROSSORIGIN", iMglPlotGetAxisYCrossOriginAttrib, iMglPlotSetAxisYCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZCROSSORIGIN", iMglPlotGetAxisZCrossOriginAttrib, iMglPlotSetAxisZCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XORIGIN", iMglPlotGetAxisXOriginAttrib, iMglPlotSetAxisXOriginAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YORIGIN", iMglPlotGetAxisYOriginAttrib, iMglPlotSetAxisYOriginAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZORIGIN", iMglPlotGetAxisZOriginAttrib, iMglPlotSetAxisZOriginAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XSCALE", iMglPlotGetAxisXScaleAttrib, iMglPlotSetAxisXScaleAttrib, IUPAF_SAMEASSYSTEM, "LIN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YSCALE", iMglPlotGetAxisYScaleAttrib, iMglPlotSetAxisYScaleAttrib, IUPAF_SAMEASSYSTEM, "LIN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZSCALE", iMglPlotGetAxisZScaleAttrib, iMglPlotSetAxisZScaleAttrib, IUPAF_SAMEASSYSTEM, "LIN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XFONTSIZE", iMglPlotGetAxisXFontSizeAttrib, iMglPlotSetAxisXFontSizeAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YFONTSIZE", iMglPlotGetAxisYFontSizeAttrib, iMglPlotSetAxisYFontSizeAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YFONTSIZE", iMglPlotGetAxisZFontSizeAttrib, iMglPlotSetAxisZFontSizeAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XFONTSTYLE", iMglPlotGetAxisXFontStyleAttrib, iMglPlotSetAxisXFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YFONTSTYLE", iMglPlotGetAxisYFontStyleAttrib, iMglPlotSetAxisYFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZFONTSTYLE", iMglPlotGetAxisZFontStyleAttrib, iMglPlotSetAxisZFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "AXS_XTICK", iMglPlotGetAxisXTickAttrib, iMglPlotSetAxisXTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICK", iMglPlotGetAxisYTickAttrib, iMglPlotSetAxisYTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICK", iMglPlotGetAxisZTickAttrib, iMglPlotSetAxisZTickAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKVALUES", iMglPlotGetAxisXTickShowValuesAttrib, iMglPlotSetAxisXTickShowValuesAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKVALUES", iMglPlotGetAxisYTickShowValuesAttrib, iMglPlotSetAxisYTickShowValuesAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKVALUES", iMglPlotGetAxisZTickShowValuesAttrib, iMglPlotSetAxisZTickShowValuesAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKVALUESROTATION", iMglPlotGetAxisXTickValuesRotationAttrib, iMglPlotSetAxisXTickValuesRotationAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKVALUESROTATION", iMglPlotGetAxisYTickValuesRotationAttrib, iMglPlotSetAxisYTickValuesRotationAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKVALUESROTATION", iMglPlotGetAxisZTickValuesRotationAttrib, iMglPlotSetAxisZTickValuesRotationAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKFORMAT", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKFORMAT", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKFORMAT", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKFONTSIZE", iMglPlotGetAxisXTickFontSizeAttrib, iMglPlotSetAxisXTickFontSizeAttrib, IUPAF_SAMEASSYSTEM, "0.8", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKFONTSIZE", iMglPlotGetAxisYTickFontSizeAttrib, iMglPlotSetAxisYTickFontSizeAttrib, IUPAF_SAMEASSYSTEM, "0.8", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKFONTSIZE", iMglPlotGetAxisZTickFontSizeAttrib, iMglPlotSetAxisZTickFontSizeAttrib, IUPAF_SAMEASSYSTEM, "0.8", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKFONTSTYLE", iMglPlotGetAxisXTickFontStyleAttrib, iMglPlotSetAxisXTickFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKFONTSTYLE", iMglPlotGetAxisYTickFontStyleAttrib, iMglPlotSetAxisYTickFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKFONTSTYLE", iMglPlotGetAxisZTickFontStyleAttrib, iMglPlotSetAxisZTickFontStyleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XAUTOTICK", iMglPlotGetAxisXTickAutoAttrib, iMglPlotSetAxisXTickAutoAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YAUTOTICK", iMglPlotGetAxisYTickAutoAttrib, iMglPlotSetAxisYTickAutoAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZAUTOTICK", iMglPlotGetAxisZTickAutoAttrib, iMglPlotSetAxisZTickAutoAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKAUTO", iMglPlotGetAxisXTickAutoAttrib, iMglPlotSetAxisXTickAutoAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKAUTO", iMglPlotGetAxisYTickAutoAttrib, iMglPlotSetAxisYTickAutoAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKAUTO", iMglPlotGetAxisZTickAutoAttrib, iMglPlotSetAxisZTickAutoAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMAJORSPAN", iMglPlotGetAxisXTickMajorSpanAttrib, iMglPlotSetAxisXTickMajorSpanAttrib, IUPAF_SAMEASSYSTEM, "-5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMAJORSPAN", iMglPlotGetAxisYTickMajorSpanAttrib, iMglPlotSetAxisYTickMajorSpanAttrib, IUPAF_SAMEASSYSTEM, "-5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKMAJORSPAN", iMglPlotGetAxisZTickMajorSpanAttrib, iMglPlotSetAxisZTickMajorSpanAttrib, IUPAF_SAMEASSYSTEM, "-5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKDIVISION", iMglPlotGetAxisXTickMinorDivisionAttrib, iMglPlotSetAxisXTickMinorDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKDIVISION", iMglPlotGetAxisYTickMinorDivisionAttrib, iMglPlotSetAxisYTickMinorDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKDIVISION", iMglPlotGetAxisZTickMinorDivisionAttrib, iMglPlotSetAxisZTickMinorDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMINORDIVISION", iMglPlotGetAxisXTickMinorDivisionAttrib, iMglPlotSetAxisXTickMinorDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMINORDIVISION", iMglPlotGetAxisYTickMinorDivisionAttrib, iMglPlotSetAxisYTickMinorDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKMINORDIVISION", iMglPlotGetAxisZTickMinorDivisionAttrib, iMglPlotSetAxisZTickMinorDivisionAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XAUTOTICKSIZE", iMglPlotGetAxisXTickAutoSizeAttrib, iMglPlotSetAxisXTickAutoSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YAUTOTICKSIZE", iMglPlotGetAxisYTickAutoSizeAttrib, iMglPlotSetAxisYTickAutoSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZAUTOTICKSIZE", iMglPlotGetAxisZTickAutoSizeAttrib, iMglPlotSetAxisZTickAutoSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKAUTOSIZE", iMglPlotGetAxisXTickAutoSizeAttrib, iMglPlotSetAxisXTickAutoSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKAUTOSIZE", iMglPlotGetAxisYTickAutoSizeAttrib, iMglPlotSetAxisYTickAutoSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKAUTOSIZE", iMglPlotGetAxisZTickAutoSizeAttrib, iMglPlotSetAxisZTickAutoSizeAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMAJORSIZE", iMglPlotGetAxisXTickMajorSizeAttrib, iMglPlotSetAxisXTickMajorSizeAttrib, IUPAF_SAMEASSYSTEM, "0.1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMAJORSIZE", iMglPlotGetAxisYTickMajorSizeAttrib, iMglPlotSetAxisYTickMajorSizeAttrib, IUPAF_SAMEASSYSTEM, "0.1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKMAJORSIZE", iMglPlotGetAxisZTickMajorSizeAttrib, iMglPlotSetAxisZTickMajorSizeAttrib, IUPAF_SAMEASSYSTEM, "0.1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_XTICKMINORSIZE", iMglPlotGetAxisXTickMinorSizeAttrib, iMglPlotSetAxisXTickMinorSizeAttrib, IUPAF_SAMEASSYSTEM, "0.6", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YTICKMINORSIZE", iMglPlotGetAxisYTickMinorSizeAttrib, iMglPlotSetAxisYTickMinorSizeAttrib, IUPAF_SAMEASSYSTEM, "0.6", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZTICKMINORSIZE", iMglPlotGetAxisZTickMinorSizeAttrib, iMglPlotSetAxisZTickMinorSizeAttrib, IUPAF_SAMEASSYSTEM, "0.6", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DRAWCOLOR", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWMARKSTYLE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWLINESTYLE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWLINEWIDTH", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWFONTSTYLE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWFONTSIZE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "ZOOM", iMglPlotGetZoomAttrib, iMglPlotSetZoomAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ROTATE", iMglPlotGetRotateAttrib, iMglPlotSetRotateAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupMglPlot(void)
{
  return IupCreate("mglplot");
}

void IupMglPlotOpen(void)
{
  IupGLCanvasOpen();

  if (!IupGetGlobal("_IUP_MGLPLOT_OPEN"))
  {
    iupRegisterClass(iMglPlotNewClass());
    IupSetGlobal("_IUP_MGLPLOT_OPEN", "1");
  }
}

/************************  TODO   ***********************************

1.11:
- Text style and text color positions are swapped. I.e. text style ‘r:C’ give red centered text, but not roman dark cyan text as for v.1.*.

UTF-8
SubPlots

New PPlot:
  IupPPlotGetSample IupPPlotGetSampleStr
  iupClassRegisterAttribute(ic, "SYNCVIEW", iPPlotGetSyncViewAttrib, iPPlotSetSyncViewAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  IupPPlotTransformTo missing from DLLs and from Lua.
  New: attributes REMOVE and CURRENT in IupPPlot now also accepts the DS_NAME as value when setting.
  New: PLOT_COUNT, PLOT_NUMCOL, PLOT_CURRENT, PLOT_INSERT and PLOT_REMOVE attributes for IupPPlot to support multiple plots in the same display area.
  New: PLOTBUTTON_CB and PLOTMOTION_CB calbacks for IupPPlot.

DS_EDIT+Selection+Callbacks

MathGL:
  evaluate interval, [-1,1] x [0,1] x [0,n-1]
  ***improve autoticks computation
  ***Legend does not work in OpenGL
  ***Legend background is always white regardless of plot background
  ***gr->Box AND gr->ContFA have different results in OpenGL. 
     It seems to have an invalid depth. Without OpenGL works fine.
     SOLVED BY CALLING glEnable(GL_DEPTH_TEST), but this affected anti-aliasing.
  gr->Axial is changing something that affects other graphs in OpenGL. Without OpenGL works fine.
  ***graph disapear during zoom in, only in OpenGL, depth clipping?
  ***bars at 0 and n-1
  ***Axis color is fixed in black
  Ticks
     SetTickLen - documentation says negative len puts ticks outside the bounding box, but it is NOT working
     ***TicksVal should follow ticks spacing configuration 
  Fonts
     ***font aspect ratio not being mantained in OpenGL
     option or function to draw an opaque background for text
     BOLD and ITALIC inside TeX formatting does not work for TTF or OTF fonts.
  Light and Fog

Maybe:
  curvilinear coordinates
  plots that need two datasets: BoxPlot, Region, Tens, Mark, Error, Flow, Pipe, Ring
     chart and bars can be combined in one plot (bars then can include above and fall)
  reference datasets
     dataset can be a pointer to the previous data, 
     so the same data can be displayed using different modes using the same memory
  Ternary
  IupMglPlotDrawCurve and IupMglPlotDrawFace

**************************************************************************************/
