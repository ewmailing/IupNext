/** \file
 * \brief IupLayoutDialog pre-defined dialog
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_focus.h"
#include "iup_dlglist.h"
#include "iup_assert.h"
#include "iup_drvdraw.h"
#include "iup_draw.h"
#include "iup_image.h"
#include "iup_childtree.h"
#include "iup_dialog.h"
#include "iup_drv.h"
#include "iup_func.h"
#include "iup_register.h"
#include "iup_layout.h"


enum {
  FIND_TYPE = 0,
  FIND_HANDLE = 1,
  FIND_NAME = 2,
  FIND_TITLE = 3,
  FIND_ATTRIBUTE = 4
};

typedef struct _iLayoutDialog {
  int destroy;  /* destroy the selected dialog, when the layout dialog is destroyed */
  int changed;
  Ihandle *dialog;  /* the selected dialog */
  Ihandle *tree, *status, *timer;
  Ihandle *properties;  /* elements from the layout dialog */
  Ihandle *globals;
  Ihandle *copy;
} iLayoutDialog;

static int iLayoutFindItemMatch(Ihandle *ih, const char *str, int searchType)
{
  if (searchType == FIND_TYPE)
    return iupStrEqualNoCase(IupGetClassName(ih), str);
  else if (searchType == FIND_TITLE)
  {
    char* title = iupAttribGetLocal(ih, "TITLE");
    return title ? iupStrEqualNoCase(title, str) : 0;
  }
  else if (searchType == FIND_ATTRIBUTE)
  {
    char att[80], val[80];
    char *attVal;
    iupStrToStrStr(str, att, val, '=');
    attVal = iupAttribGetLocal(ih, att);
    if (attVal == NULL)
      return 0;
    if (iupStrEqualNoCase(val, attVal))
      return 1;
  }

  return 0;
}

static Ihandle* iLayoutFindNode(Ihandle* tree, const char *str, int start_id, int end_id, int searchType)
{
  Ihandle *elem;
  int id;

  for (id = start_id; id <= end_id; id++)
  {
    elem = (Ihandle*)IupTreeGetUserId(tree, id);

    if (!elem)  /* for the vled tree */
      continue;

    if (iLayoutFindItemMatch(elem, str, searchType))
      return elem;
  }

  return NULL;
}

static int iLayoutFindDialogNext_CB(Ihandle* ih)
{
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(ih, "FIND_DIALOG");
  Ihandle* dialog = (Ihandle*)IupGetAttribute(find_dlg, "DIALOG");
  Ihandle* tree = (Ihandle*)IupGetAttribute(find_dlg, "TREE");
  Ihandle* lbl_result = IupGetDialogChild(find_dlg, "FIND_RESULT");

  Ihandle* target = IupGetDialogChild(find_dlg, "TARGET");
  Ihandle* targetValue = IupGetAttributeHandle(target, "VALUE");
  int searchType = IupGetInt(targetValue, "SEARCH_TYPE");

  char* str = IupGetAttribute(IupGetDialogChild(find_dlg, "FIND_TEXT"), "VALUE");

  int last_id = IupGetInt(tree, "VALUE");

  Ihandle* obj = NULL;

  if (searchType == FIND_HANDLE)
    obj = IupGetHandle(str);
  else if (searchType == FIND_NAME)
    obj = IupGetDialogChild(dialog, str);
  else 
  {
    int count = IupGetInt(tree, "COUNT");
    obj = iLayoutFindNode(tree, str, last_id + 1, count - 1, searchType);
    if (!obj)
      obj = iLayoutFindNode(tree, str, 0, last_id, searchType);
  }

  if (obj)
  {
    int id = IupTreeGetId(tree, obj);
    IFnii cb = (IFnii)IupGetCallback(tree, "SELECTION_CB");
    cb(tree, last_id, 0);
    IupSetInt(tree, "VALUE", id);
    cb(tree, id, 1);

    IupSetAttribute(lbl_result, "TITLE", "");
  }
  else
    IupSetAttribute(lbl_result, "TITLE", "Not Found!");

  return IUP_DEFAULT;
}

static int iLayoutFindDialogClose_CB(Ihandle* ih)
{
  Ihandle* find_dlg = IupGetDialog(ih);
  IupHide(find_dlg);  /* do not destroy, just hide */
  return IUP_DEFAULT;
}

IUP_SDK_API Ihandle* iupLayoutFindElementDialog(Ihandle *tree, Ihandle* elem)
{
  Ihandle *txt, *box, *find_dlg;
  Ihandle *type, *handle_name, *name, *title, *attribute, *radio;
  Ihandle *bt_next, *bt_close, *lbl_result;
  Ihandle *dialog = IupGetDialog(elem);

  txt = IupText(NULL);
  IupSetAttribute(txt, "NAME", "FIND_TEXT");
  IupSetAttribute(txt, "VISIBLECOLUMNS", "20");
  IupSetAttribute(txt, "EXPAND", "HORIZONTAL");

  type = IupToggle("Type", NULL);
  IupSetAttribute(type, "NAME", "FIND_TYPE");
  IupSetInt(type, "SEARCH_TYPE", 0);
  IupSetAttribute(type, "TIP", "Element Type (Class Name)");

  handle_name = IupToggle("Handle", NULL);
  IupSetAttribute(handle_name, "NAME", "FIND_HANDLE");
  IupSetInt(handle_name, "SEARCH_TYPE", 1);
  IupSetAttribute(handle_name, "TIP", "Handle Name");

  name = IupToggle("Name", NULL);
  IupSetAttribute(name, "NAME", "FIND_NAME");
  IupSetInt(name, "SEARCH_TYPE", 2);
  IupSetAttribute(name, "TIP", "NAME attribute");

  title = IupToggle("Title", NULL);
  IupSetAttribute(title, "NAME", "FIND_TITLE");
  IupSetInt(title, "SEARCH_TYPE", 3);
  IupSetAttribute(title, "TIP", "TITLE attribute");

  attribute = IupToggle("Atribute", NULL);
  IupSetAttribute(attribute, "NAME", "FIND_ATTRIBUTE");
  IupSetInt(attribute, "SEARCH_TYPE", 4);
  IupSetAttribute(attribute, "TIP", "Attribute=Value");

  bt_next = IupButton("Find Next", NULL);
  IupSetStrAttribute(bt_next, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(bt_next, "ACTION", (Icallback)iLayoutFindDialogNext_CB);
  bt_close = IupButton("Close", NULL);
  IupSetCallback(bt_close, "ACTION", (Icallback)iLayoutFindDialogClose_CB);
  IupSetStrAttribute(bt_close, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));

  lbl_result = IupLabel(NULL);
  IupSetAttribute(lbl_result, "SIZE", "30");
  IupSetAttribute(lbl_result, "NAME", "FIND_RESULT");

  radio = IupRadio(IupVbox(
    type,
    handle_name,
    name,
    title,
    attribute,
    NULL));

  IupSetAttribute(radio, "NAME", "TARGET");
  IupSetAttribute(radio, "VALUE_HANDLE", (char*)type);
  IupSetAttribute(radio, "MARGIN", "10x10");

  box = IupVbox(
    IupLabel("Find What:"),
    txt,
    IupHbox(
      IupSetAttributes(IupFrame(radio), "TITLE=\"What Kind:\""),
      IupFill(),
      IupSetAttributes(IupVbox(
        bt_next,
        bt_close,
        lbl_result,
        NULL), "NORMALIZESIZE=HORIZONTAL, MARGIN=x20"),
      NULL),
    NULL);
  IupSetAttribute(box, "NMARGIN", "10x10");
  IupSetAttribute(box, "GAP", "5");

  find_dlg = IupDialog(box);
  IupSetAttribute(find_dlg, "TITLE", "Find in Layout");
  IupSetAttribute(find_dlg, "DIALOGFRAME", "Yes");
  IupSetAttributeHandle(find_dlg, "DEFAULTENTER", bt_next);
  IupSetAttributeHandle(find_dlg, "DEFAULTESC", bt_close);
  IupSetAttributeHandle(find_dlg, "PARENTDIALOG", IupGetDialog(tree));

  /* Save the multiline to access it from the callbacks */
  IupSetAttribute(find_dlg, "TREE", (char*)tree);

  /* Save the dialog to reuse it */
  IupSetAttribute(find_dlg, "FIND_DIALOG", (char*)find_dlg);  /* from itself */
  IupSetAttribute(find_dlg, "DIALOG", (char*)dialog); /* from the main dialog, use to find NAME */

  return find_dlg;
}

IUP_SDK_API char* iupLayoutGetElementTitle(Ihandle* ih)
{
  char* title = iupAttribGetLocal(ih, "TITLE");
  char* name = IupGetName(ih);
  char* str = iupStrGetMemory(200);
  if (title)
  {
    char buffer[51];

    if (iupStrLineCount(title, (int)strlen(title)) > 1)
    {
      int len;
      iupStrNextLine(title, &len); /* get the size of the first line */
      if (len > 50) len = 50;
      iupStrCopyN(buffer, len + 1, title);
      title = &buffer[0];
    }

    if (name)
      sprintf(str, "[%s] \"%.50s\" (%.50s)", IupGetClassName(ih), title, name);
    else
      sprintf(str, "[%s] \"%.50s\"", IupGetClassName(ih), title);
  }
  else
  {
    if (name)
      sprintf(str, "[%s] (%.50s)", IupGetClassName(ih), name);
    else
      sprintf(str, "[%s]", IupGetClassName(ih));
  }
  return str;
}

static void iLayoutRemoveExt(char* title, const char* ext)
{
  int len = (int)strlen(title);
  int len_ext = (int)strlen(ext);
  if (len_ext == 1)
  {
    if (tolower(title[len - 1]) == ext[0] &&
        title[len - 2] == '.')
        title[len - 2] = 0; /* place terminator at dot */
  }
  else
  {
    if (tolower(title[len - 1]) == ext[2] &&
        tolower(title[len - 2]) == ext[1] &&
        tolower(title[len - 3]) == ext[0] &&
        title[len - 4] == '.')
        title[len - 4] = 0; /* place terminator at dot */
  }
}

static int iLayoutHasDigit(const char* name)
{
  while (*name)
  {
    if (isdigit(*name))
      return 1;
    name++;
  }
  return 0;
}

static int iLayoutCompareStr(const void *a, const void *b)
{
  int ret;
  const char* str1 = *(const char**)a;
  const char* str2 = *(const char**)b;
  int flat1 = 0, flat2 = 0;
  if (iupStrEqualPartial(str1, "flat")) { str1 += 4; flat1 = 1; } /* don't consider "flat" prefix */
  if (iupStrEqualPartial(str2, "flat")) { str2 += 4; flat2 = 1; }
  ret = strcmp(str1, str2);
  if (ret == 0)
  {
    if (flat1)
      return 1;
    if (flat2)
      return -1;
    return 0;
  }
  return ret;
}

IUP_SDK_API int iupLayoutAttributeChanged(Ihandle* ih, const char* name, const char* value, const char* def_value, int flags)
{
  if ((flags&IUPAF_NO_STRING) || /* not a string */
      (flags&IUPAF_HAS_ID) ||  /* has id */
      (flags&(IUPAF_READONLY | IUPAF_WRITEONLY)))  /* can only read or only write */
      return 0;

  if (!value || value[0] == 0 || iupATTRIB_ISINTERNAL(value))
    return 0;

  if ((flags&IUPAF_NO_SAVE) && iupBaseNoSaveCheck(ih, name))  /* can not be saved */
    return 0;

  if (def_value && iupStrEqualNoCase(def_value, value))  /* equal to the default value */
    return 0;

  if (!def_value && iupStrFalse(value))  /* default=NULL and value=NO */
    return 0;

  if (!(flags&IUPAF_NO_INHERIT) && ih->parent) /* if inherit, check if the same value is defined at parent */
  {
    char* parent_value = iupAttribGetInherit(ih->parent, name);
    if (parent_value && iupStrEqualNoCase(value, parent_value))
      return 0;
  }

  return 1;
}

static char* iLayoutGetName(Ihandle* ih)
{
  char* name = IupGetName(ih);
  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;
  if (!name && ih->iclass->nativetype == IUP_TYPEDIALOG)
    name = iupAttribGet(ih, "_IUP_DIALOG_NAME");
  return name;
}


/***************************************************************************
                          Tree Utilities
***************************************************************************/


static void iLayoutTreeSetNodeColor(Ihandle* tree, int id, Ihandle* ih)
{
  if (ih->handle != NULL)
    IupSetAttributeId(tree, "COLOR", id, "0 0 0");
  else
    IupSetAttributeId(tree, "COLOR", id, "128 128 128");
}

static void iLayoutTreeSetNodeInfo(Ihandle* tree, int id, Ihandle* ih)
{
  IupSetAttributeId(tree, "TITLE", id, iupLayoutGetElementTitle(ih));
  iLayoutTreeSetNodeColor(tree, id, ih);
  IupTreeSetUserId(tree, id, ih);
}

static Ihandle* iLayoutTreeGetFirstChild(Ihandle* ih)
{
  Ihandle* firstchild = ih->parent->firstchild;

  while (firstchild && firstchild->flags & IUP_INTERNAL)
    firstchild = firstchild->brother;

  return firstchild;
}

static int iLayoutTreeAddNode(Ihandle* tree, int id, Ihandle* ih)
{
  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    if (!ih->parent || ih == iLayoutTreeGetFirstChild(ih))
    {
      IupSetAttributeId(tree, "ADDBRANCH", id, "");
      id++;
    }
    else
    {
      IupSetAttributeId(tree, "INSERTBRANCH", id, "");
      id = IupGetInt(tree, "LASTADDNODE");
    }
  }
  else
  {
    if (!ih->parent || ih == iLayoutTreeGetFirstChild(ih))
    {
      IupSetAttributeId(tree, "ADDLEAF", id, "");
      id++;
    }
    else
    {
      IupSetAttributeId(tree, "INSERTLEAF", id, "");
      id = IupGetInt(tree, "LASTADDNODE");
    }
  }

  iLayoutTreeSetNodeInfo(tree, id, ih);
  return id;
}

static void iLayoutTreeAddChildren(Ihandle* tree, int parent_id, Ihandle* parent)
{
  Ihandle *child;
  int last_child_id = parent_id;

  for (child = parent->firstchild; child; child = child->brother)
  {
    if (!(child->flags & IUP_INTERNAL))
    {
      last_child_id = iLayoutTreeAddNode(tree, last_child_id, child);

      if (child->iclass->childtype != IUP_CHILDNONE)
        iLayoutTreeAddChildren(tree, last_child_id, child);
    }
  }
}

static void iLayoutUpdateLayout(iLayoutDialog* layoutdlg)
{
  Ihandle* canvas = IupGetBrother(layoutdlg->tree);
  int w = 0, h = 0;

  IupRefresh(layoutdlg->dialog);

  IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
  IupSetInt(canvas, "XMAX", w);
  IupSetInt(canvas, "YMAX", h);

  IupGetIntInt(canvas, "DRAWSIZE", &w, &h);
  IupSetInt(canvas, "DX", w);
  IupSetInt(canvas, "DY", h);

  /* redraw canvas */
  IupUpdate(canvas);
}

static void iLayoutUpdateMark(iLayoutDialog* layoutdlg, Ihandle* ih, int id);

static void iLayoutTreeRebuild(iLayoutDialog* layoutdlg)
{
  Ihandle* tree = layoutdlg->tree;
  IupSetAttribute(tree, "DELNODE0", "CHILDREN");

  layoutdlg->changed = 0;
  layoutdlg->copy = NULL;

  iLayoutTreeSetNodeInfo(tree, 0, layoutdlg->dialog);
  iLayoutTreeAddChildren(tree, 0, layoutdlg->dialog);

  IupSetAttribute(tree, "VALUE", "ROOT");
  iLayoutUpdateMark(layoutdlg, layoutdlg->dialog, 0);

  iLayoutUpdateLayout(layoutdlg);
}


/***************************************************************************
                         Layout Export
***************************************************************************/


static void iLayoutExportCountContainersRec(Ihandle* ih, int *index)
{
  Ihandle *child;
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (child->iclass->childtype != IUP_CHILDNONE)
    {
      if (!(child->flags & IUP_INTERNAL))
      {
        (*index)++;
        iupAttribSetInt(child, "_IUP_CONTAINER_INDEX", *index);
        iLayoutExportCountContainersRec(child, index);
      }
    }
  }
}

static int iLayoutExportCountContainers(Ihandle* dialog)
{
  int index = 0;
  iupAttribSetInt(dialog, "_IUP_CONTAINER_INDEX", index);
  iLayoutExportCountContainersRec(dialog, &index);
  return index + 1;
}

static void iLayoutExportWriteAttrib(FILE* file, const char* name, const char* value, const char* indent, int type)
{
  char attribname[1024];
  if (type == 1)  /* Lua */
  {
    iupStrLower(attribname, name);
    if (iLayoutHasDigit(attribname))
      fprintf(file, "%s[\"%s\"] = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
  }
  else if (type == -1) /* LED */
  {
    iupStrUpper(attribname, name);
    if (iupStrHasSpace(value))
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = %s,\n", indent, attribname, value);
  }
  else
    fprintf(file, "%s\"%s\", \"%s\",\n", indent, name, value);
}

static int iLayoutExportElementAttribs(FILE* file, Ihandle* ih, const char* indent, int type)
{
  int i, wcount = 0, attr_count, has_attrib_id = ih->iclass->has_attrib_id, start_id = 0,
    total_count = IupGetClassAttributes(ih->iclass->name, NULL, 0);
  char **attr_names = (char **)malloc(total_count * sizeof(char *));

  if (IupClassMatch(ih, "tree") || /* tree can only set id attributes after map, so they can not be saved */
      IupClassMatch(ih, "cells"))  /* cells does not have any savable id attributes */
      has_attrib_id = 0;

  if (IupClassMatch(ih, "list") || IupClassMatch(ih, "flatlist"))
    start_id = 1;

  attr_count = IupGetClassAttributes(ih->iclass->name, attr_names, total_count);
  for (i = 0; i < attr_count; i++)
  {
    char *name = attr_names[i];
    char* value = iupAttribGetLocal(ih, name);
    char* def_value;
    int flags;

    iupClassGetAttribNameInfo(ih->iclass, name, &def_value, &flags);

    if (iupLayoutAttributeChanged(ih, name, value, def_value, flags))
    {
      char* str = iupStrConvertToC(value);

      iLayoutExportWriteAttrib(file, name, str, indent, type);

      if (str != value)
        free(str);

      wcount++;
    }

    if (has_attrib_id && flags&IUPAF_HAS_ID)
    {
      flags &= ~IUPAF_HAS_ID; /* clear flag so the next function call can work */
      if (iupLayoutAttributeChanged(ih, name, "X", NULL, flags))
      {
        if (iupStrEqual(name, "IDVALUE"))
          name = "";

        if (flags&IUPAF_HAS_ID2)
        {
          int lin, col,
            numcol = IupGetInt(ih, "NUMCOL") + 1,
            numlin = IupGetInt(ih, "NUMLIN") + 1;
          for (lin = 0; lin < numlin; lin++)
          {
            for (col = 0; col < numcol; col++)
            {
              value = IupGetAttributeId2(ih, name, lin, col);
              if (value && value[0] && !iupATTRIB_ISINTERNAL(value))
              {
                char str[50];
                sprintf(str, "%s%d:%d", name, lin, col);
                iLayoutExportWriteAttrib(file, str, value, indent, type);
                wcount++;
              }
            }
          }
        }
        else
        {
          int id, count = IupGetInt(ih, "COUNT");
          for (id = start_id; id < count + start_id; id++)
          {
            value = IupGetAttributeId(ih, name, id);
            if (value && value[0] && !iupATTRIB_ISINTERNAL(value))
            {
              char str[50];
              sprintf(str, "%s%d", name, id);
              iLayoutExportWriteAttrib(file, str, value, indent, type);
              wcount++;
            }
          }
        }
      }
    }
  }

  if (type != 0)  /* LED or C */
  {
    int cb_count = total_count - attr_count;
    IupGetClassCallbacks(ih->iclass->name, attr_names, cb_count);
    for (i = 0; i < cb_count; i++)
    {
      char* cb_name = iupGetCallbackName(ih, attr_names[i]);
      if (cb_name && cb_name[0] && !iupATTRIB_ISINTERNAL(cb_name))
      {
        iLayoutExportWriteAttrib(file, attr_names[i], cb_name, indent, type);
        wcount++;
      }
    }
  }

  if (type == -1) /* LED */
  {
    /* remove last comma ',' and new line */
    /* if wcount==0, it will remove '[' and new line */
    fseek(file, -2, SEEK_CUR);
  }

  free(attr_names);
  return wcount;
}

static void iLayoutExportElementC(FILE* file, Ihandle* ih)
{
  char* name = IupGetName(ih);
  char* indent = "    ";
  if (ih->iclass->childtype == IUP_CHILDNONE)
    indent = "        ";
  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;

  if (ih->iclass->childtype == IUP_CHILDNONE)
    fprintf(file, "      IupSetAtt(%s%s%s, IupCreate(\"%s\"), \n", name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", ih->iclass->name);
  else
  {
    Ihandle *child;

    fprintf(file, "  containers[%s] = IupSetAtt(%s%s%s, IupCreatep(\"%s\", \n", iupAttribGet(ih, "_IUP_CONTAINER_INDEX"), name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", ih->iclass->name);

    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!(child->flags & IUP_INTERNAL))
      {
        if (child->iclass->childtype == IUP_CHILDNONE)
          iLayoutExportElementC(file, child);  /* only one level of recursion */
        else
        {
          fprintf(file, "      containers[%s],\n", iupAttribGet(child, "_IUP_CONTAINER_INDEX"));
          iupAttribSet(child, "_IUP_CONTAINER_INDEX", NULL);  /* clear when last used */
        }
      }
    }

    fprintf(file, "      NULL),\n"); /* end of IupCreatep */
  }

  iLayoutExportElementAttribs(file, ih, indent, 0);  /* C */

  /* end of IupSetAtt */
  if (ih->iclass->childtype != IUP_CHILDNONE)
    fprintf(file, "    NULL);\n\n");
  else
    fprintf(file, "        NULL),\n");
}

static void iLayoutExportContainerC(FILE* file, Ihandle* ih)
{
  Ihandle *child;
  /* export children first */
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!(child->flags & IUP_INTERNAL) && child->iclass->childtype != IUP_CHILDNONE)
      iLayoutExportContainerC(file, child);
  }

  iLayoutExportElementC(file, ih);
}

static void iLayoutExportDialogC(FILE* file, Ihandle* dialog, const char* filename)
{
  int count = iLayoutExportCountContainers(dialog);
  char* title = iupStrFileGetTitle(filename);
  iLayoutRemoveExt(title, "c");

  fprintf(file, "/*   Generated by IupLayoutDialog export to C.   */\n\n");
  fprintf(file, "#include <stdlib.h>\n");
  fprintf(file, "#include <iup.h>\n\n");
  fprintf(file, "Ihandle* create_dialog_%s(void)\n", title);
  fprintf(file, "{\n");
  fprintf(file, "  Ihandle* containers[%d];\n\n", count);

  iLayoutExportContainerC(file, dialog);
  iupAttribSet(dialog, "_IUP_CONTAINER_INDEX", NULL);  /* clear when last used */

  fprintf(file, "  return containers[0];\n");
  fprintf(file, "}\n");
  free(title);
}

static void iLayoutExportElementLua(FILE* file, Ihandle* ih)
{
  char* indent = "    ";

  if (ih->iclass->childtype == IUP_CHILDNONE)
  {
    indent = "      ";
    fprintf(file, "    iup.%s{\n", ih->iclass->name);
  }
  else
  {
    Ihandle *child;

    fprintf(file, "  containers[%d] = iup.%s{\n", iupAttribGetInt(ih, "_IUP_CONTAINER_INDEX") + 1, ih->iclass->name);

    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!(child->flags & IUP_INTERNAL))
      {
        if (child->iclass->childtype == IUP_CHILDNONE)
          iLayoutExportElementLua(file, child);  /* only one level of recursion */
        else
        {
          fprintf(file, "    containers[%d],\n", iupAttribGetInt(child, "_IUP_CONTAINER_INDEX") + 1);
          iupAttribSet(child, "_IUP_CONTAINER_INDEX", NULL);  /* clear when last used */
        }
      }
    }
  }

  iLayoutExportElementAttribs(file, ih, indent, 1);  /* Lua */

  if (ih->iclass->childtype != IUP_CHILDNONE)
    fprintf(file, "  }\n\n");
  else
    fprintf(file, "    },\n");
}

static void iLayoutExportContainerLua(FILE* file, Ihandle* ih)
{
  Ihandle *child;
  /* export children first */
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!(child->flags & IUP_INTERNAL) && child->iclass->childtype != IUP_CHILDNONE)
      iLayoutExportContainerLua(file, child);
  }

  iLayoutExportElementLua(file, ih);
}

static void iLayoutExportDialogLua(FILE* file, Ihandle* dialog, const char* filename)
{
  char* title = iupStrFileGetTitle(filename);
  iLayoutRemoveExt(title, "lua");
  iLayoutExportCountContainers(dialog);

  fprintf(file, "--   Generated by IupLayoutDialog export to Lua.\n\n");
  fprintf(file, "function create_dialog_%s()\n", title);
  fprintf(file, "  local containers = {}\n\n");

  iLayoutExportContainerLua(file, dialog);
  iupAttribSet(dialog, "_IUP_CONTAINER_INDEX", NULL);  /* clear when last used */

  fprintf(file, "  return containers[1]\n");
  fprintf(file, "end\n");
  free(title);
}

static void iLayoutExportElementLED(FILE* file, Ihandle* ih, const char* name, int indent_level)
{
  int i, count, indent_count = 0;
  const char* format = ih->iclass->format;
  char classname[100];
  char indent[300] = "";

  /* constructor indentation */
  if (indent_level)
  {
    indent_count = indent_level * 4;
    for (i = 0; i < indent_count; i++)
      indent[i] = ' ';
  }

  iupStrUpper(classname, ih->iclass->name);
  if (name)
    fprintf(file, "%s = %s[\n", name, classname);  /* start of attributes */
  else
    fprintf(file, "%s%s[\n", indent, classname);

  /* attributes indentation */
  for (i = indent_count; i < indent_count + 2; i++)
    indent[i] = ' ';
  indent_count += 2;

  if (iLayoutExportElementAttribs(file, ih, indent, -1) != 0)  /* LED */
    fprintf(file, "]"); /* end of attributes (no new line) */

  if (!format)
    fprintf(file, "()");
  else
  {
    if (*format == 'h' || *format == 'g')
    {
      Ihandle *child;

      fprintf(file, "(\n");

      /* children indentation */
      for (i = indent_count; i < indent_count + 2; i++)
        indent[i] = ' ';
      indent_count += 2;

      for (child = ih->firstchild; child; child = child->brother)
      {
        if (!(child->flags & IUP_INTERNAL))
        {
          char* childname = iLayoutGetName(child);
          if (!childname)
            iLayoutExportElementLED(file, child, NULL, indent_level + 1);   /* here process the ones that does NOT have names */
          else
            fprintf(file, "%s%s", indent, childname);

          if (child->brother)
            fprintf(file, ",\n");
        }
      }

      fprintf(file, ")");
    }
    else
    {
      count = (int)strlen(format);

      fprintf(file, "(");

      for (i = 0; i < count; i++)
      {
        if (format[i] == 's')
          fprintf(file, "\"\"");  /* empty string, let the job to the attributes */
        else if (format[i] == 'a')
        {
          char* cb_name = iupGetCallbackName(ih, "ACTION");
          if (!cb_name)
            cb_name = iupGetCallbackName(ih, "ACTION_CB");
          if (cb_name && !iupATTRIB_ISINTERNAL(cb_name))
            fprintf(file, "%s", cb_name);
          else
            fprintf(file, "do_nothing");  /* dummy name */
        }
        if (i != count - 1)
          fprintf(file, ", ");
      }

      fprintf(file, ")");
    }
  }

  if (name)
    fprintf(file, "\n\n");
}

static void iLayoutExportChildrenLED(FILE* file, Ihandle* ih)
{
  Ihandle *child;
  char* name;

  /* export children first */
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!(child->flags & IUP_INTERNAL))
      iLayoutExportChildrenLED(file, child);
  }

  name = iLayoutGetName(ih);
  if (name)  /* here process only the ones that have names */
    iLayoutExportElementLED(file, ih, name, 0);
}

static void iLayoutExportDialogLED(FILE* file, Ihandle* dialog, const char* filename)
{
  char* title = NULL;
  char* name = IupGetName(dialog);
  if (!name)
  {
    title = iupStrFileGetTitle(filename);
    iLayoutRemoveExt(title, "led");
    iupAttribSet(dialog, "_IUP_DIALOG_NAME", title);
  }

  fprintf(file, "#   Generated by IupLayoutDialog export to LED.\n\n");

  iLayoutExportChildrenLED(file, dialog);

  if (title)
    free(title);
}

static void iLayoutExportDialog(Ihandle* dialog, const char* filename, const char* format)
{
  FILE* file = fopen(filename, "wb");
  if (!file)
    return;

  if (iupStrEqualNoCase(format, "LED"))
    iLayoutExportDialogLED(file, dialog, filename);
  else if (iupStrEqualNoCase(format, "LUA"))
    iLayoutExportDialogLua(file, dialog, filename);
  else if (iupStrEqualNoCase(format, "C"))
    iLayoutExportDialogC(file, dialog, filename);

  fclose(file);
}

static int iLayoutGetExportFile(Ihandle* parent, char* filename)
{
  Ihandle *file_dlg = 0;
  int ret;
  char filter[4096] = "*.*";
  static char dir[4096] = "";  /* static will make the dir persist from one call to another if not defined */

  file_dlg = IupFileDlg();

  iupStrFileNameSplit(filename, dir, filter);

  IupSetAttribute(file_dlg, "FILTER", filter);
  IupSetAttribute(file_dlg, "DIRECTORY", dir);
  IupSetAttribute(file_dlg, "DIALOGTYPE", "SAVE");
  IupSetAttribute(file_dlg, "ALLOWNEW", "YES");
  IupSetAttribute(file_dlg, "NOCHANGEDIR", "YES");
  IupSetAttributeHandle(file_dlg, "PARENTDIALOG", parent);
  IupSetAttribute(file_dlg, "ICON", IupGetGlobal("ICON"));

  IupPopup(file_dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  ret = IupGetInt(file_dlg, "STATUS");
  if (ret != -1)
  {
    char* value = IupGetAttribute(file_dlg, "VALUE");
    if (value)
    {
      strcpy(filename, value);
      iupStrFileNameSplit(filename, dir, NULL);
    }
  }

  IupDestroy(file_dlg);

  return ret;
}


/***************************************************************************
                             Layout Dialog Menus
***************************************************************************/


static int iLayoutMenuNew_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (layoutdlg->destroy)
    IupDestroy(layoutdlg->dialog);
  layoutdlg->dialog = IupDialog(NULL);
  layoutdlg->destroy = 1;
  iLayoutTreeRebuild(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuUpdate_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  /* rebuild tree and redraw canvas */
  iLayoutTreeRebuild(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuExportLED_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  char filename[4096] = "*.led";
  int ret = iLayoutGetExportFile(dlg, filename);
  if (ret != -1) /* ret==0 existing file. TODO: replace existing contents. */
    iLayoutExportDialog(layoutdlg->dialog, filename, "LED");
  return IUP_DEFAULT;
}

static int iLayoutMenuExportLua_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  char filename[4096] = "*.lua";
  int ret = iLayoutGetExportFile(dlg, filename);
  if (ret != -1) /* ret==0 existing file. TODO: replace existing contents. */
    iLayoutExportDialog(layoutdlg->dialog, filename, "Lua");
  return IUP_DEFAULT;
}

static int iLayoutMenuExportC_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  char filename[4096] = "*.c";
  int ret = iLayoutGetExportFile(dlg, filename);
  if (ret != -1) /* ret==0 existing file. TODO: replace existing contents. */
    iLayoutExportDialog(layoutdlg->dialog, filename, "C");
  return IUP_DEFAULT;
}

static int iLayoutMenuClose_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  if (IupGetInt(dlg, "DESTROYWHENCLOSED"))
  {
    IupDestroy(dlg);
    return IUP_IGNORE;
  }
  else
  {
    IupHide(dlg);
    return IUP_DEFAULT;
  }
}

static int iLayoutMenuHierarchy_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  Ihandle* split = IupGetChild(IupGetChild(dlg, 0), 0);
  if (!IupGetInt(ih, "VALUE"))
    IupSetAttribute(split, "VALUE", "0");
  else
    IupSetAttribute(split, "VALUE", "300");
  return IUP_DEFAULT;
}

static int iLayoutMenuGlobals_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (!layoutdlg->globals)
  {
    layoutdlg->globals = IupGlobalsDialog();
    IupSetAttributeHandle(layoutdlg->globals, "PARENTDIALOG", dlg);
  }
  IupShow(layoutdlg->globals);
  return IUP_DEFAULT;
}

static int iLayoutMenuRefresh_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutUpdateLayout(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutTimerAutoUpdate_CB(Ihandle* ih)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(ih, "_IUP_LAYOUTDIALOG");
  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));
  return IUP_DEFAULT;
}

static int iLayoutMenuShowHidden_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (!IupGetInt(ih, "VALUE"))
    iupAttribSet(dlg, "SHOWHIDDEN", "No");
  else
    iupAttribSet(dlg, "SHOWHIDDEN", "Yes");
  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));
  return IUP_DEFAULT;
}

static int iLayoutMenuShowInternal_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (!IupGetInt(ih, "VALUE"))
    iupAttribSet(dlg, "SHOWINTERNAL", "No");
  else
    iupAttribSet(dlg, "SHOWINTERNAL", "Yes");
  /* rebuild tree and redraw canvas */
  iLayoutTreeRebuild(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuAutoUpdate_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (IupGetInt(layoutdlg->timer, "RUN"))
    IupSetAttribute(layoutdlg->timer, "RUN", "No");
  else
    IupSetAttribute(layoutdlg->timer, "RUN", "Yes");
  return IUP_DEFAULT;
}

static int iLayoutMenuRedraw_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupRedraw(layoutdlg->dialog, 1);
  return IUP_DEFAULT;
}

static int iLayoutGetParamOpacity_CB(Ihandle* dialog, int param_index, void* user_data)
{
  if (param_index == 0)
  {
    Ihandle* dlg = (Ihandle*)user_data;
    Ihandle* param = (Ihandle*)IupGetAttribute(dialog, "PARAM0");
    int opacity = IupGetInt(param, "VALUE");
    IupSetInt(dlg, "OPACITY", opacity);
  }
  return 1;
}

static int iLayoutMenuOpacity_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  int opacity = IupGetInt(dlg, "OPACITY");
  if (opacity == 0)
    opacity = 255;

  if (IupGetParam("Dialog Layout", iLayoutGetParamOpacity_CB, dlg,
                  "Opacity: %i[0,255]\n",
                  &opacity, NULL))
  {

    if (opacity == 0 || opacity == 255)
      IupSetAttribute(dlg, "OPACITY", NULL);
    else
      IupSetInt(dlg, "OPACITY", opacity);
  }

  return IUP_DEFAULT;
}

static int iLayoutMenuFindElement_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(dlg, "FIND_DIALOG");
  Ihandle* tree = IupGetDialogChild(ih, "TREE");

  if (!find_dlg)
  {
    find_dlg = iupLayoutFindElementDialog(tree, layoutdlg->dialog);
    IupSetAttribute(dlg, "FIND_DIALOG", (char*)find_dlg);
  }

  IupSetAttribute(IupGetDialogChild(find_dlg, "FIND_NAME"), "ACTIVE", "YES"); /* we are going to disable it in other situations */

  IupShow(find_dlg);

  return IUP_DEFAULT;
}

static int iLayoutMenuShow_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupShow(layoutdlg->dialog);
  return IUP_DEFAULT;
}

static int iLayoutMenuHide_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupHide(layoutdlg->dialog);
  return IUP_DEFAULT;
}

static int iLayoutDialogGetDialogs(Ihandle* *dlg_list, char* *dlg_list_str, int count, int only_visible)
{
  int i;
  Ihandle *dlg;

  for (dlg = iupDlgListFirst(), i = 0; dlg && i < count; dlg = iupDlgListNext())
  {
    if (!only_visible ||
        (dlg->handle && IupGetInt(dlg, "VISIBLE")))
    {
      dlg_list[i] = dlg;
      dlg_list_str[i] = iupStrDup(iupLayoutGetElementTitle(dlg));
      i++;
    }
  }

  return i;
}

static void iLayoutDialogLoad(Ihandle* dlg, iLayoutDialog* layoutdlg, int only_visible)
{
  int ret, count, i;
  Ihandle* *dlg_list;
  char* *dlg_list_str;

  if (only_visible)
    count = iupDlgListVisibleCount();
  else
    count = iupDlgListCount();

  dlg_list = (Ihandle**)malloc(count*sizeof(Ihandle*));
  dlg_list_str = (char**)malloc(count*sizeof(char*));

  i = iLayoutDialogGetDialogs(dlg_list, dlg_list_str, count, only_visible);

  iupASSERT(i == count);
  if (i != count)
    count = i;

  IupStoreGlobal("_IUP_OLD_PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttributeHandle(NULL, "PARENTDIALOG", dlg);

  ret = IupListDialog(1, "Dialogs", count, (const char**)dlg_list_str, 1, 15, count < 15 ? count + 1 : 15, NULL);

  IupStoreGlobal("PARENTDIALOG", IupGetGlobal("_IUP_OLD_PARENTDIALOG"));
  IupSetGlobal("_IUP_OLD_PARENTDIALOG", NULL);

  if (ret != -1)
  {
    int w = 0, h = 0;

    if (layoutdlg->destroy)
      IupDestroy(layoutdlg->dialog);
    layoutdlg->dialog = dlg_list[ret];
    layoutdlg->destroy = 0;

    IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
    if (w && h)
    {
      Ihandle* canvas = IupGetBrother(layoutdlg->tree);
      IupSetfAttribute(canvas, "USERSIZE", "%dx%d", w, h);
      IupSetAttribute(dlg, "RASTERSIZE", NULL);

      IupShow(dlg);

      IupSetAttribute(canvas, "USERSIZE", NULL);
    }
  }

  for (i = 0; i < count; i++)
    free(dlg_list_str[i]);

  free(dlg_list);
  free(dlg_list_str);
}

static int iLayoutMenuLoad_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutDialogLoad(dlg, layoutdlg, 0);
  iLayoutTreeRebuild(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuLoadVisible_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutDialogLoad(dlg, layoutdlg, 1);
  iLayoutTreeRebuild(layoutdlg);
  return IUP_DEFAULT;
}


/***************************************************************************
                               Canvas Drawing
***************************************************************************/


static void iLayoutDrawElement(IdrawCanvas* dc, Ihandle* ih, int marked, int native_parent_x, int native_parent_y)
{
  int x, y, w, h;
  char *bgcolor;
  long color, fg, fg_void, bg, fg_max;

  bg = iupDrawColor(255, 255, 255, 255);  /* background color */
  fg = iupDrawColor(0, 0, 0, 255);        /* foreground color */
  fg_void = iupDrawColor(160, 160, 160, 255);  /* foreground color for void elements */
  fg_max = iupDrawColor(255, 0, 0, 255);      /* foreground color for elements that are maximizing parent size */

  x = ih->x + native_parent_x;
  y = ih->y + native_parent_y;
  w = ih->currentwidth;
  h = ih->currentheight;
  if (w <= 0) w = 1;
  if (h <= 0) h = 1;

  bgcolor = IupGetAttribute(ih, "BGCOLOR");
  if (bgcolor && ih->iclass->nativetype != IUP_TYPEVOID)
  {
    color = iupDrawStrToColor(bgcolor, bg);
    iupdrvDrawRectangle(dc, x, y, x + w - 1, y + h - 1, color, IUP_DRAW_FILL, 1);
  }

  if (ih->iclass->nativetype == IUP_TYPEVOID)
    iupdrvDrawRectangle(dc, x, y, x + w - 1, y + h - 1, fg_void, IUP_DRAW_STROKE_DASH, 1);
  else
    iupdrvDrawRectangle(dc, x, y, x + w - 1, y + h - 1, fg, IUP_DRAW_STROKE, 1);

  iupdrvDrawSetClipRect(dc, x, y, x + w - 1, y + h - 1);

  if (ih->iclass->childtype == IUP_CHILDNONE)
  {
    int pw, ph;
    IupGetIntInt(ih->parent, "CLIENTSIZE", &pw, &ph);

    if (ih->currentwidth == pw && ih->currentwidth == ih->naturalwidth)
    {
      iupdrvDrawLine(dc, x + 1, y + 1, x + w - 2, y + 1, fg_max, IUP_DRAW_STROKE, 1);
      iupdrvDrawLine(dc, x + 1, y + h - 2, x + w - 2, y + h - 2, fg_max, IUP_DRAW_STROKE, 1);
    }

    if (ih->currentheight == ph && ih->currentheight == ih->naturalheight)
    {
      iupdrvDrawLine(dc, x + 1, y + 1, x + 1, y + h - 2, fg_max, IUP_DRAW_STROKE, 1);
      iupdrvDrawLine(dc, x + w - 2, y + 1, x + w - 2, y + h - 2, fg_max, IUP_DRAW_STROKE, 1);
    }
  }
  else if (ih->iclass->nativetype != IUP_TYPEVOID)
  {
    /* if ih is a Tabs, position the title accordingly */
    if (IupClassMatch(ih, "tabs"))
    {
      /* TABORIENTATION is ignored */
      char* tabtype = IupGetAttribute(ih, "TABTYPE");
      if (iupStrEqualNoCase(tabtype, "BOTTOM"))
      {
        int cw = 0, ch = 0;
        IupGetIntInt(ih, "CLIENTSIZE", &cw, &ch);
        y += ch;  /* position after the client area */
      }
      else if (iupStrEqualNoCase(tabtype, "RIGHT"))
      {
        int cw = 0, ch = 0;
        IupGetIntInt(ih, "CLIENTSIZE", &cw, &ch);
        x += cw;  /* position after the client area */
      }
    }
  }

  /* always draw the image first */
  if (ih->iclass->nativetype != IUP_TYPEVOID)
  {
    char *title, *image;

    image = IupGetAttribute(ih, "IMAGE0");  /* Tree root node title */
    if (!image)
      image = IupGetAttribute(ih, "TABIMAGE0");  /* Tabs first tab image */
    if (image)
    {
      /* returns the image of the active tab */
      int pos = IupGetInt(ih, "VALUEPOS");
      image = IupGetAttributeId(ih, "TABIMAGE", pos);
    }
    if (!image)
      image = IupGetAttribute(ih, "IMAGE");
    if (image)
    {
      char* position;
      int img_w, img_h;

      iupImageGetInfo(image, &img_w, &img_h, NULL);

      iupdrvDrawImage(dc, image, 0, bgcolor, x + 1, y + 1, img_w, img_h);

      position = IupGetAttribute(ih, "IMAGEPOSITION");  /* used only for buttons */
      if (position &&
          (iupStrEqualNoCase(position, "BOTTOM") ||
          iupStrEqualNoCase(position, "TOP")))
          y += img_h;
      else
        x += img_w;  /* position text usually at right */
    }

    title = IupGetAttribute(ih, "0:0");  /* Matrix title cell */
    if (!title)
      title = IupGetAttribute(ih, "1");  /* List first item */
    if (!title)
      title = IupGetAttribute(ih, "TITLE0");  /* Tree root node title */
    if (!title)
    {
      title = IupGetAttribute(ih, "TABTITLE0");  /* Tabs first tab title */
      if (title)
      {
        /* returns the title of the active tab */
        int pos = IupGetInt(ih, "VALUEPOS");
        title = IupGetAttributeId(ih, "TABTITLE", pos);
      }
    }
    if (!title)
      title = iupAttribGetLocal(ih, "TITLE");
    if (title)
    {
      int len;
      iupStrNextLine(title, &len);  /* get the size of the first line */
      color = iupDrawStrToColor(IupGetAttribute(ih, "FGCOLOR"), fg);
      iupdrvDrawText(dc, title, len, x + 1, y + 1, w, h, color, IupGetAttribute(ih, "FONT"), IUP_DRAW_LEFT, 0);
    }

    if (ih->iclass->childtype == IUP_CHILDNONE &&
        !title && !image)
    {
      if (IupClassMatch(ih, "progressbar") || IupClassMatch(ih, "gauge"))
      {
        double min = IupGetDouble(ih, "MIN");
        double max = IupGetDouble(ih, "MAX");
        double val = IupGetDouble(ih, "VALUE");
        color = iupDrawStrToColor(IupGetAttribute(ih, "FGCOLOR"), fg);
        if (iupStrEqualNoCase(IupGetAttribute(ih, "ORIENTATION"), "VERTICAL"))
        {
          int ph = (int)(((max - val)*(h - 5)) / (max - min));
          iupdrvDrawRectangle(dc, x + 2, y + 2, x + w - 3, y + ph, color, IUP_DRAW_FILL, 1);
        }
        else
        {
          int pw = (int)(((val - min)*(w - 5)) / (max - min));
          iupdrvDrawRectangle(dc, x + 2, y + 2, x + pw, y + h - 3, color, IUP_DRAW_FILL, 1);
        }
      }
      else if (IupClassMatch(ih, "val"))
      {
        double min = IupGetDouble(ih, "MIN");
        double max = IupGetDouble(ih, "MAX");
        double val = IupGetDouble(ih, "VALUE");
        color = iupDrawStrToColor(IupGetAttribute(ih, "FGCOLOR"), fg);
        if (iupStrEqualNoCase(IupGetAttribute(ih, "ORIENTATION"), "VERTICAL"))
        {
          int ph = (int)(((max - val)*(h - 5)) / (max - min));
          iupdrvDrawRectangle(dc, x + 2, y + ph - 1, x + w - 3, y + ph + 1, color, IUP_DRAW_FILL, 1);
        }
        else
        {
          int pw = (int)(((val - min)*(w - 5)) / (max - min));
          iupdrvDrawRectangle(dc, x + pw - 1, y + 2, x + pw + 1, y + h - 3, color, IUP_DRAW_FILL, 1);
        }
      }
    }
  }

  iupdrvDrawResetClip(dc);

  if (marked)
  {
    x = ih->x + native_parent_x;
    y = ih->y + native_parent_y;
    w = ih->currentwidth;
    h = ih->currentheight;
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    iupdrvDrawSelectRect(dc, x, y, x + w - 1, y + h - 1);
  }
}

static int iLayoutElementIsVisible(Ihandle* ih, int dlgvisible)
{
  if (dlgvisible)
    return iupStrBoolean(iupAttribGetLocal(ih, "VISIBLE"));
  else
  {
    /* can not check at native implementation because it will be always not visible */
    char* value = iupAttribGet(ih, "VISIBLE");
    if (!value)
      return 1; /* default is visible */
    else
      return iupStrBoolean(value);
  }
}

static void iLayoutDrawElementTree(IdrawCanvas* dc, int showhidden, int dlgvisible, int shownotmapped, int showinternal, Ihandle* mark, Ihandle* ih, int native_parent_x, int native_parent_y)
{
  Ihandle *child;
  int dx, dy;

  if ((showhidden || iLayoutElementIsVisible(ih, dlgvisible)) &&
      (shownotmapped || ih->handle))
  {
    /* draw the element */
    iLayoutDrawElement(dc, ih, ih == mark, native_parent_x, native_parent_y);

    if (ih->iclass->childtype != IUP_CHILDNONE)
    {
      /* if ih is a native parent, then update the offset */
      if (ih->iclass->nativetype != IUP_TYPEVOID)
      {
        dx = 0, dy = 0;
        IupGetIntInt(ih, "CLIENTOFFSET", &dx, &dy);
        native_parent_x += ih->x + dx;
        native_parent_y += ih->y + dy;

        /* if ih is a Zbox like, then draw only the active child */
        if (IupClassMatch(ih, "zbox") || IupClassMatch(ih, "tabs") || IupClassMatch(ih, "flattabs"))
        {
          child = (Ihandle*)IupGetAttribute(ih, "VALUE_HANDLE");
          if (child)
            iLayoutDrawElementTree(dc, showhidden, dlgvisible, shownotmapped, showinternal, mark, child, native_parent_x, native_parent_y);
          return;
        }
      }
    }

    /* draw its children */
    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!(child->flags & IUP_INTERNAL) || showinternal)
        iLayoutDrawElementTree(dc, showhidden, dlgvisible, shownotmapped, showinternal, mark, child, native_parent_x, native_parent_y);
    }
  }
}

static void iLayoutDrawDialog(iLayoutDialog* layoutdlg, int showhidden, int showinternal, IdrawCanvas* dc, Ihandle* mark, int posx, int posy)
{
  int w, h;

  iupdrvDrawGetSize(dc, &w, &h);
  iupdrvDrawRectangle(dc, 0, 0, w - 1, h - 1, iupDrawColor(255, 255, 255, 255), IUP_DRAW_FILL, 1);

  /* draw the dialog */
  IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
  iupdrvDrawRectangle(dc, 0, 0, w - 1, h - 1, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE, 1);

  if (layoutdlg->dialog->firstchild)
  {
    int native_parent_x = 0, native_parent_y = 0;
    int shownotmapped = layoutdlg->dialog->handle == NULL;  /* only show not mapped if dialog is also not mapped */
    int dlgvisible = IupGetInt(layoutdlg->dialog, "VISIBLE");
    IupGetIntInt(layoutdlg->dialog, "CLIENTOFFSET", &native_parent_x, &native_parent_y);
    native_parent_x -= posx;
    native_parent_y -= posy;
    iLayoutDrawElementTree(dc, showhidden, dlgvisible, shownotmapped, showinternal, mark, layoutdlg->dialog->firstchild, native_parent_x, native_parent_y);
  }
}

static void iLayoutDrawCursor(Ihandle* canvas, IdrawCanvas* dc, int posx, int posy)
{
  if (iupAttribGet(canvas, "INSERTCURSOR"))
  {
    long fg_insert = iupDrawColor(0, 255, 0, 255);

    if (iupAttribGet(canvas, "INSERTCURSOR_POINT"))
    {
      int x, y;
      int IC_PS = 10;
      sscanf(iupAttribGet(canvas, "INSERTCURSOR_POINT"), "%d,%d", &x, &y);
      x -= posx;
      y -= posy;

      iupdrvDrawLine(dc, x - IC_PS, y, x + IC_PS, y, fg_insert, IUP_DRAW_STROKE, 2);
      iupdrvDrawLine(dc, x, y - IC_PS, x, y + IC_PS, fg_insert, IUP_DRAW_STROKE, 2);

      IC_PS = 3;
      iupdrvDrawArc(dc, x - IC_PS, y - IC_PS, x + IC_PS, y + IC_PS, 0, 360, fg_insert, IUP_DRAW_STROKE, 2);
    }
    else
    {
      int IC_PS = 3;
      int x1, x2, y1, y2, xc, yc;
      sscanf(iupAttribGet(canvas, "INSERTCURSOR_LINE"), "%d,%d,%d,%d", &x1, &y1, &x2, &y2);
      x1 -= posx;
      y1 -= posy;
      x2 -= posx;
      y2 -= posy;

      iupdrvDrawLine(dc, x1, y1, x2, y2, fg_insert, IUP_DRAW_STROKE, 2);

      xc = (x1 + x2) / 2;
      yc = (y1 + y2) / 2;
      iupdrvDrawArc(dc, xc - IC_PS, yc - IC_PS, xc + IC_PS, yc + IC_PS, 0, 360, fg_insert, IUP_DRAW_STROKE, 2);
    }
  }
}

static int iLayoutCanvas_CB(Ihandle* canvas, float fposx, float fposy)
{
  Ihandle* dlg = IupGetDialog(canvas);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IdrawCanvas* dc = iupdrvDrawCreateCanvas(canvas);
  int showhidden = IupGetInt(dlg, "SHOWHIDDEN");
  int showinternal = IupGetInt(dlg, "SHOWINTERNAL");
  Ihandle* mark = (Ihandle*)iupAttribGet(dlg, "_IUPLAYOUT_MARK");

  iLayoutDrawDialog(layoutdlg, showhidden, showinternal, dc, mark, (int)fposx, (int)fposy);

  iLayoutDrawCursor(canvas, dc, (int)fposx, (int)fposy);

  iupdrvDrawFlush(dc);

  iupdrvDrawKillCanvas(dc);

  return IUP_DEFAULT;
}


/***************************************************************************
                          Context Menu
***************************************************************************/

static int iLayoutPropertiedChanged_CB(Ihandle* properties)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(properties, "_IUP_LAYOUTDIALOG");

  layoutdlg->changed = 1;

  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));
  return IUP_DEFAULT;
}

static int iLayoutContextMenuProperties_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");

  if (!layoutdlg->properties)
  {
    layoutdlg->properties = IupElementPropertiesDialog(dlg, elem);
    IupSetAttribute(layoutdlg->properties, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);
    IupSetCallback(layoutdlg->properties, "PROPERTIESCHANGED_CB", iLayoutPropertiedChanged_CB);
  }
  else
    iupLayoutPropertiesUpdate(layoutdlg->properties, elem);

  IupShow(layoutdlg->properties);

  return IUP_DEFAULT;
}

static void iLayoutTreeUpdateTitle(iLayoutDialog* layoutdlg, Ihandle* ih)
{
  int id = IupTreeGetId(layoutdlg->tree, ih);
  IupSetAttributeId(layoutdlg->tree, "TITLE", id, iupLayoutGetElementTitle(ih));
}

static int iLayoutContextMenuHandleName_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  char name[256] = "";

  char* elem_name = IupGetName(elem);
  if (elem_name)
    strcpy(name, elem_name);

  if (IupGetParam("Handle Name", NULL, NULL,
                  "Name: %s\n",
                  name, NULL))
  {
    if (name[0] == 0 || name[0] == ' ')
    {
      if (elem_name)
      {
        IupSetHandle(elem_name, NULL);
        iLayoutTreeUpdateTitle(layoutdlg, elem);
      }
    }
    else
    {
      IupSetHandle(name, elem);
      iLayoutTreeUpdateTitle(layoutdlg, elem);
    }
  }

  return IUP_DEFAULT;
}

static int iLayoutSelectClassOK_CB(Ihandle* ih)
{
  iupAttribSet(IupGetDialog(ih), "STATUS", "1");
  return IUP_CLOSE;
}

static int iLayoutSelectClassCancel_CB(Ihandle* ih)
{
  iupAttribSet(IupGetDialog(ih), "STATUS", "-1");
  return IUP_CLOSE;
}

static int iLayoutSelectClassTreeExecuteLeaf_CB(Ihandle *ih, int id)
{
  (void)id;
  iupAttribSet(IupGetDialog(ih), "STATUS", "1");
  IupExitLoop();
  return IUP_DEFAULT;
}

static const char* iLayoutSelectClassDialog(Ihandle* parent)
{
  Ihandle *tree, *ok, *dlg, *cancel, *dlg_box, *button_box;
  int last_containers_id, last_standard_id, last_additional_id;
  const char* value = NULL;
  int count, i;
  char** class_list_str, **p_str;

  tree = IupTree();
  IupSetAttribute(tree, "ADDROOT", "NO");
  IupSetAttribute(tree, "IMAGELEAF", "IMGEMPTY");
  IupSetAttribute(tree, "SIZE", "120x160");

  ok = IupButton("_@IUP_OK", NULL);
  IupSetStrAttribute(ok, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(ok, "ACTION", (Icallback)iLayoutSelectClassOK_CB);

  cancel = IupButton("_@IUP_CANCEL", NULL);
  IupSetStrAttribute(cancel, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(cancel, "ACTION", (Icallback)iLayoutSelectClassCancel_CB);

  button_box = IupHbox(
    IupFill(),
    ok,
    cancel,
    NULL);
  IupSetAttribute(button_box, "MARGIN", "0x0");
  IupSetAttribute(button_box, "NORMALIZESIZE", "HORIZONTAL");

  dlg_box = IupVbox(
    tree,
    button_box,
    NULL);

  IupSetAttribute(dlg_box, "MARGIN", "10x10");
  IupSetAttribute(dlg_box, "GAP", "10");

  dlg = IupDialog(dlg_box);

  IupSetCallback(tree, "EXECUTELEAF_CB", (Icallback)iLayoutSelectClassTreeExecuteLeaf_CB);

  IupSetStrAttribute(dlg, "TITLE", "Select IUP Class");
  IupSetAttribute(dlg, "MINBOX", "NO");
  IupSetAttribute(dlg, "MAXBOX", "NO");
  IupSetAttributeHandle(dlg, "DEFAULTENTER", ok);
  IupSetAttributeHandle(dlg, "DEFAULTESC", cancel);
  IupSetAttributeHandle(dlg, "PARENTDIALOG", parent);
  IupSetAttribute(dlg, "ICON", IupGetGlobal("ICON"));

  IupMap(dlg);

  IupSetAttribute(tree, "ADDBRANCH-1", "Containers");
  last_containers_id = 0;
  IupSetAttribute(tree, "INSERTBRANCH0", "Standard");
  last_standard_id = 1;
  IupSetAttribute(tree, "INSERTBRANCH1", "Additional");
  last_additional_id = 2;

  count = IupGetAllClasses(NULL, 0);
  class_list_str = (char**)malloc(count * sizeof(char*));

  IupGetAllClasses(class_list_str, count);
  qsort(class_list_str, count, sizeof(char*), iLayoutCompareStr);

  /* filter the list of classes */
  p_str = class_list_str;
  for (i = 0; i < count; i++)
  {
    Iclass* iclass = iupRegisterFindClass(class_list_str[i]);
    if (iclass->nativetype == IUP_TYPEVOID ||
        iclass->nativetype == IUP_TYPECONTROL ||
        iclass->nativetype == IUP_TYPECANVAS)
      *p_str++ = class_list_str[i];
  }
  count = (int)(p_str - class_list_str);

  for (i = 0; i < count; i++)
  {
    char* name = class_list_str[i];
    Iclass *elemClass = iupRegisterFindClass(name);
    char constructor[50];

    if (elemClass->cons)
      strcpy(constructor, elemClass->cons);
    else
    {
      strcpy(constructor, name);
      constructor[0] = (char)toupper(constructor[0]);
    }

    if (elemClass->childtype != IUP_CHILDNONE && elemClass->is_internal) /* internal containers */
    {
      IupSetStrfId(tree, "ADDLEAF", last_containers_id, "Iup%s", constructor);
      last_containers_id++;
      last_standard_id++;
      last_additional_id++;
    }
    else if (elemClass->is_internal)
    {
      IupSetStrfId(tree, "ADDLEAF", last_standard_id, "Iup%s", constructor);
      last_standard_id++;
      last_additional_id++;
    }
    else /* additional */
    {
      IupSetStrfId(tree, "ADDLEAF", last_additional_id, "Iup%s", constructor);
      last_additional_id++;
    }
  }

  IupSetAttribute(tree, "USERSIZE", NULL);

  IupPopup(dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  if (IupGetInt(dlg, "STATUS") == 1)
  {
    Iclass *elemClass;
    int id = IupGetInt(tree, "VALUE");
    char* name = IupGetAttributeId(tree, "TITLE", id);
    name += 3;
    iupStrLower(name, name);
    elemClass = iupRegisterFindClass(name);
    value = elemClass->name;
  }

  IupDestroy(dlg);

  free(class_list_str);
  return value;
}

static int iLayoutContextMenuNewInsertBrother_CB(Ihandle* menu)
{
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");
  const char* name = iLayoutSelectClassDialog(dlg);
  if (name)
  {
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
    Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
    Ihandle* ret_ih = NULL;

    Ihandle* new_ih = IupCreate(name);
    int ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

    /* add as brother after reference */
    if (ref_elem->brother)
      /* add before the brother, so it will be the brother */
      ret_ih = IupInsert(ref_elem->parent, ref_elem->brother, new_ih);
    else
      ret_ih = IupAppend(ref_elem->parent, new_ih);

    if (!ret_ih)
    {
      IupMessage("Error", "Add failed. Invalid operation for this node.");
      return IUP_DEFAULT;
    }

    layoutdlg->changed = 1;

    /* add to the tree */
    iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

    iLayoutUpdateLayout(layoutdlg);
  }

  return IUP_DEFAULT;
}

static int iLayoutContextMenuNewInsertChild_CB(Ihandle* menu)
{
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");
  const char* name = iLayoutSelectClassDialog(dlg);
  if (name)
  {
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
    Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

    Ihandle* new_ih = IupCreate(name);
    int ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

    /* add as first child */
    if (!IupInsert(ref_elem, NULL, new_ih))
    {
      IupMessage("Error", "Add failed. Invalid operation for this node.");
      return IUP_DEFAULT;
    }

    layoutdlg->changed = 1;

    /* add to the tree */
    iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

    iLayoutUpdateLayout(layoutdlg);
  }

  return IUP_DEFAULT;
}

static int iLayoutContextMenuNewAppendChild_CB(Ihandle* menu)
{
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");
  const char* name = iLayoutSelectClassDialog(dlg);
  if (name)
  {
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
    Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

    Ihandle* new_ih = IupCreate(name);
    int ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

    /* add as last child */
    if (!IupAppend(ref_elem, new_ih))
    {
      IupMessage("Error", "Add failed. Invalid operation for this node.");
      return IUP_DEFAULT;
    }

    layoutdlg->changed = 1;

    /* add to the tree */
    iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

    iLayoutUpdateLayout(layoutdlg);
  }

  return IUP_DEFAULT;
}

static int iLayoutContextMenuInsertCursor_CB(Ihandle* menu)
{
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");
  const char* name = iLayoutSelectClassDialog(dlg);
  if (name)
  {
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
    Ihandle* container = (Ihandle*)iupAttribGetInherit(menu, "INSERTCURSOR"); /* the container */
    Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "INSERTCURSOR_ELEMENT");
    Ihandle* ret_ih = NULL;

    Ihandle* new_ih = IupCreate(name);
    int ref_id;
    
    if (!ref_elem)
    {
      int cx, cy;
      if (sscanf(iupAttribGetInherit(menu, "INSERTCURSOR_ELEMENT_POS"), "%d,%d", &cx, &cy) == 2) /* cbox */
      {
        ret_ih = IupAppend(container, new_ih);
        ref_elem = iupChildTreeGetPrevBrother(new_ih);
        IupSetInt(new_ih, "CX", cx);
        IupSetInt(new_ih, "CY", cy);
        if (ref_elem)
          ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);
        else
          ref_id = IupTreeGetId(layoutdlg->tree, container); /* empty cbox */
      }
      else /* empty box */
      {
        ref_id = IupTreeGetId(layoutdlg->tree, container);
        ret_ih = IupAppend(container, new_ih);
      }
    }
    else
    {
      int insert_before = IupGetInt(menu, "INSERTCURSOR_BEFORE");

      ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

      if (insert_before)
      {
        ref_id--;
        IupInsert(container, ref_elem, new_ih);
      }
      else
      {
        /* add as brother after reference */
        if (ref_elem->brother)
          /* add before the brother, so it will be the brother */
          ret_ih = IupInsert(container, ref_elem->brother, new_ih);
        else
          ret_ih = IupAppend(container, new_ih);
      }
    }

    if (!ret_ih)
    {
      IupMessage("Error", "Add failed. Invalid operation for this node.");
      return IUP_DEFAULT;
    }

    layoutdlg->changed = 1;

    /* add to the tree */
    iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

    iLayoutUpdateLayout(layoutdlg);
  }

  return IUP_DEFAULT;
}

static void iLayoutUpdateColors(Ihandle* tree, Ihandle* ih)
{
  iLayoutTreeSetNodeColor(tree, IupTreeGetId(tree, ih), ih);

  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    Ihandle *child;
    for (child = ih->firstchild; child; child = child->brother)
    {
      iLayoutUpdateColors(tree, child);
    }
  }
}

static int iLayoutContextMenuMap_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

  if (IupMap(elem) == IUP_ERROR)
  {
    IupMessage("Error", "IupMap failed.");
    return IUP_DEFAULT;
  }

  iLayoutUpdateColors(layoutdlg->tree, elem);

  iLayoutUpdateLayout(layoutdlg);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuRefreshChildren_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");

  IupRefreshChildren(layoutdlg->dialog);

  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));

  return IUP_DEFAULT;
}

static void iLayoutSelectTreeItem(iLayoutDialog* layoutdlg, Ihandle* elem);

static int iLayoutContextMenuGoToParent_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

  if (elem->parent)
    iLayoutSelectTreeItem(layoutdlg, elem->parent);

  return IUP_DEFAULT;
}

static void iLayoutSaveAttributes(Ihandle* ih)
{
  IupSaveClassAttributes(ih);

  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    Ihandle *child;
    for (child = ih->firstchild; child; child = child->brother)
    {
      iLayoutSaveAttributes(child);
    }
  }
}

static int iLayoutContextMenuUnmap_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

  iLayoutSaveAttributes(elem);

  IupUnmap(elem);

  iLayoutUpdateColors(layoutdlg->tree, elem);

  iLayoutUpdateLayout(layoutdlg);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuSetFocus_CB(Ihandle* menu)
{
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

  IupShow(IupGetDialog(elem)); /* must be the active dialog to get the focus */

  IupSetFocus(elem);

  return IUP_DEFAULT;
}

static void iLayoutBlink(Ihandle* ih);

static int iLayoutContextMenuBlink_CB(Ihandle* menu)
{
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");

  iLayoutBlink(elem);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuRemove_CB(Ihandle* menu)
{
  Ihandle* msg_dlg;
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  if (!elem)  /* can be called from a key press */
    elem = (Ihandle*)IupTreeGetUserId(layoutdlg->tree, IupGetInt(layoutdlg->tree, "VALUE"));
  if (!elem)
    return IUP_DEFAULT;

  if (elem->flags & IUP_INTERNAL)
  {
    IupMessage("Error", "Can NOT remove this child. It is an internal element of the container.");
    return IUP_DEFAULT;
  }

  msg_dlg = IupMessageDlg();
  IupSetAttribute(msg_dlg, "DIALOGTYPE", "QUESTION");
  IupSetAttribute(msg_dlg, "BUTTONS", "OKCANCEL");
  IupSetAttribute(msg_dlg, "TITLE", "Element Remove");
  IupSetAttribute(msg_dlg, "VALUE", "Remove the selected element?");

  IupPopup(msg_dlg, IUP_MOUSEPOS, IUP_MOUSEPOS);

  if (IupGetInt(msg_dlg, "BUTTONRESPONSE") == 1)
  {
    int id = IupTreeGetId(layoutdlg->tree, elem);

    layoutdlg->changed = 1;

    /* remove from the tree */
    IupSetAttributeId(layoutdlg->tree, "DELNODE", id, "SELECTED");

    /* update properties if necessary */
    if (layoutdlg->properties && IupGetInt(layoutdlg->properties, "VISIBLE"))
    {
      Ihandle* propelem = (Ihandle*)iupAttribGetInherit(layoutdlg->properties, "_IUP_PROPELEMENT");
      if (iupChildTreeIsChild(elem, propelem))
      {
        /* if current element will be removed, then use the previous element on the tree |*/
        iupLayoutPropertiesUpdate(layoutdlg->properties, (Ihandle*)IupTreeGetUserId(layoutdlg->tree, id - 1));
      }
    }

    IupDestroy(elem);

    iLayoutUpdateLayout(layoutdlg);
  }

  IupDestroy(msg_dlg);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuCopy_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  layoutdlg->copy = elem;
  return IUP_DEFAULT;
}

static int iLayoutContextMenuPasteInsertBrother_CB(Ihandle* menu)
{
  Ihandle* new_ih, *ret_ih = NULL;
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  int ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

  new_ih = IupCreate(layoutdlg->copy->iclass->name);
  IupCopyClassAttributes(layoutdlg->copy, new_ih);

  /* add as brother after reference */
  if (ref_elem->brother)
    /* add before the brother, so it will be the brother */
    ret_ih = IupInsert(ref_elem->parent, ref_elem->brother, new_ih);
  else
    ret_ih = IupAppend(ref_elem->parent, new_ih);

  if (!ret_ih)
  {
    IupMessage("Error", "Paste failed. Invalid operation for this node.");
    return IUP_DEFAULT;
  }

  layoutdlg->changed = 1;

  /* add to the tree */
  iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

  iLayoutUpdateLayout(layoutdlg);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuPasteInsertChild_CB(Ihandle* menu)
{
  Ihandle* new_ih;
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  int ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

  new_ih = IupCreate(layoutdlg->copy->iclass->name);
  IupCopyClassAttributes(layoutdlg->copy, new_ih);

  /* add as first child */
  if (!IupInsert(ref_elem, NULL, new_ih))
  {
    IupMessage("Error", "Paste failed. Invalid operation for this node.");
    return IUP_DEFAULT;
  }

  layoutdlg->changed = 1;

  /* add to the tree */
  iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

  iLayoutUpdateLayout(layoutdlg);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuPasteAppendChild_CB(Ihandle* menu)
{
  Ihandle* new_ih;
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTCONTEXTELEMENT");
  int ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

  new_ih = IupCreate(layoutdlg->copy->iclass->name);
  IupCopyClassAttributes(layoutdlg->copy, new_ih);

  /* add as last child */
  if (!IupAppend(ref_elem, new_ih))
  {
    IupMessage("Error", "Paste failed. Invalid operation for this node.");
    return IUP_DEFAULT;
  }

  layoutdlg->changed = 1;

  /* add to the tree */
  iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

  iLayoutUpdateLayout(layoutdlg);

  return IUP_DEFAULT;
}

static int iLayoutContextMenuPasteCursor_CB(Ihandle* menu)
{
  Ihandle* new_ih, *ret_ih = NULL;
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* container = (Ihandle*)iupAttribGetInherit(menu, "INSERTCURSOR"); /* the container */
  Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "INSERTCURSOR_ELEMENT");
  int ref_id;

  new_ih = IupCreate(layoutdlg->copy->iclass->name);
  IupCopyClassAttributes(layoutdlg->copy, new_ih);

  if (!ref_elem)
  {
    int cx, cy;
    if (sscanf(iupAttribGetInherit(menu, "INSERTCURSOR_ELEMENT_POS"), "%d,%d", &cx, &cy) == 2) /* cbox */
    {
      ret_ih = IupAppend(container, new_ih);
      ref_elem = iupChildTreeGetPrevBrother(new_ih);
      IupSetInt(new_ih, "CX", cx);
      IupSetInt(new_ih, "CY", cy);
      if (ref_elem)
        ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);
      else
        ref_id = IupTreeGetId(layoutdlg->tree, container);  /* empty cbox */
    }
    else /* empty box */
    {
      ref_id = IupTreeGetId(layoutdlg->tree, container);
      ret_ih = IupAppend(container, new_ih);
    }
  }
  else
  {
    int insert_before = IupGetInt(menu, "INSERTCURSOR_BEFORE");

    ref_id = IupTreeGetId(layoutdlg->tree, ref_elem);

    if (insert_before)
    {
      ref_id--;
      IupInsert(container, ref_elem, new_ih);
    }
    else
    {
      /* add as brother after reference */
      if (ref_elem->brother)
        /* add before the brother, so it will be the brother */
        ret_ih = IupInsert(container, ref_elem->brother, new_ih);
      else
        ret_ih = IupAppend(container, new_ih);
    }
  }

  if (!ret_ih)
  {
    IupMessage("Error", "Paste failed. Invalid operation for this node.");
    return IUP_DEFAULT;
  }

  layoutdlg->changed = 1;

  /* add to the tree */
  iLayoutTreeAddNode(layoutdlg->tree, ref_id, new_ih);

  iLayoutUpdateLayout(layoutdlg);

  return IUP_DEFAULT;
}

static int iLayoutIsEmptyContainer(Ihandle* elem)
{
  int child_count = IupGetChildCount(elem);
  if (elem->firstchild && (elem->firstchild->flags & IUP_INTERNAL))
    child_count--;
  return child_count == 0;
}

static void iLayoutContextMenu(iLayoutDialog* layoutdlg, Ihandle* elem, Ihandle* dlg)
{
  Ihandle* menu;
  int is_container = elem->iclass->childtype != IUP_CHILDNONE;
  int can_copy = !is_container || iLayoutIsEmptyContainer(elem);
  int can_paste = layoutdlg->copy != NULL;
  int can_map = (elem->handle == NULL) && (elem->parent == NULL || elem->parent->handle != NULL);
  int can_unmap = elem->handle != NULL;
  int can_blink = (elem->iclass->nativetype != IUP_TYPEVOID && IupGetInt(elem, "VISIBLE"));
  int can_focus = iupFocusCanAccept(elem);
  Ihandle* canvas = IupGetBrother(layoutdlg->tree);
  Ihandle* insert_cursor = (Ihandle*)IupGetAttribute(canvas, "INSERTCURSOR");
  int can_cursor = insert_cursor != NULL;
  int can_brother = 1;
  int can_child = 1;

  if (!iupObjectCheck(layoutdlg->copy))
  {
    layoutdlg->copy = NULL;
    can_paste = 0;
  }

  if (elem->iclass->childtype == IUP_CHILDNONE)  /* not a container */
    can_child = 0;
  if (elem->iclass->childtype > IUP_CHILDMANY &&    /* container is full */
      IupGetChildCount(elem) == elem->iclass->childtype - IUP_CHILDMANY)
    can_child = 0;

  if (!elem->parent)
    can_brother = 0;
  else
  {
    if (elem->parent->iclass->childtype == IUP_CHILDNONE)
      can_brother = 0;
    if (elem->parent->iclass->childtype > IUP_CHILDMANY &&
        IupGetChildCount(elem->parent) == elem->parent->iclass->childtype - IUP_CHILDMANY)
      can_brother = 0;
  }

  menu = IupMenu(
    IupSetCallbacks(IupItem("Properties...", NULL), "ACTION", iLayoutContextMenuProperties_CB, NULL),
    IupSetCallbacks(IupItem("Handle Name...", NULL), "ACTION", iLayoutContextMenuHandleName_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Map", NULL), can_map ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuMap_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Unmap", NULL), can_unmap ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuUnmap_CB, NULL),
    IupSetCallbacks(IupItem("Refresh Children", NULL), "ACTION", iLayoutContextMenuRefreshChildren_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("Go to Parent", NULL), "ACTION", iLayoutContextMenuGoToParent_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Blink", NULL), can_blink ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuBlink_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Set Focus", NULL), can_focus ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuSetFocus_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupSetAttributes(IupItem("Copy", NULL), can_copy ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuCopy_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Paste Insert Child", NULL), can_paste && can_child ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuPasteInsertChild_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Paste Insert at Cursor", NULL), can_paste && can_cursor ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuPasteCursor_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Paste Append Child", NULL), can_paste && can_child ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuPasteAppendChild_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Paste Insert Brother", NULL), can_paste && can_brother ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuPasteInsertBrother_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupSetAttributes(IupItem("New Insert Child...", NULL), can_child ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuNewInsertChild_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("New Insert at Cursor...", NULL), can_cursor ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuInsertCursor_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("New Append Child...", NULL), can_child ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuNewAppendChild_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("New Insert Brother...", NULL), can_brother ? "ACTIVE=Yes" : "ACTIVE=No"), "ACTION", iLayoutContextMenuNewInsertBrother_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("Remove...\tDel", NULL), "ACTION", iLayoutContextMenuRemove_CB, NULL),
    NULL);

  if (can_cursor)
  {
    iupAttribSet(menu, "INSERTCURSOR", (char*)insert_cursor);
    iupAttribSetStr(menu, "INSERTCURSOR_ELEMENT_POS", iupAttribGet(canvas, "INSERTCURSOR_ELEMENT_POS"));
    iupAttribSet(menu, "INSERTCURSOR_ELEMENT", iupAttribGet(canvas, "INSERTCURSOR_ELEMENT"));
    iupAttribSetStr(menu, "INSERTCURSOR_BEFORE", iupAttribGet(canvas, "INSERTCURSOR_BEFORE"));
  }

  iupAttribSet(menu, "_IUP_LAYOUTCONTEXTELEMENT", (char*)elem);
  iupAttribSet(menu, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);
  iupAttribSet(menu, "_IUP_LAYOUTDLG", (char*)dlg);

  IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);

  IupDestroy(menu);
}


/***************************************************************************
                       Layout Canvas Interaction
***************************************************************************/


static void iLayoutBlink(Ihandle* ih)
{
  if (ih->iclass->nativetype != IUP_TYPEVOID && IupGetInt(ih, "VISIBLE"))
  {
    int i;
    for (i = 0; i < 3; i++)
    {
      IupSetAttribute(ih, "VISIBLE", "NO");
      IupFlush();
      iupdrvSleep(100);
      IupSetAttribute(ih, "VISIBLE", "Yes");
      IupFlush();
      iupdrvSleep(100);
    }
  }
}

static void iLayoutUpdateMark(iLayoutDialog* layoutdlg, Ihandle* ih, int id)
{
  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    int x = 0, y = 0;
    int w, h;
    IupGetIntInt(ih, "CLIENTOFFSET", &x, &y);
    IupGetIntInt(ih, "CLIENTSIZE", &w, &h);

    if (!IupClassMatch(ih, "dialog"))
    {
      IupSetfAttribute(layoutdlg->status, "TITLE", "Position:%4d,%4d | User:%4d,%4d | Natural:%4d,%4d | Current:%4d,%4d\n"
                                                   "Client Offset: %4d, %4d | Client Size: %4d, %4d",
                       ih->x, ih->y, ih->userwidth, ih->userheight, ih->naturalwidth, ih->naturalheight, ih->currentwidth, ih->currentheight,
                       x, y, w, h);
    }
    else
    {
      int border, caption, menu;
      iupdrvDialogGetDecoration(ih, &border, &caption, &menu);
      IupSetfAttribute(layoutdlg->status, "TITLE", "Position:%4d,%4d | User:%4d,%4d | Natural:%4d,%4d | Current:%4d,%4d\n"
                                                   "Client Offset: %4d, %4d | Client Size: %4d, %4d | Border: %3d, Caption: %3d, Menu: %3d",
                       ih->x, ih->y, ih->userwidth, ih->userheight, ih->naturalwidth, ih->naturalheight, ih->currentwidth, ih->currentheight,
                       x, y, w, h, border, caption, menu);
    }
  }
  else
  {
    IupSetfAttribute(layoutdlg->status, "TITLE", "Position:%4d,%4d | User:%4d,%4d | Natural:%4d,%4d | Current:%4d,%4d\n",
                     ih->x, ih->y, ih->userwidth, ih->userheight, ih->naturalwidth, ih->naturalheight, ih->currentwidth, ih->currentheight);
  }


  if (!ih->handle)
    IupSetAttributeId(layoutdlg->tree, "COLOR", id, "128 0 0");
  else
    IupSetAttributeId(layoutdlg->tree, "COLOR", id, "255 0 0");

  iupAttribSet(IupGetDialog(layoutdlg->tree), "_IUPLAYOUT_MARK", (char*)ih);
  IupUpdate(IupGetBrother(layoutdlg->tree));

  if (layoutdlg->properties && IupGetInt(layoutdlg->properties, "VISIBLE"))
    iupLayoutPropertiesUpdate(layoutdlg->properties, ih);
}

static Ihandle* iLayoutGetElementByPos(Ihandle* ih, int native_parent_x, int native_parent_y, int x, int y, int showhidden, int dlgvisible, int shownotmapped)
{
  Ihandle *child, *elem;
  int dx, dy;

  if ((showhidden || iLayoutElementIsVisible(ih, dlgvisible)) &&
      (shownotmapped || ih->handle))
  {
    /* check the element */
    if (x >= ih->x + native_parent_x &&
        y >= ih->y + native_parent_y &&
        x < ih->x + native_parent_x + ih->currentwidth &&
        y < ih->y + native_parent_y + ih->currentheight)
    {
      if (ih->iclass->childtype != IUP_CHILDNONE)
      {
        /* if ih is a native parent, then update the offset */
        if (ih->iclass->nativetype != IUP_TYPEVOID)
        {
          dx = 0, dy = 0;
          IupGetIntInt(ih, "CLIENTOFFSET", &dx, &dy);
          native_parent_x += ih->x + dx;
          native_parent_y += ih->y + dy;

          /* if ih is a Zbox like, then draw only the active child */
          if (IupClassMatch(ih, "zbox") || IupClassMatch(ih, "tabs") || IupClassMatch(ih, "flattabs"))
          {
            child = (Ihandle*)IupGetAttribute(ih, "VALUE_HANDLE");
            if (child)
            {
              elem = iLayoutGetElementByPos(child, native_parent_x, native_parent_y, x, y, showhidden, dlgvisible, shownotmapped);
              if (elem)
                return elem;
            }

            return ih;
          }
        }
      }

      /* check its children */
      for (child = ih->firstchild; child; child = child->brother)
      {
        elem = iLayoutGetElementByPos(child, native_parent_x, native_parent_y, x, y, showhidden, dlgvisible, shownotmapped);
        if (elem)
          return elem;
      }

      return ih;
    }
  }
  return NULL;
}

static Ihandle* iLayoutGetDialogElementByPos(iLayoutDialog* layoutdlg, int x, int y)
{
  int w, h;
  IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);

  if (layoutdlg->dialog->firstchild &&
      x >= 0 && y >= 0 &&
      x < w && y < h)
  {
    Ihandle* elem;
    Ihandle* canvas = IupGetBrother(layoutdlg->tree);
    int native_parent_x = 0, native_parent_y = 0;
    Ihandle* dlg = IupGetDialog(canvas);
    int showhidden = IupGetInt(dlg, "SHOWHIDDEN");
    int shownotmapped = layoutdlg->dialog->handle == NULL;  /* only check not mapped if dialog is also not mapped */
    int dlgvisible = IupGetInt(layoutdlg->dialog, "VISIBLE");
    IupGetIntInt(layoutdlg->dialog, "CLIENTOFFSET", &native_parent_x, &native_parent_y);
    native_parent_x -= IupGetInt(canvas, "POSX");
    native_parent_y -= IupGetInt(canvas, "POSY");
    elem = iLayoutGetElementByPos(layoutdlg->dialog->firstchild, native_parent_x, native_parent_y, x, y, showhidden, dlgvisible, shownotmapped);
    if (elem)
      return elem;
    return layoutdlg->dialog;
  }
  return NULL;
}

static void iLayoutSelectTreeItem(iLayoutDialog* layoutdlg, Ihandle* elem)
{
  int id = IupTreeGetId(layoutdlg->tree, elem);
  int old_id = IupGetInt(layoutdlg->tree, "VALUE");
  Ihandle* old_elem = (Ihandle*)IupTreeGetUserId(layoutdlg->tree, old_id);
  iLayoutTreeSetNodeColor(layoutdlg->tree, old_id, old_elem);
  IupSetInt(layoutdlg->tree, "VALUE", id);
  iLayoutUpdateMark(layoutdlg, elem, id);
}

static int iLayoutCanvasButton_CB(Ihandle* canvas, int but, int pressed, int x, int y, char* status)
{
  (void)status;
  if (but == IUP_BUTTON1 && pressed)
  {
    Ihandle* dlg = IupGetDialog(canvas);
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
    Ihandle* elem = iLayoutGetDialogElementByPos(layoutdlg, x, y);
    if (elem)
    {
      if (iup_isdouble(status))
      {
        iLayoutBlink(elem);
        IupUpdate(canvas);
      }
      else
        iLayoutSelectTreeItem(layoutdlg, elem);
    }
  }
  else if (but == IUP_BUTTON3 && pressed)
  {
    Ihandle* dlg = IupGetDialog(canvas);
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
    Ihandle* elem = iLayoutGetDialogElementByPos(layoutdlg, x, y);
    if (elem)
      iLayoutContextMenu(layoutdlg, elem, dlg);
  }
  return IUP_DEFAULT;
}

static void iLayoutGetNativeParentOffset(Ihandle* elem, int *native_parent_x, int *native_parent_y)
{
  Ihandle* native_parent = iupChildTreeGetNativeParent(elem);
  if (native_parent)
  {
    int dx = 0, dy = 0;
    IupGetIntInt(native_parent, "CLIENTOFFSET", &dx, &dy);
    *native_parent_x += native_parent->x + dx;
    *native_parent_y += native_parent->y + dy;

    if (native_parent->parent)
      iLayoutGetNativeParentOffset(native_parent, native_parent_x, native_parent_y);
  }
}

static int iSqr(int x)
{
  return x*x;
}

static int iLayoutGetBetweenPosX(Ihandle* child1, Ihandle* child2)
{
  if (child1 == NULL)
  {
    /* find previous child */
    child1 = iupChildTreeGetPrevBrother(child2);
    if (!child1)
      return child2->x;
    else
      return (child1->x + child1->currentwidth-1 + child2->x) / 2;
  }
  else /* child2 == NULL */
  {
    /* find next child */
    child2 = child1->brother;
    if (!child2)
      return child1->x + child1->currentwidth-1;
    else
      return (child1->x + child1->currentwidth-1 + child2->x) / 2;
  }
}

static int iLayoutGetBetweenPosY(Ihandle* child1, Ihandle* child2)
{
  if (child1 == NULL)
  {
    /* find previous child */
    child1 = iupChildTreeGetPrevBrother(child2);
    if (!child1)
      return child2->y;
    else
      return (child1->y + child1->currentheight-1 + child2->y) / 2;
  }
  else /* child2 == NULL */
  {
    /* find next child */
    child2 = child1->brother;
    if (!child2)
      return child1->y + child1->currentheight-1;
    else
      return (child1->y + child1->currentheight-1 + child2->y) / 2;
  }
}

static int iLayoutCanvasMotion_CB(Ihandle* canvas, int x, int y, char* status)
{
  Ihandle* container;
  Ihandle* dlg = IupGetDialog(canvas);
  Ihandle* mark = (Ihandle*)iupAttribGet(dlg, "_IUPLAYOUT_MARK");
  int container_x, container_y;
  int native_parent_x = 0, native_parent_y = 0;
  (void)status;

  if (mark->iclass->childtype == IUP_CHILDNONE)
  {
    if (iupAttribGet(canvas, "INSERTCURSOR"))
    {
      iupAttribSet(canvas, "INSERTCURSOR", NULL);
      iupAttribSet(canvas, "INSERTCURSOR_POINT", NULL);
      iupAttribSet(canvas, "INSERTCURSOR_ELEMENT_POS", NULL);
      iupAttribSet(canvas, "INSERTCURSOR_LINE", NULL);
      iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", NULL);
      iupAttribSet(canvas, "INSERTCURSOR_BEFORE", NULL);
      IupUpdate(canvas);
    }
    return IUP_DEFAULT;
  }

  x += IupGetInt(canvas, "POSX");
  y += IupGetInt(canvas, "POSY");

  container = mark;
  iLayoutGetNativeParentOffset(container, &native_parent_x, &native_parent_y);
  container_x = container->x + native_parent_x;
  container_y = container->y + native_parent_y;

  if (x >= container_x &&
      y >= container_y &&
      x < container_x + container->currentwidth &&
      y < container_y + container->currentheight)
  {
    /* check if container still has room for more children */
    if (container->iclass->childtype == IUP_CHILDMANY ||
        IupGetChildCount(container) < container->iclass->childtype - IUP_CHILDMANY)
    {
      int r_x = x - native_parent_x;
      int r_y = y - native_parent_y;

      iupAttribSet(canvas, "INSERTCURSOR", (char*)container);

      if (IupClassMatch(container, "cbox"))
      {
        iupAttribSetStrf(canvas, "INSERTCURSOR_POINT", "%d,%d", x, y);
        iupAttribSetStrf(canvas, "INSERTCURSOR_ELEMENT_POS", "%d,%d", r_x - container->x, r_y - container->y);
        iupAttribSet(canvas, "INSERTCURSOR_LINE", NULL);
        iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", NULL);
        iupAttribSet(canvas, "INSERTCURSOR_BEFORE", NULL);
        IupUpdate(canvas);
        return IUP_DEFAULT;
      }
      else 
      {
        Ihandle* child_min = NULL;
        int is_horizontal = iupStrEqualNoCase(IupGetAttribute(container, "ORIENTATION"), "HORIZONTAL");
        int is_multi = IupClassMatch(container, "gridbox") || IupClassMatch(container, "multibox");

        if (IupClassMatch(container, "zbox") || IupClassMatch(container, "tabs") || IupClassMatch(container, "flattabs"))
        {
          child_min = (Ihandle*)IupGetAttribute(container, "VALUE_HANDLE");
        }
        else
        {
          Ihandle* child;
          int d_min = 0, d;

          /* find the closest child to the cursor */
          for (child = container->firstchild; child; child = child->brother)
          {
            int c_x = child->x + child->currentwidth / 2;
            int c_y = child->y + child->currentheight / 2;

            if (is_multi)
              d = iSqr(c_x - r_x) + iSqr(c_y - r_y);
            else
            {
              if (is_horizontal)
                d = abs(c_x - r_x);
              else
                d = abs(c_y - r_y);
            }

            if (child == container->firstchild || d < d_min)
            {
              d_min = d;
              child_min = child;
            }
          }
        }

        if (!child_min) /* empty container */
        {
          if (is_horizontal)  /* insertion line will be a vertical line to mark an horizontal position */
          {
            int y1 = container_y;
            int y2 = container_y + container->currentheight - 1;
            int xx = container_x;

            iupAttribSetStrf(canvas, "INSERTCURSOR_LINE", "%d,%d,%d,%d", xx, y1, xx, y2);
            iupAttribSet(canvas, "INSERTCURSOR_POINT", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT_POS", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_BEFORE", NULL);
            IupUpdate(canvas);
            return IUP_DEFAULT;
          }
          else /* ORIENTATION=VERTICAL */    /* insertion line will be an horizontal line to mark a vertical position */
          {
            int x1 = container_x;
            int x2 = container_x + container->currentwidth - 1;
            int yy = container_y;

            iupAttribSetStrf(canvas, "INSERTCURSOR_LINE", "%d,%d,%d,%d", x1, yy, x2, yy);
            iupAttribSet(canvas, "INSERTCURSOR_POINT", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT_POS", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_BEFORE", NULL);
            IupUpdate(canvas);
            return IUP_DEFAULT;
          }
        }
        else
        {
          if (is_horizontal)  /* insertion line will be a vertical line to mark an horizontal position */
          {
            int c_x = child_min->x + child_min->currentwidth / 2;
            int xx;
            int y1;
            int y2;
            int insert_before = 0;

            if (is_multi)
            {
              y1 = native_parent_y + child_min->y;
              y2 = native_parent_y + child_min->y + child_min->currentheight - 1;

              if (r_x < c_x)
              {
                insert_before = 1;
                xx = native_parent_x + child_min->x;
              }
              else
                xx = native_parent_x + child_min->x + child_min->currentwidth - 1;
            }
            else
            {
              y1 = container_y;
              y2 = container_y + container->currentheight - 1;

              if (r_x < c_x)
              {
                insert_before = 1;
                xx = native_parent_x + iLayoutGetBetweenPosX(NULL, child_min);
              }
              else
                xx = native_parent_x + iLayoutGetBetweenPosX(child_min, NULL);
            }

            iupAttribSetStrf(canvas, "INSERTCURSOR_LINE", "%d,%d,%d,%d", xx, y1, xx, y2);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", (char*)child_min);
            iupAttribSetInt(canvas, "INSERTCURSOR_BEFORE", insert_before);
            iupAttribSet(canvas, "INSERTCURSOR_POINT", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT_POS", NULL);
            IupUpdate(canvas);
            return IUP_DEFAULT;
          }
          else /* ORIENTATION=VERTICAL */    /* insertion line will be an horizontal line to mark a vertical position */
          {
            int c_y = child_min->y + child_min->currentheight / 2;
            int yy;
            int x1;
            int x2;
            int insert_before = 0;

            if (is_multi)
            {
              x1 = native_parent_x + child_min->x;
              x2 = native_parent_x + child_min->x + child_min->currentwidth - 1;

              if (r_y < c_y)
              {
                insert_before = 1;
                yy = native_parent_y + child_min->y;
              }
              else
                yy = native_parent_y + child_min->y + child_min->currentheight - 1;
            }
            else
            {
              x1 = container_x;
              x2 = container_x + container->currentwidth - 1;

              if (r_y < c_y)
              {
                insert_before = 1;
                yy = native_parent_y + iLayoutGetBetweenPosY(NULL, child_min);
              }
              else
                yy = native_parent_y + iLayoutGetBetweenPosY(child_min, NULL);
            }

            iupAttribSetStrf(canvas, "INSERTCURSOR_LINE", "%d,%d,%d,%d", x1, yy, x2, yy);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", (char*)child_min);
            iupAttribSetInt(canvas, "INSERTCURSOR_BEFORE", insert_before);
            iupAttribSet(canvas, "INSERTCURSOR_POINT", NULL);
            iupAttribSet(canvas, "INSERTCURSOR_ELEMENT_POS", NULL);
            IupUpdate(canvas);
            return IUP_DEFAULT;
          }
        }
      }
    }
  }
  
  if (iupAttribGet(canvas, "INSERTCURSOR"))
  {
    iupAttribSet(canvas, "INSERTCURSOR", NULL);
    iupAttribSet(canvas, "INSERTCURSOR_POINT", NULL);
    iupAttribSet(canvas, "INSERTCURSOR_ELEMENT_POS", NULL);
    iupAttribSet(canvas, "INSERTCURSOR_LINE", NULL);
    iupAttribSet(canvas, "INSERTCURSOR_ELEMENT", NULL);
    iupAttribSet(canvas, "INSERTCURSOR_BEFORE", NULL);
    IupUpdate(canvas);
  }
  return IUP_DEFAULT;
}

static int iLayoutCanvasResize_CB(Ihandle* canvas, int canvas_w, int canvas_h)
{
  IupSetInt(canvas, "DX", canvas_w);
  IupSetInt(canvas, "DY", canvas_h);
  return IUP_DEFAULT;
}

/***************************************************************************
                              Layout Tree
***************************************************************************/


static int iLayoutTreeExecuteLeaf_CB(Ihandle* tree, int id)
{
  Ihandle* elem = (Ihandle*)IupTreeGetUserId(tree, id);
  iLayoutBlink(elem);
  return IUP_DEFAULT;
}

static int iLayoutTreeRightClick_CB(Ihandle* tree, int id)
{
  Ihandle* dlg = IupGetDialog(tree);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)IupTreeGetUserId(tree, id);
  iLayoutContextMenu(layoutdlg, elem, dlg);
  return IUP_DEFAULT;
}

static int iLayoutTreeDragDrop_CB(Ihandle* tree, int drag_id, int drop_id, int isshift, int iscontrol)
{
  Ihandle* dlg = IupGetDialog(tree);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  Ihandle* drag_elem = (Ihandle*)IupTreeGetUserId(tree, drag_id);
  Ihandle* drop_elem = (Ihandle*)IupTreeGetUserId(tree, drop_id);
  int error;

  /* no support for copy */
  if (iscontrol)
  {
    IupMessage("Error", "Copy not supported for drag&drop.");
    return IUP_IGNORE;
  }

  if (drag_elem->flags & IUP_INTERNAL)
  {
    IupMessage("Error", "Can NOT drag an internal element. This element exists only inside this container.");
    return IUP_IGNORE;
  }

  if (iupStrEqualNoCase(IupGetAttributeId(tree, "KIND", drop_id), "BRANCH") &&
      iupStrEqualNoCase(IupGetAttributeId(tree, "STATE", drop_id), "EXPANDED"))
  {
    Ihandle* ref_child = drop_elem->firstchild;   /* the first child as reference */

    /* if first element is internal, use the next one. */
    if (drop_elem->firstchild && (drop_elem->firstchild->flags & IUP_INTERNAL))
    {
      /* the first child is internal, so use brother as reference */
      if (drop_elem->firstchild->brother)
        ref_child = drop_elem->firstchild->brother;
    }

    if (drop_elem == drag_elem->parent && ref_child == drag_elem)
    {
      /* dropped at the same place, just ignore it */
      return IUP_IGNORE;
    }

    /* If the drop node is a branch and it is expanded, */
    /* add as first child */
    error = IupReparent(drag_elem, drop_elem, ref_child);  /* add before the reference */
  }
  else
  {
    if (!drop_elem->parent)
    {
      IupMessage("Error", "Can NOT drop here as brother.");
      return IUP_IGNORE;
    }

    if (drop_elem->parent == drag_elem->parent && drop_elem->brother == drag_elem)
    {
      /* dropped at the same place, just ignore it */
      return IUP_IGNORE;
    }

    /* If the branch is not expanded or the drop node is a leaf, */
    /* add as brother after reference */
    error = IupReparent(drag_elem, drop_elem->parent, drop_elem->brother);  /* drop_elem->brother can be NULL here */
  }

  if (error == IUP_ERROR)
  {
    IupMessage("Error", "Drop failed. Invalid operation for this node.");
    return IUP_IGNORE;
  }

  layoutdlg->changed = 1;

  iLayoutUpdateLayout(layoutdlg);

  /* since we are only moving existing nodes,
     title, map state, and user data was not changed.
     there is no need to update the node info */

  (void)isshift;
  return IUP_CONTINUE;  /* the nodes of the tree will be automatically moved */
}

static int iLayoutTreeSelection_CB(Ihandle* tree, int id, int status)
{
  Ihandle* elem = (Ihandle*)IupTreeGetUserId(tree, id);
  if (status == 1)
  {
    Ihandle* dlg = IupGetDialog(tree);
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
    iLayoutUpdateMark(layoutdlg, elem, id);
  }
  else
    iLayoutTreeSetNodeColor(tree, id, elem);
  return IUP_DEFAULT;
}


/***************************************************************************
                            Layout Dialog Callbacks
***************************************************************************/


static int iLayoutDialogKAny_CB(Ihandle* dlg, int key)
{
  switch (key)
  {
  case K_DEL:
    return iLayoutContextMenuRemove_CB(dlg);
  case K_F5:
    return iLayoutMenuUpdate_CB(dlg);
  case K_ESC:
    return iLayoutMenuClose_CB(dlg);
  case K_cO:
    return iLayoutMenuLoad_CB(dlg);
  case K_cF5:
    return iLayoutMenuRefresh_CB(dlg);
  case K_F3:
  {
    Ihandle* find_dlg = (Ihandle*)IupGetAttribute(dlg, "FIND_DIALOG");
    if (!find_dlg)
      return iLayoutMenuFindElement_CB(dlg);
    else
      return iLayoutFindDialogNext_CB(find_dlg);
  }
  case K_cF:
    return iLayoutMenuFindElement_CB(dlg);
  case K_cMinus:
  case K_cPlus:
    {
      int opacity = IupGetInt(dlg, "OPACITY");
      if (opacity == 0)
        opacity = 255;
      if (key == K_cPlus)
        opacity++;
      else
        opacity--;
      if (opacity == 0 || opacity == 255)
        IupSetAttribute(dlg, "OPACITY", NULL);
      else
        IupSetInt(dlg, "OPACITY", opacity);
      break;
    }
  }

  return IUP_DEFAULT;
}

static int iLayoutDialogClose_CB(Ihandle* dlg)
{
  if (IupGetInt(dlg, "DESTROYWHENCLOSED"))
  {
    IupDestroy(dlg);
    return IUP_IGNORE;
  }
  return IUP_DEFAULT;
}

static int iLayoutDialogDestroy_CB(Ihandle* dlg)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupDestroy(layoutdlg->timer);
  if (iupObjectCheck(layoutdlg->globals))
    IupDestroy(layoutdlg->globals);
  if (iupObjectCheck(layoutdlg->properties))
    IupDestroy(layoutdlg->properties);
  if (layoutdlg->destroy && iupObjectCheck(layoutdlg->dialog))
    IupDestroy(layoutdlg->dialog);
  free(layoutdlg);
  return IUP_DEFAULT;
}

IUP_API Ihandle* IupLayoutDialog(Ihandle* dialog)
{
  Ihandle *tree, *canvas, *dlg, *menu, *status, *split;
  iLayoutDialog* layoutdlg;

  layoutdlg = calloc(1, sizeof(iLayoutDialog));
  if (dialog)
    layoutdlg->dialog = dialog;
  else
  {
    layoutdlg->dialog = IupDialog(NULL);
    layoutdlg->destroy = 1;
  }

  layoutdlg->timer = IupTimer();
  IupSetCallback(layoutdlg->timer, "ACTION_CB", iLayoutTimerAutoUpdate_CB);
  IupSetAttribute(layoutdlg->timer, "TIME", "300");
  IupSetAttribute(layoutdlg->timer, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);

  canvas = IupCanvas(NULL);
  IupSetCallback(canvas, "ACTION", (Icallback)iLayoutCanvas_CB);
  IupSetCallback(canvas, "BUTTON_CB", (Icallback)iLayoutCanvasButton_CB);
  IupSetCallback(canvas, "MOTION_CB", (Icallback)iLayoutCanvasMotion_CB);
  IupSetCallback(canvas, "RESIZE_CB", (Icallback)iLayoutCanvasResize_CB);
  IupSetAttribute(canvas, "SCROLLBAR", "YES");
  IupSetAttribute(canvas, "XAUTOHIDE", "NO");
  IupSetAttribute(canvas, "YAUTOHIDE", "NO");
  IupSetAttribute(canvas, "BORDER", "NO");

  tree = IupTree();
  layoutdlg->tree = tree;
  IupSetAttribute(tree, "NAME", "TREE");
  IupSetAttribute(tree, "RASTERSIZE", NULL);
  IupSetAttribute(tree, "USERSIZE", "200x");
  IupSetAttribute(tree, "SHOWDRAGDROP", "Yes");
  IupSetCallback(tree, "SELECTION_CB", (Icallback)iLayoutTreeSelection_CB);
  IupSetCallback(tree, "EXECUTELEAF_CB", (Icallback)iLayoutTreeExecuteLeaf_CB);
  IupSetCallback(tree, "RIGHTCLICK_CB", (Icallback)iLayoutTreeRightClick_CB);
  IupSetCallback(tree, "DRAGDROP_CB", (Icallback)iLayoutTreeDragDrop_CB);
  IupSetAttribute(tree, "IMAGELEAF", "IMGEMPTY");

  status = IupLabel(NULL);
  IupSetAttribute(status, "EXPAND", "HORIZONTAL");
  IupSetAttribute(status, "FONT", "Courier, 11");
  IupSetAttribute(status, "SIZE", "x20");
  layoutdlg->status = status;

  split = IupSplit(tree, canvas);
  IupSetAttribute(split, "VALUE", "300");
  IupSetAttribute(split, "AUTOHIDE", "Yes");


  menu = IupMenu(
    IupSubmenu("&Layout", IupMenu(
    IupSetCallbacks(IupSetAttributes(IupItem("&Show Tree", NULL), "AUTOTOGGLE=YES, VALUE=ON"), "ACTION", iLayoutMenuHierarchy_CB, NULL),
    IupSetCallbacks(IupItem("Refresh\tCtrl+F5", NULL), "ACTION", iLayoutMenuRefresh_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("Update (Tree and Draw)\tF5", NULL), "ACTION", iLayoutMenuUpdate_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Auto Update Draw", NULL), "AUTOTOGGLE=YES, VALUE=OFF"), "ACTION", iLayoutMenuAutoUpdate_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Show Hidden", NULL), "AUTOTOGGLE=YES, VALUE=OFF"), "ACTION", iLayoutMenuShowHidden_CB, NULL),
    IupSetCallbacks(IupSetAttributes(IupItem("Show Internal", NULL), "AUTOTOGGLE=YES, VALUE=OFF"), "ACTION", iLayoutMenuShowInternal_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("Opacity\tCtrl+/Ctrl-", NULL), "ACTION", iLayoutMenuOpacity_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("Find Element...\tCtrl+F", NULL), "ACTION", iLayoutMenuFindElement_CB, NULL),
    NULL)),
    NULL);

  if (!dialog || !iupAttribGet(dialog, "_IUPLED_FILENAME"))
  {
    Ihandle* dlg_menu = IupSubmenu("&Dialog", IupMenu(
      IupSetCallbacks(IupItem("New", NULL), "ACTION", iLayoutMenuNew_CB, NULL),
      IupSetCallbacks(IupItem("Load...\tCtrl+O", NULL), "ACTION", iLayoutMenuLoad_CB, NULL),
      IupSetCallbacks(IupItem("Load Visible...", NULL), "ACTION", iLayoutMenuLoadVisible_CB, NULL),
      IupSubmenu("&Export", IupMenu(
      IupSetCallbacks(IupItem("C...", NULL), "ACTION", iLayoutMenuExportC_CB, NULL),
      IupSetCallbacks(IupItem("LED...", NULL), "ACTION", iLayoutMenuExportLED_CB, NULL),
      IupSetCallbacks(IupItem("Lua...", NULL), "ACTION", iLayoutMenuExportLua_CB, NULL),
      NULL)),
      IupSeparator(),
      IupSetCallbacks(IupItem("Redraw", NULL), "ACTION", iLayoutMenuRedraw_CB, NULL),
      IupSetCallbacks(IupItem("Show", NULL), "ACTION", iLayoutMenuShow_CB, NULL),
      IupSetCallbacks(IupItem("Hide", NULL), "ACTION", iLayoutMenuHide_CB, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("&Globals...", NULL), "ACTION", iLayoutMenuGlobals_CB, NULL),
      IupSetCallbacks(IupItem("&Close\tEsc", NULL), "ACTION", iLayoutMenuClose_CB, NULL),
      NULL));

    IupInsert(menu, NULL, dlg_menu);
  }

  dlg = IupDialog(IupVbox(split, status, NULL));
  IupSetAttribute(dlg, "TITLE", "Dialog Layout");
  IupSetAttribute(dlg, "SHRINK", "Yes");
  IupSetAttribute(dlg, "PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dlg, "TREE", (char*)tree);
  IupSetAttribute(dlg, "ICON", IupGetGlobal("ICON"));
  IupSetCallback(dlg, "DESTROY_CB", iLayoutDialogDestroy_CB);
  IupSetCallback(dlg, "K_ANY", (Icallback)iLayoutDialogKAny_CB);
  IupSetCallback(dlg, "CLOSE_CB", iLayoutDialogClose_CB);
  iupAttribSet(dlg, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);
  IupSetAttributeHandle(dlg, "MENU", menu);
  iupAttribSet(dlg, "OPACITY", "255");

  iupAttribSet(dlg, "DESTROYWHENCLOSED", "Yes");

  {
    int w = 0, h = 0;
    IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
    if (w == 0 && h == 0)
    {
      IupRefresh(layoutdlg->dialog);
      IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
    }

    if (w && h)
    {
      int sb = iupdrvGetScrollbarSize();
      w += sb;
      h += sb;
      IupSetfAttribute(canvas, "USERSIZE", "%dx%d", w, h);
    }
    else
      IupSetAttribute(canvas, "USERSIZE", "600x400");
  }

  IupMap(dlg);

  IupSetAttribute(canvas, "USERSIZE", NULL);

  iLayoutTreeRebuild(layoutdlg);

  return dlg;
}
