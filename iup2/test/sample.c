#if 0
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iup.h>
#include <iupcontrols.h>

static unsigned char img_bits1[] = 
{
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,0,2,0,2,0,2,2,0,2,2,2,0,0,0,2,2,2,0,0,2,0,2,2,0,0,0,2,2,2
,2,2,2,0,2,0,0,2,0,0,2,0,2,0,2,2,2,0,2,0,2,2,0,0,2,0,2,2,2,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,2,2,2,2,0,2,0,2,2,2,0,2,0,2,2,2,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,2,0,0,0,0,2,0,2,2,2,0,2,0,0,0,0,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,0,2,2,2,0,2,0,2,2,2,0,2,0,2,2,2,2,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,0,2,2,2,0,2,0,2,2,0,0,2,0,2,2,2,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,2,0,0,0,0,2,2,0,0,2,0,2,2,0,0,0,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,0,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static unsigned char img_bits2[] = 
{
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,0,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,0,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,0,3,0,3,0,3,3,0,3,3,3,1,1,0,3,3,3,0,0,3,0,3,3,0,0,0,3,3,3
,3,3,3,0,3,0,0,3,0,0,3,0,3,0,1,1,3,0,3,0,3,3,0,0,3,0,3,3,3,0,3,3
,3,3,3,0,3,0,3,3,0,3,3,0,3,3,1,1,3,0,3,0,3,3,3,0,3,0,3,3,3,0,3,3
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,3,3,3,0,3,0,3,3,0,3,3,0,3,0,1,1,3,0,3,0,3,3,0,0,3,0,3,3,3,0,3,3
,3,3,3,0,3,0,3,3,0,3,3,0,3,3,1,1,0,0,3,3,0,0,3,0,3,3,0,0,0,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,0,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,0,3,3,3,0,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,0,0,0,3,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,2,2,2,2,2,2,2,3,3,3,3,3,3,3,1,1,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,3,3,3,3,3,3,3,3,1,1,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

static int line = 0;
int getfocus_cb(Ihandle *self)
{
  printf("%2d-getfocus(%s#%s)\n", line, IupGetClassName(self), 
                                        IupGetAttribute(self, "CINDEX"));
  line++;
  return IUP_DEFAULT;
}

int killfocus_cb(Ihandle *self)
{
  printf("%2d-killfocus(%s#%s)\n", line, IupGetClassName(self), 
                                         IupGetAttribute(self, "CINDEX"));
  line++;
  return IUP_DEFAULT;
}

int action(Ihandle* self)
{
  printf("%2d-action(%s#%s) Value=%s\n", line, IupGetClassName(self), 
                                               IupGetAttribute(self, "CINDEX"), 
                                               IupGetAttribute(self, "VALUE"));
  line++;
  return IUP_DEFAULT;
}

int list_cb(Ihandle* self, char *t, int i, int v)
{
  printf("%2d-action(%s#%s) Value=%s (%s, %d, %d)\n", line, IupGetClassName(self), 
                                               IupGetAttribute(self, "CINDEX"), 
                                               IupGetAttribute(self, "VALUE"), t, i, v);
  line++;
  return IUP_DEFAULT;
}

int multiselect_cb(Ihandle* self, char* value)
{
  printf("%2d-multiselect_cb(%s#%s) Value=%s (%s)\n", line, IupGetClassName(self), 
                                               IupGetAttribute(self, "CINDEX"), 
                                               IupGetAttribute(self, "VALUE"), value);
  line++;
  return IUP_DEFAULT;
}

void createImages (void)
{
  Ihandle *img;

  img = IupImage(32,32, img_bits1);
  IupSetHandle ("img1", img); 
  IupSetAttribute (img, "0", "0 0 0"); 
  IupSetAttribute (img, "1", "BGCOLOR");
  IupSetAttribute (img, "2", "255 0 0");

  img = IupImage(32,32, img_bits2);
  IupSetHandle ("img2", img); 
  IupSetAttribute (img, "0", "0 0 0"); 
  IupSetAttribute (img, "1", "0 255 0");
  IupSetAttribute (img, "2", "BGCOLOR");
  IupSetAttribute (img, "3", "255 0 0");
}

Ihandle* createDialog (void)
{
  Ihandle *mnu, *_hbox_1, *_cnv_1, *_vbox_1, *dlg, 
    *_frm_1, *_frm_2, *_frm_3, *_frm_4, *_frm_5,
    *_list_1, *_list_2, *_list_3, *_list_4, *_text_1, *_ml_1;

  mnu = IupMenu(
  IupSubmenu("IupSubmenu 1",IupMenu(
      IupSetAttributes(IupItem("IupItem 1 Checked", NULL), "VALUE=ON,CINDEX=1"),
      IupSeparator(),
      IupSetAttributes(IupItem("IupItem 2 Disabled", NULL), "ACTIVE=NO,CINDEX=2"),
      NULL)),
    IupSetAttributes(IupItem("IupItem 3", NULL), "CINDEX=3"),
    IupSetAttributes(IupItem("IupItem 4", NULL), "CINDEX=4"),
    NULL);
  IupSetHandle("mnu",mnu);

  _frm_1 = IupFrame(
    IupVbox(
      IupSetAttributes(IupButton("Button Text", NULL), "CINDEX=1"),
      IupSetAttributes(IupButton("", NULL), "CINDEX=2, BGCOLOR=\"255 128 0\", RASTERSIZE=30x30"),
      IupSetAttributes(IupButton("", NULL), "IMAGE=img1,CINDEX=3"),
      IupSetAttributes(IupButton("", NULL), "IMAGE=img1,CINDEX=4, FLAT=YES"),
      IupSetAttributes(IupButton("", NULL), "IMAGE=img1,IMPRESS=img2,CINDEX=5"),
      NULL));
  IupSetAttribute(_frm_1,"TITLE","IupButton");

  _frm_2 = IupFrame(
    IupVbox(
      IupSetAttributes(IupLabel("Label Text\nLine 2\nLine 3"), "CINDEX=1"),
      IupSetAttributes(IupLabel(""), "SEPARATOR=HORIZONTAL,CINDEX=2"),
      IupSetAttributes(IupLabel(""), "IMAGE=img1,CINDEX=3"),
      NULL));
  IupSetAttribute(_frm_2,"TITLE","IupLabel");

  _frm_3 = IupFrame(
    IupVbox(
      IupSetAttributes(IupToggle("Toggle Text", NULL), "VALUE=ON,CINDEX=1"),
      IupSetAttributes(IupToggle("3State Text", NULL), "VALUE=NOTDEF,CINDEX=2,3STATE=YES"),
      IupSetAttributes(IupToggle("", NULL), "IMAGE=img1,IMPRESS=img2,CINDEX=3"),
      IupSetAttributes(IupFrame(IupRadio(IupVbox(
        IupSetAttributes(IupToggle("Toggle Text", NULL), "CINDEX=4"),
        IupSetAttributes(IupToggle("Toggle Text", NULL), "CINDEX=5"),
        NULL))), "TITLE=IupRadio"),
      NULL));
  IupSetAttribute(_frm_3,"TITLE","IupToggle");

  _text_1 = IupText( NULL);
  IupSetAttribute(_text_1,"VALUE","IupText Text");
//  IupSetAttribute(_text_1,"SIZE","80x");
  IupSetAttribute(_text_1,"CINDEX","1");

  _ml_1 = IupMultiLine( NULL);
  IupSetAttribute(_ml_1,"VALUE","IupMultiline Text\nSecond Line\nThird Line");
//  IupSetAttribute(_ml_1,"SIZE","50x30");
//  IupSetAttribute(_ml_1,"EXPAND","YES");
//  IupSetAttribute(_ml_1,"SIZE","80x60");
  IupSetAttribute(_ml_1,"CINDEX","1");

  _frm_4 = IupFrame(IupVbox(
    _text_1,
    _ml_1,
    NULL));
  IupSetAttribute(_frm_4,"TITLE","IupText/IupMultiline");

  _list_1 = IupList( NULL);
  IupSetAttribute(_list_1,"EXPAND","YES");
//  IupSetAttribute(_list_1, "SIZE", "50x40");
  IupSetAttribute(_list_1,"VALUE","1");
  IupSetAttribute(_list_1,"MULTIPLE","YES");  
  IupSetAttribute(_list_1,"1","Item 1 Text");
  IupSetAttribute(_list_1,"2","Item 2 Text");
  IupSetAttribute(_list_1,"3","Item 3 Text Big Item");
  IupSetAttribute(_list_1,"4","Item 4 Text");
  IupSetAttribute(_list_1,"5","Item 5 Text");
  IupSetAttribute(_list_1,"6","Item 6 Text");
  IupSetAttribute(_list_1,"CINDEX","1");        

//  IupSetCallback(_list_1,"MULTISELECT_CB",(Icallback)multiselect_cb);
//  IupSetCallback(_list_1,"ACTION",(Icallback)list_cb);

  _list_2 = IupList( NULL);
  IupSetAttribute(_list_2,"DROPDOWN","YES");
  IupSetAttribute(_list_2,"EXPAND","YES");
  IupSetAttribute(_list_2, "VISIBLE_ITEMS", "3");
//  IupSetAttribute(_list_2, "SIZE", "50x");
  IupSetAttribute(_list_2,"VALUE","2");
  IupSetAttribute(_list_2,"1","Item 1 Text");
  IupSetAttribute(_list_2,"2","Item 2 Text Big Item");
  IupSetAttribute(_list_2,"3","Item 3 Text");
  IupSetAttribute(_list_2,"4","Item 4 Text");
  IupSetAttribute(_list_2,"5","Item 5 Text");
  IupSetAttribute(_list_2,"6","Item 6 Text");
  IupSetAttribute(_list_2,"CINDEX","2");

  _list_3 = IupList( NULL);
  IupSetAttribute(_list_3,"EDITBOX","YES");
  IupSetAttribute(_list_3,"EXPAND","YES");
//  IupSetAttribute(_list_3, "SIZE", "50x40");
  IupSetAttribute(_list_3,"VALUE","Test Value");
  IupSetAttribute(_list_3,"1","Item 1 Text");
  IupSetAttribute(_list_3,"2","Item 2 Text Big Item");
  IupSetAttribute(_list_3,"3","Item 3 Text");
  IupSetAttribute(_list_3,"4","Item 4 Text");
  IupSetAttribute(_list_3,"5","Item 5 Text");
  IupSetAttribute(_list_3,"6","Item 6 Text");
  IupSetAttribute(_list_3,"CINDEX","3");

  _list_4 = IupList( NULL);
  IupSetAttribute(_list_4,"EDITBOX","YES");
  IupSetAttribute(_list_4,"DROPDOWN","YES");
  IupSetAttribute(_list_4,"EXPAND","YES");
  IupSetAttribute(_list_4,"VISIBLE_ITEMS", "3");
//  IupSetAttribute(_list_4, "SIZE", "50x10");
  IupSetAttribute(_list_4,"VALUE","Test Value");
  IupSetAttribute(_list_4,"1","Item 1 Text");
  IupSetAttribute(_list_4,"2","Item 2 Text Big Item");
  IupSetAttribute(_list_4,"3","Item 3 Text");
  IupSetAttribute(_list_4,"4","Item 4 Text");
  IupSetAttribute(_list_4,"5","Item 5 Text");
  IupSetAttribute(_list_4,"6","Item 6 Text");
  IupSetAttribute(_list_4,"CINDEX","4");

  _frm_5 =  IupFrame(IupVbox(
      _list_1,
      _list_2,
      _list_3,
      _list_4,
      NULL));
  IupSetAttribute(_frm_5,"TITLE","IupList");

  _hbox_1 = IupHbox(
    _frm_1,
    _frm_2,
    _frm_3,
    _frm_4,
    _frm_5,
    NULL);

  _cnv_1 = IupCanvas( "do_nothing");
  IupSetAttribute(_cnv_1,"BGCOLOR","128 255 0");

  _vbox_1 = IupVbox(
//    IupTabs(_hbox_1, NULL),
    _hbox_1,
    _cnv_1,
    NULL);
  IupSetAttribute(_vbox_1,"MARGIN","5x5");
  IupSetAttribute(_vbox_1,"ALIGNMENT","ARIGHT");
  IupSetAttribute(_vbox_1,"GAP","5");
  IupSetCallback(_vbox_1, "GETFOCUS_CB", (Icallback)getfocus_cb);
  IupSetCallback(_vbox_1, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(_vbox_1, "ACTION", (Icallback)action);

  dlg = IupDialog(_vbox_1);
  IupSetAttribute(dlg,"MENU","mnu");
  IupSetAttribute(dlg,"TITLE","IupDialog Title");
//  IupSetAttribute(dlg,"SHRINK","YES");
//  IupSetAttribute(dlg, "SIZE", "500x200");

//  IupSetAttribute(dlg,"BGCOLOR","255 0 255");
//  IupSetAttribute(dlg,"FONT","Times New Roman:ITALIC:10");
//  IupSetAttribute(dlg, "FONT", "TIMES_BOLD_14");
//  IupSetAttribute(_vbox_1,"ACTIVE","NO");

  //IupSetAttribute(dlg,"COMPOSITED", "YES");
  //IupSetAttribute(dlg,"LAYERED", "YES");
  //IupSetAttribute(dlg,"LAYERALPHA", "192");

//  IupMap(dlg);
//  IupSetAttribute(dlg, "SIZE", NULL);

  return dlg;
}

int main(int argc, char **argv)
{
  Ihandle* dlg;
  IupOpen(&argc, &argv);      
//  IupControlsOpen();      
  createImages();
  dlg = createDialog();
//  IupSetAttribute(dlg, "PLACEMENT", "FULL");
//  IupSetAttribute(dlg, "FULLSCREEN", "YES");
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
  IupMainLoop();
  IupDestroy(IupGetHandle("img1"));
  IupDestroy(IupGetHandle("img2"));
  IupDestroy(dlg);
//  IupControlsClose();      
  IupClose();  
  return 0;
}
