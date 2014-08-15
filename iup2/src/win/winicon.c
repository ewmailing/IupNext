/** \file
 * \brief Windows Driver ICON management
 *
 * See Copyright Notice in iup.h
 *  */

#include <assert.h>
#include <stdio.h>

#include <windows.h>

#include "iglobal.h"
#include "idrv.h"
#include "win.h"
#include "winicon.h"
#include "wincreat.h"

/*
 * Cria icone windows a partir de image 
 */

HBITMAP iupwinCreateBitmap(Ihandle* hImage, HDC hDC, Ihandle* parent)
{
  BITMAPINFOHEADER* bmpheader;
  HBITMAP hBmp;
  IwinBitmap* image;
  int transp_index;
  DWORD old_bgcolor, bgcolor;
  RGBQUAD old_colors[256];

  assert(hImage && hDC);
  if(!hImage || !hDC)
    return NULL;

  if (handle(hImage) == NULL)
    iupdrvCreateObjects (hImage);

  image = (IwinBitmap*)handle(hImage);
  if (!image) 
    return NULL;

  bmpheader = (BITMAPINFOHEADER*)image->bmpinfo;
  hBmp = CreateCompatibleBitmap(hDC, bmpheader->biWidth, bmpheader->biHeight);
  if (!hBmp) return NULL;

  transp_index = IupGetInt(hImage, "_IUPWIN_TRANSP_INDEX");
  if (transp_index != -1)
  {
    unsigned int r = 0, g = 0, b = 0;
    RGBQUAD* bmpcolors = (RGBQUAD*)((BYTE*)image->bmpinfo + sizeof(BITMAPINFOHEADER));
    bmpcolors += transp_index;

    if (parent)
      iupGetColor(parent, IUP_BGCOLOR, &r, &g, &b);
    else
      iupGetColor(hImage, IUP_BGCOLOR, &r, &g, &b);

    bgcolor = RGB(r,g,b);
    old_bgcolor = RGB(bmpcolors->rgbRed, bmpcolors->rgbGreen, bmpcolors->rgbBlue);

    bmpcolors->rgbBlue = GetBValue(bgcolor); 
    bmpcolors->rgbGreen = GetGValue(bgcolor); 
    bmpcolors->rgbRed = GetRValue(bgcolor); 
  }

  if (IupGetInt(hImage, "_IUPWIN_MAKE_INACTIVE"))
  {
    int i;
    unsigned int bg_r = 0, bg_g = 0, bg_b = 0, 
                 in_r, in_g, in_b;
    RGBQUAD* bmpcolors = (RGBQUAD*)((BYTE*)image->bmpinfo + sizeof(BITMAPINFOHEADER));
    memcpy(old_colors, bmpcolors, sizeof(RGBQUAD)*bmpheader->biClrUsed);

    if (parent)
      iupGetColor(parent, IUP_BGCOLOR, &bg_r, &bg_g, &bg_b);
    else
      iupGetColor(hImage, IUP_BGCOLOR, &bg_r, &bg_g, &bg_b);

    /* replace the non transparent colors by a darker version of the background color. */
    /* replace near white by the background color */
    /* replace similar colors to the background color by itself */

    in_r = bg_r/2; 
    in_g = bg_g/2; 
    in_b = bg_b/2;

    for (i = 0; i < (int)bmpheader->biClrUsed; i++)
    {
      if ((bmpcolors->rgbRed>190 && bmpcolors->rgbGreen>190 && bmpcolors->rgbBlue>190) ||
          ((abs(bmpcolors->rgbRed-bg_r) + abs(bmpcolors->rgbGreen-bg_g) + abs(bmpcolors->rgbBlue-bg_b))) < 15 )
      {
        bmpcolors->rgbRed = bg_r; 
        bmpcolors->rgbGreen = bg_g; 
        bmpcolors->rgbBlue = bg_b; 
      }
      else
      {
        bmpcolors->rgbRed = in_r; 
        bmpcolors->rgbGreen = in_g; 
        bmpcolors->rgbBlue = in_b; 
      }
      bmpcolors++;
    }
  }

  SetDIBits(hDC, hBmp, 0, bmpheader->biHeight, 
            image->bitmap, (BITMAPINFO*)image->bmpinfo, DIB_RGB_COLORS);

  if (IupGetInt(hImage, "_IUPWIN_MAKE_INACTIVE"))
  {
    RGBQUAD* bmpcolors = (RGBQUAD*)((BYTE*)image->bmpinfo + sizeof(BITMAPINFOHEADER));
    memcpy(bmpcolors, old_colors, sizeof(RGBQUAD)*bmpheader->biClrUsed);
  }

  if (transp_index != -1)
  {
    RGBQUAD* bmpcolors = (RGBQUAD*)((BYTE*)image->bmpinfo + sizeof(BITMAPINFOHEADER));
    bmpcolors += transp_index;

    bmpcolors->rgbBlue = GetBValue(old_bgcolor); 
    bmpcolors->rgbGreen = GetGValue(old_bgcolor); 
    bmpcolors->rgbRed = GetRValue(old_bgcolor); 
  }

  return hBmp;
}

static HICON winIconCreateFromImage(Ihandle *himage, HDC hdc, char *type)
{
  HBITMAP hbmColor, hbmMask;
  ICONINFO iconinfo;
  IwinBitmap* image=NULL;
  HICON icon;
   
  hbmColor = iupwinCreateBitmap(himage, hdc, NULL);
  if (!hbmColor) 
    return NULL;

  /* Cria mascara de transparencia */

  image = (IwinBitmap*)handle(himage);
  hbmMask = CreateBitmap(IupGetInt(himage,IUP_WIDTH),
                         IupGetInt(himage,IUP_HEIGHT),1,1,image->bitmask); 
 
  iconinfo.hbmMask = hbmMask;    /* AND */
  iconinfo.hbmColor = hbmColor;  /* XOR */

  if(iupStrEqual(type, IUP_CURSOR))
  {
    char *hotspot = IupGetAttribute(himage, IUP_HOTSPOT);
    int x=0,y=0;

    if(hotspot != NULL)
      iupStrToIntInt(hotspot, &x, &y, ':');

    iconinfo.xHotspot = x;
    iconinfo.yHotspot = y;
    iconinfo.fIcon = FALSE;
  }
  else
    iconinfo.fIcon = TRUE;
  
  icon = CreateIconIndirect(&iconinfo);

  /* Libera recursos */

  if(hbmColor) DeleteObject(hbmColor);
  if(hbmMask)  DeleteObject(hbmMask);
  
  return icon;
}

/********************************************************************
** Creates an icon or cursor from an IupImage. If the type parameter 
** is IUP_CURSOR this function returns an HICON that works as a 
** cursor and can be used as if it were an HCURSOR. If the icon is 
** in the resources, it is created from there. Attention: 
** the IupImage MUST be <= 32x32 or the cursor won't be created.
** parameters: 
** v    -> This string corresponds to the IupImage
** hwnd -> This is used to read the context 
** type -> either IUP_CURSOR or IUP_ICON
********************************************************************/
HICON iupwinGetIcon(char *v, HWND hwnd, char *type)
{
  HICON icon=NULL;
  Ihandle *h=NULL;

  assert(v != NULL);  /* Ponteiro nao pode ser NULL */
  if(v == NULL)
    return NULL;

  /* verifica primeiro se icone existe nos recursos */
  icon = LoadIcon(iupwin_hinstance, (LPCTSTR) v);

  if(icon != NULL)
    return icon;

  /* verifica se icone existe como image */
  h = IupGetHandle(v);
  if(h == NULL)   /* Se nao for image */
    return NULL;
  
  
  /* Tries to read a previously created icon or cursor */
  if(iupStrEqual(type,IUP_CURSOR))
    icon = (HICON) IupGetAttribute(h, "_IUPWIN_HICON_CURSOR");
  else  /* IUP_ICON */
    icon = (HICON) IupGetAttribute(h, "_IUPWIN_HICON_ICON");

  if(icon != NULL)
    return icon;

  /* Tenta criar icone a partir de image */
  {
    HDC hdc = GetDC(hwnd);

    assert(type != NULL);
    if(type == NULL)
      type = IUP_ICON;

    icon = winIconCreateFromImage(IupGetHandle(v), hdc, type);
    
    ReleaseDC(hwnd, hdc);
  }

  /* Stores icon so that it doesn't need to be recreated */
  if(iupStrEqual(type,IUP_CURSOR))
    IupSetAttribute(h, "_IUPWIN_HICON_CURSOR", (char*)icon);
  else  /* IUP_ICON */
    IupSetAttribute(h, "_IUPWIN_HICON_ICON", (char*)icon);

  return icon;
}

HBITMAP iupwinGetBitmap(Ihandle* image, Ihandle* parent)
{
  HBITMAP bmp=NULL;
  HWND hwnd = handle(parent);

  if(!image)
    return NULL;
  
  /* Tries to read a previously created bitmap */
  if (IupGetInt(image, "_IUPWIN_MAKE_INACTIVE"))
    bmp = (HBITMAP) IupGetAttribute(image, "_IUPWIN_HBITMAP_INACTIVE");
  else
    bmp = (HBITMAP) IupGetAttribute(image, "_IUPWIN_HBITMAP");

  if(bmp != NULL)
    return bmp;

  /* Tenta criar um bitmap a partir de image */
  {
    HDC hdc = GetDC(hwnd);
    bmp = iupwinCreateBitmap(image, hdc, parent);
    ReleaseDC(hwnd, hdc);
  }

  /* Stores bitmap */
  if (IupGetInt(image, "_IUPWIN_MAKE_INACTIVE"))
    IupSetAttribute(image, "_IUPWIN_HBITMAP_INACTIVE", (char*) bmp);
  else
    IupSetAttribute(image, "_IUPWIN_HBITMAP", (char*) bmp);

  return bmp;
}
