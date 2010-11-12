/** \file
* \brief Web Browser Control
*
* See Copyright Notice in "iup.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

///////////////////////////////////////////////////////////////////////////////
// Interface to get WebBrowserEvents
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#include <shlguid.h>   // IID_IWebBrowser2, DIID_DWebBrowserEvents2
#include <exdispid.h>  // DISPID_DOCUMENTCOMPLETE

#include <comutil.h>   // ConvertBSTRToString
#pragma comment(lib, "comsuppw.lib")  // ConvertBSTRToString

using namespace ATL;

interface CSink:public IDispEventImpl<0, CSink, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 0>
{
public:
  Ihandle* ih;

  BEGIN_SINK_MAP(CSink)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW3, NewWindow3)
  END_SINK_MAP()

  void STDMETHODCALLTYPE BeforeNavigate2(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName,
                                         VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
  {
    IFnss cbNavigate = (IFnss)IupGetCallback(ih, "NAVIGATE_CB");
    if (cbNavigate)
    {
      char* resultString = _com_util::ConvertBSTRToString(url->bstrVal);
      cbNavigate(ih, "NOREASON", resultString);
      // TO DO: detect types of action, like "link_clicked", "refresh", "form submitted", "backward/forward"
    }

    (void)Cancel;
    (void)Headers;
    (void)PostData;
    (void)TargetFrameName;
    (void)Flags;
    (void)pDisp;
  }

  void STDMETHODCALLTYPE NewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags,
                                    BSTR bstrUrlContext, BSTR bstrUrl)
  {
    IFns cbNewWindow = (IFns)IupGetCallback(ih, "NEWWINDOW_CB");
    if (cbNewWindow)
    {
      char* urlString = _com_util::ConvertBSTRToString(bstrUrl);
      cbNewWindow(ih, urlString);
    }

    (void)bstrUrlContext;
    (void)dwFlags;
    (void)Cancel;
    (void)ppDisp;
  }
};

static WCHAR* winWebBrowserChar2Wide(const char* str)
{
  int n = strlen(str)+1;
  WCHAR* wstr = (WCHAR*)malloc(n * sizeof(WCHAR));
  MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, n);
  return wstr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int winWebBrowserSetLoadAttrib(Ihandle* ih, const char* value)
{
  if (value)
  {
    IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
    WCHAR* wvalue = winWebBrowserChar2Wide(value);
    pweb->Navigate(wvalue, NULL, NULL, NULL, NULL);
    free(wvalue);
  }
  return 0;
}

static int winWebBrowserCreateMethod(Ihandle* ih, void **params)
{
  (void)params;
  IupSetAttribute(ih, "PROGID", "Shell.Explorer.2");

  // Get the current IUnknown*
  IUnknown *punk = (IUnknown*)IupGetAttribute(ih, "IUNKNOWN");

  IWebBrowser2 *pweb = NULL;
  punk->QueryInterface(IID_IWebBrowser2, (void **)&pweb);
  iupAttribSetStr(ih, "_IUPWEB_BROWSER", (char*)pweb);

  //// CComModule implements a COM server module, allowing a client to access the module's components
  //CComModule* module = new CComModule();

  //// CSink object to capture events
  //CSink* sink = new CSink();

  //// Set handle to use in CSink Interface
  //sink->ih = ih;

  //// Initializing ATL Support
  //module->Init(NULL, GetModuleHandle(NULL));

  //// Connecting to the server's outgoing interface
  //sink->DispEventAdvise(punk);

  //iupAttribSetStr(ih, "_IUPWEB_MODULE", (char*)module);
  //iupAttribSetStr(ih, "_IUPWEB_SINK", (char*)sink);
  punk->Release();

  return IUP_NOERROR; 
}

static void winWebBrowserDestroyMethod(Ihandle* ih)
{
  IWebBrowser2 *pweb = (IWebBrowser2*)iupAttribGet(ih, "_IUPWEB_BROWSER");
  pweb->Release();

  //CComModule* module = (CComModule*)iupAttribGet(ih, "_IUPWEB_MODULE");
  //CSink* sink = (CSink*)iupAttribGet(ih, "_IUPWEB_SINK");

  //// Get the current IUnknown*
  //IUnknown *punk = (IUnknown*)IupGetAttribute(ih, "IUNKNOWN");

  //// Disconnecting from the server's outgoing interface
  //sink->DispEventUnadvise(punk);
  //delete sink;

  //// Terminating ATL support
  //module->Term();
  //delete module;
}

Iclass* iupWebBrowserGetClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("olecontrol"));

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
  iupClassRegisterAttribute(ic, "LOAD", NULL, winWebBrowserSetLoadAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "BACKFORWARD", NULL, gtkWebBrowserSetBackForwardAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "STOP", NULL, gtkWebBrowserSetStopAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "RELOAD", NULL, gtkWebBrowserSetReloadAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  return ic;
}
