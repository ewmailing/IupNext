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

enum{VLED_EXPORT_LUA, VLED_EXPORT_C, VLED_EXPORT_LED};

static void iLayoutExportRemoveExt(char* title, const char* ext)
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

static int iLayoutExportHasReserved(const char* name)
{
  char c = *name;
  if (c >= '1' && c <= '9') /* first character can NOT be a number */
    return 0;
  
  while (*name)
  {
    c = *name;

    /* can only has letters or numbers as characters, or underscore */
    if ( c < '0' || 
        (c > '9' && c < 'A') ||
        (c > 'Z' && c < 'a' && c != '_') ||
         c > 'z')
      return 1;

    name++;
  }
  return 0;
}

static void iLayoutExportWriteAttrib(FILE* file, const char* name, const char* value, const char* indent, int export_format)
{
  char attribname[1024];
  const char* old_value = value;
  value = iupStrConvertToC(value);

  if (export_format == VLED_EXPORT_LUA)  /* Lua */
  {
    iupStrLower(attribname, name);
    if (iLayoutExportHasReserved(attribname))
      fprintf(file, "%s[\"%s\"] = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
  }
  else if (export_format == VLED_EXPORT_LED) /* LED */
  {
    if (iLayoutExportHasReserved(attribname))  /* can not be saved in LED */
    {
      if (old_value != value)
        free((char*)value);
      return;
    }

    iupStrUpper(attribname, name);
    if (iLayoutExportHasReserved(value))
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = %s,\n", indent, attribname, value);
  }
  else   /* C */
    fprintf(file, "%s\"%s\", \"%s\",\n", indent, name, value);

  if (old_value != value)
    free((char*)value);
}

static void iLayoutExportElementAttribs(FILE* file, Ihandle* ih, const char* indent, int export_format)
{
  int i, attr_count;
  char **attr_names;
  char localIndent[1024];

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  attr_count = iupAttribGetAllSaved(ih, NULL, 0);
  attr_names = (char **)malloc(attr_count * sizeof(char *));

  attr_count = iupAttribGetAllSaved(ih, attr_names, attr_count);
  for (i = 0; i < attr_count; i++)
  {
    if (export_format != VLED_EXPORT_LUA || !iupClassObjectAttribIsCallback(ih, attr_names[i]))
    {
      char* value = iupAttribGetLocal(ih, attr_names[i]);
      iLayoutExportWriteAttrib(file, attr_names[i], value, localIndent, export_format);
    }
  }

  if (export_format == VLED_EXPORT_LED) /* LED */
  {
    /* remove last comma ',' and new line */
    /* if wcount==0, it will remove '[' and new line */
    fseek(file, -2, SEEK_CUR);
  }

  free(attr_names);
}

static void iLayoutExportElementC(FILE* file, Ihandle* ih, const char *indent, const char* terminator)
{
  char* name = IupGetName(ih);
  const char *elemClass = ih->iclass->name;

  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;

  fprintf(file, "%sIupSetAtt(%s%s%s, IupCreate(\"%s\"), \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", elemClass);

  iLayoutExportElementAttribs(file, ih, indent, VLED_EXPORT_C);

  fprintf(file, "%s  NULL)%s\n", indent, terminator);

  if (terminator[0] == ';')
    fprintf(file, "\n");
}

static void iLayoutExportElementLua(FILE* file, Ihandle* ih, const char *indent)
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

  iLayoutExportElementAttribs(file, ih, indent, VLED_EXPORT_LUA);

  fprintf(file, "%s}", indent);
}

static void iLayoutExportContainerC(FILE* file, Ihandle* ih, const char *indent, const char* terminator)
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
      iLayoutExportContainerC(file, child, localIndent, ",");  /* no ; */
    else
      iLayoutExportElementC(file, child, localIndent, ","); /* no ; */
  }

  fprintf(file, "%sNULL),\n", localIndent);  /* IupCreate */

  iLayoutExportElementAttribs(file, ih, indent, VLED_EXPORT_C);

  fprintf(file, "%s  NULL)%s\n", indent, terminator);  /* IupSetAtt */

  if (terminator[0] == ';')
    fprintf(file, "\n");
}

static void iLayoutExportContainerLua(FILE* file, Ihandle* ih, const char *indent)
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
      iLayoutExportContainerLua(file, child, localIndent);
      fprintf(file, ",\n");
    }
    else
    {
      iLayoutExportElementLua(file, child, localIndent);
      fprintf(file, ",\n");
    }
  }

  iLayoutExportElementAttribs(file, ih, indent, VLED_EXPORT_LUA);

  fprintf(file, "%s}", indent);
}

//TODO replace by iupChildTreeIsParent
static int iLayoutExportIsRelated(Ihandle *elem, Ihandle *container)
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

static int iLayoutExportCompareElem(const void* i1, const void* i2)
{
  Ihandle* ih1 = *((Ihandle**)i1);
  Ihandle* ih2 = *((Ihandle**)i2);
  int childType1 = ih1->iclass->childtype;
  int childType2 = ih2->iclass->childtype;

  if (childType1 == IUP_CHILDNONE && childType2 == IUP_CHILDNONE) /* both non containers */
  {
    const char *elemClass1 = ih1->iclass->name;
    const char *elemClass2 = ih2->iclass->name;
    if (iupStrEqualPartial(elemClass1, "image") && !iupStrEqualPartial(elemClass2, "image"))
      return -1;
    else if (iupStrEqualPartial(elemClass2, "image") && !iupStrEqualPartial(elemClass1, "image"))
      return 1;
    return strcmp(IupGetName(ih1), IupGetName(ih2)); /* sort elements by their names */
  }
  else if ((childType1 != IUP_CHILDNONE && childType2 != IUP_CHILDNONE)) /* both are containers */
  {
    if (iLayoutExportIsRelated(ih1, ih2))
      return -1;
    else if (iLayoutExportIsRelated(ih2, ih1))
      return 1;
    else
      return strcmp(IupGetName(ih1), IupGetName(ih2)); /* sort elements by their names */
  }
  else if (childType1 == IUP_CHILDNONE && childType2 != IUP_CHILDNONE)  /* container are always after non containers */
    return -1;
  else  /* childType1 != IUP_CHILDNONE && childType2 == IUP_CHILDNONE */
    return 1;
}

void vLedExport(const char* src_filename, const char* dst_filename, const char* format)
{
  char* title;
  char* *names;
  int num_names;
  Iarray* names_array;
  Ihandle* *data = NULL;
  int i, count = 0;
  FILE* file;
  int export_format = iupStrEqualNoCase(format, "LUA") ? VLED_EXPORT_LUA : (iupStrEqualNoCase(format, "LED") ? VLED_EXPORT_LED : VLED_EXPORT_C);

  names_array = iupArrayCreate(1024, sizeof(Ihandle*));  /* just set an initial size, but count is 0 */

  num_names = IupGetAllNames(NULL, -1);
  names = malloc(sizeof(char*)*num_names);
  IupGetAllNames(names, num_names);

  for (i = 0; i < num_names; i++)
  {
    Ihandle *elem = IupGetHandle(names[i]);
    if (elem)
    {
      if (iupStrEqualPartial(names[i], "_IUP_NAME") ||
          vLedIsAlien(elem, src_filename))
        continue;

      data = iupArrayAdd(names_array, 1);
      data[count] = elem;
      count++;
    }
  }

  if (count == 0)
  {
    iupArrayDestroy(names_array);
    free(names);
    return;
  }

  qsort(data, count, sizeof(Ihandle*), iLayoutExportCompareElem);

  file = fopen(dst_filename, "wb");
  if (!file)
  {
    iupArrayDestroy(names_array);
    free(names);
    return;
  }

  title = iupStrFileGetTitle(dst_filename);

  if (export_format == VLED_EXPORT_LUA)
  {
    iLayoutExportRemoveExt(title, "lua");

    fprintf(file, "--   Generated by IupVisualLED export to Lua.\n\n");

    fprintf(file, "function create_%s()\n\n", title);
  }
  else if (export_format == VLED_EXPORT_C)
  {
    iLayoutExportRemoveExt(title, "c");

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
          iLayoutExportElementLua(file, elem, "  ");
        else if (export_format == VLED_EXPORT_C)
          iLayoutExportElementC(file, elem, "  ", ";");
      }
      else
      {
        if (export_format == VLED_EXPORT_LUA)
          iLayoutExportContainerLua(file, elem, "  ");
        else if (export_format == VLED_EXPORT_C)
          iLayoutExportContainerC(file, elem, "  ", ";");
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
  free(names);
}

