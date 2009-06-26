/** \file
 * \brief Image Resource.
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_image.h"

#include "iupwin_drv.h"

/* RGB in RGBA DIBs are pre-multiplied by alpha to AlphaBlend usage. */
#define iupALPHAPRE(_src, _alpha) (((_src)*(_alpha))/255)

static int winImageInitDibColors(iupColor* colors, RGBQUAD* bmpcolors, int colors_count, 
                                  unsigned char bg_r, unsigned char bg_g, unsigned char bg_b, int make_inactive)
{
  int i, ret = 0;
  for (i=0;i<colors_count;i++)
  {
    bmpcolors[i].rgbRed   = colors[i].r;
    bmpcolors[i].rgbGreen = colors[i].g;
    bmpcolors[i].rgbBlue  = colors[i].b;
    bmpcolors[i].rgbReserved = colors[i].a;

    if (bmpcolors[i].rgbReserved == 0) /* full transparent alpha */
    {
      bmpcolors[i].rgbBlue = bg_b; 
      bmpcolors[i].rgbGreen = bg_g; 
      bmpcolors[i].rgbRed = bg_r;
      ret = 1;
    }
    else
      bmpcolors[i].rgbReserved = 0; /* clear non transparent mark */

    if (make_inactive)
      iupImageColorMakeInactive(&(bmpcolors[i].rgbRed), &(bmpcolors[i].rgbGreen), &(bmpcolors[i].rgbBlue), 
                                bg_r, bg_g, bg_b);
  }

  return ret;
}

static HBITMAP winImageCreateBitmap(Ihandle *ih, int width, int height, int bpp, BYTE** bitmap, 
                                    unsigned char bg_r, unsigned char bg_g, unsigned char bg_b, int make_inactive)
{
  HDC hDC;
  int colors_count;
  HBITMAP hBitmap;
  BITMAPINFOHEADER* bitmap_info;  /* bitmap info header */
  iupColor colors[256];

  if (bpp == 32 || bpp == 24)
    colors_count = 0;
  else /* bpp == 8 */
    iupImageInitColorTable(ih, colors, &colors_count);

  bitmap_info = malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*colors_count);
  if (!bitmap_info)
    return NULL;

  memset(bitmap_info, 0, sizeof(BITMAPINFOHEADER));
  bitmap_info->biSize = sizeof(BITMAPINFOHEADER);
  bitmap_info->biWidth  = width;
  bitmap_info->biHeight = height;
  bitmap_info->biPlanes = 1; /* not the same as PLANES */
  bitmap_info->biBitCount = (WORD)bpp;
  bitmap_info->biCompression = BI_RGB;
  bitmap_info->biClrUsed = colors_count;

  if (colors_count)
  {
    /* since colors are only passed to the CreateDIBSection here, must update BGCOLOR and inactive here */
    RGBQUAD* bitmap_colors = (RGBQUAD*)(((BYTE*)bitmap_info) + sizeof(BITMAPINFOHEADER));
    if (winImageInitDibColors(colors, bitmap_colors, colors_count, bg_r, bg_g, bg_b, make_inactive))
      iupAttribSetStr(ih, "_IUP_BGCOLOR_DEPEND", "1");
  }

  hDC = GetDC(NULL);
  hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)bitmap_info, DIB_RGB_COLORS, (void**)bitmap, NULL, 0x0);
  ReleaseDC(NULL, hDC);
  free(bitmap_info);

  return hBitmap;
}

void* iupdrvImageCreateImage(Ihandle *ih, const char* bgcolor, int make_inactive)
{
  unsigned char bg_r = 0, bg_g = 0, bg_b = 0;
  int y,x,bmp_line_size,data_line_size,
      width = ih->currentwidth,
      height = ih->currentheight,
      channels = iupAttribGetInt(ih, "CHANNELS"),
      flat_alpha = iupAttribGetInt(ih, "FLAT_ALPHA"),
      bpp = iupAttribGetInt(ih, "BPP");
  unsigned char *imgdata = (unsigned char*)iupAttribGetStr(ih, "WID");
  HBITMAP hBitmap;
  BYTE* bitmap;   /* DIB bitmap bits, created in CreateDIBSection and filled here */

  iupStrToRGB(bgcolor, &bg_r, &bg_g, &bg_b);

  hBitmap = winImageCreateBitmap(ih, width, height, bpp, &bitmap, bg_r, bg_g, bg_b, make_inactive);
  if (!hBitmap)
    return NULL;

  bmp_line_size = ((width * bpp + 31) / 32) * 4;    /* DWORD aligned, 4 bytes boundary in a N bpp image */
  data_line_size = width*channels;

  /* windows bitmaps are bottom up */
  imgdata += (height-1)*data_line_size;  /* iupimage is top down */

  for (y=0; y<height; y++)
  {
    for (x=0; x<width; x++)
    {
      if (bpp != 8) /* (bpp == 32 || bpp == 24) */
      {
        int offset = channels*x;
        /* Windows Bitmap order is BGRA */
        BYTE *b = &bitmap[offset],
             *g = b+1,
             *r = g+1,
             *a = r+1,
             *dat = &imgdata[offset];

        *r = *(dat);
        *g = *(dat+1);
        *b = *(dat+2);

        if (channels == 4)  /* bpp==32 */
        {
          if (flat_alpha)
          {
            *a = *(dat+3);
            *r = iupALPHABLEND(*r, bg_r, *a);
            *g = iupALPHABLEND(*g, bg_g, *a);
            *b = iupALPHABLEND(*b, bg_b, *a);
            *a = 255;
          }

          if (make_inactive)
            iupImageColorMakeInactive(r, g, b, bg_r, bg_g, bg_b);

          if (!flat_alpha)
          {
            *a = *(dat+3);

            /* RGB in RGBA DIBs are pre-multiplied by alpha to AlphaBlend usage. */
            *r = iupALPHAPRE(*r,*a);
            *g = iupALPHAPRE(*g,*a);
            *b = iupALPHAPRE(*b,*a);
          }
        }
      }
      else /* bpp == 8 */
      {
        bitmap[x] = imgdata[x];
      }
    }

    bitmap += bmp_line_size;
    imgdata -= data_line_size;    /* iupimage is top down */
  }

  if (make_inactive || (channels == 4 && flat_alpha))
    iupAttribSetStr(ih, "_IUP_BGCOLOR_DEPEND", "1");

  return hBitmap;
}

static HBITMAP winImageCreateBitmask(Ihandle *ih)
{
  int y, x, mask_line_size,data_line_size, colors_count, set,
      width = ih->currentwidth,
      height = ih->currentheight,
      channels = iupAttribGetInt(ih, "CHANNELS"),
      bpp = iupAttribGetInt(ih, "BPP");
  unsigned char *imgdata = (unsigned char*)iupAttribGetStr(ih, "WID");
  HBITMAP hBitmap;
  BYTE* bitmask, *bitmask_ptr;
  iupColor colors[256];

  if (bpp == 8)
    iupImageInitColorTable(ih, colors, &colors_count);

  mask_line_size = ((width * 1 + 15) / 16) * 2;      /* WORD aligned, 2 bytes boundary in a 1 bpp image */
  data_line_size = width*channels;

  bitmask = malloc(height * mask_line_size);
  memset(bitmask, 0, height * mask_line_size); /* opaque */

  /* mask and iupimage are top down */

  bitmask_ptr = bitmask;
  for (y=0; y<height; y++)
  {
    for (x=0; x<width; x++)
    {
      set = 0;

      if (bpp == 32)
      {
        BYTE* dat = &imgdata[channels*x];
        if (*(dat+3) == 0) /* full transparent alpha */
          set = 1;
      }
      else if (bpp == 8)
      {
        unsigned char index = imgdata[x];
        if (colors[index].a == 0) /* full transparent alpha */
          set = 1;
      }

      if (set)
        bitmask_ptr[x/8] |= 1 << (7 - (x % 8)); /* set transparent mask bit */
    }

    bitmask_ptr += mask_line_size;
    imgdata += data_line_size;
  }

  hBitmap = CreateBitmap(width, height, 1, 1, bitmask);
  free(bitmask);
  return hBitmap;
}

static HICON winImageCreateIcon(Ihandle *ih, int is_cursor)
{
  HBITMAP hBitmap, hBitmapMask;
  ICONINFO iconinfo;
  HICON icon;
  char* color0 = NULL;

  /* If cursor and no transparency defined, assume 0 if transparent. 
     We do this only in Windows and because of backward compatibility. */
  if (is_cursor)
  {
    int bpp = iupAttribGetInt(ih, "BPP");
    if (bpp == 8)
    {
      if (!iupStrEqual(iupAttribGet(ih, "0"), "BGCOLOR") &&
          !iupStrEqual(iupAttribGet(ih, "1"), "BGCOLOR") &&
          !iupStrEqual(iupAttribGet(ih, "2"), "BGCOLOR"))
      {
        color0 = iupStrDup(iupAttribGet(ih, "0"));
        iupAttribSetStr(ih, "0", "BGCOLOR");
      }
    }
  }
   
  hBitmap = iupdrvImageCreateImage(ih, NULL, 0);
  if (!hBitmap) 
    return NULL;

  /* Transparency mask */
  hBitmapMask = winImageCreateBitmask(ih);
  if (!hBitmapMask)
  {
    DeleteObject(hBitmap);
    return NULL;
  }
 
  /* destination = (destination AND bitmask) XOR bitmap */
  iconinfo.hbmMask = hBitmapMask;   /* AND */
  iconinfo.hbmColor = hBitmap;      /* XOR */

  if (is_cursor)
  {
    int x=0,y=0;
    iupStrToIntInt(iupAttribGet(ih, "HOTSPOT"), &x, &y, ':');

    iconinfo.xHotspot = x;
    iconinfo.yHotspot = y;
    iconinfo.fIcon = FALSE;
  }
  else
    iconinfo.fIcon = TRUE;
  
  icon = CreateIconIndirect(&iconinfo);

  DeleteObject(hBitmap);
  DeleteObject(hBitmapMask);

  if (color0)
  {
    iupAttribStoreStr(ih, "0", color0);
    free(color0);
  }
  
  return icon;
}

void* iupdrvImageCreateIcon(Ihandle *ih)
{
  return winImageCreateIcon(ih, 0);
}

void* iupdrvImageCreateCursor(Ihandle *ih)
{
  return winImageCreateIcon(ih, 1);
}

void* iupdrvImageCreateMask(Ihandle *ih)
{
  return (void*)winImageCreateBitmask(ih);
}

void* iupdrvImageLoad(const char* name, int type)
{
  int iup2win[3] = {IMAGE_BITMAP, IMAGE_ICON, IMAGE_CURSOR};
  HANDLE hImage = LoadImage(iupwin_hinstance, (LPCTSTR)name, iup2win[type], 0, 0, type==0?LR_CREATEDIBSECTION:0);
  if (!hImage && iupwin_dll_hinstance)
    hImage = LoadImage(iupwin_dll_hinstance, (LPCTSTR)name, iup2win[type], 0, 0, type==0?LR_CREATEDIBSECTION:0);
  if (!hImage)
    hImage = LoadImage(NULL, (LPCTSTR)name, iup2win[type], 0, 0, LR_LOADFROMFILE|(type==0?LR_CREATEDIBSECTION:0));
  return hImage;
}

static void winGetBitmapInfo(HDC hDC, HBITMAP hBitmap, int *w, int *h, int *bpp)
{
  BITMAPINFO info;
  ZeroMemory(&info, sizeof(BITMAPINFO));
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

  GetDIBits(hDC, hBitmap, 0, 0, NULL, &info, DIB_RGB_COLORS);

  if (w) *w = info.bmiHeader.biWidth;
  if (h) *h = abs(info.bmiHeader.biHeight);
  if (bpp) *bpp = info.bmiHeader.biBitCount;
}

void iupdrvImageGetInfo(void* image, int *w, int *h, int *bpp)
{
  HBITMAP hBitmap = (HBITMAP)image;
  HDC hDC = GetDC(NULL);
  SelectObject(hDC, hBitmap);
  winGetBitmapInfo(hDC, hBitmap, w, h, bpp);
  ReleaseDC(NULL, hDC);
}

void iupdrvImageDestroy(void* image, int type)
{
  switch (type)
  {
  case IUPIMAGE_IMAGE:
    DeleteObject((HBITMAP)image);
    break;
  case IUPIMAGE_ICON:
    DestroyIcon((HICON)image);
    break;
  case IUPIMAGE_CURSOR:
    DestroyCursor((HCURSOR)image);
    break;
  }
}
