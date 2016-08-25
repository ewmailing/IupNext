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

#include <Shlwapi.h>


#pragma comment(lib, "Shlwapi.lib")


extern HINSTANCE  g_hInstDll;
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
  : m_cRef(1), m_pPathFile(NULL), m_hwndPreview(NULL), 
    m_hwndParent(NULL), m_punkSite(NULL)
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
    LocalFree(m_pPathFile);
    m_pPathFile = NULL;
  }

  if (m_hwndPreview)
  {
    DestroyWindow(m_hwndPreview);
    m_hwndPreview = NULL;
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
      LocalFree(m_pPathFile);
      m_pPathFile = NULL;
    }

    m_pPathFile = StrDup(pszFilePath);
    hr = S_OK;
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
    m_rcParent = *prc;    // Cache the RECT for later use

    if (m_hwndPreview)
    {
      // Update preview window parent and rect information
      SetParent(m_hwndPreview, m_hwndParent);
      SetWindowPos(m_hwndPreview, NULL, m_rcParent.left, m_rcParent.top,
                   RECTWIDTH(m_rcParent), RECTHEIGHT(m_rcParent),
                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
  }
  return S_OK;
}

// Directs the preview handler to set focus to itself.
IFACEMETHODIMP IupPreviewHandler::SetFocus()
{
  HRESULT hr = S_FALSE;
  if (m_hwndPreview)
  {
    ::SetFocus(m_hwndPreview);
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
    m_rcParent = *prc;
    if (m_hwndPreview)
    {
      // Preview window is already created, so set its size and position.
      SetWindowPos(m_hwndPreview, NULL, m_rcParent.left, m_rcParent.top,
                   (m_rcParent.right - m_rcParent.left), // Width
                   (m_rcParent.bottom - m_rcParent.top), // Height
                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
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
  if (m_hwndPreview != NULL || !m_pPathFile)
    return E_FAIL;

  // Create the preview window.
  CreatePreviewWindow();

  return S_OK;
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

  if (m_hwndPreview)
  {
    DestroyWindow(m_hwndPreview);
    m_hwndPreview = NULL;
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

#include <iup.h>
#include "IupPreviewCanvas.h"

extern "C" void iupwinSetInstance(HINSTANCE hInstance);

// Create the preview window based on the recipe information.
HRESULT IupPreviewHandler::CreatePreviewWindow()
{
  HRESULT hr = S_OK;

  iupwinSetInstance(g_hInstDll);

  IupOpen(NULL, NULL);

  Ihandle* cnv = IupPreviewCanvasCreate();

  Ihandle* m_dialog = IupDialog(cnv);
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

  IupSetStrf(m_dialog, "RASTERSIZE", "%dx%d", RECTWIDTH(m_rcParent), RECTHEIGHT(m_rcParent));
  IupMap(m_dialog);
  IupSetAttribute(m_dialog, "RASTERSIZE", NULL);

  m_hwndPreview = (HWND)IupGetAttribute(m_dialog, "HWND");

  // Set the preview window position.
  SetWindowPos(m_hwndPreview, NULL, m_rcParent.left, m_rcParent.top,
               RECTWIDTH(m_rcParent), RECTHEIGHT(m_rcParent),
               SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

  ShowWindow(m_hwndPreview, SW_SHOW);

  return hr;
}

// MessageBox(NULL, L"ShowWindow-Fail", L"IUP", MB_OK);

#pragma endregion
