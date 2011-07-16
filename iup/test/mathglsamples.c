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

static Ihandle *plot;

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

static void SampleGradientLinesVolume(void)
{
  SampleVolume("VOLUME_GRADIENTLINES");
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

static void SampleGridPlanar(void)
{
  SamplePlanar("PLANAR_GRID");
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

  //TODO: rearrange data in linear form

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

static void SampleCrustLinear(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotLoadData(plot, 0, "../test/hotdogs.pts", 0, 0, 0);
  IupSetAttribute(plot, "DS_MODE", "CRUST");

  //TODO: rearrange data in linear form

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
  //TODO: use original dimentions and rearrange data in linear form

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.4*sin(pi*(2*x+y/2))+0.1*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "RADAR");

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
  {"Grid (Planar)", SampleGridPlanar},

  {"----------", Dummy},

  {"Iso Surface (Volume)", SampleIsoSurfaceVolume},
  {"CloudCubes (Volume)", SampleCloudCubesVolume},
  {"Cloud (Volume)", SampleCloudVolume},
  {"Density (Volume)", SampleDensityVolume},
  {"Contour (Volume)", SampleContourVolume},
  {"ContourFilled (Volume)", SampleContourFilledVolume},
  {"ContourProject (Volume)", SampleContourProjectVolume},
  {"DensityProject (Volume)", SampleDensityProjectVolume},
  {"GradientLines (Volume)", SampleGradientLinesVolume},
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

static int action_cb(Ihandle *ih, char *text, int item, int state)
{
  (void)text;
  (void)ih;

  if (state==1)
  {
    ResetClear();
    test_list[item-1].func();
    IupSetAttribute(plot, "REDRAW", NULL);
  }

  return IUP_DEFAULT;
}

static int close_cb(Ihandle *ih)
{
  (void)ih;
  return IUP_CLOSE;
}

int main(int argc, char* argv[])
{
  int i, count = sizeof(test_list)/sizeof(TestItems);
  char str[50];
  Ihandle *dlg, *list;

  IupOpen(&argc, &argv);
  IupControlsOpen();
  IupMglPlotOpen();     /* init IupMGLPlot library */

  IupSetGlobal("MGLFONTS", "../etc/mglfonts");

  list = IupList(NULL);
  plot = IupMglPlot();

  dlg = IupDialog(IupHbox(list, plot, NULL));
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "10");
  IupSetAttribute(dlg, "TITLE", "MathGL samples w/ IupMglPlot");
  IupSetCallback(dlg, "CLOSE_CB", close_cb);

  IupSetAttribute(plot, "RASTERSIZE", "400x400");
//  IupSetAttribute(plot, "OPENGL", "NO");

  IupSetAttribute(list, "EXPAND", "VERTICAL");
  IupSetAttribute(list, "VISIBLELINES", "15");
  IupSetAttribute(list, "VISIBLECOLUMNS", "12");
  IupSetCallback(list, "ACTION", (Icallback)action_cb);

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

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}

/* TODO:
  Add toggles and dial, just like the other example

  Profundidade do Box em OpenGL

  PieChart
  Crust
  Dots

  ->Axial está setando algo que afeta a cor em OpenGL
  ColorBar
  Grid
  Density não aparece quando não é OpenGL

  ContourFilled
  ContourProject
  GradientLines
  Grid

  Plots do "Basic Features"
*/
