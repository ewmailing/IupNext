/*IupList Example in C 
Creates a dialog with four frames, each one containing a list. 
*/


#include <stdio.h>
#include <iup.h>

int multilist_cb (Ihandle *self, char *s)
{
  Ihandle *lbl = (Ihandle*)IupGetAttribute(self, "_LABEL");
  IupStoreAttribute(lbl, IUP_TITLE, s);
  printf("multilist_cb(%s)\n", s);
  return IUP_DEFAULT;
}

int list_cb (Ihandle *self, char *t, int i, int v)
{
  Ihandle *lbl = (Ihandle*)IupGetAttribute(self, "_LABEL");
  IupStoreAttribute(lbl, IUP_TITLE, t);
  printf("list_cb(%s, %i, %i)\n", t, i, v);
  return IUP_DEFAULT;
}

int edit_cb (Ihandle *self, int c, char *after)
{
  Ihandle *lbl = (Ihandle*)IupGetAttribute(self, "_LABEL");
  if (isxkey(c) && c != K_DEL) return IUP_DEFAULT;
  IupStoreAttribute(lbl, IUP_TITLE, after);
  return IUP_DEFAULT;
}

int btclose_cb(Ihandle *self)
{
  return IUP_CLOSE;
}

int bttest_cb(Ihandle *self)
{
  return IUP_DEFAULT;
}

int bt_cb(Ihandle *self)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(self, "_LIST");
  IupMessagef ("List", "Value=%s", IupGetAttribute(list, IUP_VALUE));
IupSetAttribute(list, "8", "Teste Bem Grande para Nao Ficar Faltando Nada");
  return IUP_DEFAULT;
}

int getfocus_cb(Ihandle *self)
{
  Ihandle *bt = (Ihandle*)IupGetAttribute(self, "_BUTTON");
  IupSetAttribute(bt, IUP_BGCOLOR, "255 0 128");
  return IUP_DEFAULT;
}

int killfocus_cb(Ihandle *self)
{
  Ihandle *bt = (Ihandle*)IupGetAttribute(self, "_BUTTON");
  IupSetAttribute(bt, IUP_BGCOLOR, "0 128 255");
  return IUP_DEFAULT;
}

int main(int argc, char **argv) 
{
  Ihandle *dlg, *list1, *list2, *list3, *list4, 
                *box1, *box2, *box3, *box4, *lbl,
                *bt1, *bt2, *bt3, *bt4, *btok, *btcancel;

  IupOpen(&argc, &argv);

  bt1 = IupButton("Drop+Edit", "bt_cb");
  bt2 = IupButton("Drop", "bt_cb");
  bt3 = IupButton("List+Edit", "bt_cb");
  bt4 = IupButton("List", "bt_cb");

  list1 = IupList("list_cb");
  list2 = IupList("list_cb");
  list3 = IupList("list_cb");
  list4 = IupList(NULL);

  IupSetAttributes(list1, "1=\"US$ 1000\", 2=\"US$ 2000\", 3=\"US$ 30000000\", 4=\"US$ 4000\", 5=\"US$ 5000\", 6=\"US$ 6000\", 7=\"US$ 7000\","
                          "EXPAND=HORIZONTAL, EDITBOX=YES, DROPDOWN=YES, VISIBLE_ITEMS=5");
  IupSetAttributes(list2, "1=\"R$ 1000\", 2=\"R$ 2000\", 3=\"R$ 123456789\", 4=\"R$ 4000\", 5=\"R$ 5000\", 6=\"R$ 6000\", 7=\"R$ 7000\","
//                          "EXPAND=HORIZONTAL, DROPDOWN=YES, VISIBLE_ITEMS=5");
                          "SIZE=50x, DROPDOWN=YES, VISIBLE_ITEMS=5");
  IupSetAttributes(list3, "1=\"Char A\", 2=\"Char B\", 3=\"Char CCCCC\", 4=\"Char D\", 5=\"Char F\", 6=\"Char G\", 7=\"Char H\","
                          "SIZE=50x100, EXPAND=YES, EDITBOX=YES");
//                          "EXPAND=YES, EDITBOX=YES");
  IupSetAttributes(list4, "1=\"Number 1\", 2=\"Number 2\", 3=\"Number 3\", 4=\"Number 4\", 5=\"Number 5\", 6=\"Number 6\", 7=\"Number 7\","
                          "MULTIPLE=YES, VALUE=\"+--+---\", EXPAND=YES");

  IupSetAttribute(bt1, "_LIST", (char*)list1);
  IupSetAttribute(bt2, "_LIST", (char*)list2);
  IupSetAttribute(bt3, "_LIST", (char*)list3);
  IupSetAttribute(bt4, "_LIST", (char*)list4);

  IupSetAttribute(list1, "_BUTTON", (char*)bt1);
  IupSetAttribute(list2, "_BUTTON", (char*)bt2);
  IupSetAttribute(list3, "_BUTTON", (char*)bt3);
  IupSetAttribute(list4, "_BUTTON", (char*)bt4);

  IupSetAttribute(bt1, IUP_BGCOLOR, "192 192 192");
  IupSetAttribute(bt2, IUP_BGCOLOR, "192 192 192");
  IupSetAttribute(bt3, IUP_BGCOLOR, "192 192 192");
  IupSetAttribute(bt4, IUP_BGCOLOR, "192 192 192");

  IupSetAttribute(list1, IUP_GETFOCUS_CB, "getfocus_cb");
  IupSetAttribute(list1, IUP_KILLFOCUS_CB, "killfocus_cb");
  IupSetAttribute(list2, IUP_GETFOCUS_CB, "getfocus_cb");
  IupSetAttribute(list2, IUP_KILLFOCUS_CB, "killfocus_cb");
  IupSetAttribute(list3, IUP_GETFOCUS_CB, "getfocus_cb");
  IupSetAttribute(list3, IUP_KILLFOCUS_CB, "killfocus_cb");
  IupSetAttribute(list4, IUP_GETFOCUS_CB, "getfocus_cb");
  IupSetAttribute(list4, IUP_KILLFOCUS_CB, "killfocus_cb");

//  IupSetAttribute(list4, "MULTISELECT_CB", "multilist_cb");
  IupSetAttribute(list4, "ACTION", "list_cb");

  IupSetAttribute(list1, "EDIT_CB", "edit_cb");
  IupSetAttribute(list3, "EDIT_CB", "edit_cb");

/*  IupSetAttribute(list3, IUP_READONLY, IUP_YES); */

  box1 = IupFrame(IupVbox(list1, bt1, NULL));
  box2 = IupFrame(IupVbox(list2, bt2, NULL));
  box3 = IupFrame(IupVbox(list3, bt3, NULL));
  box4 = IupFrame(IupVbox(list4, bt4, NULL));

  IupSetAttribute(box1, "TITLE", "Drop+Edit");
  IupSetAttribute(box2, "TITLE", "Drop");
  IupSetAttribute(box3, "TITLE", "List+Edit");
  IupSetAttribute(box4, "TITLE", "List");

  IupSetFunction("bt_cb", bt_cb);
  IupSetFunction("list_cb", (Icallback)list_cb);
  IupSetFunction("multilist_cb", (Icallback)multilist_cb);
  IupSetFunction("getfocus_cb", getfocus_cb);
  IupSetFunction("killfocus_cb", killfocus_cb);
  IupSetFunction("edit_cb", (Icallback)edit_cb);
  IupSetFunction("btclose_cb", (Icallback)btclose_cb);
  IupSetFunction("bttest_cb", (Icallback)bttest_cb);

  lbl = IupLabel("");
  IupSetAttribute(lbl, "EXPAND", "HORIZONTAL");

  btok = IupButton("Test", "bttest_cb");
  IupSetHandle("btok", btok);
  btcancel = IupButton("Close", "btclose_cb");
  IupSetHandle("btcancel", btcancel);

  IupSetAttribute(btok, "_LIST1", (char*)list1);
  IupSetAttribute(btok, "_LIST2", (char*)list2);
  IupSetAttribute(btok, "_LIST3", (char*)list3);
  IupSetAttribute(btok, "_LIST4", (char*)list4);
           
  dlg = IupDialog(IupSetAttributes(IupVbox(IupHbox(box1, box2, box3, box4, NULL), lbl, btok, btcancel, NULL),"MARGIN=10x10, GAP=10"));
  IupSetAttribute(dlg, IUP_TITLE, "IupList Example");
  IupSetAttribute(dlg, "_LABEL", (char*)lbl);
  IupSetAttribute(dlg, IUP_DEFAULTENTER, "btok");
  IupSetAttribute(dlg, IUP_DEFAULTESC, "btcancel");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupDestroy(dlg);
  IupClose();
  return 0;
}
