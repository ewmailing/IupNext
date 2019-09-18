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



int vLedIsAlien(Ihandle *elem, const char* filename);
Ihandle* vLedGetCurrentMultitext(Ihandle* ih);

#define VLED_MAX_NAMES 5000
#define VLED_ATTRIB_ISINTERNAL(_name) (((_name[0] == 'V' && _name[1] == 'L' && _name[2] == 'E' && _name[3] == 'D') || (_name[0] == 'v' && _name[1] == 'l' && _name[2] == 'e' && _name[3] == 'd')) ? 1 : 0)

enum{VLED_EXPORT_LUA, VLED_EXPORT_C, VLED_EXPORT_LED};

static void iExportRemoveExt(char* title, const char* ext)
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

static int iExportHasReserved(const char* name)
{
  char c = *name;
  if (c >= '1' && c <= '9') /* first character can NOT be a number */
    return 0;
  
  while (*name)
  {
    c = *name;

    /* can only has letters or numbers as characters */
    if ( c < '0' || 
        (c > '9' && c < 'A') ||
        (c > 'Z' && c < 'a') ||
         c > 'z')
      return 1;

    name++;
  }
  return 0;
}

static void iExportWriteAttrib(FILE* file, const char* name, const char* value, const char* indent, int export_format)
{
  char attribname[1024];
  const char* old_value = value;
  value = iupStrConvertToC(value);

  if (export_format == VLED_EXPORT_LUA)  /* Lua */
  {
    iupStrLower(attribname, name);
    if (iExportHasReserved(attribname))
      fprintf(file, "%s[\"%s\"] = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
  }
  else if (export_format == VLED_EXPORT_LED) /* LED */
  {
    if (iExportHasReserved(attribname))  /* can not be saved in LED */
    {
      if (old_value != value)
        free((char*)value);
      return;
    }

    iupStrUpper(attribname, name);
    if (iExportHasReserved(value))
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = %s,\n", indent, attribname, value);
  }
  else   /* C */
    fprintf(file, "%s\"%s\", \"%s\",\n", indent, name, value);

  if (old_value != value)
    free((char*)value);
}

static int iExportCheckAttributeChanged(Ihandle* ih, const char* name, const char* value, const char* def_value, int flags)
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

static int iExportElementAttribs(FILE* file, Ihandle* ih, const char* indent, int export_format)
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

    if (iExportCheckAttributeChanged(ih, name, value, def_value, flags))
    {
      iExportWriteAttrib(file, name, value, localIndent, export_format);
      wcount++;
    }

    if (has_attrib_id && flags&IUPAF_HAS_ID)
    {
      flags &= ~IUPAF_HAS_ID; /* clear flag so the next function call can work */
      if (iExportCheckAttributeChanged(ih, name, "X", NULL, flags))
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
                iExportWriteAttrib(file, str, value, localIndent, export_format);
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
              iExportWriteAttrib(file, str, value, indent, export_format);
              wcount++;
            }
          }
        }
      }
    }
  }

  if (export_format != VLED_EXPORT_LUA)  /* LED or C */
  {
    int cb_count = total_count - attr_count;
    IupGetClassCallbacks(ih->iclass->name, attr_names, cb_count);
    for (i = 0; i < cb_count; i++)
    {
      char* cb_name = iupGetCallbackName(ih, attr_names[i]);
      if (cb_name && cb_name[0] && !iupATTRIB_ISINTERNAL(cb_name))
      {
        iExportWriteAttrib(file, attr_names[i], cb_name, localIndent, export_format);
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
      iExportWriteAttrib(file, attr_names[i], value, localIndent, export_format);
    }
  }

  if (export_format == VLED_EXPORT_LED) /* LED */
  {
    /* remove last comma ',' and new line */
    /* if wcount==0, it will remove '[' and new line */
    fseek(file, -2, SEEK_CUR);
  }

  free(attr_names);
  return wcount;
}

static void iExportElementC(FILE* file, Ihandle* ih, const char *indent, const char* terminator)
{
  char* name = IupGetName(ih);
  const char *elemClass = ih->iclass->name;

  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;

  fprintf(file, "%sIupSetAtt(%s%s%s, IupCreate(\"%s\"), \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", elemClass);

  iExportElementAttribs(file, ih, indent, VLED_EXPORT_C);

  fprintf(file, "%s  NULL)%s\n", indent, terminator);

  if (terminator[0] == ';')
    fprintf(file, "\n");
}

static void iExportElementLua(FILE* file, Ihandle* ih, const char *indent)
{
  char* name = IupGetName(ih);

  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;

  if (name)
    fprintf(file, "%slocal %s = ", indent, name);
  else
    fprintf(file, "%s", indent);

  fprintf(file, "iup.%s\n", ih->iclass->name);
  fprintf(file, "%s{\n", indent);

  iExportElementAttribs(file, ih, indent, VLED_EXPORT_LUA);

  fprintf(file, "%s}", indent);
}

static void iExportContainerC(FILE* file, Ihandle* ih, const char *indent, const char* terminator)
{
  Ihandle *child;
  char* name = IupGetName(ih);
  char localIndent[1024];

  fprintf(file, "%sIupSetAtt(%s%s%s, IupCreatep(\"%s\", \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", ih->iclass->name);

  strcpy(localIndent, indent);
  strcat(localIndent, "    ");  /* indent twice for IupCreate */

  for (child = ih->firstchild; child; child = child->brother)
  {
    char* childName = IupGetName(child);
    int isContainer = child->iclass->childtype != IUP_CHILDNONE;

    if (childName)
      fprintf(file, "%sIupGetHandle(\"%s\"),\n", localIndent, childName);
    else if (isContainer)
      iExportContainerC(file, child, localIndent, ",");  /* no ; */
    else
      iExportElementC(file, child, localIndent, ","); /* no ; */
  }

  fprintf(file, "%sNULL),\n", localIndent);  /* IupCreate */

  iExportElementAttribs(file, ih, indent, VLED_EXPORT_C);

  fprintf(file, "%s  NULL)%s\n", indent, terminator);  /* IupSetAtt */

  if (terminator[0] == ';')
    fprintf(file, "\n");
}

static void iExportContainerLua(FILE* file, Ihandle* ih, const char *indent)
{
  Ihandle *child;
  char* name = IupGetName(ih);
  char localIndent[1024];

  if (name)
    fprintf(file, "%slocal %s = ", indent, name);
  else
    fprintf(file, "%s", indent);

  fprintf(file, "iup.%s\n", ih->iclass->name);
  fprintf(file, "%s{\n", indent);

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  for (child = ih->firstchild; child; child = child->brother)
  {
    char* childName = IupGetName(child);
    int isContainer = child->iclass->childtype != IUP_CHILDNONE;

    if (childName)
    {
      if (iupAttribGet(child, "_IUPVLED_EXPORT_SAVED")) /* saved in the same scope */
        fprintf(file, "%s%s,\n", localIndent, childName);
      else
        fprintf(file, "%siup.GetHandle(\"%s\"),\n", localIndent, childName);
    }
    else if (isContainer)
    {
      iExportContainerLua(file, child, localIndent);
      fprintf(file, ",\n");
    }
    else
    {
      iExportElementLua(file, child, localIndent);
      fprintf(file, ",\n");
    }
  }

  iExportElementAttribs(file, ih, indent, VLED_EXPORT_LUA);

  fprintf(file, "%s}", indent);
}

static int iExportIsRelated(Ihandle *elem, Ihandle *container)
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

  if (childType1 == IUP_CHILDNONE && childType2 == IUP_CHILDNONE)
  {
    if (iupStrEqualPartial(elemClass1, "image") && !iupStrEqualPartial(elemClass2, "image"))
      return -1;
    else if (iupStrEqualPartial(elemClass2, "image") && !iupStrEqualPartial(elemClass1, "image"))
      return 1;
    return strcmp(name1, name2);
  }
  else if ((childType1 != IUP_CHILDNONE && childType2 != IUP_CHILDNONE))
  {
    if (iExportIsRelated(ih1, ih2))
      return -1;
    else if (iExportIsRelated(ih2, ih1))
      return 1;
    else
      return strcmp(name1, name2);
  }
  else if (childType2 != IUP_CHILDNONE && childType1 == IUP_CHILDNONE)
    return -1;

  return 1;
}

void vLedExport(Ihandle* ih, const char* filename, const char* format)
{
  Ihandle* multitext = vLedGetCurrentMultitext(ih);
  char* led_filename = IupGetAttribute(multitext, "FILENAME");
  char* title;
  char *names[VLED_MAX_NAMES];
  int num_names;
  Iarray* names_array;
  Ihandle* *data = NULL;
  int i, count = 0;
  FILE* file;
  int export_format = iupStrEqualNoCase(format, "LUA") ? VLED_EXPORT_LUA : (iupStrEqualNoCase(format, "LED") ? VLED_EXPORT_LED : VLED_EXPORT_C);

  names_array = iupArrayCreate(1024, sizeof(Ihandle*));  /* just set an initial size, but count is 0 */

  num_names = IupGetAllNames(names, VLED_MAX_NAMES);

  for (i = 0; i < num_names; i++)
  {
    Ihandle *elem = IupGetHandle(names[i]);
    if (elem)
    {
      if (iupAttribGetInt(elem, "VLED_INTERNAL") != 0 ||
          iupStrEqualPartial(names[i], "_IUP_NAME") ||
          vLedIsAlien(elem, led_filename))
        continue;

      data = iupArrayAdd(names_array, 1);
      data[count] = elem;
      count++;
    }
  }

  if (count == 0)
  {
    iupArrayDestroy(names_array);
    return;
  }

  qsort(data, count, sizeof(Ihandle*), compare_elem);

  file = fopen(filename, "wb");
  if (!file)
  {
    iupArrayDestroy(names_array);
    return;
  }

  title = iupStrFileGetTitle(filename);

  if (export_format == VLED_EXPORT_LUA)
  {
    iExportRemoveExt(title, "lua");

    fprintf(file, "--   Generated by IupVisualLED export to Lua.\n\n");
    fprintf(file, "function create_%s()\n\n", title);
  }
  else if (export_format == VLED_EXPORT_C)
  {
    iExportRemoveExt(title, "c");

    fprintf(file, "/*   Generated by IupVisualLED export to C.   */\n\n");
    fprintf(file, "#include <stdlib.h>\n");
    fprintf(file, "#include <iup.h>\n\n");
    fprintf(file, "void create_%s(void)\n", title);
    fprintf(file, "{\n");
  }
  else /* LED */
  {
    fprintf(file, "#   Generated by IupVisualLED export to LED.\n\n");
  }

  for (i = 0; i < count; i++)
  {
    Ihandle *elem = data[i];
    int isContainer = elem->iclass->childtype != IUP_CHILDNONE;
    const char *elemClass = elem->iclass->name;
    char* name = IupGetName(elem);

    if (iupStrEqualPartial(elemClass, "image"))
    {
      iupImageExportToFile(elem, file, format, name, 0);

      if (export_format == VLED_EXPORT_LUA && name)
      {
        fprintf(file, "  iup.SetHandle(\"%s\", %s)\n\n", name, name);
        iupAttribSet(elem, "_IUPVLED_EXPORT_SAVED", "1");
      }
    }
    else
    {
      if (!isContainer)
      {
        if (export_format == VLED_EXPORT_LUA)
          iExportElementLua(file, elem, "  ");
        else if (export_format == VLED_EXPORT_C)
          iExportElementC(file, elem, "  ", ";");
      }
      else
      {
        if (export_format == VLED_EXPORT_LUA)
          iExportContainerLua(file, elem, "  ");
        else if (export_format == VLED_EXPORT_C)
          iExportContainerC(file, elem, "  ", ";");
      }

      if (export_format == VLED_EXPORT_LUA && name)
      {
        fprintf(file, "\n  iup.SetHandle(\"%s\", %s)\n\n", name, name);
        iupAttribSet(elem, "_IUPVLED_EXPORT_SAVED", "1");
      }
    }
  }

  if (export_format == VLED_EXPORT_LUA)
  {
    for (i = 0; i < count; i++)
    {
      Ihandle *elem = data[i];
      char* name = IupGetName(elem);
      if (name)
        iupAttribSet(elem, "_IUPVLED_EXPORT_SAVED", NULL);
    }
  }

  if (export_format == VLED_EXPORT_LUA)
    fprintf(file, "end\n");
  else if (export_format == VLED_EXPORT_C)
    fprintf(file, "}\n");

  iupArrayDestroy(names_array);
  fclose(file);
  free(title);
}

