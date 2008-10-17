
#include <stdio.h>

#include "iup.h"
#include "iupcontrols.h"

int cbType(Ihandle* self, int v)
{
  if (v)
  {
    char* title = IupGetAttribute(self, "TITLE");
    Ihandle* tabs = (Ihandle*)IupGetAttribute(self, "APP_TABS");
    switch(*title)
    {
    case 'L':
      IupSetAttribute(tabs, "TABTYPE", "LEFT");
      break;
    case 'R':
      IupSetAttribute(tabs, "TABTYPE", "RIGHT");
      break;
    case 'T':
      IupSetAttribute(tabs, "TABTYPE", "TOP");
      break;
    case 'B':
      IupSetAttribute(tabs, "TABTYPE", "BOTTOM");
      break;
    }
    IupSetAttribute(tabs, "REPAINT", "YES");
  }
  return IUP_DEFAULT;
}

int cbOrientation(Ihandle* self, int v)
{
  if (v)
  {
    char* title = IupGetAttribute(self, "TITLE");
    Ihandle* tabs = (Ihandle*)IupGetAttribute(self, "APP_TABS");
    switch(*title)
    {
    case 'H':
      IupSetAttribute(tabs, "TABORIENTATION", "HORIZONTAL");
      break;
    case 'V':
      IupSetAttribute(tabs, "TABORIENTATION", "VERTICAL");
      break;
    }
    IupSetAttribute(tabs, "REPAINT", "YES");
  }
  return IUP_DEFAULT;
}

int cbAddTab(Ihandle* self)
{
  Ihandle* tabs = (Ihandle*)IupGetAttribute(self, "APP_TABS");
  Ihandle *vbox, *zbox;

  vbox = IupVbox(IupLabel("TABS XXX"), IupButton("XXX", ""), NULL);
  IupSetAttribute(vbox, "TABTITLE", "XXX");
  IupSetAttribute(vbox, "VISIBLE", "NO");

//  zbox = IupGetNextChild(tabs, NULL);
//  IupAppend(zbox, vbox);
  IupAppend(tabs, vbox);
  IupMap(vbox);
//  IupSetAttribute(tabs, "UPDATE", NULL);

  return IUP_DEFAULT;
}

int cbTabChange(Ihandle* self, Ihandle* new_tab, Ihandle* old_tab)
{
  printf("new Tab: %s, old Tab: %s\n", IupGetAttribute(new_tab, "TABTITLE"), IupGetAttribute(old_tab, "TABTITLE"));
  return IUP_DEFAULT;
}

Ihandle* CreateTabs(int tab)
{
  Ihandle *vboxA, *vboxB, *vboxG,
          *vboxC, *vboxD,*vboxE, *vboxF, *vboxH, *vboxI,
          *tabs;

//  if (tab)
  //  vboxA = IupVbox(CreateTabs(0), NULL);
//  else
    vboxA = IupVbox(IupLabel("TABS AAA"), IupButton("AAA", ""), NULL);
  vboxB = IupVbox(IupLabel("TABS BBB"), IupButton("BBB", ""), NULL);
  vboxC = IupVbox(IupLabel("TABS CCC"), IupButton("CCC", ""), NULL);
  vboxD = IupVbox(IupLabel("TABS DDD"), IupButton("DDD", ""), NULL);
  vboxE = IupVbox(IupLabel("TABS EEE"), IupButton("EEE", ""), NULL);
  vboxF = IupVbox(IupLabel("TABS FFF"), IupButton("FFF", ""), NULL);
  vboxG = IupVbox(IupLabel("TABS GGG"), IupButton("GGG", ""), NULL);
  vboxH = IupVbox(IupLabel("TABS HHH"), IupButton("HHH", ""), NULL);
  vboxI = IupVbox(IupLabel("TABS III"), IupButton("III", ""), NULL);

  IupSetAttribute(vboxA, "TABTITLE", "A");
  IupSetAttribute(vboxB, "TABTITLE", "BB");
  IupSetAttribute(vboxC, "TABTITLE", "CCC");
  IupSetAttribute(vboxD, "TABTITLE", "DDDD");
  IupSetAttribute(vboxE, "TABTITLE", "EEEEE");
  IupSetAttribute(vboxF, "TABTITLE", "FFFFFF");
  IupSetAttribute(vboxG, "TABTITLE", "GGGGGGG");
  IupSetAttribute(vboxH, "TABTITLE", "HHHHHHHH");
  IupSetAttribute(vboxI, "TABTITLE", "IIIIIIIII");

  tabs = IupTabs(vboxA, vboxB, vboxC, vboxD, vboxE, vboxF, vboxG, vboxH, vboxI, NULL);
  IupSetAttribute(tabs, "MARGIN", "0x0");
  IupSetCallback(tabs, "TABCHANGE_CB", (Icallback)cbTabChange);
//  IupSetAttribute(tabs, "DOUBLEBUFFER", "NO");

  return tabs;
}

void ShowTabs(void)
{
  Ihandle *box, *frm1, *frm2, *dlg, *tabs;

  tabs = CreateTabs(1);
  
  box = IupHbox(tabs, 
                frm1 = IupFrame(IupRadio(IupVbox(IupToggle("TOP", "cbType"), 
                                                 IupToggle("LEFT", "cbType"), 
                                                 IupToggle("BOTTOM", "cbType"), 
                                                 IupToggle("RIGHT", "cbType"), 
                                                 NULL))), 
                frm2 = IupFrame(IupRadio(IupVbox(IupToggle("HORIZONTAL", "cbOrientation"), 
                                                 IupToggle("VERTICAL", "cbOrientation"), 
                                                 NULL))), 
                IupButton("Add Tab", "cbAddTab"),
                NULL);

  IupSetAttribute(frm1, "MARGIN", "5x5");
  IupSetAttribute(frm2, "MARGIN", "5x5");
  IupSetAttribute(frm1, "GAP", "0");
  IupSetAttribute(frm2, "GAP", "0");
  IupSetAttribute(frm1, "TITLE", "Type");
  IupSetAttribute(frm2, "TITLE", "Orientation");
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");
  dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupTabs Test");
  IupSetAttribute(dlg, "TIP", "IupTIP TEST");
  IupSetAttribute(dlg, "APP_TABS", (char*)tabs);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupSetFunction("cbOrientation", (Icallback)cbOrientation);
  IupSetFunction("cbType", (Icallback)cbType);
  IupSetFunction("cbAddTab", (Icallback)cbAddTab);
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();
  
  ShowTabs();

  IupMainLoop();
  IupClose();  

  return 0 ;
}
