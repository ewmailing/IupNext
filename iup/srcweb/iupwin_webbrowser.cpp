/** \file
* \brief Web Browser Control
*
* See Copyright Notice in "iup.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <exdisp.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupole.h"

#include "iup_object.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_stdcontrols.h"
#include "iup_str.h"
#include "iup_layout.h"
#include "iup_webbrowser.h"
#include "iup_drv.h"
#include "iup_drvfont.h"


extern "C" WCHAR* iupwinStrChar2Wide(const char* str);
extern "C" char* iupwinStrWide2Char(const WCHAR* wstr);


///////////////////////////////////////////////////////////////////////////////
// Interface to get WebBrowserEvents
///////////////////////////////////////////////////////////////////////////////

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#include <shlguid.h>   // IID_IWebBrowser2, DIID_DWebBrowserEvents2
#include <exdispid.h>  // DISPID_*

using namespace ATL;

interface CSink:public IDispEventImpl<0, CSink, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 0>
{
public:
  Ihandle* ih;

  BEGIN_SINK_MAP(CSink)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2)
#ifdef DISPID_NEWWINDOW3
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW3, NewWindow3)
#endif
  END_SINK_MAP()

  void STDMETHODCALLTYPE BeforeNavigate2(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName,
                                         VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
  {
    IFnss cbNavigate = (IFnss)IupGetCallback(ih, "NAVIGATE_CB");
    if (cbNavigate)
    {
      char* resultString = iupwinStrWide2Char(url->bstrVal);
      cbNavigate(ih, "NOREASON", resultString);
      free(resultString);
      // TO DO: detect types of action, like "link_clicked", "refresh", "form submitted", "backward/forward"
    }

    (void)Cancel;
    (void)Headers;
    (void)PostData;
    (void)TargetFrameName;
    (void)Flags;
    (void)pDisp;
  }

#ifdef DISPID_NEWWINDOW3
  void STDMETHODCALLTYPE NewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags,
                                    BSTR bstrUrlContext, BSTR bstrUrl)
  {
    IFns cbNewWindow = (IFns)IupGetCallback(ih, "NEWWINDOW_CB");
    if (cbNewWindow)
    {
      char* urlString = iupwinStrWide2Char(bstrUrl);
      cbNewWindow(ih, urlString);
      free(urlString);
    }

    (void)bstrUrlContext;
    (void)dwFlags;
    (void)Cancel;
    (void)ppDisp;
  }
#endif
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int winWebBrowserSetHTMLAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    return 0;

  IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
  int size = strlen(value)+1;

  // Create the memory for the stream
  HGLOBAL hHTMLText = GlobalAlloc(GPTR, size);
  strcpy((char*)hHTMLText, value); // weird but we follow the tutorial on MSDN

  // Create the stream
  IStream* pStream = NULL;
  CreateStreamOnHGlobal(hHTMLText, TRUE, &pStream);

  // Retrieve the document object.
  IDispatch* pHtmlDoc = NULL;
  pweb->get_Document(&pHtmlDoc);
  if (!pHtmlDoc)
  {
    pweb->Navigate(L"about:blank", NULL, NULL, NULL, NULL);
    IupFlush();

    pweb->get_Document(&pHtmlDoc);
  }

  // Query for IPersistStreamInit.
  IPersistStreamInit* pPersistStreamInit = NULL;
  pHtmlDoc->QueryInterface(IID_IPersistStreamInit, (void**)&pPersistStreamInit);
  //Initialize the document.
  pPersistStreamInit->InitNew();
  // Load the contents of the stream.
  pPersistStreamInit->Load(pStream);

  // Releases
  pPersistStreamInit->Release();
  pStream->Release();
  pHtmlDoc->Release();
  GlobalFree(hHTMLText);
  
  return 0; /* do not store value in hash table */
}

static int winWebBrowserSetBackForwardAttrib(Ihandle* ih, const char* value)
{
  int i, val;
  if (iupStrToInt(value, &val))
  {
    IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");

    /* Negative values represent steps backward while positive values represent steps forward. */
    if(val > 0)
    {
      for(i = 0; i < val; i++)
        pweb->GoForward();
    }
    else if(val < 0)
    {
      for(i = 0; i < -(val); i++)
        pweb->GoBack();
    }
  }

  return 0; /* do not store value in hash table */
}

static int winWebBrowserSetReloadAttrib(Ihandle* ih, const char* value)
{
  IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
  pweb->Refresh();

  (void)value;
  return 0; /* do not store value in hash table */
}

static int winWebBrowserSetStopAttrib(Ihandle* ih, const char* value)
{
  IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
  pweb->Stop();

  (void)value;
  return 0; /* do not store value in hash table */
}

static int winWebBrowserSetValueAttrib(Ihandle* ih, const char* value)
{
  if (value)
  {
    IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
    WCHAR* wvalue = iupwinStrChar2Wide(value);
    //CComVariant var = CComVariant("_top");
    //pweb->Navigate(wvalue, NULL, &var, NULL, NULL);
    pweb->Navigate(wvalue, NULL, NULL, NULL, NULL);
    free(wvalue);
  }
  return 0;
}

static char* winWebBrowserGetValueAttrib(Ihandle* ih)
{
  IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
  BSTR pbstrLocationURL = NULL;
  if (pweb->get_LocationURL(&pbstrLocationURL)==S_OK && pbstrLocationURL)
  {
    char* str = iupwinStrWide2Char(pbstrLocationURL);
    SysFreeString(pbstrLocationURL);
    char* value = iupStrGetMemoryCopy(str);
    free(str);
    return value;
  }
  return NULL;
}

static int winWebBrowserCreateMethod(Ihandle* ih, void **params)
{
  (void)params;
  IupSetAttribute(ih, "PROGID", "Shell.Explorer.2");
  IupSetAttribute(ih, "DESIGNMODE", "NO");

  // Get the current IUnknown*
  IUnknown *punk = (IUnknown*)IupGetAttribute(ih, "IUNKNOWN");

  IWebBrowser2 *pweb = NULL;
  punk->QueryInterface(IID_IWebBrowser2, (void **)&pweb);
  iupAttribSetStr(ih, "_IUPWEB_BROWSER", (char*)pweb);

  // CComModule implements a COM server module, allowing a client to access the module's components
  CComModule* module = new CComModule();

  // CSink object to capture events
  CSink* sink = new CSink();

  // Set handle to use in CSink Interface
  sink->ih = ih;

  // Initializing ATL Support
  module->Init(NULL, GetModuleHandle(NULL));

  // Connecting to the server's outgoing interface
  sink->DispEventAdvise(punk);

  iupAttribSetStr(ih, "_IUPWEB_MODULE", (char*)module);
  iupAttribSetStr(ih, "_IUPWEB_SINK", (char*)sink);
  punk->Release();

  return IUP_NOERROR; 
}

static void winWebBrowserDestroyMethod(Ihandle* ih)
{
  IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
  pweb->Release();

  CComModule* module = (CComModule*)iupAttribGet(ih, "_IUPWEB_MODULE");
  CSink* sink = (CSink*)iupAttribGet(ih, "_IUPWEB_SINK");

  // Get the current IUnknown*
  IUnknown *punk = (IUnknown*)IupGetAttribute(ih, "IUNKNOWN");

  // Disconnecting from the server's outgoing interface
  sink->DispEventUnadvise(punk);
  delete sink;

  // Terminating ATL support
  module->Term();
  delete module;

  punk->Release();
}

extern "C" Iclass* iupOleControlGetClass(void);

Iclass* iupWebBrowserGetClass(void)
{
  Iclass* ic = iupClassNew(iupOleControlGetClass());

  ic->name = "webbrowser";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype  = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->Create = winWebBrowserCreateMethod;
  ic->Destroy = winWebBrowserDestroyMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "NEWWINDOW_CB", "s");
  iupClassRegisterCallback(ic, "NAVIGATE_CB", "ss");

  /* Attributes */
  iupClassRegisterAttribute(ic, "VALUE", winWebBrowserGetValueAttrib, winWebBrowserSetValueAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKFORWARD", NULL, winWebBrowserSetBackForwardAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STOP", NULL, winWebBrowserSetStopAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RELOAD", NULL, winWebBrowserSetReloadAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HTML", NULL, winWebBrowserSetHTMLAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  return ic;
}
