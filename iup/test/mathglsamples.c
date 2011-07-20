/*
MathGL samples w/ IupMglPlot
Based on: MathGL documentation (v. 1.10), Cap. 9
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "iup.h"
#include "iupcontrols.h"
#include "iup_mglplot.h"
#include "iupkey.h"

#include <cd.h>
#include <cdiup.h>

static Ihandle *plot,           /* plot */
               *dial1, *dial2,  /* dials for zooming */
               *tgg1, *tgg2,    /* auto scale on|off toggles */
               *tgg3, *tgg4,    /* grid show|hide toggles */
               *tgg5,           /* legend show|hide toggle */
               *tgg6,           /* box show|hide toggle */
               *tgg7,           /* antialias enable|disable toggle */
               *tgg8,           /* transparent enable|disable toggle */
               *tgg9,           /* light enable|disable toggle */
               *tgg10;          /* opengl enable|disable toggle */

static char filenameSVG[100], filenameEPS[100];

static void ResetClear(void)
{
  IupSetAttribute(plot, "RESET", NULL);
  IupSetAttribute(plot, "CLEAR", NULL);

  // Some defaults in MathGL are different in IupMglPlot
  IupSetAttribute(plot, "AXS_X", "NO");
  IupSetAttribute(plot, "AXS_Y", "NO");
  IupSetAttribute(plot, "AXS_Z", "NO");
}

static void SampleVolume(const char* ds_mode)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "-2*((2*x-1)^2 + (2*y-1)^2 + (2*z-1)^4 - (2*z-1)^2 - 0.1)", 60, 50, 40);
  IupSetAttribute(plot, "DS_MODE", ds_mode);

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "TRANSPARENT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleDensityProjectVolume(void)
{
  SampleVolume("VOLUME_DENSITY");
  IupSetAttribute(plot, "PROJECT", "YES");
  IupSetAttribute(plot, "PROJECTVALUEX", "-1");
  IupSetAttribute(plot, "PROJECTVALUEY", "1");
  IupSetAttribute(plot, "PROJECTVALUEZ", "-1");
  IupSetAttribute(plot, "TRANSPARENT", "NO");
  IupSetAttribute(plot, "LIGHT", "NO");
  //IupSetAttribute(plot, "AXS_XORIGIN", "0");
  //IupSetAttribute(plot, "AXS_YORIGIN", "0");
  //IupSetAttribute(plot, "AXS_ZORIGIN", "0");
}

static void SampleContourProjectVolume(void)
{
  SampleVolume("VOLUME_CONTOUR");
  IupSetAttribute(plot, "PROJECT", "YES");
  IupSetAttribute(plot, "PROJECTVALUEX", "-1");
  IupSetAttribute(plot, "PROJECTVALUEY", "1");
  IupSetAttribute(plot, "PROJECTVALUEZ", "-1");
  IupSetAttribute(plot, "TRANSPARENT", "NO");
  IupSetAttribute(plot, "LIGHT", "NO");
}

static void SampleContourFilledProjectVolume(void)
{
  SampleVolume("VOLUME_CONTOUR");
  IupSetAttribute(plot, "PROJECT", "YES");
  IupSetAttribute(plot, "PROJECTVALUEX", "-1");
  IupSetAttribute(plot, "PROJECTVALUEY", "1");
  IupSetAttribute(plot, "PROJECTVALUEZ", "-1");
  IupSetAttribute(plot, "CONTOURFILLED", "YES");
  IupSetAttribute(plot, "TRANSPARENT", "NO");
  IupSetAttribute(plot, "LIGHT", "NO");
}

static void SampleContourFilledVolume(void)
{
  SampleVolume("VOLUME_CONTOUR");
  IupSetAttribute(plot, "CONTOURFILLED", "YES");
  IupSetAttribute(plot, "TRANSPARENT", "NO");
}

static void SampleContourVolume(void)
{
  SampleVolume("VOLUME_CONTOUR");
  IupSetAttribute(plot, "TRANSPARENT", "NO");
  IupSetAttribute(plot, "LIGHT", "NO");
}

static void SampleDensityVolume(void)
{
  SampleVolume("VOLUME_DENSITY");
  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "AXS_ZORIGIN", "0");
  IupSetAttribute(plot, "AXS_X", "Yes");
  IupSetAttribute(plot, "AXS_Y", "Yes");
  IupSetAttribute(plot, "AXS_Z", "Yes");
  IupSetAttribute(plot, "LIGHT", "NO");
}

static void SampleCloudVolume(void)
{
  SampleVolume("VOLUME_CLOUD");
  IupSetAttribute(plot, "COLORSCHEME", "wyrRk");
  IupSetAttribute(plot, "CLOUDCUBES", "NO");
  IupSetAttribute(plot, "LIGHT", "NO");
}

static void SampleCloudCubesVolume(void)
{
  SampleVolume("VOLUME_CLOUD");
  IupSetAttribute(plot, "COLORSCHEME", "wyrRk");
  IupSetAttribute(plot, "LIGHT", "NO");
}

static void SampleIsoSurfaceVolume(void)
{
  SampleVolume("VOLUME_ISOSURFACE");
}

static void SamplePlanar(const char* ds_mode)
{
  int ds = IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, ds, "0.6*sin(2*pi*x)*sin(3*pi*y) + 0.4*cos(3*pi*(x*y))", 50, 40, 1);
  IupSetAttribute(plot, "DS_MODE", ds_mode);

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleGradientLinesPlanar(void)
{
  SamplePlanar("PLANAR_GRADIENTLINES");
  SamplePlanar("PLANAR_DENSITY");
  IupSetAttribute(plot, "TRANSPARENT", "YES");
  IupSetAttribute(plot, "ROTATE", NULL);
}

static void SampleAxialContourPlanar(void)
{
  SamplePlanar("PLANAR_AXIALCONTOUR");
  IupSetAttribute(plot, "TRANSPARENT", "YES");
}

static void SampleContourFilledPlanar(void)
{
  SamplePlanar("PLANAR_CONTOUR");
  IupSetAttribute(plot, "CONTOURFILLED", "Yes");
}

static void SampleContourPlanar(void)
{
  SamplePlanar("PLANAR_CONTOUR");
}

static void SampleDensityPlanar(void)
{
  SamplePlanar("PLANAR_DENSITY");
  IupSetAttribute(plot, "COLORBAR", "Yes");
//  IupSetAttribute(plot, "COLORBARPOS", "TOP");
  IupSetAttribute(plot, "ROTATE", NULL);
}

static void SampleBoxesPlanar(void)
{
  SamplePlanar("PLANAR_BOXES");
  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "AXS_ZORIGIN", "0");
}

static void SampleTilePlanar(void)
{
  SamplePlanar("PLANAR_TILE");
}

static void SampleBeltPlanar(void)
{
  SamplePlanar("PLANAR_BELT");
}

static void SampleFallPlanar(void)
{
  SamplePlanar("PLANAR_FALL");
}

static void SampleMeshPlanar(void)
{
  SamplePlanar("PLANAR_MESH");
}

static void SampleSurfaceColorsPlanar(void)
{
  SamplePlanar("PLANAR_SURFACE");
  IupSetAttribute(plot, "COLORSCHEME", "BbcyrR|");
}

static void SampleSurfacePlanar(void)
{
  SamplePlanar("PLANAR_SURFACE");
}

static void SampleDotsLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotLoadData(plot, 0, "../test/hotdogs.pts", 0, 0, 0);
  IupSetAttribute(plot, "DS_MODE", "DOTS");
  IupSetAttribute(plot, "DS_REARRANGE", NULL);

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleCrustLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotLoadData(plot, 0, "../test/hotdogs.pts", 0, 0, 0);
  IupSetAttribute(plot, "DS_MODE", "CRUST");
  IupSetAttribute(plot, "DS_REARRANGE", NULL);

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SamplePieLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "rnd+0.1", 7, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "CHART");

  IupSetAttribute(plot, "COLORSCHEME", "bgr cmy");
  IupSetAttribute(plot, "PIECHART", "Yes");
  IupSetAttribute(plot, "DATAGRID", "Yes");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleChartLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "rnd+0.1", 7, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "CHART");
  IupSetAttribute(plot, "DATAGRID", "Yes");

  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleStemLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x) + 0.5*cos(3*pi*x) + 0.2*sin(pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "STEM");
  IupSetAttribute(plot, "DS_MARKSTYLE", "HOLLOW_CIRCLE");
  IupSetAttribute(plot, "DS_SHOWMARKS", "Yes");

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "STEM");
  IupSetAttribute(plot, "DS_MARKSTYLE", "HOLLOW_CIRCLE");
  IupSetAttribute(plot, "DS_SHOWMARKS", "Yes");

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "STEM");
  IupSetAttribute(plot, "DS_MARKSTYLE", "HOLLOW_CIRCLE");
  IupSetAttribute(plot, "DS_SHOWMARKS", "Yes");

  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleStepLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x) + 0.5*cos(3*pi*x) + 0.2*sin(pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "STEP");

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "STEP");

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "STEP");

  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleBarhLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.8*sin(pi*(2*x+y/2))+0.2*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "BARHORIZONTAL");

  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleBarsLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.8*sin(pi*(2*x+y/2))+0.2*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "BAR");

  //TODO: allow combination of several datasets into one bar plot
  //TODO: allow bars "above" sample

  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleAreaLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x) + 0.5*cos(3*pi*x) + 0.2*sin(pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "AREA");

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "AREA");

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "AREA");
  
  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleRadarLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.4*sin(pi*(2*x+y/2))+0.1*rnd", 10, 3, 1);
  IupSetAttribute(plot, "DS_MODE", "RADAR");
  IupSetAttribute(plot, "DS_SPLIT", NULL);

  IupSetAttribute(plot, "CURRENT", "1");
  IupSetAttribute(plot, "DS_MODE", "RADAR");

  IupSetAttribute(plot, "CURRENT", "2");
  IupSetAttribute(plot, "DS_MODE", "RADAR");

  IupSetAttribute(plot, "RADARSHIFT", "0.4");  // So all datasets will use the same radarshift
  IupSetAttribute(plot, "DATAGRID", "Yes");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SamplePlotLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x)+0.5*cos(3*pi*x)+0.2*sin(pi*x)", 50, 1, 1);

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);

  IupSetAttribute(plot, "BOX", "YES");
}

static void Dummy(void)
{
}

typedef struct _TestItems{
  char* title;
  void (*func)(void);
}TestItems;

static TestItems test_list[] = {
  {"Plot (Linear)", SamplePlotLinear},
  {"Radar (Linear)", SampleRadarLinear},
  {"Area (Linear)", SampleAreaLinear},
  {"Bars (Linear)", SampleBarsLinear}, 
  {"Barh (Linear)", SampleBarhLinear}, 
  {"Step (Linear)", SampleStepLinear}, 
  {"Stem (Linear)", SampleStemLinear}, 
  {"Chart (Linear)", SampleChartLinear}, 
  {"Pie (Linear)", SamplePieLinear}, 
  {"Dots (Linear)", SampleDotsLinear}, 
  {"Crust (Linear)", SampleCrustLinear},

  {"----------", Dummy},
                 
  {"Surface (Planar)", SampleSurfacePlanar},
  {"Surface Colors (Planar)", SampleSurfaceColorsPlanar},
  {"Mesh (Planar)", SampleMeshPlanar},
  {"Fall (Planar)", SampleFallPlanar},
  {"Belt (Planar)", SampleBeltPlanar},
  {"Tile (Planar)", SampleTilePlanar},
  {"Boxes (Planar)", SampleBoxesPlanar},
  {"Density (Planar)", SampleDensityPlanar},
  {"Contour (Planar)", SampleContourPlanar},
  {"Contour Filled (Planar)", SampleContourFilledPlanar},
  {"Axial Contour (Planar)", SampleAxialContourPlanar},
  {"GradientLines (Planar)", SampleGradientLinesPlanar},

  {"----------", Dummy},

  {"Iso Surface (Volume)", SampleIsoSurfaceVolume},
  {"CloudCubes (Volume)", SampleCloudCubesVolume},
  {"Cloud (Volume)", SampleCloudVolume},
  {"Density (Volume)", SampleDensityVolume},
  {"Contour (Volume)", SampleContourVolume},
  {"ContourFilled (Volume)", SampleContourFilledVolume},
  {"ContourProject (Volume)", SampleContourProjectVolume},
  {"ContourFilledProject (Volume)", SampleContourFilledProjectVolume},
  {"DensityProject (Volume)", SampleDensityProjectVolume},
};

static int k_enter_cb(Ihandle*ih)
{
  int pos = IupGetInt(ih, "VALUE");
  if (pos > 0)
  {
    ResetClear();
    test_list[pos-1].func();
    IupSetAttribute(plot, "REDRAW", NULL);
  }
  return IUP_DEFAULT;
}

static int changed_cb(Ihandle *ih)
{
  char *value;
  (void)ih;

  /* auto scaling Y axis */
  if (IupGetInt(plot, "AXS_YAUTOMIN") && IupGetInt(plot, "AXS_YAUTOMAX")) 
  {
    IupSetAttribute(tgg1, "VALUE", "ON");
    IupSetAttribute(dial1, "ACTIVE", "NO");
  }
  else 
  {
    IupSetAttribute(tgg1, "VALUE", "OFF");
    IupSetAttribute(dial1, "ACTIVE", "YES");
  }

  /* auto scaling X axis */
  if (IupGetInt(plot, "AXS_XAUTOMIN") && IupGetInt(plot, "AXS_XAUTOMAX")) 
  {
    IupSetAttribute(tgg2, "VALUE", "ON");
    IupSetAttribute(dial2, "ACTIVE", "NO");
  }
  else 
  {
    IupSetAttribute(tgg2, "VALUE", "OFF");
    IupSetAttribute(dial2, "ACTIVE", "YES");
  }

  /* grid */
  value = IupGetAttribute(plot, "GRID");
  if (value && strstr(value, "X"))
    IupSetAttribute(tgg3, "VALUE", "ON");
  else
    IupSetAttribute(tgg3, "VALUE", "OFF");

  if (value && strstr(value, "Y"))
    IupSetAttribute(tgg4, "VALUE", "ON");
  else
    IupSetAttribute(tgg4, "VALUE", "OFF");

  /* legend */
  if (IupGetInt(plot, "LEGEND"))
    IupSetAttribute(tgg5, "VALUE", "ON");
  else
    IupSetAttribute(tgg5, "VALUE", "OFF");

  /* box */
  if (IupGetInt(plot, "BOX"))
    IupSetAttribute(tgg6, "VALUE", "ON");
  else
    IupSetAttribute(tgg6, "VALUE", "OFF");

  /* antialias */
  if (IupGetInt(plot, "ANTIALIAS"))
    IupSetAttribute(tgg7, "VALUE", "ON");
  else
    IupSetAttribute(tgg7, "VALUE", "OFF");

  /* transparent */
  if (IupGetInt(plot, "TRANSPARENT"))
    IupSetAttribute(tgg8, "VALUE", "ON");
  else
    IupSetAttribute(tgg8, "VALUE", "OFF");

  /* light */
  if (IupGetInt(plot, "LIGHT"))
    IupSetAttribute(tgg9, "VALUE", "ON");
  else
    IupSetAttribute(tgg9, "VALUE", "OFF");

  /* opengl */
  if (IupGetInt(plot, "OPENGL"))
    IupSetAttribute(tgg10, "VALUE", "ON");
//   else
//     IupSetAttribute(tgg10, "VALUE", "OFF");

  return IUP_DEFAULT;
}

static int action_cb(Ihandle *ih, char *text, int item, int state)
{
  (void)text;
  (void)ih;

  if (state==1)
  {
    ResetClear();
    test_list[item-1].func();
    sprintf(filenameSVG, "../%s.svg", test_list[item-1].title);
    sprintf(filenameEPS, "../%s.eps", test_list[item-1].title);
    IupSetAttribute(plot, "REDRAW", NULL);
  }

  return IUP_DEFAULT;
}

static int close_cb(Ihandle *ih)
{
  (void)ih;
  return IUP_CLOSE;
}

/* Y zoom */
static int dial1_btndown_cb(Ihandle *self, double angle)
{
  (void)angle;
  (void)self;

  IupStoreAttribute(plot, "OLD_YMIN", IupGetAttribute(plot, "AXS_YMIN"));
  IupStoreAttribute(plot, "OLD_YMAX", IupGetAttribute(plot, "AXS_YMAX"));

  return IUP_DEFAULT;
}

static int dial1_btnup_cb(Ihandle *self, double angle)
{
  double x1, x2, xm;
  char *ss;
  (void)self;

  x1 = IupGetFloat(plot, "OLD_YMIN");
  x2 = IupGetFloat(plot, "OLD_YMAX");

  ss = IupGetAttribute(plot, "AXS_YMODE");
  if ( ss && ss[3]=='2' )
  {
    /* LOG2:  one circle will zoom 2 times */
    xm = 4.0 * fabs(angle) / 3.141592;
    if (angle>0.0) { x2 /= xm; x1 *= xm; }
    else { x2 *= xm; x1 /= xm; }
  }

  if ( ss && ss[3]=='1' )
  {
    /* LOG10:  one circle will zoom 10 times */
    xm = 10.0 * fabs(angle) / 3.141592;
    if (angle>0.0) { x2 /= xm; x1 *= xm; }
    else { x2 *= xm; x1 /= xm; }
  }
  else
  {
    /* LIN: one circle will zoom 2 times */
    xm = (x1 + x2) / 2.0;
    x1 = xm - (xm - x1)*(1.0-angle*1.0/3.141592);
    x2 = xm + (x2 - xm)*(1.0-angle*1.0/3.141592);
  }

  if (x1<x2)
  {
    IupSetfAttribute(plot, "AXS_YMIN", "%g", x1);
    IupSetfAttribute(plot, "AXS_YMAX", "%g", x2);
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* X zoom */
static int dial2_btndown_cb(Ihandle *self, double angle)
{
  (void)angle;
  (void)self;

  IupStoreAttribute(plot, "OLD_XMIN", IupGetAttribute(plot, "AXS_XMIN"));
  IupStoreAttribute(plot, "OLD_XMAX", IupGetAttribute(plot, "AXS_XMAX"));

  return IUP_DEFAULT;
}

static int dial2_btnup_cb(Ihandle *self, double angle)
{
  double x1, x2, xm;
  (void)self;

  x1 = IupGetFloat(plot, "OLD_XMIN");
  x2 = IupGetFloat(plot, "OLD_XMAX");

  xm = (x1 + x2) / 2.0;

  x1 = xm - (xm - x1)*(1.0-angle*1.0/3.141592); /* one circle will zoom 2 times */
  x2 = xm + (x2 - xm)*(1.0-angle*1.0/3.141592);

  IupSetfAttribute(plot, "AXS_XMIN", "%g", x1);
  IupSetfAttribute(plot, "AXS_XMAX", "%g", x2);

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* auto scale Y */
static int tgg1_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
  {
    IupSetAttribute(dial1, "ACTIVE", "NO");
    IupSetAttribute(plot, "AXS_YAUTOMIN", "YES");
    IupSetAttribute(plot, "AXS_YAUTOMAX", "YES");
  }
  else
  {
    IupSetAttribute(dial1, "ACTIVE", "YES");
    IupSetAttribute(plot, "AXS_YAUTOMIN", "NO");
    IupSetAttribute(plot, "AXS_YAUTOMAX", "NO");
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* auto scale X */
static int tgg2_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
  {
    IupSetAttribute(dial2, "ACTIVE", "NO");
    IupSetAttribute(plot, "AXS_XAUTOMIN", "YES");
    IupSetAttribute(plot, "AXS_XAUTOMAX", "YES");
  }
  else
  {
    IupSetAttribute(dial2, "ACTIVE", "YES");
    IupSetAttribute(plot, "AXS_XAUTOMIN", "NO");
    IupSetAttribute(plot, "AXS_XAUTOMAX", "NO");
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* show/hide V grid */
static int tgg3_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
  {
    if (IupGetInt(tgg4, "VALUE"))
      IupSetAttribute(plot, "GRID", "YES");
    else
      IupSetAttribute(plot, "GRID", "VERTICAL");
  }
  else
  {
    if (!IupGetInt(tgg4, "VALUE"))
      IupSetAttribute(plot, "GRID", "NO");
    else
      IupSetAttribute(plot, "GRID", "HORIZONTAL");
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* show/hide H grid */
static int tgg4_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
  {
    if (IupGetInt(tgg3, "VALUE"))
      IupSetAttribute(plot, "GRID", "YES");
    else
      IupSetAttribute(plot, "GRID", "HORIZONTAL");
  }
  else
  {
    if (!IupGetInt(tgg3, "VALUE"))
      IupSetAttribute(plot, "GRID", "NO");
    else
      IupSetAttribute(plot, "GRID", "VERTICAL");
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* show/hide legend */
static int tgg5_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "LEGEND", "YES");
  else
    IupSetAttribute(plot, "LEGEND", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* show/hide box */
static int tgg6_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "BOX", "YES");
  else
    IupSetAttribute(plot, "BOX", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* enable/disable antialias */
static int tgg7_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "ANTIALIAS", "YES");
  else
    IupSetAttribute(plot, "ANTIALIAS", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* enable/disable transparent */
static int tgg8_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "TRANSPARENT", "YES");
  else
    IupSetAttribute(plot, "TRANSPARENT", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* enable/disable light */
static int tgg9_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "LIGHT", "YES");
  else
    IupSetAttribute(plot, "LIGHT", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* enable/disable opengl */
static int tgg10_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "OPENGL", "YES");
//   else
//     IupSetAttribute(plot, "OPENGL", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

static int bt1_cb(Ihandle *self)
{
  (void)self;

  IupMglPlotPaintTo(plot, "SVG", 0, 0, 0, filenameSVG);
  return IUP_DEFAULT;
}

static int bt2_cb(Ihandle *self)
{
  (void)self;

  IupMglPlotPaintTo(plot, "EPS", 0, 0, 0, filenameEPS);
  return IUP_DEFAULT;
}

Ihandle* controlPanel(void)
{
  Ihandle *vboxl, *lbl1, *lbl2, *lbl3, *lbl4, *bt1, *bt2,
    *boxinfo, *boxdial1, *boxdial2, *f1, *f2;

  /* left panel: plot control
     Y zooming               */
  dial1 = IupDial("VERTICAL");
  lbl1 = IupLabel("+");
  lbl2 = IupLabel("-");
  boxinfo = IupVbox(lbl1, IupFill(), lbl2, NULL);
  boxdial1 = IupHbox(boxinfo, dial1, NULL);

  IupSetAttribute(boxdial1, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "SIZE", "20x52");
  IupSetAttribute(boxinfo, "GAP", "2");
  IupSetAttribute(boxinfo, "MARGIN", "2");
  IupSetAttribute(boxinfo, "EXPAND", "YES");
  IupSetAttribute(lbl1, "EXPAND", "NO");
  IupSetAttribute(lbl2, "EXPAND", "NO");

  IupSetAttribute(dial1, "ACTIVE", "NO");
  IupSetAttribute(dial1, "SIZE", "20x52");
  IupSetCallback(dial1, "BUTTON_PRESS_CB", (Icallback)dial1_btndown_cb);
  IupSetCallback(dial1, "MOUSEMOVE_CB", (Icallback)dial1_btnup_cb);
  IupSetCallback(dial1, "BUTTON_RELEASE_CB", (Icallback)dial1_btnup_cb);

  tgg1 = IupToggle("Y Autoscale", NULL);
   IupSetCallback(tgg1, "ACTION", (Icallback)tgg1_cb);
  IupSetAttribute(tgg1, "VALUE", "ON");

  f1 = IupFrame( IupVbox(boxdial1, tgg1, NULL) );
  IupSetAttribute(f1, "TITLE", "Y Zoom");

  /* X zooming */
  dial2 = IupDial("HORIZONTAL");
  lbl1 = IupLabel("-");
  lbl2 = IupLabel("+");
  boxinfo = IupHbox(lbl1, IupFill(), lbl2, NULL);
  boxdial2 = IupVbox(dial2, boxinfo, NULL);

  IupSetAttribute(boxdial2, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "SIZE", "64x16");
  IupSetAttribute(boxinfo, "GAP", "2");
  IupSetAttribute(boxinfo, "MARGIN", "2");
  IupSetAttribute(boxinfo, "EXPAND", "HORIZONTAL");

  IupSetAttribute(lbl1, "EXPAND", "NO");
  IupSetAttribute(lbl2, "EXPAND", "NO");

  IupSetAttribute(dial2, "ACTIVE", "NO");
  IupSetAttribute(dial2, "SIZE", "64x16");
  IupSetCallback(dial2, "BUTTON_PRESS_CB", (Icallback)dial2_btndown_cb);
  IupSetCallback(dial2, "MOUSEMOVE_CB", (Icallback)dial2_btnup_cb);
  IupSetCallback(dial2, "BUTTON_RELEASE_CB", (Icallback)dial2_btnup_cb);

  tgg2 = IupToggle("X Autoscale", NULL);
  IupSetCallback(tgg2, "ACTION", (Icallback)tgg2_cb);

  f2 = IupFrame( IupVbox(boxdial2, tgg2, NULL) );
  IupSetAttribute(f2, "TITLE", "X Zoom");

  lbl1 = IupLabel("");
  IupSetAttribute(lbl1, "SEPARATOR", "HORIZONTAL");

  tgg3 = IupToggle("Vertical Grid", NULL);
  IupSetCallback(tgg3, "ACTION", (Icallback)tgg3_cb);
  tgg4 = IupToggle("Horizontal Grid", NULL);
  IupSetCallback(tgg4, "ACTION", (Icallback)tgg4_cb);

  tgg6 = IupToggle("Box", NULL);
  IupSetCallback(tgg6, "ACTION", (Icallback)tgg6_cb);

  lbl2 = IupLabel("");
  IupSetAttribute(lbl2, "SEPARATOR", "HORIZONTAL");

  tgg5 = IupToggle("Legend", NULL);
  IupSetCallback(tgg5, "ACTION", (Icallback)tgg5_cb);

  lbl3 = IupLabel("");
  IupSetAttribute(lbl3, "SEPARATOR", "HORIZONTAL");

  tgg7 = IupToggle("Antialias", NULL);
  IupSetCallback(tgg7, "ACTION", (Icallback)tgg7_cb);

  tgg8 = IupToggle("Transparent", NULL);
  IupSetCallback(tgg8, "ACTION", (Icallback)tgg8_cb);

  tgg9 = IupToggle("Light", NULL);
  IupSetCallback(tgg9, "ACTION", (Icallback)tgg9_cb);

  tgg10 = IupToggle("OpenGL", NULL);
  IupSetCallback(tgg10, "ACTION", (Icallback)tgg10_cb);

  lbl4 = IupLabel("");
  IupSetAttribute(lbl4, "SEPARATOR", "HORIZONTAL");

  bt1 = IupButton("Export SVG", NULL);
  IupSetCallback(bt1, "ACTION", (Icallback)bt1_cb);

  bt2 = IupButton("Export EPS", NULL);
  IupSetCallback(bt2, "ACTION", (Icallback)bt2_cb);

  vboxl = IupVbox(f1, f2, lbl1, tgg3, tgg4, tgg6, lbl2, tgg5, lbl3, tgg7, tgg8, tgg9, tgg10, lbl4, IupHbox(bt1, bt2, NULL), NULL);
  IupSetAttribute(vboxl, "GAP", "4");
  IupSetAttribute(vboxl, "EXPAND", "NO");

  return vboxl;
}

int main(int argc, char* argv[])
{
  int i, count = sizeof(test_list)/sizeof(TestItems);
  char str[50];
  Ihandle *dlg, *list, *panel;

  IupOpen(&argc, &argv);
  IupControlsOpen();
  IupMglPlotOpen();     /* init IupMGLPlot library */

  IupSetGlobal("MGLFONTS", "../etc/mglfonts");

  list = IupList(NULL);
  plot = IupMglPlot();
  panel = controlPanel();

  dlg = IupDialog(IupHbox(list, panel, plot, NULL));
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "10");
  IupSetAttribute(dlg, "TITLE", "MathGL samples w/ IupMglPlot");
  IupSetCallback(dlg, "CLOSE_CB", close_cb);

  IupSetAttribute(plot, "RASTERSIZE", "500x500");

  IupSetAttribute(list, "EXPAND", "VERTICAL");
  IupSetAttribute(list, "VISIBLELINES", "35");
  IupSetAttribute(list, "VISIBLECOLUMNS", "15");
  IupSetCallback(list, "ACTION", (Icallback)action_cb);
  IupSetCallback(list, "VALUECHANGED_CB", (Icallback)changed_cb);

  for (i=0; i<count; i++)
  {
    sprintf(str, "%d", i+1);
    IupSetAttribute(list, str, test_list[i].title);
  }

  IupSetAttribute(list, "VALUE", "1");

  IupShowXY(dlg, 100, IUP_CENTER);

  IupSetAttribute(plot, "RASTERSIZE", NULL);

  ResetClear();
  SamplePlotLinear();
  IupSetAttribute(plot, "REDRAW", NULL);
  changed_cb(plot);

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
