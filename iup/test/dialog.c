#if 0
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdio.h>
#include "iup.h"
#include "iupkey.h"

static unsigned char pixmap_x [ ] = 
{
  1,2,3,3,3,3,3,3,3,2,1, 
  2,1,2,3,3,3,3,3,2,1,2, 
  0,2,1,2,3,3,3,2,1,2,0, 
  0,0,2,1,2,3,2,1,2,0,0, 
  0,0,0,2,1,2,1,2,0,0,0, 
  0,0,0,0,2,1,2,0,0,0,0, 
  0,0,0,2,1,2,1,2,0,0,0, 
  0,0,2,1,2,3,2,1,2,0,0, 
  0,2,1,2,3,3,3,2,1,2,0, 
  2,1,2,3,3,3,3,3,2,1,2, 
  1,2,3,3,3,3,3,3,3,2,1
};

static unsigned char pixmap_cursor [ ] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,2,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
} ;

int close_cb(Ihandle *ih)
{
  printf("CLOSE_CB(%s) with IupDestroy\n", IupGetAttribute(ih, "TESTTITLE"));
  IupDestroy(ih);
  return IUP_IGNORE;
}

int close_cb2(Ihandle *ih)
{
  printf("CLOSE_CB(%s) with IUP_IGNORE\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_IGNORE;
}

int show_cb(Ihandle *ih, int state)
{
  char* state_str[5] ={
    "SHOW",
    "RESTORE", 
    "MINIMIZE",
    "MAXIMIZE",
    "HIDE"
  };
  printf("SHOW_CB(%s, %s)\n", IupGetAttribute(ih, "TESTTITLE"), state_str[state]);
  return IUP_DEFAULT;
}

int map_cb(Ihandle *ih)
{
  printf("MAP_CB(%s)\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_DEFAULT;
}

int dropfiles_cb(Ihandle *ih, const char* filename, int num, int x, int y)
{
  printf("DROPFILES_CB(%s, %s, %d, %d, %d)\n", IupGetAttribute(ih, "TESTTITLE"), filename, num, x, y);
  return IUP_DEFAULT;
}

int resize_cb(Ihandle *ih, int w, int h)
{
  printf("RESIZE_CB(%s, %d, %d) RASTERSIZE=%s\n", IupGetAttribute(ih, "TESTTITLE"), w, h, IupGetAttribute(ih, "RASTERSIZE"));
  return IUP_DEFAULT;
}

int getfocus_cb(Ihandle *ih)
{
  printf("GETFOCUS_CB(%s)\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_DEFAULT;
}

int help_cb(Ihandle* ih)
{
  printf("HELP_CB(%s)\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_DEFAULT;
}
     
int killfocus_cb(Ihandle *ih)
{
  printf("KILLFOCUS_CB(%s)\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_DEFAULT;
}

int leavewindow_cb(Ihandle *ih)
{
  printf("LEAVEWINDOW_CB(%s)\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_DEFAULT;
}

int enterwindow_cb(Ihandle *ih)
{
  printf("ENTERWINDOW_CB(%s)\n", IupGetAttribute(ih, "TESTTITLE"));
  return IUP_DEFAULT;
}

char *iupKeyCodeToName(int code);
void new_dialog(int test, char* tip);

int k_any(Ihandle *ih, int c)
{
  if (iup_isprint(c))
    printf("K_ANY(%s, %d = %s \'%c\')", IupGetAttribute(ih, "TESTTITLE"), c, iupKeyCodeToName(c), (char)c);
  else
    printf("K_ANY(%s, %d = %s)", IupGetAttribute(ih, "TESTTITLE"), c, iupKeyCodeToName(c));
  printf("  MODKEYSTATE(%s)\n", IupGetGlobal("MODKEYSTATE"));

  switch(c)
  {
  case K_r:
    IupSetAttribute(ih, "RASTERSIZE", "300x300");
    IupRefresh(ih);
    break;
  case K_1:
    new_dialog(1, "SIZE=FULLxFULL + IupShow.\n"
                  "Check also F1 = RASTERSIZE+IupRefresh.\n"
                  "close_cb returns IGNORE.");
    break;
  case K_2:
    new_dialog(2, "Only common dlg decorations.\n"
                  "ShowXY.\n"
                  "min/max size.");
    break;
  case K_3:
    new_dialog(3, "NO decorations");
    break;
  case K_4:
    new_dialog(4, "PLACEMENT.\n"
                  "Repeat key to test variations.");
    break;
  case K_5:
    new_dialog(5, "IupPopup");
    break;
  case K_6:
    new_dialog(6, "FULLSCREEN");
    break;
  case K_p:
    {
      static int count = 0;
      if (count == 0)
        IupSetAttribute(ih, "PLACEMENT", "MAXIMIZED");
      else if (count == 1)
        IupSetAttribute(ih, "PLACEMENT", "MINIMIZED");
      else if (count == 2)
        IupSetAttribute(ih, "PLACEMENT", "NORMAL");
      else 
        IupSetAttribute(ih, "PLACEMENT", "FULL");

      count++;
      if (count == 4) // cicle from 0 to 3
        count = 0;

      IupShow(ih);
    }
    break;
  case K_s:
    IupShow( IupGetHandle("_MAIN_DIALOG_TEST_"));
    break;
  case K_f:
    if (IupGetInt(ih, "FULLSCREEN"))
      IupSetAttribute(ih, "FULLSCREEN", "NO");
    else 
      IupSetAttribute(ih, "FULLSCREEN", "YES");
    break;
  case K_ESC:
    IupDestroy(ih);
    return IUP_IGNORE;
  case K_c:
    return IUP_CLOSE;
  case K_h:
    IupHide(ih);
    break;
  }
  return IUP_DEFAULT;
}

void new_dialog(int test, char* tip)
{
  Ihandle *dlg = IupDialog(NULL);
  IupSetAttribute(dlg, "TIP", tip);
  IupSetfAttribute(dlg, "_TEST_", "%d", test);

  if (test == 0)
    IupSetHandle("_MAIN_DIALOG_TEST_", dlg);
  else
  {
    IupSetAttribute(dlg, "PARENTDIALOG", "_MAIN_DIALOG_TEST_");
    IupSetAttribute(dlg, "CURSOR", "CROSS");
  }

  if (test != 3)                                            
    IupSetfAttribute(dlg, "TITLE", "dlg%d", test);
  IupSetfAttribute(dlg, "TESTTITLE", "dlg%d", test);

  if (test == 1)
    IupSetAttribute(dlg, "SIZE", "FULLxFULL");
  else if (test != 6)
    IupSetAttribute(dlg, "RASTERSIZE", "500x500");

  if (test == 2)
  {
    IupSetAttribute(dlg, "CURSOR", "DLG_CURSOR");
    IupSetAttribute(dlg, "ICON", "DLG_ICON");
    IupSetAttribute(dlg, "MINSIZE", "300x300");
    IupSetAttribute(dlg, "MAXSIZE", "600x600");
//    IupSetAttribute(dlg, "FONTSIZE", "20");

    {
      static int count = 0;
      if (count == 1)
      {
//        IupSetAttribute(dlg, "FONT", "TIMES_ITALIC_14");
        IupSetAttribute(dlg, "FONT", "Times, Italic 14");
//        IupSetAttribute(dlg, "FONT", "Times, Underline Italic Strikeout -14");
        IupSetAttribute(dlg, "TIPBGCOLOR", "255 128 128");
        IupSetAttribute(dlg, "TIPFGCOLOR", "0 92 255");

        // Windows Only  
        IupSetAttribute(dlg, "TIPBALLON", "YES");
        IupSetAttribute(dlg, "TIPBALLONTITLE", "Tip Title Test");
        IupSetAttribute(dlg, "TIPBALLONTITLEICON", "2");
        //IupSetAttribute(dlg, "TIPDELAY", "5000");
      }
      if (count == 2)
      {
        // Windows Only  
        IupSetAttribute(dlg, "TOPMOST", "YES");
        IupSetAttribute(dlg, "LAYERED", "YES");
        IupSetAttribute(dlg, "LAYERALPHA", "128");
        IupSetAttribute(dlg, "TOOLBOX", "YES");
      }
      count++;
    }
  }

  if (test == 3)
  {
    IupSetAttribute(dlg, "BGCOLOR", "255 0 255");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "MENUBOX", "NO");
    IupSetAttribute(dlg, "MAXBOX", "NO");
    IupSetAttribute(dlg, "MINBOX", "NO");
    IupSetAttribute(dlg, "BORDER", "NO");

    IupSetAttribute(dlg, "RASTERSIZE", "500x500");
  }

  if (test == 5)
  {
    IupSetAttribute(dlg, "DIALOGFRAME", "YES");

    // Windows Only  
    IupSetAttribute(dlg, "HELPBUTTON", "YES");
  }

  if (test == 1)
    IupSetCallback(dlg, "CLOSE_CB",     (Icallback)close_cb2);
  else
    IupSetCallback(dlg, "CLOSE_CB",     (Icallback)close_cb);

  IupSetCallback(dlg, "SHOW_CB",      (Icallback)show_cb);
  IupSetCallback(dlg, "MAP_CB",       (Icallback)map_cb);
  IupSetCallback(dlg, "K_ANY",        (Icallback)k_any);
  IupSetCallback(dlg, "GETFOCUS_CB",  (Icallback)getfocus_cb); 
  IupSetCallback(dlg, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(dlg, "HELP_CB",      (Icallback)help_cb);
  IupSetCallback(dlg, "ENTERWINDOW_CB", (Icallback)enterwindow_cb);
  IupSetCallback(dlg, "LEAVEWINDOW_CB", (Icallback)leavewindow_cb);
  IupSetCallback(dlg, "RESIZE_CB", (Icallback)resize_cb);
                   
  // Windows and GTK Only  
  IupSetCallback(dlg, "DROPFILES_CB", (Icallback)dropfiles_cb);

  if (test == 0 || test == 1)
    IupShow(dlg);
  else if (test == 3)
    IupShowXY(dlg, IUP_RIGHT, IUP_CENTER);
  else if (test == 2)
    IupShowXY(dlg, 100, 100);
  else if (test == 4)
  {
    static int count = 0;
    if (count == 0)
    {
      IupSetAttribute(dlg, "PLACEMENT", "MINIMIZED");
      IupSetfAttribute(dlg, "TIP", "%s%s", IupGetAttribute(dlg, "TIP"), "\n.PLACEMENT=MINIMIZED.");
    }
    else if (count == 1)
    {
      IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");
      IupSetfAttribute(dlg, "TIP", "%s%s", IupGetAttribute(dlg, "TIP"), "\n.PLACEMENT=MAXIMIZED.");
    }
    else 
    {
      IupSetAttribute(dlg, "PLACEMENT", "FULL");
      IupSetfAttribute(dlg, "TIP", "%s%s", IupGetAttribute(dlg, "TIP"), "\n.PLACEMENT=FULL.");
    }

    count++;
    if (count == 3) // cicle from 0 to 2
      count = 0;

    IupShow(dlg);
  }
  else if (test == 5)
  {
    IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);
  }
  else if (test == 6)
  {
    IupSetAttribute(dlg, "FULLSCREEN", "YES");
    IupShow(dlg);
  }
}

void create_images(void)
{
  Ihandle *image; 

  image = IupImage(11, 11, pixmap_x);
  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "0 1 0");
  IupSetAttribute(image, "2", "255 0 0"); 
  IupSetAttribute(image, "3", "255 255 0");
  IupSetHandle("DLG_ICON", image);

  image = IupImage(32, 32, pixmap_cursor);
  IupSetAttribute(image, "0", "BGCOLOR");  /* always for cursor */
  IupSetAttribute(image, "1", "255 0 0"); 
  IupSetAttribute(image, "2", "128 0 0"); 
  IupSetAttribute(image, "HOTSPOT", "21:10");
  IupSetHandle("DLG_CURSOR", image);
}

void destroy_images(void)
{
  IupDestroy(IupGetHandle("DLG_ICON"));
  IupDestroy(IupGetHandle("DLG_CURSOR"));
}

int main(int argc, char* argv[])
{
//  IupOpen(&argc, &argv);
IupOpen(NULL, NULL);
  create_images();

  new_dialog(0, "IupDialog as a main window,\n"
                "all decorations.\n"
                "rastersize+centered.");

  IupMainLoop();

  destroy_images();

  IupClose();

  return 0;
}

/*****************************************
--------------TESTS-------------------

to check IUP sizes x Native sizes:
in UNIX use xwininfo
in Windows use Spy++
take screen shots, measure dlg sizes if necessary

0- with all decorations
   check: TITLE
          TIP
          CURSOR
          BGCOLOR
          RASTERSIZE (check window real size - 500x500)
          CLOSE_CB, SHOW_CB, MAP_CB, K_ANY, GETFOCUS_CB, KILLFOCUS_CB,
          HELP_CB, ENTERWINDOW_CB, LEAVEWINDOW_CB, RESIZE_CB
          Close_cb using IupDestroy
          closing main window will close child dialogs
          Esc - call IupDestroy
          'h' = IupHide - no destroy
          'c' = return IUP_CLOSE - no destroy

1- SIZE = FULLxFULL (check window real size)
   press 'r' to change - RASTERSIZE+IupRefresh (300x300)
   close_cb with IGNORE

2- ShowXY (check position)
   MINSIZE (300x300) and MAXSIZE (600x600)
   Custom Icon and Cursor

3- no decorations

4- 1o. PLACEMENT MINIMIZED
   2o. PLACEMENT MAXIMIZED
   3o. PLACEMENT FULL
   (press 'p' to change placement any time)

5- only "dlg" decorations
   IupPopup
   'c' = return IUP_CLOSE - no destroy
   'h' = IupHide - no destroy
   's' = IupShow for dialog0

6- FULLSCREEN
   (press 'f' to change fullscreen any time)

TODO - Build tests for:
  Append/Detach/Insert, Reparent - (all after map)
  ZORDER
  FLOATING/POSITION
*/
