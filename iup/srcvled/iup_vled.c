#if 0
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iup.h>
#include <iupcbs.h>
#include <iup_scintilla.h>
#include <iup_config.h>

#include "iup_str.h"
#include "iup_object.h"
#include "iup_ledlex.h"
#include "iup_attrib.h"

#define MAX_NAMES 5000
#define FOLDING_MARGIN "20"

static Ihandle* load_image_shortcut(void)
{
  unsigned char imgdata[] = {
    108, 110, 114, 180, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 114, 117, 120, 194,
    117, 117, 118, 255, 244, 255, 255, 236, 236, 253, 255, 226, 234, 251, 255, 226, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 251, 255, 255, 225, 255, 255, 255, 235, 117, 117, 118, 255,
    117, 117, 118, 255, 235, 253, 255, 226, 226, 242, 255, 216, 232, 246, 255, 216, 127, 160, 199, 237, 12, 69, 140, 255, 25, 79, 146, 255, 37, 88, 153, 255, 38, 89, 152, 255, 38, 89, 152, 255, 38, 89, 152, 255, 26, 81, 148, 255, 38, 88, 153, 255, 253, 255, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 236, 254, 255, 226, 224, 240, 254, 217, 228, 242, 255, 216, 251, 255, 255, 212, 168, 193, 221, 229, 33, 86, 150, 254, 25, 80, 146, 255, 31, 84, 151, 255, 31, 85, 149, 255, 31, 85, 150, 255, 21, 76, 144, 255, 32, 81, 146, 255, 252, 254, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 237, 254, 255, 226, 226, 240, 254, 217, 227, 242, 255, 217, 232, 247, 255, 216, 255, 255, 255, 208, 123, 158, 197, 239, 19, 74, 143, 255, 32, 86, 151, 255, 33, 86, 151, 255, 33, 86, 151, 255, 22, 76, 144, 255, 36, 83, 144, 254, 253, 255, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 238, 254, 255, 226, 227, 243, 255, 217, 228, 243, 255, 217, 253, 255, 255, 213, 182, 205, 228, 228, 34, 86, 152, 255, 32, 85, 151, 255, 33, 86, 151, 255, 33, 86, 151, 255, 33, 87, 152, 255, 23, 76, 144, 255, 36, 83, 145, 254, 253, 255, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 239, 255, 255, 226, 229, 243, 255, 217, 245, 255, 255, 213, 205, 221, 239, 223, 40, 91, 154, 255, 30, 84, 149, 255, 33, 86, 152, 255, 33, 86, 151, 255, 30, 82, 148, 255, 28, 81, 147, 255, 22, 76, 143, 255, 37, 83, 145, 254, 252, 254, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 240, 255, 255, 226, 234, 248, 255, 216, 244, 254, 255, 214, 75, 117, 172, 251, 22, 78, 146, 255, 35, 87, 151, 255, 32, 86, 151, 255, 25, 78, 144, 255, 24, 74, 139, 255, 32, 81, 144, 255, 12, 66, 135, 255, 35, 81, 143, 255, 253, 255, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 242, 255, 255, 226, 246, 255, 255, 213, 194, 213, 234, 225, 24, 78, 145, 255, 34, 87, 152, 255, 32, 86, 151, 255, 25, 78, 144, 255, 3, 57, 128, 255, 165, 187, 212, 231, 199, 214, 231, 225, 0, 56, 127, 255, 24, 72, 136, 255, 253, 255, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 244, 255, 255, 226, 254, 255, 255, 213, 148, 176, 209, 234, 16, 73, 143, 255, 34, 87, 152, 255, 28, 81, 147, 255, 4, 58, 128, 255, 137, 163, 197, 236, 255, 255, 255, 212, 255, 255, 255, 212, 170, 191, 214, 231, 15, 64, 130, 255, 252, 254, 255, 217, 255, 255, 255, 224, 117, 117, 118, 255,
    117, 117, 118, 255, 245, 255, 255, 226, 255, 255, 255, 213, 146, 175, 208, 234, 13, 70, 140, 255, 32, 85, 150, 255, 8, 64, 133, 255, 113, 143, 184, 241, 255, 255, 255, 212, 249, 255, 255, 216, 248, 254, 255, 216, 255, 255, 255, 213, 173, 191, 215, 230, 253, 255, 255, 217, 255, 255, 255, 225, 117, 117, 118, 255,
    117, 117, 118, 255, 247, 255, 255, 226, 252, 255, 255, 213, 187, 207, 229, 226, 8, 67, 139, 255, 24, 78, 146, 255, 25, 74, 139, 255, 233, 242, 249, 219, 255, 255, 255, 215, 244, 252, 254, 217, 246, 251, 254, 217, 248, 253, 255, 217, 255, 255, 255, 215, 253, 255, 255, 217, 255, 255, 255, 225, 117, 117, 118, 255,
    117, 117, 118, 255, 249, 255, 255, 226, 241, 251, 255, 216, 248, 255, 255, 216, 62, 107, 163, 250, 0, 46, 123, 255, 51, 96, 153, 252, 255, 255, 255, 213, 252, 255, 255, 216, 246, 252, 254, 217, 247, 252, 254, 217, 248, 252, 254, 217, 250, 253, 254, 217, 252, 254, 255, 217, 255, 255, 255, 225, 117, 117, 118, 255,
    117, 117, 118, 255, 250, 255, 255, 226, 238, 248, 255, 216, 252, 255, 255, 215, 236, 243, 250, 219, 83, 120, 168, 245, 0, 55, 126, 255, 85, 123, 171, 242, 199, 214, 231, 225, 250, 255, 255, 216, 250, 253, 255, 217, 251, 254, 255, 217, 252, 254, 255, 217, 253, 255, 255, 217, 255, 255, 255, 225, 117, 117, 118, 255,
    117, 117, 118, 255, 255, 255, 255, 235, 253, 255, 255, 225, 255, 255, 255, 225, 255, 255, 255, 223, 255, 255, 255, 222, 255, 255, 255, 227, 255, 255, 255, 225, 255, 255, 255, 225, 255, 255, 255, 227, 255, 255, 255, 225, 255, 255, 255, 225, 255, 255, 255, 225, 255, 255, 255, 225, 255, 255, 255, 234, 117, 117, 118, 255,
    110, 112, 115, 194, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 117, 117, 118, 255, 127, 128, 128, 203 };

  Ihandle* image = IupImageRGBA(16, 16, imgdata);
  return image;
}

static int isAlien(Ihandle *elem, const char* filename)
{
  char *elem_file;

  if (!elem || !filename)
    return 0;

  elem_file = IupGetAttribute(elem, "VLED_FILENAME");

  if (elem_file && !iupStrEqual(elem_file, filename))
    return 1;

  return 0;
}

static Ihandle* get_current_multitext(Ihandle* ih)
{
  Ihandle* tabs = IupGetDialogChild(ih, "MULTITEXT_TABS");
  return (Ihandle*)IupGetAttribute(tabs, "VALUE_HANDLE");
}

static char* vLedGetElementTreeTitle(Ihandle* ih)
{
  char* title = iupAttribGetLocal(ih, "TITLE");
  char* name = IupGetName(ih);
  char* className = IupGetClassName(ih);
  char* str = iupStrGetMemory(200);
  if (iupStrEqual(className, "user") != 0)
    name = IupGetAttribute(ih, "LEDPARSER_NAME");
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
      sprintf(str, "[%s] %.50s \"%.50s\"", className, title, name);
    else
      sprintf(str, "[%s] %.50s", className, title);
  }
  else
  {
    if (name)
      sprintf(str, "[%s] \"%.50s\"", className, name);
    else
      sprintf(str, "[%s]", className);
  }
  return str;
}

static void vLedTreeSetNodeInfo(Ihandle* tree, int id, Ihandle* ih, int link)
{
  IupSetAttributeId(tree, "TITLE", id, vLedGetElementTreeTitle(ih));
  //iLayoutTreeSetNodeColor(tree, id, ih);
  IupTreeSetUserId(tree, id, ih);
  if (link)
  {
    if (ih->iclass->childtype != IUP_CHILDNONE)
      IupSetAttributeId(tree, "IMAGEEXPANDED", id, IupGetAttribute(tree, "IMG_SHORTCUT"));
    else
      IupSetAttributeId(tree, "IMAGE", id, IupGetAttribute(tree, "IMG_SHORTCUT"));
  }
}

static Ihandle* vLedTreeGetFirstChild(Ihandle* ih)
{
  Ihandle* firstchild = ih->parent->firstchild;

  while (firstchild && firstchild->flags & IUP_INTERNAL)
    firstchild = firstchild->brother;

  return firstchild;
}

static int vLedTreeAddNode(Ihandle* tree, int id, Ihandle* ih, const char *filename)
{
  int link = 0;

  if (ih->iclass->childtype != IUP_CHILDNONE && !IupGetInt(ih, "LEDPARSER_NOTDEFINED"))
  {
    if (!ih->parent || isAlien(ih->parent, filename) || ih == vLedTreeGetFirstChild(ih))
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
    if (!ih->parent || isAlien(ih->parent, filename) || ih == vLedTreeGetFirstChild(ih))
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

  if (isAlien(ih, filename) || iupStrEqual(ih->iclass->name, "user"))
    link = 1;

  vLedTreeSetNodeInfo(tree, id, ih, link);
  return id;
}

static int vLedTreeAddChildren(Ihandle* tree, int parent_id, Ihandle* parent, const char *filename)
{
  Ihandle *child;
  int last_child_id = parent_id;

  for (child = parent->firstchild; child; child = child->brother)
  {
    if (!(child->flags & IUP_INTERNAL))
    {
      last_child_id = vLedTreeAddNode(tree, last_child_id, child, filename);

      if (child->iclass->childtype != IUP_CHILDNONE && !isAlien(child, filename))
        vLedTreeAddChildren(tree, last_child_id, child, filename);
    }
  }

  return last_child_id;
}

static void mainUpdateList(Ihandle* tree, const char* file_name)
{
  char *names[MAX_NAMES];
  int i, num_names = IupGetAllNames(names, MAX_NAMES);
  int last_child_id = 0;

  IupSetAttribute(tree, "DELNODE0", "CHILDREN");

  IupStoreAttributeId(tree, "TITLE", 0, file_name);

  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
    Ihandle*elem_parent = elem->parent;

    if (IupGetInt(elem, "VLED_INTERNAL") != 0 || (elem && isAlien(elem, file_name)) || (elem_parent && !isAlien(elem_parent, file_name)))
      continue;

    last_child_id = vLedTreeAddNode(tree, last_child_id, elem, file_name);
      
    if (elem->iclass->childtype != IUP_CHILDNONE && !isAlien(elem, file_name))
      last_child_id = vLedTreeAddChildren(tree, last_child_id, elem, file_name);

    last_child_id = 0;
  }

  IupSetAttribute(tree, "VALUE", "ROOT");
}

static void mainFlagInternalElements(void)
{
  char *names[MAX_NAMES];
  int i, num_names = IupGetAllNames(names, MAX_NAMES);
  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);

    if (!IupGetAttribute(elem, "VLED_FILENAME") && !IupGetInt(elem, "VLED_INTERNAL"))
      IupSetAttribute(elem, "VLED_INTERNAL", "YES");
  }
}

static void mainFlagLedElements(const char *file_name)
{
  char *names[MAX_NAMES];
  int i, num_names = IupGetAllNames(names, MAX_NAMES);
  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);

    if (!IupGetAttribute(elem, "VLED_FILENAME"))
      IupStoreAttribute(elem, "VLED_FILENAME", file_name);
  }
}

static int load_led(Ihandle *list, const char *file_name)
{
  Ihandle* multitext = get_current_multitext(list);
  char* error;

  mainFlagInternalElements();

  IupSetInt(multitext, "MARKERDELETEALL", 1);

  error = iupLoadLed(file_name, 1, 0);
  if (error)
  {
    int line;
    const char* error_mark = "bad input at line";
    const char* error_line = strstr(error, error_mark);
    error_line += strlen(error_mark) + 1;
    sscanf(error_line, "%d", &line);
    IupMessageError(IupGetDialog(list), error);
    IupSetIntId(multitext, "MARKERADD", line-1, 1);
    IupSetStrf(multitext, "CARET", "%d,0", line-1);
  }
  else
  {
    mainFlagLedElements(file_name);
    mainUpdateList(list, file_name);
  }

  return IUP_DEFAULT;
}

static void unloadNamedElements(Ihandle *element)
{
  Ihandle *child;

  for (child = element->firstchild; child; child = child->brother)
  {
    if (child->iclass->childtype != IUP_CHILDNONE)
      unloadNamedElements(child);

    if (IupGetName(child))
      IupDetach(child);
  }
}

static int unload_led(char *file_name)
{
  char *names[MAX_NAMES];
  int i, num_names = IupGetAllNames(names, MAX_NAMES);

  for (i = 0; i < num_names; i++)
  {
    Ihandle *element, *parent, *brother;
    char title[80], *name = names[i];

    element = IupGetHandle(name);
    if (!element)
      continue;

    if (IupGetInt(element, "VLED_INTERNAL") != 0 || isAlien(element, file_name))
      continue;

    if (element->iclass->childtype != IUP_CHILDNONE)
      unloadNamedElements(element);

    parent = element->parent;
    strcpy(title, name);

    if (parent && isAlien(parent, file_name))
      brother = element->brother;

    IupDestroy(element);

    if (parent && isAlien(parent, file_name))
    {
      Ihandle *user = IupUser();
      IupSetAttribute(user, "LEDPARSER_NOTDEFINED", "1");
      IupStoreAttribute(user, "LEDPARSER_NAME", title);
      IupInsert(parent, brother, user);
    }
  }

  return IUP_DEFAULT;
}

static Ihandle* get_config(Ihandle* ih)
{
  Ihandle* config = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "CONFIG_HANDLE");
  return config;
}

static int item_help_action_cb(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

static int item_about_action_cb(void)
{
  IupMessage("About", "   IUP Visual LED\n\nAuthors:\n   Camilo Freire\n   Antonio Scuri");
  return IUP_DEFAULT;
}

static char* getLedKeywords(void)
{
  return "IMAGE IMAGERGB IMAGERGBA TIMER USER BUTTON CANVAS FLATBUTTON FLATTOGGLE DIALOG FILL FILEDLG MESSAGEDLG COLORDLG FONTDLG PROGRESSBAR FRAME FLATFRAME HBOX ITEM LABEL FLATLABEL LIST "
    "SBOX SCROLLBOX FLATSCROLLBOX DETACHBOX BACKGROUNDBOX EXPANDER DROPBUTTON MENU MULTILINE RADIO SEPARATOR SUBMENU TEXT VAL TREE TABS FLATTABS TOGGLE VBOX ZBOX GRIDBOX NORMALIZER LINK "
    "CBOX FLATSEPARATOR SPACE SPIN SPINBOX SPLIT GAUGE COLORBAR COLORBROWSER DIAL ANIMATEDLABEL CELLS MATRIX MATRIXLIST MATRIXEX GLCANVAS GLBACKGROUNDBOX OLECONTROL PLOT MGLPLOT SCINTILLA "
    "WEBBROWSER GLCANVASBOX GLSUBCANVAS GLLABEL GLSEPARATOR GLBUTTON GLTOGGLE GLTEXT GLPROGRESSBAR GLVAL GLLINK GLFRAME GLEXPANDER GLSCROLLBOX GLSIZEBOX FLATMULTIBOX FLATLIST FLATVAL";
}

static const char *getLastNonAlphaNumeric(const char *text)
{
  int len = (int)strlen(text);
  const char *c = text + len - 1;
  if (*c == '\n')
    c--;
  for (; c != text; c--)
  {
    if (*c < 48 || (*c > 57 && *c < 65) || (*c > 90 && *c < 97) || *c > 122)
      return c + 1;
  }
  return NULL;
}

static char *filterList(const char *text, const char *list)
{
  char *filteredList[1024];
  char *retList;
  int count = 0;

  int i, len;
  const char *lastValue = list;
  const char *nextValue = iupStrNextValue(list, (int)strlen(list), &len, ' ');
  while (len != 0)
  {
    if ((int)strlen(text) <= len && iupStrEqualPartial(lastValue, text))
    {
      char *value = malloc(80);

      strncpy(value, lastValue, len);
      value[len] = 0;
      filteredList[count++] = value;
    }
    lastValue = nextValue;
    nextValue = iupStrNextValue(nextValue, (int)strlen(nextValue), &len, ' ');
  }

  retList = malloc(1024);
  retList[0] = 0;
  for (i = 0; i < count; i++)
  {
    if (i == 0)
    {
      strcpy(retList, filteredList[i]);
      strcat(retList, " ");
    }
    else
    {
      strcat(retList, filteredList[i]);
      strcat(retList, " ");
    }
  }

  for (i = 0; i < count; i++)
    free(filteredList[i]);

  return retList;
}

static int multitext_valuechanged_cb(Ihandle* multitext)
{
  if (IupGetInt(multitext, "AUTOCOMPLETION"))
  {
    const char *t;
    int pos = IupGetInt(multitext, "CARETPOS");
    char *text = IupGetAttribute(multitext, "VALUE");
    text[pos + 1] = '\0';
    t = getLastNonAlphaNumeric(text);
    if (t != NULL && *t != '\n' && *t != 0)
    {
      char *fList = filterList(t, getLedKeywords());
      if (strlen(fList) > 0)
        IupSetAttributeId(multitext, "AUTOCSHOW", (int)strlen(t) - 1, fList);
      free(fList);
    }

    return IUP_DEFAULT;
  }

  return IUP_CONTINUE;
}

static int multitext_kesc_cb(Ihandle *multitext)
{
  if (!IupGetInt(multitext, "AUTOCOMPLETION"))
    return IUP_CONTINUE;

  if (IupGetInt(multitext, "AUTOCACTIVE"))
    IupSetAttribute(multitext, "AUTOCCANCEL", "YES");

  return IUP_CONTINUE;
}


static int multitext_map_cb(Ihandle* multitext)
{
  Ihandle* config = get_config(multitext);
  const char *value;

  IupSetAttribute(multitext, "LEXERLANGUAGE", "led");
  IupSetAttribute(multitext, "KEYWORDS0", getLedKeywords());

  IupSetAttribute(multitext, "STYLEFGCOLOR1", "0 128 0");    /* 1-Led comment */
  IupSetAttribute(multitext, "STYLEFGCOLOR2", "255 128 0");  /* 2-Number  */
  IupSetAttribute(multitext, "STYLEFGCOLOR3", "0 0 255");    /* 3-Keyword  */
  IupSetAttribute(multitext, "STYLEFGCOLOR4", "164 0 164");  /* 4-String  */
  IupSetAttribute(multitext, "STYLEFGCOLOR5", "164 0 164");  /* 5-Character  */
  IupSetAttribute(multitext, "STYLEFGCOLOR6", "164 0 0");   /* 6-Operator  */
  /* 3, 8 and 9 - are not used */
  IupSetAttribute(multitext, "STYLEBOLD10", "YES");

  IupSetAttribute(multitext, "MARKERHIGHLIGHT", "YES");

  IupSetAttributeId(multitext, "MARKERBGCOLOR", 25, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDEREND */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 25, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDEREND */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 26, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDEROPENMID */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 26, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDEROPENMID */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 27, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDERMIDTAIL */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 27, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDERMIDTAIL */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 28, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDERTAIL */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 28, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDERTAIL */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 29, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDERSUB */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 29, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDERSUB */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 30, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDER */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 30, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDER */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 31, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDEROPEN */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 31, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDEROPEN */

  IupSetAttribute(multitext, "PROPERTY", "fold=1");
  IupSetAttribute(multitext, "PROPERTY", "fold.compact=0"); /* avoid folding of blank lines */
  IupSetAttribute(multitext, "_IUP_FOLDDING", "1");

  /* Folding margin=3 */
  IupSetAttribute(multitext, "MARGINWIDTH3", FOLDING_MARGIN);
  IupSetAttribute(multitext, "MARGINMASKFOLDERS3", "Yes");
  IupSetAttribute(multitext, "MARGINSENSITIVE3", "YES");

  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDER=BOXPLUS");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDEROPEN=BOXMINUS");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDEREND=BOXPLUSCONNECTED");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDERMIDTAIL=TCORNER");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDEROPENMID=BOXMINUSCONNECTED");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDERSUB=VLINE");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDERTAIL=LCORNER");

  IupSetAttribute(multitext, "FOLDFLAGS", "LINEAFTER_CONTRACTED");

  value = IupConfigGetVariableStr(config, "LedScripter", "CommentColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR1", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "CommentColor", IupGetAttribute(multitext, "STYLEFGCOLOR1"));

  value = IupConfigGetVariableStr(config, "LedScripter", "NumberColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR2", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "NumberColor", IupGetAttribute(multitext, "STYLEFGCOLOR4"));

  value = IupConfigGetVariableStr(config, "LedScripter", "KeywordColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR3", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "KeywordColor", IupGetAttribute(multitext, "STYLEFGCOLOR5"));

  value = IupConfigGetVariableStr(config, "LedScripter", "StringColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR4", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "StringColor", IupGetAttribute(multitext, "STYLEFGCOLOR6"));

  value = IupConfigGetVariableStr(config, "LedScripter", "CharacterColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR5", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "CharacterColor", IupGetAttribute(multitext, "STYLEFGCOLOR7"));

  value = IupConfigGetVariableStr(config, "LedScripter", "OperatorColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR6", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "OperatorColor", IupGetAttribute(multitext, "STYLEFGCOLOR10"));

  value = IupConfigGetVariableStr(config, "LedScripter", "AutoCompletion");
  if (value)
    IupSetStrAttribute(multitext, "AUTOCOMPLETION", value);

  value = IupConfigGetVariableStr(config, "LedScripter", "Folding");
  if (iupStrBoolean(value))
  {
    IupSetAttribute(multitext, "MARGINWIDTH3", FOLDING_MARGIN);
    IupSetAttribute(multitext, "PROPERTY", "fold=1");
    IupSetAttribute(multitext, "_IUP_FOLDDING", "1");
    IupSetAttribute(multitext, "FOLDALL", "EXPAND");
  }
  else
  {
    IupSetAttribute(multitext, "MARGINWIDTH3", "0");
    IupSetAttribute(multitext, "PROPERTY", "fold=0");
    IupSetAttribute(multitext, "_IUP_FOLDDING", NULL);
  }

  return IUP_DEFAULT;
}

static int tabChange_cb(Ihandle* tabs, Ihandle* new_tab, Ihandle* old_tab)
{
  char *filename = IupGetAttribute(new_tab, "FILENAME");
  Ihandle* elementsList = IupGetDialogChild(tabs, "ELEMENTS_TREE");

  IFnnn oldTabChange_cb = (IFnnn)IupGetCallback(tabs, "OLDTABCHANGE_CB");
  if (oldTabChange_cb)
    oldTabChange_cb(tabs, new_tab, old_tab);

  mainUpdateList(elementsList, filename);

  return IUP_DEFAULT;
}

static int loadfile_cb(Ihandle* self, char *t)
{
  Ihandle* elementsList = IupGetDialogChild(self, "ELEMENTS_TREE");

  load_led(elementsList, t);

  return IUP_DEFAULT;
}

static int savefile_cb(Ihandle* self, char *t)
{
  Ihandle* elementsList = IupGetDialogChild(self, "ELEMENTS_TREE");

  unload_led(t);

  load_led(elementsList, t);

  return IUP_DEFAULT;
}

static int newtext_cb(Ihandle* ih, Ihandle *multitext)
{
  (void)ih;
  /* this is called before the multitext is mapped */
  IupSetCallback(multitext, "VALUECHANGED_CB", (Icallback)multitext_valuechanged_cb);
  IupSetCallback(multitext, "K_ESC", (Icallback)multitext_kesc_cb);
  IupSetCallback(multitext, "MAP_CB", (Icallback)multitext_map_cb);

  IupSetAttribute(multitext, "AUTOCOMPLETION", "OFF");

  IupSetAttribute(multitext, "MARGINWIDTH2", "0");
  IupSetAttribute(multitext, "MARGINTYPE2", "SYMBOL");
  IupSetAttribute(multitext, "MARGINSENSITIVE2", "YES");
  IupSetAttribute(multitext, "MARGINMASKFOLDERS2", "NO");  /* (disable folding) */
  IupSetAttributeId(multitext, "MARGINMASK", 2, "10");  /* 1110 - marker=1 and marker=2 ad marker=3 */

  IupSetAttributeId(multitext, "MARKERBGCOLOR", 1, "0 255 0");
  IupSetAttributeId(multitext, "MARKERALPHA", 1, "80");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 1, "BACKGROUND");

#ifdef WIN32
  IupSetAttribute(multitext, "FONT", "Consolas, 11");
#else
  IupSetAttribute(multitext, "FONT", "Monospace, 12");
  /* Other alternatives for "Consolas" in Linux: "DejaVu Sans Mono", "Liberation Mono", "Nimbus Mono L", "FreeMono" */
#endif

  return IUP_DEFAULT;
}

static int closetext_cb(Ihandle* ih, Ihandle *multitext)
{
  char *filename = IupGetAttribute(multitext, "FILENAME");
  Ihandle* tree = IupGetDialogChild(ih, "ELEMENTS_TREE");
  Ihandle* currMutltitext = get_current_multitext(ih);

  if (!filename)
    return IUP_DEFAULT;

  unload_led(filename);
  IupSetAttribute(tree, "DELNODE0", "CHILDREN");

  if (currMutltitext != multitext)
  {
    char *currentFile = IupGetAttribute(currMutltitext, "FILENAME");
    mainUpdateList(tree, currentFile);
  }

  return IUP_DEFAULT;
}

static int configload_cb(Ihandle *ih, Ihandle* config)
{
  const char* value;

  value = IupConfigGetVariableStr(config, "LedScripter", "AutoCompletion");
  if (value)
  {
    Ihandle* ih_item = IupGetDialogChild(ih, "ITM_AUTOCOMPLETE");
    IupSetStrAttribute(ih_item, "VALUE", value);
  }

  value = IupConfigGetVariableStr(config, "LedScripter", "Folding");
  if (value)
  {
    Ihandle* ih_item = IupGetDialogChild(ih, "ITM_FOLDING");
    IupSetStrAttribute(ih_item, "VALUE", value);
  }

  return IUP_DEFAULT;
}

static int marker_changed_cb(Ihandle *ih, Ihandle *multitext, int lin, int margin)
{
  if (margin == 3)
    IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);

  (void)ih;
  return IUP_DEFAULT;
}

static int led_menu_open_cb(Ihandle *ih_menu)
{
  Ihandle* menu_foldall = IupGetDialogChild(ih_menu, "ITM_FOLD_ALL");
  Ihandle* item_toggle_folding = IupGetDialogChild(ih_menu, "ITM_TOGGLE_FOLDING");
  Ihandle* item_folding = IupGetDialogChild(ih_menu, "ITM_FOLDING");
  Ihandle* item_comments = IupGetDialogChild(ih_menu, "ITM_COMMENTS");
  Ihandle* multitext = get_current_multitext(ih_menu);
  char *selpos = IupGetAttribute(multitext, "SELECTIONPOS");

  if (IupGetInt(item_folding, "VALUE"))
  {
    IupSetAttribute(item_toggle_folding, "ACTIVE", "Yes");
    IupSetAttribute(menu_foldall, "ACTIVE", "Yes");
  }
  else
  {
    IupSetAttribute(item_toggle_folding, "ACTIVE", "NO");
    IupSetAttribute(menu_foldall, "ACTIVE", "NO");
  }

  if (selpos)
    IupSetAttribute(item_comments, "ACTIVE", "Yes");
  else
    IupSetAttribute(item_comments, "ACTIVE", "NO");

  return IUP_DEFAULT;
}


static int item_autocomplete_action_cb(Ihandle* ih_item)
{
  Ihandle* tabs = IupGetDialogChild(ih_item, "MULTITEXT_TABS");
  int children_count = IupGetChildCount(tabs);
  Ihandle* multitext;
  Ihandle* config = get_config(ih_item);
  int i;

  if (IupGetInt(ih_item, "VALUE"))
  {
    IupSetAttribute(ih_item, "VALUE", "OFF");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);
      IupSetAttribute(multitext, "AUTOCOMPLETION", "OFF");
    }

  }
  else
  {
    IupSetAttribute(ih_item, "VALUE", "ON");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);
      IupSetAttribute(multitext, "AUTOCOMPLETION", "ON");
    }
  }

  IupConfigSetVariableStr(config, "LedScripter", "AutoCompletion", IupGetAttribute(ih_item, "VALUE"));

  return IUP_DEFAULT;
}


static int setparent_param_cb(Ihandle* param_dialog, int param_index, void* user_data)
{
  if (param_index == IUP_GETPARAM_MAP)
  {
    Ihandle* ih = (Ihandle*)user_data;
    IupSetAttributeHandle(param_dialog, "PARENTDIALOG", ih);
  }

  return 1;
}

static int item_style_config_action_cb(Ihandle* ih_item)
{
  Ihandle* tabs = IupGetDialogChild(ih_item, "MULTITEXT_TABS");
  int children_count = IupGetChildCount(tabs);
  Ihandle* multitext;
  Ihandle* config = get_config(ih_item);
  char commentColor[30], numberColor[30], keywordColor[30],
    stringColor[30], characterColor[30], operatorColor[30];
  int i;

  strcpy(commentColor, IupConfigGetVariableStr(config, "LedScripter", "CommentColor"));
  strcpy(numberColor, IupConfigGetVariableStr(config, "LedScripter", "NumberColor"));
  strcpy(keywordColor, IupConfigGetVariableStr(config, "LedScripter", "KeywordColor"));
  strcpy(stringColor, IupConfigGetVariableStr(config, "LedScripter", "StringColor"));
  strcpy(characterColor, IupConfigGetVariableStr(config, "LedScripter", "CharacterColor"));
  strcpy(operatorColor, IupConfigGetVariableStr(config, "LedScripter", "OperatorColor"));

  if (!IupGetParam("Syntax Colors", setparent_param_cb, IupGetDialog(ih_item),
    "Comment: %c\n"
    "Number: %c\n"
    "Keyword: %c\n"
    "String: %c\n"
    "Character: %c\n"
    "Operator: %c\n",
    commentColor, numberColor, keywordColor, stringColor, characterColor, operatorColor, NULL))
    return IUP_DEFAULT;

  IupConfigSetVariableStr(config, "LedScripter", "CommentColor", commentColor);
  IupConfigSetVariableStr(config, "LedScripter", "NumberColor", numberColor);
  IupConfigSetVariableStr(config, "LedScripter", "KeywordColor", keywordColor);
  IupConfigSetVariableStr(config, "LedScripter", "StringColor", stringColor);
  IupConfigSetVariableStr(config, "LedScripter", "CharacterColor", characterColor);
  IupConfigSetVariableStr(config, "LedScripter", "OperatorColor", operatorColor);

  for (i = 0; i < children_count; i++)
  {
    multitext = IupGetChild(tabs, i);

    IupSetStrAttribute(multitext, "STYLEFGCOLOR1", commentColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR2", numberColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR3", keywordColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR4", stringColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR5", characterColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR6", operatorColor);
  }

  return IUP_DEFAULT;
}

static int item_folding_action_cb(Ihandle* ih)
{
  Ihandle* tabs = IupGetDialogChild(ih, "MULTITEXT_TABS");
  int children_count = IupGetChildCount(tabs);
  Ihandle* multitext;
  Ihandle* config = get_config(ih);
  int i;

  if (IupGetInt(ih, "VALUE"))
  {
    IupSetAttribute(ih, "VALUE", "OFF");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);

      IupSetAttribute(multitext, "PROPERTY", "fold=0");
      IupSetAttribute(multitext, "MARGINWIDTH3", "0");
      IupSetAttribute(multitext, "_IUP_FOLDDING", NULL);
    }
  }
  else
  {
    IupSetAttribute(ih, "VALUE", "ON");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);

      IupSetAttribute(multitext, "PROPERTY", "fold=1");
      IupSetAttribute(multitext, "MARGINWIDTH3", FOLDING_MARGIN);
      IupSetAttribute(multitext, "_IUP_FOLDDING", "1");
      IupSetAttribute(multitext, "FOLDALL", "EXPAND");
    }
  }

  IupConfigSetVariableStr(config, "LedScripter", "Folding", IupGetAttribute(ih, "VALUE"));

  return IUP_DEFAULT;
}

static int item_toggle_folding_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);
  int pos = IupGetInt(multitext, "CARETPOS");
  int lin, col;

  /* must test again because it can be called by the hot key */
  if (!IupGetInt(multitext, "_IUP_FOLDDING"))
    return IUP_DEFAULT;

  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);

  if (!IupGetIntId(multitext, "FOLDLEVELHEADER", lin))
  {
    lin = IupGetIntId(multitext, "FOLDPARENT", lin);
    if (lin < 0)
      return IUP_DEFAULT;
  }

  IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
  IupSetfAttribute(multitext, "CARET", "%d:0", lin);

  return IUP_DEFAULT;
}

static int item_fold_collapse_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "CONTRACT");

  return IUP_DEFAULT;
}

static int item_fold_expand_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "EXPAND");

  return IUP_DEFAULT;
}

static int item_fold_toggle_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "TOGGLE");

  return IUP_DEFAULT;
}

static int item_fold_level_action_cb(Ihandle* ih_item)
{
  int level = 0, action = 0;
  Ihandle* config = get_config(ih_item);

  const char* value = IupConfigGetVariableStr(config, "LedScripter", "FoldAllLevel");
  if (value) iupStrToInt(value, &level);

  value = IupConfigGetVariableStr(config, "LedScripter", "FoldAllLevelAction");
  if (value) iupStrToInt(value, &action);

  if (IupGetParam("Fold All by Level", setparent_param_cb, IupGetDialog(ih_item),
    "Level: %i\n"
    "Options: %o|Collapse|Expand|Toggle|\n",
    &level, &action, NULL))
  {
    Ihandle* multitext = get_current_multitext(ih_item);
    int lin, count = IupGetInt(multitext, "LINECOUNT");

    IupConfigSetVariableInt(config, "LuaScripter", "FoldAllLevel", level);
    IupConfigSetVariableInt(config, "LuaScripter", "FoldAllLevelAction", action);

    for (lin = 0; lin < count; lin++)
    {
      if (IupGetIntId(multitext, "FOLDLEVELHEADER", lin))
      {
        int foldLevel = IupGetIntId(multitext, "FOLDLEVEL", lin);
        if (foldLevel + 1 == level) /* level at header is different from child */
        {
          switch (action)
          {
            case 0: /* Collapse */
              if (IupGetIntId(multitext, "FOLDEXPANDED", lin))
                IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
              break;
            case 1: /* Expand */
              if (!IupGetIntId(multitext, "FOLDEXPANDED", lin))
                IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
              break;
            case 2: /* Toggle */
              IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
              break;
          }
        }
      }
    }
  }

  return IUP_DEFAULT;
}

static int item_linescomment_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = get_current_multitext(ih_item);
  char *sel = IupGetAttribute(multitext, "SELECTION");
  char *text_line;
  int lin, col, lin1, lin2, col1, col2;

  if (!sel)
    return IUP_DEFAULT;

  sscanf(sel, "%d,%d:%d,%d", &lin1, &col1, &lin2, &col2);

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  for (lin = lin1; lin <= lin2; lin++)
  {
    int len, pos;
    text_line = IupGetAttributeId(multitext, "LINE", lin);
    len = (int)strlen(text_line);

    for (col = 0; col < len; col++)
    {
      char c = text_line[col];
      if (c != ' ' && c != '\t')
        break;
    }

    IupTextConvertLinColToPos(multitext, lin, col, &pos);
    IupSetAttributeId(multitext, "INSERT", pos, "# ");
  }

  IupSetAttribute(multitext, "UNDOACTION", "END");

  IupSetStrf(multitext, "SELECTION", "%d,0:%d,999", lin1, lin2);

  return IUP_DEFAULT;
}

static int item_linesuncomment_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = get_current_multitext(ih_item);
  char *sel = IupGetAttribute(multitext, "SELECTION");
  char *text_line;
  int lin, col, lin1, lin2, col1, col2;

  if (!sel)
    return IUP_DEFAULT;

  sscanf(sel, "%d,%d:%d,%d", &lin1, &col1, &lin2, &col2);

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  for (lin = lin1; lin <= lin2; lin++)
  {
    int len, pos, nChar;
    text_line = IupGetAttributeId(multitext, "LINE", lin);
    len = (int)strlen(text_line);
    nChar = 0;

    for (col = 0; col < len; col++)
    {
      if (text_line[col] == '#')
      {
        nChar = 1;
        if (text_line[col + 2] == ' ')
          nChar++;
        break;
      }
    }

    if (nChar == 0)
      continue;

    IupTextConvertLinColToPos(multitext, lin, col, &pos);
    IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, nChar);
  }

  IupSetAttribute(multitext, "UNDOACTION", "END");

  IupSetStrf(multitext, "SELECTION", "%d,0:%d,999", lin1, lin2);

  return IUP_DEFAULT;
}

static int tree_elements_selection_cb(Ihandle* self, char *t, int i, int v)
{
  return IUP_DEFAULT;
}

static int list_elements_dblclick_cb(Ihandle *ih, int index, char *t)
{
  return IUP_DEFAULT;
}

static Ihandle* buildLedMenu(void)
{
  Ihandle *item_autocomplete, *item_style_config, *item_expand, *item_toggle, *item_level,
    *item_folding, *item_toggle_folding, *ledMenu, *item_collapse,
    *item_linescomment, *item_linesuncomment;

  item_autocomplete = IupItem("Auto Completion", NULL);
  IupSetAttribute(item_autocomplete, "NAME", "ITM_AUTOCOMPLETE");
  IupSetCallback(item_autocomplete, "ACTION", (Icallback)item_autocomplete_action_cb);

  item_style_config = IupItem("Syntax Colors...", NULL);
  IupSetAttribute(item_style_config, "NAME", "ITM_STYLE");
  IupSetCallback(item_style_config, "ACTION", (Icallback)item_style_config_action_cb);

  item_folding = IupItem("Folding", NULL);
  IupSetAttribute(item_folding, "NAME", "ITM_FOLDING");
  IupSetCallback(item_folding, "ACTION", (Icallback)item_folding_action_cb);

  item_toggle_folding = IupItem("Toggle Fold\tF8", NULL);
  IupSetAttribute(item_toggle_folding, "NAME", "ITM_TOGGLE_FOLDING");
  IupSetCallback(item_toggle_folding, "ACTION", (Icallback)item_toggle_folding_action_cb);

  item_collapse = IupItem("Collapse", NULL);
  IupSetAttribute(item_collapse, "NAME", "ITM_COLLAPSE");
  IupSetCallback(item_collapse, "ACTION", (Icallback)item_fold_collapse_action_cb);

  item_expand = IupItem("Expand", NULL);
  IupSetAttribute(item_expand, "NAME", "ITM_EXPAND");
  IupSetCallback(item_expand, "ACTION", (Icallback)item_fold_expand_action_cb);

  item_toggle = IupItem("Toggle", NULL);
  IupSetAttribute(item_toggle, "NAME", "ITM_TOGGLE");
  IupSetCallback(item_toggle, "ACTION", (Icallback)item_fold_toggle_action_cb);

  item_level = IupItem("by Level...", NULL);
  IupSetAttribute(item_level, "NAME", "ITM_LEVEL");
  IupSetCallback(item_level, "ACTION", (Icallback)item_fold_level_action_cb);

  item_linescomment = IupItem("Lines Comment", NULL);
  IupSetCallback(item_linescomment, "ACTION", (Icallback)item_linescomment_action_cb);

  item_linesuncomment = IupItem("Lines Uncomment", NULL);
  IupSetCallback(item_linesuncomment, "ACTION", (Icallback)item_linesuncomment_action_cb);

  ledMenu = IupMenu(
    item_folding,
    item_toggle_folding,
    IupSubmenu("Fold All",
    IupSetAttributes(IupMenu(
    item_collapse,
    item_expand,
    item_toggle,
    item_level,
    NULL), "NAME=ITM_FOLD_ALL")),
    IupSeparator(),
    IupSubmenu("Comments",
    IupSetAttributes(IupMenu(
    item_linescomment,
    item_linesuncomment,
    NULL), "NAME=ITM_COMMENTS")),
    IupSeparator(),
    item_autocomplete,
    IupSeparator(),
    item_style_config,
    NULL);

  IupSetCallback(ledMenu, "OPEN_CB", (Icallback)led_menu_open_cb);

  return IupSubmenu("&Led", ledMenu);
}

int main(int argc, char **argv)
{
  Ihandle *main_dialog;
  Ihandle *config;
  Ihandle *menu;
  Ihandle *ledMenu;
  Ihandle *panelTabs;
  Ihandle *multitextTabs;
  Ihandle *elementsList;
  Ihandle *elementsFrame;
  Icallback oldTabChange_cb;
  int i;

  IupOpen(&argc, &argv);
  IupImageLibOpen();
  IupScintillaOpen();

#ifdef _DEBUG
  IupSetGlobal("GLOBALLAYOUTDLGKEY", "Yes");
#endif

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "iupvled");
  IupConfigLoad(config);

  main_dialog = IupScintillaDlg();

  ledMenu = buildLedMenu();
  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupInsert(menu, IupGetChild(menu, IupGetChildCount(menu) - 1), ledMenu);

  panelTabs = IupGetDialogChild(main_dialog, "PANEL_TABS");

  multitextTabs = IupGetDialogChild(main_dialog, "MULTITEXT_TABS");

  elementsList = IupTree();
  IupSetAttribute(elementsList, "EXPAND", "YES");
  IupSetAttribute(elementsList, "NAME", "ELEMENTS_TREE");
  IupSetCallback(elementsList, "SELECTION_CB", (Icallback)tree_elements_selection_cb);
  IupSetCallback(elementsList, "DBLCLICK_CB", (Icallback)list_elements_dblclick_cb);
  IupSetAttribute(elementsList, "VISIBLELINES", "3");
  IupSetAttributeHandle(elementsList, "IMG_SHORTCUT", load_image_shortcut());

  elementsFrame = IupFrame(elementsList);
  IupSetAttribute(elementsFrame, "NAME", "ELEMENTS_FRAME");
  IupSetAttribute(elementsFrame, "MARGIN", "4x4");
  IupSetAttribute(elementsFrame, "GAP", "4");
  IupSetAttribute(elementsFrame, "TITLE", "Elements:");
  IupSetAttribute(elementsFrame, "TABTITLE", "Elements");

  IupAppend(panelTabs, elementsFrame);

  IupSetAttribute(main_dialog, "SUBTITLE", "IUP Visual LED");
  IupSetAttributeHandle(main_dialog, "CONFIG", config);

  IupSetAttribute(main_dialog, "EXTRAFILTERS", "Led Files|*.led|");

  oldTabChange_cb = IupGetCallback(multitextTabs, "TABCHANGE_CB");
  IupSetCallback(multitextTabs, "OLDTABCHANGE_CB", oldTabChange_cb);
  IupSetCallback(multitextTabs, "TABCHANGE_CB", (Icallback)tabChange_cb);

  IupSetCallback(main_dialog, "LOADFILE_CB", (Icallback)loadfile_cb);
  IupSetCallback(main_dialog, "SAVEFILE_CB", (Icallback)savefile_cb);
  IupSetCallback(main_dialog, "NEWTEXT_CB", (Icallback)newtext_cb);
  IupSetCallback(main_dialog, "CLOSETEXT_CB", (Icallback)closetext_cb);
  IupSetCallback(main_dialog, "CONFIGLOAD_CB", (Icallback)configload_cb);
  IupSetCallback(main_dialog, "MARKERCHANGED_CB", (Icallback)marker_changed_cb);

  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupAppend(menu, IupSubmenu("&Help", IupMenu(
    IupSetCallbacks(IupItem("&Help...", NULL), "ACTION", (Icallback)item_help_action_cb, NULL),
    IupSetCallbacks(IupItem("&About...", NULL), "ACTION", (Icallback)item_about_action_cb, NULL),
    NULL)));

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, main_dialog, IupGetAttribute(main_dialog, "SUBTITLE"));

  /* open a file from the command line (allow file association in Windows) */
  for (i = 1; i < argc; i++)
  {
    const char* filename = argv[i];
    IupSetStrAttribute(main_dialog, "OPENFILE", filename);
  }

  /* Call NEW_TEXT_CB because the first tab was already created */
  newtext_cb(main_dialog, get_current_multitext(main_dialog));

  IupMainLoop();

  IupDestroy(config);
  IupDestroy(main_dialog);

  IupClose();
  return EXIT_SUCCESS;
}

