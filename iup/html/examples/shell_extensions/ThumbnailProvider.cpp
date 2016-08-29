/******************************** Module Header ********************************\
Module Name:  ThumbnailProvider.cpp
Copyright (c) Microsoft Corporation.

The code sample demonstrates the C++ implementation of a thumbnail handler 
for a new file type registered with the given extension. 

A thumbnail image handler provides an image to represent the item. It lets you 
customize the thumbnail of files with a specific file extension. Windows Vista 
and newer operating systems make greater use of file-specific thumbnail images 
than earlier versions of Windows. Thumbnails of 32-bit resolution and as large 
as 256x256 pixels are often used. File format owners should be prepared to 
display their thumbnails at that size. 

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\*******************************************************************************/

#include "ThumbnailProvider.h"

#include <im.h>
#include <im_image.h>
#include <im_process.h>
#include <im_convert.h>
#include <im_dib.h>

#include <Shlwapi.h>
#include <stdio.h>

#pragma comment(lib, "Shlwapi.lib")


extern HINSTANCE g_hInstDll;
extern long      g_cRefDll;


ThumbnailProvider::ThumbnailProvider()
  : m_cRef(1), m_pPathFile(NULL)
{
  InterlockedIncrement(&g_cRefDll);
}


ThumbnailProvider::~ThumbnailProvider()
{
  if (m_pPathFile)
  {
    LocalFree(m_pPathFile);
    m_pPathFile = NULL;
  }

  InterlockedDecrement(&g_cRefDll);
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP ThumbnailProvider::QueryInterface(REFIID riid, void **ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(ThumbnailProvider, IThumbnailProvider),
    QITABENT(ThumbnailProvider, IInitializeWithFile),
    { 0 },
  };
  return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ThumbnailProvider::AddRef()
{
  return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ThumbnailProvider::Release()
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

IFACEMETHODIMP ThumbnailProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode)
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


#pragma region IThumbnailProvider

static void imImageViewFitRect(int cnv_width, int cnv_height, int img_width, int img_height, int *w, int *h)
{
  double rView, rImage;
  int correct = 0;

  *w = cnv_width;
  *h = cnv_height;

  rView = ((double)cnv_height) / cnv_width;
  rImage = ((double)img_height) / img_width;

  if ((rView <= 1 && rImage <= 1) || (rView >= 1 && rImage >= 1)) /* view and image are horizontal rectangles */
  {
    if (rView > rImage)
      correct = 2;
    else
      correct = 1;
  }
  else if (rView < 1 && rImage > 1) /* view is a horizontal rectangle and image is a vertical rectangle */
    correct = 1;
  else if (rView > 1 && rImage < 1) /* view is a vertical rectangle and image is a horizontal rectangle */
    correct = 2;

  if (correct == 1)
    *w = (int)(cnv_height / rImage);
  else if (correct == 2)
    *h = (int)(cnv_width * rImage);
}


// Gets a thumbnail image and alpha type. The GetThumbnail is called with the 
// largest desired size of the image, in pixels. Although the parameter is 
// called cx, this is used as the maximum size of both the x and y dimensions. 
// If the retrieved thumbnail is not square, then the longer axis is limited 
// by cx and the aspect ratio of the original image respected. On exit, 
// GetThumbnail provides a handle to the retrieved image. It also provides a 
// value that indicates the color format of the image and whether it has 
// valid alpha information.
IFACEMETHODIMP ThumbnailProvider::GetThumbnail(UINT thumb_size, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
  char filename[10240];
  size_t size;
  wcstombs_s(&size, filename, 10240, m_pPathFile, 10240);

  int error;
  imImage* image = imFileImageLoadBitmap(filename, 0, &error);
  if (!image)
    return E_FAIL;

  if (image->color_space != IM_RGB)
  {
    imImage* rgb_image = imImageCreateBased(image, -1, -1, IM_RGB, -1);
    imConvertColorSpace(image, rgb_image);
    imImageDestroy(image);
    image = rgb_image;
  }

  int thumb_width, thumb_height;
  imImageViewFitRect(thumb_size, thumb_size, image->width, image->height, &thumb_width, &thumb_height);

  imImage* thumb_image = imImageCreateBased(image, thumb_width, thumb_height, -1, -1);
  imProcessResize(image, thumb_image, 1);
  imImageDestroy(image);

  HDC hDC = GetDC(NULL);
  imDib* dib = imDibSectionFromImage(hDC, phbmp, thumb_image);
  ReleaseDC(NULL, hDC);

  if (image->has_alpha)
    *pdwAlpha = WTSAT_ARGB;
  else
    *pdwAlpha = WTSAT_RGB;

  imDibDestroy(dib);
  imImageDestroy(thumb_image);

  return S_OK;
}

#pragma endregion
