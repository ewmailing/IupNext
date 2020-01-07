#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "iup.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_image.h"
#include "iup_func.h"
#include "iup_layout.h"
#include "iup_childtree.h"
#include "iup_array.h"



static int iLayoutExportHasReserved(const char* name, int check_num)
{
  char c;

  if (*name == 0)  /* empty string must be in quotes */
    return 1;

  c = *name;
  if (check_num && c >= '1' && c <= '9') /* first character can NOT be a number */
    return 1;
  
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

  if (export_format == IUP_LAYOUT_EXPORT_LUA)  /* Lua */
  {
    iupStrLower(attribname, name);
    if (iLayoutExportHasReserved(attribname, 1))
      fprintf(file, "%s[\"%s\"] = \"%s\",\n", indent, attribname, value);
    else
      fprintf(file, "%s%s = \"%s\",\n", indent, attribname, value);
  }
  else if (export_format == IUP_LAYOUT_EXPORT_LED) /* LED */
  {
    if (iLayoutExportHasReserved(name, 0))  /* can not be saved in LED */
    {
      if (old_value != value)
        free((char*)value);
      return;
    }

    iupStrUpper(attribname, name);
    if (iLayoutExportHasReserved(value, 0))
      fprintf(file, "%s=\"%s\", ", attribname, value);
    else
      fprintf(file, "%s=%s, ", attribname, value);
  }
  else   /* C */
    fprintf(file, "%s\"%s\", \"%s\",\n", indent, name, value);

  if (old_value != value)
    free((char*)value);
}

static int iLayoutCheckExportedInConstructor(Ihandle* ih, const char* check_name)
{
  const char *format = ih->iclass->format;
  if (!format || format[0] == 0)
    return 0;
  else
  {
    int i, num_format = (int)strlen(format);

    for (i = 0; i < num_format; i++)
    {
      if (format[i] == 's' || format[i] == 'a')
      {
        const char* name = NULL;
        if (i == 0)
          name = ih->iclass->format_attr;
        if (!name)
        {
          if (format[i] == 'a')
            name = "ACTION";
          else
            name = "TITLE";
        }

        if (name && iupStrEqual(name, check_name))
          return 1;
      }
    }

    return 0;
  }
}

static void iLayoutExportSavedAttribs(FILE* file, Ihandle* ih, const char* indent, int export_format)
{
  int i, attr_count;
  char **attr_names;
  char localIndent[1024];
  int wcount = 0;

  strcpy(localIndent, indent);
  strcat(localIndent, "  ");

  attr_count = iupAttribGetAllSaved(ih, NULL, 0);
  attr_names = (char **)malloc(attr_count * sizeof(char *));

  attr_count = iupAttribGetAllSaved(ih, attr_names, attr_count);
  for (i = 0; i < attr_count; i++)
  {
    if (export_format == IUP_LAYOUT_EXPORT_LED && iLayoutCheckExportedInConstructor(ih, attr_names[i]))
      continue;

    if (export_format != IUP_LAYOUT_EXPORT_LUA || 
        !iupClassObjectAttribIsCallback(ih, attr_names[i])) /* do NOT save callbacks in Lua */
    {
      char* value = iupAttribGetLocal(ih, attr_names[i]);  /* do NOT check for inherited values */
      if (value)
      {
        iLayoutExportWriteAttrib(file, attr_names[i], value, localIndent, export_format);
        wcount++;
      }
    }
  }

  if (export_format == IUP_LAYOUT_EXPORT_LED) /* LED */
  {
    if (wcount != 0)
      fseek(file, -2, SEEK_CUR);  /* remove last comma ',' and space */
    else
      fseek(file, -1, SEEK_CUR);/* remove "[" */

    if (wcount != 0)
      fprintf(file, "]"); /* end of attributes (no new line) */
  }

  free(attr_names);
}

static void iLayoutExportChangedAttribs(FILE* file, Ihandle* ih, const char* indent, int export_format)
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
    char* value = iupAttribGetLocal(ih, name);  /* do NOT check for inherited values */
    char* def_value;
    int flags;

    iupClassGetAttribNameInfo(ih->iclass, name, &def_value, &flags);

    if (value && iupLayoutAttributeHasChanged(ih, name, value, def_value, flags))
    {
      char* str = iupStrConvertToC(value);

      iLayoutExportWriteAttrib(file, name, str, indent, export_format);
      wcount++;

      if (str != value)
        free(str);
    }

    if (has_attrib_id && flags&IUPAF_HAS_ID)
    {
      flags &= ~IUPAF_HAS_ID; /* clear flag so the next function call can work (it will check for read-only, write-only, no-string and no-save) */
      if (iupLayoutAttributeHasChanged(ih, name, "XXX", NULL, flags)) /* use a non null and non empty value just to pass the other tests */
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
                iLayoutExportWriteAttrib(file, str, value, indent, export_format);
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
              iLayoutExportWriteAttrib(file, str, value, indent, export_format);
              wcount++;
            }
          }
        }
      }
    }
  }

  if (export_format != IUP_LAYOUT_EXPORT_LUA)  /* LED or C - additional step for callbacks */
  {
    int cb_count = total_count - attr_count;
    IupGetClassCallbacks(ih->iclass->name, attr_names, cb_count);
    for (i = 0; i < cb_count; i++)
    {
      char* cb_name = iupGetCallbackName(ih, attr_names[i]);
      if (cb_name && cb_name[0] && !iupATTRIB_ISINTERNAL(cb_name))
      {
        iLayoutExportWriteAttrib(file, attr_names[i], cb_name, indent, export_format);
        wcount++;
      }
    }
  }

  if (export_format == IUP_LAYOUT_EXPORT_LED ) /* LED */
  {
    if (wcount != 0)
      fseek(file, -2, SEEK_CUR);  /* remove last comma ',' and space */
    else
      fseek(file, -1, SEEK_CUR);/* remove "[" */

    if (wcount != 0)
      fprintf(file, "]"); /* end of attributes (no new line) */
  }

  free(attr_names);
}

static char* iLayoutGetName(Ihandle* ih)
{
  char* name = IupGetName(ih);
  if (name && iupATTRIB_ISINTERNAL(name))
    name = NULL;
  return name;
}

static void iLayoutExportElementLED(FILE* file, Ihandle* ih, const char* indent, int saved_info)
{
  int i, count;
  const char* format = ih->iclass->format;
  char classname[100];

  char* name = iLayoutGetName(ih);

  iupStrUpper(classname, ih->iclass->name);
  if (name)
    fprintf(file, "%s = %s[", name, classname);  /* start of attributes */
  else
    fprintf(file, "%s%s[", indent, classname);

  if (saved_info)
    iLayoutExportSavedAttribs(file, ih, indent, IUP_LAYOUT_EXPORT_LED);
  else
    iLayoutExportChangedAttribs(file, ih, indent, IUP_LAYOUT_EXPORT_LED);

  if (!format || format[0] == 0)
    fprintf(file, "()");
  else
  {
    if (*format == 'g')  /* a multi child container (can only has 1 format) */
    {
      Ihandle *child;
      char localIndent[1024];

      strcpy(localIndent, indent);
      strcat(localIndent, "  ");

      fprintf(file, "(\n");

      for (child = ih->firstchild; child; child = child->brother)
      {
        if (!(child->flags & IUP_INTERNAL))
        {
          char* childname = iLayoutGetName(child);
          if (!childname)
            iLayoutExportElementLED(file, child, localIndent, saved_info);   /* here process the ones that does NOT have names */
          else
            fprintf(file, "%s%s", localIndent, childname);

          if (child->brother)
            fprintf(file, ",\n");
        }
      }

      fprintf(file, "\n%s)", indent);
    }
    else
    {
      count = (int)strlen(format);

      fprintf(file, "(");

      for (i = 0; i < count; i++)
      {
        if (format[i] == 's')
        {
          char* value;
          const char* attribname = NULL;
          if (i == 0)
            attribname = ih->iclass->format_attr;
          if (!attribname)
            attribname = "TITLE";

          value = iupAttribGetLocal(ih, attribname);  /* do NOT check for inherited values */
          if (value)
            fprintf(file, "\"%s\"", value); /* always with quotes in constructor */
          else
            fprintf(file, "\"\"");  /* empty string */
        }
        else if (format[i] == 'a')
        {
          char* cb_name = NULL;
          if (i == 0 && ih->iclass->format_attr)
            cb_name = iupGetCallbackName(ih, ih->iclass->format_attr);
          if (!cb_name)
            cb_name = iupGetCallbackName(ih, "ACTION");

          if (cb_name && cb_name[0] && !iupATTRIB_ISINTERNAL(cb_name))
            fprintf(file, "%s", cb_name);
          else
            fprintf(file, "do_nothing");  /* dummy name */
        }
        else if (format[i] == 'h')
        {
          Ihandle *child;
          for (child = ih->firstchild; child; child = child->brother)
          {
            if (!(child->flags & IUP_INTERNAL))
            {
              char* childname = iLayoutGetName(child);
              if (!childname)
              {
                char localIndent[1024];

                strcpy(localIndent, indent);
                strcat(localIndent, "  ");

                fprintf(file, "\n");
                iLayoutExportElementLED(file, child, localIndent, saved_info);   /* here process the ones that does NOT have names */
              }
              else
                fprintf(file, "%s", childname);

              break; /* only one child */
            }
          }
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

static void iLayoutExportElementLua(FILE* file, Ihandle* ih, const char *indent, int saved_info)
{
  Ihandle *child;
  char* name = iLayoutGetName(ih);

  if (name)
    fprintf(file, "%slocal %s = ", indent, name);
  else
    fprintf(file, "%s", indent);

  fprintf(file, "iup.%s{\n", ih->iclass->name);

  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    char localIndent[1024];

    strcpy(localIndent, indent);
    strcat(localIndent, "  ");

    for (child = ih->firstchild; child; child = child->brother)
    {
      char* childName = iLayoutGetName(child);
      if (childName)
      {
        if (iupAttribGet(child, "_IUP_EXPORT_LUA_SAVED")) /* saved in the same scope */
          fprintf(file, "%s%s,\n", localIndent, childName);
        else
          fprintf(file, "%siup.GetHandle(\"%s\"),\n", localIndent, childName);
      }
      else
      {
        iLayoutExportElementLua(file, child, localIndent, saved_info);
        fprintf(file, ",\n");
      }
    }
  }

  if (saved_info)
    iLayoutExportSavedAttribs(file, ih, indent, IUP_LAYOUT_EXPORT_LUA);
  else
    iLayoutExportChangedAttribs(file, ih, indent, IUP_LAYOUT_EXPORT_LUA);

  fprintf(file, "%s}", indent);
}

static void iLayoutExportElementC(FILE* file, Ihandle* ih, const char *indent, const char* terminator, int saved_info)
{
  char* name = iLayoutGetName(ih);

  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    Ihandle *child;
    char localIndent[1024];

    fprintf(file, "%sIupSetAtt(%s%s%s, IupCreatep(\"%s\", \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", ih->iclass->name);

    strcpy(localIndent, indent);
    strcat(localIndent, "    ");  /* indent twice for children */

    for (child = ih->firstchild; child; child = child->brother)
    {
      char* childName = iLayoutGetName(child);
      if (childName)
        fprintf(file, "%sIupGetHandle(\"%s\"),\n", localIndent, childName);
      else 
        iLayoutExportElementC(file, child, localIndent, ",", saved_info); /* no ; */
    }

    fprintf(file, "%sNULL),\n", localIndent);  /* IupCreatep */
  }
  else
    fprintf(file, "%sIupSetAtt(%s%s%s, IupCreate(\"%s\"), \n", indent, name ? "\"" : "", name ? name : "NULL", name ? "\"" : "", ih->iclass->name);

  if (saved_info)
    iLayoutExportSavedAttribs(file, ih, indent, IUP_LAYOUT_EXPORT_C);
  else
    iLayoutExportChangedAttribs(file, ih, indent, IUP_LAYOUT_EXPORT_C);

  fprintf(file, "%s  NULL)%s\n", indent, terminator);  /* IupSetAtt */

  if (terminator[0] == ';')
    fprintf(file, "\n");
}

IUP_SDK_API void iupLayoutExportNamedElemList(FILE* file, Ihandle* *named_elem, int count, int export_format, int saved_info)
{
  int i;
  char* name;
  Ihandle *elem;

  for (i = 0; i < count; i++)
  {
    elem = named_elem[i];
    name = IupGetName(elem);

    if (iupStrEqualPartial(elem->iclass->name, "image"))
    {
      char* format[] = { "LUA", "C", "LED" };
      iupImageExportToFile(elem, file, format[export_format], name, 0);  /* don't put it inside a function (Lua and C) */
    }
    else
    {
      if (export_format == IUP_LAYOUT_EXPORT_LUA)
      {
        iLayoutExportElementLua(file, elem, "  ", saved_info);
        fprintf(file, "\n");
      }
      else if (export_format == IUP_LAYOUT_EXPORT_C)
        iLayoutExportElementC(file, elem, "  ", ";", saved_info);
      else
        iLayoutExportElementLED(file, elem, "  ", saved_info);
    }

    if (export_format == IUP_LAYOUT_EXPORT_LUA && name)
    {
      fprintf(file, "  iup.SetHandle(\"%s\", %s)\n\n", name, name);
      iupAttribSet(elem, "_IUP_EXPORT_LUA_SAVED", "1");
    }
  }

  if (export_format == IUP_LAYOUT_EXPORT_LUA)
  {
    for (i = 0; i < count; i++)
    {
      elem = named_elem[i];
      name = IupGetName(elem);
      if (name)
        iupAttribSet(elem, "_IUP_EXPORT_LUA_SAVED", NULL);
    }
  }
}

/******************************************************************************/


static int iImagePrintBuffer(Iarray *buffer, const char *format, va_list arglist)
{
  char str[100];
  int count, len;
  char *data;

  count = iupArrayCount(buffer);

  len = vsprintf(str, format, arglist);

  if (len < 0)
    return len;

  iupArrayAdd(buffer, len);

  data = (char*)iupArrayGetData(buffer);

  strcat(data + count, str);

  return len;
}

static int iImagePrint(FILE *file, Iarray *buffer, char *format, ...)
{
  int len;
  va_list arglist;
  va_start(arglist, format);

  if (file)
    len = vfprintf(file, format, arglist);
  else
    len = iImagePrintBuffer(buffer, format, arglist);

  va_end(arglist);

  return len;
}

static int SaveImageC(const char* filename, Ihandle* ih, const char* name, FILE* packfile, Iarray* buffer, int inFunction)
{
  int y, x, width, height, channels, linesize;
  unsigned char* data;
  FILE* file = NULL;

  data = (unsigned char*)iupAttribGet(ih, "WID");
  if (!data)
    return 0;

  if (packfile)
    file = packfile;
  else if (filename)
    file = fopen(filename, "wb");

  if (!file && !buffer)
    return 0;

  width = IupGetInt(ih, "WIDTH");
  height = IupGetInt(ih, "HEIGHT");
  channels = IupGetInt(ih, "CHANNELS");
  linesize = width * channels;

  if (inFunction)
  {
    if (iImagePrint(file, buffer, "static Ihandle* load_image_%s(void)\n", name) < 0)
    {
      if (filename)
        fclose(file);
      return 0;
    }
    iImagePrint(file, buffer, "{\n");
  }
  else
    iImagePrint(file, buffer, "  {\n"); /* to isolate the declarations */

  if (IupGetInt(NULL, "IMAGEEXPORT_STATIC"))
    iImagePrint(file, buffer, "  static unsigned char imgdata[] = {\n");
  else
    iImagePrint(file, buffer, "  unsigned char imgdata[] = {\n");

  for (y = 0; y < height; y++)
  {
    iImagePrint(file, buffer, "    ");

    for (x = 0; x < linesize; x++)
    {
      if (x != 0)
        iImagePrint(file, buffer, ", ");

      iImagePrint(file, buffer, "%d", (int)data[y*linesize + x]);
    }

    if (y == height - 1)
      iImagePrint(file, buffer, "};\n\n");
    else
      iImagePrint(file, buffer, ",\n");
  }

  if (channels == 1)
  {
    int c;
    char* color;

    iImagePrint(file, buffer, "  Ihandle* image = IupImage(%d, %d, imgdata);\n\n", width, height);

    for (c = 0; c < 256; c++)
    {
      color = IupGetAttributeId(ih, "", c);
      if (!color)
        break;

      iImagePrint(file, buffer, "  IupSetAttribute(image, \"%d\", \"%s\");\n", c, color);
    }

    if (inFunction)
      iImagePrint(file, buffer, "\n");
  }
  else if (channels == 3)
    iImagePrint(file, buffer, "  Ihandle* image = IupImageRGB(%d, %d, imgdata);\n", width, height);
  else /* channels == 4 */
    iImagePrint(file, buffer, "  Ihandle* image = IupImageRGBA(%d, %d, imgdata);\n", width, height);

  if (inFunction)
    iImagePrint(file, buffer, "  return image;\n");
  else
    iImagePrint(file, buffer, "  IupSetHandle(\"%s\", image);\n", name, name);

  if (inFunction)
    iImagePrint(file, buffer, "}\n\n");
  else
    iImagePrint(file, buffer, "  }\n\n");

  if (filename)
    fclose(file);

  return 1;
}

static int SaveImageLua(const char* filename, Ihandle* ih, const char* name, FILE* packfile, Iarray* buffer, int inFunction)
{
  int y, x, width, height, channels, linesize;
  unsigned char* data;
  FILE* file = NULL;

  data = (unsigned char*)iupAttribGet(ih, "WID");
  if (!data)
    return 0;

  if (packfile)
    file = packfile;
  else if (filename)
    file = fopen(filename, "wb");

  if (!file && !buffer)
    return 0;

  width = IupGetInt(ih, "WIDTH");
  height = IupGetInt(ih, "HEIGHT");
  channels = IupGetInt(ih, "CHANNELS");
  linesize = width * channels;

  if (inFunction)
  {
    if (fprintf(file, "function load_image_%s()\n", name) < 0)
    {
      if (!packfile)
        fclose(file);
      return 0;
    }
  }

  if (channels == 1)
    iImagePrint(file, buffer, "  local %s = iup.image{\n", name);
  else if (channels == 3)
    iImagePrint(file, buffer, "  local %s = iup.imagergb{\n", name);
  else /* channels == 4 */
    iImagePrint(file, buffer, "  local %s = iup.imagergba{\n", name);

  iImagePrint(file, buffer, "    width = %d,\n", width);
  iImagePrint(file, buffer, "    height = %d,\n", height);
  iImagePrint(file, buffer, "    pixels = {\n");

  for (y = 0; y < height; y++)
  {
    iImagePrint(file, buffer, "      ");
    for (x = 0; x < linesize; x++)
    {
      iImagePrint(file, buffer, "%d, ", (int)data[y*linesize + x]);
    }
    iImagePrint(file, buffer, "\n");
  }

  iImagePrint(file, buffer, "    },\n");

  if (channels == 1)
  {
    int c;
    char* color;
    unsigned char r, g, b;

    iImagePrint(file, buffer, "    colors = {\n");

    for (c = 0; c < 256; c++)
    {
      color = IupGetAttributeId(ih, "", c);
      if (!color)
        break;

      /* don't use index, this Lua constructor assumes index=0 the first item in the array */
      if (iupStrEqualNoCase(color, "BGCOLOR"))
        iImagePrint(file, buffer, "      \"BGCOLOR\",\n"); 
      else
      {
        iupStrToRGB(color, &r, &g, &b);
        iImagePrint(file, buffer, "      \"%d %d %d\",\n", (int)r, (int)g, (int)b);
      }
    }

    iImagePrint(file, buffer, "    }\n");
  }

  iImagePrint(file, buffer, "  }\n");

  if (inFunction)
  {
    iImagePrint(file, buffer, "  return %s\n", name);
    iImagePrint(file, buffer, "end\n\n");
  }

  if (filename)
    fclose(file);

  return 1;
}

static int SaveImageLED(const char* filename, Ihandle* ih, const char* name, FILE* packfile, Iarray* buffer)
{
  int y, x, width, height, channels, linesize;
  unsigned char* data;
  FILE* file = NULL;

  data = (unsigned char*)iupAttribGet(ih, "WID");
  if (!data)
    return 0;

  if (packfile)
    file = packfile;
  else if (filename)
    file = fopen(filename, "wb");

  if (!file && !buffer)
    return 0;

  width = IupGetInt(ih, "WIDTH");
  height = IupGetInt(ih, "HEIGHT");
  channels = IupGetInt(ih, "CHANNELS");
  linesize = width * channels;

  if (channels == 1)
  {
    int c;
    unsigned char r, g, b;
    char* color;

    if (iImagePrint(file, buffer, "%s = IMAGE[\n", name) < 0)
    {
      if (filename)
        fclose(file);
      return 0;
    }

    for (c = 0; c < 256; c++)
    {
      color = IupGetAttributeId(ih, "", c);
      if (!color)
      {
        if (c < 16)
          continue;
        else
          break;
      }

      if (c != 0)
        iImagePrint(file, buffer, ",\n");

      if (iupStrEqualNoCase(color, "BGCOLOR"))
        iImagePrint(file, buffer, "  %d = \"BGCOLOR\"", c);
      else
      {
        iupStrToRGB(color, &r, &g, &b);
        iImagePrint(file, buffer, "  %d = \"%d %d %d\"", c, (int)r, (int)g, (int)b);
      }
    }
    iImagePrint(file, buffer, "]\n  ");
  }
  else if (channels == 3)
  {
    if (iImagePrint(file, buffer, "%s = IMAGERGB", name) < 0)
    {
      if (filename)
        fclose(file);
      return 0;
    }
  }
  else /* channels == 4 */
  {
    if (iImagePrint(file, buffer, "%s = IMAGERGBA", name) < 0)
    {
      if (filename)
        fclose(file);
      return 0;
    }
  }

  iImagePrint(file, buffer, "(%d, %d,\n", width, height);

  for (y = 0; y < height; y++)
  {
    iImagePrint(file, buffer, "  ");
    for (x = 0; x < linesize; x++)
    {
      if (y == height - 1 && x == linesize - 1)
        iImagePrint(file, buffer, "%d", (int)data[y*linesize + x]);
      else
        iImagePrint(file, buffer, "%d, ", (int)data[y*linesize + x]);
    }
    iImagePrint(file, buffer, "\n");
  }

  iImagePrint(file, buffer, ")\n\n");

  if (filename)
    fclose(file);

  return 1;
}

IUP_API int IupSaveImageAsText(Ihandle* ih, const char* filename, const char* format, const char* name)
{
  int ret = 0;

  if (!name)
  {
    name = IupGetName(ih);
    if (!name)
      name = "image";
  }

  if (iupStrEqualNoCase(format, "LED"))
    ret = SaveImageLED(filename, ih, name, NULL, NULL);
  else if (iupStrEqualNoCase(format, "LUA"))
    ret = SaveImageLua(filename, ih, name, NULL, NULL, 1);
  else if (iupStrEqualNoCase(format, "C"))
    ret = SaveImageC(filename, ih, name, NULL, NULL, 1);
  return ret;
}

IUP_SDK_API int iupImageExportToFile(Ihandle* ih, FILE* packfile, const char* format, const char* name, int inFunction)
{
  int ret = 0;

  if (!name)
  {
    name = IupGetName(ih);
    if (!name)
      name = "image";
  }

  if (iupStrEqualNoCase(format, "LED"))
    ret = SaveImageLED(NULL, ih, name, packfile, NULL);
  else if (iupStrEqualNoCase(format, "LUA"))
    ret = SaveImageLua(NULL, ih, name, packfile, NULL, inFunction);
  else if (iupStrEqualNoCase(format, "C"))
    ret = SaveImageC(NULL, ih, name, packfile, NULL, inFunction);
  return ret;
}

IUP_SDK_API int iupImageExportToString(Ihandle* ih, char **str, const char* format, const char* name, int inFunction)
{
  Iarray *buffer = iupArrayCreate(1024, sizeof(char *));
  int ret = 0;

  if (!name)
  {
    name = IupGetName(ih);
    if (!name)
      name = "image";
  }

  if (iupStrEqualNoCase(format, "LED"))
    ret = SaveImageLED(NULL, ih, name, NULL, buffer);
  else if (iupStrEqualNoCase(format, "LUA"))
    ret = SaveImageLua(NULL, ih, name, NULL, buffer, inFunction);
  else if (iupStrEqualNoCase(format, "C"))
    ret = SaveImageC(NULL, ih, name, NULL, buffer, inFunction);

  *str = iupArrayReleaseData(buffer);

  iupArrayDestroy(buffer);

  return ret;
}
