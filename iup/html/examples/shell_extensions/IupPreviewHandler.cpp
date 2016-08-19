/****************************** Module Header ******************************\
Module Name:  RecipePreviewHandler.cpp
Copyright (c) Microsoft Corporation.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "IupPreviewHandler.h"
#include "IupPreviewCanvas.h"

#include <Shlwapi.h>


#pragma comment(lib, "Shlwapi.lib")


extern long        g_cRefDll;


inline int RECTWIDTH(const RECT &rc)
{
  return (rc.right - rc.left);
}

inline int RECTHEIGHT(const RECT &rc)
{
  return (rc.bottom - rc.top);
}


IupPreviewHandler::IupPreviewHandler() 
  : m_cRef(1), m_pPathFile(NULL), m_hwndParent(NULL), m_punkSite(NULL), m_dialog(NULL)
{
  InterlockedIncrement(&g_cRefDll);
}

IupPreviewHandler::~IupPreviewHandler()
{
  if (m_punkSite)
  {
    m_punkSite->Release();
    m_punkSite = NULL;
  }

  if (m_pPathFile)
  {
    m_pPathFile = NULL;
  }

  if (m_dialog)
  {
    IupDestroy(m_dialog);
    m_dialog = NULL;

    IupClose();
  }

  InterlockedDecrement(&g_cRefDll);
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP IupPreviewHandler::QueryInterface(REFIID riid, void **ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(IupPreviewHandler, IPreviewHandler),
    QITABENT(IupPreviewHandler, IInitializeWithFile),
    QITABENT(IupPreviewHandler, IPreviewHandlerVisuals),
    QITABENT(IupPreviewHandler, IOleWindow),
    QITABENT(IupPreviewHandler, IObjectWithSite),
    { 0 },
  };
  return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) IupPreviewHandler::AddRef()
{
  return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) IupPreviewHandler::Release()
{
  ULONG cRef = InterlockedDecrement(&m_cRef);
  if (0 == cRef)
  {
    delete this;
  }

  return cRef;
}

#pragma endregion


#pragma region IInitializeWithFile

// Initializes the preview handler with a file path. 
// Store the path and mode parameters so that you can read the item's data 
// when you are ready to preview the item. Do not load the data in Initialize. 
// Load the data in IPreviewHandler::DoPreview just before you render.
IFACEMETHODIMP IupPreviewHandler::Initialize(LPCWSTR pszFilePath, DWORD grfMode)
{
  HRESULT hr = E_INVALIDARG;
  if (pszFilePath)
  {
    // Initialize can be called more than once, so release existing valid 
    // m_pStream.
    if (m_pPathFile)
    {
      m_pPathFile = NULL;
    }

    m_pPathFile = pszFilePath;
    hr = S_OK;

    if (m_dialog)
    {
      char str[10240];
      size_t size;
      wcstombs_s(&size, str, 10240, m_pPathFile, 10240);
      IupSetStrAttribute(m_dialog, "PATHFILE", str);
    }
  }
  return hr;
}

#pragma endregion


#pragma region IPreviewHandler

// This method gets called when the previewer gets created. It sets the parent 
// window of the previewer window, as well as the area within the parent to be 
// used for the previewer window.
IFACEMETHODIMP IupPreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
  if (hwnd && prc)
  {
    m_hwndParent = hwnd;  // Cache the HWND for later use
    RECT m_rcParent = *prc;    // Cache the RECT for later use

    m_width = RECTWIDTH(m_rcParent);
    m_height = RECTHEIGHT(m_rcParent);
    POINT pt;
    pt.x = m_rcParent.left;
    pt.y = m_rcParent.top;
    ClientToScreen(m_hwndParent, &pt);
    m_x = pt.x;
    m_y = pt.y;

    m_x = 0;
    m_y = 0;

    if (m_dialog)
    {
      IupUnmap(m_dialog);

      MessageBox(NULL, L"SetWindow-IupUnmap", L"IUP", MB_OK);

      IupSetAttribute(m_dialog, "NATIVEPARENT", (char*)m_hwndParent);

      IupSetStrf(m_dialog, "RASTERSIZE", "%dx%d", m_width, m_height);
      IupShowXY(m_dialog, m_x, m_y);
    }
  }
  return S_OK;
}

// Directs the preview handler to set focus to itself.
IFACEMETHODIMP IupPreviewHandler::SetFocus()
{
  HRESULT hr = S_FALSE;
  if (m_dialog)
  {
    IupSetFocus(IupGetChild(m_dialog, 0));
    hr = S_OK;
  }
  return hr;
}

// Directs the preview handler to return the HWND from calling the GetFocus 
// function.
IFACEMETHODIMP IupPreviewHandler::QueryFocus(HWND *phwnd)
{
  HRESULT hr = E_INVALIDARG;
  if (phwnd)
  {
    *phwnd = ::GetFocus();
    if (*phwnd)
    {
      hr = S_OK;
    }
    else
    {
      hr = HRESULT_FROM_WIN32(GetLastError());
    }
  }
  return hr;
}

// Directs the preview handler to handle a keystroke passed up from the 
// message pump of the process in which the preview handler is running.
IFACEMETHODIMP IupPreviewHandler::TranslateAccelerator(MSG *pmsg)
{
  HRESULT hr = S_FALSE;
  IPreviewHandlerFrame *pFrame = NULL;
  if (m_punkSite && SUCCEEDED(m_punkSite->QueryInterface(&pFrame)))
  {
    // If your previewer has multiple tab stops, you will need to do 
    // appropriate first/last child checking. This sample previewer has 
    // no tabstops, so we want to just forward this message out.
    hr = pFrame->TranslateAccelerator(pmsg);

    pFrame->Release();
  }
  return hr;
}

// This method gets called when the size of the previewer window changes 
// (user resizes the Reading Pane). It directs the preview handler to change 
// the area within the parent hwnd that it draws into.
IFACEMETHODIMP IupPreviewHandler::SetRect(const RECT *prc)
{
  HRESULT hr = E_INVALIDARG;
  if (prc != NULL)
  {
    RECT m_rcParent = *prc;

    m_width = RECTWIDTH(m_rcParent);
    m_height = RECTHEIGHT(m_rcParent);
    POINT pt;
    pt.x = m_rcParent.left;
    pt.y = m_rcParent.top;
    ClientToScreen(m_hwndParent, &pt);
    m_x = pt.x;
    m_y = pt.y;

    m_x = 0;
    m_y = 0;

    if (m_dialog)
    {
      // Preview window is already created, so set its size and position.
      IupSetStrf(m_dialog, "RASTERSIZE", "%dx%d", m_width, m_height);
      IupShowXY(m_dialog, m_x, m_y);
    }
    hr = S_OK;
  }
  return hr;
}

// The method directs the preview handler to load data from the source 
// specified in an earlier Initialize method call, and to begin rendering to 
// the previewer window.
IFACEMETHODIMP IupPreviewHandler::DoPreview()
{
  // Cannot call more than once.
  // (Unload should be called before another DoPreview)
  if (m_dialog != NULL || !m_pPathFile)
  {
    MessageBox(NULL, L"DoPreview-Abort", L"IUP", MB_OK);
    return E_FAIL;
  }

  HRESULT hr = E_FAIL;

  // Create the preview window.
  hr = CreatePreviewWindow();
  if (FAILED(hr))
  {
    goto Cleanup;
  }

Cleanup:
  // Clean up the allocated resources in a centralized place.

  return hr;
}

// This method gets called when a shell item is de-selected. It directs the 
// preview handler to cease rendering a preview and to release all resources 
// that have been allocated based on the item passed in during the 
// initialization.
IFACEMETHODIMP IupPreviewHandler::Unload()
{
  if (m_pPathFile)
  {
    m_pPathFile = NULL;
  }

  if (m_dialog)
  {
    IupDestroy(m_dialog);
    m_dialog = NULL;

    MessageBox(NULL, L"IupDestroy", L"IUP", MB_OK);

    IupClose();
  }

  return S_OK;
}

#pragma endregion


#pragma region IPreviewHandlerVisuals (Optional)

// Sets the background color of the preview handler.
IFACEMETHODIMP IupPreviewHandler::SetBackgroundColor(COLORREF color)
{
  return S_OK;
}

// Sets the font attributes to be used for text within the preview handler.
IFACEMETHODIMP IupPreviewHandler::SetFont(const LOGFONTW *plf)
{
  return S_OK;
}

// Sets the color of the text within the preview handler.
IFACEMETHODIMP IupPreviewHandler::SetTextColor(COLORREF color)
{
  return S_OK;
}

#pragma endregion


#pragma region IOleWindow

// Retrieves a handle to one of the windows participating in in-place 
// activation (frame, document, parent, or in-place object window).
IFACEMETHODIMP IupPreviewHandler::GetWindow(HWND *phwnd)
{
  HRESULT hr = E_INVALIDARG;
  if (phwnd)
  {
    *phwnd = m_hwndParent;
    hr = S_OK;
  }
  return hr;
}

// Determines whether context-sensitive help mode should be entered during an 
// in-place activation session
IFACEMETHODIMP IupPreviewHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
  return E_NOTIMPL;
}

#pragma endregion


#pragma region IObjectWithSite

// Provides the site's IUnknown pointer to the object.
IFACEMETHODIMP IupPreviewHandler::SetSite(IUnknown *punkSite)
{
  if (m_punkSite)
  {
    m_punkSite->Release();
    m_punkSite = NULL;
  }
  return punkSite ? punkSite->QueryInterface(&m_punkSite) : S_OK;
}

// Gets the last site set with IObjectWithSite::SetSite. If there is no known 
// site, the object returns a failure code.
IFACEMETHODIMP IupPreviewHandler::GetSite(REFIID riid, void **ppv)
{
  *ppv = NULL;
  return m_punkSite ? m_punkSite->QueryInterface(riid, ppv) : E_FAIL;
}

#pragma endregion


#pragma region Helper Functions

// Create the preview window based on the recipe information.
HRESULT IupPreviewHandler::CreatePreviewWindow()
{
  HRESULT hr = S_OK;

  IupOpen(NULL, NULL);          

  MessageBox(NULL, L"IupOpen", L"IUP", MB_OK);

  Ihandle* cnv = IupPreviewCanvasCreate();

  m_dialog = IupDialog(cnv);
  IupSetAttribute(m_dialog, "BORDER", "NO");
  IupSetAttribute(m_dialog, "MAXBOX", "NO");
  IupSetAttribute(m_dialog, "MINBOX", "NO");
  IupSetAttribute(m_dialog, "MENUBOX", "NO");
  IupSetAttribute(m_dialog, "RESIZE", "NO");
  IupSetAttribute(m_dialog, "CONTROL", "YES");

  char str[10240];
  size_t size;
  wcstombs_s(&size, str, 10240, m_pPathFile, 10240);
  IupSetStrAttribute(m_dialog, "PATHFILE", str);

  IupSetAttribute(m_dialog, "NATIVEPARENT", (char*)m_hwndParent);

  IupSetStrf(m_dialog, "RASTERSIZE", "%dx%d", m_width, m_height);
  IupShowXY(m_dialog, m_x, m_y);

  IupFlush();

  return hr;
}


#pragma endregion
