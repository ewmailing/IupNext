#include <stdio.h>
#include <iup.h>
#include <iupcontrols.h>
#include <cd.h>
#include <cdiup.h>

Ihandle *dlg     = NULL;
Ihandle *bt      = NULL;
Ihandle *gauge   = NULL;
Ihandle *tabs    = NULL;
Ihandle *cv      = NULL;
cdCanvas*cdcanvas= NULL;

int need_redraw;

int toggle_redraw(void)
{
  need_redraw = !need_redraw;
  return IUP_DEFAULT;
}

int fake_redraw(void)
{
  need_redraw = 1;
  return IUP_DEFAULT;
}

int redraw(void)
{
  if(need_redraw == 1)
  {
    int i;

    need_redraw = 0;
    IupSetAttribute(bt, IUP_FGCOLOR, "255 0 0");
    IupFlush();

    cdCanvasActivate(cdcanvas);
    cdCanvasForeground(cdcanvas, CD_BLUE);
    cdCanvasClear(cdcanvas);

    for(i = 0; i < 300000; i++)
    {
      cdCanvasBox(cdcanvas, 0, 300, 0, i/1000);
      IupSetfAttribute(gauge, IUP_VALUE, "%f", (float)i/300000);

      IupLoopStep();
    }

    IupSetAttribute(bt, IUP_FGCOLOR, "255 255 255");
    IupFlush();
  }
  return IUP_DEFAULT;
}

void init(void) 
{
  gauge = IupGauge();
  cv    = IupCanvas(NULL);
  bt    = IupButton("Restart", NULL);
  IupSetAttribute(bt,    IUP_SIZE, "50x50");
  IupSetAttribute(bt,    IUP_FGCOLOR, "255 255 255");
  IupSetAttribute(gauge, IUP_SIZE, "200x15");
  IupSetAttribute(cv,    IUP_SIZE, "200x200");
  dlg   = IupDialog(IupVbox(cv, IupHbox(gauge, bt, NULL), NULL));
  IupSetAttribute(dlg, IUP_TITLE, "Redraw test");
}

int main(int argc, char **argv) 
{
  IupOpen(&argc, &argv);
  IupControlsOpen();
  
  init();

  IupSetFunction(IUP_IDLE_ACTION, (Icallback)redraw);

  IupMap(dlg);
  
  cdcanvas = cdCreateCanvas(CD_IUP, cv) ;
  
  IupSetCallback(cv, "ACTION", (Icallback)fake_redraw);
  IupSetCallback(bt, "ACTION", (Icallback)toggle_redraw);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupMainLoop();
  IupDestroy(dlg);
  IupControlsClose();
  IupClose();

  return 0;
}
