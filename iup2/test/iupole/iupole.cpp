// C Binding for using iupole 

#ifdef _WIN32
#include <windows.h>
#include <exdisp.h>

#include <iup.h>
#include <iupole.h>

//#include "siupole.h"

Ihandle *IupOleCreateBrowser(void)
{
 Ihandle *browser;
 static int firsttime=1;
 if (firsttime)
 {
  IupOleControlOpen();
  firsttime=0;
 }
 browser=IupOleControl("Shell.Explorer.2");
 IupSetAttribute(browser,"DESIGNMODE",IUP_NO);
 return browser;
}

static WCHAR* Char2Wide(char* str)
{
  if (str)
  {
    int n = strlen(str)+1;
    WCHAR* wstr = (WCHAR*)malloc(n * sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, n);
    return wstr;
  }

  return NULL;
}

static IWebBrowser2 *_IupOleGetPunkPweb(Ihandle *browser)
{
 IWebBrowser2 *pweb = (IWebBrowser2*)IupGetAttribute(browser, "IUP_IWebBrowser2");
 if (!pweb)
 {
  IUnknown *punk=(IUnknown*) IupGetAttribute(browser,"IUNKNOWN");
  punk->QueryInterface(IID_IWebBrowser2, (void **) &pweb);
  punk->Release();
  IupSetAttribute(browser, "IUP_IWebBrowser2", (char*)pweb);
 }
 return pweb;
}

void IupOleNavigate(Ihandle *browser,char *fname)
{
 IWebBrowser2 *pweb=_IupOleGetPunkPweb(browser);
 WCHAR* url = Char2Wide(fname);
 pweb->Navigate(url, NULL, NULL, NULL, NULL);
 free(url);
}

void IupOleClose(Ihandle* browser)
{
 IWebBrowser2 *pweb=_IupOleGetPunkPweb(browser);
 pweb->Release();
} 
#endif

static int load_cb(Ihandle* self)
{
  Ihandle* txt = (Ihandle*)IupGetAttribute(self, "MY_TEXT");
  Ihandle* browser = (Ihandle*)IupGetAttribute(self, "MY_BROWSER");
  IupOleNavigate(browser, IupGetAttribute(txt, "VALUE"));

  return IUP_DEFAULT;
}

static int close_cb(Ihandle* self)
{
  Ihandle* browser = (Ihandle*)IupGetAttribute(self, "MY_BROWSER");
  IupOleClose(browser);
  return IUP_DEFAULT;
}

static Ihandle *  createdialog(void)
{
  Ihandle* txt, *bt;
  Ihandle * browser = IupOleCreateBrowser();

  // Creates a dialog containing the OLE control
  Ihandle* dlg = IupDialog(IupVbox(IupHbox(txt = IupText(""), bt = IupButton("Load", NULL), NULL), browser, NULL));
  IupSetAttribute(dlg, "TITLE", "IupOle");
  IupSetAttribute(dlg, "MY_TEXT", (char*)txt);
  IupSetAttribute(dlg, "MY_BROWSER", (char*)browser);
  IupSetAttribute(txt, "EXPAND", "HORIZONTAL");
  IupSetAttribute(txt, "VALUE", "d:/EditalUniversal.doc");
  IupSetCallback(bt, "ACTION", (Icallback)load_cb);

  // Shows dialog
  IupShow(dlg);

  return dlg;
}

int main()
{
  IupOpen();

  Ihandle * dlg = createdialog();
//  createdialog();
//  createdialog();

  IupMainLoop();

  IupDestroy(dlg);

  IupClose();

  return 0;
}
