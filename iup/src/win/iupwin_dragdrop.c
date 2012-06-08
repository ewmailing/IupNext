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

/*
 * File: DragAndDrop.c
 * A layer for implementation of the Drag and Drop in Windows for C-applications. This enables
 * C applications to easily implement drag and drop, without having to deal with any OLE or
 * something like that. Some shortcuts has been taken, such as only one TYMED is supported (HGLOBAL)
 * and only the DVASPECT_CONTENT aspect.
 * Both drag-drop target and source is supported.
 */

typedef struct tagIUPWINDROPDATA
{
  CLIPFORMAT cf;
  POINTL pt;
  DWORD dwKeyState;
  HGLOBAL hData;
} IUPWINDROPDATA, *PIUPWINDROPDATA;

typedef struct tagIUPWINDROPSOURCE *PIUPWINDROPSOURCE;
typedef struct tagIUPWINDROPTARGET *PIUPWINDROPTARGET;
typedef DWORD (*IUPWINDDCALLBACK)(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);

/* Public function prototypes. */
void winDragDropInit(HANDLE hHeap);
PIUPWINDROPSOURCE winCreateIupDropSource(BOOL bRightClick, CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt);
PIUPWINDROPSOURCE winCreateIupDropSourceText(BOOL bRightClick, LPCTSTR pText);
DWORD winDragDropSourceEx(PIUPWINDROPSOURCE pDropSrc, DWORD dwCopyMove, DWORD *pdwEffect);
DWORD winDragDropSource(PIUPWINDROPSOURCE pDropSrc);
DWORD winDragDropText(LPCTSTR pText);
PIUPWINDROPSOURCE winFreeDropSource(PIUPWINDROPSOURCE pDropSrc);
PIUPWINDROPTARGET winRegisterDragDrop(HWND hWnd, CLIPFORMAT *pFormat, ULONG lFmt, UINT nMsg, IUPWINDDCALLBACK, void *pUserData);
PIUPWINDROPTARGET winRevokeDragDrop(PIUPWINDROPTARGET pTarget);
IDataObject *winCreateDataObject(CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt);
IDropSource *winCreateDropSource(BOOL bRightClick);
IDropTarget *winCreateDropTarget(CLIPFORMAT *pFormat, ULONG lFmt, HWND hWnd, UINT nMsg, IUPWINDDCALLBACK pDropProc, void *pUserData);

/* Macros. */
#define IUPWINDD_HEAP (g_hHeap == NULL ? (g_hHeap = GetProcessHeap()) : g_hHeap)

/* Typedefs. */
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
	BOOL bRightClick;
	HWND hWndMenu;
	HMENU hPopup;
} IUPWINIDROPSOURCE, *PIUPWINIDROPSOURCE;

typedef struct tagIUPWINIDROPTARGET
{
	IDropTarget idt;
	LONG lRefCount;
	ULONG lNumFormats;
	CLIPFORMAT *pFormat;
	HWND hWnd;
	BOOL bAllowDrop;
	DWORD dwKeyState;
	IDataObject *pDataObject;
	UINT nMsg;
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

/* Static globals. */
static HANDLE g_hHeap = NULL;

/* Static helper function prototypes. */
static int LookupFormatEtc(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc);

/* IUPWINIDROPSOURCE functions. */
static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryInterface(PIUPWINIDROPSOURCE pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPSOURCE_AddRef(PIUPWINIDROPSOURCE pThis);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPSOURCE_Release(PIUPWINIDROPSOURCE pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryContinueDrag(PIUPWINIDROPSOURCE pThis, BOOL fEscapePressed, DWORD grfKeyState);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_GiveFeedback(PIUPWINIDROPSOURCE pThis, DWORD dwEffect);

/* IUPWINIDATAOBJECT functions. */
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

/* IUPWINIDROPTARGET functions. */
static BOOL    IUPWINIDROPTARGET_QueryDataObject(PIUPWINIDROPTARGET pDropTarget, IDataObject *pDataObject);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_QueryInterface(PIUPWINIDROPTARGET pThis, REFIID riid, LPVOID *ppvObject);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPTARGET_AddRef(PIUPWINIDROPTARGET pThis);
static ULONG   STDMETHODCALLTYPE IUPWINIDROPTARGET_Release(PIUPWINIDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragEnter(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragOver(PIUPWINIDROPTARGET pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragLeave(PIUPWINIDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_Drop(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);

/* Public functions. */
/*
 * Function: winDragDropInit()
 * Initialize the DragDrop library and OLE itself.
 * Arguments:
 * HANDLE hHeap - Handle to the heap memory handle to use for winDragDropInit().
 */
void winDragDropInit(HANDLE hHeap)
{
  /* Only initialize this if not already set or used before. */
	if(g_hHeap == NULL && hHeap == NULL)
		g_hHeap = GetProcessHeap();
	else if(g_hHeap == NULL)
		g_hHeap = hHeap;

  /* Initialize OLE, to be sure. */
	OleInitialize(NULL);

	return;
} /* End of winDragDropInit(). */


/*
 * Function: winCreateIupDropSource()
 * Create a Drop source and Data object.
 * Arguments:
 * BOOL bRightClick - Flag if the right mouse button was used to drag.
 * CLIPFORMAT *pFormat - An array of CLIPFORMATs being dragged.
 * HGLOBAL *phData - An array of handles, corresponding to the formats above.
 * ULONG lFmt - The number of formats / handles in the arrays above.
 * Returns:
 * PMYDRAGSOURCE - A pointer to the controlling struct, NULL if there is an error.
 */
PIUPWINDROPSOURCE winCreateIupDropSource(BOOL bRightClick, CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt)
{
	PIUPWINDROPSOURCE pRet;

	if((pRet = HeapAlloc(IUPWINDD_HEAP, 0, sizeof(IUPWINDROPSOURCE))) == NULL)
		return NULL;

/* Create the two members. */
	if((pRet->pSrc = winCreateDropSource(bRightClick)) == NULL)
	{
		HeapFree(IUPWINDD_HEAP, 0, pRet);
		return NULL;
	}
	if((pRet->pObj = winCreateDataObject(pFormat, phData, lFmt)) == NULL)
	{
		pRet->pSrc->lpVtbl->Release(pRet->pSrc);
		HeapFree(IUPWINDD_HEAP, 0, pRet);
		return NULL;
	}
	pRet->hData = NULL;

	return pRet;
} /* End of winCreateIupDropSource(). */

/*
 * Function: winCreateIupDropSourceText()
 * Create a Drop source and Data object, a simplified version of
 * winCreateIupDropSource that only allows one format and object, a text string.
 * Arguments:
 * BOOL bRightClick - Flag if the right mouse button was used to drag.
 * LPCTSTR pText - The text to drop.
 * Returns:
 * PMYDRAGSOURCE - A pointer to the controlling struct, NULL if there is an error.
 */
PIUPWINDROPSOURCE winCreateIupDropSourceText(BOOL bRightClick, LPCTSTR pText)
{
	PIUPWINDROPSOURCE pRet;
	CLIPFORMAT cf = CF_TEXT;
	HANDLE hText;

  /* Allocate a handle for text. */
	if((hText = GlobalAlloc(GMEM_FIXED, _tcslen(pText) + sizeof(TCHAR))) == NULL)
		return NULL;
	_tcscpy(hText, pText);

  /* Create the source itself. */
	if((pRet = winCreateIupDropSource(bRightClick, &cf, &hText, 1)) == NULL)
	{
		GlobalFree(hText);
		return NULL;
	}

  /* Set the text. */
	pRet->hData = hText;
	
	return pRet;
} /* End of winCreateIupDropSourceText(). */

/*
 * Function: winDragDropSourceEx()
 * Start dragging the source.
 * Arguments:
 * PIUPWINDROPSOURCE pDropSrc - The source to drag.
 * DWORD dwOKEffect - Allowed effects to drop.
 * DWORD *pdwEffect - Apointer to a DWORD to receive the effect that was used.
 * Returns:
 * DWORD - The result from the DoDragDrop() operation.
 */
DWORD winDragDropSourceEx(PIUPWINDROPSOURCE pDropSrc, DWORD dwOKEffect, DWORD *pdwEffect)
{
	return DoDragDrop(pDropSrc->pObj, pDropSrc->pSrc, dwOKEffect, pdwEffect);
} /* End of winDragDropSourceEx(). */

/*
 * Function: winDragDropSource()
 * Start dragging the source, a simplified version of DragDropSrcEx(), which
 * assumed the COPY drop effect only and which does not return the used effect.
 * Arguments:
 * PIUPWINDROPSOURCE pDropSrc - The source to drag.
 * Returns:
 * DWORD - The result from the DoDragDrop() operation.
 */
DWORD winDragDropSource(PIUPWINDROPSOURCE pDropSrc)
{
	DWORD dwEffect;

	return winDragDropSourceEx(pDropSrc, DROPEFFECT_COPY, &dwEffect);
} /* End of winDragDropSource(). */

/*
 * Function: winDragDropText()
 * Simplified function for dragging text only.
 * Arguments:
 * LPCTSTR pText - The text to drag.
 * Returns:
 * DWORD - The result from DoDragDrop().
 */
DWORD winDragDropText(LPCTSTR pText)
{
	PIUPWINDROPSOURCE pSrc;
	DWORD dwRet;

  /* Create the stuff to drag. */
	if((pSrc = winCreateIupDropSourceText(FALSE, pText)) == NULL)
		return DROPEFFECT_NONE;

  /* Start dragging it. */
	dwRet = winDragDropSource(pSrc);

  /* Before returning, free my temporary object. */
	winFreeDropSource(pSrc);

  /* And now return what I got back. */
	return dwRet;
} /* End of winDragDropText(). */

/*
 * Function: winFreeDropSource()
 * Free the drop source.
 * Arguments:
 * PIUPWINDROPSOURCE pDropSrc - The source to free.
 * Returns:
 * PIUPWINDROPSOURCE - NULL.
 */
PIUPWINDROPSOURCE winFreeDropSource(PIUPWINDROPSOURCE pDropSrc)
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
	HeapFree(IUPWINDD_HEAP, 0, pDropSrc);

	return NULL;
} /* End of winFreeDropSource(). */

/*
 * Function: winRegisterDragDrop()
 * Simplified registration for a drop target.
 * Arguments:
 * HWND hWnd - The window where drops occur and messages are sent etc.
 * CLIPFORMAT *pFormat - An array of CLIPFORMATs supported.
 * ULONG lFmt - The count of CLIPFORMATS in the pFormat array.
 * HWND hWnd - The window that is the parent of this target.
 * UINT nMsg - The message to send to hWnd when data is dropped. Set to WM_NULL if we are not
 *   to send any messages.
 * IUPWINDDCALLBACK pDropProc - The callback to call when stuff is dropped. Set to NULL
 *   to send nMsg instead of doing the callback.
 * void *pUserData - A user defined data pointer, that is passed back in the
 *   callback or message, whichever method is used.
 * Returns:
 * PIUPWINDROPTARGET - The target object, NULL if there was an error.
 */
PIUPWINDROPTARGET winRegisterDragDrop(HWND hWnd, CLIPFORMAT *pFormat, ULONG lFmt, UINT nMsg, IUPWINDDCALLBACK pDropProc, void *pUserData)
{
	IDropTarget *pTarget;

  /* First, create the target. */
	if((pTarget = winCreateDropTarget(pFormat, lFmt, hWnd, nMsg, pDropProc, pUserData)) == NULL)
		return NULL;

  /* Now, register for drop. If this fails, free my target the old-fashioned way, as none knows about it anyway. */
	if(RegisterDragDrop(hWnd, pTarget) != S_OK)
	{
		HeapFree(IUPWINDD_HEAP, 0, pTarget);
		return NULL;
	}

	return (PIUPWINDROPTARGET) pTarget;
} /* End of winRegisterDragDrop(). */

/*
 * Function: winRevokeDragDrop()
 * Convenience function to revoke drag drop and also free the associated IDropTarget().
 * Arguments:
 * PIUPWINDROPTARGET - The target to free and revoke drop from.
 * Returns:
 * PIUPWINDROPTARGET - NULL
 */
PIUPWINDROPTARGET winRevokeDragDrop(PIUPWINDROPTARGET pTarget)
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
} /* End of winRevokeDragDrop(). */

/*
 * Function: winCreateDropSource()
 * Create an IDropSource object.
 * Arguments:
 * BOOL bRightClick - Flag if right button is clicked when object is created.
 * Returns:
 * IDropSource * - A pointer to an IDropSource object, NULL if there is an error.
 */
IDropSource *winCreateDropSource(BOOL bRightClick)
{
	PIUPWINIDROPSOURCE pRet;
	static IUPWINIDROPSOURCE_VTBL ids_vtbl = {
	  IUPWINIDROPSOURCE_QueryInterface,
	  IUPWINIDROPSOURCE_AddRef,
	  IUPWINIDROPSOURCE_Release,
	  IUPWINIDROPSOURCE_QueryContinueDrag,
	  IUPWINIDROPSOURCE_GiveFeedback};

	if((pRet = HeapAlloc(IUPWINDD_HEAP, 0, sizeof(IUPWINIDROPSOURCE))) == NULL)
		return NULL;

  pRet->ids.lpVtbl = (IDropSourceVtbl*)&ids_vtbl;
	/* IUP // (PIUPWINIDROPSOURCE_VTBL)pRet->ids.lpVtbl = &ids_vtbl; */
	pRet->lRefCount = 1;
	pRet->bRightClick = bRightClick;

	return (IDropSource *) pRet;
} /* End of winCreateDropSource(). */

/*
 * Function: winCreateDataObject()
 * Create an IDataObject with the specified formats. Media is always TYMED_HGLOBAL.
 * Arguments:
 * CLIPFORMAT *pFormat - An array of clipboard formats to support.
 * HGLOBAL *phData - The data handles for the different formats.
 * ULONG lFmt - The size of the two arrays above, i.e. # of formats.
 * Returns:
 * IDataObject * - The pointer to the created IDataObject, NULL if there is an error.
 */
IDataObject *winCreateDataObject(CLIPFORMAT *pFormat, HGLOBAL *phData, ULONG lFmt)
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

  /* Allocate IUPWINIDATAOBJECT and set pointers. */
	if((pRet = HeapAlloc(IUPWINDD_HEAP, 0, sizeof(IUPWINIDATAOBJECT)
	   + lFmt * (sizeof(FORMATETC) + sizeof(HGLOBAL)))) == NULL)
		return NULL;
	pRet->pFormatEtc = (FORMATETC *) (((char *) pRet) + sizeof(IUPWINIDATAOBJECT));
	pRet->phData = (HGLOBAL *) (((char *) pRet->pFormatEtc) + lFmt * sizeof(FORMATETC));

  /* Initialize data object. */
	pRet->lRefCount  = 1;
	pRet->lNumFormats = lFmt;
  pRet->ido.lpVtbl = (IDataObjectVtbl*)&ido_vtbl;
  /* IUP // (PIUPWINIDATAOBJECT_VTBL) pRet->ido.lpVtbl = &ido_vtbl; */

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
} /* End of winCreateDataObject(). */

/*
 * Function: winCreateDropTarget()
 * Create a IDropTarget, that has some added private members, used only here. When
 * data is dropped, either pDropProc is called, or if this is NULL, then nMsg is
 * sent to hWnd instead. When a message is sent, a pointer to a IUPWINDROPDATA
 * is passed a wParam, and the user defined data pointer, pUserData, is
 * passed as lParam.
 * Arguments:
 * CLIPFORMAT *pFormat - An array of CLIPFORMATs supported.
 * ULONG lFmt - The count of CLIPFORMATS in the pFormat array.
 * HWND hWnd - The window that is the parent of this target.
 * UINT nMsg - The message to send to hWnd when data is dropped. Set to WM_NULL if we are not
 *   to send any messages.
 * DWORD (*pDropProc)() - The callback to call when stuff is dropped. Set to NULL
 *   to send nMsg instead of doing the callback.
 * void *pUserData - A user defined data pointer, that is passed back in the
 *   callback or message, whichever method is used.
 * Returns:
 * IDropTarget * - A pointert to a created IDropTarget, NULL if there is an error.
 */
IDropTarget *winCreateDropTarget(CLIPFORMAT *pFormat, ULONG lFmt, HWND hWnd, UINT nMsg,
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

  /* Allocate the nasty little thing, and supply space for CLIPFORMAT array in the process. */
	if((pRet = HeapAlloc(IUPWINDD_HEAP, 0, sizeof(IUPWINIDROPTARGET) + lFmt * sizeof(CLIPFORMAT))) == NULL)
		return NULL;
	pRet->pFormat = (CLIPFORMAT *) (((char *) pRet) + sizeof(IUPWINIDROPTARGET));

  /* Set up the struct members. */
  pRet->idt.lpVtbl = (IDropTargetVtbl*)&idt_vtbl;
  /* IUP // (PIUPWINIDROPTARGET_VTBL) pRet->idt.lpVtbl = &idt_vtbl; */
	pRet->lRefCount = 1;
	pRet->hWnd = hWnd;
	pRet->nMsg = nMsg;
  pRet->bAllowDrop = TRUE;
	/* IUP // pRet->bAllowDrop = FALSE; */
	pRet->dwKeyState = 0;
	pRet->lNumFormats = lFmt;
	pRet->pDropProc = pDropProc;
	pRet->pUserData = pUserData;

  /* Set the format members. */
	for(lFmt = 0; lFmt < pRet->lNumFormats; lFmt++)
		pRet->pFormat[lFmt] = pFormat[lFmt];

	return (IDropTarget *) pRet;
} /* End of winCreateDropTarget(). */

/* Static helper functions. */
/*
 * Function: LookupFormatEtc()
 * Match a format in a PIUPWINIDATAOBJECT object.
 * Arguments:
 * PIUPWINIDATAOBJECT pThis - The object to look for a match in.
 * FORMATETC *pFormat - The format to match with.
 * Returns:
 * int - The index of the formatetc in pThis, and -1 if no match was found.
 */
static int LookupFormatEtc(PIUPWINIDATAOBJECT pThis, FORMATETC *pFormatEtc)
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
} /* End of LookupFormatEtc(). */

/*
 * IUPWINIDROPSOURCE functions.
 * These functions are just implementations of the standard IDropSource methods.
 */
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
} /* End of IUPWINIDROPSOURCE_QueryInterface(). */

static ULONG STDMETHODCALLTYPE IUPWINIDROPSOURCE_AddRef(PIUPWINIDROPSOURCE pThis)
{
	return InterlockedIncrement(&pThis->lRefCount);
} /* End of IUPWINIDROPSOURCE_AddRef(). */

static ULONG STDMETHODCALLTYPE IUPWINIDROPSOURCE_Release(PIUPWINIDROPSOURCE pThis)
{
	LONG nCount;
	
	if((nCount = InterlockedDecrement(&pThis->lRefCount)) == 0)
	{
		HeapFree(IUPWINDD_HEAP, 0, pThis);
		return 0;
	}
	return nCount;
} /* End of IUPWINIDROPSOURCE_Release(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_QueryContinueDrag(PIUPWINIDROPSOURCE pThis, BOOL fEscapePressed, DWORD dwKeyState)
{
	if(fEscapePressed)
		return DRAGDROP_S_CANCEL;
	else if((dwKeyState & MK_LBUTTON) == 0 && !pThis->bRightClick)
		return DRAGDROP_S_DROP;
	else if((dwKeyState & MK_RBUTTON) == 0 && pThis->bRightClick)
		return DRAGDROP_S_DROP;

	return S_OK;
} /* End of IUPWINIDROPSOURCE_QueryContinueDrag(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDROPSOURCE_GiveFeedback(PIUPWINIDROPSOURCE pThis, DWORD dwEffect)
{
  (void)pThis;
  (void)dwEffect;
	return DRAGDROP_S_USEDEFAULTCURSORS;
} /* End of IUPWINIDROPSOURCE_GiveFeedback(). */

/*
 * IUPWINIDATAOBJECT functions.
 * These functions are just implementations of the standard IDataObject methods.
 */
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
} /* End of IUPWINIDATAOBJECT_QDragDrop Custom textueryInterface(). */

static ULONG STDMETHODCALLTYPE IUPWINIDATAOBJECT_AddRef(PIUPWINIDATAOBJECT pThis)
{
	return InterlockedIncrement(&pThis->lRefCount);
} /* End of IUPWINIDATAOBJECT_AddRef(). */

static ULONG STDMETHODCALLTYPE IUPWINIDATAOBJECT_Release(PIUPWINIDATAOBJECT pThis)
{
	LONG nCount;

	if((nCount = InterlockedDecrement(&pThis->lRefCount)) == 0)
	{
		HeapFree(IUPWINDD_HEAP, 0, pThis);
		return 0;
	}
	return nCount;
} /* End of IUPWINIDATAOBJECT_Release(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
	int nIndex;
	size_t nLen;
	void *pData;

  /* Try to match the requested FORMATETC with one of our supported formats */
	if((nIndex = LookupFormatEtc(pThis, pFormatEtc)) == -1)
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
} /* End of IUPWINIDATAOBJECT_GetData(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetDataHere(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  (void)pThis;
  (void)pFormatEtc;
  (void)pMedium;
	return DATA_E_FORMATETC;
} /* End of IUPWINIDATAOBJECT_GetDataHere(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_QueryGetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc)
{
	return LookupFormatEtc(pThis, pFormatEtc) == -1 ? DV_E_FORMATETC : S_OK;
} /* End of IUPWINIDATAOBJECT_QueryGetData(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_GetCanonicalFormatEtc(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtcIn, LPFORMATETC pFormatEtcOut)
{
  (void)pThis;
  (void)pFormatEtcIn;
	pFormatEtcOut->ptd = NULL;
	return E_NOTIMPL;
} /* End of IUPWINIDATAOBJECT_GetCanonicalFormatEtc(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_SetData(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium, BOOL fRelease)
{
  (void)pThis;
  (void)pFormatEtc;
  (void)pMedium;
  (void)fRelease;
	return E_UNEXPECTED;
} /* End of IUPWINIDATAOBJECT_SetData(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_EnumFormatEtc(PIUPWINIDATAOBJECT pThis, DWORD dwDirection, LPENUMFORMATETC *ppEnumFormatEtc)
{
	if(dwDirection != DATADIR_GET)
		return E_NOTIMPL;

  /* Note that this is for W2K and up only. Before this, this will NOT work! */
	SHCreateStdEnumFmtEtc(pThis->lNumFormats, pThis->pFormatEtc, ppEnumFormatEtc);

	return S_OK;
} /* End of IUPWINIDATAOBJECT_EnumFormatEtc(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_DAdvise(PIUPWINIDATAOBJECT pThis, LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
  (void)pThis;
  (void)pFormatetc;
  (void)advf;
  (void)pAdvSink;
  (void)pdwConnection;
	return E_FAIL;
} /* End of IUPWINIDATAOBJECT_DAdvise(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_Dunadvise(PIUPWINIDATAOBJECT pThis, DWORD dwConnection)
{
  (void)pThis;
  (void)dwConnection;
	return E_FAIL;
} /* End of IUPWINIDATAOBJECT_Dunadvise(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDATAOBJECT_EnumDAdvise(PIUPWINIDATAOBJECT pThis, LPENUMSTATDATA *ppEnumAdvise)
{
  (void)pThis;
  (void)ppEnumAdvise;
	return E_FAIL;
} /* End of IUPWINIDATAOBJECT_EnumDAdvise(). */

/*
 * IUPWINIDROPTARGET functions.
 * These functions are just implementations of the standard IDropTarget methods.
 */
static BOOL IUPWINIDROPTARGET_QueryDataObject(PIUPWINIDROPTARGET pDropTarget, IDataObject *pDataObject)
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
} /* End of IUPWINIDROPTARGET_QueryDataObject(). */

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

/* IDropTarget private methods. */
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
} /* End of IUPWINIDROPTARGET_QueryInterface(). */

static ULONG STDMETHODCALLTYPE IUPWINIDROPTARGET_AddRef(PIUPWINIDROPTARGET pThis)
{
	return InterlockedIncrement(&pThis->lRefCount);
} /* End of IUPWINIDROPTARGET_AddRef(). */

static ULONG STDMETHODCALLTYPE IUPWINIDROPTARGET_Release(PIUPWINIDROPTARGET pThis)
{
	LONG nCount;

	if((nCount = InterlockedDecrement(&pThis->lRefCount)) == 0)
	{
		HeapFree(IUPWINDD_HEAP, 0, pThis);
		return 0;
	}

	return nCount;
} /* End of IUPWINIDROPTARGET_Release(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragEnter(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(pThis->bAllowDrop == IUPWINIDROPTARGET_QueryDataObject(pThis, pDataObject))
	{
		*pdwEffect = IUPWINIDROPTARGET_DropEffect(dwKeyState, pt, *pdwEffect);
		SetFocus(pThis->hWnd);
	}
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
} /* End of IUPWINIDROPTARGET_DragEnter(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragOver(PIUPWINIDROPTARGET pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(pThis->bAllowDrop)
	{
		pThis->dwKeyState = dwKeyState;

		*pdwEffect = IUPWINIDROPTARGET_DropEffect(dwKeyState, pt, *pdwEffect);
	}
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
} /* End of IUPWINIDROPTARGET_DragEffect(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_DragLeave(PIUPWINIDROPTARGET pThis)
{
  (void)pThis;
	return S_OK;
} /* End of IUPWINIDROPTARGET_DragLeave(). */

static HRESULT STDMETHODCALLTYPE IUPWINIDROPTARGET_Drop(PIUPWINIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM medium;
	ULONG lFmt;
	IUPWINDROPDATA DropData;

	if(pThis->bAllowDrop)
	{
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
			if(pThis->pDropProc != NULL)
				*pdwEffect = (*pThis->pDropProc)(pThis->pFormat[lFmt], medium.hGlobal, pThis->hWnd, pThis->dwKeyState, pt, pThis->pUserData);
      /* Else, if a message is valid, then send that. */
			else if(pThis->nMsg != WM_NULL)
			{
        /* Fill the struct with the relevant data. */
				DropData.cf = pThis->pFormat[lFmt];
				DropData.dwKeyState = pThis->dwKeyState;
				DropData.hData = medium.hGlobal;
				DropData.pt = pt;

        /* And send the message. */
				*pdwEffect = (DWORD) SendMessage(pThis->hWnd, pThis->nMsg, (WPARAM) &DropData, (LPARAM) pThis->pUserData);
			}
      /* Release the medium, if it was used. */
			if(*pdwEffect != DROPEFFECT_NONE)
				ReleaseStgMedium(&medium);
		}
	}
	else
		*pdwEffect = DROPEFFECT_NONE;

  (void)dwKeyState;
	return S_OK;
} /* End of IUPWINIDROPTARGET_Drop(). */


/*////////////////////////////////////////////////////////////////*/
/* IUP - Functions to access the Windows Drag and Drop procedures */
/*////////////////////////////////////////////////////////////////*/

static DWORD winDropProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *ih)
{
  IFnsCiii cbDrop = (IFnsCiii)IupGetCallback((Ihandle*)ih, "DROPDATA_CB");
  void* targetData = NULL;
  char* type = iupStrGetMemory(256);
  int size, res;
  DWORD dwEffect = 0;

  targetData = GlobalLock(hData);
  GetClipboardFormatName(cf, type, 256);
  size = (int)sizeof(targetData);

  if(dwKeyState & MK_SHIFT)
    dwEffect = DROPEFFECT_MOVE;
  else
    dwEffect = DROPEFFECT_COPY;

  if(size <= 0)
    dwEffect = DROPEFFECT_NONE;

  if(cbDrop)
  {
    int x = (int)pt.x;
    int y = (int)pt.y;
    iupdrvScreenToClient(ih, &x, &y);
    res = cbDrop(ih, type, targetData, size, x, y);
  }

  GlobalUnlock(hData);
  (void)hWnd;
  return dwEffect;
}

void winRegisterDrop(Ihandle *ih)
{
  static PIUPWINDROPTARGET pTarget = NULL;
  char *dropList = iupAttribGet(ih, "_IUP_DROP_TYPES");
  int i, count = iupArrayCount((Iarray*)dropList);
  char** iupList = (char**)iupArrayGetData((Iarray*)dropList);
  CLIPFORMAT* cf = malloc(count * sizeof(CLIPFORMAT));

  for(i = 0; i < count; i++)
    cf[i] = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]);

  pTarget = winRegisterDragDrop(ih->handle, cf, (ULONG)count, WM_NULL, winDropProc, (void*)ih);
}

void winRegisterDrag(Ihandle *ih)
{
  PIUPWINDROPSOURCE pSrc;
  char *dragList = iupAttribGet(ih, "_IUP_DRAG_TYPES");
  int i, size, len, count = 0;
  char **iupList;
  CLIPFORMAT *cf;
  HGLOBAL *hData;
  DWORD dwEffect;
  void *sourceData;
  IFns cbDragDataSize = (IFns)IupGetCallback(ih, "DRAGDATASIZE_CB");
  IFnsCi cbDragData = (IFnsCi)IupGetCallback(ih, "DRAGDATA_CB");
  IFni cbDragEnd = (IFni)IupGetCallback(ih, "DRAGEND_CB");

  if(!dragList)
    return;

  len = iupArrayCount((Iarray*)dragList);
  iupList = (char**)iupArrayGetData((Iarray*)dragList);

  cf = malloc(count * sizeof(CLIPFORMAT));
  hData = malloc(count * sizeof(HGLOBAL));

  /* Register all the drag types. */
  for(i = 0; i < len; i++)
  {
    size = cbDragDataSize(ih, iupList[i]);
    if (size <= 0)
      continue;

    sourceData = malloc(size + 1);
    ((char*)sourceData)[size] = 0;

    /* fill data */
    cbDragData(ih, iupList[i], sourceData, size);

    cf[count] = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]);
    hData[count] = GlobalAlloc(GMEM_FIXED, size);
    memcpy(hData[count], sourceData, size);

    count++;
    free(sourceData);
  }

  pSrc = winCreateIupDropSource(FALSE, cf, hData, (ULONG)count);

  /* Start dragging now. */
  winDragDropSourceEx(pSrc, iupAttribGetBoolean(ih, "DRAGSOURCEMOVE") ? DROPEFFECT_MOVE : DROPEFFECT_COPY, &dwEffect);

  /* Free the source for the dragging now, as well as the handles. */
  winFreeDropSource(pSrc);
  for(i = 0; i < count; i++)
    GlobalFree(hData[i]);

  if(cbDragEnd)
  {
    int remove = -1;

    if (dwEffect == DROPEFFECT_MOVE && iupAttribGetBoolean(ih, "DRAGSOURCEMOVE"))
      remove = 1;  /* Shift pressed and DRAGSOURCEMOVE = YES */
    else if (dwEffect == DROPEFFECT_COPY)
      remove = 0;

    cbDragEnd(ih, remove);
  }
}

void winBeginDrag(Ihandle *ih, int x, int y)
{
  IFnii cbDragBegin = (IFnii)IupGetCallback(ih, "DRAGBEGIN_CB");

  if(cbDragBegin)
    if (cbDragBegin(ih, x, y) == IUP_IGNORE)
      return;

  winRegisterDrag(ih);
}

static int winDragProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
    case WM_LBUTTONDOWN:
    {
      POINT pt;
      GetCursorPos(&pt);

      if(DragDetect(ih->handle, pt) && iupAttribGetBoolean(ih, "DRAGSOURCE"))
      {
        int x = pt.x, y = pt.y;

        iupdrvScreenToClient(ih, &x, &y);
        
        winBeginDrag(ih, x, y);

        *result = 0;
        return 1;
      }
      break;
    }
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}

static Iarray* winRegisterAttrib(const char *value)
{
  char valueCopy[256];
  char valueTemp[256];
  int append = 0;

  sprintf(valueCopy, "%s", value);
  while(iupStrToStrStr(valueCopy, valueTemp, valueCopy, ',') > 0)
  {
    append++;
    if(iupStrEqualNoCase(valueCopy, valueTemp))
      break;
  }

  if(append > 0)
  {
    Iarray *newList = iupArrayCreate(append, sizeof(char*));
    char** newListData;
    int i;

    sprintf(valueCopy, "%s", value);
    for(i = 0; i < append; i++)
    {
      iupStrToStrStr(valueCopy, valueTemp, valueCopy, ',');
      newListData = (char**)iupArrayInc(newList);
      newListData[i] = iupStrDup(valueTemp);
    }

    return newList;
  }

  return NULL;
}

static int winSetDropTypesAttrib(Ihandle* ih, const char* value)
{
  Iarray *drop_types_list;

  if(!value)
    return 0;

  drop_types_list = winRegisterAttrib(value);
  iupAttribSetStr(ih, "_IUP_DROP_TYPES", (char*)drop_types_list);

  return 1;
}

static int winSetDropTargetAttrib(Ihandle* ih, const char* value)
{
  if (!ih->handle)
    return 0;

  /* Are there defined drop types? */
  if(!iupAttribGet(ih, "_IUP_DROP_TYPES"))
    return 0;

  if(iupStrBoolean(value))
  {
    winDragDropInit(NULL);      
    winRegisterDrop(ih);
  }
  else
  {
    iupArrayDestroy((Iarray*)iupAttribGet(ih, "_IUP_DROP_TYPES"));
    iupAttribSetStr(ih, "_IUP_DROP_TYPES", NULL);
  }

  return 1;
}

static int winSetDragTypesAttrib(Ihandle* ih, const char* value)
{
  Iarray *drag_types_list;

  if(!value)
    return 0;

  drag_types_list = winRegisterAttrib(value);
  iupAttribSetStr(ih, "_IUP_DRAG_TYPES", (char*)drag_types_list);

  return 1;
}

static int winSetDragSourceAttrib(Ihandle* ih, const char* value)
{
  if (!ih->handle)
    return 0;

  /* Are there defined drag types? */
  if(!iupAttribGet(ih, "_IUP_DRAG_TYPES"))
    return 0;

  if(iupStrBoolean(value))
  {
    IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winDragProc);
  }
  else
  {
    iupArrayDestroy((Iarray*)iupAttribGet(ih, "_IUP_DRAG_TYPES"));
    iupAttribSetStr(ih, "_IUP_DRAG_TYPES", NULL);
  }

  return 1;
}

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

void iupdrvRegisterDragDropAttrib(Iclass* ic)
{
  iupClassRegisterCallback(ic, "DROPFILES_CB", "siii");

  iupClassRegisterCallback(ic, "DRAGBEGIN_CB", "hsii");
  iupClassRegisterCallback(ic, "DRAGDATA_CB",  "hsCi");
  iupClassRegisterCallback(ic, "DRAGEND_CB",   "hi");
  iupClassRegisterCallback(ic, "DROPDATA_CB",  "hsCiii");

  iupClassRegisterAttribute(ic, "DRAGTYPES",  NULL, winSetDragTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPTYPES",  NULL, winSetDropTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGSOURCE", NULL, winSetDragSourceAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPTARGET", NULL, winSetDropTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGSOURCEMOVE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, winSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPFILESTARGET", NULL, winSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
}
