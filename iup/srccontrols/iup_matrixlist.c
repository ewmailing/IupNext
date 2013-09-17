/** \file
 * \brief IupMatrixList
 *
 * See Copyright Notice in "iup.h"
 *
 * Based on MTXLIB, developed at Tecgraf/PUC-Rio
 * by Renata Trautmann and Andre Derraik
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "cd.h"
#include "iupcbs.h"
#include "iupcontrols.h"

#include "iup_attrib.h"
#include "iup_object.h"
#include "iup_str.h"
#include "iup_drv.h"
//#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
//#include "iup_image.h"
#include "iup_register.h"

#include "iup_controls.h"
#include "iup_cdutil.h"

/* Use IupMatrix internal definitions to speed up access */
#include "matrix/iupmat_def.h"
#include "matrix/iupmat_getset.h"
#include "matrix/iupmat_edit.h"
//#include "matrix/iupmat_draw.h"
//#include "matrix/iupmat_mem.h"
#include "matrix/iupmat_numlc.h"


#define IMTXL_COLOR_WIDTH 16


typedef struct _ImatrixListData  /* Used only by the IupMatrixList control */
{
  /* attributes */
  int editable;     /* allow adding new lines by editing the last line */
//  int selectedLine;    /* Number of the selected line */
//  int lastSelLine;

  /* internal variables */
  int label_col, color_col, image_col;  /* column order (0 means it is hidden) */
  int showing_delete;  /* showing the delete button, after a double click in an editable list */

  Ihandle *def_image_unmark, *def_image_mark, *def_image_del, *def_image_del_unsel;
  Ihandle *def_image_add;
} ImatrixListData;


/******************************************************************************
    Utilities
******************************************************************************/

//static void iMatrixListPutIconMap(Ihandle* ih, void *img, int x, int y, int lin)
//{
//  //long int mtxColors[2];
//  //unsigned char *imageMap;
// 
//  //if(index == IMG_MARK || index == IMG_UNMARK || index == IMG_ADD)
//  //{
//  //  mtxColors[0] = cdEncodeColor(10, 10, 10);
//  //  mtxColors[1] = cdEncodeColor(255, 255, 255);
//  //  imageMap = (index == IMG_MARK ? img_mark : index == IMG_UNMARK ? img_unmark : img_add);
//  //}
//  //else if(index == IMG_BLOCK_MARK || index == IMG_BLOCK_UNMARK)
//  //{
//  //  mtxColors[0] = cdEncodeColor(180, 180, 180);
//  //  mtxColors[1] = cdEncodeColor(255, 255, 255);
//  //  imageMap = (index == IMG_BLOCK_MARK ? img_mark : img_unmark);
//  //}
//  //else if(index == IMG_DEL)
//  //{
//  //  mtxColors[0] = cdEncodeColor(255, 235, 155);
//  //  mtxColors[1] = cdEncodeColor(255, 0, 0);
//  //  imageMap = img_del;
//  //}
//  //else if(index == IMG_DEL_UNSEL)
//  //{
//  //  mtxColors[0] = cdEncodeColor(255, 255, 255);
//  //  mtxColors[1] = cdEncodeColor(255, 0, 0);
//  //  imageMap = img_del;
//  //}
//  //else if(index == IMG_BLOCK_DEL)
//  //{
//  //  mtxColors[0] = (mtxList->lastSelLine != lin ? cdEncodeColor(255, 255, 255) : cdEncodeColor(255, 235, 155));
//  //  mtxColors[1] = cdEncodeColor(180, 180, 180);
//  //  imageMap = img_del;
//  //}
//
//  //cdCanvasPutImageRectMap(cnv, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, imageMap,
//  //                        mtxColors, x, y, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, 0, 0, 0, 0);
//}
//
//static void iMatrixListPutIconRGBA(Ihandle* ih, void *img, int x, int y)
//{
//  //cdCanvasPutImageRectRGBA(cnv, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT,
//  //                         mtxList->imageRGBA[index]->data[0],  /* não tenho certeza aqui... */
//  //                         mtxList->imageRGBA[index]->data[1],
//  //                         mtxList->imageRGBA[index]->data[2],
//  //                         mtxList->imageRGBA[index]->data[3],
//  //                         x, y, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, 0, 0, 0, 0);
//}

static void iMatrixListInitializeImages(void)
{
  Ihandle *image_unmark, *image_mark, *image_del, *image_del_unsel;  
  Ihandle *image_add;

#define IMTXL_IMG_WIDTH  16
#define IMTXL_IMG_HEIGHT 16

  unsigned char img_mark[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  unsigned char img_unmark[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  unsigned char img_del[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1,
    1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1,
    1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1,
    0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0
  };

  unsigned char img_add[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  image_unmark       = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_unmark);
  image_mark         = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_mark);
  image_del          = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_del);
  image_del_unsel    = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_del);
  image_add          = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_add);

  IupSetAttribute(image_unmark, "0", "10 10 10");
  IupSetAttribute(image_unmark, "1", "BGCOLOR");

  IupSetAttribute(image_mark, "0", "10 10 10");
  IupSetAttribute(image_mark, "1", "BGCOLOR");

  IupSetAttribute(image_del, "0", "BGCOLOR");
  IupSetAttribute(image_del, "1", "255 0 0");

  IupSetAttribute(image_del_unsel, "0", "BGCOLOR");
  IupSetAttribute(image_del_unsel, "1", "255 0 0");

  IupSetAttribute(image_add, "0", "10 10 10");
  IupSetAttribute(image_add, "1", "BGCOLOR");

  IupSetHandle("MTXLIST_IMG_UNMARK", image_unmark);
  IupSetHandle("MTXLIST_IMG_MARK", image_mark);
  IupSetHandle("MTXLIST_IMG_DEL", image_del);
  IupSetHandle("MTXLIST_IMG_DEL_UNSEL", image_del_unsel);
  IupSetHandle("MTXLIST_IMG_ADD", image_add);

#undef IMTXL_IMG_WIDTH
#undef IMTXL_IMG_HEIGHT
}

static void iMatrixListCopyLinAttributes(Ihandle* ih, int lin1, int lin2)
{
  char* value;

  /* Update the line attributes */
  value = iupAttribGetId(ih, "IMAGE", lin1);
  iupAttribSetStrId(ih, "IMAGE", lin2, value);

  value = iupAttribGetId(ih, "COLOR", lin1);
  iupAttribSetStrId(ih, "COLOR", lin2, value);
}

static void iMatrixListClearLinAttributes(Ihandle* ih, int lin)
{
  iupAttribSetId(ih, "IMAGE", lin, NULL);
  iupAttribSetId(ih, "COLOR", lin, NULL);
}

static void iMatrixListUpdateLineAttributes(Ihandle* ih, int base, int count, int add)
{
  int lin;

  if (add)  /* ADD */
  {
    /* copy the attributes of the moved cells, from base+count to num */
    /*   do it in reverse order to avoid overlapping */
    /* then clear the new space starting from base to base+count */

    for(lin = ih->data->lines.num-1; lin >= base+count; lin--)
      iMatrixListCopyLinAttributes(ih, lin-count, lin);

    for(lin = base; lin < base+count; lin++)
      iMatrixListClearLinAttributes(ih, lin);
  }
  else  /* DEL */
  {
    /* copy the attributes of the moved cells from base+count to base */
    /* then clear the remaining space starting at num */

    for(lin = base; lin < ih->data->lines.num; lin++)
      iMatrixListCopyLinAttributes(ih, lin+count, lin);

    for(lin = ih->data->lines.num; lin < ih->data->lines.num+count; lin++)
      iMatrixListClearLinAttributes(ih, lin);
  }
}

static void iMatrixListAddLineAttributes(Ihandle* ih, int base, int count)
{
  int lin;
  for(lin = base; lin < base+count; lin++)
  {
    /* all bottom horizontal lines transparent, except title and the last one */
    if (lin!=0 && lin!=ih->data->lines.num-1)
      IupSetAttributeId2(ih, "FRAMEHORIZCOLOR", lin, IUP_INVALID_ID, "BGCOLOR");
  }
}

static void iMatrixListUpdateLastLineAttributes(Ihandle* ih, int lines_num)
{
  if (lines_num < ih->data->lines.num && lines_num > 1)
    IupSetAttributeId2(ih, "FRAMEHORIZCOLOR", lines_num-1, IUP_INVALID_ID, "BGCOLOR");
  IupSetAttributeId2(ih, "FRAMEHORIZCOLOR", ih->data->lines.num-1, IUP_INVALID_ID, NULL);
}

static void iMatrixListInitSize(Ihandle* ih, ImatrixListData* mtxList)
{
  char str[30];
  int num_col = 0;

  if (mtxList->label_col != 0)
    num_col++;
  if (mtxList->color_col != 0)
    num_col++;
  if (mtxList->image_col != 0)
    num_col++;
  
  sprintf(str, "%d", num_col);
  iupMatrixSetNumColAttrib(ih, str);  /* "NUMCOL" */
  IupSetStrAttribute(ih, "NUMCOL_VISIBLE", str);

  if (mtxList->color_col != 0)
  {
    if (!iupAttribGetId(ih, "WIDTH", mtxList->color_col))
      IupSetIntId(ih, "WIDTH", mtxList->color_col, IMTXL_COLOR_WIDTH);
  }

  if (mtxList->image_col != 0)
  {
    if (!iupAttribGetId(ih, "WIDTH", mtxList->image_col))
    {
      int w = IupGetInt(mtxList->def_image_unmark, "WIDTH");
      IupSetIntId(ih, "WIDTH", mtxList->image_col, w < 16? 16: w);  /* minimum image column size */
    }
  }
}

static void iMatrixListInitializeAttributes(Ihandle* ih, ImatrixListData* mtxList)
{
  int num_lin, col;

  for(col = 1; col < ih->data->columns.num; col++)
  {
    /* all right vertical lines transparent, except the last one */
    if (col != ih->data->columns.num-1)
      IupSetAttributeId2(ih, "FRAMEVERTCOLOR", IUP_INVALID_ID, col, "BGCOLOR");
  }

  num_lin = ih->data->lines.num-1;  /* remove the title line count, even if not visible */
  if (num_lin > 1 && mtxList->editable)
    IupSetInt(ih, "NUMLIN", num_lin+1);  /* reserve space for the empty line */

  /* Set the text alignment for the item column */
  IupSetAttributeId(ih, "ALIGNMENT", mtxList->label_col, "ALEFT");

  /* Set toggle mtxList->editable or not */
//  if (!mtxList->image_col != 0)
//    mtxList->editModeToggle = 0;
//  else
//    mtxList->editModeToggle = 1;
}

static int iMatrixListPostRedraw(Ihandle* ih)
{
  ih->data->need_redraw = 1;
  IupUpdate(ih);
  return 1;  /* store value in hash table */
}

static char* iMatrixApplyInactiveColor(const char* color)
{
  static char inactive_color[20];
  unsigned char r=0, g=0, b=0;

  iupStrToRGB(color, &r, &g, &b);

  r = cdIupLIGTHER(r);
  g = cdIupLIGTHER(g);
  b = cdIupLIGTHER(b);

  return inactive_color;
}

static void iMatrixListUpdateLineActiveColors(Ihandle* ih, ImatrixListData* mtxList, int lin, int active)
{
  char *bgcolor, *fgcolor;

  if (active)
  {
    bgcolor = iupAttribGetId2(ih, "_IUPMTXLIST_OLDBGCOLOR", lin, mtxList->label_col);
    IupSetStrAttributeId2(ih, "BGCOLOR", lin, mtxList->label_col, bgcolor);
    IupSetStrAttributeId2(ih, "BGCOLOR", lin, mtxList->image_col, bgcolor);
    IupSetStrAttributeId2(ih, "BGCOLOR", lin, mtxList->color_col, bgcolor);

    fgcolor = iupAttribGetId2(ih, "_IUPMTXLIST_OLDFGCOLOR", lin, mtxList->label_col);
    IupSetStrAttributeId2(ih, "FGCOLOR", lin, mtxList->label_col, fgcolor);
  }
  else
  {
    /* save original color, check only at the hash table */
    bgcolor = iupAttribGetId2(ih, "BGCOLOR", lin, mtxList->label_col);
    iupAttribSetStrId2(ih, "_IUPMTXLIST_OLDBGCOLOR", lin, mtxList->label_col, bgcolor);

    /* get the actual color */
    bgcolor = IupGetAttributeId2(ih, "CELLBGCOLOR", lin, mtxList->label_col);
    bgcolor = iMatrixApplyInactiveColor(bgcolor);
    IupSetStrAttributeId2(ih, "BGCOLOR", lin, mtxList->label_col, bgcolor);
    IupSetStrAttributeId2(ih, "BGCOLOR", lin, mtxList->image_col, bgcolor);
    IupSetStrAttributeId2(ih, "BGCOLOR", lin, mtxList->color_col, bgcolor);

    fgcolor = iupAttribGetId2(ih, "FGCOLOR", lin, mtxList->label_col);
    iupAttribSetStrId2(ih, "_IUPMTXLIST_OLDFGCOLOR", lin, mtxList->label_col, fgcolor);

    fgcolor = IupGetAttributeId2(ih, "CELLFGCOLOR", lin, mtxList->label_col);
    fgcolor = iMatrixApplyInactiveColor(fgcolor);
    IupSetStrAttributeId2(ih, "FGCOLOR", lin, mtxList->label_col, fgcolor);
  }
}


/******************************************************************************
 Attributes
******************************************************************************/


static char* iMatrixListGetEditableAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  return iupStrReturnBoolean(mtxList->editable);
}

static int iMatrixListSetEditableAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  mtxList->editable = iupStrBoolean(value);
  return 0;
}

static int iMatrixListSetAddLinAttrib(Ihandle* ih, const char* value)
{
  int base, count, lines_num = ih->data->lines.num;

  if (!ih->handle)  /* do not do the action before map */
    return 0;       /* allowing this method to be called before map will avoid its storage in the hash table */

  if (!iupMatrixGetStartEnd(value, &base, &count, lines_num, 0))
    return 0;

  if (base < lines_num)  /* If before the last line. */
    iMatrixListUpdateLineAttributes(ih, base, count, 1);

  iMatrixListAddLineAttributes(ih, base, count);

  iupMatrixSetAddLinAttrib(ih, value);

  iMatrixListUpdateLastLineAttributes(ih, lines_num);

  return 0;
}

static int iMatrixListSetDelLinAttrib(Ihandle* ih, const char* value)
{
  int base, count, lines_num = ih->data->lines.num;

  if (!ih->handle)  /* do not do the action before map */
    return 0;       /* allowing this method to be called before map will avoid its storage in the hash table */

  if (!iupMatrixGetStartEnd(value, &base, &count, lines_num, 0))
    return 0;

  if (base < lines_num)  /* If before the last line. (always true when deleting) */
    iMatrixListUpdateLineAttributes(ih, base, count, 0);

  iupMatrixSetDelLinAttrib(ih, value);

  iMatrixListUpdateLastLineAttributes(ih, lines_num);

  return 0;
}

static int iMatrixListSetNumLinAttrib(Ihandle* ih, const char* value)
{
  int lines_num = ih->data->lines.num;
  iupMatrixSetNumLinAttrib(ih, value);

  if (lines_num < ih->data->lines.num)  /* lines were added */
    iMatrixListAddLineAttributes(ih, lines_num, ih->data->lines.num-lines_num);

  iMatrixListUpdateLastLineAttributes(ih, lines_num);

  return 0;
}

static char* iMatrixListGetColumnOrderAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  char* str = iupStrGetMemory(30);

  if (mtxList->label_col == 1)
    strcat(str, "LABEL");
  else if (mtxList->color_col == 1)
    strcat(str, "COLOR");
  else if (mtxList->image_col == 1)
    strcat(str, "IMAGE");

  if (mtxList->label_col == 2)
    strcat(str, ":LABEL");
  else if (mtxList->color_col == 2)
    strcat(str, ":COLOR");
  else if (mtxList->image_col == 2)
    strcat(str, ":IMAGE");
  else
    return str;

  if (mtxList->label_col == 3)
    strcat(str, ":LABEL");
  else if (mtxList->color_col == 3)
    strcat(str, ":COLOR");
  else if (mtxList->image_col == 3)
    strcat(str, ":IMAGE");

  return str;
}

static int iMatrixListSetColumnOrderAttrib(Ihandle *ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  char value1[30]="", value2[30]="", value3[30]="";
  int ret;

  /* valid only before map */
  if (ih->handle)
    return 0;

  ret = iupStrToStrStr(value, value1, value2, ':');
  if (ret == 0)
    return 0;

  if (iupStrEqualNoCase(value1, "IMAGE"))
  {
    mtxList->image_col = 1;
    mtxList->label_col = 0;
    mtxList->color_col = 0;
  }
  else if (iupStrEqualNoCase(value1, "COLOR"))
  {
    mtxList->color_col = 1;
    mtxList->label_col = 0;
    mtxList->image_col = 0;
  }
  else if (iupStrEqualNoCase(value1, "LABEL"))
  {
    mtxList->label_col = 1;
    mtxList->color_col = 0;
    mtxList->image_col = 0;
  }
  else
    return 0; /* must have a first column */

  if (ret==1)
  {
    if (!ih->handle)
      iMatrixListInitSize(ih, mtxList);
    return 0;
  }
  
  ret = iupStrToStrStr(value2, value2, value3, ':');
  if (ret == 0)
    return 0;

  if (iupStrEqualNoCase(value2, "IMAGE"))
  {
    if (mtxList->image_col == 0)   /* don't allow repeated columns */
      mtxList->image_col = 2;
  }
  else if (iupStrEqualNoCase(value2, "COLOR"))
  {
    if (mtxList->color_col == 0)
      mtxList->color_col = 2;
  }
  else if (iupStrEqualNoCase(value2, "LABEL"))
  {
    if (mtxList->label_col == 0)
      mtxList->label_col = 2;
  }

  if (ret==1)
  {
    if (!ih->handle)
      iMatrixListInitSize(ih, mtxList);
    return 0;
  }

  if (mtxList->image_col != 2 &&
      mtxList->color_col != 2 &&
      mtxList->label_col != 2)
    return 0;  /* must have the second to allow the third */

  if (iupStrEqualNoCase(value3, "IMAGE"))
  {
    if (mtxList->image_col == 0)   /* don't allow repeated columns */
      mtxList->image_col = 3;
  }
  else if (iupStrEqualNoCase(value3, "COLOR"))
  {
    if (mtxList->color_col == 0)
      mtxList->color_col = 3;
  }
  else if (iupStrEqualNoCase(value3, "LABEL"))
  {
    if (mtxList->label_col == 0)
      mtxList->label_col = 3;
  }

  if (!ih->handle)
    iMatrixListInitSize(ih, mtxList);
  return 0;
}

static char* iMatrixListGetImageColAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  return iupStrReturnInt(mtxList->image_col);
}

static char* iMatrixListGetColorColAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  return iupStrReturnInt(mtxList->color_col);
}

static char* iMatrixListGetLabelColAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  return iupStrReturnInt(mtxList->label_col);
}

static int iMatrixListSetImageUnmarkAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  mtxList->def_image_unmark = IupGetHandle(value);
  IupSetAttribute(ih, "REDRAW", "ALL");
  return 0;
}

static int iMatrixListSetImageMarkAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  mtxList->def_image_mark = IupGetHandle(value);
  IupSetAttribute(ih, "REDRAW", "ALL");
  return 0;
}

static int iMatrixListSetImageDelAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  mtxList->def_image_del = IupGetHandle(value);
  IupSetAttribute(ih, "REDRAW", "ALL");
  return 0;
}

static int iMatrixListSetImageDelUnselAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  mtxList->def_image_del_unsel = IupGetHandle(value);
  IupSetAttribute(ih, "REDRAW", "ALL");
  return 0;
}

static int iMatrixListSetImageAddAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  mtxList->def_image_add = IupGetHandle(value);
  IupSetAttribute(ih, "REDRAW", "ALL");
  return 0;
}

static char* iMatrixListGetImageActiveAttrib(Ihandle* ih, int lin)
{
  char* value = iupAttribGetId(ih, "IMAGEACTIVE", lin);
  if (!value)
    return "Yes"; /* default is Yes for all lines */
  else
    return value;
}

static int iMatrixListSetImageActiveAttrib(Ihandle* ih, int lin, const char* value)
{
  (void)lin;
  (void)value;
  return iMatrixListPostRedraw(ih);
}

static char* iMatrixListGetLineActiveAttrib(Ihandle* ih, int lin)
{
  char* value = iupAttribGetId(ih, "LINEACTIVE", lin);
  if (!value)
    return "Yes"; /* default is Yes for all lines */
  else
    return value;
}

static int iMatrixListSetLineActiveAttrib(Ihandle* ih, int lin, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  
  if (mtxList->editable && mtxList->showing_delete)
    IupStoreAttribute(ih, "ACTIVE", value);
  else
    iMatrixListUpdateLineActiveColors(ih, mtxList, lin, iupStrBoolean(value));

  return iMatrixListPostRedraw(ih);
}

static int iMatrixListSetTitleAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  if (!ih->handle)
    iupAttribSetId2(ih, "", 0, mtxList->label_col, value);
  else
    iupMatrixSetValue(ih, 0, mtxList->label_col, value, 0);
  return 0;
}

static char* iMatrixListGetTitleAttrib(Ihandle* ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  if (!ih->handle)
    return iupAttribGetId2(ih, "", 0, mtxList->label_col);
  else
    return iupMatrixGetValueString(ih, 0, mtxList->label_col);
}

static int iMatrixListSetIdValueAttrib(Ihandle* ih, int lin, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  if (iupMatrixCheckCellPos(ih, lin, mtxList->label_col))
    iupMatrixSetValue(ih, lin, mtxList->label_col, value, 0);
  return 0;
}

static char* iMatrixListGetIdValueAttrib(Ihandle* ih, int lin)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  if (iupMatrixCheckCellPos(ih, lin, mtxList->label_col))
    return iupMatrixGetValueString(ih, lin, mtxList->label_col);
  return NULL;
}

static int iMatrixListSetValueAttrib(Ihandle* ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* Is the focus cell a item column cell ? */
  if(ih->data->columns.focus_cell != mtxList->label_col)
    return 0;

  if (ih->data->columns.num <= 1 || ih->data->lines.num <= 1)
    return 0;

  if (IupGetInt(ih->data->datah, "VISIBLE"))
    IupStoreAttribute(ih->data->datah, "VALUE", value);
  else 
    iupMatrixSetValue(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell, value, 0);
  return 0;
}

static char* iMatrixListGetValueAttrib(Ihandle* ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* Is the focus cell a item column cell ? */
  if(ih->data->columns.focus_cell != mtxList->label_col)
    return NULL;

  if (ih->data->columns.num <= 1 || ih->data->lines.num <= 1)
    return NULL;

  if (IupGetInt(ih->data->datah, "VISIBLE"))
    return iupMatrixEditGetValue(ih);
  else 
    return iupMatrixGetValueString(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell);
}

static int iMatrixListSetAddColAttrib(Ihandle* ih, const char* value)
{
  (void)ih;
  (void)value;
  /* does nothing */
  return 0;
}

static int iMatrixListSetDelColAttrib(Ihandle* ih, const char* value)
{
  (void)ih;
  (void)value;
  /* does nothing */
  return 0;
}

static char* iMatrixListGetNumLinAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->lines.num-1);  /* the attribute does not include the title */
}

static char* iMatrixListGetNumColAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->columns.num-1);  /* the attribute does not include the title */
}

static int iMatrixListSetNumColAttrib(Ihandle* ih, const char* value)
{
  (void)ih;
  (void)value;
  /* does nothing */
  return 0;
}

static int iMatrixListSetNumColNoScrollAttrib(Ihandle* ih, const char* value)
{
  (void)ih;
  (void)value;
  /* does nothing */
  return 0;
}
static char* iMatrixListGetNumColNoScrollAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->columns.num_noscroll-1);  /* the attribute does not include the title */
}

static char* iMatrixListGetNumColVisibleAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->columns.last - ih->data->columns.first);
}


/******************************************************************************
         Callbacks
******************************************************************************/

static int iMatrixListDrawColorCol(Ihandle *ih, int lin, int col, int x1, int x2, int y1, int y2, cdCanvas *cnv)
{
  IFniiiiiiC cb = (IFniiiiiiC)IupGetCallback(ih, "DRAWCOLORCOL_CB");
      
  /* If draw callback is defined, delegate the draw action. */ 
  if(cb)
    return cb(ih, lin, col, x1, x2, y1, y2, cnv);
  else
  {    
    unsigned char red, green, blue;
    char* color = iupAttribGetId(ih, "COLOR", lin);

    if (iupStrToRGB(color, &red, &green, &blue))
    {
      static const int DX_BORDER = 2;
      static const int DY_BORDER = 3;
      static const int DX_FILL = 3;
      static const int DY_FILL = 4;
      int active = iupdrvIsActive(ih);
      long framecolor;

      if (!active)
      {
        red = cdIupLIGTHER(red);
        green = cdIupLIGTHER(green);
        blue = cdIupLIGTHER(blue);
      }

      /* Fill the box with the color */
      cdCanvasForeground(cnv, cdEncodeColor(red, green, blue));
      cdCanvasBox(cnv, x1 + DX_FILL, x2 - DX_FILL, y1 - DY_FILL, y2 + DY_FILL);

      /* Draw the border */
      framecolor = cdIupConvertColor(iupAttribGetStr(ih, "FRAMECOLOR"));
      cdCanvasForeground(cnv, framecolor);

      cdCanvasRect(cnv, x1 + DX_BORDER, x2 - DX_BORDER, y1 - DY_BORDER, y2 + DY_BORDER);
    }
  }

  return IUP_IGNORE;  /* draw nothing */
}

static int iMatrixListDrawImageCol(Ihandle *ih, ImatrixListData* mtxList, int lin, int col, int x1, int x2, int y1, int y2, cdCanvas *cnv)
{
#if 0
  /* Find the image point */
  int x = x2 - x1 - image_width;
  int y = y1 - y2 - 1 - image_height;
  x /= 2; x += x1;
  y /= 2; y += y2;

  image = ih->data->cells[lin][mtxList->image_col].image;
  line_active = !(ih->data->lines.dt[lin].flags & IMAT_IS_LINE_INACTIVE);
  button_active = !(ih->data->lines.dt[lin].flags & IMAT_IS_BUTTON_INACTIVE);

  /* Get the index image to display */
  /* Select the correct image icon to display */
  if (!mtxList->editable && (!line_active || !button_active))
  {
    /* Put the icon */
    if(checked)
      iMatrixListDrawImage(ih, image_block_mark, x, y);
    else
      iMatrixListDrawImage(ih, image_block_unmark, x, y, lin);
  }
  else if(mtxList->showing_delete && !line_active)
  {
    iMatrixListDrawImage(ih, image_block_del, x, y);
  }
  else if(mtxList->showing_delete && line_active)
  {
    if (!unsel)
      iMatrixListDrawImage(ih, image_del, x, y);
    else
      iMatrixListDrawImage(ih, image_del_unsel, x, y, lin);
  }
  else
  {
    iMatrixListDrawImage(ih, image, x, y, lin);
  }

  if(lin == num_lin && IupGetHandle("IMG_ADD") != NULL && mtxList->editable)
  {
    int x = x2 - x1 - IMTXL_IMG_WIDTH;
    int y = y1 - y2 - 1 - IMTXL_IMG_HEIGHT;
    x /= 2; x += x1;
    y /= 2; y += y2;

    iMatrixListPutIconRGBA(ih, image_add, x, y);
  }
#endif

  return IUP_IGNORE;  /* draw nothing */
}

static int iMatrixListDraw_CB(Ihandle *ih, int lin, int col, int x1, int x2, int y1, int y2, cdCanvas *cnv)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  int num_lin = ih->data->lines.num-1;
//  int mtxList->editable = iupAttribGetInt(ih, "EDIT_MODE_NAME");

  /* Just checking */
  if (lin <= 0 || col <= 0 || !cnv)
    return IUP_IGNORE;  /* draw regular text */

  /* Don't draw on the empty line. */
  if ((lin < num_lin && mtxList->editable) || (lin <= num_lin && !mtxList->editable))
  {
    if (mtxList->label_col && col == mtxList->label_col)
      return IUP_IGNORE;  /* draw regular text */

    if (mtxList->color_col && col == mtxList->color_col)
      return iMatrixListDrawColorCol(ih, lin, col, x1, x2, y1, y2, cnv);

    if (mtxList->image_col && col == mtxList->image_col)
      return iMatrixListDrawImageCol(ih, mtxList, lin, col, x1, x2, y1, y2, cnv);
  }

  return IUP_IGNORE;  /* draw nothing */
}

//static int iMatrixListEdition_CB(Ihandle *ih, int lin, int col, int mode, int update)
//{
//  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
//  int status   = IUP_DEFAULT;
//  int num_lin = ih->data->lines.num-1;
//  int mtxList->editable = iupAttribGetInt(ih, "EDIT_MODE_NAME");
//  IFniii editCB = (IFniii)IupGetCallback(ih, "EDIT_CB");
//
//  if(col != mtxList->label_col)
//    return IUP_IGNORE;
//
//  if(editCB != NULL && (ih->data->lines.dt[lin].flags & IMAT_IS_LINE_INACTIVE))
//    status = editCB(ih, lin, col, mode);
//
//  if(!mtxList->editable)
//  {
//    IFnis dbClickCB = (IFnis)IupGetCallback(ih, "DBLCLICK_CB");
//
//    if(dbClickCB != NULL)
//      status = dbClickCB(ih, lin, IupGetAttributeId(ih, "", lin));
//    else
//      status = IUP_IGNORE;
//  }
//  else
//  {
//    /* Update last action */
//    IupSetInt(ih, "ACTION_TYPE", 1);  /* Edit action */
//
//    /* Editing the item column ? */
//    if(col == mtxList->label_col)
//    {
//      if(mode == 1)
//      {
//        mtxList->lastSelLine = lin;
//      }
//      else if(mode == 0)
//      {
//        IFni insertCB = (IFni)IupGetCallback(ih, "INSERT_CB");
//
//        if(update == 0)
//          return IUP_IGNORE;
//
//        if(num_lin == lin)
//        {
//          int i;
//          if(insertCB != NULL)
//          {
//            char buffer[30];
//            for(i = 1; i <= lin; i++)
//            {
//              sprintf(buffer, "LINEACTIVE%d", i);
//              IupSetAttribute(ih, buffer, "OFF");
//            }
//            
//            status = insertCB(ih, lin);
//            
//            for(i = 1; i <= lin; i++)
//            {
//              sprintf(buffer, "LINEACTIVE%d", i);
//              IupSetAttribute(ih, buffer, "ON");
//            }
//
//            if(status == IUP_IGNORE)
//            {
//              IupSetAttributeId2(ih, "", lin, mtxList->label_col, "");
//              
//              /* Force a matrix redisplay */
//              IupSetAttribute(ih, "REDRAW", "ALL");
//              
//              return IUP_CONTINUE;
//            }
//            else
//            {
//              IFnii actionCB = (IFnii)IupGetCallback(ih, "ACTION_CB");
//
//              /* Add the line */
//              IupSetInt(ih, "ADDLIN", lin);
//              
//              /* Set it active */
//              IupSetAttributeId2(ih, "", lin+1, mtxList->label_col, "");
//              sprintf(buffer, "LINEACTIVE%d", lin+1);
//              IupSetAttribute(ih, buffer, "ON");
//
//              /* Check if we need to call a function */
//              if(actionCB != NULL)
//                actionCB(ih, mtxList->lastSelLine, mtxList->label_col);
//            }
//          }
//        }
//      }
//    }
//    else
//    {
//      /* Don't allow to edit this cell */
//      status = IUP_IGNORE;
//    }
//
//    /* Force a matrix redisplay */
//    IupSetAttribute(ih, "REDRAW", "ALL");
//  }
//
//  /* Notify if the application is or is not in edit mode */
//  if(status != IUP_IGNORE)
//  {
//    IFni notifyEditionCB = (IFni)IupGetCallback(ih, "NOTIFYEDITION_CB");
//
//    if(notifyEditionCB != NULL)
//      status = notifyEditionCB(ih, mode);
//  }
//
//  /* Return the action */
//  return status;
//}
//
//static int iMatrixListClick_CB(Ihandle *ih, int lin, int col, char *status)
//{
//  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
//  static int lock = 0;
//  IFnis clickCB = (IFnis)IupGetCallback(ih, "CLICK_CB");
//  int ret = IUP_DEFAULT;
//  char buffer[30];
//  (void)col;
//
//  /* The line is locked ? */
//  if(lock || lin <= 0)
//    return IUP_IGNORE;
//
//  /* Lock line */
//  lock = 1;
//
//  /* Select the clicked line */
//  sprintf(buffer, "MARK%d:0", lin);
//  IupSetAttribute(ih, buffer, "YES");
//
//  /* Set the cell focus */
//  sprintf(buffer, "%d:%d", lin, mtxList->label_col);
//  IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));
//
//  /* Set the last selected line */
//  mtxList->lastSelLine = lin;
//
//  /* Force a matrix redisplay */
//  IupSetAttribute(ih, "REDRAW", "ALL");
//
//  /* Check if we need to call a function */
//  if(clickCB != NULL && !(ih->data->lines.dt[lin].flags & IMAT_IS_LINE_INACTIVE))
//    ret = clickCB(ih, lin, status);
//
//  /* Release line */
//  lock = 0;
//
//  return ret;
//}
//
//static int iMatrixListLeave_CB(Ihandle *ih, int lin, int col)
//{
//  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
//  int ret = IUP_DEFAULT;
//  int num_lin = ih->data->lines.num-1;
//  int mtxList->editable = iupAttribGetInt(ih, "EDIT_MODE_NAME");
//  int lastAction = IupGetInt(ih, "ACTION_TYPE");
//  char buffer[30];
//
//  /* Return if not mtxList->editable */
//  if(mtxList->editable)
//    return IUP_IGNORE;
//
//  if(lin == num_lin && lastAction)  /* last action = edition */
//  {
//    /* Get the cell value */
//    char *data = IupGetAttributeId2(ih, "", lin, col);
//    if(data != NULL && data[0] != '\0')
//    {
//      /* Add new line */
//      IupSetInt(ih, "ADDLIN", lin);
//
//      /* Activate the new line */
//      sprintf(buffer, "LINEACTIVE%d", lin+1);
//      IupSetAttribute(ih, buffer, "ON");
//
//      /* Show delete button ? */
//      if(mtxList->showing_delete)
//      {
//        sprintf(buffer, "%d:%d", lin, mtxList->label_col);
//        IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));
//      }
//
//      /* Reset values the line created */
//      IupStoreAttributeId2(ih, "", lin+1, mtxList->label_col, "");
//
//      /* Update the last select line */
//      mtxList->lastSelLine = lin;
//
//      /* Force a matrix redisplay */
//      IupSetAttribute(ih, "REDRAW", "ALL");
//    }
//  }
//
//  /* Update last matrix state */
//  IupSetInt(ih, "ACTION_TYPE", 0);  /* Action none */
//
//  return ret;
//}
//
//static int iMatrixListEditLabel(Ihandle *ih, ImatrixListData* mtxList, int c, int lin, int col, int active, char* after)
//{
//  int line = lin;
//  char buffer[30];
//  int num_lin = IupGetInt(ih, IUP_NUMLIN);
//  IFnii actionCB = (IFnii)IupGetCallback(ih, "ACTION_CB");
//  int ret = IUP_DEFAULT;
//  (void)active;
//
//  if(!IupGetInt(ih, IUP_ACTIVE))  /* return = NO */
//    return IUP_IGNORE;
//
//  if(((c == K_DOWN) || (c == K_UP)) && mtxList->showing_delete)
//  {
//    int linSel = 0;
//
//    if(c == K_DOWN)
//      linSel = (lin == num_lin ?  num_lin : lin + 1);
//    else if(c == K_UP)
//      linSel = (lin == 1 ? 1 : lin - 1);
//  }
//
//  if(col == mtxList->label_col || col == mtxList->image_col)
//  {
//    char *data = after;
//
//    /* Is the item column ? */
//    if(col != mtxList->label_col)
//    {
//      /* Set the cell focus */
//      sprintf(buffer, "%d:%d", lin, mtxList->label_col);
//      IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));
//
//      /* Get the cell value */
//      data = IupGetAttributeId2(ih, "", lin, mtxList->label_col);
//    }
//
//    if(((c == K_CR) &&  (iupStrEqualNoCase(IupGetAttribute(ih, "EDITNEXT"), "NONE"))) || c == K_UP || c == K_DOWN)
//    {
//      if(c == K_UP && lin != 1)
//      {
//        line = lin - 1;
//
//        /* Check if we need to call a function */
//        if(actionCB != NULL)
//          ret = actionCB(ih, line, mtxList->label_col);
//
//        mtxList->lastSelLine = line;
//      }
//      else if((c == K_DOWN && lin != num_lin) || 
//               ((c == K_CR &&  (iupStrEqualNoCase(IupGetAttribute(ih, "EDITNEXT"), "NONE"))) && data != NULL && data[0] != '\0'))
//      {
//        line = lin + 1;
//
//        /* Check if we need to call a function */
//        if(actionCB != NULL)
//          ret = actionCB(ih, line, mtxList->label_col);
//
//        /* Save the selected line */
//        mtxList->lastSelLine = line;
//      }
//    }
//  }
//
//  /* Return the status */
//  return ret;
//}
//
//static int iMatrixListSelectLine(Ihandle *ih, ImatrixListData* mtxList, int c, int lin, int col, int active, char* after)
//{
//  int i;
//  int line = lin;
//  int ret = IUP_DEFAULT;
//  int num_lin = ih->data->lines.num-1;
//  IFnii actionCB = (IFnii)IupGetCallback(ih, "ACTION_CB");
//  char buffer[30];
//  (void)after;
//  (void)active;
//
//  if(mtxList->label_col != col)
//    return IUP_DEFAULT;
//
//  /* Process the key */
//  if(c == K_UP)
//  {
//    line = mtxList->lastSelLine - 1;
//
//    for(i = line; i >= 1; i--)
//    {
//      /* Unset the current selected line */
//      sprintf(buffer, "MARK%d:0", mtxList->lastSelLine);
//      IupSetAttribute(ih, buffer, "NO");
//
//      /* Set the new selected line */
//      sprintf(buffer, "MARK%d:0", i);
//      IupSetAttribute(ih, buffer, "YES");
//
//      /* Set the new focus */
//      sprintf(buffer, "%d:%d", i, mtxList->label_col);
//      IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));
//
//      /* Save the last selected line */
//      mtxList->lastSelLine = i;
//
//      if(ih->data->lines.dt[i].flags & IMAT_IS_LINE_INACTIVE)
//      {
//        /* Check if we need to call a function */
//        if(actionCB != NULL)
//          ret = actionCB(ih, i, mtxList->label_col);
//
//        /* Stop the loop */
//        break;
//      }
//    }
//  }
//  else if(c == K_DOWN)
//  {
//    line = mtxList->lastSelLine + 1;
//
//    for(i = line; i <= num_lin; i++)
//    {
//      /* Unset the current selected line */
//      sprintf(buffer, "MARK%d:0", mtxList->lastSelLine);
//      IupSetAttribute(ih, buffer, "NO");
//
//      /* Set the new selected line */
//      sprintf(buffer, "MARK%d:0", i);
//      IupSetAttribute(ih, buffer, "YES");
//
//      /* Set the new focus */
//      sprintf(buffer, "%d:%d", i, mtxList->label_col);
//      IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));
//
//      /* Save the last selected line */
//      mtxList->lastSelLine = i;
//
//      if(ih->data->lines.dt[i].flags & IMAT_IS_LINE_INACTIVE)
//      {
//        /* Check if we need to call a function */
//        if(actionCB != NULL)
//          ret = actionCB(ih, i, mtxList->label_col);
//
//        /* Stop the loop */
//        break;
//      }
//    }
//  }
//  else if(c == K_CR)
//  {
//    IFnis dbClickCB = (IFnis)IupGetCallback(ih, "DBLCLICK_CB");
//
//    /* The line is active ? */
//    if(ih->data->lines.dt[lin].flags & IMAT_IS_LINE_INACTIVE)
//    {
//      /* Check if we need to call a function */
//      if(dbClickCB != NULL)
//        ret = dbClickCB(ih, lin, IupGetAttributeId(ih, "", lin));
//      else
//        ret = IUP_IGNORE;
//    }
//  }
//
//  /* Return the status */
//  return ret;
//}
//
//static int iMatrixListAction_CB(Ihandle *ih, int c, int lin, int col, int active, char* after)
//{
//  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
//  int ret = IUP_DEFAULT;
//  int mtxList->editable = iupAttribGetInt(ih, "EDIT_MODE_NAME");
//
//  /* The matrix is being edited */
//  if(mtxList->editable)
//    ret = iMatrixListEditLabel(ih, mtxList, c, lin, col, active, after);
//  else
//    ret = iMatrixListSelectLine(ih, mtxList, c, lin, col, active, after);
//
//  /* Force a matrix redisplay */
//  IupSetAttribute(ih, "REDRAW", "ALL");
//
//  return ret;
//}


/******************************************************************************
 Methods
******************************************************************************/


static int iMatrixListMapMethod(Ihandle* ih)
{
  /* defining default attributes */
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  iMatrixListInitializeAttributes(ih, mtxList);
  return IUP_NOERROR;
}

static int iMatrixListCreateMethod(Ihandle* ih, void **params)
{
  ImatrixListData* mtxList = (ImatrixListData*)calloc(1, sizeof(ImatrixListData));
  iupAttribSet(ih, "_IUP_MATRIXLIST_DATA", (char*)mtxList);

  /* default matrix list values */
  mtxList->label_col = 1;

  /* change the IupCanvas default values */
  iupAttribSet(ih, "EXPAND", "NO");      /* Disable the expand option */

  /* Change the IupMatrix default values */
  iupAttribSet(ih, "HIDEFOCUS", "YES");  /* Hide the matrix focus feedback */
  iupAttribSet(ih, "SCROLLBAR", "VERTICAL");
  iupAttribSet(ih, "CURSOR", "ARROW");
  iupAttribSet(ih, "ALIGNMENTLIN0", "ALEFT");
  iupAttribSet(ih, "FRAMETITLEHIGHLIGHT", "No");

  /* iMatrix callbacks */
  IupSetCallback(ih, "DRAW_CB",  (Icallback)iMatrixListDraw_CB);
//  IupSetCallback(ih, "CLICK_CB", (Icallback)iMatrixListClick_CB);
//  //IupSetCallback(ih, "EDITION_CB",   (Icallback)iMatrixListEdition_CB);
//  //IupSetCallback(ih, "LEAVEITEM_CB", (Icallback)iMatrixListLeave_CB);
//  IupSetCallback(ih, "ACTION_CB",    (Icallback)iMatrixListAction_CB);

  (void)params;
  return IUP_NOERROR;
}

Iclass* iupMatrixListNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("matrix"));
  
  ic->name = "matrixlist";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id  = 2;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->New    = iupMatrixListNewClass;
  ic->Create = iMatrixListCreateMethod;
  ic->Map    = iMatrixListMapMethod;

  /* IupMatrixList Callbacks */
//  iupClassRegisterCallback(ic, "DBLCLICK_CB", "is");
//  iupClassRegisterCallback(ic, "INSERT_CB", "i");
//  iupClassRegisterCallback(ic, "NOTIFYEDITION_CB", "i");
//  iupClassRegisterCallback(ic, "EDIT_CB", "iii");
  iupClassRegisterCallback(ic, "DRAWCOLORCOL_CB", "iiiiiiv");

  iupClassRegisterReplaceAttribDef(ic, "CURSOR", IUPAF_SAMEASSYSTEM, "ARROW");

  /* IupMatrixList Attributes */

  /* IMPORTANT: this two will hide the IupMatrix VALUE and L:C attributes */
  iupClassRegisterAttributeId(ic, "IDVALUE", iMatrixListGetIdValueAttrib, iMatrixListSetIdValueAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iMatrixListGetValueAttrib, iMatrixListSetValueAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", iMatrixListGetTitleAttrib, iMatrixListSetTitleAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "COLUMNORDER", iMatrixListGetColumnOrderAttrib, iMatrixListSetColumnOrderAttrib, IUPAF_SAMEASSYSTEM, "LABEL", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COLORCOL", iMatrixListGetColorColAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGECOL", iMatrixListGetImageColAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LABELCOL", iMatrixListGetLabelColAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "IMAGE", NULL, NULL, IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR", NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "IMAGEACTIVE", iMatrixListGetImageActiveAttrib, iMatrixListSetImageActiveAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "LINEACTIVE",   iMatrixListGetLineActiveAttrib, iMatrixListSetLineActiveAttrib, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGEUNMARK", NULL, iMatrixListSetImageUnmarkAttrib, IUPAF_SAMEASSYSTEM, "IMG_UNMARK", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEMARK",   NULL, iMatrixListSetImageMarkAttrib,   IUPAF_SAMEASSYSTEM, "IMG_MARK",   IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEDEL",    NULL, iMatrixListSetImageDelAttrib,    IUPAF_SAMEASSYSTEM, "IMG_DEL",    IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEDELUNSEL",  NULL, iMatrixListSetImageDelUnselAttrib,  IUPAF_SAMEASSYSTEM, "IMG_BLOCK_UNSEL",  IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEADD",    NULL, iMatrixListSetImageAddAttrib,    IUPAF_SAMEASSYSTEM, "IMG_ADD",    IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "EDITABLE",   iMatrixListGetEditableAttrib, iMatrixListSetEditableAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "ADDLIN", NULL, iMatrixListSetAddLinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);  /* allowing these methods to be called before map will avoid its storage in the hash table */
  iupClassRegisterAttribute(ic, "DELLIN", NULL, iMatrixListSetDelLinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMLIN", iMatrixListGetNumLinAttrib, iMatrixListSetNumLinAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* Does nothing... this control defines automatically the number of columns to be used */
  iupClassRegisterAttribute(ic, "ADDCOL", NULL, iMatrixListSetAddColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DELCOL", NULL, iMatrixListSetDelColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMCOL", iMatrixListGetNumColAttrib, iMatrixListSetNumColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMCOL_NOSCROLL", iMatrixListGetNumColNoScrollAttrib, iMatrixListSetNumColNoScrollAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMCOL_VISIBLE",  iMatrixListGetNumColVisibleAttrib, NULL, IUPAF_SAMEASSYSTEM, "3", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* initialize default images */
  iMatrixListInitializeImages();

  return ic;
}

Ihandle* IupMatrixList(void)
{
  return IupCreate("matrixlist");
}

#if 0

#define IUP_MTX_SELECT_LINE              "MTX_SELECT_LINE"             /* Attribute applies only to non-mtxList->editable matrix with two columns. */
#define IUP_MTX_SHOW_BTN_DEL_DCLICK      "MTX_SHOW_BTN_DEL_DCLICK"     /* Attribute applies only to non-mtxList->editable matrix. */
#define IUP_MTX_LAST_SELECT_LINE         "MTX_LAST_SELECT_LINE"        /* Save the last line selected. */

/* Matrix attributes for Callbacks, including LUA */
#define IUP_MTX_ACT_DOUBLE_CLICK_CB      "MTX_ACT_DOUBLE_CLICK_CB"     /* Attribute applies only to non-mtxList->editable matrix. */
#define IUP_MTX_DEL_ATTRIB_CB            "MTX_DEL_ATTRIB_CB"           /* Callback de delete. - int function(Ihandle *ih, int lin) */
#define IUP_MTX_CLICK_CB                 "MTX_CLICK_CB"                /* Callback de click. - int function(Ihandle *ih, int lin, int col) */
#define IUP_MTX_ACTION_CB                "MTX_ACTION_CB"               /* Callback de action. - int function(Ihandle *ih, int lin, int col) */
#define IUP_MTX_CHECK_CB                 "MTX_CHECK_CB"                /* Callback de check. - int function(Ihandle *ih, int lin, int col) */
#define IUP_MTX_INSERT_CB                "MTX_INSERT_CB"               /* Callback de enter. Action generated when an item is inserted into the matrix. - int function(Ihandle *ih, int lin, int col) */
#define IUP_MTX_EDIT_CB                  "MTX_EDIT_CB"                 /* Callback de check. - int function(Ihandle *ih, int lin, int col, int mode) */
#define IUP_MTX_NOTIFY_EDIT_MODE_CB      "MTX_NOTIFY_EDIT_MODE_CB"     /* Callback de notificação chamada quando entra e quando sai do modo de edição . - int function(Ihandle *ih, int mode) */

StatusLine => State checked/unchecked
Selected Line

#endif
