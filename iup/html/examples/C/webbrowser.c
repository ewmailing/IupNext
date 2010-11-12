/*
 * IupWebBrowser sample
 */

#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupweb.h"


#ifndef WIN32
static int history_cb(Ihandle* ih)
{
  int i;
  char str[50];
  int back = IupGetInt(ih, "BACKCOUNT");
  int fwrd = IupGetInt(ih, "FORWARDCOUNT");

  printf("HISTORY ITEMS\n");
  for(i = -(back); i < 0; i++)
  {
    sprintf(str, "ITEMHISTORY%d", i);
    printf("Backward %02d: %s\n", i, IupGetAttribute(ih, str));
  }

  sprintf(str, "ITEMHISTORY%d", 0);
  printf("Current  %02d: %s\n", 0, IupGetAttribute(ih, str));

  for(i = 1; i <= fwrd; i++)
  {
    sprintf(str, "ITEMHISTORY%d", i);
    printf("Forward  %02d: %s\n", i, IupGetAttribute(ih, str));
  }

  return IUP_DEFAULT;
}
#endif

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
  Ihandle* web  = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "LOAD", IupGetAttribute(txt, "VALUE"));
  return IUP_DEFAULT;
}

void WebBrowserTest(void)
{
  Ihandle *txt, *bt, *dlg, *history, *web;
  
  IupWebBrowserOpen();              

  // Creates an instance of the WebBrowser control
  web = IupWebBrowser();

  // Creates a dialog containing the control
  dlg = IupDialog(IupVbox(IupHbox(txt = IupText(""), 
                                  bt = IupButton("Load", NULL),
#ifndef WIN32
                                  history = IupButton("History", NULL), 
#endif
                                  NULL), 
                                  web, NULL));
  IupSetAttribute(dlg, "TITLE", "IupWebBrowser");
  IupSetAttribute(dlg, "MY_TEXT", (char*)txt);
  IupSetAttribute(dlg, "MY_WEB", (char*)web);
  IupSetAttribute(dlg, "RASTERSIZE", "800x600");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "10");

  IupSetAttribute(web, "LOAD", "http://www.tecgraf.puc-rio.br/iup");
  IupSetAttribute(txt, "VALUE", "http://www.tecgraf.puc-rio.br/iup");
  IupSetAttributeHandle(dlg, "DEFAULTENTER", bt);

  IupSetAttribute(txt, "EXPAND", "HORIZONTAL");
  IupSetCallback(bt, "ACTION", (Icallback)load_cb);
#ifndef WIN32
  IupSetCallback(history, "ACTION", (Icallback)history_cb);
#endif

  IupSetCallback(web, "NEWWINDOW_CB", (Icallback)newwindow_cb);
  IupSetCallback(web, "NAVIGATE_CB", (Icallback)navigate_cb);

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
