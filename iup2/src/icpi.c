/** \file
 * \brief Manages IUP control classes
 *
 * See Copyright Notice in iup.h
 * $Id: icpi.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h> 

#include "iglobal.h"
#include "idrv.h"

typedef int      (*itype_setnaturalsize) (Ihandle*);
typedef void     (*itype_setcurrentsize) (Ihandle*, int w, int h);
typedef int      (*itype_getsize)        (Ihandle*, int* w, int* h);
typedef void     (*itype_setposition)    (Ihandle*, int w, int h);
typedef Ihandle* (*itype_create)         (Iclass*, void** params);
typedef void     (*itype_destroy)        (Ihandle*);
typedef void     (*itype_map)            (Ihandle*, Ihandle* parent);
typedef void     (*itype_unmap)          (Ihandle*);
typedef void     (*itype_setattr)        (Ihandle*, const char* attr, const char* value);
typedef char*    (*itype_getattr)        (Ihandle*, const char* attr);
typedef char*    (*itype_getdefaultattr) (Ihandle*, const char* attr);
typedef int      (*itype_popup)          (Ihandle*, int x, int y);

/* Iclass definition */
struct Iclass_ {
   char* name;
   const char* format; /* n: name, s: string, c: interface control */
   itype_setnaturalsize setnaturalsize;
   itype_setcurrentsize setcurrentsize;
   itype_getsize        getsize;
   itype_setposition    setposition;
   itype_create         create;
   itype_destroy        destroy;
   itype_map            map;
   itype_unmap          unmap;
   itype_setattr        setattr;
   itype_getattr        getattr;
   itype_getdefaultattr getdefaultattr;
   itype_popup          popup;
};

static Itable *control_classes = NULL;

#define H0  0x10
#define H1  0x20
#define W1  0x40
#define W0  0x80

#define isW1(n) (usersize(n) & W1)
#define isW0(n) (usersize(n) & W0)
#define isW(n)  (isW1(n) || isW0(n))
#define isH1(n) (usersize(n) & H1)
#define isH0(n) (usersize(n) & H0)
#define isH(n)  (isH1(n) || isH0(n))

/******************************************************************************/
/******************************************************************************/

int iupCpiDefaultSetNaturalSize (Ihandle* self)
{
  return iupCpiGetSize(self, &naturalwidth(self), &naturalheight(self));
}

void iupCpiDefaultSetCurrentSize (Ihandle* self, int w, int h)
{
  currentwidth(self)= isW(self)  ? w : naturalwidth(self);
  currentheight(self)= isH(self) ? h : naturalheight(self);
}

int iupCpiDefaultGetSize (Ihandle* self, int* w, int* h)
{
  return iupdrvCanvasSize(self,w,h);
}

void iupCpiDefaultSetPosition (Ihandle* self, int x, int y)
{
  posx(self) = x;
  posy(self) = y;
}

Ihandle *iupCpiDefaultCreate (Iclass *ic, void** array)
{
  return IupCanvas("");
}

void iupCpiDefaultDestroy (Ihandle* self)
{
  /* empty */
}

void iupCpiDefaultUnmap (Ihandle* self)
{
  /* empty */
}

void iupCpiDefaultMap (Ihandle* self, Ihandle* parent)
{
  if (!handle(self)) iupdrvCreateNativeObject(self);
}

void iupCpiDefaultSetAttr (Ihandle* self, const char* attr, const char* value)
{
  iupdrvSetAttribute(self,attr,value);
}

char* iupCpiDefaultGetAttr (Ihandle* self, const char* attr)
{
  return iupdrvGetAttribute(self,attr);
}

char* iupCpiDefaultGetDefaultAttr (Ihandle* self, const char* attr)
{
  return iupdrvGetDefault(self,attr);
}


/******************************************************************************/
/******************************************************************************/


int iupCpiSetNaturalSize( Ihandle* self )
{
  if (hclass(self))
  {
    if (hclass(self)->setnaturalsize)
      return hclass(self)->setnaturalsize(self);
    else
      return 0;
  }
  else         /* se a funcao for chamada de fora do calcsize */
    return iupSetNaturalSize(self);
}

void iupCpiSetCurrentSize( Ihandle* self, int w, int h )
{
  if (hclass(self))
  {
    if (hclass(self)->setcurrentsize)
      hclass(self)->setcurrentsize(self, w, h);
  }
  else         /* se a funcao for chamada de fora do calcsize */
    iupSetCurrentSize(self,w,h);
}

int iupCpiGetSize( Ihandle* self, int* w, int* h )
{
  if (hclass(self))
  {
    if (hclass(self)->getsize)
      return hclass(self)->getsize(self, w, h);
    else
      return 0;
  }
  else
    return iupdrvCanvasSize(self,w,h);
}

void iupCpiSetPosition ( Ihandle* self, int w, int h )
{
  if (hclass(self))
  {
    if (hclass(self)->setposition)
      hclass(self)->setposition(self, w, h);
  }
  else         /* se a funcao for chamada de fora do calcsize */
    iupSetPosition(self,w,h);
}

void iupCpiDestroy( Ihandle* self )
{
  if(hclass(self))
  {
    if (hclass(self)->destroy)
      hclass(self)->destroy(self);
    hclass(self) = NULL;
  }
}

void iupCpiUnmap( Ihandle* self )
{
  if(hclass(self))
  {
    if (hclass(self)->unmap)
      hclass(self)->unmap(self);
  }
}

void iupCpiMap( Ihandle* self, Ihandle* parent )
{
  /* se self nao tem pai na hierarquia do IUP entao pai na hierarquia */
  /* do IUP e' o mesmo pai usado no mapeamento p/ o sistema nativo */
  if (parent(self) == NULL)
    parent(self) = parent;

  if (hclass(self))
  {
    if (hclass(self)->map)
      hclass(self)->map(self,parent);
  }
  else
    iupdrvCreateObject(self,parent);
}

void iupCpiSetAttribute( Ihandle* self, const char* attr, const char* value )
{
  if (hclass(self))
  {
    if (hclass(self)->setattr)
      hclass(self)->setattr(self,attr,value);
  }
  else
    iupdrvSetAttribute(self,attr,value);
}

char* iupCpiGetAttribute( Ihandle* self, const char* attr )
{
  if (hclass(self))
  {
    if (hclass(self)->getattr)
      return hclass(self)->getattr(self,attr);
    else
      return NULL;
  }
  else
    return iupdrvGetAttribute(self,attr);
}

char* iupCpiGetDefaultAttr( Ihandle* self, const char* attr )
{
  if (hclass(self))
  {
    if (hclass(self)->getdefaultattr)
      return hclass(self)->getdefaultattr(self,attr);
    else
      return NULL;
  }
  else
    return iupdrvGetDefault(self,attr);
}

int iupCpiPopup( Ihandle* self, int x, int y )
{
  if (hclass(self))
  {
    if (hclass(self)->popup)
      return hclass(self)->popup(self,x,y);
    else
      return 0;
  }
  return 0;
}

Ihandle *iupCpiCreate(Iclass *ic, void **params)
{
  Ihandle *h = NULL;
  if (!ic || !ic->create) 
    return NULL;

  h = ic->create(ic, params);

  /* save class inside ihandle */
  if(h)
    h->iclass = ic;

  return h;
}

/*
 * Returns a new class with the default method filled. Registers class with 
 * led and saves class name.
 */
Iclass* iupCpiCreateNewClass(const char *name, const char *format)
{
  Iclass *new_class = NULL;

  assert(name != NULL);
  if(name == NULL)
    return 0;

  if(iupCpiGetClass(name))
    return NULL;

  new_class = (Iclass *) malloc(sizeof(Iclass));

  if(new_class == NULL)
    return NULL;

  new_class->name = (char*)iupStrDup(name);

  new_class->format = format;
  new_class->setnaturalsize = iupCpiDefaultSetNaturalSize;
  new_class->setcurrentsize =  iupCpiDefaultSetCurrentSize;
  new_class->getsize =  iupCpiDefaultGetSize;
  new_class->setposition = iupCpiDefaultSetPosition;
  new_class->create = iupCpiDefaultCreate;
  new_class->destroy = iupCpiDefaultDestroy;
  new_class->map = iupCpiDefaultMap;
  new_class->unmap = iupCpiDefaultUnmap;
  new_class->setattr = iupCpiDefaultSetAttr;
  new_class->getattr = iupCpiDefaultGetAttr;
  new_class->getdefaultattr = iupCpiDefaultGetDefaultAttr;
  new_class->popup = NULL;

  iupLexRegisterLED(name, new_class);
  iupTableSet(control_classes, name, (char*) new_class, IUP_POINTER);

  return new_class;
}

void iupCpiFreeClass(Iclass *a)
{
  assert(a);
  iupTableRemove(control_classes, a->name);
  if(a)
  {
    free(a->name);
    a->name = NULL;
    free(a);
  }
}

/* returns the classe's method */
Imethod iupCpiGetClassMethod(Iclass *ic, const char *method)
{
  assert(ic != NULL);
  if (ic == NULL)
    return NULL;

  assert(method != NULL);
  if (method == NULL)
    return NULL;

  if (iupStrEqual(method,ICPI_SETNATURALSIZE))      return (Imethod)ic->setnaturalsize;
  else if (iupStrEqual(method,ICPI_SETCURRENTSIZE)) return (Imethod)ic->setcurrentsize;
  else if (iupStrEqual(method,ICPI_SETPOSITION))    return (Imethod)ic->setposition;
  else if (iupStrEqual(method,ICPI_CREATE))         return (Imethod)ic->create;
  else if (iupStrEqual(method,ICPI_DESTROY))        return (Imethod)ic->destroy;
  else if (iupStrEqual(method,ICPI_MAP))            return (Imethod)ic->map;
  else if (iupStrEqual(method,ICPI_UNMAP))          return (Imethod)ic->unmap;
  else if (iupStrEqual(method,ICPI_SETATTR))        return (Imethod)ic->setattr;
  else if (iupStrEqual(method,ICPI_GETATTR))        return (Imethod)ic->getattr;
  else if (iupStrEqual(method,ICPI_GETDEFAULTATTR)) return (Imethod)ic->getdefaultattr;
  else if (iupStrEqual(method,ICPI_POPUP))          return (Imethod)ic->popup;
  else if (iupStrEqual(method,ICPI_GETSIZE))        return (Imethod)ic->getsize;
  else return NULL;
}

int iupCpiSetClassMethod(Iclass* iupCpiclass, const char *method, Imethod func)
{
  assert(iupCpiclass != NULL);
  if(iupCpiclass == NULL)
    return IUP_ERROR;

  assert(method != NULL);
  if(method == NULL)
    return IUP_ERROR;

  if(iupStrEqual(method, "SETNATURALSIZE"))
    iupCpiclass->setnaturalsize = (itype_setnaturalsize) func;
  else if(iupStrEqual(method, "SETCURRENTSIZE"))
    iupCpiclass->setcurrentsize = (itype_setcurrentsize) func;
  else if(iupStrEqual(method, "SETPOSITION"))
    iupCpiclass->setposition = (itype_setposition)  func;
  else if(iupStrEqual(method, "CREATE"))
    iupCpiclass->create = (itype_create) func;
  else if(iupStrEqual(method, "DESTROY"))
    iupCpiclass->destroy = (itype_destroy) func;
  else if(iupStrEqual(method, "MAP"))
    iupCpiclass->map = (itype_map) func;
  else if(iupStrEqual(method, "UNMAP"))
    iupCpiclass->unmap = (itype_unmap) func;
  else if(iupStrEqual(method, "SETATTR"))
    iupCpiclass->setattr = (itype_setattr) func;
  else if(iupStrEqual(method, "GETATTR"))
    iupCpiclass->getattr = (itype_getattr) func;
  else if(iupStrEqual(method, "GETDEFAULTATTR"))
    iupCpiclass->getdefaultattr = (itype_getdefaultattr) func;
  else if(iupStrEqual(method, "POPUP"))
    iupCpiclass->popup = (itype_popup) func;
  else if(iupStrEqual(method, "GETSIZE"))
    iupCpiclass->getsize = (itype_getsize) func;
  else
    return IUP_ERROR;

  return IUP_NOERROR;
}

char *iupCpiGetClassName(Iclass *ic)
{
  return ic->name;
}

Iclass* iupCpiGetClass(const char *name)
{
  return iupTableGet(control_classes, name);
}

void iupCpiFinish(void)
{
  char* name = iupTableFirst(control_classes);
  while (name)
  {
    Iclass* a = (Iclass*)iupTableGetCurr(control_classes);
    if (a)
    {
      free(a->name);
      a->name = NULL;
      free(a);
    }

    name=iupTableNext(control_classes);
  }

  iupTableDestroy(control_classes);
  control_classes = NULL;
}

void iupCpiInit(void)
{
  control_classes = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

char *iupCpiGetClassFormat(Iclass *ic)
{
  return (char*)ic->format;
}

/*
* Funcoes de interface com a estrutura interna
* dos elementos IUP
*/

char* IupGetClassName(Ihandle *n)
{
	assert(n);
	if(n==NULL)
		return NULL;
	else if(n->iclass != NULL) /* iupCpi control */
    return iupCpiGetClassName(n->iclass);
  else
    return type(n);
}


void* iupGetImageData(Ihandle* self)
{
  return image_data(self);
}

void* iupGetNativeHandle(Ihandle* self)
{
  return self->handle;
}

void iupSetNativeHandle(Ihandle* self, void* handle)
{
  self->handle = handle;
}

void  iupSetPosX(Ihandle* self, int x)
{
  posx(self) = x;
}

void  iupSetPosY(Ihandle* self, int y)
{
  posy(self) = y;
}

int iupGetPosX(Ihandle* self)
{
  return posx(self);
}

int iupGetPosY(Ihandle* self)
{
  return posy(self);
}

void iupSetCurrentWidth(Ihandle* self, int w)
{
  currentwidth(self) = w;
}

void iupSetCurrentHeight(Ihandle* self, int h)
{
  currentheight(self) = h;
}

int iupGetCurrentWidth(Ihandle* self)
{
  return currentwidth(self);
}

int iupGetCurrentHeight(Ihandle* self)
{
  return currentheight(self);
}

void iupSetNaturalWidth(Ihandle* self, int w)
{
  naturalwidth(self) = w;
}

void iupSetNaturalHeight(Ihandle* self, int h)
{
  naturalheight(self) = h;
}

int iupGetNaturalWidth(Ihandle* self)
{
  return naturalwidth(self);
}

int iupGetNaturalHeight(Ihandle* self)
{
  return naturalheight(self);
}

Ihandle** iupGetParamList(Ihandle* first, va_list arglist)
{
  const int INITIAL_NUMBER = 50;
  Ihandle **params = NULL;
  Ihandle *elem = NULL;
  int max_elements = 0, num_elements = 0;

  params = (Ihandle **) malloc (sizeof (Ihandle *) * INITIAL_NUMBER);

  max_elements = INITIAL_NUMBER;

  elem = first;

  while (elem != NULL)
  {
    params[num_elements] = elem;
    num_elements++;

    /* verifica se precisa realocar memoria */
    if (num_elements >= max_elements)
    {
      Ihandle **new_params = NULL;

      max_elements += INITIAL_NUMBER;

      new_params = (Ihandle **) realloc (params, sizeof (Ihandle *) * max_elements);

      params = new_params;
    }

    elem = va_arg (arglist, Ihandle*);
  }
  params[num_elements] = NULL;

  return params;
}

