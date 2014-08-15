/** \file
 * \brief iuptree control
 * Functions used to edit a node name in place.
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include <iup.h>
#include <iupcpi.h>
#include <iupcompat.h>

#include "treedef.h"
#include "itedit.h"
#include "itcallback.h"

#include "iglobal.h"


static int treeTextActionCb(Ihandle* self, int c, char *after)
{
  TtreePtr tree=(TtreePtr)tree_data(self);
  if (iupCheck(tree_texth(tree), "VISIBLE")!=YES)
    return IUP_DEFAULT;

  if (c == K_ESC)
  {
    IupSetAttribute(tree_texth(tree),IUP_VISIBLE,IUP_NO);
    IupSetAttribute(tree_texth(tree),IUP_ACTIVE,IUP_NO);
    IupSetFocus(tree_self(tree));
    return IUP_DEFAULT;
  }

  if (c == K_CR)
  {
    int ret;
    iupSetEnv(tree_self(tree), "_IUPTREE_CALL_RENAME", "1");
    ret = treecallRenameCb(tree_self(tree), after);
    iupSetEnv(tree_self(tree), "_IUPTREE_CALL_RENAME", NULL);

    if (ret == IUP_IGNORE)
      return IUP_DEFAULT;

    IupSetAttribute(tree_texth(tree),IUP_VISIBLE,IUP_NO);
    IupSetAttribute(tree_texth(tree),IUP_ACTIVE,IUP_NO);
    IupSetFocus(tree_self(tree));
    return IUP_DEFAULT;
  }

  return IUP_DEFAULT;
}

#ifdef _MOTIF_
static int treeKeyAnyTextCb(Ihandle *h, int c)
{
  int ret = IUP_DEFAULT;

  /* In motif, these keys are not handled in IupText ACTION. */
  if (c == K_ESC || c == K_CR)
    ret = treeTextActionCb(h, c, IupGetAttribute(h,IUP_VALUE));

  return ret;
}
#endif

/* called whenever something happend that 
   the edit control should be closed if visible */
void treeEditCheckHidden(Ihandle* self)
{
  TtreePtr tree=(TtreePtr)tree_data(self);

  if (iupCheck(tree_texth(tree), "VISIBLE")==YES)
  {
    char* value;

    /* if the user caused a kill focus during a RENAME_CB just ignore it. */
    if (IupGetInt(tree_self(tree), "_IUPTREE_CALL_RENAME")==1)
      return;

    value = iupStrDup(IupGetAttribute(tree_texth(tree), IUP_VALUE));
    treecallRenameCb(tree_self(tree), value);
    free(value);
    IupSetAttribute(tree_texth(tree),IUP_VISIBLE,IUP_NO);
    IupSetAttribute(tree_texth(tree),IUP_ACTIVE,IUP_NO);
  }
}

static int treeTextKillFocusCb(Ihandle* self)
{
#ifdef _MOTIF_
  if(IupGetInt(self, "_IUPTREE_DOUBLE_CLICK"))
  {
    IupSetAttribute(self, "_IUPTREE_DOUBLE_CLICK", NULL);
    return IUP_DEFAULT;
  }
#endif

  treeEditCheckHidden(self);
  return IUP_DEFAULT;
}

void treeEditShow(Ihandle* self, int text_x, int x, int y)
{
  static char s[30];
  TtreePtr tree=(TtreePtr)tree_data(self);
  int w, h;
  int charwidth,charheight;

  /* active */
  if (!iupCheck(self,IUP_ACTIVE))
    return;

  /* notify application */
  if (treecallShowRenameCb(self) == IUP_IGNORE)
    return;

  /* calc size */
  x = x + NODE_WIDTH;
	y = y - TEXT_BOX_OFFSET_Y - 1;
	w = 2*TEXT_MARGIN_X-TEXT_RIGHT_FIX + text_x+2+15 + 15;
  h = NODE_HEIGHT+2;
  y = YmaxCanvas(tree) - y - NODE_Y;

  if (x + w > XmaxCanvas(tree))
    w = XmaxCanvas(tree) - x;

  /* set attributes */
  IupSetAttribute(tree_texth(tree), IUP_VALUE, IupGetAttribute(self, IUP_NAME));
  IupSetAttribute(tree_texth(tree), IUP_FONT, IupGetAttribute(self, IUP_FONT));

  /* set position */
  iupSetPosX(tree_texth(tree),x);
  iupSetPosY(tree_texth(tree),y);

  /* set size */
  IupSetfAttribute(tree_texth(tree), "RASTERSIZE", "%dx%d", w, h);

  iupdrvGetCharSize(self,&charwidth, &charheight);
  sprintf (s, "%dx%d", 4*w/charwidth, 8*h/charheight);
  iupSetEnv(tree_texth(tree), IUP_SIZE, s);
  iupSetCurrentWidth(tree_texth(tree), w);
  iupSetCurrentHeight(tree_texth(tree), h);
  iupdrvResizeObjects(tree_texth(tree));

  /* activate and show */
  IupSetAttribute(tree_texth(tree),IUP_ACTIVE,IUP_YES);
  IupSetAttribute(tree_texth(tree),IUP_VISIBLE,IUP_YES);
  IupSetFocus(tree_texth(tree));

  /* update text attributes */
  {
    char* selection = iupGetEnv(self, IUP_SELECTION);
    if (selection)
    {
      /* this allow the user to set the SELECTION inside the SHOWRENAME_CB */
      IupStoreAttribute(tree_texth(tree),IUP_SELECTION,selection);
      iupSetEnv(self, IUP_SELECTION, NULL);
    }
    else
    {
      char* caret = iupGetEnv(self, IUP_CARET);
      if (caret)
      {
        /* this allow the user to set the CARET inside the SHOWRENAME_CB */
        IupStoreAttribute(tree_texth(tree),IUP_CARET,caret);
        iupSetEnv(self, IUP_CARET, NULL);
      }
    }
  }
}

void treeEditCreate(TtreePtr tree)
{
   tree_texth(tree) = IupText(NULL);
   IupSetCallback(tree_texth(tree),"ACTION",(Icallback)treeTextActionCb);
   IupSetCallback(tree_texth(tree),IUP_KILLFOCUS_CB ,(Icallback)treeTextKillFocusCb);
   iupSetEnv(tree_texth(tree),IUP_VALUE, "");
   iupSetEnv(tree_texth(tree),IUP_BORDER,IUP_YES);

#ifdef _MOTIF_
   IupSetCallback(tree_texth(tree),IUP_K_ANY,(Icallback)treeKeyAnyTextCb);
#else
   /* avoid callback inheritance */
   iupSetEnv(tree_texth(tree),IUP_K_ANY,"__do_nothing__");
#endif
}

void treeEditMap(TtreePtr tree, Ihandle* self)
{
  iupCpiSetPosition(tree_texth(tree),iupGetPosX(self),iupGetPosY(self));
  iupCpiMap(tree_texth(tree),self);

  IupSetAttribute(tree_texth(tree),IUP_VISIBLE,IUP_NO);
  IupSetAttribute(tree_texth(tree),IUP_ACTIVE,IUP_NO);
}
