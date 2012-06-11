/** \file
 * \brief Windows Drag&Drop Functions
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>              
#include <stdlib.h>
#include <string.h>             

#include <windows.h>
#include <commctrl.h>
#include <ole2.h>
#include <shlobj.h>
#include <tchar.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_key.h"
#include "iup_str.h"
#include "iup_class.h"
#include "iup_attrib.h"
#include "iup_focus.h"
#include "iup_image.h"
#include "iup_dialog.h"
#include "iup_drvinfo.h"
#include "iup_drv.h"
#include "iup_array.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_brush.h"
#include "iupwin_info.h"


typedef struct tagIUPWINDROPSOURCE *PIUPWINDROPSOURCE;
typedef struct tagIUPWINDROPTARGET *PIUPWINDROPTARGET;
typedef DWORD (*IUPWINDDCALLBACK)(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);

static PIUPWINDROPSOURCE winCreateDropSourceData(CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt);
static PIUPWINDROPSOURCE winFreeDropSource(PIUPWINDROPSOURCE pDropSrc);
static PIUPWINDROPTARGET winRegisterDropTarget(HWND hWnd, CLIPFORMAT *pFormat, ULONG lFmt, IUPWINDDCALLBACK, void *pUserData);
static PIUPWINDROPTARGET winRevokeDropTarget(PIUPWINDROPTARGET pTarget);

static IDataObject *winCreateDataObject(CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt);
static IDropSource *winCreateDropSource(void);
static IDropTarget *winCreateDropTarget(CLIPFORMAT *pFormat, ULONG lFmt, HWND hWnd, IUPWINDDCALLBACK pDropProc, void *pUserData);

typedef struct tagIUPWINIDATAOBJECT
{
  IDataObject ido;
  LONG lRefCount;
  FORMATETC *pFormatEtc;
  HGLOBAL *phData;
  ULONG lNumFormats;
} IUPWINIDATAOBJECT, *PIUPWINIDATAOBJECT;

typedef struct tagIUPWINIDROPSOURCE
{
  IDropSource ids;
  LONG lRefCount;
} IUPWINIDROPSOURCE, *PIUPWINIDROPSOURCE;

typedef struct tagIUPWINIDROPTARGET
{
  IDropTarget idt;
  LONG lRefCount;
  ULONG lNumFormats;
  CLIPFORMAT *pFormat;
  HWND hWnd;
  DWORD dwKeyState;
  IDataObject *pDataObject;
  void *pUserData;
  IUPWINDDCALLBACK pDropProc;
} IUPWINIDROPTARGET, *PIUPWINIDROPTARGET;

typedef struct tagIUPWINDROPSOURCE
{
  IDataObject *pObj;
  IDropSource *pSrc;
  HANDLE hData;
} IUPWINDROPSOURCE, *PIUPWINDROPSOURCE;

typedef struct tagIUPWINDROPTARGET
{
  void *nothing;
} IUPWINDROPTARGET, *PIUPWINDROPTARGET;

typedef struct tagIUPWINIDATAOBJECT_VTBL
{
  BEGIN_INTERFACE
  HRESULT (STDMETHODCALLTYPE *QueryInterface)(PIUPWINIDATAOBJECT pThis, REFIID riid, void **ppvObject);
  ULONG   (STDMETHODCALLTYPE *AddRef)(PIUPWINIDATAOBJECT pThis);
  ULONG   (STDMETHODCALLTYPE *Release)(PIUPWINIDATAOBJECT pThis);
  HRESULT (STDMETHODCALLTYPE *GetData)(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtcIn, STGMEDIUM *pMedium);
  HRESULT (STDMETHODCALLTYPE *GetDataHere)(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc, STGMEDIUM *pMedium);
  HRESULT (STDMETHODCALLTYPE *QueryGetData)(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc);
  HRESULT (STDMETHODCALLTYPE *GetCanonicalFormatEtc)(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtcIn, FORMATETC *pFormatEtcOut);
  HRESULT (STDMETHODCALLTYPE *SetData)(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease);
  HRESULT (STDMETHODCALLTYPE *EnumFormatEtc)(PIUPWINIDATAOBJECT pThis, DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
  HRESULT (STDMETHODCALLTYPE *DAdvise)(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
  HRESULT (STDMETHODCALLTYPE *DUnadvise)(PIUPWINIDATAOBJECT pThis, DWORD dwConnection);
  HRESULT (STDMETHODCALLTYPE *EnumDAdvise)(PIUPWINIDATAOBJECT pThis, IEnumSTATDATA **ppEnumAdvise);
  END_INTERFACE
} IUPWINIDATAOBJECT_VTBL, *PIUPWINIDATAOBJECT_VTBL;

typedef struct IUPWINIDROPSOURCE_VTBL
{
  BEGIN_INTERFACE
  HRESULT (STDMETHODCALLTYPE *QueryInterface)(PIUPWINIDROPSOURCE pThis, REFIID riid, void **ppvObject);
  ULONG   (STDMETHODCALLTYPE *AddRef)(PIUPWINIDROPSOURCE pThis);
  ULONG   (STDMETHODCALLTYPE *Release)(PIUPWINIDROPSOURCE pThis);
  HRESULT (STDMETHODCALLTYPE *QueryContinueDrag)(PIUPWINIDROPSOURCE pThis, BOOL fEscapePressed, DWORD dwKeyState);
  HRESULT (STDMETHODCALLTYPE *GiveFeedback)(PIUPWINIDROPSOURCE pThis, DWORD dwEffect);
  END_INTERFACE
} IUPWINIDROPSOURCE_VTBL, *PIUPWINIDROPSOURCE_VTBL;

typedef struct IUPWINIDROPTARGET_VTBL
{
  BEGIN_INTERFACE
  HRESULT (STDMETHODCALLTYPE *QueryInterface)(PIUPWINIDROPTARGET pThis, REFIID riid, void **ppvObject);
  ULONG   (STDMETHODCALLTYPE *AddRef)(PIUPWINIDROPTARGET pThis);
  ULONG   (STDMETHODCALLTYPE *Release)(PIUPWINIDROPTARGET pThis);
  HRESULT (STDMETHODCALLTYPE *DragEnter)(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  HRESULT (STDMETHODCALLTYPE *DragOver)(PIUPWINIDROPTARGET pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  HRESULT (STDMETHODCALLTYPE *DragLeave)(PIUPWINIDROPTARGET pThis);
  HRESULT (STDMETHODCALLTYPE *Drop)(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  END_INTERFACE
} IUPWINIDROPTARGET_VTBL, *PIUPWINIDROPTARGET_VTBL;

/* Utilities. */
static int winLookupFormatEtc(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc);
static BOOL winQueryDataObject(PIUPWINIDROPTARGET pDropTarget, IDataObject *pDataObject);

/* IUPWINIDROPSOURCE methods. */
static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryInterface(PIUPWINIDROPSOURCE pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPSOURCE_AddRef(PIUPWINIDROPSOURCE pThis);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPSOURCE_Release(PIUPWINIDROPSOURCE pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryContinueDrag(PIUPWINIDROPSOURCE pThis, BOOL fEscapePressed, DWORD grfKeyState);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_GiveFeedback(PIUPWINIDROPSOURCE pThis, DWORD dwEffect);

/* IUPWINIDATAOBJECT methods. */
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_QueryInterface(PIUPWINIDATAOBJECT pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IUPWINIDATAOBJECT_AddRef(PIUPWINIDATAOBJECT pThis);
static ULONG   STDMETHODCALLTYPE IUPWINIDATAOBJECT_Release(PIUPWINIDATAOBJECT pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetDataHere(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_QueryGetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetCanonicalFormatEtc(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtcIn, LPFORMATETC pFormatEtcOut);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_SetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium, BOOL fRelease);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_EnumFormatEtc(PIUPWINIDATAOBJECT pThis, DWORD dwDirection, LPENUMFORMATETC *ppEnumFormatEtc);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_DAdvise(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_Dunadvise(PIUPWINIDATAOBJECT pThis, DWORD dwConnection);
static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_EnumDAdvise(PIUPWINIDATAOBJECT pThis, LPENUMSTATDATA *ppEnumAdvise);

/* IUPWINIDROPTARGET methods. */
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_QueryInterface(PIUPWINIDROPTARGET pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPTARGET_AddRef(PIUPWINIDROPTARGET pThis);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPTARGET_Release(PIUPWINIDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragEnter(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragOver(PIUPWINIDROPTARGET pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragLeave(PIUPWINIDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_Drop(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);


static PIUPWINDROPSOURCE winCreateDropSourceData(CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt)
{
  PIUPWINDROPSOURCE pRet;

  if((pRet = HeapAlloc(GetProcessHeap(), 0, sizeof(IUPWINDROPSOURCE))) == NULL)
    return NULL;

  /* Create the two members. */
  if((pRet->pSrc = winCreateDropSource()) == NULL)
  {
    HeapFree(GetProcessHeap(), 0, pRet);
    return NULL;
  }
  if((pRet->pObj = winCreateDataObject(pFormat, phData, lFmt)) == NULL)
  {
    pRet->pSrc->lpVtbl->Release(pRet->pSrc);
    HeapFree(GetProcessHeap(), 0, pRet);
    return NULL;
  }
  pRet->hData = NULL;

  return pRet;
}

static PIUPWINDROPSOURCE winFreeDropSource(PIUPWINDROPSOURCE pDropSrc)
{
  if(pDropSrc == NULL)
    return NULL;

  /* Release the members. */
  pDropSrc->pSrc->lpVtbl->Release(pDropSrc->pSrc);
  pDropSrc->pObj->lpVtbl->Release(pDropSrc->pObj);

  /* Free the managed hData member, if it is allocated. */
  if(pDropSrc->hData != NULL)
    GlobalFree(pDropSrc->hData);

  /* Free the struct itself. */
  HeapFree(GetProcessHeap(), 0, pDropSrc);

  return NULL;
}

static PIUPWINDROPTARGET winRegisterDropTarget(HWND hWnd, CLIPFORMAT *pFormat, ULONG lFmt, IUPWINDDCALLBACK pDropProc, void *pUserData)
{
  IDropTarget *pTarget;

  /* First, create the target. */
  if((pTarget = winCreateDropTarget(pFormat, lFmt, hWnd, pDropProc, pUserData)) == NULL)
    return NULL;

  /* Now, register for drop. If this fails, free my target the old-fashioned way, as none knows about it anyway. */
  if(RegisterDragDrop(hWnd, pTarget) != S_OK)
  {
    HeapFree(GetProcessHeap(), 0, pTarget);
    return NULL;
  }

  return (PIUPWINDROPTARGET) pTarget;
}

static PIUPWINDROPTARGET winRevokeDropTarget(PIUPWINDROPTARGET pTarget)
{
  if(pTarget == NULL)
    return NULL;

  /* If there is a HWND, then revoke it as a drop object. */
  if(((PIUPWINIDROPTARGET) pTarget)->hWnd != NULL)
  {
    /* Now, this is a little precaution to know that this is an OK PMIIDROPTARGET object. */
    if(GetWindowLong(((PIUPWINIDROPTARGET) pTarget)->hWnd, GWL_WNDPROC) != 0)
      RevokeDragDrop(((PIUPWINIDROPTARGET) pTarget)->hWnd);
  }

  /* Now, release the target. */
  ((IDropTarget *) pTarget)->lpVtbl->Release((IDropTarget *) pTarget);

  return NULL;
}

static IDropSource *winCreateDropSource(void)
{
  PIUPWINIDROPSOURCE pRet;
  static IUPWINIDROPSOURCE_VTBL ids_vtbl = {
    IUPWINIDROPSOURCE_QueryInterface,
    IUPWINIDROPSOURCE_AddRef,
    IUPWINIDROPSOURCE_Release,
    IUPWINIDROPSOURCE_QueryContinueDrag,
    IUPWINIDROPSOURCE_GiveFeedback};

  if((pRet = HeapAlloc(GetProcessHeap(), 0, sizeof(IUPWINIDROPSOURCE))) == NULL)
    return NULL;

  pRet->ids.lpVtbl = (IDropSourceVtbl*)&ids_vtbl;
  pRet->lRefCount = 1;

  return (IDropSource *) pRet;
}

static IDataObject *winCreateDataObject(CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt)
{
  PIUPWINIDATAOBJECT pRet;
  ULONG i;
  static IUPWINIDATAOBJECT_VTBL ido_vtbl = {
    IUPWINIDATAOBJECT_QueryInterface,
    IUPWINIDATAOBJECT_AddRef,
    IUPWINIDATAOBJECT_Release,
    IUPWINIDATAOBJECT_GetData,
    IUPWINIDATAOBJECT_GetDataHere,
    IUPWINIDATAOBJECT_QueryGetData,
    IUPWINIDATAOBJECT_GetCanonicalFormatEtc,
    IUPWINIDATAOBJECT_SetData,
    IUPWINIDATAOBJECT_EnumFormatEtc,
    IUPWINIDATAOBJECT_DAdvise,
    IUPWINIDATAOBJECT_Dunadvise,
    IUPWINIDATAOBJECT_EnumDAdvise};

  if((pRet = HeapAlloc(GetProcessHeap(), 0, 
      sizeof(IUPWINIDATAOBJECT) + lFmt * (sizeof(FORMATETC) + sizeof(HGLOBAL)))) == NULL)
    return NULL;

  pRet->pFormatEtc = (FORMATETC *) (((char *) pRet) + sizeof(IUPWINIDATAOBJECT));
  pRet->phData = (HGLOBAL *) (((char *) pRet->pFormatEtc) + lFmt * sizeof(FORMATETC));

  /* Initialize data object. */
  pRet->lRefCount  = 1;
  pRet->lNumFormats = lFmt;
  pRet->ido.lpVtbl = (IDataObjectVtbl*)&ido_vtbl;

  /* Set the format and the handles. */
  for(i = 0; i < lFmt; i++)
  {
    pRet->pFormatEtc[i].cfFormat = pFormat[i];
    pRet->pFormatEtc[i].dwAspect = DVASPECT_CONTENT;
    pRet->pFormatEtc[i].ptd = NULL;
    pRet->pFormatEtc[i].lindex = -1;
    pRet->pFormatEtc[i].tymed = TYMED_HGLOBAL;
    pRet->phData[i] = phData[i];
  }

  return (IDataObject *) pRet;
}

static IDropTarget *winCreateDropTarget(CLIPFORMAT *pFormat, ULONG lFmt, HWND hWnd, 
                                 DWORD (*pDropProc)(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData),
                                 void *pUserData)
{
  PIUPWINIDROPTARGET pRet;
  static IUPWINIDROPTARGET_VTBL idt_vtbl = {
    IUPWINIDROPTARGET_QueryInterface,
    IUPWINIDROPTARGET_AddRef,
    IUPWINIDROPTARGET_Release,
    IUPWINIDROPTARGET_DragEnter,
    IUPWINIDROPTARGET_DragOver,
    IUPWINIDROPTARGET_DragLeave,
    IUPWINIDROPTARGET_Drop};

  if((pRet = HeapAlloc(GetProcessHeap(), 0, 
      sizeof(IUPWINIDROPTARGET) + lFmt * sizeof(CLIPFORMAT))) == NULL)
    return NULL;

  pRet->pFormat = (CLIPFORMAT *) (((char *) pRet) + sizeof(IUPWINIDROPTARGET));

  /* Set up the struct members. */
  pRet->idt.lpVtbl = (IDropTargetVtbl*)&idt_vtbl;
  pRet->lRefCount = 1;
  pRet->hWnd = hWnd;
  pRet->dwKeyState = 0;
  pRet->lNumFormats = lFmt;
  pRet->pDropProc = pDropProc;
  pRet->pUserData = pUserData;

  /* Set the format members. */
  for(lFmt = 0; lFmt < pRet->lNumFormats; lFmt++)
    pRet->pFormat[lFmt] = pFormat[lFmt];

  return (IDropTarget *) pRet;
}

static int winLookupFormatEtc(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc)
{
  ULONG i;
  
  for(i = 0; i < pThis->lNumFormats; i++)
  {
    if((pFormatEtc->tymed & pThis->pFormatEtc[i].tymed)
      && pFormatEtc->cfFormat == pThis->pFormatEtc[i].cfFormat
      && pFormatEtc->dwAspect == pThis->pFormatEtc[i].dwAspect)
      return i;
  }

  return -1;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryInterface(PIUPWINIDROPSOURCE pThis, REFIID riid, LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if(IsEqualGUID(riid, &IID_IUnknown))
  {
    IUPWINIDROPSOURCE_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  else if(IsEqualGUID(riid, &IID_IDropSource))
  {
    IUPWINIDROPSOURCE_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }

  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IUPWINIDROPSOURCE_AddRef(PIUPWINIDROPSOURCE pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}

static ULONG STDMETHODCALLTYPE IUPWINIDROPSOURCE_Release(PIUPWINIDROPSOURCE pThis)
{
  LONG nCount = InterlockedDecrement(&pThis->lRefCount);
  if(nCount == 0)
  {
    HeapFree(GetProcessHeap(), 0, pThis);
    return 0;
  }
  return nCount;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryContinueDrag(PIUPWINIDROPSOURCE pThis, BOOL fEscapePressed, DWORD dwKeyState)
{
  (void)pThis;
  if(fEscapePressed)
    return DRAGDROP_S_CANCEL;
  else if((dwKeyState & MK_LBUTTON) == 0)
    return DRAGDROP_S_DROP;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_GiveFeedback(PIUPWINIDROPSOURCE pThis, DWORD dwEffect)
{
  (void)pThis;
  (void)dwEffect;
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_QueryInterface(PIUPWINIDATAOBJECT pThis, REFIID riid, LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if(IsEqualGUID(riid, &IID_IUnknown))
  {
    IUPWINIDATAOBJECT_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  else if(IsEqualGUID(riid, &IID_IDataObject))
  {
    IUPWINIDATAOBJECT_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }

  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IUPWINIDATAOBJECT_AddRef(PIUPWINIDATAOBJECT pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}

static ULONG STDMETHODCALLTYPE IUPWINIDATAOBJECT_Release(PIUPWINIDATAOBJECT pThis)
{
  LONG nCount = InterlockedDecrement(&pThis->lRefCount);
  if(nCount == 0)
  {
    HeapFree(GetProcessHeap(), 0, pThis);
    return 0;
  }
  return nCount;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  int nIndex;
  size_t nLen;
  void *pData;

  /* Try to match the requested FORMATETC with one of our supported formats */
  if((nIndex = winLookupFormatEtc(pThis, pFormatEtc)) == -1)
    return DV_E_FORMATETC;

  /* Set the medium hGlobal, which is for now the only supported format. */
  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease  = 0;

  /* Get the size of the source. */
  if((nLen = GlobalSize(pThis->phData[nIndex])) == 0)
    return STG_E_MEDIUMFULL;

  /* Allocate the destination. */
  if((pMedium->hGlobal = GlobalAlloc(GMEM_FIXED, nLen)) == NULL)
    return STG_E_MEDIUMFULL;

  /* Lock the object and get a pointer to it. */
  if((pData = GlobalLock(pThis->phData[nIndex])) == NULL)
  {
    GlobalFree(pMedium->hGlobal);
    return STG_E_MEDIUMFULL;
  }

  /* Copy the data. */
  memcpy(pMedium->hGlobal, pData, nLen);

  /* Unlock the memory. */
  GlobalUnlock(pThis->phData[nIndex]);

  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetDataHere(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  (void)pThis;
  (void)pFormatEtc;
  (void)pMedium;
  return DATA_E_FORMATETC;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_QueryGetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc)
{
  return winLookupFormatEtc(pThis, pFormatEtc) == -1 ? DV_E_FORMATETC : S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetCanonicalFormatEtc(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtcIn, LPFORMATETC pFormatEtcOut)
{
  (void)pThis;
  (void)pFormatEtcIn;
  pFormatEtcOut->ptd = NULL;
  return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_SetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium, BOOL fRelease)
{
  (void)pThis;
  (void)pFormatEtc;
  (void)pMedium;
  (void)fRelease;
  return E_UNEXPECTED;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_EnumFormatEtc(PIUPWINIDATAOBJECT pThis, DWORD dwDirection, LPENUMFORMATETC *ppEnumFormatEtc)
{
  if(dwDirection != DATADIR_GET)
    return E_NOTIMPL;

  /* Note that this is for W2K and up only. Before this, this will NOT work! */
  SHCreateStdEnumFmtEtc(pThis->lNumFormats, pThis->pFormatEtc, ppEnumFormatEtc);

  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_DAdvise(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
  (void)pThis;
  (void)pFormatetc;
  (void)advf;
  (void)pAdvSink;
  (void)pdwConnection;
  return E_FAIL;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_Dunadvise(PIUPWINIDATAOBJECT pThis, DWORD dwConnection)
{
  (void)pThis;
  (void)dwConnection;
  return E_FAIL;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_EnumDAdvise(PIUPWINIDATAOBJECT pThis, LPENUMSTATDATA *ppEnumAdvise)
{
  (void)pThis;
  (void)ppEnumAdvise;
  return E_FAIL;
}

static BOOL winQueryDataObject(PIUPWINIDROPTARGET pDropTarget, IDataObject *pDataObject)
{
  ULONG lFmt;
  FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  
  for(lFmt = 0; lFmt < pDropTarget->lNumFormats; lFmt++)
  {
    fmtetc.cfFormat = pDropTarget->pFormat[lFmt];
    if(pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
      return TRUE;
  }

  return FALSE;
}

static DWORD IUPWINIDROPTARGET_DropEffect(DWORD dwKeyState, POINTL pt, DWORD dwAllowed)
{
  DWORD dwEffect = 0;

  if(dwKeyState & MK_CONTROL)
    dwEffect = dwAllowed & DROPEFFECT_COPY;
  else if(dwKeyState & MK_SHIFT)
    dwEffect = dwAllowed & DROPEFFECT_MOVE;

  if(dwEffect == 0)
  {
    if(dwAllowed & DROPEFFECT_COPY)
      dwEffect = DROPEFFECT_COPY;
    if(dwAllowed & DROPEFFECT_MOVE)
      dwEffect = DROPEFFECT_MOVE;
  }

  (void)pt;
  return dwEffect;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_QueryInterface(PIUPWINIDROPTARGET pThis, REFIID riid, LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if(IsEqualGUID(riid, &IID_IUnknown))
  {
    IUPWINIDROPTARGET_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  else if(IsEqualGUID(riid, &IID_IDropTarget))
  {
    IUPWINIDROPTARGET_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }

  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IUPWINIDROPTARGET_AddRef(PIUPWINIDROPTARGET pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}

static ULONG STDMETHODCALLTYPE IUPWINIDROPTARGET_Release(PIUPWINIDROPTARGET pThis)
{
  LONG nCount = InterlockedDecrement(&pThis->lRefCount);
  if(nCount == 0)
  {
    HeapFree(GetProcessHeap(), 0, pThis);
    return 0;
  }
  return nCount;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragEnter(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  if(winQueryDataObject(pThis, pDataObject))
  {
    *pdwEffect = IUPWINIDROPTARGET_DropEffect(dwKeyState, pt, *pdwEffect);
    SetFocus(pThis->hWnd);  // TODO: let this here?
  }
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragOver(PIUPWINIDROPTARGET pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  pThis->dwKeyState = dwKeyState;  // TODO: why keystate from drag over and not from drop?
  *pdwEffect = IUPWINIDROPTARGET_DropEffect(dwKeyState, pt, *pdwEffect);
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragLeave(PIUPWINIDROPTARGET pThis)
{
  (void)pThis;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_Drop(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM medium;
  ULONG lFmt;

  /* Find the first matching CLIPFORMAT that I can handle. */
  for(lFmt = 0; lFmt < pThis->lNumFormats; lFmt++)
  {
    fmtetc.cfFormat = pThis->pFormat[lFmt];
    if(pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
      break;
  }

  /* If we found a matching format, then handle it now. */
  if(lFmt < pThis->lNumFormats)
  {
    /* Get the data being dragged. */
    pDataObject->lpVtbl->GetData(pDataObject, &fmtetc, &medium);
    *pdwEffect = DROPEFFECT_NONE;

    /* If a callback procedure is defined, then use that. */
    *pdwEffect = pThis->pDropProc(pThis->pFormat[lFmt], medium.hGlobal, pThis->hWnd, pThis->dwKeyState, pt, pThis->pUserData);

    /* Release the medium, if it was used. */
    if(*pdwEffect != DROPEFFECT_NONE)
      ReleaseStgMedium(&medium);
  }

  (void)dwKeyState;
  return S_OK;
}


/******************************************************************************************/


static DWORD winDropProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *ih)
{
  IFnsCiii cbDrop = (IFnsCiii)IupGetCallback((Ihandle*)ih, "DROPDATA_CB");
  if(cbDrop)
  {
    void* targetData = NULL;
    char type[256];
    SIZE_T size;
    DWORD dwEffect = 0;
    int x = (int)pt.x;
    int y = (int)pt.y;

    iupdrvScreenToClient(ih, &x, &y);

    if(dwKeyState & MK_SHIFT)
      dwEffect = DROPEFFECT_MOVE;
    else
      dwEffect = DROPEFFECT_COPY;

    targetData = GlobalLock(hData);
    size = GlobalSize(hData);
    if(size <= 0 || !targetData)
      return DROPEFFECT_NONE;

    GetClipboardFormatName(cf, type, 256);

    cbDrop(ih, type, targetData, size, x, y);

    GlobalUnlock(hData);

    return dwEffect;
  }

  (void)hWnd;
  return DROPEFFECT_NONE;
}

static void winRegisterDrop(Ihandle *ih)
{
  Iarray* dropList = (Iarray*)iupAttribGet(ih, "_IUPWIN_DROP_TYPES");
  int i, j, count = iupArrayCount(dropList);
  char** iupList = (char**)iupArrayGetData(dropList);
  CLIPFORMAT* cf = malloc(count * sizeof(CLIPFORMAT));

  j = 0;
  for(i = 0; i < count; i++)
  {
    CLIPFORMAT f = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]);
    if (f)
    {
      cf[j] = f;
      j++;
    }
  }

  if (j)
  {
    PIUPWINDROPTARGET pTarget = winRegisterDropTarget(ih->handle, cf, (ULONG)j, winDropProc, (void*)ih);
    iupAttribSetStr(ih, "_IUPWIN_DROPTARGET", (char*)pTarget);
  }

  free(cf);
}

static void winRegisterProcessDrag(Ihandle *ih)
{
  PIUPWINDROPSOURCE pSrc;
  char *dragList = iupAttribGet(ih, "_IUPWIN_DRAG_TYPES");
  int i, j, dragListCount;
  char **iupList;
  CLIPFORMAT *cf;
  HGLOBAL *hData;
  DWORD dwEffect = 0;
  IFns cbDragDataSize = (IFns)IupGetCallback(ih, "DRAGDATASIZE_CB");
  IFnsCi cbDragData = (IFnsCi)IupGetCallback(ih, "DRAGDATA_CB");
  IFni cbDragEnd;

  if(!dragList || !cbDragDataSize || !cbDragData)
    return;

  dragListCount = iupArrayCount((Iarray*)dragList);
  iupList = (char**)iupArrayGetData((Iarray*)dragList);

  cf = malloc(dragListCount * sizeof(CLIPFORMAT));
  hData = malloc(dragListCount * sizeof(HGLOBAL));

  /* Register all the drag types. */
  j = 0;
  for(i = 0; i < dragListCount; i++)
  {
    CLIPFORMAT f = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]);
    if (f)
    {
      void *sourceData;

      int size = cbDragDataSize(ih, iupList[i]);
      if (size <= 0)
        continue;

      cf[j] = f;
      hData[j] = GlobalAlloc(GMEM_FIXED, size);  // TODO GMEM_MOVEABLE ???
      sourceData = GlobalLock(hData[j]);

      /* fill data */
      cbDragData(ih, iupList[i], sourceData, size);

      GlobalUnlock(hData[j]);
      j++;
    }
  }

  pSrc = winCreateDropSourceData(cf, hData, (ULONG)j);

  /* Process drag, this will stop util drag is done or canceled. */
  DoDragDrop(pSrc->pObj, pSrc->pSrc, iupAttribGetBoolean(ih, "DRAGSOURCEMOVE") ? DROPEFFECT_MOVE|DROPEFFECT_COPY: DROPEFFECT_COPY, &dwEffect);

  /* Free the source for the dragging now, as well as the handles. */
  winFreeDropSource(pSrc);
  for(i = 0; i < j; i++)
    GlobalFree(hData[i]);

  cbDragEnd = (IFni)IupGetCallback(ih, "DRAGEND_CB");
  if(cbDragEnd)
  {
    int remove = -1;
    if (dwEffect == DROPEFFECT_MOVE)
      remove = 1;
    else if (dwEffect == DROPEFFECT_COPY)
      remove = 0;

    cbDragEnd(ih, remove);
  }

  free(cf);
  free(hData);
}

int iupwinDragStart(Ihandle* ih)
{
  POINT pt;
  GetCursorPos(&pt);

  if (DragDetect(ih->handle, pt))
  {
    IFnii cbDragBegin = (IFnii)IupGetCallback(ih, "DRAGBEGIN_CB");
    if(cbDragBegin)
    {
      int x = pt.x, y = pt.y;
      iupdrvScreenToClient(ih, &x, &y);
      if (cbDragBegin(ih, x, y) == IUP_IGNORE)
        return 0;

      winRegisterProcessDrag(ih);
      return 1;
    }
  }

  return 0;
}

static void winDestroyTypesList(Iarray *list)
{
  int i, count = iupArrayCount(list);
  char** listData = (char**)iupArrayGetData(list);
  for (i=0; i<count; i++)
    free(listData[i]);
  iupArrayDestroy(list);
}

static Iarray* winCreateTypesList(const char *value)
{
  Iarray *newList = iupArrayCreate(10, sizeof(char*));
  char** newListData;
  char valueCopy[256];
  char valueTemp[256];
  int i = 0;

  sprintf(valueCopy, "%s", value);
  while(iupStrToStrStr(valueCopy, valueTemp, valueCopy, ',') > 0)
  {
    newListData = (char**)iupArrayInc(newList);
    newListData[i] = iupStrDup(valueTemp);
    i++;

    if(iupStrEqualNoCase(valueCopy, valueTemp))
      break;
  }

  if (i == 0)
  {
    iupArrayDestroy(newList);
    return NULL;
  }

  return newList;
}

static int winSetDropTypesAttrib(Ihandle* ih, const char* value)
{
  Iarray* drop_types_list = (Iarray*)iupAttribGet(ih, "_IUPWIN_DROP_TYPES");
  if (drop_types_list)
  {
    winDestroyTypesList(drop_types_list);
    iupAttribSetStr(ih, "_IUPWIN_DROP_TYPES", NULL);
  }

  if(!value)
    return 0;

  drop_types_list = winCreateTypesList(value);
  if (drop_types_list)
    iupAttribSetStr(ih, "_IUPWIN_DROP_TYPES", (char*)drop_types_list);

  return 1;
}

static void winInitOle(void)
{
  if (!IupGetGlobal("_IUPWIN_OLEINITIALIZE"))
  {
    OleInitialize(NULL);
    IupSetGlobal("_IUPWIN_OLEINITIALIZE", "1");
  }
}

static int winSetDropTargetAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
    winInitOle();
    winRegisterDrop(ih);
  }
  else
  {
    PIUPWINDROPTARGET pTarget = (PIUPWINDROPTARGET)iupAttribGet(ih, "_IUPWIN_DROPTARGET");
    if (pTarget)
      winRevokeDropTarget(pTarget);
  }

  return 1;
}

static int winSetDragTypesAttrib(Ihandle* ih, const char* value)
{
  Iarray* drag_types_list = (Iarray*)iupAttribGet(ih, "_IUPWIN_DRAG_TYPES");
  if (drag_types_list)
  {
    winDestroyTypesList(drag_types_list);
    iupAttribSetStr(ih, "_IUPWIN_DRAG_TYPES", NULL);
  }

  if(!value)
    return 0;

  drag_types_list = winCreateTypesList(value);
  if (drag_types_list)
    iupAttribSetStr(ih, "_IUPWIN_DRAG_TYPES", (char*)drag_types_list);

  return 1;
}

static int winSetDragSourceAttrib(Ihandle* ih, const char* value)
{
  (void)ih;
  if (iupStrBoolean(value))
    winInitOle();
  return 1;
}


/******************************************************************************************/


static int winSetDropFilesTargetAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    DragAcceptFiles(ih->handle, TRUE);
  else
    DragAcceptFiles(ih->handle, FALSE);
  return 1;
}

void iupwinDropFiles(HDROP hDrop, Ihandle *ih)
{
  /* called for a WM_DROPFILES */
  char *filename;
  int i, numFiles, numchar, ret;
  POINT point;

  IFnsiii cb = (IFnsiii)IupGetCallback(ih, "DROPFILES_CB");
  if (!cb) return; 

  numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
  DragQueryPoint(hDrop, &point);  
  for (i = 0; i < numFiles; i++)
  {
    numchar = DragQueryFile(hDrop, i, NULL, 0);
    filename = (char*)malloc(numchar+1); 
    if (!filename)
      break;

    DragQueryFile(hDrop, i, filename, numchar+1);

    ret = cb(ih, filename, numFiles-i-1, (int) point.x, (int) point.y); 

    free(filename);

    if (ret == IUP_IGNORE)
      break;
  }
  DragFinish(hDrop);
}


/******************************************************************************************/


void iupdrvRegisterDragDropAttrib(Iclass* ic)
{
  iupClassRegisterCallback(ic, "DROPFILES_CB", "siii");

  iupClassRegisterCallback(ic, "DRAGBEGIN_CB", "ii");
  iupClassRegisterCallback(ic, "DRAGDATASIZE_CB", "s");
  iupClassRegisterCallback(ic, "DRAGDATA_CB",  "sCi");
  iupClassRegisterCallback(ic, "DRAGEND_CB",   "i");
  iupClassRegisterCallback(ic, "DROPDATA_CB",  "sCiii");

  iupClassRegisterAttribute(ic, "DRAGTYPES",  NULL, winSetDragTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPTYPES",  NULL, winSetDropTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGSOURCE", NULL, winSetDragSourceAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPTARGET", NULL, winSetDropTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGSOURCEMOVE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, winSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPFILESTARGET", NULL, winSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
}
