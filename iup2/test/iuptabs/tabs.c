#include <stdio.h>

#include "iup.h"
#include "iupcontrols.h"

int cbChildButton(Ihandle* ih)
{
  printf("button(%s)\n", IupGetAttribute(ih, "TITLE"));
  return IUP_DEFAULT;
}

int cbValuePos(Ihandle* ih)
{
  Ihandle* tabs = (Ihandle*)IupGetAttribute(ih, "APP_TABS");
  IupSetAttribute(tabs, "VALUEPOS", "0");
  return IUP_DEFAULT;
}

int cbType(Ihandle* ih, int v)
{
  if (v)
  {
    char* title = IupGetAttribute(ih, "TITLE");
    Ihandle* tabs = (Ihandle*)IupGetAttribute(ih, "APP_TABS");
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
    IupSetAttribute(tabs, "UPDATE", NULL);

    IupRefresh(tabs); /* update children layout */
  }
  return IUP_DEFAULT;
}

int cbOrientation(Ihandle* ih, int v)
{
  if (v)
  {
    char* title = IupGetAttribute(ih, "TITLE");
    Ihandle* tabs = (Ihandle*)IupGetAttribute(ih, "APP_TABS");
    switch(*title)
    {
    case 'H':
      IupSetAttribute(tabs, "TABORIENTATION", "HORIZONTAL");
      break;
    case 'V':
      IupSetAttribute(tabs, "TABORIENTATION", "VERTICAL");
      break;
    }
    IupSetAttribute(tabs, "UPDATE", NULL);

    IupRefresh(tabs); /* update children layout */
  }
  return IUP_DEFAULT;
}

int cbAddTab(Ihandle* ih)
{
  Ihandle* tabs = (Ihandle*)IupGetAttribute(ih, "APP_TABS");
  Ihandle *vbox;

  vbox = IupFrame(IupVbox(IupLabel("Label XXX"), IupButton("Button XXX", "cbChildButton"), NULL));
  IupSetAttribute(vbox, "TABTITLE", "XXX");
  IupSetAttribute(vbox, "TITLE", "TABS XXX");

  IupAppend(tabs, vbox);
  IupMap(vbox);
  IupSetAttribute(tabs, "UPDATE", NULL);

  IupRefresh(tabs); /* update children layout */

  return IUP_DEFAULT;
}

int cbTabChange(Ihandle* ih, Ihandle* new_tab, Ihandle* old_tab)
{
  printf("new Tab: %s, old Tab: %s\n", IupGetAttribute(new_tab, "TABTITLE"), IupGetAttribute(old_tab, "TABTITLE"));
  return IUP_DEFAULT;
}

int cbInactive(Ihandle *ih, int state)
{
  Ihandle* tabs = (Ihandle*)IupGetAttribute(ih, "APP_TABS");
  IupSetAttribute(tabs, "ACTIVE", state? "NO": "YES");
  return IUP_DEFAULT;
}

Ihandle* CreateTabs(int tab)
{
  Ihandle *vboxA, *vboxB, *vboxG,
          *vboxC, *vboxD,*vboxE, *vboxF, *vboxH, *vboxI,
          *tabs;

//  if (tab)  // to test Tabs inside Tabs
  //  vboxA = IupVbox(CreateTabs(0), NULL);
//  else
    vboxA = IupFrame(IupVbox(IupFill(), IupLabel("Label AAA"), IupButton("Button AAA", "cbChildButton"), NULL));
  vboxB = IupFrame(IupVbox(IupLabel("Label BBB"), IupButton("Button BBB", "cbChildButton"), NULL));
  vboxC = IupFrame(IupVbox(IupLabel("Label CCC"), IupButton("Button CCC", "cbChildButton"), NULL));
  vboxD = IupFrame(IupVbox(IupLabel("Label DDD"), IupButton("Button DDD", "cbChildButton"), NULL));
  vboxE = IupVbox(IupLabel("Label EEE"), IupButton("Button EEE", "cbChildButton"), NULL);
  vboxF = IupVbox(IupLabel("Label FFF"), IupButton("Button FFF", "cbChildButton"), NULL);
  vboxG = IupVbox(IupLabel("Label GGG"), IupButton("Button GGG", "cbChildButton"), NULL);
  vboxH = IupVbox(IupLabel("Label HHH"), IupButton("Button HHH", "cbChildButton"), NULL);
  vboxI = IupVbox(IupLabel("Label III"), IupButton("Button III", "cbChildButton"), NULL);

  IupSetAttribute(vboxA, "TABTITLE", "A");
  IupSetAttribute(vboxB, "TABTITLE", "BB");
  IupSetAttribute(vboxC, "TABTITLE", "CCC");
  IupSetAttribute(vboxD, "TABTITLE", "DDDD");
  IupSetAttribute(vboxE, "TABTITLE", "EEEEE");
  IupSetAttribute(vboxF, "TABTITLE", "FFFFFF");
  IupSetAttribute(vboxG, "TABTITLE", "GGGGGGG");
  IupSetAttribute(vboxH, "TABTITLE", "HHHHHHHH");
  IupSetAttribute(vboxI, "TABTITLE", "IIIIIIIII");
  IupSetAttribute(vboxA, "TITLE", "TABS A");
  IupSetAttribute(vboxB, "TITLE", "TABS BB");
  IupSetAttribute(vboxC, "TITLE", "TABS CCC");
  IupSetAttribute(vboxC, "ACTIVE", "NO");
  IupSetAttribute(vboxF, "TABSIZE", "100");

  tabs = IupTabs(vboxA, vboxB, vboxC, vboxD, vboxE, vboxF, vboxG, vboxH, vboxI, NULL);
//  tabs = IupFrame(IupZbox(vboxA, vboxB, vboxC, vboxD, vboxE, vboxF, vboxG, vboxH, vboxI, NULL));
  IupSetAttribute(tabs, "MARGIN", "0x0");
  IupSetCallback(tabs, "TABCHANGE_CB", (Icallback)cbTabChange);

//  IupSetAttribute(tabs, "BGCOLOR", "10 150 200");
//  IupSetAttribute(tabs, "FONT", "Helvetica, 16");

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
                IupVbox(IupButton("Add Tab", "cbAddTab"),
                        IupToggle("Inactive", "cbInactive"),
                        IupButton("VALUEPOS=0", "cbValuePos"),
                        NULL), 
                NULL);

  IupSetAttribute(tabs, "TIP", "IupTabs Tip");
//  IupSetAttribute(tabs, "EXPAND", "YES");
  IupSetAttribute(tabs, "RASTERSIZE", "300x300");
//  IupSetAttribute(tabs, "ALIGNMENT", "NW");
//  IupSetAttribute(tabs, "ALIGNMENT", "NORTH");
//  IupSetAttribute(tabs, "ALIGNMENT", "WEST");
//  IupSetAttribute(tabs, "ALIGNMENT", "EAST");
//  IupSetAttribute(tabs, "ALIGNMENT", "SOUTH");
//  IupSetAttribute(tabs, "ALIGNMENT", "SE");

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
  IupSetAttribute(dlg, "APP_TABS", (char*)tabs);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupSetFunction("cbOrientation", (Icallback)cbOrientation);
  IupSetFunction("cbType", (Icallback)cbType);
  IupSetFunction("cbAddTab", (Icallback)cbAddTab);
  IupSetFunction("cbInactive", (Icallback)cbInactive);
  IupSetFunction("cbChildButton", (Icallback)cbChildButton);
  IupSetFunction("cbValuePos", (Icallback)cbValuePos);
}

int main(void)
{
  IupOpen();
  IupControlsOpen();
  
  ShowTabs();

  IupMainLoop();
  IupClose();  

  return 0 ;
}
