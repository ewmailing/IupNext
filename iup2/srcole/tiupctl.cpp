// tIupCtl.cpp: implementation of the tIupCtl class.
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <assert.h>

#include "iup.h"
#include "iupcpi.h"
#include "iglobal.h"

#include "tIupCtl.h"

#define IUP_CONTROL_INTERNAL_DATA "@CONTROL_INTERNAL_DATA"

// stub methods

static void stub_setcurrentsize(Ihandle *self, int w, int h)
{
  tIupCtl *ctl = tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
  {
    iupCpiDefaultSetCurrentSize(self, w, h); /* chama metodo default */
    return;
  }

  ctl->setcurrentsize(w, h);
}

static int stub_setnaturalsize(Ihandle *self)
{
  tIupCtl *ctl = tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
    return iupCpiDefaultSetNaturalSize(self); /* chama metodo default */

  ctl->setnaturalsize();
  return 0;
}

static void stub_setattr(Ihandle *self, 
                         const char *attr, 
                         const char *value)
{
  tIupCtl *ctl = tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
  {
   iupCpiDefaultSetAttr(self, attr, value); /* chama metodo default */
   return;
  }

  ctl->setattr(attr, value);
}

static char *stub_getattr(Ihandle *self, 
                         char *attr 
                         )
{
  tIupCtl *ctl = tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
  {
   iupCpiDefaultGetAttr(self, attr); /* chama metodo default */
   return NULL;
  }

  return ctl->getattr(attr);
}


static void stub_map (Ihandle* self, Ihandle* parent)
{
  tIupCtl *ctl = tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
  {
   iupCpiDefaultMap(self, parent); /* chama metodo default */
   return;
  }

  ctl->map(parent);
}

static void stub_destroy(Ihandle *self)
{
  tIupCtl *ctl = tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
  {
    iupCpiDefaultDestroy(self); /* chama metodo default */
    return;
  }

  delete ctl;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tIupCtl::tIupCtl()
{
  handle = NULL;
}

tIupCtl::~tIupCtl()
{
  if(handle != NULL)
    iupCpiDefaultDestroy(handle);
}

tIupCtl * tIupCtl::GetObjFromIhandle(Ihandle * handle)
{
  return (tIupCtl *) iupGetEnv(handle, IUP_CONTROL_INTERNAL_DATA);
}


void tIupCtl::setattr(const char * attribute, const char * value)
{
  iupCpiDefaultSetAttr(handle, attribute, value);
}

Iclass *tIupCtl::RegisterIupClass(char *classname,
                               char * led_name, 
                               char * args,
                               tCreateFunction creation_function)
{
  Iclass *new_class = iupCpiCreateNewClass(led_name, args);

  iupCpiSetClassMethod(new_class, ICPI_CREATE,   (Imethod) creation_function);
  iupCpiSetClassMethod(new_class, ICPI_DESTROY,  (Imethod) stub_destroy);
  iupCpiSetClassMethod(new_class, ICPI_SETATTR,  (Imethod) stub_setattr);
  iupCpiSetClassMethod(new_class, ICPI_GETATTR,  (Imethod) stub_getattr);
  iupCpiSetClassMethod(new_class, ICPI_MAP,      (Imethod) stub_map);
  iupCpiSetClassMethod(new_class, ICPI_SETNATURALSIZE,  (Imethod) stub_setnaturalsize);
  iupCpiSetClassMethod(new_class, ICPI_SETCURRENTSIZE,  (Imethod) stub_setcurrentsize);

  //IupRegisterLED(led_name, new_class);

  return new_class;
}

Ihandle * tIupCtl::get_ihandle()
{
  return handle;
}

void tIupCtl::initIhandle()
{
  iupSetEnv(handle, IUP_CONTROL_INTERNAL_DATA, (char *) this);  
}

void tIupCtl::map(Ihandle * parent)
{
  iupCpiDefaultMap(handle, parent);
}

char * tIupCtl::getattr(const char * attribute)
{
  return iupCpiDefaultGetAttr(handle, attribute);
}

void tIupCtl::setnaturalsize()
{
  iupCpiDefaultSetNaturalSize(handle);
}

void tIupCtl::setcurrentsize(int w, int h)
{
  iupCpiDefaultSetCurrentSize(handle, w, h);
}
