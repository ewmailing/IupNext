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
               *minmaxY_dial, *minmaxX_dial,  /* dials for zooming */
               *autoscaleY_tgg, *autoscaleX_tgg,    /* auto scale on|off toggles */
               *grid_tgg,           /* grid show|hide toggles */
               *legend_tgg,           /* legend show|hide toggle */
               *box_tgg,           /* box show|hide toggle */
               *aa_tgg,           /* antialias enable|disable toggle */
               *transp_tgg,           /* transparent enable|disable toggle */
               *light_tgg,           /* light enable|disable toggle */
               *opengl_tgg;          /* opengl enable|disable toggle */

static char filenameSVG[300], filenameEPS[300];

static void ResetClear(void)
{
  IupSetAttribute(plot, "RESET", NULL);
  IupSetAttribute(plot, "CLEAR", NULL);

  // Some defaults in MathGL are different in IupMglPlot
  IupSetAttribute(plot, "AXS_X", "NO");
  IupSetAttribute(plot, "AXS_Y", "NO");
  IupSetAttribute(plot, "AXS_Z", "NO");
}

static void UpdateFlags(void)
{
  char *value;

  /* auto scaling Y axis */
  if (IupGetInt(plot, "AXS_YAUTOMIN") && IupGetInt(plot, "AXS_YAUTOMAX")) 
  {
    IupSetAttribute(autoscaleY_tgg, "VALUE", "ON");
    IupSetAttribute(minmaxY_dial, "ACTIVE", "NO");
  }
  else 
  {
    IupSetAttribute(autoscaleY_tgg, "VALUE", "OFF");
    IupSetAttribute(minmaxY_dial, "ACTIVE", "YES");
  }

  /* auto scaling X axis */
  if (IupGetInt(plot, "AXS_XAUTOMIN") && IupGetInt(plot, "AXS_XAUTOMAX")) 
  {
    IupSetAttribute(autoscaleX_tgg, "VALUE", "ON");
    IupSetAttribute(minmaxX_dial, "ACTIVE", "NO");
  }
  else 
  {
    IupSetAttribute(autoscaleX_tgg, "VALUE", "OFF");
    IupSetAttribute(minmaxX_dial, "ACTIVE", "YES");
  }

  /* grid */
  value = IupGetAttribute(plot, "GRID");
  if (value && strstr(value, "XYZ"))
    IupSetAttribute(grid_tgg, "VALUE", "ON");
  else
    IupSetAttribute(grid_tgg, "VALUE", "OFF");

  /* legend */
  if (IupGetInt(plot, "LEGEND"))
    IupSetAttribute(legend_tgg, "VALUE", "ON");
  else
    IupSetAttribute(legend_tgg, "VALUE", "OFF");

  /* box */
  if (IupGetInt(plot, "BOX"))
    IupSetAttribute(box_tgg, "VALUE", "ON");
  else
    IupSetAttribute(box_tgg, "VALUE", "OFF");

  /* antialias */
  if (IupGetInt(plot, "ANTIALIAS"))
    IupSetAttribute(aa_tgg, "VALUE", "ON");
  else
    IupSetAttribute(aa_tgg, "VALUE", "OFF");

  /* transparent */
  if (IupGetInt(plot, "TRANSPARENT"))
    IupSetAttribute(transp_tgg, "VALUE", "ON");
  else
    IupSetAttribute(transp_tgg, "VALUE", "OFF");

  /* light */
  if (IupGetInt(plot, "LIGHT"))
    IupSetAttribute(light_tgg, "VALUE", "ON");
  else
    IupSetAttribute(light_tgg, "VALUE", "OFF");

  /* opengl */
  if (IupGetInt(plot, "OPENGL"))
    IupSetAttribute(opengl_tgg, "VALUE", "ON");
   else
    IupSetAttribute(opengl_tgg, "VALUE", "OFF");
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

static void SampleDotsLinear3D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotLoadData(plot, 0, "../test/hotdogs.pts", 0, 0, 0);
  IupSetAttribute(plot, "DS_MODE", "DOTS");
  IupSetAttribute(plot, "DS_REARRANGE", NULL);

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleCrustLinear3D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotLoadData(plot, 0, "../test/hotdogs.pts", 0, 0, 0);
  IupSetAttribute(plot, "DS_MODE", "CRUST");
  IupSetAttribute(plot, "DS_REARRANGE", NULL);

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SamplePieLinear1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "rnd+0.1", 7, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "CHART");

  IupSetAttribute(plot, "COLORSCHEME", "bgr cmy");
  IupSetAttribute(plot, "PIECHART", "Yes");
  IupSetAttribute(plot, "DATAGRID", "Yes");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleChartLinear1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "rnd+0.1", 7, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "CHART");
  IupSetAttribute(plot, "DATAGRID", "Yes");

  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleStemLinear1D(void)
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

static void SampleStepLinear1D(void)
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

static void SampleBarhLinear1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.8*sin(pi*(2*x+y/2))+0.2*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "BARHORIZONTAL");

  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleBarsLinear1D(void)
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

static void SampleAreaLinear1D(void)
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

static void SampleRadarLinear1D(void)
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

static void SamplePlotLinear1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x)+0.5*cos(3*pi*x)+0.2*sin(pi*x)", 50, 1, 1);

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);

  IupSetAttribute(plot, "BOX", "YES");
}

/*
mglData y0(50); y0.Modify("sin(pi*(2*x-1))");
mglData x(50); x.Modify("cos(pi*2*x-pi)");

gr->SubPlot(2,2,2); gr->Rotate(60,40);
mglData z(50); z.Modify("2*x-1");
gr->Plot(x,y0,z); gr->Box();
mglData y2(10,3); y2.Modify("cos(pi*(2*x-1-y))");
y2.Modify("2*x-1",2);
gr->Plot(y2.SubData(-1,0),y2.SubData(-1,1),y2.SubData(-1,2),"bo ");


gr->SubPlot(2,2,3); gr->Rotate(60,40);
gr->Bars(x,y0,z,"ri"); gr->Box();
*/

static void Dummy(void)
{
}

typedef struct _TestItems{
  char* title;
  void (*func)(void);
}TestItems;

static TestItems test_list[] = {
  {"Plot (Linear 1D)", SamplePlotLinear1D},
  {"Radar (Linear 1D)", SampleRadarLinear1D},
  {"Area (Linear 1D)", SampleAreaLinear1D},
  {"Bars (Linear 1D)", SampleBarsLinear1D}, 
  {"Barh (Linear 1D)", SampleBarhLinear1D}, 
  {"Step (Linear 1D)", SampleStepLinear1D}, 
  {"Stem (Linear 1D)", SampleStemLinear1D}, 
  {"Chart (Linear 1D)", SampleChartLinear1D}, 
  {"Pie (Linear 1D)", SamplePieLinear1D}, 
  {"Dots (Linear 3D)", SampleDotsLinear3D}, 
  {"Crust (Linear 3D)", SampleCrustLinear3D},

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

static void ChangePlot(int item)
{
  ResetClear();
  test_list[item].func();
  UpdateFlags();
  sprintf(filenameSVG, "../%s.svg", test_list[item].title);
  sprintf(filenameEPS, "../%s.eps", test_list[item].title);
  IupSetAttribute(plot, "REDRAW", NULL);
}

static int k_enter_cb(Ihandle*ih)
{
  int pos = IupGetInt(ih, "VALUE");
  if (pos > 0)
    ChangePlot(pos-1);
  return IUP_DEFAULT;
}

static int action_cb(Ihandle *ih, char *text, int item, int state)
{
  (void)text;
  (void)ih;

  if (state==1)
    ChangePlot(item-1);

  return IUP_DEFAULT;
}

static int close_cb(Ihandle *ih)
{
  (void)ih;
  return IUP_CLOSE;
}

/* Y zoom */
static int minmaxY_dial_btndown_cb(Ihandle *self, double angle)
{
  (void)angle;
  (void)self;

  IupStoreAttribute(plot, "OLD_YMIN", IupGetAttribute(plot, "AXS_YMIN"));
  IupStoreAttribute(plot, "OLD_YMAX", IupGetAttribute(plot, "AXS_YMAX"));

  return IUP_DEFAULT;
}

static int minmaxY_dial_btnup_cb(Ihandle *self, double angle)
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
static int minmaxX_dial_btndown_cb(Ihandle *self, double angle)
{
  (void)angle;
  (void)self;

  IupStoreAttribute(plot, "OLD_XMIN", IupGetAttribute(plot, "AXS_XMIN"));
  IupStoreAttribute(plot, "OLD_XMAX", IupGetAttribute(plot, "AXS_XMAX"));

  return IUP_DEFAULT;
}

static int minmaxX_dial_btnup_cb(Ihandle *self, double angle)
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
static int autoscaleY_tgg_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
  {
    IupSetAttribute(minmaxY_dial, "ACTIVE", "NO");
    IupSetAttribute(plot, "AXS_YAUTOMIN", "YES");
    IupSetAttribute(plot, "AXS_YAUTOMAX", "YES");
  }
  else
  {
    IupSetAttribute(minmaxY_dial, "ACTIVE", "YES");
    IupSetAttribute(plot, "AXS_YAUTOMIN", "NO");
    IupSetAttribute(plot, "AXS_YAUTOMAX", "NO");
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* auto scale X */
static int autoscaleX_tgg_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
  {
    IupSetAttribute(minmaxX_dial, "ACTIVE", "NO");
    IupSetAttribute(plot, "AXS_XAUTOMIN", "YES");
    IupSetAttribute(plot, "AXS_XAUTOMAX", "YES");
  }
  else
  {
    IupSetAttribute(minmaxX_dial, "ACTIVE", "YES");
    IupSetAttribute(plot, "AXS_XAUTOMIN", "NO");
    IupSetAttribute(plot, "AXS_XAUTOMAX", "NO");
  }

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* show/hide grid */
static int grid_tgg_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "GRID", "YES");
  else
    IupSetAttribute(plot, "GRID", "NO");

  IupSetAttribute(plot, "REDRAW", NULL);

  return IUP_DEFAULT;
}

/* show/hide legend */
static int legend_tgg_cb(Ihandle *self, int v)
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
static int box_tgg_cb(Ihandle *self, int v)
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
static int aa_tgg_cb(Ihandle *self, int v)
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
static int transp_tgg_cb(Ihandle *self, int v)
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
static int light_tgg_cb(Ihandle *self, int v)
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
static int opengl_tgg_cb(Ihandle *self, int v)
{
  (void)self;

  if (v)
    IupSetAttribute(plot, "OPENGL", "YES");
  else
    IupSetAttribute(plot, "OPENGL", "NO");

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
    *boxinfo, *boxminmaxY_dial, *boxminmaxX_dial, *f1, *f2;

  /* left panel: plot control
     Y zooming               */
  minmaxY_dial = IupDial("VERTICAL");
  lbl1 = IupLabel("+");
  lbl2 = IupLabel("-");
  boxinfo = IupVbox(lbl1, IupFill(), lbl2, NULL);
  boxminmaxY_dial = IupHbox(boxinfo, minmaxY_dial, NULL);

  IupSetAttribute(boxminmaxY_dial, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "SIZE", "20x52");
  IupSetAttribute(boxinfo, "GAP", "2");
  IupSetAttribute(boxinfo, "MARGIN", "2");
  IupSetAttribute(boxinfo, "EXPAND", "YES");
  IupSetAttribute(lbl1, "EXPAND", "NO");
  IupSetAttribute(lbl2, "EXPAND", "NO");

  IupSetAttribute(minmaxY_dial, "ACTIVE", "NO");
  IupSetAttribute(minmaxY_dial, "SIZE", "20x52");
  IupSetCallback(minmaxY_dial, "BUTTON_PRESS_CB", (Icallback)minmaxY_dial_btndown_cb);
  IupSetCallback(minmaxY_dial, "MOUSEMOVE_CB", (Icallback)minmaxY_dial_btnup_cb);
  IupSetCallback(minmaxY_dial, "BUTTON_RELEASE_CB", (Icallback)minmaxY_dial_btnup_cb);

  autoscaleY_tgg = IupToggle("Y Autoscale", NULL);
   IupSetCallback(autoscaleY_tgg, "ACTION", (Icallback)autoscaleY_tgg_cb);
  IupSetAttribute(autoscaleY_tgg, "VALUE", "ON");

  f1 = IupFrame( IupVbox(boxminmaxY_dial, autoscaleY_tgg, NULL) );
  IupSetAttribute(f1, "TITLE", "Y Zoom");
  IupSetAttribute(f1, "GAP", "0");
  IupSetAttribute(f1, "MARGIN", "5x5");

  /* X zooming */
  minmaxX_dial = IupDial("HORIZONTAL");
  lbl1 = IupLabel("-");
  lbl2 = IupLabel("+");
  boxinfo = IupHbox(lbl1, IupFill(), lbl2, NULL);
  boxminmaxX_dial = IupVbox(minmaxX_dial, boxinfo, NULL);

  IupSetAttribute(boxminmaxX_dial, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "ALIGNMENT", "ACENTER");
  IupSetAttribute(boxinfo, "SIZE", "64x16");
  IupSetAttribute(boxinfo, "GAP", "2");
  IupSetAttribute(boxinfo, "MARGIN", "2");
  IupSetAttribute(boxinfo, "EXPAND", "HORIZONTAL");

  IupSetAttribute(lbl1, "EXPAND", "NO");
  IupSetAttribute(lbl2, "EXPAND", "NO");

  IupSetAttribute(minmaxX_dial, "ACTIVE", "NO");
  IupSetAttribute(minmaxX_dial, "SIZE", "64x16");
  IupSetCallback(minmaxX_dial, "BUTTON_PRESS_CB", (Icallback)minmaxX_dial_btndown_cb);
  IupSetCallback(minmaxX_dial, "MOUSEMOVE_CB", (Icallback)minmaxX_dial_btnup_cb);
  IupSetCallback(minmaxX_dial, "BUTTON_RELEASE_CB", (Icallback)minmaxX_dial_btnup_cb);

  autoscaleX_tgg = IupToggle("X Autoscale", NULL);
  IupSetCallback(autoscaleX_tgg, "ACTION", (Icallback)autoscaleX_tgg_cb);

  f2 = IupFrame( IupVbox(boxminmaxX_dial, autoscaleX_tgg, NULL) );
  IupSetAttribute(f2, "TITLE", "X Zoom");
  IupSetAttribute(f2, "GAP", "0");
  IupSetAttribute(f2, "MARGIN", "5x5");

  lbl1 = IupLabel("");
  IupSetAttribute(lbl1, "SEPARATOR", "HORIZONTAL");

  grid_tgg = IupToggle("Grid", NULL);
  IupSetCallback(grid_tgg, "ACTION", (Icallback)grid_tgg_cb);

  box_tgg = IupToggle("Box", NULL);
  IupSetCallback(box_tgg, "ACTION", (Icallback)box_tgg_cb);

  lbl2 = IupLabel("");
  IupSetAttribute(lbl2, "SEPARATOR", "HORIZONTAL");

  legend_tgg = IupToggle("Legend", NULL);
  IupSetCallback(legend_tgg, "ACTION", (Icallback)legend_tgg_cb);

  lbl3 = IupLabel("");
  IupSetAttribute(lbl3, "SEPARATOR", "HORIZONTAL");

  aa_tgg = IupToggle("Antialias", NULL);
  IupSetCallback(aa_tgg, "ACTION", (Icallback)aa_tgg_cb);

  transp_tgg = IupToggle("Transparent", NULL);
  IupSetCallback(transp_tgg, "ACTION", (Icallback)transp_tgg_cb);

  light_tgg = IupToggle("Light", NULL);
  IupSetCallback(light_tgg, "ACTION", (Icallback)light_tgg_cb);

  opengl_tgg = IupToggle("OpenGL", NULL);
  IupSetCallback(opengl_tgg, "ACTION", (Icallback)opengl_tgg_cb);

  lbl4 = IupLabel("");
  IupSetAttribute(lbl4, "SEPARATOR", "HORIZONTAL");

  bt1 = IupButton("Export SVG", NULL);
  IupSetCallback(bt1, "ACTION", (Icallback)bt1_cb);

  bt2 = IupButton("Export EPS", NULL);
  IupSetCallback(bt2, "ACTION", (Icallback)bt2_cb);

  vboxl = IupVbox(f1, f2, lbl1, grid_tgg, box_tgg, lbl2, legend_tgg, lbl3, aa_tgg, transp_tgg, light_tgg, opengl_tgg, lbl4, IupHbox(bt1, bt2, NULL), NULL);
  IupSetAttribute(vboxl, "GAP", "4");
  IupSetAttribute(vboxl, "MARGIN", "5x0");
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
  IupSetAttribute(dlg, "GAP", "5");
  IupSetAttribute(dlg, "TITLE", "MathGL samples w/ IupMglPlot");
  IupSetCallback(dlg, "CLOSE_CB", close_cb);

  IupSetAttribute(plot, "RASTERSIZE", "700x500");  // Minimum initial size

  IupSetAttribute(list, "EXPAND", "VERTICAL");
  IupSetAttribute(list, "VISIBLELINES", "15");  // Not all, because the dialog will be too big
  IupSetAttribute(list, "VISIBLECOLUMNS", "15");
  IupSetCallback(list, "ACTION", (Icallback)action_cb);

  for (i=0; i<count; i++)
  {
    sprintf(str, "%d", i+1);
    IupSetAttribute(list, str, test_list[i].title);
  }

  IupSetAttribute(list, "VALUE", "1");

  IupShowXY(dlg, 100, IUP_CENTER);

  IupSetAttribute(plot, "RASTERSIZE", NULL);  // Clear initial size

  ChangePlot(0);

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
