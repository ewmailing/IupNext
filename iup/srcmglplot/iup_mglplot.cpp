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
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_array.h"

#include "mgl/mgl.h"
#include "mgl/mgl_ab.h"
#include "mgl/mgl_gl.h"
#include "mgl/mgl_eps.h"
#include "mgl/mgl_zb.h"
#include "mgl/mgl_eval.h"


enum {IUP_MGLPLOT_BOTTOMLEFT, IUP_MGLPLOT_BOTTOMRIGHT, IUP_MGLPLOT_TOPLEFT, IUP_MGLPLOT_TOPRIGHT};
enum {IUP_MGLPLOT_INHERIT, IUP_MGLPLOT_PLAIN, IUP_MGLPLOT_BOLD, IUP_MGLPLOT_ITALIC, IUP_MGLPLOT_BOLD_ITALIC};

//TODO dataset can be a pointer to the previous data, 
//so the same data can be displayed using diferent modes using the same memory
typedef struct _IdataSet
{
  char dsLineStyle;
  float dsLineWidth;
  char dsMarkStyle;
  float dsMarkSize;
  bool dsShowValues;
  char* dsMode;
  char* dsLegend;
  mglColor dsColor;

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
  float axOrigin;
  const char* axScale;
  bool axShow, axShowArrow;

  int axLabelPos;
  int axLabelFontStyle;
  float axLabelFontSizeFactor;
  bool axLabelRotation;

  int axTickFontStyle;
  float axTickFontSizeFactor;
  bool axTickShowValues, axTickShow, 
       axTickAutoSpace, axTickAutoSize;
  float axTickMinorSizeFactor, axTickMajorSize;          
  int axTickMinorDivision;
  float axTickMajorSpan;
  bool axTickValuesRotation;

  bool axReverse;
  float axMax, axMin;
  bool axAutoScaleMax, axAutoScaleMin;
} Iaxis;

struct _IcontrolData
{
  iupCanvas canvas;     /* from IupCanvas (must reserve it)  */

  mglGraphGL* mgl;

  int w, h;
  float dpi;
  bool redraw, alpha;
  mglColor bgColor, fgColor;
  char FontDef[32];     /* aux, obtained from FONT */
  float FontSize;

  /* Title */
  mglColor titleColor;
  int titleFontStyle;
  float titleFontSizeFactor;

  /* Axes */
  Iaxis xAxis, yAxis, zAxis;

  /* Box */
  bool Box, boxTicks;
  mglColor boxColor;

  /* Legend */
  bool legendShow, legendBox;
  mglColor legendColor;
  int legendPosition;
  int legendFontStyle;
  float legendFontSizeFactor;

  /* Grid */
  mglColor gridColor;
  const char* gridShow;
  char gridLineStyle;

  /* Dataset */
  int dataSetCurrent;
  int dataSetCount, dataSetMaxCount;
  IdataSet* dataSet;
};

/* PPlot function pointer typedefs. */
// TODO
// typedef int (*IFniiff)(Ihandle*, int, int, float, float); /* delete_cb */
// typedef int (*IFniiffi)(Ihandle*, int, int, float, float, int); /* select_cb */
// typedef int (*IFniiffff)(Ihandle*, int, int, float, float, float*, float*); /* edit_cb */


/******************************************************************************
 Useful Functions
******************************************************************************/


// Quantize 0-1 values into 0-255.
inline unsigned char iQuant(const float& value)
{
  if (value >= 1.0f) return 255;
  if (value <= 0.0f) return 0;
  return (unsigned char)(value*256.0f);
}                               

// Reconstruct 0-255 values into 0-1.
inline float iRecon(const unsigned char& value)
{
  if (value <= 0) return 0.0f;
  if (value >= 255) return 1.0f;
  return ((float)value + 0.5f)/256.0f;
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

static bool iMglPlotIsGraph3D(Ihandle* ih)
{
  int i;
  for (i=0; i< ih->data->dataSetCount; i++)
  {
    IdataSet* ds = &ih->data->dataSet[i];
    if ((ds->dsDim == 1 && (ds->dsX->ny>1 || ds->dsX->nz>1)) ||  // Planar or Volumetric
        ds->dsDim == 3)  // 3D data
      return true;
  }

  return false;
}

static char* iMglPlotGetFontName(const char* typeface)
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
  
  const char* font = iupAttribGetStr(ih, "MGLFONT");
  if (!font)
    font = iupGetFontAttrib(ih);

  if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return;

  if (is_bold && is_italic)
  {
    ih->data->FontDef[i++] = 'b';
    ih->data->FontDef[i++] = 'i';
  }
  else if (is_bold)
    ih->data->FontDef[i++] = 'b';
  else if (is_italic)
    ih->data->FontDef[i++] = 'i';
  else
    ih->data->FontDef[i++] = 'r';

  if (is_underline)
    ih->data->FontDef[i++] = 'u';

  if (is_strikeout)
    ih->data->FontDef[i++] = 'o';

  ih->data->FontDef[i] = 0;

  if (size < 0) 
    size = -size;
  else 
    size = (int)((size*ih->data->dpi)/72.0f);   //from points to pixels

  ih->data->FontSize = ((float)size/(float)ih->data->h)*90;   //TODO Magic factor for acceptable size 

  const char* name = iMglPlotGetFontName(typeface);
  if (!name)
    name = typeface;

  char *path = getenv("IUP_MGLFONTS");
  if (!path) 
    path = IupGetGlobal("MGLFONTS");

  if (!iupStrEqualNoCase(name, iupAttribGetStr(ih, "_IUP_MGL_FONTNAME")) ||
      !iupStrEqualNoCase(path, iupAttribGetStr(ih, "_IUP_MGL_FONTPATH")))
  {
    gr->LoadFont(name, path);

    iupAttribStoreStr(ih, "_IUP_MGL_FONTNAME", name);
    iupAttribStoreStr(ih, "_IUP_MGL_FONTPATH", path);
  }
}

static void iMglPlotConfigFont(Ihandle* ih, mglGraph *gr, int fontstyle, float fontsizefactor)
{
  int i=0;
  char fnt[32];

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

  gr->SetFontSize(fontsizefactor*ih->data->FontSize);
}

static void iMglPlotConfigColor(Ihandle* ih, mglGraph *gr, mglColor color)
{
  if (color.r == NAN || color.g == NAN || color.b == NAN)
    color = ih->data->fgColor;

  gr->DefColor(color, 1);
}

static char* iMglPlotConfigPen(mglGraph *gr, char* pen, char line_style, float line_width)
{
  *pen++ = line_style;

  gr->SetBaseLineWidth(line_width);
  *pen++ = '1'; // 1*BaseLineWidth

  *pen = 0;
  return pen;
}

static void iMglPlotConfigDataSetLineStyle(IdataSet* ds, mglGraph *gr, char* style, int mark)
{
  style = iMglPlotConfigPen(gr, style, ds->dsLineStyle, ds->dsLineWidth);

  if (mark)
  {
    if (ds->dsMarkStyle == 'O' || ds->dsMarkStyle == 'S' || ds->dsMarkStyle == 'D')
    {
      *style++ = '#';
      *style++ = (char)tolower(ds->dsMarkStyle);
    }
    else
      *style++ = ds->dsMarkStyle;

    gr->SetMarkSize(ds->dsMarkSize);
  }

  *style = 0;
}

static void iMglPlotSetFunc(mglFormula* *func, const char* scale)
{
  if(*func) delete *func;
  *func = new mglFormula(scale);
}

static void iMglPlotConfigAxisTicks(Ihandle* ih, mglGraph *gr, char dir, Iaxis& axis, float min, float max)
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
      iMglPlotSetFunc(&(gr->fx), axis.axScale);
    if (dir == 'y')
      iMglPlotSetFunc(&(gr->fy), axis.axScale);
    if (dir == 'z')
      iMglPlotSetFunc(&(gr->fz), axis.axScale);

    gr->SetTicks(dir, 0, 0);  // Disable normal ticks, logarithm ticks will be done
  }
  else  
  {
    // step for axis ticks (if positive) or 
    // it’s number on the axis range (if negative).
    float step = -5;
    int numsub = 0;  // Number of axis submarks

    if (axis.axTickAutoSpace)
    {
      // Copied from MathGL AdjustTicks internal function
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
  attrib[4] = (char)toupper(dir);
  char* format = iupAttribGetStr(ih, attrib);
  if (format && axis.axTickShowValues)
  {
    if (dir == 'x') gr->SetXTT(format);
    if (dir == 'y') gr->SetYTT(format);
    if (dir == 'z') gr->SetZTT(format);
  }
}

static void iMglPlotConfigScale(Iaxis& axis, float& min, float& max)
{
  if (!axis.axAutoScaleMax)
    max = axis.axMax;
  else
    axis.axMax = max;  // Update the attribute value
  if (!axis.axAutoScaleMin)
    min = axis.axMin;
  else
    axis.axMin = min;  // Update the attribute value

  if (axis.axReverse)
  {
    float t = max;
    max = min;
    min = t;
  }
}

static void iMglPlotFindMinMaxValues(mglData& ds_data, bool add, float& min, float& max)
{
  int i, count = ds_data.nx*ds_data.ny*ds_data.nz;
  float* data = ds_data.a;

  for (i = 1; i < count; i++, data++)
  {
    if (*data != *data) // test if NAN
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

  if (ih->data->xAxis.axAutoScaleMax || ih->data->xAxis.axAutoScaleMin ||
      ih->data->yAxis.axAutoScaleMax || ih->data->yAxis.axAutoScaleMin ||
      ih->data->zAxis.axAutoScaleMax || ih->data->zAxis.axAutoScaleMin)
  {
    for(i = 0; i < ih->data->dataSetCount; i++)
    {
      IdataSet* ds = &ih->data->dataSet[i];

      if (ds->dsDim == 1)
      {
        /* the data will be plotted as Y, X will be 0,1,2,3,... */
        if (ih->data->xAxis.axAutoScaleMax || ih->data->xAxis.axAutoScaleMin)
          gr->SetAutoRanges(0, (mreal)(ds->dsCount-1), 0, 0, 0, 0);
        if (ih->data->yAxis.axAutoScaleMax || ih->data->yAxis.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsX, i==0? false: true, gr->Min.y, gr->Max.y);  
      }
      else if (ds->dsDim == 2)
      {
        if (ih->data->xAxis.axAutoScaleMax || ih->data->xAxis.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsX, i==0? false: true, gr->Min.x, gr->Max.x);
        if (ih->data->yAxis.axAutoScaleMax || ih->data->yAxis.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsY, i==0? false: true, gr->Min.y, gr->Max.y);
      }
      else if (ds->dsDim == 3)
      {
        if (ih->data->xAxis.axAutoScaleMax || ih->data->xAxis.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsX, i==0? false: true, gr->Min.x, gr->Max.x);
        if (ih->data->yAxis.axAutoScaleMax || ih->data->yAxis.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsY, i==0? false: true, gr->Min.y, gr->Max.y);
        if (ih->data->zAxis.axAutoScaleMax || ih->data->zAxis.axAutoScaleMin)
          iMglPlotFindMinMaxValues(*ds->dsZ, i==0? false: true, gr->Min.z, gr->Max.z);
      }
    }
  }

  iMglPlotConfigScale(ih->data->xAxis, gr->Min.x, gr->Max.x);
  iMglPlotConfigScale(ih->data->yAxis, gr->Min.y, gr->Max.y);
  iMglPlotConfigScale(ih->data->zAxis, gr->Min.z, gr->Max.z);

  iMglPlotConfigAxisTicks(ih, gr, 'x', ih->data->xAxis, gr->Min.x, gr->Max.x);
  iMglPlotConfigAxisTicks(ih, gr, 'y', ih->data->yAxis, gr->Min.y, gr->Max.y);
  iMglPlotConfigAxisTicks(ih, gr, 'z', ih->data->zAxis, gr->Min.z, gr->Max.z);

  gr->SetOrigin(ih->data->xAxis.axOrigin, ih->data->yAxis.axOrigin, ih->data->zAxis.axOrigin);

  gr->RecalcBorder();
}

static void iMglPlotDrawAxisLabel(Ihandle* ih, mglGraph *gr, char dir, Iaxis& axis)
{
  char attrib[11] = "AXS_?LABEL";
  attrib[4] = (char)toupper(dir);
  char* label = iupAttribGetStr(ih, attrib);
  if (label)
  {
    // TODO Rotation is working only for Y axis
    gr->SetRotatedText(axis.axLabelRotation);
    iMglPlotConfigFont(ih, gr, axis.axLabelFontStyle, axis.axLabelFontSizeFactor);
    // TODO somethimes the label gets too close to the ticks
    gr->Label(dir, label, (mreal)axis.axLabelPos, -1);  
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
      mreal *val = new mreal[count];
      for(int i=0; i< count; i++)
        val[i] = (mreal)i;
      gr->SetTicksVal('x', count, val, (const char**)dsNames);
      delete val;
    }
    else
      gr->SetTicksVal('x', 0, (mreal*)NULL, (const char **)NULL);
  }
}

static void iMglPlotConfigAxisTicksLen(mglGraph *gr, Iaxis& axis)
{
  if (!axis.axTickAutoSize)
  {
    //TODO: documentation says negative len puts tic outside the bounding box, 
    //      but it is NOT working
    mreal stt = 1.0f/(axis.axTickMinorSizeFactor*axis.axTickMinorSizeFactor) - 1.0f;
    gr->SetTickLen(axis.axTickMajorSize, stt);
  }
  else
  {
    mreal stt = 1.0f/(0.6f*0.6f) - 1.0f;
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
  // TODO Rotation is working only for Y axis
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
}

static void iMglPlotDrawAxes(Ihandle* ih, mglGraph *gr)
{
  // Draw axes lines, ticks and ticks labels
  if(ih->data->xAxis.axShow)  
    iMglPlotDrawAxis(ih, gr, 'x', ih->data->xAxis);
  if(ih->data->yAxis.axShow)  
    iMglPlotDrawAxis(ih, gr, 'y', ih->data->yAxis);
  if(ih->data->zAxis.axShow && iMglPlotIsGraph3D(ih))
    iMglPlotDrawAxis(ih, gr, 'z', ih->data->zAxis);

  // Reset to default
  gr->SetFunc(NULL, NULL, NULL);
}

static void iMglPlotDrawGrid(Ihandle* ih, mglGraph *gr)
{
  char pen[10], grid[10];
  iMglPlotConfigColor(ih, gr, ih->data->gridColor);
  iMglPlotConfigPen(gr, pen, ih->data->gridLineStyle, 1);
  iupStrLower(grid, ih->data->gridShow);
  gr->Grid(grid, pen);
}

static char* iMglPlotMakeFormatString(float inValue, float range) 
{
  if (inValue<0)
    inValue = range/(-inValue);

  if (inValue > (float)1.0e4 || inValue < (float)1.0e-3)
    return "%.1e";
  else 
  {
    int thePrecision = 0;
    if (inValue<1) 
    {
      float theSpan = inValue;
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
    default: return "%.0f";
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
  if (!xformat) xformat = iMglPlotMakeFormatString(gr->dx, gr->Max.x-gr->Min.x);
  if (!yformat) yformat = iMglPlotMakeFormatString(gr->dy, gr->Max.y-gr->Min.y);
  if (!zformat) zformat = iMglPlotMakeFormatString(gr->dz, gr->Max.z-gr->Min.z);

  iMglPlotConfigColor(ih, gr, ds->dsColor);
  iMglPlotConfigFont(ih, gr, ih->data->legendFontStyle, ih->data->legendFontSizeFactor);

  float* dsXPoints = ds->dsX->a;
  if (ds->dsDim == 3)
  {
    float* dsYPoints = ds->dsY->a;
    float* dsZPoints = ds->dsZ->a;
    mglPoint d = mglPoint(gr->Max.x-gr->Min.x, gr->Max.y-gr->Min.y, gr->Max.z-gr->Min.z);

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
    float* dsYPoints = ds->dsY->a;
    mglPoint d = mglPoint(gr->Max.x-gr->Min.x, gr->Max.y-gr->Min.y);

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
    mglPoint d = mglPoint(gr->Max.x-gr->Min.x, gr->Max.y-gr->Min.y);

    for(i = 0; i < ds->dsCount; i++)
    {
      sprintf(text, format, i, dsXPoints[i]);
      p = mglPoint((float)i, dsXPoints[i]);
      gr->Puts(p, d, text, 0, -1);
    }
  }
}

static float iMglPlotGetAttribFloatNAN(Ihandle* ih, const char* name)
{
  float val = NAN;
  iupStrToFloat(iupAttribGet(ih, name), &val);
  return val;
}

static void iMglPlotDrawVolumetricData(Ihandle* ih, mglGraph *gr, IdataSet* ds)
{               
  char style[64];
  char* value;

  //TODO
  //gr->Light(true);
  //gr->Light(1,mglPoint(0,1,0),’c’);
  //gr->Rotate(40, 60);
  //gr->Fog(1);

  if (iupStrEqualNoCase(ds->dsMode, "VOLUME_ISOSURFACE"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))   // Default false
      { style[0] = '#'; style[1] = 0; }        // Here means wire
    else
      style[0] = 0;

    value = iupAttribGet(ih, "ISOVALUE");
    if (value)
    {
      float isovalue;
      if (iupStrToFloat(value, &isovalue))
        gr->Surf3(isovalue, *ds->dsX, style);   // only 1 isosurface
    }
    else
    {
      int isocount = iupAttribGetInt(ih, "ISOCOUNT");  //Default 3
      gr->Surf3(*ds->dsX, style, isocount);
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_DENSITY"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))  // Default false
      { style[0] = '#'; style[1] = 0; }
    else
      style[0] = 0;

    char* slicedir = iupAttribGetStr(ih, "SLICEDIR"); //Default "XYZ"
    int project = iupAttribGetBoolean(ih, "PROJECT");  //Default false
    if (project)
    {
      float valx = iMglPlotGetAttribFloatNAN(ih, "PROJECTVALUEX");
      float valy = iMglPlotGetAttribFloatNAN(ih, "PROJECTVALUEY");
      float valz = iMglPlotGetAttribFloatNAN(ih, "PROJECTVALUEZ");
      if (tolower(*slicedir)=='x') { gr->DensX(ds->dsX->Sum("x"), style, valx); slicedir++; }
      if (tolower(*slicedir)=='y') { gr->DensY(ds->dsX->Sum("y"), style, valy); slicedir++; }
      if (tolower(*slicedir)=='z') { gr->DensZ(ds->dsX->Sum("z"), style, valz); slicedir++; }
    }
    else
    {
      int slicex = iupAttribGetInt(ih, "SLICEX");  //Default -1 (central)
      int slicey = iupAttribGetInt(ih, "SLICEY");  //Default -1 (central)
      int slicez = iupAttribGetInt(ih, "SLICEZ");  //Default -1 (central)
      if (tolower(*slicedir)=='x') { gr->Dens3(*ds->dsX, 'x', slicex, style); slicedir++; }
      if (tolower(*slicedir)=='y') { gr->Dens3(*ds->dsX, 'y', slicey, style); slicedir++; }
      if (tolower(*slicedir)=='z') { gr->Dens3(*ds->dsX, 'z', slicez, style); slicedir++; }
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_CONTOUR"))
  {
    if (iupAttribGetBoolean(ih, "DATAGRID"))
      { style[0] = '#'; style[1] = 0; }
    else
      style[0] = 0;

    int contourcount = iupAttribGetInt(ih, "CONTOURCOUNT");  //Default 7
    int countourfilled = iupAttribGetBoolean(ih, "CONTOURFILLED");  //Default false
    char* slicedir = iupAttribGetStr(ih, "SLICEDIR"); //Default "XYZ"
    int project = iupAttribGetBoolean(ih, "PROJECT");  //Default false
    if (project)
    {
      float valx = iMglPlotGetAttribFloatNAN(ih, "PROJECTVALUEX");
      float valy = iMglPlotGetAttribFloatNAN(ih, "PROJECTVALUEY");
      float valz = iMglPlotGetAttribFloatNAN(ih, "PROJECTVALUEZ");
      if (countourfilled)
      {
        if (tolower(*slicedir)=='x') { gr->ContFX(ds->dsX->Sum("x"), style, valx, contourcount); slicedir++; }
        if (tolower(*slicedir)=='y') { gr->ContFY(ds->dsX->Sum("y"), style, valy, contourcount); slicedir++; }
        if (tolower(*slicedir)=='z') { gr->ContFZ(ds->dsX->Sum("z"), style, valz, contourcount); slicedir++; }
      }
      else
      {
        if (tolower(*slicedir)=='x') { gr->ContX(ds->dsX->Sum("x"), style, valx, contourcount); slicedir++; }
        if (tolower(*slicedir)=='y') { gr->ContY(ds->dsX->Sum("y"), style, valy, contourcount); slicedir++; }
        if (tolower(*slicedir)=='z') { gr->ContZ(ds->dsX->Sum("z"), style, valz, contourcount); slicedir++; }
      }
    }
    else
    {
      int slicex = iupAttribGetInt(ih, "SLICEX");  //Default -1 (central)
      int slicey = iupAttribGetInt(ih, "SLICEY");  //Default -1 (central)
      int slicez = iupAttribGetInt(ih, "SLICEZ");  //Default -1 (central)

      if (countourfilled)
      {
        if (tolower(*slicedir)=='x') { gr->ContF3(*ds->dsX, 'x', slicex, style, contourcount); slicedir++; }
        if (tolower(*slicedir)=='y') { gr->ContF3(*ds->dsX, 'y', slicey, style, contourcount); slicedir++; }
        if (tolower(*slicedir)=='z') { gr->ContF3(*ds->dsX, 'z', slicez, style, contourcount); slicedir++; }
      }
      else
      {
        if (tolower(*slicedir)=='x') { gr->Cont3(*ds->dsX, 'x', slicex, style, contourcount); slicedir++; }
        if (tolower(*slicedir)=='y') { gr->Cont3(*ds->dsX, 'y', slicey, style, contourcount); slicedir++; }
        if (tolower(*slicedir)=='z') { gr->Cont3(*ds->dsX, 'z', slicez, style, contourcount); slicedir++; }
      }
    }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_GRID"))
  {
    style[0] = 0;

    char* slicedir = iupAttribGetStr(ih, "SLICEDIR"); //Default "XYZ"

    int slicex = iupAttribGetInt(ih, "SLICEX");  //Default -1 (central)
    int slicey = iupAttribGetInt(ih, "SLICEY");  //Default -1 (central)
    int slicez = iupAttribGetInt(ih, "SLICEZ");  //Default -1 (central)
    if (tolower(*slicedir)=='x') { gr->Grid3(*ds->dsX, 'x', slicex, style); slicedir++; }
    if (tolower(*slicedir)=='y') { gr->Grid3(*ds->dsX, 'y', slicey, style); slicedir++; }
    if (tolower(*slicedir)=='z') { gr->Grid3(*ds->dsX, 'z', slicez, style); slicedir++; }
  }
  else if (iupStrEqualNoCase(ds->dsMode, "VOLUME_CLOUD"))
  {
    style[0] = 0;

    float alpha = iupAttribGetFloat(ih, "OVERALLALPHA");  //Default 1
    int cubes = iupAttribGetBoolean(ih, "CLOUDCUBES");  //Default true
    if (cubes)
      gr->Cloud(*ds->dsX, style, alpha);
    else
      gr->CloudP(*ds->dsX, style, alpha);
  }
}

static void iMglPlotDrawPlanarData(Ihandle* ih, mglGraph *gr, IdataSet* ds)
{               
  char style[64];

  //TODO
  //gr->Light(true);
  //gr->Light(1,mglPoint(0,1,0),’c’);
  //gr->Rotate(40, 60);
  //gr->Fog(1);

  if (iupStrEqualNoCase(ds->dsMode, "PLANAR_MESH"))
  {
    style[0] = 0;
    gr->Mesh(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_FALL"))
  {
    style[0] = 0;

    char* falldir = iupAttribGetStr(ih, "DIR"); //Default "Y"
    if (tolower(*falldir) == 'x')
      { style[0] = 'x'; style[1] = 0; }

    gr->Fall(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_BELT"))
  {
    style[0] = 0;

    char* beltdir = iupAttribGetStr(ih, "DIR"); //Default "Y"
    if (tolower(*beltdir) == 'x')
      { style[0] = 'x'; style[1] = 0; }

    gr->Belt(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_SURFACE"))
  {
    style[0] = 0;

    if (iupAttribGetBoolean(ih, "DATAGRID"))   // Default false
      { style[0] = '#'; style[1] = 0; }

    gr->Surf(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_BOXES"))
  {
    style[0] = 0;

    if (iupAttribGetBoolean(ih, "DATAGRID"))   // Default false
      { style[0] = '#'; style[1] = 0; }   // means box lines, as wire

    gr->Boxs(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_TILE"))
  {
    style[0] = 0;
    gr->Tile(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_DENSITY"))
  {
    style[0] = 0;
    float val = iMglPlotGetAttribFloatNAN(ih, "PLANARVALUE");   // Default NAN
    gr->Dens(*ds->dsX, style, val);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_CONTOUR"))
  {
    int contourcount = iupAttribGetInt(ih, "CONTOURCOUNT");  //Default 7
    int countourfilled = iupAttribGetBoolean(ih, "CONTOURFILLED");  //Default false
    float val = iMglPlotGetAttribFloatNAN(ih, "PLANARVALUE");  // Default NAN
    if (countourfilled)
      gr->ContF(*ds->dsX, style, contourcount, val);
    else
      gr->Cont(*ds->dsX, style, contourcount, val);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_AXIALCONTOUR"))
  {
    int axialcount = iupAttribGetInt(ih, "AXIALCOUNT");  //Default 3
    gr->Axial(*ds->dsX, style, axialcount);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_GRADIENTLINES"))
  {
    int gradlinescount = iupAttribGetInt(ih, "GRADLINESCOUNT");  //Default 5
    float val = iMglPlotGetAttribFloatNAN(ih, "PLANARVALUE");  // Default NAN
    gr->Grad(*ds->dsX, style, gradlinescount, val);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "PLANAR_GRID"))
  {
    float val = iMglPlotGetAttribFloatNAN(ih, "PLANARVALUE");  // Default NAN
    gr->Grid(*ds->dsX, style, val);
  }
}

static void iMglPlotDrawLinearData(Ihandle* ih, mglGraph *gr, IdataSet* ds)
{               
  char style[64];

  iMglPlotConfigColor(ih, gr, ds->dsColor);
  // Some 1D plots depend on this
  gr->SetPalNum(1);
  gr->SetPalColor(0, ds->dsColor.r, ds->dsColor.g, ds->dsColor.b);

  if (iupStrEqualNoCase(ds->dsMode, "LINE") ||
      iupStrEqualNoCase(ds->dsMode, "MARKLINE") ||
      iupStrEqualNoCase(ds->dsMode, "MARK"))
  {
    int mark = 0;
    if (!iupStrEqualNoCase(ds->dsMode, "LINE"))
      mark = 1;

    iMglPlotConfigDataSetLineStyle(ds, gr, style, mark);

    if (iupStrEqualNoCase(ds->dsMode, "MARK")) 
      style[0] = ' '; // No line

    if (ds->dsDim == 1)
      gr->Plot(*ds->dsX, style);
    else if (ds->dsDim == 2)
      gr->Plot(*ds->dsX, *ds->dsY, style);
    else if (ds->dsDim == 3)
      gr->Plot(*ds->dsX, *ds->dsY, *ds->dsZ, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "RADAR"))
  {
    int marks = iupAttribGetBoolean(ih, "MARKS");  //Default false
    iMglPlotConfigDataSetLineStyle(ds, gr, style, marks);

    if (iupAttribGetBoolean(ih, "DATAGRID"))  //Default false
      strcat(style, "#");

    if (ds->dsDim == 1)
      gr->Radar(*ds->dsX, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "AREA"))
  {
    style[0] = 0;

    if (ds->dsDim == 1)
      gr->Area(*ds->dsX, style);
    else if (ds->dsDim == 2)
      gr->Area(*ds->dsX, *ds->dsY, style);
    else if (ds->dsDim == 3)
      gr->Area(*ds->dsX, *ds->dsY, *ds->dsZ, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "BAR"))
  {
    style[0] = 0;

    if (ds->dsDim == 1)
      gr->Bars(*ds->dsX, style);
    else if (ds->dsDim == 2)
      gr->Bars(*ds->dsX, *ds->dsY, style);
    else if (ds->dsDim == 3)
      gr->Bars(*ds->dsX, *ds->dsY, *ds->dsZ, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "BARHORIZONTAL"))
  {
    style[0] = 0;

    if (ds->dsDim == 1)
      gr->Barh(*ds->dsX, style);
    else if (ds->dsDim == 2)
      gr->Barh(*ds->dsX, *ds->dsY, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "STEP"))
  {
    int marks = iupAttribGetBoolean(ih, "MARKS");  //Default false
    iMglPlotConfigDataSetLineStyle(ds, gr, style, marks);

    if (ds->dsDim == 1)
      gr->Step(*ds->dsX, style);
    else if (ds->dsDim == 2)
      gr->Step(*ds->dsX, *ds->dsY, style);
    else if (ds->dsDim == 3)
      gr->Step(*ds->dsX, *ds->dsY, *ds->dsZ, style);
  }
  else if (iupStrEqualNoCase(ds->dsMode, "STEM"))
  {
    int marks = iupAttribGetBoolean(ih, "MARKS");  //Default false
    iMglPlotConfigDataSetLineStyle(ds, gr, style, marks);

    if (ds->dsDim == 1)
      gr->Stem(*ds->dsX, style);
    else if (ds->dsDim == 2)
      gr->Stem(*ds->dsX, *ds->dsY, style);
    else if (ds->dsDim == 3)
      gr->Stem(*ds->dsX, *ds->dsY, *ds->dsZ, style);
  }

  //TODO
  //  Chart
  //  Crust
  //  Dots
  //  Dual Ploting...
  //BARWIDTH/BAR min-max

  //TODO pots that need two datasets: region, tens, error, flow, pipe, ...
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

    //iMglPlotConfigDataSetLineStyle(ds, gr, style, 0);  //TODO Mark as dsMode if marks are used
    gr->AddLegend(ds->dsLegend, style);
  }

  gr->SetLegendBox(ih->data->legendBox);

  // Draw legend of accumulated strings
  iMglPlotConfigColor(ih, gr, ih->data->legendColor);
  iMglPlotConfigFont(ih, gr, ih->data->legendFontStyle, ih->data->legendFontSizeFactor);
  gr->Legend(ih->data->legendPosition, NULL, -1, 0.08f);
}

static void iMglPlotDrawTitle(Ihandle* ih, mglGraph *gr, const char* title)
{
  iMglPlotConfigColor(ih, gr, ih->data->titleColor);
  iMglPlotConfigFont(ih, gr, ih->data->titleFontStyle, ih->data->titleFontSizeFactor);
  gr->Title(title, NULL, -1);
}

static void iMglPlotDrawPlot(Ihandle* ih, mglGraph *gr)
{
  gr->SubPlot(1, 1, 0);

  gr->AutoOrg = false;

  // larger values, smaller plots
  // 1.0 show exactly the dataset area
  gr->PlotFactor = 1.4f;  // This zooms everything, except title   // TODO compare this to Zoom()
                          
  if (ih->data->alpha)
    gr->Alpha(true);
  else
    gr->Alpha(false);

  iMglPlotConfigFontDef(ih, gr);

  // Configure Min-Max
  iMglPlotConfigAxesRange(ih, gr);

  iupAttribSetStr(ih, "_IUP_MGLPLOT_GRAPH", (char*)gr);

  IFn cb = IupGetCallback(ih, "PREDRAW_CB");
  if (cb)
    cb(ih);

  if(ih->data->gridShow)
    iMglPlotDrawGrid(ih, gr);

  if (ih->data->Box)
  {
    char pen[10];
    iMglPlotConfigColor(ih, gr, ih->data->boxColor);
    iMglPlotConfigPen(gr, pen, '-', 1);
    gr->Box(pen, ih->data->boxTicks);
  }

  iMglPlotDrawAxes(ih, gr);

  iMglPlotDrawData(ih, gr);

  if(ih->data->legendShow)
    iMglPlotDrawLegend(ih, gr);

  char* value = iupAttribGetStr(ih, "TITLE");
  if (value)
    iMglPlotDrawTitle(ih, gr, value);

  cb = IupGetCallback(ih, "POSTDRAW_CB");
  if (cb)
    cb(ih);

  iupAttribSetStr(ih, "_IUP_MGLPLOT_GRAPH", NULL);
}

static void iMglPlotRepaint(Ihandle* ih, int force, int flush)
{
  if (!IupGLIsCurrent(ih))
    force = 1;

  IupGLMakeCurrent(ih);

  if (force || ih->data->redraw)
  {
    ih->data->mgl->Clf(ih->data->bgColor); /* Clear */

    /* update render */
    iMglPlotDrawPlot(ih, ih->data->mgl);  /* Draw the graphics plot */

    ih->data->mgl->Finish();
    ih->data->redraw = false;
  }

  if (flush)
    IupGLSwapBuffers(ih);
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
    color.Set(iRecon(rr), iRecon(gg), iRecon(bb));
    ih->data->redraw = true;
  }

  return 0;
}

static char* iMglPlotGetColorAttrib(const mglColor& color)
{
  char* buffer = iupStrGetMemory(30);
  sprintf(buffer, "%d %d %d", iQuant(color.r), iQuant(color.g), iQuant(color.b));
  return buffer;
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

static int iMglPlotSetFloat(Ihandle* ih, const char* value, float& num)
{
  float old_num = num;

  iupStrToFloat(value, &num);

  if (old_num != num)
    ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetFloat(float num)
{
  char* buffer = iupStrGetMemory(30);
  sprintf(buffer, "%g", num);
  return buffer;
}

static char* iMglPlotGetInt(int num)
{
  char* buffer = iupStrGetMemory(30);
  sprintf(buffer, "%d", num);
  return buffer;
}

static char* iMglPlotGetBoolean(bool num)
{
  if (num)
    return "YES";
  else
    return "NO";
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
  return iMglPlotGetColorAttrib(ih->data->bgColor);
}

static int iMglPlotSetFGColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->fgColor);
}

static char* iMglPlotGetFGColorAttrib(Ihandle* ih)
{
  return iMglPlotGetColorAttrib(ih->data->fgColor);
}

static int iMglPlotSetTitleFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->titleFontSizeFactor);
}

static char* iMglPlotGetTitleFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->titleFontSizeFactor);
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
    return iMglPlotGetInt(fontstyle);
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
  return iMglPlotGetColorAttrib(ih->data->titleColor);
}

static int iMglPlotSetLegendFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->legendFontSizeFactor);
}

static char* iMglPlotGetLegendFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->legendFontStyle);
}

static char* iMglPlotGetLegendFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->legendFontSizeFactor);
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
  return iMglPlotGetBoolean(ih->data->legendShow);
}

static int iMglPlotSetLegendBoxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->legendBox);
}

static char* iMglPlotGetLegendBoxAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->legendBox);
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
  return iMglPlotGetColorAttrib(ih->data->legendColor);
}

static int iMglPlotSetGridColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->gridColor);
}

static char* iMglPlotGetGridColorAttrib(Ihandle* ih)
{
  return iMglPlotGetColorAttrib(ih->data->gridColor);
}

static int iMglPlotSetBoxColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->boxColor);
}

static char* iMglPlotGetBoxColorAttrib(Ihandle* ih)
{
  return iMglPlotGetColorAttrib(ih->data->boxColor);
}

static int iMglPlotSetLineStyle(Ihandle* ih, const char* value, char &linestyle)
{
  char old_linestyle = linestyle;

  if (!value || iupStrEqualNoCase(value, "CONTINUOUS"))
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
           iupStrEqualNoCase(value, "DASH_DOT_DOT")) // for compatibility
    linestyle = 'i';
  else
    linestyle = '-';  /* reset to default */

  if (old_linestyle != linestyle)
    ih->data->redraw = true;

  return 0;
}

static char* iMglPlotGetGridLineStyle(char linestyle)
{
  switch(linestyle)
  {
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
  return iMglPlotGetGridLineStyle(ih->data->gridLineStyle);
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
  return iMglPlotGetInt(ih->data->dataSetCount);
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
  return iMglPlotGetInt(ih->data->dataSetCurrent);
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
  ih->data->redraw = true;
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
  return iMglPlotGetGridLineStyle(ds->dsLineStyle);
}

static int iMglPlotSetDSLineWidthAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetFloat(ih, value, ds->dsLineWidth);
}

static char* iMglPlotGetDSLineWidthAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;
  else
  {
    IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
    return iMglPlotGetFloat(ds->dsLineWidth);
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
    markstyle = 'O';  // In fact "#o"
  else if (iupStrEqualNoCase(value, "PLUS"))
    markstyle = '+';
  else if (iupStrEqualNoCase(value, "BOX"))
    markstyle = 'S';  // In fact "#s"
  else if (iupStrEqualNoCase(value, "DIAMOND"))
    markstyle = 'D';  // In fact "#d"
  else if (iupStrEqualNoCase(value, "HOLLOW_CIRCLE"))
    markstyle = 'o';
  else if (iupStrEqualNoCase(value, "HOLLOW_BOX"))
    markstyle = 's';
  else if (iupStrEqualNoCase(value, "HOLLOW_DIAMOND"))
    markstyle = 'd';
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
    default: return "PLUS";
  }
}

static int iMglPlotSetDSMarkSizeAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetFloat(ih, value, ds->dsMarkSize);
}

static char* iMglPlotGetDSMarkSizeAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;
  else
  {
    IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
    return iMglPlotGetFloat(ds->dsMarkSize);
  }
}

static int iMglPlotSetDSColorAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->dataSetCurrent==-1)
    return 0;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotSetColor(ih, value, ds->dsColor);
}

static char* iMglPlotGetDSColorAttrib(Ihandle* ih)
{
  if (ih->data->dataSetCurrent==-1)
    return NULL;

  IdataSet* ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  return iMglPlotGetColorAttrib(ds->dsColor);
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

static char* iMglPlotDefaultLegend(int ds)
{
  char legend[50];
  sprintf(legend, "plot %d", ds);
  return iupStrDup(legend);
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
  return iMglPlotGetBoolean(ds->dsShowValues);
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
      memmove(ds->dsX->a + index, ds->dsX->a + (index + remove_count), sizeof(float)*(ds->dsCount - (index + remove_count)));
      if (ds->dsY)
        memmove(ds->dsY->a + index, ds->dsY->a + (index + remove_count), sizeof(float)*(ds->dsCount - (index + remove_count)));
      if (ds->dsZ)
        memmove(ds->dsZ->a + index, ds->dsZ->a + (index + remove_count), sizeof(float)*(ds->dsCount - (index + remove_count)));
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
    return iMglPlotGetInt(ds->dsCount);
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
    {
      char* buffer = iupStrGetMemory(30);
      sprintf(buffer, "%dx%dx%d", ds->dsX->nx, ds->dsX->ny, ds->dsX->nz);
      return buffer;
    }
    else
      return iMglPlotGetInt(ds->dsDim);
  }
}

static int iMglPlotSetDSEditAttrib(Ihandle* ih, const char* value)
{
  IdataSet* ds;

  if (ih->data->dataSetCurrent==-1)
    return 0;

  ds = &ih->data->dataSet[ih->data->dataSetCurrent];
  if (iMglPlotIsPlanarOrVolumetricData(ds))
    return 0;

  // TODO
  if (iupStrBoolean(value))
    ;
  else
    ;

  return 0;
}

static char* iMglPlotGetDSEditAttrib(Ihandle* ih)
{
  (void)ih;
  return "NO";
}

static int iMglPlotSetBoxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->Box);
}

static char* iMglPlotGetBoxAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->Box);
}

static int iMglPlotSetBoxTicksAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->boxTicks);
}

static char* iMglPlotGetBoxTicksAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->boxTicks);
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
  return iMglPlotSetAxisLabelPosition(ih, value, ih->data->xAxis.axLabelPos);
}

static int iMglPlotSetAxisYLabelPositionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisLabelPosition(ih, value, ih->data->yAxis.axLabelPos);
}

static int iMglPlotSetAxisZLabelPositionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisLabelPosition(ih, value, ih->data->zAxis.axLabelPos);
}

static char* iMglPlotGetAxisXLabelPositionAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisLabelPosition(ih->data->xAxis.axLabelPos);
}

static char* iMglPlotGetAxisYLabelPositionAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisLabelPosition(ih->data->yAxis.axLabelPos);
}

static char* iMglPlotGetAxisZLabelPositionAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisLabelPosition(ih->data->zAxis.axLabelPos);
}

static int iMglPlotSetAxisXLabelRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axLabelRotation);
}

static int iMglPlotSetAxisYLabelRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axLabelRotation);
}

static int iMglPlotSetAxisZLabelRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axLabelRotation);
}

static char* iMglPlotGetAxisXLabelRotationAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axLabelRotation);
}

static char* iMglPlotGetAxisYLabelRotationAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axLabelRotation);
}

static char* iMglPlotGetAxisZLabelRotationAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axLabelRotation);
}

static int iMglPlotSetAxisXLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    return iMglPlotSetAxisLabelPosition(ih, "CENTER", ih->data->xAxis.axLabelPos);
  else 
    return iMglPlotSetAxisLabelPosition(ih, "MAX", ih->data->xAxis.axLabelPos);
}

static int iMglPlotSetAxisYLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    return iMglPlotSetAxisLabelPosition(ih, "CENTER", ih->data->yAxis.axLabelPos);
  else 
    return iMglPlotSetAxisLabelPosition(ih, "MAX", ih->data->yAxis.axLabelPos);
}

static int iMglPlotSetAxisZLabelCenteredAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    return iMglPlotSetAxisLabelPosition(ih, "CENTER", ih->data->zAxis.axLabelPos);
  else 
    return iMglPlotSetAxisLabelPosition(ih, "MAX", ih->data->zAxis.axLabelPos);
}

static char* iMglPlotGetAxisXLabelCenteredAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axLabelPos==0? true: false);
}

static char* iMglPlotGetAxisYLabelCenteredAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axLabelPos==0? true: false);
}

static char* iMglPlotGetAxisZLabelCenteredAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axLabelPos==0? true: false);
}

static int iMglPlotSetAxisXColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->xAxis.axColor);
}

static int iMglPlotSetAxisYColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->yAxis.axColor);
}

static int iMglPlotSetAxisZColorAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetColor(ih, value, ih->data->zAxis.axColor);
}

static char* iMglPlotGetAxisXColorAttrib(Ihandle* ih)
{
  return iMglPlotGetColorAttrib(ih->data->xAxis.axColor);
}

static char* iMglPlotGetAxisYColorAttrib(Ihandle* ih)
{
  return iMglPlotGetColorAttrib(ih->data->yAxis.axColor);
}

static char* iMglPlotGetAxisZColorAttrib(Ihandle* ih)
{
  return iMglPlotGetColorAttrib(ih->data->zAxis.axColor);
}

static int iMglPlotSetAxisXFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axLabelFontSizeFactor);
}

static int iMglPlotSetAxisYFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axLabelFontSizeFactor);
}

static int iMglPlotSetAxisZFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axLabelFontSizeFactor);
}

static char* iMglPlotGetAxisXFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axLabelFontSizeFactor);
}

static char* iMglPlotGetAxisYFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axLabelFontSizeFactor);
}

static char* iMglPlotGetAxisZFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axLabelFontSizeFactor);
}

static int iMglPlotSetAxisXFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->xAxis.axLabelFontStyle);
}

static int iMglPlotSetAxisYFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->yAxis.axLabelFontStyle);
}

static int iMglPlotSetAxisZFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->zAxis.axLabelFontStyle);
}

static char* iMglPlotGetAxisXFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->xAxis.axLabelFontStyle);
}

static char* iMglPlotGetAxisYFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->yAxis.axLabelFontStyle);
}

static char* iMglPlotGetAxisZFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->zAxis.axLabelFontStyle);
}

static int iMglPlotSetAxisXTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axTickFontSizeFactor);
}

static int iMglPlotSetAxisYTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axTickFontSizeFactor);
}

static int iMglPlotSetAxisZTickFontSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axTickFontSizeFactor);
}

static char* iMglPlotGetAxisXTickFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axTickFontSizeFactor);
}

static char* iMglPlotGetAxisYTickFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axTickFontSizeFactor);
}

static char* iMglPlotGetAxisZTickFontSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axTickFontSizeFactor);
}

static int iMglPlotSetAxisXTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->xAxis.axTickFontStyle);
}

static int iMglPlotSetAxisYTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->yAxis.axTickFontStyle);
}

static int iMglPlotSetAxisZTickFontStyleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFontStyle(ih, value, ih->data->zAxis.axTickFontStyle);
}

static char* iMglPlotGetAxisXTickFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->xAxis.axTickFontStyle);
}

static char* iMglPlotGetAxisYTickFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->yAxis.axTickFontStyle);
}

static char* iMglPlotGetAxisZTickFontStyleAttrib(Ihandle* ih)
{
  return iMglPlotGetFontStyle(ih->data->zAxis.axTickFontStyle);
}

static int iMglPlotSetAxisXTickMinorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axTickMinorSizeFactor);
}

static int iMglPlotSetAxisYTickMinorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axTickMinorSizeFactor);
}

static int iMglPlotSetAxisZTickMinorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axTickMinorSizeFactor);
}

static char* iMglPlotGetAxisXTickMinorSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axTickMinorSizeFactor);
}

static char* iMglPlotGetAxisYTickMinorSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axTickMinorSizeFactor);
}

static char* iMglPlotGetAxisZTickMinorSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axTickMinorSizeFactor);
}

static int iMglPlotSetAxisXTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axTickMajorSize);
}

static int iMglPlotSetAxisYTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axTickMajorSize);
}

static int iMglPlotSetAxisZTickMajorSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axTickMajorSize);
}

static char* iMglPlotGetAxisXTickMajorSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axTickMajorSize);
}

static char* iMglPlotGetAxisYTickMajorSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axTickMajorSize);
}

static char* iMglPlotGetAxisZTickMajorSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axTickMajorSize);
}

static int iMglPlotSetAxisXTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axTickMajorSpan);
}

static int iMglPlotSetAxisYTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axTickMajorSpan);
}

static int iMglPlotSetAxisZTickMajorSpanAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axTickMajorSpan);
}

static char* iMglPlotGetAxisXTickMajorSpanAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axTickMajorSpan);
}

static char* iMglPlotGetAxisYTickMajorSpanAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axTickMajorSpan);
}

static char* iMglPlotGetAxisZTickMajorSpanAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axTickMajorSpan);
}

static int iMglPlotSetAxisScale(Ihandle* ih, const char* value, const char** scale, char dir)
{
  if(iupStrEqualNoCase(value, "LIN"))
    *scale = NULL;
  else if(iupStrEqualNoCase(value, "LOG10"))
    *scale = (dir=='x')? "lg(x)": (dir=='y'? "lg(y)": "lg(z)");
/*  else if(iupStrEqualNoCase(value, "LOG2"))   // NOT supported yet
    *scale = (dir=='x')? "log(x, 2)": (dir=='y'? "log(y, 2)": "log(z, 2)");
  else if(iupStrEqualNoCase(value, "LOGN"))
    *scale = (dir=='x')? "ln(x)": (dir=='y'? "ln(y)": "ln(z)"); */

  ih->data->redraw = true;
  return 0;
}

static int iMglPlotSetAxisXScaleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisScale(ih, value, &(ih->data->xAxis.axScale), 'x');
}

static int iMglPlotSetAxisYScaleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisScale(ih, value, &(ih->data->yAxis.axScale), 'y');
}

static int iMglPlotSetAxisZScaleAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisScale(ih, value, &(ih->data->zAxis.axScale), 'z');
}

static char* iMglPlotGetAxisScale(const char* scale)
{
  if (!scale)
    return "LIN";
  else if (strstr(scale, "lg"))
    return "LOG10";
/*  else if (strstr(scale, "log"))  // NOT supported yet
    return "LOG2";
  else if (strstr(scale, "ln"))
    return "LOGN"; */
  return NULL;
}

static char* iMglPlotGetAxisXScaleAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisScale(ih->data->xAxis.axScale);
}

static char* iMglPlotGetAxisYScaleAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisScale(ih->data->yAxis.axScale);
}

static char* iMglPlotGetAxisZScaleAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisScale(ih->data->zAxis.axScale);
}

static int iMglPlotSetAxisXReverseAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axReverse);
}

static int iMglPlotSetAxisYReverseAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axReverse);
}

static int iMglPlotSetAxisZReverseAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axReverse);
}

static char* iMglPlotGetAxisXReverseAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axReverse);
}

static char* iMglPlotGetAxisYReverseAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axReverse);
}

static char* iMglPlotGetAxisZReverseAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axReverse);
}

static int iMglPlotSetAxisXShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axShow);
}

static int iMglPlotSetAxisYShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axShow);
}

static int iMglPlotSetAxisZShowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axShow);
}

static char* iMglPlotGetAxisXShowAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axShow);
}

static char* iMglPlotGetAxisYShowAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axShow);
}

static char* iMglPlotGetAxisZShowAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axShow);
}

static int iMglPlotSetAxisXShowArrowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axShowArrow);
}

static int iMglPlotSetAxisYShowArrowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axShowArrow);
}

static int iMglPlotSetAxisZShowArrowAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axShowArrow);
}

static char* iMglPlotGetAxisXShowArrowAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axShowArrow);
}

static char* iMglPlotGetAxisYShowArrowAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axShowArrow);
}

static char* iMglPlotGetAxisZShowArrowAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axShowArrow);
}

static int iMglPlotSetAxisXTickShowValuesAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axTickShowValues);
}

static int iMglPlotSetAxisYTickShowValuesAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axTickShowValues);
}

static int iMglPlotSetAxisZTickShowValuesAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axTickShowValues);
}

static char* iMglPlotGetAxisXTickShowValuesAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axTickShowValues);
}

static char* iMglPlotGetAxisYTickShowValuesAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axTickShowValues);
}

static char* iMglPlotGetAxisZTickShowValuesAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axTickShowValues);
}

static int iMglPlotSetAxisXTickAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axTickShow);
}

static int iMglPlotSetAxisYTickAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axTickShow);
}

static int iMglPlotSetAxisZTickAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axTickShow);
}

static char* iMglPlotGetAxisXTickAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axTickShow);
}

static char* iMglPlotGetAxisYTickAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axTickShow);
}

static char* iMglPlotGetAxisZTickAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axTickShow);
}

static int iMglPlotSetAxisXTickAutoSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axTickAutoSize);
}

static int iMglPlotSetAxisYTickAutoSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axTickAutoSize);
}

static int iMglPlotSetAxisZTickAutoSizeAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axTickAutoSize);
}

static char* iMglPlotGetAxisXTickAutoSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axTickAutoSize);
}

static char* iMglPlotGetAxisYTickAutoSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axTickAutoSize);
}

static char* iMglPlotGetAxisZTickAutoSizeAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axTickAutoSize);
}

static int iMglPlotSetAxisXTickAutoAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axTickAutoSpace);
}

static int iMglPlotSetAxisYTickAutoAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axTickAutoSpace);
}

static int iMglPlotSetAxisZTickAutoAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axTickAutoSpace);
}

static char* iMglPlotGetAxisXTickAutoAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axTickAutoSpace);
}

static char* iMglPlotGetAxisYTickAutoAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axTickAutoSpace);
}

static char* iMglPlotGetAxisZTickAutoAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axTickAutoSpace);
}

//////////////////////////
static int iMglPlotSetAxisXTickValuesRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axTickValuesRotation);
}

static int iMglPlotSetAxisYTickValuesRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axTickValuesRotation);
}

static int iMglPlotSetAxisZTickValuesRotationAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axTickValuesRotation);
}

static char* iMglPlotGetAxisXTickValuesRotationAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axTickValuesRotation);
}

static char* iMglPlotGetAxisYTickValuesRotationAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axTickValuesRotation);
}

static char* iMglPlotGetAxisZTickValuesRotationAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axTickValuesRotation);
}


static int iMglPlotSetAxisXTickMinorDivisionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetInt(ih, value, ih->data->xAxis.axTickMinorDivision);
}

static int iMglPlotSetAxisYTickMinorDivisionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetInt(ih, value, ih->data->yAxis.axTickMinorDivision);
}

static int iMglPlotSetAxisZTickMinorDivisionAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetInt(ih, value, ih->data->zAxis.axTickMinorDivision);
}

static char* iMglPlotGetAxisXTickMinorDivisionAttrib(Ihandle* ih)
{
  return iMglPlotGetInt(ih->data->xAxis.axTickMinorDivision);
}

static char* iMglPlotGetAxisYTickMinorDivisionAttrib(Ihandle* ih)
{
  return iMglPlotGetInt(ih->data->yAxis.axTickMinorDivision);
}

static char* iMglPlotGetAxisZTickMinorDivisionAttrib(Ihandle* ih)
{
  return iMglPlotGetInt(ih->data->zAxis.axTickMinorDivision);
}


static int iMglPlotSetAxisXAutoMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axAutoScaleMin);
}

static int iMglPlotSetAxisYAutoMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axAutoScaleMin);
}

static int iMglPlotSetAxisZAutoMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axAutoScaleMin);
}

static char* iMglPlotGetAxisXAutoMinAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axAutoScaleMin);
}

static char* iMglPlotGetAxisYAutoMinAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axAutoScaleMin);
}

static char* iMglPlotGetAxisZAutoMinAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axAutoScaleMin);
}

static int iMglPlotSetAxisXAutoMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->xAxis.axAutoScaleMax);
}

static int iMglPlotSetAxisYAutoMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->yAxis.axAutoScaleMax);
}

static int iMglPlotSetAxisZAutoMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetBoolean(ih, value, ih->data->zAxis.axAutoScaleMax);
}

static char* iMglPlotGetAxisXAutoMaxAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->xAxis.axAutoScaleMax);
}

static char* iMglPlotGetAxisYAutoMaxAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->yAxis.axAutoScaleMax);
}

static char* iMglPlotGetAxisZAutoMaxAttrib(Ihandle* ih)
{
  return iMglPlotGetBoolean(ih->data->zAxis.axAutoScaleMax);
}

static int iMglPlotSetAxisXMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axMin);
}

static int iMglPlotSetAxisYMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axMin);
}

static int iMglPlotSetAxisZMinAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axMin);
}

static char* iMglPlotGetAxisXMinAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axMin);
}

static char* iMglPlotGetAxisYMinAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axMin);
}

static char* iMglPlotGetAxisZMinAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axMin);
}

static int iMglPlotSetAxisXMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->xAxis.axMax);
}

static int iMglPlotSetAxisYMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->yAxis.axMax);
}

static int iMglPlotSetAxisZMaxAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetFloat(ih, value, ih->data->zAxis.axMax);
}

static char* iMglPlotGetAxisXMaxAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->xAxis.axMax);
}

static char* iMglPlotGetAxisYMaxAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->yAxis.axMax);
}

static char* iMglPlotGetAxisZMaxAttrib(Ihandle* ih)
{
  return iMglPlotGetFloat(ih->data->zAxis.axMax);
}

static int iMglPlotSetAxisCrossOrigin(Ihandle* ih, const char* value, float& origin)
{
  float old_origin = origin;

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
  return iMglPlotSetAxisCrossOrigin(ih, value, ih->data->xAxis.axOrigin);
}

static int iMglPlotSetAxisYCrossOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisCrossOrigin(ih, value, ih->data->yAxis.axOrigin);
}

static int iMglPlotSetAxisZCrossOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisCrossOrigin(ih, value, ih->data->zAxis.axOrigin);
}

static char* iMglPlotGetAxisXCrossOrigin(float origin)
{
  if (origin==0)
    return "YES";
  else if (origin==NAN)
    return "NO";
  else
    return NULL;
}

static char* iMglPlotGetAxisXCrossOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisXCrossOrigin(ih->data->xAxis.axOrigin);
}

static char* iMglPlotGetAxisYCrossOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisXCrossOrigin(ih->data->yAxis.axOrigin);
}

static char* iMglPlotGetAxisZCrossOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisXCrossOrigin(ih->data->zAxis.axOrigin);
}

static int iMglPlotSetAxisOrigin(Ihandle* ih, const char* value, float& num)
{
  float old_num = num;

  if (!value)
    num = NAN;
  else
    iupStrToFloat(value, &num);

  if (old_num != num)
    ih->data->redraw = true;

  return 0;
}

static int iMglPlotSetAxisXOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisOrigin(ih, value, ih->data->xAxis.axOrigin);
}

static int iMglPlotSetAxisYOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisOrigin(ih, value, ih->data->yAxis.axOrigin);
}

static int iMglPlotSetAxisZOriginAttrib(Ihandle* ih, const char* value)
{
  return iMglPlotSetAxisOrigin(ih, value, ih->data->zAxis.axOrigin);
}

static char* iMglPlotGetAxisOrigin(float Origin)
{
  if (Origin != NAN)
    return iMglPlotGetFloat(Origin);
  else
    return NULL;
}

static char* iMglPlotGetAxisXOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisOrigin(ih->data->xAxis.axOrigin);
}

static char* iMglPlotGetAxisYOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisOrigin(ih->data->yAxis.axOrigin);
}

static char* iMglPlotGetAxisZOriginAttrib(Ihandle* ih)
{
  return iMglPlotGetAxisOrigin(ih->data->zAxis.axOrigin);
}

static int iMglPlotSetAlphaAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->alpha = true;
  else
    ih->data->alpha = false;
  return 0;
}

static char* iMglPlotGetAlphaAttrib(Ihandle* ih)
{
  if (ih->data->alpha)
    return "1";
  else
    return "0";
}

static int iMglPlotSetAntialiasAttrib(Ihandle* ih, const char* value)
{
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
  IupGLMakeCurrent(ih);

  if (glIsEnabled(GL_LINE_SMOOTH))
    return "1";
  else
    return "0";
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

  // Alocate memory if necessary
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
  ds->dsLineStyle  = '-';  // SOLID/CONTINUOUS
  ds->dsLineWidth  = 1;
  ds->dsMarkStyle  = 'x';  // "X"
  ds->dsMarkSize   = 0.02f;
  ds->dsMode = iupStrDup("LINE");
  ds->dsLegend = iMglPlotDefaultLegend(ih->data->dataSetCurrent);

  switch(ds_index)
  {
    case 0: ds->dsColor.Set(1, 0, 0); break;
    case 1: ds->dsColor.Set(0, 0, 1); break;
    case 2: ds->dsColor.Set(0, 1, 0); break;
    case 3: ds->dsColor.Set(0, 1, 1); break;
    case 4: ds->dsColor.Set(1, 0, 1); break;
    case 5: ds->dsColor.Set(1, 1, 0); break;
    default: ds->dsColor.Set(0, 0, 0); break;
  }

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

  inXData =  iupArrayCreate(10, sizeof(float));
  if (dim>1)
    inYData =  iupArrayCreate(10, sizeof(float));
  if (dim>2)
    inZData =  iupArrayCreate(10, sizeof(float));

  iupAttribSetStr(ih, "_IUP_MGLPLOT_XDATA", (char*)inXData);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_YDATA", (char*)inYData);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_ZDATA", (char*)inZData);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_NAMES", NULL);
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
    float* x = (float*)iupArrayGetData(inXData);
    int count = iupArrayCount(inXData);
    IupMglPlotSet1D(ih, ds_index, (const char**)names, x, count);
  }
  else if (dim==2)
  {
    float* x = (float*)iupArrayGetData(inXData);
    float* y = (float*)iupArrayGetData(inYData);
    int count = iupArrayCount(inXData);
    IupMglPlotSet2D(ih, ds_index, x, y, count);
  }
  else if (dim==3)
  {
    float* x = (float*)iupArrayGetData(inXData);
    float* y = (float*)iupArrayGetData(inYData);
    float* z = (float*)iupArrayGetData(inZData);
    int count = iupArrayCount(inXData);
    IupMglPlotSet3D(ih, ds_index, x, y, z, count);
  }

  if (inXData) iupArrayDestroy(inXData);
  if (inYData) iupArrayDestroy(inYData);
  if (inZData) iupArrayDestroy(inZData);
  if (inNames) iupArrayDestroy(inNames);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_XDATA", NULL);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_YDATA", NULL);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_ZDATA", NULL);
  iupAttribSetStr(ih, "_IUP_MGLPLOT_NAMES", NULL);

  return ds_index;
}

void IupMglPlotAdd1D(Ihandle* ih, const char* inName, float inX)
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
      iupAttribSetStr(ih, "_IUP_MGLPLOT_NAMES", (char*)inNames);
    }
    else
      inNames = (Iarray*)iupAttribGet(ih, "_IUP_MGLPLOT_NAMES");

    if (inNames)
    {
      char** names = (char**)iupArrayInc(inNames);
      names[sample_index] = iupStrDup(inName);
    }
  }

  float* x = (float*)iupArrayInc(inXData);
  x[sample_index] = inX;
}

void IupMglPlotAdd2D(Ihandle* ih, float inX, float inY)
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

  float* x = (float*)iupArrayInc(inXData);
  float* y = (float*)iupArrayInc(inYData);
  x[sample_index] = inX;
  y[sample_index] = inY;
}

void IupMglPlotAdd3D(Ihandle *ih, float inX, float inY, float inZ)
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

  float* x = (float*)iupArrayInc(inXData);
  float* y = (float*)iupArrayInc(inYData);
  float* z = (float*)iupArrayInc(inZData);
  x[sample_index] = inX;
  y[sample_index] = inY;
  z[sample_index] = inZ;
}

void IupMglPlotInsert1D(Ihandle* ih, int inIndex, int inSampleIndex, const char** inNames, const float* inX, int inCount)
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
    memmove(ds->dsX->a + inSampleIndex + inCount, ds->dsX->a + inSampleIndex, inCount*sizeof(float));
  memcpy(ds->dsX->a + inSampleIndex, inX, inCount*sizeof(float));

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

void IupMglPlotInsert2D(Ihandle* ih, int inIndex, int inSampleIndex, const float *inX, const float *inY, int inCount)
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
    memmove(ds->dsX->a + inSampleIndex + inCount, ds->dsX->a + inSampleIndex, inCount*sizeof(float));
    memmove(ds->dsY->a + inSampleIndex + inCount, ds->dsY->a + inSampleIndex, inCount*sizeof(float));
  }
  memcpy(ds->dsX->a + inSampleIndex, inX, inCount*sizeof(float));
  memcpy(ds->dsY->a + inSampleIndex, inY, inCount*sizeof(float));
}

void IupMglPlotInsert3D(Ihandle* ih, int inIndex, int inSampleIndex, const float* inX, const float* inY, const float* inZ, int inCount)
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
    memmove(ds->dsX->a + inSampleIndex + inCount, ds->dsX->a + inSampleIndex, inCount*sizeof(float));
    memmove(ds->dsY->a + inSampleIndex + inCount, ds->dsY->a + inSampleIndex, inCount*sizeof(float));
    memmove(ds->dsZ->a + inSampleIndex + inCount, ds->dsZ->a + inSampleIndex, inCount*sizeof(float));
  }
  memcpy(ds->dsX->a + inSampleIndex, inX, inCount*sizeof(float));
  memcpy(ds->dsY->a + inSampleIndex, inY, inCount*sizeof(float));
  memcpy(ds->dsZ->a + inSampleIndex, inZ, inCount*sizeof(float));
}

void IupMglPlotSet1D(Ihandle* ih, int inIndex, const char** inNames, const float* inX, int inCount)
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

void IupMglPlotSet2D(Ihandle* ih, int inIndex, const float *inX, const float *inY, int inCount)
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

void IupMglPlotSet3D(Ihandle* ih, int inIndex, const float* inX, const float* inY, const float* inZ, int inCount)
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

void IupMglPlotSetData(Ihandle* ih, int inIndex, const float* data, int count_x, int count_y, int count_z)
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

void IupMglPlotTransform(Ihandle* ih, float x, float y, float z, int *ix, int *iy)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "mglplot"))
    return;

  ih->data->mgl->CalcScr(mglPoint(x, y, z), ix, iy);
}

void IupMglPlotTransformXYZ(Ihandle* ih, int ix, int iy, float *x, float *y, float *z)
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

void IupMglPlotPaintTo(Ihandle* ih, const char* format, int w, int h, float dpi, void *data)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
    !IupClassMatch(ih, "mglplot"))
    return;

  if (!format || !data || w<=0 || h<=0)
    return;

  int old_w = ih->data->w;
  int old_h = ih->data->h;
  float old_dpi = ih->data->dpi;
  ih->data->w = w;
  ih->data->h = h;
  ih->data->dpi = dpi;

  if (dpi == 0)
    ih->data->dpi = old_dpi;

  if (*format == 'E') //EPS
  {
    char* filename = (char*)data;

    mglGraphPS* gr = new mglGraphPS(w, h);
    iMglPlotDrawPlot(ih, gr);
    gr->WriteEPS(filename, "IupMglPlot");

    delete gr;
  }
  else if (*format == 'S') //SVG
  {
    char* filename = (char*)data;

    mglGraphPS* gr = new mglGraphPS(w, h);
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

    mglGraphZB* gr = new mglGraphZB(w, h);
    iMglPlotDrawPlot(ih, gr);
    if (alpha)
      input_bits = gr->GetRGBA();
    else
      input_bits = gr->GetBits();
    memcpy(output_bits, input_bits, w*h*(alpha? 4:3));

    delete gr;
  }

  ih->data->w = old_w;
  ih->data->h = old_h;
  ih->data->dpi = old_dpi;
}

void IupMglPlotDrawMark(Ihandle* ih, float x, float y, float z)
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
  mglColor color;
  if (!value) color = ih->data->fgColor;
  else iMglPlotSetColor(ih, value, color);
  iMglPlotConfigColor(ih, gr, color);

  value = iupAttribGetStr(ih, "DRAWMARKSTYLE");
  char markstyle = 'x';
  iMglPlotSetMarkStyle(ih, value, markstyle);

  value = iupAttribGetStr(ih, "DRAWMARKSIZE");
  float marksize = 0.02f;
  iMglPlotSetFloat(ih, value, marksize);
  gr->SetMarkSize(marksize);

  gr->Mark(mglPoint(x, y, z), markstyle);
}

void IupMglPlotDrawLine(Ihandle* ih, float x1, float y1, float z1, float x2, float y2, float z2)
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
  mglColor color;
  if (!value) color = ih->data->fgColor;
  else iMglPlotSetColor(ih, value, color);

  value = iupAttribGetStr(ih, "DRAWLINESTYLE");
  char linestyle = '-';
  iMglPlotSetLineStyle(ih, value, linestyle);

  value = iupAttribGetStr(ih, "DRAWLINEWIDTH");
  float linewidth = 0;
  iupStrToFloat(value, &linewidth);
  if (linewidth<=0) linewidth = 1.0f;

  char pen[10];
  iMglPlotConfigColor(ih, gr, color);
  iMglPlotConfigPen(gr, pen, linestyle, linewidth);
  gr->Line(mglPoint(x1, y1, z1), mglPoint(x2, y2, z2), pen);
}

void IupMglPlotDrawText(Ihandle* ih, const char* text, float x, float y, float z)
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
  mglColor color;
  if (!value) color = ih->data->fgColor;
  else iMglPlotSetColor(ih, value, color);

  value = iupAttribGetStr(ih, "DRAWFONTSTYLE");
  int fontstyle = IUP_MGLPLOT_INHERIT;
  iMglPlotSetFontStyle(ih, value, fontstyle);

  value = iupAttribGetStr(ih, "DRAWFONTSIZE");
  float fontsize = 1.0f;
  iMglPlotSetFloat(ih, value, fontsize);

  iMglPlotConfigColor(ih, gr, color);
  iMglPlotConfigFont(ih, gr, fontstyle, fontsize);

  gr->Puts(mglPoint(x, y, z), text, NULL, -1);
}

/******************************************************************************
 Canvas Callbacks
******************************************************************************/

static int iMglPlotResize_CB(Ihandle* ih, int width, int height)
{
  IupGLMakeCurrent(ih);
  glViewport(0, 0, width, height);

  ih->data->redraw = true;
  ih->data->w = width;
  ih->data->h = height;
  ih->data->dpi = IupGetFloat(NULL, "SCREENDPI");

  return IUP_DEFAULT;
}

static int iMglPlotRedraw_CB(Ihandle* ih)
{
  iMglPlotRepaint(ih, 0, 1);  /* full redraw only if nothing changed */
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

static void iMglPlotSetAxisDefaults(Iaxis& axis)
{
  axis.axLabelFontSizeFactor = 1;
  axis.axTickFontSizeFactor = 0.8f;

  axis.axColor.Set(NAN, NAN, NAN);

  axis.axAutoScaleMin = true;
  axis.axAutoScaleMax = true;
  axis.axMax = 1;
  axis.axShow = true;
  axis.axShowArrow = true;
  axis.axLabelRotation = true;

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

static int iMglPlotCreateMethod(Ihandle* ih, void **params)
{
  (void)params;

  /* free the data allocated by IupCanvas */
  free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  ih->data->mgl = new mglGraphGL();

  ih->data->dataSetMaxCount = 5;
  ih->data->dataSet = (IdataSet*)malloc(sizeof(IdataSet)*ih->data->dataSetMaxCount);
  memset(ih->data->dataSet, 0, sizeof(IdataSet)*ih->data->dataSetMaxCount);

  /* IupCanvas callbacks */
  IupSetCallback(ih, "RESIZE_CB", (Icallback)iMglPlotResize_CB);
  IupSetCallback(ih, "ACTION",    (Icallback)iMglPlotRedraw_CB);
// TODO
//  IupSetCallback(ih, "BUTTON_CB",   (Icallback)iMglPlotMouseButton_CB);
//  IupSetCallback(ih, "MOTION_CB",   (Icallback)iMglPlotMouseMove_CB);
//  IupSetCallback(ih, "KEYPRESS_CB", (Icallback)iMglPlotKeyPress_CB);

  IupSetAttribute(ih, "BUFFER", "DOUBLE");

  // Default values
  ih->data->dataSetCurrent = -1;
  ih->data->redraw = true;
  ih->data->legendPosition = IUP_MGLPLOT_TOPRIGHT;
  ih->data->legendBox = true;
  ih->data->boxTicks = true;
  ih->data->alpha = true;

  ih->data->legendFontSizeFactor = 0.8f;
  ih->data->titleFontSizeFactor = 1.6f;

  ih->data->bgColor.Set(1, 1, 1);
  ih->data->gridColor.Set(iRecon(200), iRecon(200), iRecon(200));
  ih->data->titleColor.Set(NAN, NAN, NAN);
  ih->data->legendColor.Set(NAN, NAN, NAN);
  ih->data->boxColor.Set(NAN, NAN, NAN);

  iMglPlotSetAxisDefaults(ih->data->xAxis);
  iMglPlotSetAxisDefaults(ih->data->yAxis);
  iMglPlotSetAxisDefaults(ih->data->zAxis);

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
//   iupClassRegisterCallback(ic, "EDIT_CB", "iiffvv");
//   iupClassRegisterCallback(ic, "EDITBEGIN_CB", "");
//   iupClassRegisterCallback(ic, "EDITEND_CB", "");

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", iMglPlotGetBGColorAttrib, iMglPlotSetBGColorAttrib, "255 255 255", NULL, IUPAF_NOT_MAPPED);   /* overwrite canvas implementation, set a system default to force a new default */
  iupClassRegisterAttribute(ic, "FGCOLOR", iMglPlotGetFGColorAttrib, iMglPlotSetFGColorAttrib, IUPAF_SAMEASSYSTEM, "0 0 0", IUPAF_NOT_MAPPED);

  /* IupMglPlot only */
  iupClassRegisterAttribute(ic, "REDRAW", NULL, iMglPlotSetRedrawAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ALPHA", iMglPlotGetAlphaAttrib, iMglPlotSetAlphaAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ANTIALIAS", iMglPlotGetAntialiasAttrib, iMglPlotSetAntialiasAttrib, "Yes", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MGLFONT", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

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
  iupClassRegisterAttribute(ic, "DS_LEGEND", iMglPlotGetDSLegendAttrib, iMglPlotSetDSLegendAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_COLOR", iMglPlotGetDSColorAttrib, iMglPlotSetDSColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_SHOWVALUES", iMglPlotGetDSShowValuesAttrib, iMglPlotSetDSShowValuesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_MODE", iMglPlotGetDSModeAttrib, iMglPlotSetDSModeAttrib, IUPAF_SAMEASSYSTEM, "LINE", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_EDIT", iMglPlotGetDSEditAttrib, iMglPlotSetDSEditAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_REMOVE", NULL, iMglPlotSetDSRemoveAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_COUNT", iMglPlotGetDSCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DS_DIMENSION", iMglPlotGetDSDimAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "MARKS", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DATAGRID", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PLANARVALUE", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "GRADLINESCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXIALCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "3", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONTOURFILLED", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONTOURCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "7", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DIR", NULL, NULL, IUPAF_SAMEASSYSTEM, "Y", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLOUDCUBES", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "OVERALLALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "1.0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEX", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEY", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEZ", NULL, NULL, IUPAF_SAMEASSYSTEM, "-1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLICEDIR", NULL, NULL, IUPAF_SAMEASSYSTEM, "XYZ", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECTVALUEX", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECTVALUEY", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECTVALUEZ", NULL, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PROJECT", NULL, NULL, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ISOCOUNT", NULL, NULL, IUPAF_SAMEASSYSTEM, "3", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

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
  iupClassRegisterAttribute(ic, "AXS_XMIN", iMglPlotGetAxisXMinAttrib, iMglPlotSetAxisXMinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YMIN", iMglPlotGetAxisYMinAttrib, iMglPlotSetAxisYMinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZMIN", iMglPlotGetAxisZMinAttrib, iMglPlotSetAxisZMinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
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
  iupClassRegisterAttribute(ic, "AXS_XCROSSORIGIN", iMglPlotGetAxisXCrossOriginAttrib, iMglPlotSetAxisXCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_YCROSSORIGIN", iMglPlotGetAxisYCrossOriginAttrib, iMglPlotSetAxisYCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AXS_ZCROSSORIGIN", iMglPlotGetAxisZCrossOriginAttrib, iMglPlotSetAxisZCrossOriginAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
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

  return ic;
}

Ihandle* IupMglPlot(void)
{
  return IupCreate("mglplot");
}

void IupMglPlotOpen(void)
{
  IupGLCanvasOpen();

  if (!IupGetGlobal("_IUP_IUP_MGLPLOT_OPEN"))
  {
    iupRegisterClass(iMglPlotNewClass());
    IupSetGlobal("_IUP_IUP_MGLPLOT_OPEN", "1");
  }
}

/* TODO
Rafael:
  Callbacks
  Interaction
  DS_EDIT
	gr->View(tet,phi);   3D
	gr->Perspective(per);  3D
	gr->Zoom(x1,y1,x2,y2);  2D and 3D 

Depois:
  teste IupMglPlotLoadData e IupMglPlotSetFromFormula, 
  teste IupMglPlotPaintTo  SVG, EPS e RGB
  teste BOLD e ITALIC
  explicar pixels x plot x normalized coordinates
  documentar suporte a Tek formulas nas strings
  exemplos com os recursos novos
  Binding Lua
  rever IupGraph
  Light
  Rotate
  Colorbar/Cmin-Cmax
  TranspType/Transparent
  reference datasets
  Legend

Talvez:
  cutting
  curvilinear coordinates
  Ternary
  StickPlot

MathGL:
  How to improve text quality?  native fonts?
  lib adicional para carregar direto TTF e OTF via FreeType
  outras typefaces?
  "The font type (STIX, arial, courier, times and so on) can be selected by function SetFont"???
  FontSize units?
  too slow to load font, binary format
  roman is not the same thing as regular
  bug in make_font
  Driver CD?
  FTGL/Freetype?
  option to draw an opaque background for text
  data text file using ';' or '\t' also
  Doc samples in a zip
  legend is not been displayed in OpenGL.
  mudanças no código da MathGL
  API da MathGL
  font size in [0-1]
  esticar o gráfico, mas não esticar as fontes, manter aspecto, também possibilidade para manter fixo tamanho
  TicksVal should follow ticks spacing configuration
  documentation says negative len puts tic outside the bounding box, but it is NOT working
  somethimes the label gets too close to the ticks
    it can be manualy moved by changing the origin
  rotation along axis is only for Y axis?
  position axis, acording to title, ticks and labels
  // for 72dpi, FontSize=6 --> 26pt
  void mglGraphAB::SetFontSizePT(mreal pt, int dpi)
  {  FontSize = 16.6154*pt/dpi;  }
  void mglGraph::SetFontSizePT(mreal pt, int dpi)
  {  FontSize = pt*27.f/dpi;  }
*/
