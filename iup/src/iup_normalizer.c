/** \file
 * \brief Normalizer Element.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"


enum {NORMALIZE_NONE, NORMALIZE_WIDTH, NORMALIZE_HEIGHT};

static int iNormalizeGetNormalizeSize(const char* value)
{
  if (!value)
    return NORMALIZE_NONE;
  if (iupStrEqualNoCase(value, "HORIZONTAL"))
    return NORMALIZE_WIDTH;
  if (iupStrEqualNoCase(value, "VERTICAL"))
    return NORMALIZE_HEIGHT;
  if (iupStrEqualNoCase(value, "BOTH"))
    return NORMALIZE_WIDTH|NORMALIZE_HEIGHT;
  return NORMALIZE_NONE;
}

void iupNormalizeSizeBoxChild(Ihandle *ih, int children_natural_maxwidth, int children_natural_maxheight)
{
  Ihandle* child;
  int normalize = iNormalizeGetNormalizeSize(iupAttribGetStr(ih, "NORMALIZESIZE"));
  if (!normalize)
    return;

  /* is called after the natural size is calculated */

  /* reset the natural width and/or height */
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating && (child->iclass->nativetype != IUP_TYPEVOID || !iupStrEqual(child->iclass->name, "fill")))
    {
      if (normalize & NORMALIZE_WIDTH) 
        child->naturalwidth = children_natural_maxwidth;
      if (normalize & NORMALIZE_HEIGHT)
        child->naturalheight = children_natural_maxheight;
    }
  }
}

static int iNormalizerSetNormalizeAttributesAttrib(Ihandle* ih, const char* value)
{
  Ihandle* child;
  int natural_maxwidth = 0, natural_maxheight = 0;
  int normalize = iNormalizeGetNormalizeSize(value);
  if (!normalize)
    return 0;

  for (child = ih->firstchild; child; child = child->brother)
  {
    iupClassObjectComputeNaturalSize(child);
    natural_maxwidth = iupMAX(natural_maxwidth, child->naturalwidth);
    natural_maxheight = iupMAX(natural_maxheight, child->naturalheight);
  }

  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating && (child->iclass->nativetype != IUP_TYPEVOID || !iupStrEqual(child->iclass->name, "fill")))
    {
      if (normalize & NORMALIZE_WIDTH)
        child->userwidth = natural_maxwidth;
      if (normalize & NORMALIZE_HEIGHT)
        child->userheight = natural_maxheight;
    }
  }
  return 0;
}

static int iNormalizerCreateMethod(Ihandle* ih, void** params)
{
  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    while (*iparams) 
    {
      IupAppend(ih, *iparams);
      iparams++;
    }
  }

  return IUP_NOERROR;
}

static int iNormalizerMapMethod(Ihandle* ih)
{
  ih->handle = (InativeHandle*)-1; /* fake value just to indicate that it is already mapped */
  return IUP_NOERROR;
}

Iclass* iupNormalizerGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "normalizer";
  ic->format = "g"; /* array of Ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDMANY;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = iNormalizerCreateMethod;
  ic->Map = iNormalizerMapMethod;

  iupClassRegisterAttribute(ic, "NORMALIZE", NULL, iNormalizerSetNormalizeAttributesAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}

Ihandle *IupNormalizerv(Ihandle **children)
{
  return IupCreatev("normalizer", (void**)children);
}

Ihandle *IupNormalizer(Ihandle* child, ...)
{
  Ihandle **children;
  Ihandle *ih;

  va_list arglist;
  va_start(arglist, child);
  children = (Ihandle **)iupObjectGetParamList(child, arglist);
  va_end(arglist);

  ih = IupCreatev("normalizer", (void**)children);
  free(children);

  return ih;
}
