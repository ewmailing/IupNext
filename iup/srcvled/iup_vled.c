#if 0
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iup.h>
#include <iupim.h>
#include <iupcbs.h>
#include <iup_scintilla.h>
#include <iup_config.h>
#include <iupcontrols.h>

#define USE_NO_OPENGL  
#define USE_NO_WEB
#define USE_NO_PLOT

#ifndef USE_NO_OPENGL  
#include <iupgl.h>
#include <iupglcontrols.h>
#endif  
#ifndef USE_NO_WEB
#include <iupweb.h>
#endif  
#ifndef USE_NO_PLOT
#include <iup_plot.h>
#endif  


#include "iup_str.h"
#include "iup_object.h"
#include "iup_ledlex.h"
#include "iup_attrib.h"
#include "iup_image.h"
#include "iup_dlglist.h"


void vLedExport(const char* src_filename, const char* dst_filename, const char* format);

#define VLED_FOLDING_MARGIN "20"

#define vled_isprint(_c) ((_c) > 31 && (_c) < 127 && _c != 40 && _c != '(' && _c != ')' && _c != '[' && _c != ']' && _c != ',')

enum {
  FIND_TYPE = 0,
  FIND_HANDLE = 1,
  FIND_NAME = 2,
  FIND_TITLE = 3,
  FIND_ATTRIBUTE = 4
};


static int compare_image_names(const void* i1, const void* i2)
{
  Ihandle* ih1 = *((Ihandle**)i1);
  Ihandle* ih2 = *((Ihandle**)i2);
  char* name1 = IupGetName(ih1);
  char* name2 = IupGetName(ih2);
  return strcmp(name1, name2);
}

static char* getfileformat()
{
#define NUM_FORMATS 4
  int ret, count = NUM_FORMATS;
  static char *options[NUM_FORMATS] = {
    "ICO",
    "BMP",
    "GIF",
    "PNG"
  };

  ret = IupListDialog(1, "File Format", count, options, 1, 9, count + 1, NULL);
  if (ret == -1)
    return NULL;
  else
    return options[ret];
}

static Ihandle* get_config(Ihandle* ih)
{
  Ihandle* config = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "CONFIG_HANDLE");
  return config;
}

static char* getfolder(Ihandle* ih)
{
  static char folder[10240];
  Ihandle *filedlg = IupFileDlg();
  Ihandle* config = get_config(ih);
  const char* dir = IupConfigGetVariableStr(config, "IupVisualLED", "LastExportDirectory");
  if (!dir) dir = IupConfigGetVariableStr(config, "IupVisualLED", "LastDirectory");

  IupSetAttribute(filedlg, "DIALOGTYPE", "DIR");
  IupSetStrAttribute(filedlg, "DIRECTORY", dir);
  IupSetAttributeHandle(filedlg, "PARENTDIALOG", IupGetDialog(ih));
  IupSetAttribute(filedlg, "TITLE", "Select Folder for Files");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    strcpy(folder, IupGetAttribute(filedlg, "VALUE"));
    IupDestroy(filedlg);
    return folder;
  }

  IupDestroy(filedlg);
  return NULL;
}

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

int vLedIsAlien(Ihandle *elem, const char* filename);
static int vLedTreeAddNode(Ihandle* elem_tree, int id, Ihandle* ih, const char *filename);
static int vLedTreeAddChildren(Ihandle* elem_tree, int parent_id, Ihandle* parent, const char *filename);

static void mainUpdateElemTree(Ihandle* elem_tree, const char* filename)
{
  int last_child_id = -1;
  int i, num_names = IupGetAllNames(NULL, -1);
  char* *names = malloc(sizeof(char*)*num_names);
  IupGetAllNames(names, num_names);

  IupSetAttribute(elem_tree, "DELNODE0", "ALL");

  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
    if (iupObjectCheck(elem))
    {
      Ihandle* elem_parent = elem->parent;

      if (vLedIsAlien(elem, filename) || (elem_parent && !vLedIsAlien(elem_parent, filename)))
        continue;

      last_child_id = vLedTreeAddNode(elem_tree, last_child_id, elem, filename);

      if (elem->iclass->childtype != IUP_CHILDNONE && !vLedIsAlien(elem, filename))
        last_child_id = vLedTreeAddChildren(elem_tree, last_child_id, elem, filename);

      last_child_id = -1;
    }
  }

  IupSetAttribute(elem_tree, "VALUE", "1");
  IupSetAttribute(elem_tree, "VALUE", "0");

  free(names);
}

static void replaceDot(char* filename)
{
  /* replace all "." by "_" */
  /* replace all "-" by "_" */
  while (*filename)
  {
    if (*filename == '.')
      *filename = '_';
    if (*filename == '-')
      *filename = '_';

    filename++;
  }
}

static char* strdup_free(const char* str, char* str_ptr)
{
  int len = (int)strlen(str);
  char* tmp = malloc(len + 1);
  memcpy(tmp, str, len + 1);
  free(str_ptr);
  return tmp;
}

static char* mainGetFileTitle(const char* filename)
{
  int i, last = 1, len = (int)strlen(filename);
  char* file_title = malloc(len + 1);
  char* dot, *ft_str = file_title;

  memcpy(file_title, filename, len + 1);

  dot = strchr(file_title, '.');
  if (dot) *dot = 0;

  for (i = len - 1; i >= 0; i--)
  {
    if (last && file_title[i] == '.')
    {
      /* cut last "." forward */
      file_title[i] = 0;
      last = 0;
    }

    if (file_title[i] == '\\' || file_title[i] == '/')
    {
      replaceDot(file_title + i + 1);
      return strdup_free(file_title + i + 1, ft_str);
    }
  }

  replaceDot(file_title);
  return strdup_free(file_title, ft_str);
}

static Ihandle* vLedGetCurrentMultitext(Ihandle* ih)
{
  Ihandle* tabs = IupGetDialogChild(ih, "MULTITEXT_TABS");
  return (Ihandle*)IupGetAttribute(tabs, "VALUE_HANDLE");
}

static void LoadImageFile(Ihandle* self, const char* filename)
{
  Ihandle* currMultitext = vLedGetCurrentMultitext(self);

  Ihandle* new_image = IupLoadImage(filename);

  if (new_image)
  {
    int lin, col, pos, i, n = 0;
    char *buffer;
    char* file_title = mainGetFileTitle(filename);
    char name[80];

    for (i = 0; file_title[i] != 0; i++)
    {
      if (!vled_isprint(file_title[i]))
        continue;
      if (file_title[i] == ' ')
        name[n] = '_';
      else
        name[n] = file_title[i];
      n++;
    }
    name[n] = 0;

    IupSetHandle(file_title, new_image);

    iupImageExportToString(new_image, &buffer, "LED", name, 1);

    pos = IupGetInt(currMultitext, "CARETPOS");
    IupTextConvertPosToLinCol(currMultitext, pos, &lin, &col);

    IupTextConvertLinColToPos(currMultitext, lin - 1, 0, &pos);

    IupSetAttribute(currMultitext, "UNDOACTION", "BEGIN");

    IupSetAttributeId(currMultitext, "INSERT", pos, buffer);

    IupSetAttribute(currMultitext, "UNDOACTION", "END");

    free(file_title);
    free(buffer);
    IupDestroy(new_image);
  }
  else
  {
    char* err_msg = IupGetGlobal("IUPIM_LASTERROR");
    if (err_msg)
      IupMessageError(IupGetDialog(self), err_msg);
  }
}

int vLedIsAlien(Ihandle *elem, const char* filename)
{
  char *elem_file;

  if (!elem || !filename)
    return 0;

  elem_file = iupAttribGet(elem, "_IUPLED_LEDFILENAME");

  if (!elem_file || !iupStrEqual(elem_file, filename))
    return 1;

  return 0;
}

static char* vLedGetElementTreeTitle(Ihandle* ih)
{
  char* title = iupAttribGetLocal(ih, "TITLE");
  char* str = iupStrGetMemory(200);
  char* name = IupGetName(ih);
  if (IupClassMatch(ih, "user") && iupAttribGet(ih, "LEDPARSER_NAME"))
    name = iupAttribGet(ih, "LEDPARSER_NAME");

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

static void vLedTreeSetNodeInfo(Ihandle* elem_tree, int id, Ihandle* ih, int link)
{
  IupSetAttributeId(elem_tree, "TITLE", id, vLedGetElementTreeTitle(ih));
  //iLayoutTreeSetNodeColor(elem_tree, id, ih);
  IupTreeSetUserId(elem_tree, id, ih);
  if (link)
  {
    if (ih->iclass->childtype != IUP_CHILDNONE)
      IupSetAttributeId(elem_tree, "IMAGEEXPANDED", id, IupGetAttribute(elem_tree, "IMG_SHORTCUT"));
    else
      IupSetAttributeId(elem_tree, "IMAGE", id, IupGetAttribute(elem_tree, "IMG_SHORTCUT"));
  }
}

static Ihandle* vLedTreeGetFirstChild(Ihandle* ih)
{
  Ihandle* firstchild = ih->parent->firstchild;

  while (firstchild && firstchild->flags & IUP_INTERNAL)
    firstchild = firstchild->brother;

  return firstchild;
}

static int vLedTreeAddNode(Ihandle* elem_tree, int id, Ihandle* ih, const char *filename)
{
  int link = 0;

  if (ih->iclass->childtype != IUP_CHILDNONE && !IupGetInt(ih, "LEDPARSER_NOTDEFINED"))
  {
    if (!ih->parent || vLedIsAlien(ih->parent, filename) || ih == vLedTreeGetFirstChild(ih))
    {
      IupSetAttributeId(elem_tree, "ADDBRANCH", id, "");
      id++;
    }
    else
    {
      IupSetAttributeId(elem_tree, "INSERTBRANCH", id, "");
      id = IupGetInt(elem_tree, "LASTADDNODE");
    }
  }
  else
  {
    if (!ih->parent || vLedIsAlien(ih->parent, filename) || ih == vLedTreeGetFirstChild(ih))
    {
      IupSetAttributeId(elem_tree, "ADDLEAF", id, "");
      id++;
    }
    else
    {
      IupSetAttributeId(elem_tree, "INSERTLEAF", id, "");
      id = IupGetInt(elem_tree, "LASTADDNODE");
    }
  }

  if (vLedIsAlien(ih, filename) || iupStrEqual(ih->iclass->name, "user"))
    link = 1;

  vLedTreeSetNodeInfo(elem_tree, id, ih, link);
  return id;
}

static int vLedTreeAddChildren(Ihandle* elem_tree, int parent_id, Ihandle* parent, const char *filename)
{
  Ihandle *child;
  int last_child_id = parent_id;

  for (child = parent->firstchild; child; child = child->brother)
  {
    if (!(child->flags & IUP_INTERNAL))
    {
      last_child_id = vLedTreeAddNode(elem_tree, last_child_id, child, filename);

      if (child->iclass->childtype != IUP_CHILDNONE && !vLedIsAlien(child, filename))
        vLedTreeAddChildren(elem_tree, last_child_id, child, filename);
    }
  }

  return last_child_id;
}

static int unload_led(const char *filename);

static void autoload_off(Ihandle *elem_tree)
{
  IupSetAttribute(elem_tree, "DELNODE0", "ALL");
}

static int load_led(Ihandle *elem_tree, const char *filename, int use_buffer)
{
  Ihandle* multitext = vLedGetCurrentMultitext(elem_tree);
  const char* error;

  IupSetInt(multitext, "MARKERDELETEALL", 1);

  if (use_buffer)
  {
    char* buffer = IupGetAttribute(multitext, "VALUE");
    error = iupLoadLed(filename, buffer, 1);  /* save info */
  }
  else
    error = iupLoadLed(filename, NULL, 1);  /* save info */

  if (error)
  {
    int line;
    const char* error_mark = "bad input at line";
    const char* error_line = strstr(error, error_mark);
    error_line += strlen(error_mark) + 1;
    sscanf(error_line, "%d", &line);
    IupSetIntId(multitext, "MARKERADD", line-1, 1);
    IupSetStrf(multitext, "CARET", "%d,0", line-1);
    IupMessageError(IupGetDialog(elem_tree), error);
  }

  if (error)
    unload_led(filename);
  else
    mainUpdateElemTree(elem_tree, filename);

  return IUP_DEFAULT;
}

static void unloadNamedElements(Ihandle *elem)
{
  Ihandle *child;

  for (child = elem->firstchild; child; child = child->brother)
  {
    if (child->iclass->childtype != IUP_CHILDNONE)
      unloadNamedElements(child);

    if (IupGetName(child))
      IupDetach(child);
  }
}

static int unload_led(const char *filename)
{
  int i, num_names = IupGetAllNames(NULL, -1);
  char* *names = malloc(sizeof(char*)*num_names);
  Ihandle* *named_elems = malloc(sizeof(Ihandle*)*num_names);
  IupGetAllNames(names, num_names);

  for (i = 0; i < num_names; i++)
    named_elems[i] = IupGetHandle(names[i]);
  
  for (i = 0; i < num_names; i++)
  {
    Ihandle *elem = named_elems[i],
           *parent, *brother;
    char old_name[80];

    if (!iupObjectCheck(elem))  /* it may already being destroyed in the hierarchy */
      continue;

    if (vLedIsAlien(elem, filename))
      continue;

    if (IupClassMatch(elem, "menu"))
    {
      Ihandle *parent = elem->parent;
      if (parent && IupClassMatch(parent, "dialog"))
        IupSetAttribute(parent, "MENU", NULL);
    }

    if (IupClassMatch(elem, "dialog"))
    {
      Ihandle *menu = (Ihandle *) IupGetAttribute(elem, "MENU");
      if (menu)
        IupSetAttribute(elem, "MENU", NULL);
    }

    if (elem->iclass->childtype != IUP_CHILDNONE)
      unloadNamedElements(elem);

    parent = elem->parent;
    strcpy(old_name, IupGetName(elem));

    if (parent && vLedIsAlien(parent, filename))
      brother = elem->brother;

    IupDestroy(elem);

    if (parent && vLedIsAlien(parent, filename))
    {
      Ihandle *user = IupUser();
      IupSetAttribute(user, "LEDPARSER_NOTDEFINED", "1");
      IupStoreAttribute(user, "LEDPARSER_NAME", old_name);
      IupInsert(parent, brother, user);
    }
  }

  free(names);
  free(named_elems);
  return IUP_DEFAULT;
}

static int item_help_action_cb(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

static int item_about_action_cb(void)
{
  IupVersionShow();
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
  IupSetAttribute(multitext, "MARGINWIDTH3", VLED_FOLDING_MARGIN);
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

  value = IupConfigGetVariableStr(config, "IupVisualLED", "CommentColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR1", value);
  else
    IupConfigSetVariableStr(config, "IupVisualLED", "CommentColor", IupGetAttribute(multitext, "STYLEFGCOLOR1"));

  value = IupConfigGetVariableStr(config, "IupVisualLED", "NumberColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR2", value);
  else
    IupConfigSetVariableStr(config, "IupVisualLED", "NumberColor", IupGetAttribute(multitext, "STYLEFGCOLOR4"));

  value = IupConfigGetVariableStr(config, "IupVisualLED", "KeywordColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR3", value);
  else
    IupConfigSetVariableStr(config, "IupVisualLED", "KeywordColor", IupGetAttribute(multitext, "STYLEFGCOLOR5"));

  value = IupConfigGetVariableStr(config, "IupVisualLED", "StringColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR4", value);
  else
    IupConfigSetVariableStr(config, "IupVisualLED", "StringColor", IupGetAttribute(multitext, "STYLEFGCOLOR6"));

  value = IupConfigGetVariableStr(config, "IupVisualLED", "CharacterColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR5", value);
  else
    IupConfigSetVariableStr(config, "IupVisualLED", "CharacterColor", IupGetAttribute(multitext, "STYLEFGCOLOR7"));

  value = IupConfigGetVariableStr(config, "IupVisualLED", "OperatorColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR6", value);
  else
    IupConfigSetVariableStr(config, "IupVisualLED", "OperatorColor", IupGetAttribute(multitext, "STYLEFGCOLOR10"));

  value = IupConfigGetVariableStr(config, "IupVisualLED", "AutoCompletion");
  if (value)
    IupSetStrAttribute(multitext, "AUTOCOMPLETION", value);

  value = IupConfigGetVariableStr(config, "IupVisualLED", "Folding");
  if (iupStrBoolean(value))
  {
    IupSetAttribute(multitext, "MARGINWIDTH3", VLED_FOLDING_MARGIN);
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
  Ihandle* elem_tree = IupGetDialogChild(tabs, "ELEMENTS_TREE");
  if (!filename) filename = IupGetAttribute(new_tab, "NEW_FILENAME");

  IFnnn oldTabChange_cb = (IFnnn)IupGetCallback(tabs, "OLDTABCHANGE_CB");
  if (oldTabChange_cb)
    oldTabChange_cb(tabs, new_tab, old_tab);

  mainUpdateElemTree(elem_tree, filename);

  return IUP_DEFAULT;
}

static int loadfile_cb(Ihandle* self, char* filename)
{
  /* called after the file is loaded */

  Ihandle* elem_tree = IupGetDialogChild(self, "ELEMENTS_TREE");
  Ihandle* config = get_config(self);

  if (IupConfigGetVariableIntDef(config, "IupVisualLED", "AutoLoad", 1))
    load_led(elem_tree, filename, 0);
  else
    autoload_off(elem_tree);

  return IUP_DEFAULT;
}

static int savefile_cb(Ihandle* self, char* filename)
{
  /* called after the file is saved */

  Ihandle* elem_tree = IupGetDialogChild(self, "ELEMENTS_TREE");
  Ihandle* config = get_config(self);

  /* reload the elements because they may have changed */
  unload_led(filename);

  if (IupConfigGetVariableIntDef(config, "IupVisualLED", "AutoLoad", 1))
    load_led(elem_tree, filename, 0);
  else
    autoload_off(elem_tree);

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
  Ihandle* elem_tree = IupGetDialogChild(ih, "ELEMENTS_TREE");
  Ihandle* currMutltitext = vLedGetCurrentMultitext(ih);
  if (!filename) filename = IupGetAttribute(multitext, "NEW_FILENAME");

  unload_led(filename);
  IupSetAttribute(elem_tree, "DELNODE0", "ALL");

  if (currMutltitext != multitext)
  {
    filename = IupGetAttribute(currMutltitext, "FILENAME");
    if (!filename) filename = IupGetAttribute(currMutltitext, "NEW_FILENAME");
    mainUpdateElemTree(elem_tree, filename);
  }

  return IUP_DEFAULT;
}

static int configload_cb(Ihandle *ih, Ihandle* config)
{
  const char* value;

  value = IupConfigGetVariableStr(config, "IupVisualLED", "AutoCompletion");
  if (value)
  {
    Ihandle* ih_item = IupGetDialogChild(ih, "ITM_AUTOCOMPLETE");
    IupSetStrAttribute(ih_item, "VALUE", value);
  }

  value = IupConfigGetVariableStr(config, "IupVisualLED", "Folding");
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
  Ihandle* multitext = vLedGetCurrentMultitext(ih_menu);
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


static int tools_menu_open_cb(Ihandle *ih_menu)
{
  Ihandle* item_import_img = IupGetDialogChild(ih_menu, "ITM_IMP_IMG");
  Ihandle* item_export_img = IupGetDialogChild(ih_menu, "ITM_EXP_IMG");
  Ihandle* item_show_all_img = IupGetDialogChild(ih_menu, "ITM_SHOW_ALL_IMG");
  Ihandle* item_export_lua = IupGetDialogChild(ih_menu, "ITM_EXP_LUA");
  Ihandle* item_export_open_lua = IupGetDialogChild(ih_menu, "ITM_EXPORT_OPEN_LUA");
  Ihandle* item_export_proj_lua = IupGetDialogChild(ih_menu, "ITM_EXP_PROJ_LUA");
  Ihandle* item_export_c = IupGetDialogChild(ih_menu, "ITM_EXP_C");
  Ihandle* item_export_open_c = IupGetDialogChild(ih_menu, "ITM_EXPORT_OPEN_C");
  Ihandle* item_export_proj_c = IupGetDialogChild(ih_menu, "ITM_EXP_PROJ_C");
  Ihandle* multitext = vLedGetCurrentMultitext(ih_menu);
  char* filename = IupGetAttribute(multitext, "FILENAME");
  Ihandle *projConfig =  (Ihandle*)iupAttribGetInherit(ih_menu, "_IUP_PROJECT_CONFIG");

  if (filename)
  {
    IupSetAttribute(item_import_img, "ACTIVE", "YES");
    IupSetAttribute(item_export_img, "ACTIVE", "YES");
    IupSetAttribute(item_show_all_img, "ACTIVE", "YES");
    IupSetAttribute(item_export_lua, "ACTIVE", "YES");
    IupSetAttribute(item_export_open_lua, "ACTIVE", "YES");
    IupSetAttribute(item_export_c, "ACTIVE", "YES");
    IupSetAttribute(item_export_open_c, "ACTIVE", "YES");
  }
  else
  {
    IupSetAttribute(item_import_img, "ACTIVE", "NO");
    IupSetAttribute(item_export_img, "ACTIVE", "NO");
    IupSetAttribute(item_show_all_img, "ACTIVE", "NO");
    IupSetAttribute(item_export_lua, "ACTIVE", "NO");
    IupSetAttribute(item_export_open_lua, "ACTIVE", "NO");
    IupSetAttribute(item_export_c, "ACTIVE", "NO");
    IupSetAttribute(item_export_open_c, "ACTIVE", "NO");
  }

  if (projConfig)
  {
    IupSetAttribute(item_export_proj_c, "ACTIVE", "YES");
    IupSetAttribute(item_export_proj_lua, "ACTIVE", "YES");
  }
  else
  {
    IupSetAttribute(item_export_proj_c, "ACTIVE", "NO");
    IupSetAttribute(item_export_proj_lua, "ACTIVE", "NO");
  }

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

  IupConfigSetVariableStr(config, "IupVisualLED", "AutoCompletion", IupGetAttribute(ih_item, "VALUE"));

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

  strcpy(commentColor, IupConfigGetVariableStr(config, "IupVisualLED", "CommentColor"));
  strcpy(numberColor, IupConfigGetVariableStr(config, "IupVisualLED", "NumberColor"));
  strcpy(keywordColor, IupConfigGetVariableStr(config, "IupVisualLED", "KeywordColor"));
  strcpy(stringColor, IupConfigGetVariableStr(config, "IupVisualLED", "StringColor"));
  strcpy(characterColor, IupConfigGetVariableStr(config, "IupVisualLED", "CharacterColor"));
  strcpy(operatorColor, IupConfigGetVariableStr(config, "IupVisualLED", "OperatorColor"));

  if (!IupGetParam("Syntax Colors", setparent_param_cb, IupGetDialog(ih_item),
    "Comment: %c\n"
    "Number: %c\n"
    "Keyword: %c\n"
    "String: %c\n"
    "Character: %c\n"
    "Operator: %c\n",
    commentColor, numberColor, keywordColor, stringColor, characterColor, operatorColor, NULL))
    return IUP_DEFAULT;

  IupConfigSetVariableStr(config, "IupVisualLED", "CommentColor", commentColor);
  IupConfigSetVariableStr(config, "IupVisualLED", "NumberColor", numberColor);
  IupConfigSetVariableStr(config, "IupVisualLED", "KeywordColor", keywordColor);
  IupConfigSetVariableStr(config, "IupVisualLED", "StringColor", stringColor);
  IupConfigSetVariableStr(config, "IupVisualLED", "CharacterColor", characterColor);
  IupConfigSetVariableStr(config, "IupVisualLED", "OperatorColor", operatorColor);

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
      IupSetAttribute(multitext, "MARGINWIDTH3", VLED_FOLDING_MARGIN);
      IupSetAttribute(multitext, "_IUP_FOLDDING", "1");
      IupSetAttribute(multitext, "FOLDALL", "EXPAND");
    }
  }

  IupConfigSetVariableStr(config, "IupVisualLED", "Folding", IupGetAttribute(ih, "VALUE"));

  return IUP_DEFAULT;
}

static int item_toggle_folding_action_cb(Ihandle* ih)
{
  Ihandle* multitext = vLedGetCurrentMultitext(ih);
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
  Ihandle* multitext = vLedGetCurrentMultitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "CONTRACT");

  return IUP_DEFAULT;
}

static int item_fold_expand_action_cb(Ihandle* ih)
{
  Ihandle* multitext = vLedGetCurrentMultitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "EXPAND");

  return IUP_DEFAULT;
}

static int item_fold_toggle_action_cb(Ihandle* ih)
{
  Ihandle* multitext = vLedGetCurrentMultitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "TOGGLE");

  return IUP_DEFAULT;
}

static int item_fold_level_action_cb(Ihandle* ih_item)
{
  int level = 0, action = 0;
  Ihandle* config = get_config(ih_item);

  const char* value = IupConfigGetVariableStr(config, "IupVisualLED", "FoldAllLevel");
  if (value) iupStrToInt(value, &level);

  value = IupConfigGetVariableStr(config, "IupVisualLED", "FoldAllLevelAction");
  if (value) iupStrToInt(value, &action);

  if (IupGetParam("Fold All by Level", setparent_param_cb, IupGetDialog(ih_item),
    "Level: %i\n"
    "Options: %o|Collapse|Expand|Toggle|\n",
    &level, &action, NULL))
  {
    Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
    int lin, count = IupGetInt(multitext, "LINECOUNT");

    IupConfigSetVariableInt(config, "IupVisualLED", "FoldAllLevel", level);
    IupConfigSetVariableInt(config, "IupVisualLED", "FoldAllLevelAction", action);

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
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
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
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
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

static int item_load_action_cb(Ihandle *ih_item)
{
  Ihandle* elem_tree = IupGetDialogChild(ih_item, "ELEMENTS_TREE");
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
  char *filename = IupGetAttribute(multitext, "FILENAME");
  int dirty = IupGetInt(multitext, "MODIFIED");
  if (!filename) filename = IupGetAttribute(multitext, "NEW_FILENAME");

  unload_led(filename);

  if (dirty)
    load_led(elem_tree, filename, 1);
  else
    load_led(elem_tree, filename, 0);

  return IUP_DEFAULT;
}

static int item_unload_action_cb(Ihandle *ih_item)
{
  Ihandle* elem_tree = IupGetDialogChild(ih_item, "ELEMENTS_TREE");
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
  char *filename = IupGetAttribute(multitext, "FILENAME");
  if (!filename) filename = IupGetAttribute(multitext, "NEW_FILENAME");

  unload_led(filename);
  IupSetAttribute(elem_tree, "DELNODE0", "ALL");

  return IUP_DEFAULT;
}

static int item_autoload_action_cb(Ihandle *ih_item)
{
  Ihandle* config = get_config(ih_item);

  if (IupGetInt(ih_item, "VALUE"))
    IupConfigSetVariableInt(config, "IupVisualLED", "AutoLoad", 1);
  else
    IupConfigSetVariableInt(config, "IupVisualLED", "AutoLoad", 0);

  return IUP_DEFAULT;
}

static int item_import_img_action_cb(Ihandle *ih_item)
{
  Ihandle* config = get_config(ih_item);
  Ihandle* filedlg = IupFileDlg();
  const char* dir = IupConfigGetVariableStr(config, "IupVisualLED", "LastImageDirectory");
  if (!dir) dir = IupConfigGetVariableStr(config, "IupVisualLED", "LastDirectory");

  IupSetStrAttribute(filedlg, "DIRECTORY", dir);
  IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute(filedlg, "TITLE", "Load Image File(s)");
  IupSetAttribute(filedlg, "MULTIPLEFILES", "YES");
  IupSetAttributeHandle(filedlg, "PARENTDIALOG", IupGetDialog(ih_item));
  IupSetAttribute(filedlg, "ICON", IupGetGlobal("ICON"));
  IupSetAttribute(filedlg, "EXTFILTER", "Image Files|*.bmp;*.jpg;*.png;*.tif;*.tga|All Files|*.*|");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    int i, count = IupGetInt(filedlg, "MULTIVALUECOUNT");
    dir = IupGetAttributeId(filedlg, "MULTIVALUE", 0);

    for (i = 1; i < count; i++)
    {
      char* filetitle = IupGetAttributeId(filedlg, "MULTIVALUE", i);
      char filename[10240];
      strcpy(filename, dir);
      strcat(filename, filetitle);

      LoadImageFile(ih_item, filename);
    }

    IupConfigSetVariableStr(config, "IupVisualLED", "LastImageDirectory", dir);
  }

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

static int item_export_img_cb(Ihandle *ih_item)
{
  char* folder;
  int i, num_images, num_names = IupGetAllNames(NULL, -1);
  char* *names;
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
  char *currFilename = IupGetAttribute(multitext, "FILENAME");
  if (!currFilename) currFilename = IupGetAttribute(multitext, "NEW_FILENAME");

  char* imgtype = getfileformat(1);
  if (!imgtype)
    return IUP_DEFAULT;

  folder = getfolder(ih_item);
  if (!folder)
    return IUP_DEFAULT;

  names = malloc(sizeof(char*)*num_names);
  IupGetAllNames(names, num_names);

  num_images = 0;
  for (i = 1; i < num_names; i++)
  {
    Ihandle *elem = IupGetHandle(names[i]);

    if (!iupObjectCheck(elem))
      continue;

    if (!iupStrEqualPartial(IupGetClassType(elem), "image"))
      continue;

    /* save only loaded images of the current file */
    if (!iupStrEqual(iupAttribGet(elem, "_IUPLED_LEDFILENAME"), currFilename))
      continue;

    {
      char filename[10240] = "";
      char buff[80];

      strcpy(filename, folder);
      strcat(filename, "/");
      strcat(filename, mainGetFileTitle(filename));
      strcat(filename, "_");
      strcat(filename, names[i]);
      iupStrLower(buff, imgtype);
      strcat(filename, ".");
      strcat(filename, buff);

      if (!IupSaveImage(elem, filename, imgtype))
      {
        char* err_msg = IupGetGlobal("IUPIM_LASTERROR");
        if (err_msg)
          IupMessageError(IupGetDialog(ih_item), err_msg);
        return IUP_DEFAULT;
      }

      num_images++;
    }
  }

  if (num_images == 0)
  {
    IupMessageError(IupGetDialog(ih_item), "No images.");
    return IUP_DEFAULT;
  }

  return IUP_DEFAULT;
}

static int showimages_close_cb(void)
{
  return IUP_CLOSE;
}

static int inactivetoggle_cb(Ihandle* self, int v)
{
  Ihandle* tabs = (Ihandle*)IupGetAttribute(self, "TABS");
  Ihandle* child = IupGetNextChild(tabs, NULL);
  while (child)
  {
    IupSetAttribute(child, "ACTIVE", v ? "NO" : "YES");
    child = IupGetNextChild(tabs, child);
  }
  return IUP_DEFAULT;
}

static int imagebutton_cb(Ihandle* self)
{
  Ihandle* label = (Ihandle*)IupGetAttribute(self, "_INFO_LABEL");
  IupSetAttribute(label, "TITLE", IupGetAttribute(self, "_INFO"));
  return IUP_DEFAULT;
}

static int item_show_all_img_cb(Ihandle *ih_item)
{
  Ihandle *dialog, *box, *files, *tabs, *toggle, *label;
  Ihandle* params[500];
  int show_stock = iupAttribGetInt(ih_item, "_IUP_SHOWSTOCK");
  int num_tabs = 0, num_images;
  int i, num_names = IupGetAllNames(NULL, -1);
  char* *names = malloc(sizeof(char*)*num_names);
  Ihandle* *images = malloc(sizeof(Ihandle*)*num_names);
  IupGetAllNames(names, num_names);

  files = IupUser();

  num_images = 0;
  for (i = 0; i < num_names; i++)
  {
    Ihandle *elem = IupGetHandle(names[i]);
    char* type;

    if (!iupObjectCheck(elem))
      continue;

    type = IupGetClassType(elem);

    if (iupStrEqualPartial(type, "image"))
    {
      if (show_stock && !iupAttribGet(elem, "_IUPIMAGE_STOCK_LOAD"))
        continue;

      /* show only loaded images */
      if (!show_stock && !iupAttribGet(elem, "_IUPLED_LEDFILENAME"))
        continue;

      images[num_images] = elem;
      num_images++;
    }
  }

  if (num_images == 0)
  {
    IupMessageError(IupGetDialog(ih_item), "No images.");
    free(names);
    free(images);
    return IUP_DEFAULT;
  }

  qsort(images, num_images, sizeof(Ihandle*), compare_image_names);

  for (i = 0; i < num_images; i++)
  {
    Ihandle *tbox, *lbox, *button, *elem;
    char* name;
    char* filename;

    elem = images[i];
    name = IupGetName(elem);

    if (show_stock)
      filename = "IupImgLib";
    else
      filename = iupAttribGet(elem, "_IUPLED_LEDFILENAME");
    if (!filename)
      continue;

    tbox = (Ihandle*)IupGetAttribute(files, filename);
    if (!tbox)
    {
      char* file_title = mainGetFileTitle(filename);
      tbox = IupVbox(NULL);
      IupSetAttribute(files, filename, (char*)tbox);
      IupSetStrAttribute(tbox, "TABTITLE", file_title);
      free(file_title);
      params[num_tabs] = tbox;
      num_tabs++;
    }

    lbox = (Ihandle*)IupGetAttribute(tbox, filename);
    if (!lbox || IupGetInt(lbox, "LINE_COUNT") == 10)
    {
      lbox = IupHbox(NULL);
      IupAppend(tbox, lbox);
      IupSetAttribute(tbox, filename, (char*)lbox);
      IupSetAttribute(lbox, "LINE_COUNT", "0");
    }

    button = IupButton("", NULL);
    IupSetStrAttribute(button, "IMAGE", name);
    IupSetfAttribute(button, "_INFO", "%s [%d,%d]", name, IupGetInt(elem, "WIDTH"), IupGetInt(elem, "HEIGHT"));
    IupSetCallback(button, "ACTION", (Icallback)imagebutton_cb);
    IupAppend(lbox, button);
    IupSetfAttribute(lbox, "LINE_COUNT", "%d", IupGetInt(lbox, "LINE_COUNT") + 1);
  }

  params[num_tabs] = NULL;

  box = IupVbox(toggle = IupToggle("INACTIVE", NULL),
                tabs = IupTabsv(params),
                label = IupLabel(""),
                NULL);
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");
  IupSetAttribute(tabs, "ALIGNMENT", "NW");
  IupSetAttribute(tabs, "SIZE", "150x80");
  IupSetCallback(toggle, "ACTION", (Icallback)inactivetoggle_cb);
  IupSetAttribute(toggle, "TABS", (char*)tabs);
  IupSetAttribute(label, "EXPAND", "HORIZONTAL");

  dialog = IupDialog(box);
  IupSetAttribute(dialog, "TITLE", "All Images");
  IupSetCallback(dialog, "CLOSE_CB", (Icallback)showimages_close_cb);
  IupSetAttribute(dialog, "_INFO_LABEL", (char*)label);
  IupSetAttributeHandle(dialog, "PARENTDIALOG", IupGetDialog(ih_item));

  IupPopup(dialog, IUP_CENTER, IUP_CENTER);

  IupDestroy(dialog);
  IupDestroy(files);

  free(names);
  free(images);
  return IUP_DEFAULT;
}

static int item_show_stock_img_cb(Ihandle *ih_item)
{
  iupImageStockLoadAll();

  iupAttribSet(ih_item, "_IUP_SHOWSTOCK", "1");
  item_show_all_img_cb(ih_item);
  iupAttribSet(ih_item, "_IUP_SHOWSTOCK", NULL);

  return IUP_DEFAULT;
}


static int ivLedGetExportFile(Ihandle* ih, char* filename, const char *filter)
{
  Ihandle *filedlg = 0;
  int ret;
  Ihandle* config = get_config(ih);
  const char* dir = IupConfigGetVariableStr(config, "IupVisualLED", "LastExportDirectory");
  if (!dir) dir = IupConfigGetVariableStr(config, "IupVisualLED", "LastDirectory");

  filedlg = IupFileDlg();

  IupSetStrAttribute(filedlg, "EXTFILTER", filter);
  IupSetStrAttribute(filedlg, "FILE", filename);
  IupSetStrAttribute(filedlg, "DIRECTORY", dir);
  IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
  IupSetAttribute(filedlg, "ALLOWNEW", "YES");
  IupSetAttribute(filedlg, "NOCHANGEDIR", "YES");
  IupSetAttributeHandle(filedlg, "PARENTDIALOG", IupGetDialog(ih));
  IupSetAttribute(filedlg, "ICON", IupGetGlobal("ICON"));

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  ret = IupGetInt(filedlg, "STATUS");
  if (ret != -1)
  {
    char* value = IupGetAttribute(filedlg, "VALUE");
    strcpy(filename, value);

    dir = IupGetAttribute(filedlg, "DIRECTORY");
    IupConfigSetVariableStr(config, "IupVisualLED", "LastExportDirectory", dir);
  }

  IupDestroy(filedlg);

  return ret;
}

static int item_export_lua_action_cb(Ihandle *ih_item)
{
  char filename[4096];
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
  char *currFilename = IupGetAttribute(multitext, "FILENAME");
  char* title = iupStrFileGetTitle(currFilename);

  char *ext = strrchr(title, '.');
  *ext = 0;

  strcpy(filename, title);
  strcat(filename, ".lua");

  int ret = ivLedGetExportFile(ih_item, filename, "Lua Files|*.lua|All Files|*.*|");
  if (ret != -1) /* ret==0 existing file. TODO: check if filename is opened. */
    vLedExport(currFilename, filename, "LUA");

  return IUP_DEFAULT;
}

static int item_export_c_action_cb(Ihandle *ih_item)
{
  char filename[4096];
  Ihandle* multitext = vLedGetCurrentMultitext(ih_item);
  char *currFilename = IupGetAttribute(multitext, "FILENAME");
  char* title = iupStrFileGetTitle(currFilename);

  char *ext = strrchr(title, '.');
  *ext = 0;

  strcpy(filename, title);
  strcat(filename, ".c");

  int ret = ivLedGetExportFile(ih_item, filename, "C Files|*.c|All Files|*.*|");
  if (ret != -1) /* ret==0 existing file. TODO: check if filename is opened. */
    vLedExport(currFilename, filename, "C");

  return IUP_DEFAULT;
}

static int item_export_all_open_action_cb(Ihandle *ih_item)
{
  char *itemName = IupGetAttribute(ih_item, "NAME");
  Ihandle* tabs = IupGetDialogChild(ih_item, "MULTITEXT_TABS");
  int count = IupGetInt(tabs, "COUNT");
  char *folder;
  int i;

  folder = getfolder(ih_item);
  if (!folder)
    return IUP_DEFAULT;

  for (i = 0; i < count; i++)
  {
    char filename[1024];
    Ihandle *multitext = IupGetChild(tabs, i);
    char *currFilename = IupGetAttribute(multitext, "FILENAME");
    char* title = iupStrFileGetTitle(currFilename);

    char *ext = strrchr(title, '.');
    *ext = 0;

    strcpy(filename, folder);
    strcat(filename, "\\");
    strcat(filename, title);
    if (strcmp(itemName, "ITM_EXPORT_OPEN_LUA") == 0)
    {
      strcat(filename, ".lua");

      vLedExport(currFilename, filename, "LUA");
    }
    else
    {
      strcat(filename, ".c");

      vLedExport(currFilename, filename, "C");
    }
  }

  return IUP_DEFAULT;
}

static int item_export_proj_action_cb(Ihandle *ih_item)
{
  char *itemName = IupGetAttribute(ih_item, "NAME");
  Ihandle* projectTree = IupGetDialogChild(ih_item, "PROJECTTREE");
  int count = IupGetInt(projectTree, "COUNT");
  int i;

  char *folder = getfolder(ih_item);
  if (!folder)
    return IUP_DEFAULT;

  for (i = 1; i < count; i++)
  {
    char filename[1024];
    char* currFilename = IupTreeGetUserId(projectTree, i);

    char* title = iupStrFileGetTitle(currFilename);

    char *ext = strrchr(title, '.');
    *ext = 0;

    strcpy(filename, folder);
    strcat(filename, "\\");
    strcat(filename, title);
    if (strcmp(itemName, "ITM_EXP_PROJ_LUA") == 0)
    {
      strcat(filename, ".lua");

      vLedExport(currFilename, filename, "LUA");
    }
    else
    {
      strcat(filename, ".c");

      vLedExport(currFilename, filename, "C");
    }
  }

  return IUP_DEFAULT;
}

static int item_use_utf_8_action_cb(Ihandle *ih_item)
{
  Ihandle* config = get_config(ih_item);
  if (IupGetInt(ih_item, "VALUE"))
  {
    IupSetGlobal("UTF8MODE", "Yes");
    IupConfigSetVariableStr(config, "IupVisualLED", "UTF-8", "Yes");
  }
  else
  {
    IupSetGlobal("UTF8MODE", NULL);
    IupConfigSetVariableStr(config, "IupVisualLED", "UTF-8", NULL);
  }
  return IUP_DEFAULT;
}

static int item_imageexport_static_action_cb(Ihandle *ih_item)
{
  Ihandle* config = get_config(ih_item);
  if (IupGetInt(ih_item, "VALUE"))
  {
    IupSetGlobal("IMAGEEXPORT_STATIC", "Yes");
    IupConfigSetVariableStr(config, "IupVisualLED", "ImageExportStatic", "Yes");
  }
  else
  {
    IupSetGlobal("IMAGEEXPORT_STATIC", NULL);
    IupConfigSetVariableStr(config, "IupVisualLED", "ImageExportStatic", NULL);
  }
  return IUP_DEFAULT;
}

static int executeleaf_cb(Ihandle* self, int id)
{
  Ihandle* multitext = vLedGetCurrentMultitext(self);
  Ihandle *elem = (Ihandle *)IupTreeGetUserId(self, id);
  char *name = IupGetName(elem);
  Ihandle *parent = elem;
  int pos_start, pos_end, find_start, find_end;

  while(!name)
  {
    parent = IupGetParent(parent);
    if (!parent)
      break;
    name = IupGetName(parent);
  }

  if (!name)
    return IUP_DEFAULT;

  IupSetAttribute(multitext, "SEARCHFLAGS", "WHOLEWORD");

  find_start = 0;
  find_end = IupGetInt(multitext, "COUNT");

  IupSetInt(multitext, "TARGETSTART", find_start);
  IupSetInt(multitext, "TARGETEND", find_end);

  IupSetAttribute(multitext, "SEARCHINTARGET", name);

  pos_start = IupGetInt(multitext, "TARGETSTART");
  pos_end = IupGetInt(multitext, "TARGETEND");

  if (pos_start != 0 || pos_end != find_end)
  {
    IupSetFocus(multitext);
    IupSetInt(multitext, "CARETPOS", pos_start);
    IupSetInt(multitext, "SCROLLTOPOS", pos_start);
    IupSetfAttribute(multitext, "SELECTIONPOS", "%d:%d", pos_start, pos_end);
  }

  return IUP_DEFAULT;
}

static int locateInLED(Ihandle *ih)
{
  Ihandle* elem_tree = (Ihandle*)IupGetAttribute(ih, "ELEMENTS_TREE");
  int id = IupGetInt(elem_tree, "VALUE");

  if (id == 0)
    return IUP_DEFAULT;

  executeleaf_cb(elem_tree, id);

  return IUP_DEFAULT;
}

static int globalsdlg_cb(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  Ihandle* globals_dlg = (Ihandle*)IupGetAttribute(dlg, "GLOBALS_DIALOG");
  if (!globals_dlg)
  {
    globals_dlg = IupGlobalsDialog();
    IupSetAttributeHandle(globals_dlg, "PARENTDIALOG", dlg);
    IupSetAttribute(dlg, "GLOBALS_DIALOG", (char*)globals_dlg);
  }

  IupShow(globals_dlg);
  return IUP_DEFAULT;
}

static int classinfo_cb(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  Ihandle* classinfo_dlg = (Ihandle*)IupGetAttribute(dlg, "CLASSINFO_DIALOG");
  if (!classinfo_dlg)
  {
    classinfo_dlg = IupClassInfoDialog(dlg);
    IupSetAttribute(dlg, "CLASSINFO_DIALOG", (char*)classinfo_dlg);
  }

  IupShow(classinfo_dlg);
  return IUP_DEFAULT;
}

static int propertiesdlg_cb(Ihandle* self)
{
  Ihandle* elem_tree = (Ihandle*)IupGetAttribute(self, "ELEMENTS_TREE");
  int id = IupGetInt(elem_tree, "VALUE");
  Ihandle *elem = (Ihandle *)IupTreeGetUserId(elem_tree, id);
  if (elem)
  {
    Ihandle* properties_dlg = (Ihandle*)IupGetAttribute(elem_tree, "PROPERTIES_DIALOG");
    if (!properties_dlg)
    {
      properties_dlg = IupElementPropertiesDialog(IupGetDialog(elem_tree), elem);
      IupSetAttribute(elem_tree, "PROPERTIES_DIALOG", (char*)properties_dlg);
    }

    IupShow(properties_dlg);
  }
  else
    IupMessageError(IupGetDialog(self), "No elements.");

  return IUP_DEFAULT;
}

static int showElement_cb(Ihandle* self)
{
  Ihandle* elem_tree = (Ihandle*)IupGetAttribute(self, "ELEMENTS_TREE");
  int id = IupGetInt(elem_tree, "VALUE");
  Ihandle *elem = (Ihandle *)IupTreeGetUserId(elem_tree, id);
  if (elem) /* the tree may be empty */
  {
    if (IupClassMatch(elem, "dialog"))
      IupShow(elem);
    else
    {
      Ihandle* dialog = IupGetDialog(elem);
      if (dialog)
        IupShow(dialog);
      else
      {
        if (IupClassMatch(elem, "menu"))
          IupPopup(elem, IUP_MOUSEPOS, IUP_MOUSEPOS);
        else
          IupMessageError(IupGetDialog(self), "Will only show dialogs and independent menus.");
      }
    }
  }
  else
    IupMessageError(IupGetDialog(self), "No elements.");

  return IUP_DEFAULT;
}

static int hideElement_cb(Ihandle* self)
{
  Ihandle* elem_tree = (Ihandle*)IupGetAttribute(self, "ELEMENTS_TREE");
  int id = IupGetInt(elem_tree, "VALUE");
  Ihandle *elem = (Ihandle *)IupTreeGetUserId(elem_tree, id);
  if (elem) /* the tree may be empty */
  {
    if (IupClassMatch(elem, "dialog"))
    {
      if (IupGetInt(elem, "VISIBLE"))
        IupHide(elem);
    }
    else
    {
      Ihandle* dialog = IupGetDialog(elem);
      if (IupGetInt(dialog, "VISIBLE"))
        IupHide(dialog);
    }
  }
  else
    IupMessageError(IupGetDialog(self), "No elements.");

  return IUP_DEFAULT;
}

static int find_cb(Ihandle* ih)
{
  Ihandle* elem_tree = (Ihandle*)IupGetAttribute(ih, "ELEMENTS_TREE");
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(elem_tree, "FIND_DIALOG");
  int id = IupGetInt(elem_tree, "VALUE");
  Ihandle *dialog;
  Ihandle *elem = (Ihandle *)IupTreeGetUserId(elem_tree, id);
  if (!elem)
    return IUP_DEFAULT;

  if (!find_dlg)
  {
    find_dlg = IupLayoutFindDialog(elem_tree, elem);
    IupSetAttribute(elem_tree, "FIND_DIALOG", (char*)find_dlg);
  }

  dialog = IupGetDialog(elem);
  if (!dialog)
    IupSetAttribute(IupGetDialogChild(find_dlg, "FIND_NAME"), "ACTIVE", "NO");
  else
    IupSetAttribute(IupGetDialogChild(find_dlg, "FIND_NAME"), "ACTIVE", "YES");

  IupShow(find_dlg);

  return IUP_DEFAULT;
}

static int rightclick_cb(Ihandle* ih, int id)
{
  Ihandle *popup_menu;

  IupSetInt(ih, "VALUE", id);

  popup_menu = IupMenu(
    IupSetCallbacks(IupItem("Locate", "locateInLED"), "ACTION", locateInLED, NULL),
    IupSetCallbacks(IupItem("Properties...", "propertiesDlg"), "ACTION", propertiesdlg_cb, NULL),
    IupSetCallbacks(IupItem("Show Dialog", "showElement"), "ACTION", showElement_cb, NULL),
    IupSetCallbacks(IupItem("Hide Dialog", "hideElement"), "ACTION", hideElement_cb, NULL),
    IupSeparator(),
    IupSetCallbacks(IupItem("Find...", "findInElement"), "ACTION", find_cb, NULL),
    NULL);

  iupAttribSet(popup_menu, "ELEMENTS_TREE", (char*)ih);

  IupPopup(popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);

  IupDestroy(popup_menu);

  return IUP_DEFAULT;
}

static Ihandle* buildLedMenu(Ihandle* config)
{
  Ihandle *item_load, *item_unload, *item_autoload, *item_autocomplete, *item_style_config, *item_expand, *item_toggle, *item_level,
    *item_folding, *item_toggle_folding, *ledMenu, *item_collapse,
    *item_linescomment, *item_linesuncomment;

  item_autoload = IupItem("Auto Load", NULL);
  IupSetAttribute(item_autoload, "AUTOTOGGLE", "YES");
  IupSetAttribute(item_autoload, "NAME", "ITM_LOAD");
  IupSetCallback(item_autoload, "ACTION", (Icallback)item_autoload_action_cb);
  if (IupConfigGetVariableIntDef(config, "IupVisualLED", "AutoLoad", 1))
    IupSetAttribute(item_autoload, "VALUE", "ON");
  else
    IupSetAttribute(item_autoload, "VALUE", "OFF");

  item_load = IupItem("Load", NULL);
  IupSetCallback(item_load, "ACTION", (Icallback)item_load_action_cb);

  item_unload = IupItem("Unload", NULL);
  IupSetCallback(item_unload, "ACTION", (Icallback)item_unload_action_cb);

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
    item_autoload,
    item_load,
    item_unload,
    IupSeparator(),
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

  return IupSubmenu("&LED", ledMenu);
}

static Ihandle* buildToolsMenu(void)
{
  Ihandle *item_import_img, *item_export_img, *item_show_all_img, *item_export_lua,
    *item_export_open_lua, *item_export_proj_lua, *item_export_c, *item_export_open_c,
    *item_export_proj_c, *item_use_utf8, *toolsMenu, *item_imageexport_static, *item_show_stock_img;

  item_import_img = IupItem("Import Images...", NULL);
  IupSetAttribute(item_import_img, "NAME", "ITM_IMP_IMG");
  IupSetCallback(item_import_img, "ACTION", (Icallback)item_import_img_action_cb);

  item_export_img = IupItem("Export Images...", NULL);
  IupSetAttribute(item_export_img, "NAME", "ITM_EXP_IMG");
  IupSetCallback(item_export_img, "ACTION", (Icallback)item_export_img_cb);

  item_show_all_img = IupItem("Show All Images...", NULL);
  IupSetAttribute(item_show_all_img, "NAME", "ITM_SHOW_ALL_IMG");
  IupSetCallback(item_show_all_img, "ACTION", (Icallback)item_show_all_img_cb);

  item_show_stock_img = IupItem("Show Stock Images...", NULL);
  IupSetCallback(item_show_stock_img, "ACTION", (Icallback)item_show_stock_img_cb);

  item_export_lua = IupItem("Export to Lua...", NULL);
  IupSetAttribute(item_export_lua, "NAME", "ITM_EXP_LUA");
  IupSetCallback(item_export_lua, "ACTION", (Icallback)item_export_lua_action_cb);

  item_export_open_lua = IupItem("Export All Open to Lua(s)...", NULL);
  IupSetAttribute(item_export_open_lua, "NAME", "ITM_EXPORT_OPEN_LUA");
  IupSetCallback(item_export_open_lua, "ACTION", (Icallback)item_export_all_open_action_cb);

  item_export_proj_lua = IupItem("Export All Project to Lua(s)...", NULL);
  IupSetAttribute(item_export_proj_lua, "NAME", "ITM_EXP_PROJ_LUA");
  IupSetCallback(item_export_proj_lua, "ACTION", (Icallback)item_export_proj_action_cb);

  item_export_c = IupItem("Export to C...", NULL);
  IupSetAttribute(item_export_c, "NAME", "ITM_EXP_C");
  IupSetCallback(item_export_c, "ACTION", (Icallback)item_export_c_action_cb);

  item_export_open_c = IupItem("Export All Open to C(s)...", NULL);
  IupSetAttribute(item_export_open_c, "NAME", "ITM_EXPORT_OPEN_C");
  IupSetCallback(item_export_open_c, "ACTION", (Icallback)item_export_all_open_action_cb);

  item_export_proj_c = IupItem("Export All Project To C(s)...", NULL);
  IupSetAttribute(item_export_proj_c, "NAME", "ITM_EXP_PROJ_C");
  IupSetCallback(item_export_proj_c, "ACTION", (Icallback)item_export_proj_action_cb);

  item_use_utf8 = IupItem("Use UTF-8", NULL);
  IupSetAttribute(item_use_utf8, "AUTOTOGGLE", "Yes");
  IupSetCallback(item_use_utf8, "ACTION", (Icallback)item_use_utf_8_action_cb);
  if (IupGetInt(NULL, "UTF8MODE"))
    IupSetAttribute(item_use_utf8, "VALUE", "ON");
  else
    IupSetAttribute(item_use_utf8, "VALUE", "OFF");

  item_imageexport_static = IupItem("Image Export Use \"static\"", NULL);
  IupSetAttribute(item_imageexport_static, "AUTOTOGGLE", "Yes");
  IupSetCallback(item_imageexport_static, "ACTION", (Icallback)item_imageexport_static_action_cb);
  if (IupGetInt(NULL, "IMAGEEXPORT_STATIC"))
    IupSetAttribute(item_imageexport_static, "VALUE", "ON");
  else
    IupSetAttribute(item_imageexport_static, "VALUE", "OFF");

  toolsMenu = IupMenu(
    IupSetCallbacks(IupItem("Globals...", NULL), "ACTION", globalsdlg_cb, NULL),
    IupSetCallbacks(IupItem("Class Info...", NULL), "ACTION", classinfo_cb, NULL),
    IupSeparator(),
    item_import_img,
    item_export_img,
    item_show_all_img,
    item_show_stock_img,
    IupSeparator(),
    item_export_lua,
    item_export_open_lua,
    item_export_proj_lua,
    IupSeparator(),
    item_export_c,
    item_export_open_c,
    item_export_proj_c,
    IupSeparator(),
    item_imageexport_static,
    item_use_utf8,
    NULL);

  IupSetCallback(toolsMenu, "OPEN_CB", (Icallback)tools_menu_open_cb);

  return IupSubmenu("&Tools", toolsMenu);
}

static int exit_cb(Ihandle* ih)
{
  int i;
  Ihandle *dlg;
  int count = iupDlgListVisibleCount();

  /* hide all other dialogs */

  for (dlg = iupDlgListFirst(), i = 0; dlg && i < count; dlg = iupDlgListNext())
  {
    if (dlg != ih && (dlg->handle && IupGetInt(dlg, "VISIBLE")))
    {
      IupHide(dlg);
      i++;
    }
  }

  return IUP_DEFAULT;
}


int main(int argc, char **argv)
{
  Ihandle *main_dialog;
  Ihandle *config;
  Ihandle *menu;
  Ihandle *ledMenu, *toolsMenu;
  Ihandle *panelTabs;
  Ihandle *multitextTabs;
  Ihandle *elem_tree;
  Ihandle *elementsFrame;
  Icallback oldTabChange_cb;
  Ihandle* extra_dlg;
  int i;

  IupOpen(&argc, &argv);
  IupImageLibOpen();
  IupScintillaOpen();
  IupControlsOpen();
#ifndef USE_NO_OPENGL  
  IupGLCanvasOpen();
  IupGLControlsOpen();
#endif  
#ifndef USE_NO_WEB
  IupWebBrowserOpen();
#endif  
#ifndef USE_NO_PLOT
  IupPlotOpen();
#endif  

#ifdef _DEBUG
  IupSetGlobal("GLOBALLAYOUTDLGKEY", "Yes");
#endif

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "iupvled");
  IupConfigLoad(config);

  IupSetGlobal("UTF8MODE", IupConfigGetVariableStr(config, "IupVisualLED", "UTF-8"));
  IupSetGlobal("IMAGEEXPORT_STATIC", IupConfigGetVariableStr(config, "IupVisualLED", "ImageExportStatic"));

  main_dialog = IupScintillaDlg();

  ledMenu = buildLedMenu(config);
  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupInsert(menu, IupGetChild(menu, IupGetChildCount(menu) - 1), ledMenu);

  toolsMenu = buildToolsMenu();
  IupInsert(menu, IupGetChild(menu, IupGetChildCount(menu) - 1), toolsMenu);

  panelTabs = IupGetDialogChild(main_dialog, "PANEL_TABS");

  multitextTabs = IupGetDialogChild(main_dialog, "MULTITEXT_TABS");

  elem_tree = IupTree();
  IupSetAttribute(elem_tree, "EXPAND", "YES");
  IupSetAttribute(elem_tree, "NAME", "ELEMENTS_TREE");
  IupSetAttribute(elem_tree, "ADDROOT", "NO");
  IupSetCallback(elem_tree, "EXECUTELEAF_CB", (Icallback)executeleaf_cb);
  IupSetCallback(elem_tree, "RIGHTCLICK_CB", (Icallback)rightclick_cb);
  IupSetAttribute(elem_tree, "VISIBLELINES", "3");
  IupSetAttributeHandle(elem_tree, "IMG_SHORTCUT", load_image_shortcut());

  elementsFrame = IupFrame(elem_tree);
  IupSetAttribute(elementsFrame, "NAME", "ELEMENTS_FRAME");
  IupSetAttribute(elementsFrame, "MARGIN", "4x4");
  IupSetAttribute(elementsFrame, "GAP", "4");
  IupSetAttribute(elementsFrame, "TITLE", "Elements:");
  IupSetAttribute(elementsFrame, "TABTITLE", "Elements");

  IupAppend(panelTabs, elementsFrame);

  IupSetAttribute(panelTabs, "VALUE_HANDLE", (char*)elementsFrame);

  IupSetAttribute(main_dialog, "SUBTITLE", "IupVisualLED");
  IupSetAttributeHandle(main_dialog, "CONFIG", config);
  IupSetHandle("VLED_MAIN", main_dialog);
  IupSetAttribute(main_dialog, "PROJECTEXT", "vled");

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
  IupSetCallback(main_dialog, "EXIT_CB", (Icallback)exit_cb);

  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupAppend(menu, IupSubmenu("&Help", IupMenu(
    IupSetCallbacks(IupItem("&Help...", NULL), "ACTION", (Icallback)item_help_action_cb, NULL),
    IupSetCallbacks(IupItem("&About...", NULL), "ACTION", (Icallback)item_about_action_cb, NULL),
    NULL)));

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, main_dialog, "IupVisualLED");

  /* open a file from the command line (allow file association in Windows) */
  for (i = 1; i < argc; i++)
  {
    const char* filename = argv[i];
    if (strstr(filename, ".vled"))
      IupSetStrAttribute(main_dialog, "OPENPROJECT", filename);
    else
      IupSetStrAttribute(main_dialog, "OPENFILE", filename);
  }

  /* Call NEW_TEXT_CB because the first tab was already created */
  newtext_cb(main_dialog, vLedGetCurrentMultitext(main_dialog));

  IupMainLoop();

  extra_dlg = (Ihandle*)IupGetAttribute(elem_tree, "FIND_DIALOG");
  if (iupObjectCheck(extra_dlg))
    IupDestroy(extra_dlg);
  extra_dlg = (Ihandle*)IupGetAttribute(elem_tree, "PROPERTIES_DIALOG");
  if (iupObjectCheck(extra_dlg))
    IupDestroy(extra_dlg);
  extra_dlg = (Ihandle*)IupGetAttribute(main_dialog, "GLOBALS_DIALOG");
  if (iupObjectCheck(extra_dlg))
    IupDestroy(extra_dlg);
  extra_dlg = (Ihandle*)IupGetAttribute(main_dialog, "CLASSINFO_DIALOG");
  if (iupObjectCheck(extra_dlg))
    IupDestroy(extra_dlg);

  IupDestroy(main_dialog);
  IupDestroy(config);

  IupClose();
  return EXIT_SUCCESS;
}
