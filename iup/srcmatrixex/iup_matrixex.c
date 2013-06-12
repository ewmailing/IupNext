/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
//#include <limits.h>
//#include <stdarg.h>
#include <string.h>


#include "iup.h"
#include "iupcontrols.h"
#include "iupmatrixex.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_str.h"


static int iMatrixExSetFreezeAttrib(Ihandle *ih, const char* value)
{
  int freeze, lin, col;

  if (iupStrBoolean(value))
  {
    IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);
    freeze = 1;
  }
  else
  {
    if (iupStrToIntInt(value, &lin, &col, ':')==2)
      freeze = 1;
    else
      freeze = 0;
  }

  if (!freeze)
  {
    IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", IupGetInt(ih,"NUMLIN_NOSCROLL"), -1, NULL);
    IupSetAttributeId2(ih,"FRAMEVERTCOLOR", -1, IupGetInt(ih,"NUMCOL_NOSCROLL"), NULL);

    IupSetAttribute(ih,"NUMLIN_NOSCROLL","0");
    IupSetAttribute(ih,"NUMCOL_NOSCROLL","0");
  }
  else
  {
    char* fzcolor = IupGetAttribute(ih, "FREEZECOLOR");

    IupSetfAttribute(ih,"NUMLIN_NOSCROLL","%d",lin);
    IupSetfAttribute(ih,"NUMCOL_NOSCROLL","%d",col);  

    IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", lin, -1, fzcolor);
    IupSetAttributeId2(ih,"FRAMEVERTCOLOR",-1, col, fzcolor);
  }

  IupUpdate(ih);
  return 1;  /* store freeze state */
}

static int iMatrixExCreateMethod(Ihandle* ih, void **params)
{
  (void)params;
  return IUP_NOERROR;
}

static void iMatrixExInitAttribCb(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "FREEZE", NULL, iMatrixExSetFreezeAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FREEZECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "0 0 255", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
}

static Iclass* iMatrixExNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("matrix"));

  ic->name = "matrixex";
  ic->format = "";
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 2;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->New = iMatrixExNewClass;
  ic->Create  = iMatrixExCreateMethod;
  
  iMatrixExInitAttribCb(ic);

  return ic;
}

void IupMatrixExInit(Ihandle* ih)
{
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "matrix"))
    return;
    
  iMatrixExInitAttribCb(ih->iclass);
}

void IupMatrixExOpen(void)
{
  if (!IupGetGlobal("_IUP_MATRIXEX_OPEN"))
  {
    iupRegisterClass(iMatrixExNewClass());
    IupSetGlobal("_IUP_MATRIXEX_OPEN", "1");
  }
}

Ihandle* IupMatrixEx(void)
{
  return IupCreate("matrixex");
}
