
#include <stdio.h>
#include <assert.h>
#include "iup.h"

char copy_bits[20*19] = 
{
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,0,3,3,3,3,0,0,1,1,1,1,1,1,1,1,1,1,1
,1,1,0,3,3,3,3,0,3,0,1,1,1,1,1,1,1,1,1,1
,1,1,0,3,0,0,3,0,2,2,2,2,2,2,1,1,1,1,1,1
,1,1,0,3,3,3,3,3,2,3,3,3,3,2,2,1,1,1,1,1
,1,1,0,3,0,0,0,0,2,3,3,3,3,2,3,2,1,1,1,1
,1,1,0,3,3,3,3,3,2,3,0,0,3,2,2,2,2,1,1,1
,1,1,0,3,0,0,0,0,2,3,3,3,3,3,3,3,2,1,1,1
,1,1,0,3,3,3,3,3,2,3,0,0,0,0,0,3,2,1,1,1
,1,1,0,0,0,0,0,0,2,3,3,3,3,3,3,3,2,1,1,1
,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1
,1,1,1,1,1,1,1,1,2,3,3,3,3,3,3,3,2,1,1,1
,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

int action(Ihandle *self)
{
  printf("ACTION(%s#%s = %s)\n", IupGetClassName(self), IupGetAttribute(self, "CINDEX"), IupGetAttribute(self, IUP_VALUE));
  return IUP_DEFAULT;
}

int getfocus_cb(Ihandle *self)
{
  printf("GETFOCUS(%s#%s)\n\n", IupGetClassName(self), IupGetAttribute(self, "CINDEX"));
  return IUP_DEFAULT;
}

int killfocus_cb(Ihandle *self)
{
  printf("KILLFOCUS(%s#%s)\n", IupGetClassName(self), IupGetAttribute(self, "CINDEX"));
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *box, *menu, *image, *bt1, *bt2, *child;

  IupOpen(&argc, &argv);
  
  image = IupImage(20, 19, copy_bits);
  IupSetAttribute(image, "0", "0 0 0");
  IupSetAttribute(image, "1", "BGCOLOR");
  IupSetAttribute(image, "2", "0 0 255");
  IupSetAttribute(image, "3", "128 128 255");
  IupSetHandle("image", image); 

  box = IupVbox(
    IupSetAttributes(IupLabel("label"), "EXPAND=HORIZONTAL, CINDEX=99"),
    IupSetAttributes(IupLabel(""), "EXPAND=HORIZONTAL, IMAGE=image, CINDEX=98, BGCOLOR=\"255 0 0\""),
    bt1 = IupSetAttributes(IupButton("button", "action"), "EXPAND=HORIZONTAL, CINDEX=1"),
    bt2 = IupSetAttributes(IupButton("button", "action"), "EXPAND=HORIZONTAL, IMAGE=image, CINDEX=2, BGCOLOR=\"0 255 0\""),
    IupSetAttributes(IupText("action"), "EXPAND=HORIZONTAL, VALUE=text, CINDEX=3"),
    IupSetAttributes(IupMultiLine("action"), "EXPAND=HORIZONTAL, SIZE=x24, VALUE=multiline, CINDEX=4"),
    IupSetAttributes(IupToggle("toggle", "action"), "EXPAND=HORIZONTAL, CINDEX=5"),
    IupSetAttributes(IupToggle("toggle", "action"), "EXPAND=HORIZONTAL, IMAGE=image, CINDEX=6, BGCOLOR=\"0 0 255\""),
    IupSetAttributes(IupList("action"), "1=AAAA, 2=BBBB, EXPAND=HORIZONTAL, CINDEX=7"),
    IupSetAttributes(IupList("action"), "1=CCCC, 2=DDDD, EXPAND=HORIZONTAL, EDITBOX=YES, CINDEX=8"),
    IupSetAttributes(IupList("action"), "1=AAAA, 2=BBBB, DROPDOWN=YES, EXPAND=HORIZONTAL, CINDEX=9"),
    IupSetAttributes(IupList("action"), "1=CCCC, 2=DDDD, DROPDOWN=YES, EXPAND=HORIZONTAL, EDITBOX=YES, CINDEX=10"),
    IupSetAttributes(IupCanvas("action2"), "RASTERSIZE=200x100, EXPAND=HORIZONTAL, BGCOLOR=\"255 0 255\", CINDEX=11"),
    NULL);
  // box = IupVbox(IupFrame(box), NULL);
  IupSetAttribute(box, IUP_MARGIN, "10x10");
  IupSetAttribute(box, IUP_GAP, "10");

  menu = IupMenu(
    IupSubmenu("submenu", IupMenu(
        IupSetAttributes(IupItem("item1","action"), "CINDEX=1"),
        IupSetAttributes(IupItem("item2","action"), "CINDEX=2"),
        NULL)),
    IupSetAttributes(IupItem("item3","action"), "CINDEX=3"), 
    IupSetAttributes(IupItem("item4","action"), "CINDEX=4"), 
    NULL);
  IupSetHandle("menu", menu);

  IupSetHandle("button1", bt1);
  IupSetHandle("button2", bt2);

  dlg = IupDialog(box);
  IupSetAttribute(dlg, IUP_TITLE, "dialog");
  IupSetAttribute(dlg, IUP_MENU, "menu");
  IupSetAttribute(dlg, IUP_BGCOLOR, "255 128 0");
  IupSetAttribute(dlg, IUP_FGCOLOR, "0 255 128");
  IupSetAttribute(dlg, IUP_DEFAULTENTER, "button1");
  IupSetAttribute(dlg, IUP_DEFAULTESC,   "button2");
  IupSetAttribute(dlg, "CINDEX", "0");

  IupSetAttribute(dlg, IUP_GETFOCUS_CB, "getfocus_cb");
  IupSetAttribute(dlg, IUP_KILLFOCUS_CB, "killfocus_cb");

  child = IupGetNextChild(box, NULL);
  while(child)
  {
    IupSetAttribute(child, IUP_GETFOCUS_CB, "getfocus_cb");
    IupSetAttribute(child, IUP_KILLFOCUS_CB, "killfocus_cb");
    child = IupGetNextChild(box, child);
  }

  IupSetFunction("getfocus_cb", (Icallback)getfocus_cb);
  IupSetFunction("killfocus_cb", (Icallback)killfocus_cb);
  IupSetFunction("action", (Icallback)action);

  IupShow(dlg);
  IupMainLoop();

  IupDestroy(image);
  IupDestroy(menu);
  IupDestroy(dlg);

  IupClose();
  return 1;
}
