/*
 * IupWebBrowser sample
 */

#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupweb.h"

static Ihandle *control = NULL;

static int history_cb(Ihandle* self)
{
  int i;
  char str[50];
  int back = IupGetInt(control, "BACKCOUNT");
  int fwrd = IupGetInt(control, "FORWARDCOUNT");

  printf("HISTORY ITEMS\n");
  for(i = -(back); i < 0; i++)
  {
    sprintf(str, "ITEMHISTORY%d", i);
    printf("Backward %02d: %s\n", i, IupGetAttribute(control, str));
  }

  sprintf(str, "ITEMHISTORY%d", 0);
  printf("Current  %02d: %s\n", 0, IupGetAttribute(control, str));

  for(i = 1; i <= fwrd; i++)
  {
    sprintf(str, "ITEMHISTORY%d", i);
    printf("Forward  %02d: %s\n", i, IupGetAttribute(control, str));
  }

  return IUP_DEFAULT;
}

static int navigate_cb(Ihandle* self, char* reason, char* url)
{
  printf("NAVIGATE_CB: %s, %s\n", reason, url);
  return IUP_DEFAULT;
}

static int newwindow_cb(Ihandle* self, char* url)
{
  printf("NEWWINDOW_CB: %s\n", url);
  return IUP_DEFAULT;
}

static int load_cb(Ihandle* self)
{
  Ihandle* txt  = (Ihandle*)IupGetAttribute(self, "MY_TEXT");
  IupSetAttribute(control, "LOAD", IupGetAttribute(txt, "VALUE"));
  return IUP_DEFAULT;
}

void WebBrowserTest(void)
{
  Ihandle *txt, *bt, *dlg, *history;
  
  IupWebBrowserOpen();

  // Creates an instance of the WebBrowser control
  control = IupWebBrowser();

  // Creates a dialog containing the control
  dlg = IupDialog(IupVbox(IupHbox(txt = IupText(""), bt = IupButton("Load", NULL),
                                  history = IupButton("History", NULL), NULL), control, NULL));
  IupSetAttribute(dlg, "TITLE", "IupWebBrowser");
  IupSetAttribute(dlg, "MY_TEXT", (char*)txt);
  IupSetAttribute(dlg, "RASTERSIZE", "800x600");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "10");

  IupSetAttribute(control, "LOAD", "http://www.tecgraf.puc-rio.br/iup");
  IupSetAttribute(txt, "VALUE", "http://www.tecgraf.puc-rio.br/iup");
  IupSetAttributeHandle(dlg, "DEFAULTENTER", bt);

  IupSetAttribute(txt, "EXPAND", "HORIZONTAL");
  IupSetCallback(bt, "ACTION", (Icallback)load_cb);
  IupSetCallback(history, "ACTION", (Icallback)history_cb);

  IupSetCallback(control, "NEWWINDOW_CB", (Icallback)newwindow_cb);
  IupSetCallback(control, "NAVIGATE_CB", (Icallback)navigate_cb);

  // Shows dialog
  IupShow(dlg);
}

int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  WebBrowserTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
