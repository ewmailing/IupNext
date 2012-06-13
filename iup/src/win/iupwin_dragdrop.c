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



typedef struct _IwinDropSource
{
  IDropSource ids;
  LONG lRefCount;
} IwinDropSource;

typedef struct _IwinDropSourceVtbl
{
  BEGIN_INTERFACE
  HRESULT (STDMETHODCALLTYPE *QueryInterface)(IwinDropSource* pThis, REFIID riid, void **ppvObject);
  ULONG   (STDMETHODCALLTYPE *AddRef)(IwinDropSource* pThis);
  ULONG   (STDMETHODCALLTYPE *Release)(IwinDropSource* pThis);
  HRESULT (STDMETHODCALLTYPE *QueryContinueDrag)(IwinDropSource* pThis, BOOL fEscapePressed, DWORD dwKeyState);
  HRESULT (STDMETHODCALLTYPE *GiveFeedback)(IwinDropSource* pThis, DWORD dwEffect);
  END_INTERFACE
} IwinDropSourceVtbl;

static HRESULT STDMETHODCALLTYPE IwinDropSource_QueryInterface(IwinDropSource* pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IwinDropSource_AddRef(IwinDropSource* pThis);
static ULONG   STDMETHODCALLTYPE IwinDropSource_Release(IwinDropSource* pThis);
static HRESULT STDMETHODCALLTYPE IwinDropSource_QueryContinueDrag(IwinDropSource* pThis, BOOL fEscapePressed, DWORD grfKeyState);
static HRESULT STDMETHODCALLTYPE IwinDropSource_GiveFeedback(IwinDropSource* pThis, DWORD dwEffect);

static IwinDropSource* winCreateDropSource(void)
{
  static IwinDropSourceVtbl ids_vtbl = {
    IwinDropSource_QueryInterface,
    IwinDropSource_AddRef,
    IwinDropSource_Release,
    IwinDropSource_QueryContinueDrag,
    IwinDropSource_GiveFeedback};

  IwinDropSource* pDropSource = malloc(sizeof(IwinDropSource));

  pDropSource->ids.lpVtbl = (IDropSourceVtbl*)&ids_vtbl;
  pDropSource->lRefCount = 1;

  return pDropSource;
}


/**********************************************************************************/


typedef struct _IwinDataObject
{
  IDataObject ido;
  LONG lRefCount;
  FORMATETC* pFormatEtc;
  HGLOBAL* phDataList;
  ULONG lNumFormats;
} IwinDataObject;

typedef struct _IwinDataObjectVtbl
{
  BEGIN_INTERFACE
  HRESULT (STDMETHODCALLTYPE *QueryInterface)(IwinDataObject* pThis, REFIID riid, void **ppvObject);
  ULONG   (STDMETHODCALLTYPE *AddRef)(IwinDataObject* pThis);
  ULONG   (STDMETHODCALLTYPE *Release)(IwinDataObject* pThis);
  HRESULT (STDMETHODCALLTYPE *GetData)(IwinDataObject* pThis, FORMATETC *pFormatEtcIn, STGMEDIUM *pMedium);
  HRESULT (STDMETHODCALLTYPE *GetDataHere)(IwinDataObject* pThis, FORMATETC *pFormatEtc, STGMEDIUM *pMedium);
  HRESULT (STDMETHODCALLTYPE *QueryGetData)(IwinDataObject* pThis, FORMATETC *pFormatEtc);
  HRESULT (STDMETHODCALLTYPE *GetCanonicalFormatEtc)(IwinDataObject* pThis, FORMATETC *pFormatEtcIn, FORMATETC *pFormatEtcOut);
  HRESULT (STDMETHODCALLTYPE *SetData)(IwinDataObject* pThis, FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease);
  HRESULT (STDMETHODCALLTYPE *EnumFormatEtc)(IwinDataObject* pThis, DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
  HRESULT (STDMETHODCALLTYPE *DAdvise)(IwinDataObject* pThis, FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
  HRESULT (STDMETHODCALLTYPE *DUnadvise)(IwinDataObject* pThis, DWORD dwConnection);
  HRESULT (STDMETHODCALLTYPE *EnumDAdvise)(IwinDataObject* pThis, IEnumSTATDATA **ppEnumAdvise);
  END_INTERFACE
} IwinDataObjectVtbl;

static HRESULT STDMETHODCALLTYPE IwinDataObject_QueryInterface(IwinDataObject* pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IwinDataObject_AddRef(IwinDataObject* pThis);
static ULONG   STDMETHODCALLTYPE IwinDataObject_Release(IwinDataObject* pThis);
static HRESULT STDMETHODCALLTYPE IwinDataObject_GetData(IwinDataObject* pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium);
static HRESULT STDMETHODCALLTYPE IwinDataObject_GetDataHere(IwinDataObject* pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium);
static HRESULT STDMETHODCALLTYPE IwinDataObject_QueryGetData(IwinDataObject* pThis, LPFORMATETC pFormatEtc);
static HRESULT STDMETHODCALLTYPE IwinDataObject_GetCanonicalFormatEtc(IwinDataObject* pThis, LPFORMATETC pFormatEtcIn, LPFORMATETC pFormatEtcOut);
static HRESULT STDMETHODCALLTYPE IwinDataObject_SetData(IwinDataObject* pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium, BOOL fRelease);
static HRESULT STDMETHODCALLTYPE IwinDataObject_EnumFormatEtc(IwinDataObject* pThis, DWORD dwDirection, LPENUMFORMATETC *ppEnumFormatEtc);
static HRESULT STDMETHODCALLTYPE IwinDataObject_DAdvise(IwinDataObject* pThis, LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection);
static HRESULT STDMETHODCALLTYPE IwinDataObject_Dunadvise(IwinDataObject* pThis, DWORD dwConnection);
static HRESULT STDMETHODCALLTYPE IwinDataObject_EnumDAdvise(IwinDataObject* pThis, LPENUMSTATDATA *ppEnumAdvise);

static IwinDataObject* winCreateDataObject(CLIPFORMAT *pFormatList, HGLOBAL *phDataList, int lNumFormats)
{
  IwinDataObject* pDataObject;
  int i, size1, size2, size3;
  static IwinDataObjectVtbl ido_vtbl = {
    IwinDataObject_QueryInterface,
    IwinDataObject_AddRef,
    IwinDataObject_Release,
    IwinDataObject_GetData,
    IwinDataObject_GetDataHere,
    IwinDataObject_QueryGetData,
    IwinDataObject_GetCanonicalFormatEtc,
    IwinDataObject_SetData,
    IwinDataObject_EnumFormatEtc,
    IwinDataObject_DAdvise,
    IwinDataObject_Dunadvise,
    IwinDataObject_EnumDAdvise};

  size1 = sizeof(IwinDataObject);
  size2 = lNumFormats*sizeof(FORMATETC);
  size3 = lNumFormats*sizeof(HGLOBAL);

  pDataObject = malloc(size1 + size2 + size3);
  pDataObject->pFormatEtc = (FORMATETC *)(((unsigned char*)pDataObject) + size1);
  pDataObject->phDataList = (HGLOBAL*)   (((unsigned char*)pDataObject) + size1 + size2);

  pDataObject->lRefCount  = 1;
  pDataObject->lNumFormats = lNumFormats;
  pDataObject->ido.lpVtbl = (IDataObjectVtbl*)&ido_vtbl;

  for(i = 0; i < lNumFormats; i++)
  {
    pDataObject->pFormatEtc[i].cfFormat = pFormatList[i];
    pDataObject->pFormatEtc[i].dwAspect = DVASPECT_CONTENT;
    pDataObject->pFormatEtc[i].ptd = NULL;
    pDataObject->pFormatEtc[i].lindex = -1;
    pDataObject->pFormatEtc[i].tymed = TYMED_HGLOBAL;
    pDataObject->phDataList[i] = phDataList[i];
  }

  return pDataObject;
}


/**********************************************************************************/


typedef void (*IwinDropCallback)(CLIPFORMAT cf, HGLOBAL hData, POINTL pt, void *pUserData);

typedef struct _IwinDropTarget
{
  IDropTarget idt;
  LONG lRefCount;
  ULONG lNumFormats;
  CLIPFORMAT *pFormatList;
  IDataObject *pDataObject;
  void *pUserData;
  IwinDropCallback pDropCallback;
} IwinDropTarget;

typedef struct _IwinDropTargetVtbl
{
  BEGIN_INTERFACE
  HRESULT (STDMETHODCALLTYPE *QueryInterface)(IwinDropTarget* pThis, REFIID riid, void **ppvObject);
  ULONG   (STDMETHODCALLTYPE *AddRef)(IwinDropTarget* pThis);
  ULONG   (STDMETHODCALLTYPE *Release)(IwinDropTarget* pThis);
  HRESULT (STDMETHODCALLTYPE *DragEnter)(IwinDropTarget* pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  HRESULT (STDMETHODCALLTYPE *DragOver)(IwinDropTarget* pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  HRESULT (STDMETHODCALLTYPE *DragLeave)(IwinDropTarget* pThis);
  HRESULT (STDMETHODCALLTYPE *Drop)(IwinDropTarget* pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  END_INTERFACE
} IwinDropTargetVtbl;

static HRESULT STDMETHODCALLTYPE IwinDropTarget_QueryInterface(IwinDropTarget* pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IwinDropTarget_AddRef(IwinDropTarget* pThis);
static ULONG   STDMETHODCALLTYPE IwinDropTarget_Release(IwinDropTarget* pThis);
static HRESULT STDMETHODCALLTYPE IwinDropTarget_DragEnter(IwinDropTarget* pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE IwinDropTarget_DragOver(IwinDropTarget* pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE IwinDropTarget_DragLeave(IwinDropTarget* pThis);
static HRESULT STDMETHODCALLTYPE IwinDropTarget_Drop(IwinDropTarget* pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);

static IwinDropTarget* winCreateDropTarget(CLIPFORMAT *pFormatList, int lNumFormats, IwinDropCallback pDropCallback, void *pUserData)
{
  int i;
  static IwinDropTargetVtbl idt_vtbl = {
    IwinDropTarget_QueryInterface,
    IwinDropTarget_AddRef,
    IwinDropTarget_Release,
    IwinDropTarget_DragEnter,
    IwinDropTarget_DragOver,
    IwinDropTarget_DragLeave,
    IwinDropTarget_Drop};

  IwinDropTarget* pDropTarget = malloc(sizeof(IwinDropTarget) + lNumFormats * sizeof(CLIPFORMAT));

  pDropTarget->pFormatList = (CLIPFORMAT *) (((unsigned char*) pDropTarget) + sizeof(IwinDropTarget));

  pDropTarget->idt.lpVtbl = (IDropTargetVtbl*)&idt_vtbl;
  pDropTarget->lRefCount = 1;
  pDropTarget->lNumFormats = (ULONG)lNumFormats;
  pDropTarget->pDropCallback = pDropCallback;
  pDropTarget->pUserData = pUserData;

  for(i = 0; i < lNumFormats; i++)
    pDropTarget->pFormatList[i] = pFormatList[i];

  return pDropTarget;
}


/**********************************************************************************/


typedef struct _IwinDropSourceData
{
  IDataObject *pObj;
  IDropSource *pSrc;
  HANDLE hData;
} IwinDropSourceData;

static IwinDropSourceData* winCreateDropSourceData(CLIPFORMAT *pFormatList, HGLOBAL *phDataList, int lNumFormats)
{
  IwinDropSourceData* pDropSourceData = malloc(sizeof(IwinDropSourceData));

  pDropSourceData->pSrc = (IDropSource*)winCreateDropSource();
  pDropSourceData->pObj = (IDataObject*)winCreateDataObject(pFormatList, phDataList, lNumFormats);
  pDropSourceData->hData = NULL;

  return pDropSourceData;
}

static void winDestroyDropSourceData(IwinDropSourceData* pDropSourceData)
{
  pDropSourceData->pSrc->lpVtbl->Release(pDropSourceData->pSrc);
  pDropSourceData->pObj->lpVtbl->Release(pDropSourceData->pObj);

  if (pDropSourceData->hData)
    GlobalFree(pDropSourceData->hData);

  free(pDropSourceData);
}


/**********************************************************************************/


static IwinDropTarget* winRegisterDropTarget(HWND hWnd, CLIPFORMAT *pFormatList, ULONG lNumFormats, IwinDropCallback pDropCallback, void *pUserData)
{
  IwinDropTarget* pTarget = winCreateDropTarget(pFormatList, lNumFormats, pDropCallback, pUserData);

  if (RegisterDragDrop(hWnd, (IDropTarget*)pTarget) != S_OK)
  {
    free(pTarget);
    return NULL;
  }

  return pTarget;
}

static void winRevokeDropTarget(IwinDropTarget* pTarget, HWND hWnd)
{
  RevokeDragDrop(hWnd);
  ((IDropTarget*)pTarget)->lpVtbl->Release((IDropTarget*)pTarget);
  free(pTarget);
}


/**********************************************************************************/


static int winFindFormatEtc(IwinDataObject* pThis, FORMATETC *pFormatEtc)
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

static HRESULT STDMETHODCALLTYPE IwinDropSource_QueryInterface(IwinDropSource* pThis, REFIID riid, LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if(IsEqualGUID(riid, &IID_IUnknown))
  {
    IwinDropSource_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  else if(IsEqualGUID(riid, &IID_IDropSource))
  {
    IwinDropSource_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }

  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IwinDropSource_AddRef(IwinDropSource* pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}

static ULONG STDMETHODCALLTYPE IwinDropSource_Release(IwinDropSource* pThis)
{
  LONG nCount = InterlockedDecrement(&pThis->lRefCount);
  if(nCount == 0)
  {
    free(pThis);
    return 0;
  }
  return nCount;
}

static HRESULT STDMETHODCALLTYPE IwinDropSource_QueryContinueDrag(IwinDropSource* pThis, BOOL fEscapePressed, DWORD dwKeyState)
{
  (void)pThis;
  if(fEscapePressed)
    return DRAGDROP_S_CANCEL;
  else if((dwKeyState & MK_LBUTTON) == 0)
    return DRAGDROP_S_DROP;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDropSource_GiveFeedback(IwinDropSource* pThis, DWORD dwEffect)
{
  (void)pThis;
  (void)dwEffect;
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_QueryInterface(IwinDataObject* pThis, REFIID riid, LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if(IsEqualGUID(riid, &IID_IUnknown))
  {
    IwinDataObject_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  else if(IsEqualGUID(riid, &IID_IDataObject))
  {
    IwinDataObject_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }

  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IwinDataObject_AddRef(IwinDataObject* pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}

static ULONG STDMETHODCALLTYPE IwinDataObject_Release(IwinDataObject* pThis)
{
  LONG nCount = InterlockedDecrement(&pThis->lRefCount);
  if(nCount == 0)
  {
    free(pThis);
    return 0;
  }
  return nCount;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_GetData(IwinDataObject* pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  SIZE_T sSize;
  void *pData;

  int nIndex = winFindFormatEtc(pThis, pFormatEtc);
  if(nIndex == -1)
    return DV_E_FORMATETC;

  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease  = 0;

  sSize = GlobalSize(pThis->phDataList[nIndex]);
  if(!sSize)
    return STG_E_MEDIUMFULL;

  pMedium->hGlobal = GlobalAlloc(GMEM_FIXED, sSize);
  if(!pMedium->hGlobal)
    return STG_E_MEDIUMFULL;

  pData = GlobalLock(pThis->phDataList[nIndex]);

  memcpy(pMedium->hGlobal, pData, sSize);

  GlobalUnlock(pThis->phDataList[nIndex]);

  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_GetDataHere(IwinDataObject* pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  (void)pThis;
  (void)pFormatEtc;
  (void)pMedium;
  return DATA_E_FORMATETC;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_QueryGetData(IwinDataObject* pThis, LPFORMATETC pFormatEtc)
{
  return winFindFormatEtc(pThis, pFormatEtc) == -1 ? DV_E_FORMATETC : S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_GetCanonicalFormatEtc(IwinDataObject* pThis, LPFORMATETC pFormatEtcIn, LPFORMATETC pFormatEtcOut)
{
  (void)pThis;
  (void)pFormatEtcIn;
  pFormatEtcOut->ptd = NULL;
  return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_SetData(IwinDataObject* pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium, BOOL fRelease)
{
  (void)pThis;
  (void)pFormatEtc;
  (void)pMedium;
  (void)fRelease;
  return E_UNEXPECTED;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_EnumFormatEtc(IwinDataObject* pThis, DWORD dwDirection, LPENUMFORMATETC *ppEnumFormatEtc)
{
  if(dwDirection != DATADIR_GET)
    return E_NOTIMPL;

  SHCreateStdEnumFmtEtc(pThis->lNumFormats, pThis->pFormatEtc, ppEnumFormatEtc);

  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_DAdvise(IwinDataObject* pThis, LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
  (void)pThis;
  (void)pFormatetc;
  (void)advf;
  (void)pAdvSink;
  (void)pdwConnection;
  return E_FAIL;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_Dunadvise(IwinDataObject* pThis, DWORD dwConnection)
{
  (void)pThis;
  (void)dwConnection;
  return E_FAIL;
}

static HRESULT STDMETHODCALLTYPE IwinDataObject_EnumDAdvise(IwinDataObject* pThis, LPENUMSTATDATA *ppEnumAdvise)
{
  (void)pThis;
  (void)ppEnumAdvise;
  return E_FAIL;
}

static BOOL winQueryDataObject(IwinDropTarget* pDropTarget, IDataObject *pDataObject)
{
  ULONG i;
  FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  
  for(i = 0; i < pDropTarget->lNumFormats; i++)
  {
    fmtetc.cfFormat = pDropTarget->pFormatList[i];
    if(pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
      return TRUE;
  }

  return FALSE;
}

static DWORD winGetDropEffect(DWORD dwKeyState, DWORD dwAllowed)
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

  return dwEffect;
}

static DWORD IwinDropTarget_DropEffect(DWORD dwKeyState, POINTL pt, DWORD dwAllowed)
{
  (void)pt;
  return winGetDropEffect(dwKeyState, dwAllowed);
}

static HRESULT STDMETHODCALLTYPE IwinDropTarget_QueryInterface(IwinDropTarget* pThis, REFIID riid, LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if(IsEqualGUID(riid, &IID_IUnknown))
  {
    IwinDropTarget_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  else if(IsEqualGUID(riid, &IID_IDropTarget))
  {
    IwinDropTarget_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }

  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IwinDropTarget_AddRef(IwinDropTarget* pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}

static ULONG STDMETHODCALLTYPE IwinDropTarget_Release(IwinDropTarget* pThis)
{
  LONG nCount = InterlockedDecrement(&pThis->lRefCount);
  if(nCount == 0)
  {
    free(pThis);
    return 0;
  }
  return nCount;
}

static HRESULT STDMETHODCALLTYPE IwinDropTarget_DragEnter(IwinDropTarget* pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  if (winQueryDataObject(pThis, pDataObject))
    *pdwEffect = IwinDropTarget_DropEffect(dwKeyState, pt, *pdwEffect);
  else
    *pdwEffect = DROPEFFECT_NONE;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDropTarget_DragOver(IwinDropTarget* pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  (void)pThis;
  *pdwEffect = IwinDropTarget_DropEffect(dwKeyState, pt, *pdwEffect);
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDropTarget_DragLeave(IwinDropTarget* pThis)
{
  (void)pThis;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE IwinDropTarget_Drop(IwinDropTarget* pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  ULONG i;

  /* If there are more than one format, 
     the first one accepted will be used */
  for(i = 0; i < pThis->lNumFormats; i++)
  {
    fmtetc.cfFormat = pThis->pFormatList[i];

    if (pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
    {
      STGMEDIUM medium;

      pDataObject->lpVtbl->GetData(pDataObject, &fmtetc, &medium);

      *pdwEffect = winGetDropEffect(dwKeyState, *pdwEffect);

      pThis->pDropCallback(fmtetc.cfFormat, medium.hGlobal, pt, pThis->pUserData);

      ReleaseStgMedium(&medium);

      return S_OK;
    }
  }

  return S_OK;
}


/******************************************************************************************/


static void winDropCallback(CLIPFORMAT cf, HGLOBAL hData, POINTL pt, Ihandle* ih)
{
  IFnsCiii cbDrop = (IFnsCiii)IupGetCallback((Ihandle*)ih, "DROPDATA_CB");
  if(cbDrop)
  {
    void* targetData = NULL;
    char type[256];
    SIZE_T size;
    int x = (int)pt.x;
    int y = (int)pt.y;

    iupdrvScreenToClient(ih, &x, &y);

    targetData = GlobalLock(hData);
    size = GlobalSize(hData);
    if(size <= 0 || !targetData)
      return;

    GetClipboardFormatName(cf, type, 256);

    cbDrop(ih, type, targetData, size, x, y);

    GlobalUnlock(hData);
  }
}

static void winRegisterDrop(Ihandle *ih)
{
  Iarray* dropList = (Iarray*)iupAttribGet(ih, "_IUPWIN_DROP_TYPES");
  int i, j, count = iupArrayCount(dropList);
  char** iupList = (char**)iupArrayGetData(dropList);
  CLIPFORMAT* cfList = malloc(count * sizeof(CLIPFORMAT));

  j = 0;
  for(i = 0; i < count; i++)
  {
    CLIPFORMAT f = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]);
    if (f)
    {
      cfList[j] = f;
      j++;
    }
  }

  if (j)
  {
    IwinDropTarget* pTarget = winRegisterDropTarget(ih->handle, cfList, (ULONG)j, winDropCallback, (void*)ih);
    iupAttribSetStr(ih, "_IUPWIN_DROPTARGET", (char*)pTarget);
  }

  free(cfList);
}

static void winRegisterProcessDrag(Ihandle *ih)
{
  IwinDropSourceData* pSrc;
  char *dragList = iupAttribGet(ih, "_IUPWIN_DRAG_TYPES");
  int i, j, dragListCount;
  char **iupList;
  CLIPFORMAT *cfList;
  HGLOBAL* hDataList;
  DWORD dwEffect = 0;
  IFns cbDragDataSize = (IFns)IupGetCallback(ih, "DRAGDATASIZE_CB");
  IFnsCi cbDragData = (IFnsCi)IupGetCallback(ih, "DRAGDATA_CB");
  IFni cbDragEnd;

  if(!dragList || !cbDragDataSize || !cbDragData)
    return;

  dragListCount = iupArrayCount((Iarray*)dragList);
  iupList = (char**)iupArrayGetData((Iarray*)dragList);

  cfList = malloc(dragListCount * sizeof(CLIPFORMAT));
  hDataList = malloc(dragListCount * sizeof(HGLOBAL));

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

      cfList[j] = f;
      hDataList[j] = GlobalAlloc(GMEM_FIXED, size);
      sourceData = GlobalLock(hDataList[j]);

      /* fill data */
      cbDragData(ih, iupList[i], sourceData, size);

      GlobalUnlock(hDataList[j]);
      j++;
    }
  }

  pSrc = winCreateDropSourceData(cfList, hDataList, j);

  /* Process drag, this will stop util drag is done or canceled. */
  DoDragDrop(pSrc->pObj, pSrc->pSrc, iupAttribGetBoolean(ih, "DRAGSOURCEMOVE") ? DROPEFFECT_MOVE|DROPEFFECT_COPY: DROPEFFECT_COPY, &dwEffect);

  winDestroyDropSourceData(pSrc);

  for(i = 0; i < j; i++)
    GlobalFree(hDataList[i]);
  free(hDataList);

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

  free(cfList);
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
  Iarray* dropList = (Iarray*)iupAttribGet(ih, "_IUPWIN_DROP_TYPES");
  if (!dropList)
    return 0;

  if (iupStrBoolean(value))
  {
    winInitOle();
    winRegisterDrop(ih);
  }
  else
  {
    IwinDropTarget* pTarget = (IwinDropTarget*)iupAttribGet(ih, "_IUPWIN_DROPTARGET");
    if (pTarget)
      winRevokeDropTarget(pTarget, ih->handle);
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
