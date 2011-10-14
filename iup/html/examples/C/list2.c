/* IupList: Example in C 
   Creates a dialog with three frames, each one containing a list.
   The first is a simple list, the second one is a multiple list and the last one is a drop-down list.
   The second list has a callback associated.
*/

#include <stdlib.h>
#include <stdio.h>
#include <iup.h>
#include <iupkey.h>

static Ihandle* load_image_LogoTecgraf(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 120, 143, 125, 132, 148, 178, 173, 133, 149, 178, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 110, 130, 48, 130, 147, 177, 254, 124, 139, 167, 254, 131, 147, 176, 137, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 115, 128, 153, 134, 142, 159, 191, 194, 47, 52, 61, 110, 114, 128, 154, 222, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 143, 172, 192, 140, 156, 188, 99, 65, 69, 76, 16, 97, 109, 131, 251, 129, 144, 172, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131, 147, 175, 232, 140, 157, 188, 43, 0, 0, 0, 0, 100, 112, 134, 211, 126, 141, 169, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72, 78, 88, 26, 48, 52, 57, 60, 135, 150, 178, 254, 108, 121, 145, 83, 105, 118, 142, 76, 106, 119, 143, 201, 118, 133, 159, 122, 117, 129, 152, 25, 168, 176, 190, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    118, 128, 145, 3, 104, 117, 140, 92, 114, 127, 152, 180, 131, 147, 177, 237, 133, 149, 178, 249, 38, 42, 50, 222, 137, 152, 180, 249, 126, 142, 170, 182, 114, 128, 154, 182, 104, 117, 140, 227, 95, 107, 128, 238, 83, 93, 112, 248, 84, 95, 113, 239, 104, 117, 141, 180, 115, 129, 155, 93, 127, 140, 165, 4,
    98, 109, 130, 153, 109, 123, 147, 254, 145, 163, 195, 153, 138, 154, 182, 56, 115, 123, 138, 5, 92, 99, 109, 35, 134, 149, 177, 230, 0, 0, 0, 0, 0, 0, 0, 0, 120, 133, 159, 143, 135, 151, 181, 115, 86, 89, 93, 5, 41, 45, 51, 54, 40, 45, 53, 150, 107, 120, 144, 254, 122, 137, 164, 154,
    51, 57, 66, 147, 83, 93, 112, 255, 108, 121, 145, 159, 113, 126, 151, 62, 123, 136, 159, 8, 87, 93, 103, 35, 125, 141, 169, 230, 0, 0, 0, 0, 0, 0, 0, 0, 129, 143, 169, 143, 140, 156, 184, 115, 134, 147, 172, 8, 124, 138, 165, 60, 124, 139, 167, 155, 131, 147, 177, 255, 131, 147, 176, 153,
    64, 68, 73, 2, 36, 39, 45, 86, 41, 46, 54, 173, 60, 67, 80, 232, 75, 84, 101, 251, 89, 100, 120, 228, 105, 118, 142, 250, 110, 123, 148, 187, 118, 132, 158, 187, 126, 141, 169, 229, 134, 149, 177, 239, 136, 152, 179, 250, 136, 152, 181, 234, 139, 156, 186, 175, 130, 145, 173, 90, 124, 134, 151, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 74, 79, 19, 60, 64, 73, 50, 92, 103, 124, 254, 86, 95, 111, 84, 90, 100, 117, 76, 126, 141, 168, 201, 113, 126, 150, 119, 99, 105, 117, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 93, 105, 125, 231, 135, 151, 181, 46, 0, 0, 0, 0, 137, 154, 184, 212, 123, 137, 164, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 83, 98, 191, 133, 149, 179, 102, 111, 121, 139, 17, 134, 150, 180, 252, 126, 140, 166, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 48, 57, 132, 121, 136, 164, 197, 121, 135, 161, 115, 130, 146, 175, 221, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 47, 52, 46, 87, 98, 118, 254, 126, 142, 170, 254, 124, 139, 166, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 57, 67, 118, 115, 128, 152, 170, 127, 140, 164, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    Ihandle* image = IupImageRGBA(16, 16, imgdata);
    return image;
}

static Ihandle* load_image_Test(void)
{
  unsigned char imgdata[] = 
  {
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,0,1,1,1,1,1,1,2,2,2,2,2,2,0,5, 
    5,0,1,1,1,1,1,1,2,2,2,2,2,2,0,5, 
    5,0,1,1,1,1,1,1,2,2,2,2,2,2,0,5, 
    5,0,1,1,1,1,1,1,2,2,2,2,2,2,0,5, 
    5,0,1,1,1,1,1,1,2,2,2,2,2,2,0,5, 
    5,0,1,1,1,1,1,1,2,2,2,2,2,2,0,5, 
    5,0,3,3,3,3,3,3,4,4,4,4,4,4,0,5, 
    5,0,3,3,3,3,3,3,4,4,4,4,4,4,0,5, 
    5,0,3,3,3,3,3,3,4,4,4,4,4,4,0,5, 
    5,0,3,3,3,3,3,3,4,4,4,4,4,4,0,5, 
    5,0,3,3,3,3,3,3,4,4,4,4,4,4,0,5, 
    5,0,3,3,3,3,3,3,4,4,4,4,4,4,0,5,
    5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  };

  Ihandle* image = IupImage(16, 16, imgdata);
  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "255 0 0"); 
  IupSetAttribute(image, "2", "0 255 0");
  IupSetAttribute(image, "3", "0 0 255"); 
  IupSetAttribute(image, "4", "255 255 255"); 
  IupSetAttribute(image, "5", "0 0 0"); 
  return image;
}

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
                          "EXPAND=HORIZONTAL, EDITBOX=YES, DROPDOWN=YES, VISIBLE_ITEMS=5, SHOWIMAGE=YES");
  IupSetAttributeHandle(list1, "IMAGELIST", load_image_LogoTecgraf());

  IupSetAttributes(list2, "1=\"R$ 1000\", 2=\"R$ 2000\", 3=\"R$ 3000\", 4=\"R$ 4000\", 5=\"R$ 5000\", 6=\"R$ 6000\", 7=\"R$ 7000\","
                          "EXPAND=HORIZONTAL, DROPDOWN=YES, VISIBLE_ITEMS=5, SHOWIMAGE=YES");
  IupSetAttributeHandle(list2, "IMAGELIST", load_image_Test());

  IupSetAttributes(list3, "1=\"Char A\", 2=\"Char B\", 3=\"Char CCCCC\", 4=\"Char D\", 5=\"Char F\", 6=\"Char G\", 7=\"Char H\","
                          "EXPAND=YES, EDITBOX=YES, SHOWIMAGE=YES");
  IupSetAttributeHandle(list3, "IMAGE1", load_image_Test());
  IupSetAttributeHandle(list3, "IMAGE3", load_image_Test());
  IupSetAttributeHandle(list3, "IMAGE5", load_image_Test());
  IupSetAttributeHandle(list3, "IMAGE7", load_image_Test());

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
