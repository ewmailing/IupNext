
#include "simple_paint_util.h"

#include <string.h>
#include <ctype.h>

#include <iup.h>
#include <cd.h>


char* str_duplicate(const char* str)
{
  if (!str)
    return NULL;

  int size = (int)strlen(str);
  char* new_str = new char[size+1];
  memcpy(new_str, str, size + 1);
  return new_str;
}

const char* str_filetitle(const char *filename)
{
  /* Start at the last character */
  int len = (int)strlen(filename);
  int offset = len - 1;
  while (offset != 0)
  {
    if (filename[offset] == '\\' || filename[offset] == '/')
    {
      offset++;
      break;
    }
    offset--;
  }
  return filename + offset;
}

const char* str_fileext(const char *filename)
{
  /* Start at the last character */
  int len = (int)strlen(filename);
  int offset = len - 1;
  while (offset != 0)
  {
    if (filename[offset] == '\\' || filename[offset] == '/')
      break;

    if (filename[offset] == '.')
    {
      offset++;
      return filename + offset;
    }
    offset--;
  }
  return NULL;
}

int str_compare(const char *l, const char *r, int casesensitive)
{
  if (!l || !r)
    return 0;

  while (*l && *r)
  {
    int diff;
    char l_char = *l,
      r_char = *r;

    /* compute the difference of both characters */
    if (casesensitive)
      diff = l_char - r_char;
    else
      diff = tolower((int)l_char) - tolower((int)r_char);

    /* if they differ we have a result */
    if (diff != 0)
      return 0;

    /* otherwise process the next characters */
    ++l;
    ++r;
  }

  /* check also for terminator */
  if (*l == *r)
    return 1;

  if (*r == 0)
    return 1;  /* if second string is at terminator, then it is partially equal */

  return 0;
}


/********************************************************************/


void show_error(const char* message, int is_error)
{
  Ihandle* dlg = IupMessageDlg();
  IupSetStrAttribute(dlg, "PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dlg, "DIALOGTYPE", is_error ? "ERROR" : "WARNING");
  IupSetAttribute(dlg, "BUTTONS", "OK");
  IupSetStrAttribute(dlg, "TITLE", is_error ? "Error" : "Warning");
  IupSetStrAttribute(dlg, "VALUE", message);
  IupPopup(dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
  IupDestroy(dlg);
}

void show_file_error(int error)
{
  switch (error)
  {
  case IM_ERR_OPEN:
    show_error("Error Opening File.",  1);
    break;
  case IM_ERR_MEM:
    show_error("Insufficient memory.",  1);
    break;
  case IM_ERR_ACCESS:
    show_error("Error Accessing File.",  1);
    break;
  case IM_ERR_DATA:
    show_error("Image type not Supported.",  1);
    break;
  case IM_ERR_FORMAT:
    show_error("Invalid Format.",  1);
    break;
  case IM_ERR_COMPRESS:
    show_error("Invalid or unsupported compression.",  1);
    break;
  default:
    show_error("Unknown Error.",  1);
  }
}


/***********************************************************/


struct xyStack
{
  int x, y;
  xyStack* next;
};

xyStack* xy_stack_push(xyStack* q, int x, int y)
{
  xyStack* new_q = new xyStack;
  new_q->x = x;
  new_q->y = y;
  new_q->next = q;
  return new_q;
}

xyStack* xy_stack_pop(xyStack* q)
{
  xyStack* next_q = q->next;
  delete q;
  return next_q;
}

int color_is_similar(long color1, long color2, int tol)
{
  int diff_r = cdRed(color1) - cdRed(color2);
  int diff_g = cdGreen(color1) - cdGreen(color2);
  int diff_b = cdBlue(color1) - cdBlue(color2);
  int sqr_dist = diff_r*diff_r + diff_g*diff_g + diff_b*diff_b;
  /* max value = 255*255*3 = 195075 */
  /* sqrt(195075)=441 */
  if (sqr_dist < tol)
    return 1;
  else
    return 0;
}

void image_flood_fill(const imImage* image, int start_x, int start_y, long replace_color, double tol_percent)
{
  unsigned char** data = (unsigned char**)image->data;
  unsigned char *r = data[0], *g = data[1], *b = data[2];
  int offset, tol, cur_x, cur_y;
  long target_color, color;
  xyStack* q = NULL;

  offset = start_y * image->width + start_x;
  target_color = cdEncodeColor(r[offset], g[offset], b[offset]);

  if (target_color == replace_color)
    return;

  tol = (int)(441 * tol_percent) / 100;
  tol = tol*tol;  /* this is too high */
  tol = tol / 50;  /* empirical reduce. TODO: What is the best formula? */

  /* very simple 4 neighbors stack based flood fill */

  /* a color in the xy_stack is always similar to the target color,
  and it was already replaced */
  q = xy_stack_push(q, start_x, start_y);
  cdDecodeColor(replace_color, r + offset, g + offset, b + offset);

  while (q)
  {
    cur_x = q->x;
    cur_y = q->y;
    q = xy_stack_pop(q);

    /* right */
    if (cur_x < image->width - 1)
    {
      offset = cur_y * image->width + cur_x + 1;
      color = cdEncodeColor(r[offset], g[offset], b[offset]);
      if (color != replace_color && color_is_similar(color, target_color, tol))
      {
        q = xy_stack_push(q, cur_x + 1, cur_y);
        cdDecodeColor(replace_color, r + offset, g + offset, b + offset);
      }
    }

    /* left */
    if (cur_x > 0)
    {
      offset = cur_y * image->width + cur_x - 1;
      color = cdEncodeColor(r[offset], g[offset], b[offset]);
      if (color != replace_color && color_is_similar(color, target_color, tol))
      {
        q = xy_stack_push(q, cur_x - 1, cur_y);
        cdDecodeColor(replace_color, r + offset, g + offset, b + offset);
      }
    }

    /* top */
    if (cur_y < image->height - 1)
    {
      offset = (cur_y + 1) * image->width + cur_x;
      color = cdEncodeColor(r[offset], g[offset], b[offset]);
      if (color != replace_color && color_is_similar(color, target_color, tol))
      {
        q = xy_stack_push(q, cur_x, cur_y + 1);
        cdDecodeColor(replace_color, r + offset, g + offset, b + offset);
      }
    }

    /* bottom */
    if (cur_y > 0)
    {
      offset = (cur_y - 1) * image->width + cur_x;
      color = cdEncodeColor(r[offset], g[offset], b[offset]);
      if (color != replace_color && color_is_similar(color, target_color, tol))
      {
        q = xy_stack_push(q, cur_x, cur_y - 1);
        cdDecodeColor(replace_color, r + offset, g + offset, b + offset);
      }
    }
  }
}

