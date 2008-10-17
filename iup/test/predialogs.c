#if 0
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdio.h>
#include <string.h>

#include "iup.h"
#include "iupkey.h"


#ifdef WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <windows.h>
void drawTest(Ihandle* ih)
{
  RECT rect;
  HDC hDC = (HDC)IupGetAttribute(ih, "PREVIEWDC");
  int w = IupGetInt(ih, "PREVIEWWIDTH");
  int h = IupGetInt(ih, "PREVIEWHEIGHT");

  SetRect(&rect, 0, 0, w, h);
  FillRect(hDC, &rect, GetStockObject(WHITE_BRUSH));

  SelectObject(hDC, GetStockObject(DC_PEN));
  SetDCPenColor(hDC, RGB(255, 0, 0));

  MoveToEx(hDC, 0, 0, NULL);
  LineTo(hDC, w-1, h-1);
  MoveToEx(hDC, 0, h-1, NULL);
  LineTo(hDC, w-1, 0);
}
#else
#include <X11/Xlib.h>

#define xCOLOR8TO16(_x) (_x*257)  /* 65535/255 = 257 */
static unsigned long xGetPixel(Display* dpy, unsigned char cr, unsigned char cg, unsigned char cb)
{
  XColor xc;
  xc.red = xCOLOR8TO16(cr);
  xc.green = xCOLOR8TO16(cg);
  xc.blue = xCOLOR8TO16(cb);
  xc.flags = DoRed | DoGreen | DoBlue;

  XAllocColor(dpy, DefaultColormap(dpy, XDefaultScreen(dpy)), &xc);
  return xc.pixel;
}

void drawTest(Ihandle* ih)
{
  GC gc = (GC)IupGetAttribute(ih, "PREVIEWDC");
  Display* dpy = (Display*)IupGetAttribute(ih, "XDISPLAY");
  Drawable wnd = (Drawable)IupGetAttribute(ih, "XWINDOW");
  int w = IupGetInt(ih, "PREVIEWWIDTH");
  int h = IupGetInt(ih, "PREVIEWHEIGHT");

  XSetForeground(dpy, gc, xGetPixel(dpy, 255, 255, 255));
  XFillRectangle(dpy, wnd, gc, 0, 0, w, h);

  XSetForeground(dpy, gc, xGetPixel(dpy, 255, 0, 0));

  XDrawLine(dpy, wnd, gc, 0, 0, w-1, h-1);
  XDrawLine(dpy, wnd, gc, 0, h-1, w-1, 0);
}
#endif

int close_cb(Ihandle *ih)
{
  printf("CLOSE_CB\n");
  IupDestroy(ih);
  return IUP_IGNORE;
}

int help_cb(Ihandle* ih)
{
  (void)ih;
  printf("HELP_CB\n");
  return IUP_DEFAULT; 
}

int file_cb(Ihandle* ih, const char* filename, const char* status)
{
  (void)ih;
  printf("FILE_CB(%s - %s)\n", status, filename);

  if (strcmp(status, "PAINT")==0)
  {
    printf("  SIZE(%s x %s)\n", IupGetAttribute(ih, "PREVIEWWIDTH"), IupGetAttribute(ih, "PREVIEWHEIGHT"));
    drawTest(ih);
  }
  return IUP_DEFAULT; 
}

void new_message(char* type, char* buttons)
{
  Ihandle* dlg = IupMessageDlg();

  if (strcmp(type, "ERROR")!=0)
    IupSetAttribute(dlg, "PARENTDIALOG", "_MAIN_DIALOG_TEST_");
  IupSetAttribute(dlg, "DIALOGTYPE", type);
  IupSetAttribute(dlg, "TITLE", "IupMessageDlg Test");
  IupSetAttribute(dlg, "BUTTONS", buttons);
  IupSetAttribute(dlg, "VALUE", "Message Text\nSecond Line");
  if (strcmp(type, "WARNING")==0)
    IupSetAttribute(dlg, "BUTTONDEFAULT", "2");
  if (strcmp(type, "INFORMATION")!=0)
    IupSetCallback(dlg, "HELP_CB", (Icallback)help_cb);

  IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);

  printf("BUTTONRESPONSE(%s)\n", IupGetAttribute(dlg, "BUTTONRESPONSE"));

  IupDestroy(dlg);
}

void new_color(void)
{
  Ihandle* dlg = IupColorDlg();

  IupSetAttribute(dlg, "PARENTDIALOG", "_MAIN_DIALOG_TEST_");
  IupSetAttribute(dlg, "VALUE", "128 0 255");
  IupSetAttribute(dlg, "ALPHA", "142");
  IupSetAttribute(dlg, "COLORTABLE", "128 0 255;0 0 128;253 20 119");
  IupSetAttribute(dlg, "TITLE", "IupColorDlg Test");
  IupSetCallback(dlg, "HELP_CB", (Icallback)help_cb);

  IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);

  if (IupGetInt(dlg, "STATUS"))
  {
    printf("OK\n");
    printf("  VALUE(%s)\n", IupGetAttribute(dlg, "VALUE"));
    printf("  COLORTABLE(%s)\n", IupGetAttribute(dlg, "COLORTABLE"));
  }
  else
    printf("CANCEL\n");

  IupDestroy(dlg);
}

void new_font(void)
{
  Ihandle* dlg = IupFontDlg();

  IupSetAttribute(dlg, "PARENTDIALOG", "_MAIN_DIALOG_TEST_");
  IupSetAttribute(dlg, "COLOR", "128 0 255");
  IupSetAttribute(dlg, "VALUE", "Times New Roman, Bold 20");
  IupSetAttribute(dlg, "TITLE", "IupFontDlg Test");
  IupSetCallback(dlg, "HELP_CB", (Icallback)help_cb);

  IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);

  if (IupGetInt(dlg, "STATUS"))
  {
    printf("OK\n");
    printf("  VALUE(%s)\n", IupGetAttribute(dlg, "VALUE"));
    printf("  COLOR(%s)\n", IupGetAttribute(dlg, "COLOR"));
  }
  else
    printf("CANCEL\n");

  IupDestroy(dlg);
}

void new_file(char* dialogtype, int preview)
{
  Ihandle *dlg = IupFileDlg(); 

  IupSetAttribute(dlg, "PARENTDIALOG", "_MAIN_DIALOG_TEST_");
  IupSetAttribute(dlg, "DIALOGTYPE", dialogtype);
  IupSetAttribute(dlg, "TITLE", "IupFileDlg Test");
  IupSetAttribute(dlg, "DIRECTORY", "/tecgraf/iup");
  if (strcmp(dialogtype, "DIR")!=0)
  {
    IupSetAttributes(dlg, "FILTER = \"*.bmp\", FILTERINFO = \"Bitmap Files\"");
    IupSetAttribute(dlg, "EXTFILTER", "Text files|*.txt;*.doc|Image files|*.gif;*.jpg;*.bmp|");
//    IupSetAttribute(dlg, "FILE", "/tecgraf/im/test.bmp");
    IupSetAttribute(dlg, "FILE", "test.bmp");
  }
  IupSetCallback(dlg, "HELP_CB", (Icallback)help_cb);
//  IupSetAttributes(dlg, "FILE = \"\\tecgraf\\iup\\test.bmp\""); // OK
//  IupSetAttributes(dlg, "FILE = \"/tecgraf/iup/test.bmp\""); // OK
//  IupSetAttributes(dlg, "FILE = \"test.bmp\", DIRECTORY = \"/tecgraf/iup\"");   // OK
//  IupSetAttributes(dlg, "FILE = \"test.bmp\", DIRECTORY = \"\\tecgraf\\iup\"");  // OK
//  IupSetAttribute(dlg, "NOCHANGEDIR", "NO");
//  IupSetAttribute(dlg, "MULTIPLEFILES", "YES");

  if (preview)
  {
    IupSetAttribute(dlg, "SHOWPREVIEW", "YES");
    IupSetCallback(dlg, "FILE_CB", (Icallback)file_cb);
  }
  
  IupPopup(dlg, IUP_CURRENT, IUP_CURRENT); 

  switch(IupGetInt(dlg, "STATUS"))
  {
    case 1: 
      printf("OK\n");
      printf("  New file - VALUE(%s)\n", IupGetAttribute(dlg, "VALUE")); 
      break; 
    case 0 : 
      printf("OK\n");
      printf("  File exists - VALUE(%s)\n", IupGetAttribute(dlg, "VALUE"));
      break; 
    case -1 : 
      printf("CANCEL\n");
      break; 
  }

  IupDestroy(dlg);
}

void new_alarm(void)
{
  int ret = IupAlarm ("IupAlarm Test", "Message Text", "Button 1", "Button 2", "Button 3");
  printf("Button(%d)\n", ret);
}

void new_gettext(void)
{
  char text[1024] = "text first line\nsecond line";
  int ret = IupGetText("IupGetText Text", text);  // TODO add max_size
  if (ret)
  {
    printf("OK\n");
    printf("Text(%s)\n", text);
  }
  else
    printf("CANCEL\n");
}

void new_list(void)
{
  int ret;   
  int size = 8 ;
  int marks[8] = { 0,0,0,0,1,1,0,0 };
  const char *options[] = {
    "Blue"   ,
    "Red"    ,
    "Green"  ,
    "Yellow" ,
    "Black"  ,
    "White"  ,
    "Gray"   ,
    "Brown"  } ;
    
  ret = IupListDialog(2,"IupListDialog Test",size,options,0,16,5,marks);

  if (ret == -1)
  {
    printf("CANCEL\n");
  }
  else
  {
    int i;
    char selection[80] = "";
    printf("OK\n");
    
    for(i = 0 ; i < size ; i++)
    {
      if(marks[i])
      {
        char temp[10];        
        sprintf(temp,"%s\n",options[i]);      
        strcat(selection,temp);
      }
    }
    
    printf("  Options (%s)\n", selection);    
  }
}

int k_any(Ihandle *ih, int c)
{
  switch(c)
  {
  case K_m:
    IupMessage("IupMessage Test", "Message Text\nSecond Line.");
    break;
  case K_e:
    new_message("ERROR", NULL);
    break;
  case K_i:
    new_message("INFORMATION", NULL);
    break;
  case K_w:
    new_message("WARNING", "OKCANCEL");
    break;
  case K_q:
    new_message("QUESTION", "YESNO");
    break;
  case K_c:
    new_color();
    break;
  case K_f:
    new_font();
    break;
  case K_o:
    new_file("OPEN", 0);
    break;
  case K_O:
    new_file("OPEN", 1);
    break;
  case K_s:
    new_file("SAVE", 0);
    break;
  case K_d:
    new_file("DIR", 0);
    break;
  case K_a:
    new_alarm();
    break;
  case K_t:
    new_gettext();
    break;
  case K_l:
    new_list();
    break;
  case K_ESC:
    IupDestroy(ih);
    return IUP_IGNORE;
  }
  return IUP_DEFAULT;
}

void main_dialog(void)
{
  Ihandle *dlg = IupDialog(NULL);

  IupSetHandle("_MAIN_DIALOG_TEST_", dlg);

  IupSetAttribute(dlg, "TITLE", "Pre-defined Dialogs Test");
  IupSetAttribute(dlg, "RASTERSIZE", "500x500");

  IupSetCallback(dlg, "K_ANY",    (Icallback)k_any);
  IupSetCallback(dlg, "CLOSE_CB", (Icallback)close_cb);

  IupShow(dlg);
}

int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  main_dialog();

  IupMainLoop();

  IupClose();

  return 0;
}

// TODO: test IupGetText, IupListDialog
