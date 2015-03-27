#if 0
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "iupcontrols.h"
#include "iupkey.h"

/* Must define BIG_TEST on the Project and include in the build all test files */
#ifdef BIG_TEST
void ButtonTest(void);
void CanvasScrollbarTest(void);
void CanvasTest(void);
void CanvasCDDBufferTest(void);
void CanvasCDSimpleTest(void);
void DialogTest(void);
void FrameTest(void);
void TabsTest(void);
void SysInfoTest(void);
void GetColorTest(void);
#ifdef USE_OPENGL
void GLCanvasTest(void);
void GLCanvasCubeTest(void);
#endif
void HboxTest(void);
void IdleTest(void);
void LabelTest(void);
void ListTest(void);
void MatrixTest(void);
void MatrixCbModeTest(void);
void MatrixCbsTest(void);
void MdiTest(void);
void VboxTest(void);
void MenuTest(void);
void ColorBrowserTest(void);
void ColorbarTest(void);
void CellsNumberingTest(void);
void DialTest(void);
void GaugeTest(void);
void CellsDegradeTest(void);
void CellsCheckboardTest(void);
void ValTest(void);
void TreeTest(void);
void TrayTest(void);
void ToggleTest(void);
void TimerTest(void);
void TextSpinTest(void);
void TextTest(void);
void SpinTest(void);
void SampleTest(void);
void PreDialogsTest(void);
#ifdef PPLOT_TEST
void PPlotTest(void);
#endif
void GetParamTest(void);

void HelpTest(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  //IupHelp("..\\html\\index.html");
}

typedef struct _TestItems{
  char* title;
  void (*func)(void);
}TestItems;

static TestItems test_list[] = {
  {"Button", ButtonTest},
  {"Canvas", CanvasTest},
  {"CanvasCDSimpleTest", CanvasCDSimpleTest},
  {"CanvasCDDBuffer", CanvasCDDBufferTest},
  {"CanvasScrollbar", CanvasScrollbarTest},
  {"CellsCheckboard", CellsCheckboardTest},
  {"CellsDegrade", CellsDegradeTest},
  {"CellsNumbering", CellsNumberingTest},
  {"ColorBrowser", ColorBrowserTest},
  {"Colorbar", ColorbarTest},
  {"Dial", DialTest},
  {"Dialog", DialogTest},
  {"Frame", FrameTest},
#ifdef USE_OPENGL
  {"GLCanvas", GLCanvasTest},
  {"GLCanvasCube", GLCanvasCubeTest},
#endif
  {"Gauge", GaugeTest},
  {"GetColor", GetColorTest},
  {"GetParam", GetParamTest},
  {"Help", HelpTest},
  {"Hbox", HboxTest},
  {"Idle", IdleTest},
  {"Label", LabelTest},
  {"List", ListTest},
  {"Matrix", MatrixTest},
  {"MatrixCbMode", MatrixCbModeTest},
  {"MatrixCbs", MatrixCbsTest},
  {"Mdi", MdiTest},
  {"Menu", MenuTest},
#ifdef PPLOT_TEST
  {"PPlot", PPlotTest},
#endif
  {"PreDialogs", PreDialogsTest},
  {"Sample", SampleTest},
  {"Spin", SpinTest},
  {"SysInfo", SysInfoTest},
  {"Tabs", TabsTest},
  {"Text", TextTest},
  {"TextSpin", TextSpinTest},
  {"Timer", TimerTest},
  {"Toggle", ToggleTest},
  {"Tray", TrayTest},
  {"Tree", TreeTest},
  {"Val", ValTest},
  {"Vbox", VboxTest},
};

static int list_cb (Ihandle *ih, char *t, int pos, int state)
{
  static int old_pos = -1;
  if (old_pos == pos && state == 1)
  {
    test_list[pos-1].func();
    old_pos = -1;
  }
  else
    old_pos = pos;

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
//  IupOldValOpen();
//  IupOldTabsOpen();

  dlg = IupDialog(IupVbox(list = IupList(NULL), NULL));
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "TITLE", "IupTests");
  IupSetCallback(dlg, "CLOSE_CB", close_cb);

  IupSetCallback(list, "ACTION", (Icallback)list_cb);
//  IupSetAttribute(list, "VISIBLELINES", "15");
  IupSetAttribute(list, "RASTERSIZE", "100x200");

  IupSetAttribute(list, "EXPAND", "YES");

  for (i=0; i<count; i++)
  {
    sprintf(str, "%d", i+1);
    IupSetAttribute(list, str, test_list[i].title);
  }

  IupShowXY(dlg, 100, IUP_CENTER);

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
