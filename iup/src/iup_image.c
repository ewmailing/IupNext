/** \file
 * \brief Image Resource.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_image.h"
#include "iup_assert.h"
#include "iup_stdcontrols.h"


typedef struct _IimageStock
{
  iupImageStockCreateFunc func;
  Ihandle* image;            /* cache image */
  const char* native_name;   /* used to map to GTK stock images */
} IimageStock;

static Itable *istock_table = NULL;   /* the function hast table indexed by the name string */

void iupImageStockInit(void)
{
  istock_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupImageStockFinish(void)
{
  char* name = iupTableFirst(istock_table);
  while (name)
  {
    IimageStock* istock = (IimageStock*)iupTableGetCurr(istock_table);
    if (iupObjectCheck(istock->image))
      IupDestroy(istock->image);
    free(istock);
    name = iupTableNext(istock_table);
  }

  iupTableDestroy(istock_table);
  istock_table = NULL;
}

void iupImageStockSet(const char *name, iupImageStockCreateFunc func, const char* native_name)
{
  IimageStock* istock = (IimageStock*)iupTableGet(istock_table, name);
  if (istock)
    free(istock);  /* overwrite a previous registration */

  istock = (IimageStock*)malloc(sizeof(IimageStock));
  istock->func = func;
  istock->image = NULL;
  istock->native_name = native_name;

  iupTableSet(istock_table, name, (void*)istock, IUPTABLE_POINTER);
}

static void iImageStockGet(const char* name, Ihandle* *ih, const char* *native_name)
{
  IimageStock* istock = (IimageStock*)iupTableGet(istock_table, name);
  if (istock)
  {
    if (istock->image)
      *ih = istock->image;
    else if (istock->native_name)
        *native_name = istock->native_name;
    else if (istock->func)
    {
      istock->image = istock->func();
      *ih = istock->image;
    }
  }
}

void iupImageStockLoad(const char *name)
{
  const char* native_name = NULL;
  Ihandle* ih = NULL;
  iImageStockGet(name, &ih, &native_name);
  if (ih)
    IupSetHandle(name, ih);
  else if (native_name)
  {
    /* dummy image to save the GTK stock name */
    void* image = iupdrvImageLoad(native_name, IUPIMAGE_IMAGE);
    if (image)
    {
      int w, h, bpp;
      iupdrvImageGetInfo(image, &w, &h, &bpp);
      if (bpp == 32)
        ih = IupImageRGBA(w,h,NULL);
      else
        ih = IupImageRGB(w,h,NULL);
      IupSetHandle(native_name, ih);
    }
  }
}


/**************************************************************************************************/
/**************************************************************************************************/


static void iupColorSet(iupColor *c, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  c->r = r;
  c->g = g;
  c->b = b;
  c->a = a;
}

int iupImageInitColorTable(Ihandle *ih, iupColor* colors, int *colors_count)
{
  char attr[6], *value;
  unsigned char red, green, blue;
  int i, has_alpha = 0;
  static iupColor default_colors[] = {
    { 0,0,0,255 }, { 128,0,0,255 }, { 0,128,0,255 }, { 128,128,0,255 },    
    { 0,0,128,255 }, { 128,0,128,255 }, { 0,128,128,255 }, { 192,192,192,255 },    
    { 128,128,128,255 }, { 255,0,0,255 }, { 0,255,0,255 }, { 255,255,0,255 },
    { 0,0,255,255 }, { 255,0,255,255 }, { 0,255,255,255 }, { 255,255,255,255 } };

  memset(colors, 0, sizeof(iupColor)*256);

  for (i=0;i<16;i++)
  {
    sprintf(attr, "%d", i);
    value = iupAttribGet(ih, attr);

    if (value)
    {
      if (iupStrEqual(value, "BGCOLOR"))
      {
        iupColorSet(&colors[i], 0, 0, 0, 0);
        has_alpha = 1;
      }
      else
      {
        if (!iupStrToRGB(value, &red, &green, &blue))
          iupColorSet(&colors[i], default_colors[i].r, default_colors[i].g, default_colors[i].b, 255);
        else
          iupColorSet(&colors[i], red, green, blue, 255);
      }
    }
    else
    {
      iupColorSet(&colors[i], default_colors[i].r, default_colors[i].g, default_colors[i].b, 255);
    }
  }

  for (;i<256;i++)
  {
    sprintf(attr, "%d", i);
    value = iupAttribGet(ih, attr);
    if (!value)
      break;

    if (iupStrEqual(value, "BGCOLOR"))
    {
      iupColorSet(&colors[i], 0, 0, 0, 0);
      has_alpha = 1;
    }
    else
    {
      if (!iupStrToRGB(value, &red, &green, &blue))
        break;

      iupColorSet(&colors[i], red, green, blue, 255);
    }
  }

  if (colors_count) *colors_count = i;

  return has_alpha;
}

void iupImageColorMakeInactive(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char bg_r, unsigned char bg_g, unsigned char bg_b)
{
  if (*r==bg_r && *g==bg_g && *b==bg_b)  /* preserve colors identical to the background color */
  {
    *r = bg_r; 
    *g = bg_g; 
    *b = bg_b; 
  }
  else
  {
    int ir, ig, ib, 
      i = (*r+*g+*b)/3,
      bg_i = (bg_r+bg_g+bg_b)/3;

    ir = (bg_r*i)/bg_i; 
    ig = (bg_g*i)/bg_i; 
    ib = (bg_b*i)/bg_i; 

#define LIGHTER(_c) ((255 + _c)/2)
    ir = LIGHTER(ir);
    ig = LIGHTER(ig);
    ib = LIGHTER(ib);

    *r = iupBYTECROP(ir);
    *g = iupBYTECROP(ig);
    *b = iupBYTECROP(ib);
  }
}


/**************************************************************************************************/
/**************************************************************************************************/


void* iupImageGetIcon(const char* name)
{
  void* icon;
  Ihandle *ih;

  if (!name)
    return NULL;

  /* Check first in the system resources. */
  icon = iupdrvImageLoad(name, IUPIMAGE_ICON);
  if (icon) 
    return icon;

  /* get handle from name */
  ih = IupGetHandle(name);
  if (!ih)
    return NULL;
  
  /* Check for an already created icon */
  icon = iupAttribGet(ih, "_IUPIMAGE_ICON");
  if (icon)
    return icon;

  /* Not created, tries to create the icon */
  icon = iupdrvImageCreateIcon(ih);

  /* save the pixbuf */
  iupAttribSetStr(ih, "_IUPIMAGE_ICON", (char*)icon);

  return icon;
}

void* iupImageGetCursor(const char* name)
{
  void* cursor;
  Ihandle *ih;

  if (!name)
    return NULL;

  /* Check first in the system resources. */
  cursor = iupdrvImageLoad(name, IUPIMAGE_CURSOR);
  if (cursor) 
    return cursor;

  /* get handle from name */
  ih = IupGetHandle(name);
  if (!ih)
    return NULL;
  
  /* Check for an already created cursor */
  cursor = iupAttribGet(ih, "_IUPIMAGE_CURSOR");
  if (cursor)
    return cursor;

  /* Not created, tries to create the cursor */
  cursor = iupdrvImageCreateCursor(ih);

  /* save the pixbuf */
  iupAttribSetStr(ih, "_IUPIMAGE_CURSOR", (char*)cursor);

  return cursor;
}

void iupImageGetInfo(const char* name, int *w, int *h, int *bpp)
{
  void* image;
  Ihandle *ih;

  if (!name)
    return;

  /* Check first in the system resources. */
  image = iupdrvImageLoad(name, IUPIMAGE_IMAGE);
  if (image)
  {
    iupdrvImageGetInfo(image, w, h, bpp);
    return;
  }

  /* get handle from name */
  ih = IupGetHandle(name);
  if (!ih)
  {
    /* Check in the stock images. */
    const char* native_name = NULL;
    iImageStockGet(name, &ih, &native_name);

    if (native_name) 
    {
      image = iupdrvImageLoad(native_name, IUPIMAGE_IMAGE);
      if (image) 
      {
        iupdrvImageGetInfo(image, w, h, bpp);
        return;
      }
    }

    if (!ih)
      return;
  }

  if (w) *w = ih->currentwidth;
  if (h) *h = ih->currentheight;
  if (bpp) *bpp = IupGetInt(ih, "BPP");
}

void* iupImageGetImage(const char* name, Ihandle* ih_parent, int make_inactive, const char* attrib_name)
{
  char cache_name[100] = "_IUPIMAGE_";
  char* bgcolor;
  void* image;
  Ihandle *ih;

  if (!name)
    return NULL;

  /* Check first in the system resources. */
  image = iupdrvImageLoad(name, IUPIMAGE_IMAGE);
  if (image) 
    return image;

  /* get handle from name */
  ih = IupGetHandle(name);
  if (!ih)
  {
    /* Check in the stock images. */
    const char* native_name = NULL;
    iImageStockGet(name, &ih, &native_name);

    if (native_name) 
    {
      image = iupdrvImageLoad(native_name, IUPIMAGE_IMAGE);
      if (image) 
        return image;
    }

    if (!ih)
      return NULL;
  }

  bgcolor = iupAttribGet(ih, "BGCOLOR");
  if (!bgcolor && ih_parent)  
    bgcolor = IupGetAttribute(ih_parent, "BGCOLOR"); /* Use IupGetAttribute to use inheritance and native implementation */

  strcat(cache_name, attrib_name);

  if (iupAttribGet(ih, "_IUP_BGCOLOR_DEPEND") && bgcolor)
    strcat(cache_name, bgcolor);
  
  /* Check for an already created native image */
  image = (void*)iupAttribGet(ih, cache_name);
  if (image)
    return image;

  if (iupAttribGetStr(ih_parent, "FLAT_ALPHA"))
    iupAttribSetStr(ih, "FLAT_ALPHA", "1");

  /* Creates the native image */
  image = iupdrvImageCreateImage(ih, bgcolor, make_inactive);

  if (iupAttribGetStr(ih_parent, "FLAT_ALPHA"))
    iupAttribSetStr(ih, "FLAT_ALPHA", NULL);

  if (iupAttribGet(ih, "_IUP_BGCOLOR_DEPEND") && bgcolor)
    strcat(cache_name, bgcolor);

  /* save the native image in the cache */
  iupAttribSetStr(ih, cache_name, (char*)image);

  return image;
}

void iupImageUpdateParent(Ihandle *ih)  /* ih here is the element that contains images */
{
  int inherit;

  /* Called when BGCOLOR is changed */
  /* it will re-create the image, if the case */

  char* value = iupAttribGet(ih, "IMAGE");
  if (value) 
    iupClassObjectSetAttribute(ih, "IMAGE", value, &inherit);

  value = iupAttribGet(ih, "IMINACTIVE");
  if (value) 
    iupClassObjectSetAttribute(ih, "IMINACTIVE", value, &inherit);

  value = iupAttribGet(ih, "IMPRESS");
  if (value) 
    iupClassObjectSetAttribute(ih, "IMPRESS", value, &inherit);
}

static char* iImageGetWidthAttrib(Ihandle *ih)
{
  char* str = iupStrGetMemory(50);
  sprintf(str, "%d", ih->currentwidth);
  return str;
}

static char* iImageGetHeightAttrib(Ihandle *ih)
{
  char* str = iupStrGetMemory(50);
  sprintf(str, "%d", ih->currentheight);
  return str;
}

static void iImageUnMapMethod(Ihandle* ih)
{
  char *name;
  void* native_data;

  native_data = iupAttribGet(ih, "_IUPIMAGE_ICON");
  if (native_data) 
  {
    iupdrvImageDestroy(native_data, IUPIMAGE_ICON);
    iupAttribSetStr(ih, "_IUPIMAGE_ICON", NULL);
  }

  native_data = iupAttribGet(ih, "_IUPIMAGE_CURSOR");
  if (native_data) 
  {
    iupdrvImageDestroy(native_data, IUPIMAGE_CURSOR);
    iupAttribSetStr(ih, "_IUPIMAGE_CURSOR", NULL);
  }

  name = iupTableFirst(ih->attrib);
  while (name)
  {
    if (iupStrEqualPartial(name, "_IUPIMAGE_"))
    {
      native_data = iupTableGetCurr(ih->attrib);
      if (native_data) iupdrvImageDestroy(native_data, IUPIMAGE_IMAGE);
    }

    name = iupTableNext(ih->attrib);
  }
}

static int iImageCreate(Ihandle* ih, void** params, int bpp)
{
  int width, height, channels, count;
  unsigned char *imgdata;

  iupASSERT(params!=NULL);
  if (!params)
    return IUP_ERROR;

  width = (int)(params[0]);
  height = (int)(params[1]);

  iupASSERT(width>0);
  iupASSERT(height>0);

  if (width <= 0 || height <= 0)
    return IUP_ERROR;

  ih->currentwidth = width;
  ih->currentheight = height;

  channels = 1;
  if (bpp == 24)
    channels = 3;
  else if (bpp == 32)
    channels = 4;

  count = width*height*channels;
  imgdata = (unsigned char *)malloc(count);

  if (((int)(params[2])==-1) || ((int)(params[3])==-1)) /* compacted in one pointer */
  {
    if (imgdata && (int)(params[2])!=-1)
      memcpy(imgdata, params[2], count);
  }
  else /* one param for each element */
  {
    int i;
    for(i=0; i<count; i++)
    {
      imgdata[i] = (unsigned char)((int)(params[i+2]));
    }
  }

  ih->handle = (InativeHandle*)imgdata;  /* IupImage is always mapped */

  iupAttribSetInt(ih, "BPP", bpp);
  iupAttribSetInt(ih, "CHANNELS", channels);

  return IUP_NOERROR;
}

static int iImageCreateMethod(Ihandle* ih, void** params)
{
  return iImageCreate(ih, params, 8);
}

static int iImageRGBCreateMethod(Ihandle* ih, void** params)
{
  return iImageCreate(ih, params, 24);
}

static int iImageRGBACreateMethod(Ihandle* ih, void** params)
{
  return iImageCreate(ih, params, 32);
}

static void iImageDestroyMethod(Ihandle* ih)
{
  free(ih->handle);
}

/******************************************************************************/

Ihandle* IupImage(int width, int height, const unsigned char *imgdata)
{
  void *params[4];
  params[0] = (void*)width;
  params[1] = (void*)height;
  params[2] = imgdata? (void*)imgdata: (void*)-1;
  params[3] = (void*)-1;
  return IupCreatev("image", params);
}

Ihandle* IupImageRGB(int width, int height, const unsigned char *imgdata)
{
  void *params[4];
  params[0] = (void*)width;
  params[1] = (void*)height;
  params[2] = imgdata? (void*)imgdata: (void*)-1;
  params[3] = (void*)-1;
  return IupCreatev("imagergb", params);
}

Ihandle* IupImageRGBA(int width, int height, const unsigned char *imgdata)
{
  void *params[4];
  params[0] = (void*)width;
  params[1] = (void*)height;
  params[2] = imgdata? (void*)imgdata: (void*)-1;
  params[3] = (void*)-1;
  return IupCreatev("imagergba", params);
}

static Iclass* iImageGetClassBase(char* name, int (*create_func)(Ihandle* ih, void** params))
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = name;
  ic->format = "iiC"; /* (int,int,unsigned char*) */
  ic->nativetype = IUP_TYPEIMAGE;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = create_func;
  ic->Destroy = iImageDestroyMethod;
  ic->UnMap = iImageUnMapMethod;

  /* Attribute functions */
  iupClassRegisterAttribute(ic, "WID", iupBaseGetWidAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "WIDTH", iImageGetWidthAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HEIGHT", iImageGetHeightAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RASTERSIZE", iupBaseGetRasterSizeAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

Iclass* iupImageGetClass(void)
{
  return iImageGetClassBase("image", iImageCreateMethod);
}

Iclass* iupImageRGBGetClass(void)
{
  return iImageGetClassBase("imagergb", iImageRGBCreateMethod);
}

Iclass* iupImageRGBAGetClass(void)
{
  return iImageGetClassBase("imagergba", iImageRGBACreateMethod);
}
