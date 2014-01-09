#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "iupcontrols.h"

int notify(Ihandle *ih)
{
  Ihandle *newdlg = IupGetParent(ih);

  IupSetAttribute(newdlg, "TITLE", "New Dialog");
  IupSetAttribute(newdlg, "MARGIN", "10x10");
  IupSetAttribute(newdlg, "RASTERSIZE", NULL);
  IupRefresh(newdlg);

  IupSetHandle("dbox", ih);
  IupSetAttribute(IupGetHandle("restore"), "ACTIVE", "YES");
  printf("Detached!\n");

  return IUP_DEFAULT;
}

int btn_restore_cb(Ihandle *bt)
{
  Ihandle *child = IupGetHandle("dbox");
  Ihandle *parent  = (Ihandle*)IupGetAttribute(child, "OLDPARENT_HANDLE");
  Ihandle *brother = (Ihandle*)IupGetAttribute(child, "OLDBROTHER_HANDLE");
  Ihandle *newdlg = IupGetParent(child);

  IupDetach(child);

  IupInsert(parent, brother, child);
  IupMap(child);
  IupRefresh(parent);

  IupDestroy(newdlg);
  
  IupSetAttribute(bt, "ACTIVE", "NO");

  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *bt, *box, *lbl, *ml, *hbox, *bt2;
  IupOpen(&argc, &argv);

  bt = IupButton("Detach me!", NULL);
  IupSetAttribute(bt, "EXPAND", "YES");

  ml = IupMultiLine(NULL);
  IupSetAttribute(ml, "EXPAND", "YES");
  IupSetAttribute(ml, "VISIBLELINES", "5");
  hbox = IupHbox(bt, ml, NULL);

  box = IupDetachBox(hbox);
  IupSetAttribute(box, "ORIENTATION", "VERTICAL");
  //IupSetAttribute(box, "SHOWGRIP", "NO");
  //IupSetAttribute(box, "COLOR", "255 0 0");
  IupSetCallback(box, "DETACHED_CB", (Icallback)notify);

  lbl = IupLabel("Label");
  IupSetAttribute(lbl, "EXPAND", "VERTICAL");

  bt2 = IupButton("Restore me!", NULL);
  IupSetAttribute(bt2, "EXPAND", "YES");
  IupSetAttribute(bt2, "ACTIVE", "NO");
  IupSetCallback(bt2, "ACTION", (Icallback)btn_restore_cb);
  IupSetHandle("restore", bt2);

  dlg = IupDialog(IupVbox(box, lbl, bt2, NULL));
  IupSetAttribute(dlg, "TITLE", "IupDetachBox Example");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "10");
  IupSetAttribute(dlg, "RASTERSIZE", "300x300");

  IupShow(dlg);

  IupMainLoop();
  IupClose();
  return EXIT_SUCCESS;
}
