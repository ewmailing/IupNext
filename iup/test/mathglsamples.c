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

void ResetClear(void)
{
  IupSetAttribute(plot, "RESET", NULL);
  IupSetAttribute(plot, "CLEAR", NULL);

  // Some defaults in MathGL are different in IupMglPlot
  IupSetAttribute(plot, "AXS_X", "NO");
  IupSetAttribute(plot, "AXS_Y", "NO");
  IupSetAttribute(plot, "AXS_Z", "NO");
}

void SampleOne3D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "-2*((2*x-1)^2 + (2*y-1)^2 + (2*z-1)^4 - (2*z-1)^2 - 0.1)", 60, 50, 40);
  IupSetAttribute(plot, "DS_MODE", "VOLUME_ISOSURFACE");

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "TRANSPARENT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

void SampleOne2D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.6*sin(2*pi*x)*sin(3*pi*y) + 0.4*cos(3*pi*(x*y))", 50, 40, 1);
  IupSetAttribute(plot, "DS_MODE", "PLANAR_SURFACE");

  IupSetAttribute(plot, "ROTATE", "40:0:60");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
}

void SamplePie1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "rnd+0.1", 7, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "CHART");

  IupSetAttribute(plot, "COLORSCHEME", "bgr cmy");
  IupSetAttribute(plot, "PIECHART", "Yes");
  IupSetAttribute(plot, "DATAGRID", "Yes");
  IupSetAttribute(plot, "BOX", "YES");
}

void SampleChart1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "rnd+0.1", 7, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "CHART");
  IupSetAttribute(plot, "DATAGRID", "Yes");

  IupSetAttribute(plot, "BOX", "YES");
}

void SampleStem1D(void)
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

  IupSetAttribute(plot, "AXS_XORIGIN", "0.0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0.0");
  IupSetAttribute(plot, "BOX", "YES");
}

void SampleStep1D(void)
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

void SampleBarh1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.8*sin(pi*(2*x+y/2))+0.2*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "BARHORIZONTAL");

  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

void SampleBars1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.8*sin(pi*(2*x+y/2))+0.2*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "BAR");

  IupSetAttribute(plot, "AXS_XORIGIN", "0");
  IupSetAttribute(plot, "AXS_YORIGIN", "0");
  IupSetAttribute(plot, "BOX", "YES");
}

void SampleArea1D(void)
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

void SampleRadar1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.4*sin(pi*(2*x+y/2))+0.1*rnd", 10, 1, 1);
  IupSetAttribute(plot, "DS_MODE", "RADAR");

  IupSetAttribute(plot, "DATAGRID", "Yes");
  IupSetAttribute(plot, "BOX", "YES");
}

void SamplePlot1D(void)
{
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x)+0.5*cos(3*pi*x)+0.2*sin(pi*x)", 50, 1, 1);

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);

  IupSetAttribute(plot, "BOX", "YES");
}

typedef struct _TestItems{
  char* title;
  void (*func)(void);
}TestItems;

static TestItems test_list[] = {
  {"1D Plot", SamplePlot1D},
  {"1D Radar", SampleRadar1D},
  {"1D Area", SampleArea1D},
  {"1D Bars", SampleBars1D}, 
  {"1D Barh", SampleBarh1D}, 
  {"1D Step", SampleStep1D}, 
  {"1D Stem", SampleStem1D}, 
  {"1D Chart", SampleChart1D}, 
  {"1D Pie", SamplePie1D}, 
  {"One 2D", SampleOne2D},
  {"One 3D", SampleOne3D},
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

  IupSetAttribute(list, "RASTERSIZE", "100x");
  IupSetAttribute(list, "EXPAND", "VERTICAL");
  IupSetAttribute(list, "VISIBLELINES", "15");
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
  SamplePlot1D();
  IupSetAttribute(plot, "REDRAW", NULL);

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
