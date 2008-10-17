/** \file
 * \brief change focus between editable fields
 *
 * See Copyright Notice in iup.h
 * $Id: ifocus.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdio.h>		/* NULL */
#include "iglobal.h"

static Ihandle *iFocusGetField (Ihandle *h);
static Ihandle *iFocusNextField (Ihandle *h);

Ihandle* IupPreviousField (Ihandle *h)
{
 Ihandle *p=NULL;
 Ihandle *c;

 if (h == NULL)
  return NULL;

 for (c=h; parent(c); c=parent(c))
  ;
 for (;;)
 {
  c=iFocusNextField(c);
  if (((c == NULL) || (c == h))&& p)
  {
   IupSetFocus(p);
   return p;
  }
  p=c;
 }
}

Ihandle *IupNextField (Ihandle *h)
{
 Ihandle *nf=iFocusNextField(h);

 if (nf)
 {
  IupSetFocus(nf);
  return nf;
 }
 return NULL;
}

/*
Retorna 1 se o elemento e' um ponto de parada de foco,
Retorna 0 caso contrario.
*/
int iupAcceptFocus(Ihandle *h)
{
  if(type(h) == BUTTON_    || type(h) == CANVAS_    ||
     type(h) == LIST_      || type(h) == TEXT_      ||
     type(h) == MULTILINE_ || type(h) == TOGGLE_ )
  {
     if (!handle(h) || !iupCheck(h,IUP_ACTIVE) ||
                       !iupCheck(h,IUP_VISIBLE))
      return 0;
     else
      return 1;
  }
  return 0;
}

static Ihandle *iFocusNextField (Ihandle *h)
{
 Ihandle *nf;
 if (h == NULL)
  return NULL;

 /* tenta proximo campo com sendo filho de h */
 nf=iFocusGetField(h);

 if (nf && (nf != h))
  return nf;

 for (; parent(h); h=parent(h))
 {
  int oldbrother=0;
  Ihandle *c;
  foreachchild(c,parent(h))		/* para todos os irmaos mais velhos */
   if (c==h)
    oldbrother=1;
   else if (oldbrother && (nf=iFocusGetField(c)))
    return nf;
 }
 if ((nf=iFocusGetField(IupGetDialog(h))) != NULL)
  return nf;

 return NULL;
}

static Ihandle *iFocusGetField (Ihandle *h)
{
  if (h==NULL)
    return NULL;

  if(type(h) == DIALOG_)
    return iFocusGetField(child(h));
  else if(type(h) == ZBOX_ || type(h) == VBOX_ || type(h) == HBOX_)
  {
    Ihandle *c;
    Ihandle *nf;
    foreachchild(c,h)
     if ((nf=iFocusGetField(c)) != NULL)
       return nf;

    return 0;
  }
  else if(type(h) == FRAME_ || type(h) == RADIO_)
   return iFocusGetField(child(h));
  else if(type(h) == BUTTON_    || type(h) == CANVAS_ ||
          type(h) == LIST_      || type(h) == TEXT_   ||
          type(h) == MULTILINE_ || type(h) == TOGGLE_ )
  {
    if (!handle(h) || !iupCheck(h,IUP_ACTIVE) || 
                      !iupCheck(h,IUP_VISIBLE))
      return NULL;
    else
    {
      if (type(h) == TOGGLE_ && iupGetRadio(h))
      {
        if (iupCheck(h, IUP_VALUE)==YES)
          return h;
        else
          return NULL;
      }
      else
        return h;
    }
  }
  else if(type(h) == FILL_      || type(h) == LABEL_   ||
          type(h) == UNKNOWN_   || type(h) == ITEM_    ||
          type(h) == SEPARATOR_ || type(h) == SUBMENU_ ||
          type(h) == KEYACTION_ || type(h) == MENU_ )
  {
    return NULL;
  }
  else
  {
    /* unknown ihandle, lets look if it is native, 
       if not it might be a collection... */
    if(handle(h))
      return h;
    else
    {
      Ihandle *c;
      Ihandle *nf;
      foreachchild(c,h)
       if ((nf=iFocusGetField(c)) != NULL)
         return nf;
    }
  }

  return NULL;
}
