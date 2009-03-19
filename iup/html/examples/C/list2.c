/* IupList: Example in C 
   Creates a dialog with three frames, each one containing a list.
   The first is a simple list, the second one is a multiple list and the last one is a drop-down list.
   The second list has a callback associated.
*/

#include <stdlib.h>
#include <stdio.h>
#include <iup.h>
#include <iupkey.h>

int list_cb (Ihandle *self, char *t, int i, int v)
{
  Ihandle *lbl = (Ihandle*)IupGetAttribute(self, "_LABEL");
  IupStoreAttribute(lbl, "TITLE", t);
  return IUP_DEFAULT;
}

int edit_cb (Ihandle *self, int c, char *after)
{
  Ihandle *lbl = (Ihandle*)IupGetAttribute(self, "_LABEL");
  if (!c) return IUP_DEFAULT;
  IupStoreAttribute(lbl, "TITLE", after);
  return IUP_DEFAULT;
}

int btclose_cb(Ihandle *self)
{
  return IUP_CLOSE;
}

int bt_cb(Ihandle *self)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(self, "_LIST");
  IupMessagef ("List", "Value=%s", IupGetAttribute(list, "VALUE"));
  return IUP_DEFAULT;
}

int getfocus_cb(Ihandle *self)
{
  Ihandle *bt = (Ihandle*)IupGetAttribute(self, "_BUTTON");
  IupSetAttribute(bt, "BGCOLOR", "255 0 128");
  return IUP_DEFAULT;
}

int killfocus_cb(Ihandle *self)
{
  Ihandle *bt = (Ihandle*)IupGetAttribute(self, "_BUTTON");
  IupSetAttribute(bt, "BGCOLOR", NULL);
  return IUP_DEFAULT;
}

int main(int argc, char **argv) 
{
  Ihandle *dlg, *list1, *list2, *list3, *list4, 
                *box1, *box2, *box3, *box4, *lbl,
                *bt1, *bt2, *bt3, *bt4, *btok, *btcancel;

  IupOpen(&argc, &argv);

  bt1 = IupButton("Drop+Edit", NULL);
  IupSetCallback(bt1, "ACTION", (Icallback)bt_cb);
  bt2 = IupButton("Drop", NULL);
  IupSetCallback(bt2, "ACTION", (Icallback)bt_cb);
  bt3 = IupButton("List+Edit", NULL);
  IupSetCallback(bt3, "ACTION", (Icallback)bt_cb);
  bt4 = IupButton("List", NULL);
  IupSetCallback(bt4, "ACTION", (Icallback)bt_cb);

  list1 = IupList(NULL);
  IupSetCallback(list1, "ACTION", (Icallback)list_cb);
  list2 = IupList(NULL);
  IupSetCallback(list2, "ACTION", (Icallback)list_cb);
  list3 = IupList(NULL);
  IupSetCallback(list3, "ACTION", (Icallback)list_cb);
  list4 = IupList(NULL);
  IupSetCallback(list4, "ACTION", (Icallback)list_cb);

  IupSetAttributes(list1, "1=\"US$ 1000\", 2=\"US$ 2000\", 3=\"US$ 30000000\", 4=\"US$ 4000\", 5=\"US$ 5000\", 6=\"US$ 6000\", 7=\"US$ 7000\","
                          "EXPAND=HORIZONTAL, EDITBOX=YES, DROPDOWN=YES, VISIBLE_ITEMS=5");
  IupSetAttributes(list2, "1=\"R$ 1000\", 2=\"R$ 2000\", 3=\"R$ 3000\", 4=\"R$ 4000\", 5=\"R$ 5000\", 6=\"R$ 6000\", 7=\"R$ 7000\","
                          "EXPAND=HORIZONTAL, DROPDOWN=YES, VISIBLE_ITEMS=5");
  IupSetAttributes(list3, "1=\"Char A\", 2=\"Char B\", 3=\"Char CCCCC\", 4=\"Char D\", 5=\"Char F\", 6=\"Char G\", 7=\"Char H\","
                          "EXPAND=YES, EDITBOX=YES");
  IupSetAttributes(list4, "1=\"Number 1\", 2=\"Number 2\", 3=\"Number 3\", 4=\"Number 4\", 5=\"Number 5\", 6=\"Number 6\", 7=\"Number 7\","
                          "EXPAND=YES");

  IupSetAttribute(bt1, "_LIST", (char*)list1);
  IupSetAttribute(bt2, "_LIST", (char*)list2);
  IupSetAttribute(bt3, "_LIST", (char*)list3);
  IupSetAttribute(bt4, "_LIST", (char*)list4);

  IupSetAttribute(list1, "_BUTTON", (char*)bt1);
  IupSetAttribute(list2, "_BUTTON", (char*)bt2);
  IupSetAttribute(list3, "_BUTTON", (char*)bt3);
  IupSetAttribute(list4, "_BUTTON", (char*)bt4);

  IupSetAttribute(bt1, "BGCOLOR", "192 192 192");
  IupSetAttribute(bt2, "BGCOLOR", "192 192 192");
  IupSetAttribute(bt3, "BGCOLOR", "192 192 192");
  IupSetAttribute(bt4, "BGCOLOR", "192 192 192");

  IupSetCallback(list1, "GETFOCUS_CB",  (Icallback)getfocus_cb);
  IupSetCallback(list1, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(list2, "GETFOCUS_CB",  (Icallback)getfocus_cb);
  IupSetCallback(list2, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(list3, "GETFOCUS_CB",  (Icallback)getfocus_cb);
  IupSetCallback(list3, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(list4, "GETFOCUS_CB",  (Icallback)getfocus_cb);
  IupSetCallback(list4, "KILLFOCUS_CB", (Icallback)killfocus_cb);

  IupSetCallback(list1, "EDIT_CB", (Icallback)edit_cb);
  IupSetCallback(list3, "EDIT_CB", (Icallback)edit_cb);

/*  IupSetAttribute(list3, "READONLY", "YES"); */

  box1 = IupVbox(list1, bt1, NULL);
  box2 = IupVbox(list2, bt2, NULL);
  box3 = IupVbox(list3, bt3, NULL);
  box4 = IupVbox(list4, bt4, NULL);

  lbl = IupLabel("");
  IupSetAttribute(lbl, "EXPAND", "HORIZONTAL");

  btok = IupButton("OK", NULL);
  IupSetCallback(btok, "ACTION", (Icallback)btclose_cb);
  IupSetHandle("btok", btok);
  btcancel = IupButton("Cancel", "btclose_cb");
  IupSetCallback(btcancel, "ACTION", (Icallback)btclose_cb);
  IupSetHandle("btcancel", btcancel);

  IupSetAttribute(btok, "_LIST1", (char*)list1);
  IupSetAttribute(btok, "_LIST2", (char*)list2);
  IupSetAttribute(btok, "_LIST3", (char*)list3);
  IupSetAttribute(btok, "_LIST4", (char*)list4);
           
  dlg = IupDialog(IupSetAttributes(IupVbox(IupHbox(box1, box2, box3, box4, NULL), lbl, btok, btcancel, NULL),"MARGIN=10x10, GAP=10"));
  IupSetAttribute(dlg, "TITLE", "IupList Example");
  IupSetAttribute(dlg, "_LABEL", (char*)lbl);
  IupSetAttribute(dlg, "DEFAULTENTER", "btok");
  IupSetAttribute(dlg, "DEFAULTESC", "btcancel");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupClose();
  return EXIT_SUCCESS;

}
