/** \file
 * \brief Drag and Drop for Windows
 *
 * This code is based on the DragDropImpl.cpp created by Leon Finker (2001).
 * More details in: http://www.codeproject.com/KB/shell/dragdrop.aspx
 * 
 * Some parts of this code have been adapted for the
 * IUP Generic Drag and Drop purpose.
 *
 * See Copyright Notice in "iup.h"
 */
 
#include <stdio.h>              
#include <stdlib.h>
#include <string.h>             

#include <windows.h>
#include <commctrl.h>

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

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_brush.h"
#include "iupwin_info.h"

#if 0
#include <shlobj.h>
#include <atlbase.h>

/*///////////////////////////////////////////////////////////////////////////////////////////*/
class CEnumFormatEtc : public IEnumFORMATETC
{
  private:
    ULONG m_cRefCount;
    CSimpleArray<FORMATETC> m_pFmtEtc;
    int m_iCur;

  public:
    CEnumFormatEtc(const CSimpleArray<FORMATETC>& ArrFE);
	  CEnumFormatEtc(const CSimpleArray<FORMATETC*>& ArrFE);
    /* IUnknown members */
    STDMETHOD(QueryInterface)(REFIID, void FAR* FAR*);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IEnumFORMATETC members */
    STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG FAR *);
    STDMETHOD(Skip)(ULONG);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumFORMATETC FAR * FAR*);
};

/*///////////////////////////////////////////////////////////////////////////////////////////*/
class CIDropSource : public IDropSource
{
  private:
    long m_cRefCount;
  public:
    bool m_bDropped;

	  CIDropSource::CIDropSource():m_cRefCount(0),m_bDropped(false) {}
	  
    /* IUnknown */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);        
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
    virtual ULONG STDMETHODCALLTYPE Release( void);
	  
    /* IDropSource */
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag( 
        /* [in] */ BOOL fEscapePressed,
        /* [in] */ DWORD grfKeyState);
    virtual HRESULT STDMETHODCALLTYPE GiveFeedback( 
        /* [in] */ DWORD dwEffect);
};

/*///////////////////////////////////////////////////////////////////////////////////////////*/
class CIDataObject : public IDataObject/*, public IAsyncOperation */
{
  private:
    CIDropSource* m_pDropSource;
    long m_cRefCount;
    CSimpleArray<FORMATETC*> m_ArrFormatEtc;
    CSimpleArray<STGMEDIUM*> m_StgMedium;
  public:
    HWND m_hSourceWnd;  /***** IUP *****/
    CIDataObject(CIDropSource* pDropSource, HWND hSourceWnd);  /***** IUP *****/
    CIDataObject(CIDropSource* pDropSource);
	  ~CIDataObject();
	  void CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc);
    
    /* IUnknown */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);        
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
    virtual ULONG STDMETHODCALLTYPE Release( void);

    /* IDataObject */
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetData( 
        /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
        /* [out] */ STGMEDIUM __RPC_FAR *pmedium);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDataHere( 
        /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
        /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium);
    
    virtual HRESULT STDMETHODCALLTYPE QueryGetData( 
        /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc);
    
    virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc( 
        /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
        /* [out] */ FORMATETC __RPC_FAR *pformatetcOut);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE SetData( 
        /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
        /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
        /* [in] */ BOOL fRelease);
    
    virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc( 
        /* [in] */ DWORD dwDirection,
        /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc);
    
    virtual HRESULT STDMETHODCALLTYPE DAdvise( 
        /* [in] */ FORMATETC __RPC_FAR *pformatetc,
        /* [in] */ DWORD advf,
        /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
        /* [out] */ DWORD __RPC_FAR *pdwConnection);
    
    virtual HRESULT STDMETHODCALLTYPE DUnadvise( 
        /* [in] */ DWORD dwConnection);
    
    virtual HRESULT STDMETHODCALLTYPE EnumDAdvise( 
        /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);

    /* IAsyncOperation */
    /*
    virtual HRESULT STDMETHODCALLTYPE SetAsyncMode(BOOL fDoOpAsync)
    {
      // [in] => BOOL fDoOpAsync
    	return E_NOTIMPL;
    }
    
    virtual HRESULT STDMETHODCALLTYPE GetAsyncMode(BOOL __RPC_FAR *pfIsOpAsync)
    {
      // [out] => BOOL __RPC_FAR *pfIsOpAsync
    	return E_NOTIMPL;
    }
    
    virtual HRESULT STDMETHODCALLTYPE StartOperation(IBindCtx __RPC_FAR *pbcReserved)
    {
      // [optional][unique][in] => IBindCtx __RPC_FAR *pbcReserved)
    	return E_NOTIMPL;
    }
    
    virtual HRESULT STDMETHODCALLTYPE InOperation(BOOL __RPC_FAR *pfInAsyncOp)
    {
      // [out] => BOOL __RPC_FAR *pfInAsyncOp
    	return E_NOTIMPL;
    }
    
    virtual HRESULT STDMETHODCALLTYPE EndOperation(HRESULT hResult, IBindCtx __RPC_FAR *pbcReserved, DWORD dwEffects)
    {
      // [in] => HRESULT hResult,
      // [unique][in] => IBindCtx __RPC_FAR *pbcReserved,
      // [in] => DWORD dwEffects)
    	return E_NOTIMPL;
    }
    */
};

/*///////////////////////////////////////////////////////////////////////////////////////////*/
class CIDropTarget : public IDropTarget
{
  private:
    DWORD m_cRefCount;
    bool m_bAllowDrop;
    struct IDropTargetHelper *m_pDropTargetHelper;
    CSimpleArray<FORMATETC> m_formatetc;
    FORMATETC* m_pSupportedFrmt;
  protected:
    HWND m_hTargetWnd;
  public:
	  CIDropTarget(HWND m_hTargetWnd);
	  virtual ~CIDropTarget();
	  void AddSuportedFormat(FORMATETC& ftetc) { m_formatetc.Add(ftetc); }

    void OnDragDataGet(CIDataObject* obj, FORMATETC* fmt);  /***** IUP *****/
	
	  /* return values: true - release the medium. false - don't release the medium */
	  virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		    /* [in] */ REFIID riid,
		    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef( void) { ATLTRACE("CIDropTarget::AddRef\n"); return ++m_cRefCount; }
    virtual ULONG STDMETHODCALLTYPE Release( void);

    bool QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragEnter(
        /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragOver( 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragLeave( void);    
    virtual HRESULT STDMETHODCALLTYPE Drop(
        /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
};

/*///////////////////////////////////////////////////////////////////////////////////////////*/
class CDragSourceHelper
{
  private:
    IDragSourceHelper* pDragSourceHelper;
  public:
    CDragSourceHelper()
	  {
		  if(FAILED(CoCreateInstance(CLSID_DragDropHelper,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDragSourceHelper,
                          (void**)&pDragSourceHelper)))
			  pDragSourceHelper = NULL;
	  }
    virtual ~CDragSourceHelper()
    {
      if( pDragSourceHelper!= NULL )
      {
        pDragSourceHelper->Release();
        pDragSourceHelper=NULL;
      }
    }
    
	  /* IDragSourceHelper */
    HRESULT InitializeFromBitmap(HBITMAP hBitmap, 
		    POINT& pt,  /* cursor position in client coordinates of the window */
		    RECT&  rc,  /* selected item's bounding rect */
		    IDataObject* pDataObject,
		    COLORREF crColorKey=GetSysColor(COLOR_WINDOW))  /* color of the window used for transparent effect */
    {
      if(pDragSourceHelper == NULL)
        return E_FAIL;
      
      SHDRAGIMAGE di;
      BITMAP      bm;
      
      GetObject(hBitmap, sizeof(bm), &bm);
      di.sizeDragImage.cx = bm.bmWidth;
      di.sizeDragImage.cy = bm.bmHeight;
      di.hbmpDragImage = hBitmap;
      di.crColorKey = crColorKey;
      di.ptOffset.x = pt.x - rc.left;
      di.ptOffset.y = pt.y - rc.top;
      return pDragSourceHelper->InitializeFromBitmap(&di, pDataObject);
    }
    
    HRESULT InitializeFromWindow(HWND hwnd, POINT& pt,IDataObject* pDataObject)
    {
      if(pDragSourceHelper == NULL)
        return E_FAIL;
      
      return pDragSourceHelper->InitializeFromWindow(hwnd, &pt, pDataObject);
    }
};

/*//////////////////////////////////////////////////////////////////*/
/* CIDataObject Class                                               */
/*//////////////////////////////////////////////////////////////////*/

CIDataObject::CIDataObject(CIDropSource* pDropSource):
m_cRefCount(0), m_pDropSource(pDropSource)
{
}

CIDataObject::CIDataObject(CIDropSource* pDropSource, HWND hSourceWnd):
m_cRefCount(0), m_pDropSource(pDropSource), m_hSourceWnd(hSourceWnd)
{
}

CIDataObject::~CIDataObject()
{
  for(int i = 0; i < m_StgMedium.GetSize(); ++i)
	{
    ReleaseStgMedium(m_StgMedium[i]);
		delete m_StgMedium[i];
	}
	
  for(int j = 0; j < m_ArrFormatEtc.GetSize(); ++j)
		delete m_ArrFormatEtc[j];
}

STDMETHODIMP CIDataObject::QueryInterface(/* [in] */ REFIID riid,
/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	*ppvObject = NULL;
	
  if(IID_IUnknown==riid || IID_IDataObject==riid)
    *ppvObject=this;

  /*if(riid == IID_IAsyncOperation)
    *ppvObject=(IAsyncOperation*)this;*/
  
  if (NULL!=*ppvObject)
  {
    ((LPUNKNOWN)*ppvObject)->AddRef();
    return S_OK;
  }
  
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CIDataObject::AddRef(void)
{
	ATLTRACE("CIDataObject::AddRef\n");
	return ++m_cRefCount;
}

STDMETHODIMP_(ULONG) CIDataObject::Release(void)
{
  ATLTRACE("CIDataObject::Release\n");
  long nTemp;
  
  nTemp = --m_cRefCount;
  if(nTemp==0)
    delete this;
  
  return nTemp;
}

STDMETHODIMP CIDataObject::GetData( 
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ STGMEDIUM __RPC_FAR *pmedium)
{ 
	ATLTRACE("CIDataObject::GetData\n");
	if(pformatetcIn == NULL || pmedium == NULL)
    return E_INVALIDARG;
	pmedium->hGlobal = NULL;

	ATLASSERT(m_StgMedium.GetSize() == m_ArrFormatEtc.GetSize());
	for(int i=0; i < m_ArrFormatEtc.GetSize(); ++i)
	{
		if(pformatetcIn->tymed & m_ArrFormatEtc[i]->tymed &&
		   pformatetcIn->dwAspect == m_ArrFormatEtc[i]->dwAspect &&
		   pformatetcIn->cfFormat == m_ArrFormatEtc[i]->cfFormat)
		{
			CopyMedium(pmedium, m_StgMedium[i], m_ArrFormatEtc[i]);
			return S_OK;
		}
	}
	return DV_E_FORMATETC;
}

STDMETHODIMP CIDataObject::GetDataHere( 
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium)
{
  ATLTRACE("CIDataObject::GetDataHere\n");
  (void)pformatetc;
  (void)pmedium;

  return E_NOTIMPL;
}

STDMETHODIMP CIDataObject::QueryGetData( 
   /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc)
{
  ATLTRACE("CIDataObject::QueryGetData\n");
	if(pformatetc == NULL)
		return E_INVALIDARG;

	/* support others if needed DVASPECT_THUMBNAIL  //DVASPECT_ICON   //DVASPECT_DOCPRINT */
	if(!(DVASPECT_CONTENT & pformatetc->dwAspect))
    return (DV_E_DVASPECT);
	
  HRESULT hr = DV_E_TYMED;
	for(int i = 0; i < m_ArrFormatEtc.GetSize(); ++i)
	{
    if(pformatetc->tymed & m_ArrFormatEtc[i]->tymed)
    {
      if(pformatetc->cfFormat == m_ArrFormatEtc[i]->cfFormat)
        return S_OK;
		  else
        hr = DV_E_CLIPFORMAT;
    }
    else
      hr = DV_E_TYMED;
	}

	return hr;
}

STDMETHODIMP CIDataObject::GetCanonicalFormatEtc( 
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
    /* [out] */ FORMATETC __RPC_FAR *pformatetcOut)
{
  ATLTRACE("CIDataObject::GetCanonicalFormatEtc\n");
	if (pformatetcOut == NULL)
		return E_INVALIDARG;
	
  (void)pformatectIn;
  return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP CIDataObject::SetData( 
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease)
{ 
	ATLTRACE("CIDataObject::SetData\n");
	if(pformatetc == NULL || pmedium == NULL)
    return E_INVALIDARG;

	ATLASSERT(pformatetc->tymed == pmedium->tymed);
	FORMATETC* fetc=new FORMATETC;
	STGMEDIUM* pStgMed = new STGMEDIUM;

	if(fetc == NULL || pStgMed == NULL)
    return E_OUTOFMEMORY;

	ZeroMemory(fetc,sizeof(FORMATETC));
	ZeroMemory(pStgMed,sizeof(STGMEDIUM));

	*fetc = *pformatetc;
	m_ArrFormatEtc.Add(fetc);
  
  if(fRelease)
    *pStgMed = *pmedium;
  else
		CopyMedium(pStgMed, pmedium, pformatetc);

  m_StgMedium.Add(pStgMed);  
  return S_OK;
}

void CIDataObject::CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc)
{
  switch(pMedSrc->tymed)
  {
		case TYMED_HGLOBAL:
			pMedDest->hGlobal = (HGLOBAL)OleDuplicateData(pMedSrc->hGlobal,pFmtSrc->cfFormat, NULL);
			break;
		case TYMED_GDI:
			pMedDest->hBitmap = (HBITMAP)OleDuplicateData(pMedSrc->hBitmap,pFmtSrc->cfFormat, NULL);
			break;
		case TYMED_MFPICT:
			pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pMedSrc->hMetaFilePict,pFmtSrc->cfFormat, NULL);
			break;
		case TYMED_ENHMF:
			pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pMedSrc->hEnhMetaFile,pFmtSrc->cfFormat, NULL);
			break;
		case TYMED_FILE:
			pMedSrc->lpszFileName = (LPOLESTR)OleDuplicateData(pMedSrc->lpszFileName,pFmtSrc->cfFormat, NULL);
			break;
		case TYMED_ISTREAM:
			pMedDest->pstm = pMedSrc->pstm;
			pMedSrc->pstm->AddRef();
			break;
		case TYMED_ISTORAGE:
			pMedDest->pstg = pMedSrc->pstg;
			pMedSrc->pstg->AddRef();
			break;
		case TYMED_NULL:
		default:
			break;
  }
  
  pMedDest->tymed = pMedSrc->tymed;
  pMedDest->pUnkForRelease = NULL;
  if(pMedSrc->pUnkForRelease != NULL)
  {
    pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
    pMedSrc->pUnkForRelease->AddRef();
  }
}

STDMETHODIMP CIDataObject::EnumFormatEtc(
   /* [in] */ DWORD dwDirection,
   /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc)
{ 
	ATLTRACE("CIDataObject::EnumFormatEtc\n");
	if(ppenumFormatEtc == NULL)
    return E_POINTER;

	*ppenumFormatEtc=NULL;
  switch (dwDirection)
  {
    case DATADIR_GET:
      *ppenumFormatEtc= new CEnumFormatEtc(m_ArrFormatEtc);
      if(*ppenumFormatEtc == NULL)
        return E_OUTOFMEMORY;
      
      (*ppenumFormatEtc)->AddRef();
      break;

    case DATADIR_SET:
    default:
      return E_NOTIMPL;
      break;
  }
  
  return S_OK;
}

STDMETHODIMP CIDataObject::DAdvise( 
   /* [in] */ FORMATETC __RPC_FAR *pformatetc,
   /* [in] */ DWORD advf,
   /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
   /* [out] */ DWORD __RPC_FAR *pdwConnection)
{ 
	ATLTRACE("CIDataObject::DAdvise\n");
  (void)pformatetc;
  (void)advf;
  (void)pAdvSink;
  (void)pdwConnection;

	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CIDataObject::DUnadvise( 
   /* [in] */ DWORD dwConnection)
{
	ATLTRACE("CIDataObject::DUnadvise\n");
  (void)dwConnection;

	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CIDataObject::EnumDAdvise( 
   /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise)
{
	ATLTRACE("CIDataObject::EnumDAdvise\n");
  (void)ppenumAdvise;

	return OLE_E_ADVISENOTSUPPORTED;
}

/*//////////////////////////////////////////////////////////////////*/
/* CIDropSource Class                                               */
/*//////////////////////////////////////////////////////////////////*/

STDMETHODIMP CIDropSource::QueryInterface(/* [in] */ REFIID riid,
										 /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
  *ppvObject = NULL;
  if(IID_IUnknown==riid || IID_IDropSource==riid)
    *ppvObject=this;
  
  if (*ppvObject != NULL)
  {
    ((LPUNKNOWN)*ppvObject)->AddRef();
    return S_OK;
  }

  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CIDropSource::AddRef( void)
{
	ATLTRACE("CIDropSource::AddRef\n");
	return ++m_cRefCount;
}

STDMETHODIMP_(ULONG) CIDropSource::Release( void)
{
	ATLTRACE("CIDropSource::Release\n");
  long nTemp;
  
  nTemp = --m_cRefCount;
  ATLASSERT(nTemp >= 0);
  
  if(nTemp==0)
    delete this;
  
  return nTemp;
}

STDMETHODIMP CIDropSource::QueryContinueDrag( 
    /* [in] */ BOOL fEscapePressed,
    /* [in] */ DWORD grfKeyState)
{
  /* ATLTRACE("CIDropSource::QueryContinueDrag\n"); */
  if(fEscapePressed)
    return DRAGDROP_S_CANCEL;
  if(!(grfKeyState & (MK_LBUTTON|MK_RBUTTON)))
  {
    m_bDropped = true;
    return DRAGDROP_S_DROP;
  }
  
  return S_OK;
}

STDMETHODIMP CIDropSource::GiveFeedback(
    /* [in] */ DWORD dwEffect)
{
	ATLTRACE("CIDropSource::GiveFeedback\n");
  (void)dwEffect;

	return DRAGDROP_S_USEDEFAULTCURSORS;
}

/*//////////////////////////////////////////////////////////////////*/
/* CEnumFormatEtc Class                                             */
/*//////////////////////////////////////////////////////////////////*/

CEnumFormatEtc::CEnumFormatEtc(const CSimpleArray<FORMATETC>& ArrFE):
m_cRefCount(0),m_iCur(0)
{
  ATLTRACE("CEnumFormatEtc::CEnumFormatEtc()\n");
  for(int i = 0; i < ArrFE.GetSize(); ++i)
    m_pFmtEtc.Add(ArrFE[i]);
}

CEnumFormatEtc::CEnumFormatEtc(const CSimpleArray<FORMATETC*>& ArrFE):
m_cRefCount(0),m_iCur(0)
{
  for(int i = 0; i < ArrFE.GetSize(); ++i)
    m_pFmtEtc.Add(*ArrFE[i]);
}

STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID refiid, void FAR* FAR* ppv)
{
  ATLTRACE("CEnumFormatEtc::QueryInterface()\n");
  *ppv = NULL;
  if(IID_IUnknown==refiid || IID_IEnumFORMATETC==refiid)
    *ppv=this;
  
  if (*ppv != NULL)
  {
    ((LPUNKNOWN)*ppv)->AddRef();
    return S_OK;
  }

  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void)
{
  ATLTRACE("CEnumFormatEtc::AddRef()\n");
  return ++m_cRefCount;
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void)
{
  ATLTRACE("CEnumFormatEtc::Release()\n");
  long nTemp = --m_cRefCount;
  
  ATLASSERT(nTemp >= 0);
  if(nTemp == 0)
    delete this;
  
  return nTemp; 
}

STDMETHODIMP CEnumFormatEtc::Next(ULONG celt,LPFORMATETC lpFormatEtc, ULONG FAR *pceltFetched)
{
  ATLTRACE("CEnumFormatEtc::Next()\n");
  if(pceltFetched != NULL)
    *pceltFetched=0;
	
   ULONG cReturn = celt;

   if(celt <= 0 || lpFormatEtc == NULL || m_iCur >= m_pFmtEtc.GetSize())
     return S_FALSE;

   if(pceltFetched == NULL && celt != 1)  /* pceltFetched can be NULL only for 1 item request */
     return S_FALSE;
   
   while (m_iCur < m_pFmtEtc.GetSize() && cReturn > 0)
   {
     *lpFormatEtc++ = m_pFmtEtc[m_iCur++];
     --cReturn;
   }
   
   if (pceltFetched != NULL)
     *pceltFetched = celt - cReturn;
   
   return (cReturn == 0) ? S_OK : S_FALSE;
}
   
STDMETHODIMP CEnumFormatEtc::Skip(ULONG celt)
{
  ATLTRACE("CEnumFormatEtc::Skip()\n");
	if((m_iCur + int(celt)) >= m_pFmtEtc.GetSize())
    return S_FALSE;
	
  m_iCur += celt;
	return S_OK;
}

STDMETHODIMP CEnumFormatEtc::Reset(void)
{
  ATLTRACE("CEnumFormatEtc::Reset()\n");
  m_iCur = 0;
  return S_OK;
}
               
STDMETHODIMP CEnumFormatEtc::Clone(IEnumFORMATETC FAR * FAR*ppCloneEnumFormatEtc)
{
  ATLTRACE("CEnumFormatEtc::Clone()\n");
  if(ppCloneEnumFormatEtc == NULL)
    return E_POINTER;
      
  CEnumFormatEtc *newEnum = new CEnumFormatEtc(m_pFmtEtc);
  if(newEnum ==NULL)
		return E_OUTOFMEMORY;  	
  
  newEnum->AddRef();
  newEnum->m_iCur = m_iCur;
  *ppCloneEnumFormatEtc = newEnum;
  
  return S_OK;
}

/*//////////////////////////////////////////////////////////////////*/
/* CIDropTarget Class                                               */
/*//////////////////////////////////////////////////////////////////*/

CIDropTarget::CIDropTarget(HWND hTargetWnd): 
	m_hTargetWnd(hTargetWnd),
	m_cRefCount(0), m_bAllowDrop(false),
	m_pDropTargetHelper(NULL), m_pSupportedFrmt(NULL)
{ 
  ATLASSERT(m_hTargetWnd != NULL);

	if(FAILED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                             IID_IDropTargetHelper, (LPVOID*)&m_pDropTargetHelper)))
    m_pDropTargetHelper = NULL;
}

CIDropTarget::~CIDropTarget()
{
	if(m_pDropTargetHelper != NULL)
	{
		m_pDropTargetHelper->Release();
		m_pDropTargetHelper = NULL;
	}
}

HRESULT STDMETHODCALLTYPE CIDropTarget::QueryInterface( /* [in] */ REFIID riid,
						/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
  *ppvObject = NULL;
  if(IID_IUnknown==riid || IID_IDropTarget==riid)
    *ppvObject=this;

	if(*ppvObject != NULL)
	{
    ((LPUNKNOWN)*ppvObject)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CIDropTarget::Release( void)
{
  ATLTRACE("CIDropTarget::Release\n");
  long nTemp;
  
  nTemp = --m_cRefCount;
  ATLASSERT(nTemp >= 0);
  
  if(nTemp==0)
    delete this;
  
  return nTemp;
}

bool CIDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{  
	ATLTRACE("CIDropTarget::QueryDrop\n");
	DWORD dwOKEffects = *pdwEffect; 

	if(!m_bAllowDrop)
	{
    *pdwEffect = DROPEFFECT_NONE;
	  return false;
	}

	/* CTRL+SHIFT  -- DROPEFFECT_LINK */
	/* CTRL        -- DROPEFFECT_COPY */
	/* SHIFT       -- DROPEFFECT_MOVE */
	/* no modifier -- DROPEFFECT_MOVE or whatever is allowed by source */
 	*pdwEffect = (grfKeyState & MK_CONTROL) ?
         ((grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY) :
				 ((grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0);

  if(*pdwEffect == 0) 
	{
    /* No modifier keys used by user while dragging */
    if (DROPEFFECT_COPY & dwOKEffects)
      *pdwEffect = DROPEFFECT_COPY;
    else if (DROPEFFECT_MOVE & dwOKEffects)
      *pdwEffect = DROPEFFECT_MOVE;
    else if (DROPEFFECT_LINK & dwOKEffects)
      *pdwEffect = DROPEFFECT_LINK; 
    else
      *pdwEffect = DROPEFFECT_NONE;
	} 
	else
	{
    /* Check if the drag source application allows the drop effect desired by user.
       The drag source specifies this in DoDragDrop */
    if(!(*pdwEffect & dwOKEffects))
      *pdwEffect = DROPEFFECT_NONE;
	}  

	return (DROPEFFECT_NONE == *pdwEffect) ? false : true;
}   

HRESULT STDMETHODCALLTYPE CIDropTarget::DragEnter(
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ DWORD __RPC_FAR *pdwEffect)
{
  ATLTRACE("CIDropTarget::DragEnter\n");
	if(pDataObj == NULL)
    return E_INVALIDARG;

	if(m_pDropTargetHelper)
    m_pDropTargetHelper->DragEnter(m_hTargetWnd, pDataObj, (LPPOINT)&pt, *pdwEffect);

	/*
  IEnumFORMATETC* pEnum;
	pDataObj->EnumFormatEtc(DATADIR_GET,&pEnum);
	FORMATETC ftm;
	for()
	pEnum->Next(1,&ftm,0);
	pEnum->Release();
  */

	m_pSupportedFrmt = NULL;
	for(int i =0; i<m_formatetc.GetSize(); ++i)
	{
    m_bAllowDrop = (pDataObj->QueryGetData(&m_formatetc[i]) == S_OK)?true:false;
		if(m_bAllowDrop)
		{
      m_pSupportedFrmt = &m_formatetc[i];
      break;
		}
	}

	QueryDrop(grfKeyState, pdwEffect);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragOver( 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect)
{
	ATLTRACE("CIDropTarget::DragOver\n");
	if(m_pDropTargetHelper)
    m_pDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);
	
  QueryDrop(grfKeyState, pdwEffect);
	
  return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragLeave( void)
{
	ATLTRACE("CIDropTarget::DragLeave\n");

	if(m_pDropTargetHelper)
    m_pDropTargetHelper->DragLeave();
	
	m_bAllowDrop = false;
	m_pSupportedFrmt = NULL;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::Drop(
	/* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
    /* [in] */ DWORD grfKeyState, /* [in] */ POINTL pt, 
	/* [out][in] */ DWORD __RPC_FAR *pdwEffect)
{
  ATLTRACE("CIDropTarget::Drop\n");
	if (pDataObj == NULL)
		return E_INVALIDARG;	

	if(m_pDropTargetHelper)
    m_pDropTargetHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);

	if(QueryDrop(grfKeyState, pdwEffect))
	{
		if(m_bAllowDrop && m_pSupportedFrmt != NULL)
		{
      STGMEDIUM medium;
			if(pDataObj->GetData(m_pSupportedFrmt, &medium) == S_OK)
			{
        OnDragDataGet((CIDataObject*)pDataObj, m_pSupportedFrmt);  /***** IUP *****/

				if(OnDrop(m_pSupportedFrmt, medium, pdwEffect))  /* Does derive class wants us to free medium? */
					ReleaseStgMedium(&medium);
			}
		}
	}

	m_bAllowDrop=false;
	*pdwEffect = DROPEFFECT_NONE;
	m_pSupportedFrmt = NULL;
	
  return S_OK;
}

/*//////////////////////////////////////////////////////////////////*/
/* IUP - CIDropTarget methods                                       */
/*//////////////////////////////////////////////////////////////////*/

#include "iup.h"
#include "iupcbs.h"

#include "iup_drv.h"
#include "iup_drvinfo.h"
#include "iup_str.h"
#include "iup_array.h"
#include "iup_object.h"
#include "iup_attrib.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"


void CIDropTarget::OnDragDataGet(CIDataObject* obj, FORMATETC* fmt)
{
  Ihandle *ih = iupwinHandleGet(obj->m_hSourceWnd);
  char* source = iupAttribGet(ih, "IUP_DRAG_DATA");
  char* type = iupStrGetMemory(256);
  int is_ctrl;
  IFnnsi cbDrag = (IFnnsi)IupGetCallback(ih, "DRAGSOURCE_CB");

  if(!source)
    return;

  GetClipboardFormatName(fmt->cfFormat, type, 256);

  if (GetKeyState(VK_CONTROL) & 0x8000)
    is_ctrl = 1;  /* COPY */
  else
    is_ctrl = 0;  /* MOVE */

  if (cbDrag)
    cbDrag(ih, (Ihandle*)source, type, is_ctrl);

  /* Testing... */
  printf("DRAGSOURCE_CB ==> Ihandle* ih, Ihandle* source, Type: %s, 0=Move/1=Copy: %d\n", type, is_ctrl);
}

bool CIDropTarget::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
{
  Ihandle *ih = iupwinHandleGet(m_hTargetWnd);
  char *dropList = iupAttribGet(ih, "_IUP_DROP_TYPES");
  int i, count = iupArrayCount((Iarray*)dropList);
  char** iupList = (char**)iupArrayGetData((Iarray*)dropList);
  char* type = iupStrGetMemory(256);
  (void)pdwEffect;

  GetClipboardFormatName(pFmtEtc->cfFormat, type, 256);

  for(i = 0; i < count; i++)
  {
    if(iupStrEqualNoCase(type, iupList[i]))
    {
      IFnnsii cbDrop = (IFnnsii)IupGetCallback(ih, "DROPTARGET_CB");
      char* target = (char*)GlobalLock(medium.hGlobal);
      int x, y;

      iupdrvGetCursorPos(&x, &y);
      iupdrvScreenToClient(ih, &x, &y);

      if(!target)
        return false;

      GlobalUnlock(medium.hGlobal);

      if(cbDrop)
        cbDrop(ih, (Ihandle*)target, type, x, y);

      /* Testing... */
      printf("DROPTARGET_CB ==> Ihandle*: ih, Ihandle*: target, Type: %s, X: %d, Y: %d\n", type, x, y);

      return true;
    }
  }

  return false;
}

/*//////////////////////////////////////////////////////////////////*/
/* IUP - Functions to access the Drag and Drop classes              */
/*//////////////////////////////////////////////////////////////////*/

static void winBeginDrag(HWND hwndSrc)
{
  DWORD dwEffect;
  HRESULT hr;
  CIDropSource* pdSrc = new CIDropSource();
  CIDataObject* pdObj = new CIDataObject(pdSrc, hwndSrc);
  Ihandle *ih = iupwinHandleGet(hwndSrc);
  char *dragList = iupAttribGet(ih, "_IUP_DRAG_TYPES");
  char* source = iupAttribGet(ih, "IUP_DRAG_DATA");
  int i, count = iupArrayCount((Iarray*)dragList);
  char** iupList = (char**)iupArrayGetData((Iarray*)dragList);
  char* pMem;

  if(!source)
    return;

  for(i = 0; i < count; i++)
  {
    FORMATETC fmtetc = {0}; 
    STGMEDIUM medium = {0};

    /* Drag supported formats */
    fmtetc.cfFormat  = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]); 
    fmtetc.dwAspect  = DVASPECT_CONTENT; 
    fmtetc.lindex    = -1; 
    fmtetc.tymed     = TYMED_HGLOBAL;
  
    /* Init the medium used (source data) */
    medium.tymed = TYMED_HGLOBAL;
    medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, strlen(source)+1);
    pMem = (char*)GlobalLock(medium.hGlobal);
    strcpy(pMem, source);
    GlobalUnlock(medium.hGlobal);

    /* Add it to DataObject */
    pdObj->SetData(&fmtetc, &medium, TRUE);
  }

  /* Initiate the Drag & Drop */
  hr = ::DoDragDrop(pdObj, pdSrc, DROPEFFECT_MOVE | DROPEFFECT_COPY, &dwEffect);

  if(hr != DRAGDROP_S_DROP)
    return;
} 

static void winRegisterDrop(HWND hwndDst)
{
  CIDropTarget* pdDst = new CIDropTarget(hwndDst);
  Ihandle *ih = iupwinHandleGet(hwndDst);
  char *dropList = iupAttribGet(ih, "_IUP_DROP_TYPES");
  int i, count = iupArrayCount((Iarray*)dropList);
  char** iupList = (char**)iupArrayGetData((Iarray*)dropList);
  HRESULT hr;

  hr = RegisterDragDrop(hwndDst, pdDst);

  if(FAILED(hr))
    return;

  /* Drop supported formats */
  for(i = 0; i < count; i++)
  {
    FORMATETC fmtetc = {0};

    fmtetc.cfFormat  = (CLIPFORMAT)RegisterClipboardFormat(iupList[i]); 
    fmtetc.dwAspect  = DVASPECT_CONTENT; 
    fmtetc.lindex    = -1; 
    fmtetc.tymed     = TYMED_HGLOBAL;

    pdDst->AddSuportedFormat(fmtetc); 
  }
}

static int winDragProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  POINT pt;

  switch (msg)
  {
    case WM_LBUTTONDOWN:
      GetCursorPos(&pt);
      if(DragDetect(ih->handle, pt))
      {
        winBeginDrag(ih->handle);
        return 1;
      }
      break;
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
    winRegisterDrop(ih->handle);
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
#endif

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

  //iupClassRegisterCallback(ic, "DRAGSOURCE_CB", "hsi");
  //iupClassRegisterCallback(ic, "DROPTARGET_CB", "hsii");

  //iupClassRegisterAttribute(ic, "DRAGTYPES",  NULL, winSetDragTypesAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "DROPTYPES",  NULL, winSetDropTypesAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "DRAGSOURCE", NULL, winSetDragSourceAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "DROPTARGET", NULL, winSetDropTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, winSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPFILESTARGET", NULL, winSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
}
