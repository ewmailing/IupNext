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
void DialogTest(void);
void FrameTest(void);
void TabsTest(void);
void SysInfoTest(void);
void GetColorTest(void);
void GLCanvasTest(void);
void GLCanvasCubeTest(void);
void HboxTest(void);
void IdleTest(void);
void LabelTest(void);
void ListTest(void);
void MatrixTest(void);
void MdiTest(void);
void VboxTest(void);
void MenuTest(void);
void ColorBrowserTest(void);
void ColorbarTest(void);
void CellsNumberingTest(void);
void DialTest(void);
void GaugeTest(void);
void CellsDegradeTest(void);
void CellsCheckboarTest(void);
void ValTest(void);
void TreeTest(void);
void TrayTest(void);
void ToggleTest(void);
void TimerTest(void);
void TextSpinTest(void);
void TextTest(void);
void SpinTest(void);
void SampleTest(void);
void ProgressbarTest(void);
void PreDialogsTest(void);
void PPlotTest(void);
void GetParamTest(void);

typedef struct _TestItems{
  char* title;
  void (*func)(void);
}TestItems;

TestItems test_list[] = {
  {"Button", ButtonTest},
  {"Canvas", CanvasTest},
  {"CanvasCDDBuffer", CanvasCDDBufferTest},
  {"CanvasScrollbar", CanvasScrollbarTest},
  {"CellsCheckboar", CellsCheckboarTest},
  {"CellsDegrade", CellsDegradeTest},
  {"CellsNumbering", CellsNumberingTest},
  {"ColorBrowser", ColorBrowserTest},
  {"Colorbar", ColorbarTest},
  {"Dial", DialTest},
  {"Dialog", DialogTest},
  {"Frame", FrameTest},
  {"GLCanvas", GLCanvasTest},
  {"GLCanvasCube", GLCanvasCubeTest},
  {"Gauge", GaugeTest},
  {"GetColor", GetColorTest},
  {"GetParam", GetParamTest},
  {"Hbox", HboxTest},
  {"Idle", IdleTest},
  {"Label", LabelTest},
  {"List", ListTest},
  {"Matrix", MatrixTest},
  {"Mdi", MdiTest},
  {"Menu", MenuTest},
  {"PPlot", PPlotTest},
  {"PreDialogs", PreDialogsTest},
  {"Progressbar", ProgressbarTest},
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

int button_cb(Ihandle *ih,int but,int pressed,int x,int y,char* status)
{
  (void)pressed;
  if (but==IUP_BUTTON1 && iup_isdouble(status))
  {
    int pos;
    IupListConvertXYToItem(ih, x, y, &pos);
    test_list[pos-1].func();
  }
  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  int i, count = sizeof(test_list)/sizeof(TestItems);
  char str[50];
  Ihandle *dlg, *list;

  IupOpen(&argc, &argv);
  IupControlsOpen();

  dlg = IupDialog(IupVbox(list = IupList(NULL), NULL));
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "TITLE", "IupTests");

  IupSetCallback(list, "BUTTON_CB", (Icallback)button_cb);
  IupSetAttribute(list, "VISIBLELINES", "15");
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
