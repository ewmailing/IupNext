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


#ifdef MGLPLOT_TEST

static Ihandle *plot;

void JustResetClear(void)
{
  IupSetAttribute(plot, "RESET", NULL);
  IupSetAttribute(plot, "CLEAR", NULL);
}

void SampleOne3D(void)
{
  JustResetClear();

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "-2*((2*x-1)^2 + (2*y-1)^2 + (2*z-1)^4 - (2*z-1)^2 - 0.1)", 60, 50, 40);

  IupSetAttribute(plot, "ROTATE", "40:60:0");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "TRANSPARENT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
  IupSetAttribute(plot, "DS_MODE", "VOLUME_ISOSURFACE");

  IupSetAttribute(plot, "REDRAW", NULL);
}

void SampleOne2D(void)
{
  JustResetClear();

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.6*sin(2*pi*x)*sin(3*pi*y) + 0.4*cos(3*pi*(x*y))", 50, 40, 1);

  IupSetAttribute(plot, "ROTATE", "40:60:0");
  IupSetAttribute(plot, "LIGHT", "YES");
  IupSetAttribute(plot, "BOX", "YES");
  IupSetAttribute(plot, "DS_MODE", "PLANAR_SURFACE");

  IupSetAttribute(plot, "REDRAW", NULL);
}

void SampleOne1D(void)
{
  JustResetClear();

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 0, "0.7*sin(2*pi*x)+0.5*cos(3*pi*x)+0.2*sin(pi*x)", 50, 1, 1);

  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 1, "sin(2*pi*x)", 50, 1, 1);
  
  IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetFromFormula(plot, 2, "cos(2*pi*x)", 50, 1, 1);

  IupSetAttribute(plot, "BOX", "YES");
  IupSetAttribute(plot, "REDRAW", NULL);
}

typedef struct _TestItems{
  char* title;
  void (*func)(void);
}TestItems;

static TestItems test_list[] = {
  {"SampleOne1D", SampleOne1D},
  {"SampleOne2D", SampleOne2D},
  {"SampleOne3D", SampleOne3D},
};

static int k_enter_cb(Ihandle*ih)
{
  int pos = IupGetInt(ih, "VALUE");
  if (pos > 0)
    test_list[pos-1].func();
  return IUP_DEFAULT;
}

static int dblclick_cb(Ihandle *ih, int item, char *text)
{
  (void)text;
  (void)ih;
  test_list[item-1].func();
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
  IupSetAttribute(dlg, "TITLE", "MathGL samples w/ IupMglPlot");
  IupSetCallback(dlg, "CLOSE_CB", close_cb);

  IupSetAttribute(plot, "RASTERSIZE", "400x400");

  IupSetAttribute(list, "RASTERSIZE", "100x");
  IupSetAttribute(list, "EXPAND", "VERTICAL");
  IupSetAttribute(list, "VISIBLELINES", "15");
  IupSetCallback(list, "DBLCLICK_CB", (Icallback)dblclick_cb);
  IupSetCallback(list, "K_CR", k_enter_cb);

  for (i=0; i<count; i++)
  {
    sprintf(str, "%d", i+1);
    IupSetAttribute(list, str, test_list[i].title);
  }

  IupShowXY(dlg, 100, IUP_CENTER);

  SampleOne1D();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
