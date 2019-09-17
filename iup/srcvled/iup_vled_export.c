#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "iup.h"
#include "iup_object.h"
#include "iup_array.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_image.h"
#include "iup_func.h"


#define MAX_NAMES 5000

int isAlien(Ihandle *elem, const char* filename);
Ihandle* get_current_multitext(Ihandle* ih);
#define VLED_ATTRIB_ISINTERNAL(_name) (((_name[0] == 'V' && _name[1] == 'L' && _name[2] == 'E' && _name[3] == 'D') || (_name[0] == 'v' && _name[1] == 'l' && _name[2] == 'e' && _name[3] == 'd')) ? 1 : 0)

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

static int iLayoutAttributeChanged(Ihandle* ih, const char* name, const char* value, const char* def_value, int flags)
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

static int iLayoutExportElementAttribs(FILE* file, Ihandle* ih, const char* indent, int type)
{
  int i, wcount = 0, attr_count, has_attrib_id = ih->iclass->has_attrib_id, start_id = 0,
    total_count = IupGetClassAttributes(ih->iclass->name, NULL, 0);
  char **attr_names = (char **)malloc(total_count * sizeof(char *));
  char localIndent[1024];

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  if (IupClassMatch(ih, "tree") || /* tree can only set id attributes after map, so they can not be saved */
      IupClassMatch(ih, "cells"))  /* cells does not have any savable id attributes */
      has_attrib_id = 0;

  if (IupClassMatch(ih, "list"))
    start_id = 1;

  attr_count = IupGetClassAttributes(ih->iclass->name, attr_names, total_count);
  for (i = 0; i < attr_count; i++)
  {
    char *name = attr_names[i];
    char* value = iupAttribGetLocal(ih, name);
    char* def_value;
    int flags;

    iupClassGetAttribNameInfo(ih->iclass, name, &def_value, &flags);

    if (iLayoutAttributeChanged(ih, name, value, def_value, flags))
    {
      char* str = iupStrConvertToC(value);

      iLayoutExportWriteAttrib(file, name, str, localIndent, type);

      if (str != value)
        free(str);

      wcount++;
    }

    if (has_attrib_id && flags&IUPAF_HAS_ID)
    {
      flags &= ~IUPAF_HAS_ID; /* clear flag so the next function call can work */
      if (iLayoutAttributeChanged(ih, name, "X", NULL, flags))
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
                iLayoutExportWriteAttrib(file, str, value, localIndent, type);
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
        iLayoutExportWriteAttrib(file, attr_names[i], cb_name, localIndent, type);
        wcount++;
      }
    }
  }

  attr_count = IupGetAllAttributes(ih, NULL, 0);
  if (attr_count > total_count)
    attr_names = (char **)realloc(attr_names, attr_count * sizeof(char *));

  attr_count = IupGetAllAttributes(ih, attr_names, attr_count);
  for (i = 0; i < attr_count; i++)
  {
    if (!iupClassAttribIsRegistered(ih->iclass, attr_names[i]) && VLED_ATTRIB_ISINTERNAL(attr_names[i]) == 0)
    {
      char* value = iupAttribGetLocal(ih, attr_names[i]);
      iLayoutExportWriteAttrib(file, attr_names[i], value, localIndent, type);
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

static void iLayoutExportElementC(FILE* file, Ihandle* ih, char *indent)
{
  char* name = IupGetName(ih);
  const char *elemClass = ih->iclass->name;
  char localIndent[1024];

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;

  fprintf(file, "%sIupSetAtt(%s%s%s, IupCreate(\"%s\"), \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", elemClass);

  iLayoutExportElementAttribs(file, ih, localIndent, 0);  /* C */
}

static void iStrSet(char* str, char c, int count)
{
  while (count)
  {
    *str++ = c;
    count--;
  }
  *str = 0;
}

static void iLayoutExportElementLua(FILE* file, Ihandle* ih, char *indent)
{
  char* name = IupGetName(ih);
  char localIndent[1024];

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;

  if (name)
  {
    int len;
    char buff[80];
    sprintf(buff, "local %s = ", name);
    fprintf(file, "%s", buff);
    len = (int)strlen(buff);  
    iStrSet(buff, ' ', len);
    strcat(localIndent, buff);
  }
  else
    fprintf(file, "%s", localIndent);

  fprintf(file, "iup.%s{\n", ih->iclass->name);

  iLayoutExportElementAttribs(file, ih, localIndent, 1);  /* LUA */

  fprintf(file, "%s}", localIndent);
}

static void iLayoutExportContainerC(FILE* file, Ihandle* ih, char *indent)
{
  Ihandle *child;
  char* name = IupGetName(ih);
  char localIndent[1024];

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  fprintf(file, "%sIupSetAtt(%s%s%s, IupCreatep(\"%s\", \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", ih->iclass->name);

  for (child = ih->firstchild; child; child = child->brother)
  {
    char* childName = IupGetName(child);
    int isContainer = child->iclass->childtype;

    if (childName)
      fprintf(file, "%sIupGetHandle(\"%s\"),\n", localIndent, childName);
    else if (isContainer)
    {
      iLayoutExportContainerC(file, child, localIndent);
      fprintf(file, "%sNULL),\n", localIndent);
    }
    else
    {
      iLayoutExportElementC(file, child, localIndent);
      fprintf(file, "%sNULL),\n", localIndent);
    }
  }
  fprintf(file, "%sNULL),\n", localIndent);

  iLayoutExportElementAttribs(file, ih, localIndent, 0);  /* C */
}

static void iLayoutExportContainerLua(FILE* file, Ihandle* ih, char *indent)
{
  Ihandle *child;
  char* name = IupGetName(ih);
  char localIndent[1024];

  strcpy(localIndent, indent);

  if (name)
  {
    int len;
    char buff[80];
    sprintf(buff, "  local %s = ", name);
    fprintf(file, "%s", buff);
    len = (int)strlen(buff);
    if (len > 80) len = 80;
    iStrSet(buff, ' ', len);
    strcat(localIndent, buff);
  }
  else
  {
    strcat(localIndent, "  ");
    fprintf(file, "%s", localIndent);
  }

  fprintf(file, "iup.%s{\n", ih->iclass->name);

  for (child = ih->firstchild; child; child = child->brother)
  {
    char* childName = IupGetName(child);
    int isContainer = child->iclass->childtype;

    if (childName)
      fprintf(file, "%siup.GetHandle(\"%s\"),\n", localIndent, childName);
    else if (isContainer)
    {
      iLayoutExportContainerLua(file, child, localIndent);
      fprintf(file, ",\n");
    }
    else
    {
      iLayoutExportElementLua(file, child, localIndent);
      fprintf(file, ",\n");
    }
  }

  iLayoutExportElementAttribs(file, ih, localIndent, 1);  /* LUA */
  fprintf(file, "%s}", localIndent);

}

static int isRelated(Ihandle *elem, Ihandle *container)
{
  Ihandle *parent = IupGetParent(elem);
  if (parent == container)
    return -1;

  while (parent != NULL)
  {
    if (parent == elem)
      return 1;
    parent = IupGetParent(parent);
  }

  return 0;
}

static int compare_elem(const void* i1, const void* i2)
{
  Ihandle* ih1 = *((Ihandle**)i1);
  Ihandle* ih2 = *((Ihandle**)i2);
  char* name1 = IupGetName(ih1);
  char* name2 = IupGetName(ih2);
  int childType1 = ih1->iclass->childtype;
  int childType2 = ih2->iclass->childtype;
  const char *elemClass1 = ih1->iclass->name;
  const char *elemClass2 = ih2->iclass->name;

  if (childType1 == 0 && childType2 == 0)
  {
    if (iupStrEqualPartial(elemClass1, "image") && !iupStrEqualPartial(elemClass2, "image"))
      return -1;
    else if (iupStrEqualPartial(elemClass2, "image") && !iupStrEqualPartial(elemClass1, "image"))
      return 1;
    return strcmp(name1, name2);
  }
  else if ((childType1 > 0 && childType2 > 0))
  {
    if (isRelated(ih1, ih2))
      return -1;
    else if (isRelated(ih2, ih1))
      return 1;
    else
      return strcmp(name1, name2);
  }
  else if (childType2 > 0 && childType1 == 0)
    return -1;

  return 1;
}

void ivLedExport(Ihandle* ih, const char* filename, const char* format)
{
  Ihandle* multitext = get_current_multitext(ih);
  FILE* file = fopen(filename, "wb");
  char* ledFile = IupGetAttribute(multitext, "FILENAME");
  char* title = iupStrFileGetTitle(filename);
  char *names[MAX_NAMES];
  int num_names = IupGetAllNames(names, MAX_NAMES);
  Iarray* names_array = iupArrayCreate(1024, sizeof(Ihandle*));  /* just set an initial size, but count is 0 */
  Ihandle **data = iupArrayGetData(names_array);
  int i, count;

  if (!file)
    return;

  for (i = 0; i < num_names; i++)
  {
    Ihandle *elem = IupGetHandle(names[i]);

    if (IupGetInt(elem, "VLED_INTERNAL") != 0 || (elem && isAlien(elem, ledFile)))
      continue;

    count = iupArrayCount(names_array);

    iupArrayAdd(names_array, 1);

    data = iupArrayGetData(names_array);

    data[count] = elem;
  }

  count = iupArrayCount(names_array);

  qsort(data, count, sizeof(Ihandle*), compare_elem);

  if (iupStrEqualNoCase(format, "LUA"))
  {
    iLayoutRemoveExt(title, "lua");

    fprintf(file, "--   Generated by IupVisualLED export to Lua.\n\n");
    fprintf(file, "function create_%s()\n\n", title);
  }
  else
  {
    iLayoutRemoveExt(title, "c");

    fprintf(file, "/*   Generated by IupVisualLED export to C.   */\n\n");
    fprintf(file, "#include <stdlib.h>\n");
    fprintf(file, "#include <iup.h>\n\n");
    fprintf(file, "void create_%s(void)\n", title);
    fprintf(file, "{\n");
  }

  for (i = 0; i < count; i++)
  {
    Ihandle *elem = data[i];
    int isContainer = elem->iclass->childtype;
    const char *elemClass = elem->iclass->name;

    if (strcmp(elemClass, "image") == 0 || strcmp(elemClass, "imagergb") == 0 || strcmp(elemClass, "imagergba") == 0)
    {
      iupImageSaveToFile(elem, file, format, IupGetName(elem), 0);
      if (iupStrEqualNoCase(format, "LUA"))
      {
        fprintf(file, "\n");
        fprintf(file, "\n\n  iup.SetHandle(\"%s\", %s);\n", IupGetName(elem), IupGetName(elem));
        fprintf(file, "\n");
      }
    }
    else
    {
      if (!isContainer)
      {
        if (iupStrEqualNoCase(format, "LUA"))
          iLayoutExportElementLua(file, elem, "  ");
        else
          iLayoutExportElementC(file, elem, "  ");
      }
      else
      {
        if (iupStrEqualNoCase(format, "LUA"))
          iLayoutExportContainerLua(file, elem, "  ");
        else
          iLayoutExportContainerC(file, elem, "  ");
      }


      if (iupStrEqualNoCase(format, "LUA"))
        fprintf(file, "\n\n  iup.SetHandle(\"%s\", %s);\n", IupGetName(elem), IupGetName(elem));
      else
        fprintf(file, "    NULL);\n\n");
    }
  }

  if (iupStrEqualNoCase(format, "LUA"))
    fprintf(file, "end\n");
  else
    fprintf(file, "}\n");

  iupArrayDestroy(names_array);

  fclose(file);
}

