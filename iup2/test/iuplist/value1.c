
#include <stdio.h>

#if 1
#include "lua.h"
#include "iglobal.h"
#include "itable.h"
#include "idrv.h"
#endif

#include <iup.h>
//printf("gggg\n"); PrintListItems(IupGetHandle("list_analyze"));

static void PrintListItems(Ihandle *e)
{
  char *index = NULL;
  Itable *it = env(e);
  while ((index=iupTableNext(it, index)) != NULL)
    printf("index: %s value: %s\n", index, (char*) iupTableGet(it, index));
}

Ihandle *dlg, *bt, *bt2;
Ihandle *list1, *list2;

int list()
{
  PrintListItems(list1);
  return IUP_DEFAULT;
}

int list2_cb()
{
  PrintListItems(list2);
  return IUP_DEFAULT;
}

int main (void) 
{
  Ihandle *frame1, *frame2;
  IupOpen ();

  list1 = IupList ("list_act");
  IupSetHandle("list1", list1);
  IupSetAttribute(list1, "1", " Análise Geométrica ");
  IupSetAttribute(list1, "2", " Análise Numérica   ");
  //IupSetAttribute(list1, "SIZE", "80");
  IupSetAttribute(list1, "BGCOLOR", "255 255 255");
  //IupSetAttribute(list1, "DROPDOWN", "YES");
  IupSetAttribute(list1, "FGCOLOR", "0 0 0");
  IupSetAttribute(list1, "VALUE", "1");
  IupSetAttribute(list1, "VISIBLE", "YES");
  IupSetAttribute(list1, "VISIBLE_ITEMS", "2");
  IupSetAttribute(list1, "TIP", "Modo de Análise");
  IupSetAttribute(list1, "ACTIVE", "YES");
  IupSetAttribute(list1, "EXPAND", "YES");
                                     
  list2 = IupList ("list_act");
  IupSetHandle("list2", list2);
  IupSetAttributes(list2, "1=1, 2=2, 3=C, 4=D, SIZE=EIGHTHxEIGHTH, DROPDOWN=YES"); 

  bt = IupButton("List", "list");
  IupSetFunction("list", (Icallback) list);

  bt2 = IupButton("List2", "list2_cb");
  IupSetFunction("list2_cb", (Icallback) list2_cb);

  frame2 = IupFrame(list2);
  IupSetAttribute(frame2, IUP_TITLE, "funciona?");

  frame1 = IupFrame(IupVbox(list1, frame2, bt, bt2, NULL));
  dlg = IupDialog(frame1);
  IupSetAttribute (dlg, IUP_TITLE, "IupList Example");
  IupShowXY (dlg, IUP_CENTER, IUP_CENTER);
  IupSetAttribute(list2, "VALUE", "2");
  
  IupMainLoop ();
  IupClose ();
  return 0;
}

