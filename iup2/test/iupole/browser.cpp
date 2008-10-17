// browser.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "browser.h"

static int _BrowserCloseDialog(Ihandle *self)
{
 Browser *wcto=(Browser*)IupGetAttribute(self,"browser");
 wcto->Hide();
 return IUP_DEFAULT;
}

void Browser::Show()
{
 if (firstshow) {Navigate();firstshow=0;}
 IupShowXY(dialog,IUP_CENTER,IUP_CENTER);
 shown=1;
}

void Browser::Hide()
{
 if (shown)
 {
  IupHide(dialog);
  shown=0;
 }
}

int Browser::Navigate()
{
#ifdef _WIN32
 IupOleNavigate(browser,fname);
 return 1;
#endif
 return 0;
}

void Browser::CreateDialog100()
{
 close = IupButton("Close","close");
 IupSetFunction("close",(Icallback)_BrowserCloseDialog);
 IupSetAttribute(close,"browser",(char *)this);
#ifdef _WIN32
 IupSetAttribute(close,IUP_SIZE,"90");
#else
 IupSetAttribute(close,IUP_SIZE,"60");
 browser = IupCanvas(NULL);
#endif
 dialog = IupDialog(IupVbox(browser,
                            IupHbox(IupFill(),close,IupFill(),NULL),
                            NULL));
 IupSetAttribute(dialog,IUP_SIZE,"500x400");
 IupSetAttribute(dialog,IUP_TITLE,title);
 IupSetAttribute(dialog,"ICON","bitcenpes");
 IupSetHandle("dialog",dialog);
 IupMap(dialog);
}

Browser::Browser(char *brtitle,char *brfname)
{
 firstshow=1;
 strcpy(fname,brfname);
 strcpy(title,brtitle);
 browser=IupOleCreateBrowser();
 CreateDialog100();
}

Browser::~Browser()
{
 IupDestroy(dialog);
}
