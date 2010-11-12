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

  BEGIN_SINK_MAP(CSink)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW3, NewWindow3)
  END_SINK_MAP()

  void STDMETHODCALLTYPE BeforeNavigate2(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName,
                                         VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
  {
    Ihandle* ih = IupGetHandle("_IUPWIN_WEBBROWSER_HANDLE");

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
    Ihandle* ih = IupGetHandle("_IUPWIN_WEBBROWSER_HANDLE");
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

// CComModule implements a COM server module, allowing a client to access the module's components
CComModule _Module;
// CSink object to capture events
CSink sink;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */
};

static void winWebBrowserConnectServerSink(Ihandle* ih)
{
  // Get the current IUnknown*
  IUnknown *punk = (IUnknown*)IupGetAttribute(ih, "IUNKNOWN");

  // Get handle to use in CSink Interface
  IupSetHandle("_IUPWIN_WEBBROWSER_HANDLE", ih);

  // Entering primary STA
  CoInitialize(NULL);

  // Initializing ATL Support
  _Module.Init(NULL, GetModuleHandle(NULL));

  // Connecting to the server's outgoing interface
  sink.DispEventAdvise(punk);
}

static void winWebBrowserDisconnectServerSink(Ihandle* ih)
{
  // Get the current IUnknown*
  IUnknown *punk = (IUnknown*)IupGetAttribute(ih, "IUNKNOWN");

  // Disconnecting from the server's outgoing interface
  sink.DispEventUnadvise(punk);

  // Terminating ATL support
  _Module.Term();

  // Quitting primary STA
  CoUninitialize();
}

static int winWebBrowserCreateMethod(Ihandle* ih, void **params)
{
  (void)params;
  ih->data = iupALLOCCTRLDATA();
  IupSetAttribute(ih, "PROGID", "Shell.Explorer.2");

  winWebBrowserConnectServerSink(ih);

  return IUP_NOERROR; 
}

static void winWebBrowserDestroyMethod(Ihandle* ih)
{
  winWebBrowserDisconnectServerSink(ih);
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

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "NEWWINDOW_CB", "s");
  iupClassRegisterCallback(ic, "NAVIGATE_CB", "ss");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  return ic;
}
