/** \file
 * \brief iMatrixList
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "cd.h"
#include "iupim.h"
#include "iupcbs.h"
#include "iupcontrols.h"

#include "iup_attrib.h"
#include "iup_object.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_register.h"
#include "iup_assert.h"
#include "iup_image.h"
#include "iup_str.h"

#include "matrix/iupmat_def.h"
#include "matrix/iupmat_getset.h"
#include "matrix/iupmat_edit.h"

/*
* iMatrixList component
*
* Description : A component that uses the IUP matrix as a list
*      Remark : Depend on libs IUP and CD
*
* Based on MTXLIB, developed in Tecgraf/PUC-Rio
* Thanks to Renata Trautma and Andre Derraik
*/

#define IMTXL_IMG_WIDTH  16
#define IMTXL_IMG_HEIGHT 16

/* Column order options:
| item  |
| image | item  |
| item  | image |
| color | item  |
| item  | color |
| item  | image | color |
| item  | color | image |
| image | color | item  |
| color | image | item  |
*/
enum { IMTXL_INVALID_COLUMN, IMTXL_FIRST_COLUMN, IMTXL_SECOND_COLUMN, IMTXL_THIRD_COLUMN };

typedef struct _ImatrixListData  /* Used only by the IupMatrixList control */
{
  int editModeToggle;  /* Edit mode toggle flag */
  int doubleClickDel;  /* Double click to delete flag */
  int selectedLine;    /* Number of the selected line */

  int show_image, show_color;
  int itemCol, imageCol, colorCol;

  int lastSelLine;
  
} ImatrixListData;

// Deixar esses como atributo
// Tem que atualiza-los quando inserir/remover item da lista
//  char *color;          /* Color associated to the cell value */
//  void *image;          /* Image associated to the cell value */

// Para esses dois podemos criar flags adicionais na IupMatrix
//  int buttonActive;     /* Button active flag */
//  int lineActive;       /* Line active flag */
#define IMAT_IS_BUTTON_ACTIVE  0x1000
#define IMAT_IS_LINE_ACTIVE    0x2000


/******************************************************************************
 Useful functions
******************************************************************************/
static void iMatrixListPutIconMap(Ihandle* ih, void *img, int x, int y, int lin)
{
  //long int mtxColors[2];
  //unsigned char *imageMap;
 
  //if(index == IMG_MARK || index == IMG_UNMARK || index == IMG_ADD)
  //{
  //  mtxColors[0] = cdEncodeColor(10, 10, 10);
  //  mtxColors[1] = cdEncodeColor(255, 255, 255);
  //  imageMap = (index == IMG_MARK ? img_mark : index == IMG_UNMARK ? img_unmark : img_add);
  //}
  //else if(index == IMG_BLOCK_MARK || index == IMG_BLOCK_UNMARK)
  //{
  //  mtxColors[0] = cdEncodeColor(180, 180, 180);
  //  mtxColors[1] = cdEncodeColor(255, 255, 255);
  //  imageMap = (index == IMG_BLOCK_MARK ? img_mark : img_unmark);
  //}
  //else if(index == IMG_DEL)
  //{
  //  mtxColors[0] = cdEncodeColor(255, 235, 155);
  //  mtxColors[1] = cdEncodeColor(255, 0, 0);
  //  imageMap = img_del;
  //}
  //else if(index == IMG_DEL_UNSEL)
  //{
  //  mtxColors[0] = cdEncodeColor(255, 255, 255);
  //  mtxColors[1] = cdEncodeColor(255, 0, 0);
  //  imageMap = img_del;
  //}
  //else if(index == IMG_BLOCK_DEL)
  //{
  //  mtxColors[0] = (mtxList->lastSelLine != lin ? cdEncodeColor(255, 255, 255) : cdEncodeColor(255, 235, 155));
  //  mtxColors[1] = cdEncodeColor(180, 180, 180);
  //  imageMap = img_del;
  //}

  //cdCanvasPutImageRectMap(cnv, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, imageMap,
  //                        mtxColors, x, y, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, 0, 0, 0, 0);
}

static void iMatrixListPutIconRGBA(Ihandle* ih, void *img, int x, int y)
{
  //cdCanvasPutImageRectRGBA(cnv, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT,
  //                         mtxList->imageRGBA[index]->data[0],  /* não tenho certeza aqui... */
  //                         mtxList->imageRGBA[index]->data[1],
  //                         mtxList->imageRGBA[index]->data[2],
  //                         mtxList->imageRGBA[index]->data[3],
  //                         x, y, IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, 0, 0, 0, 0);
}

static unsigned char img_mark[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static unsigned char img_unmark[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static unsigned char img_del[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
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

static unsigned char img_add[IMTXL_IMG_WIDTH*IMTXL_IMG_HEIGHT] = {
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

static void iMatrixListInitializeImages()
{
  Ihandle *image_unmark, *image_mark, *image_del, *image_del_unsel;  
  Ihandle *image_block_unmark, *image_block_mark, *image_block_del, *image_add;

  image_unmark       = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_unmark);
  image_mark         = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_mark);
  image_del          = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_del);
  image_del_unsel    = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_del);
  image_block_unmark = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_unmark);
  image_block_mark   = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_mark);
  image_block_del    = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_del);
  image_add          = IupImage(IMTXL_IMG_WIDTH, IMTXL_IMG_HEIGHT, img_add);

  IupSetAttribute(image_unmark, "0", "10 10 10");
  IupSetAttribute(image_unmark, "1", "255 255 255");

  IupSetAttribute(image_mark, "0", "10 10 10");
  IupSetAttribute(image_mark, "1", "255 255 255");

  IupSetAttribute(image_del, "0", "255 235 155");
  IupSetAttribute(image_del, "1", "255 0 0");

  IupSetAttribute(image_block_mark, "0", "180 180 180");
  IupSetAttribute(image_block_mark, "1", "255 255 255");

  IupSetAttribute(image_block_unmark, "0", "180 180 180");
  IupSetAttribute(image_block_unmark, "1", "255 255 255");

  IupSetAttribute(image_del_unsel, "0", "255 255 255");
  IupSetAttribute(image_del_unsel, "1", "255 0 0");

  IupSetAttribute(image_block_del, "0", "220 220 220");
  IupSetAttribute(image_block_del, "1", "180 180 180");

  IupSetAttribute(image_add, "0", "10 10 10");
  IupSetAttribute(image_add, "1", "255 255 255");

  IupSetHandle("IMG_UNMARK", image_unmark);
  IupSetHandle("IMG_MARK", image_mark);
  IupSetHandle("IMG_DEL", image_del);
  IupSetHandle("IMG_BLOCK_UNMARK", image_block_unmark);
  IupSetHandle("IMG_BLOCK_MARK", image_block_mark);
  IupSetHandle("IMG_DEL_UNSEL", image_del_unsel);
  IupSetHandle("IMG_BLOCK_DEL", image_block_del);
  IupSetHandle("IMG_ADD", image_add);
}

static void iMatrixListInitializeAttributes(Ihandle* ih, ImatrixListData* mtxList)
{
  char buffer[30];
  int i;
  int numColumns = 1;  /* item column */

  if(mtxList->show_color)
    numColumns++;

  if(mtxList->show_image)
    numColumns++;

  IupSetInt(ih, "NUMLIN_VISIBLE", IupGetInt(ih, "NUMLIN"));
  
  IupSetInt(ih, "NUMCOL", numColumns);
  IupSetInt(ih, "NUMCOL_VISIBLE", numColumns);
  IupSetInt(ih, "NUMCOL_NOSCROLL", numColumns-1);

  IupSetAttribute(ih, "NUMLIN_VISIBLE_LAST", "YES");
  IupSetAttribute(ih, "NUMCOL_VISIBLE_LAST", "YES");

  if(mtxList->show_color)
  {
    sprintf(buffer, "WIDTH%d", mtxList->colorCol);
    IupSetInt(ih, buffer, IMTXL_IMG_WIDTH);
  }

  if(mtxList->show_image)
  {
    sprintf(buffer, "WIDTH%d", mtxList->imageCol);
    IupSetInt(ih, buffer, IMTXL_IMG_WIDTH);
  }

  /* Set the text alignment for the item column */
  sprintf(buffer, "ALIGNMENT%d", mtxList->itemCol);
  IupSetAttribute(ih, buffer, "ALEFT");

  /* Set background color of the matrix. */
  IupSetAttribute(ih, "BGCOLOR", "255 255 255");
  IupSetAttribute(ih, "CHECKFRAMECOLOR", "YES");

  /* Set toggle editable or not */
  if(!mtxList->show_image)
    mtxList->editModeToggle = 0;
  else
    mtxList->editModeToggle = 1;

  /* Create the index image to all lines */
  for(i = 0; i < IupGetInt(ih, "NUMLIN"); i++)
  {
    ih->data->cells[i][mtxList->itemCol].lineActive = 1;  /* ON */
    IupSetAttributeId2(ih, "FRAMEVERTCOLOR", i, mtxList->itemCol, "BGCOLOR");

    if(mtxList->show_image)
    {
      ih->data->cells[i][mtxList->imageCol].buttonActive = 1;  /* ON */
      IupSetAttributeId2(ih, "FRAMEVERTCOLOR", i, mtxList->imageCol, "BGCOLOR");
    }
    
    if(mtxList->show_color)
    {
      IupSetAttributeId2(ih, "BGCOLOR", i, mtxList->colorCol, "255 255 255");
      IupSetAttributeId2(ih, "FRAMEVERTCOLOR", i, mtxList->colorCol, "BGCOLOR");
    }
  }
}

/******************************************************************************
 Attributes
******************************************************************************/
static char* iMatrixListGetOrderColorColAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;
  
  /* color column will not be shown */
  if (!mtxList->show_color)
    return 0;

  if (!mtxList->show_image)
  {
    if (mtxList->colorCol == IMTXL_FIRST_COLUMN)
      return "LEFT";
    else
      return "RIGHT";
  }
  else
  {
    if (mtxList->colorCol == IMTXL_FIRST_COLUMN)
      return "LEFT";
    else if (mtxList->colorCol == IMTXL_THIRD_COLUMN)
      return "RIGHT";
    else
      return "MIDDLE";
  }
}

static int iMatrixListSetOrderColorColAttrib(Ihandle *ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;
  
  /* color column will not be shown */
  if (!mtxList->show_color)
    return 0;

  if (!mtxList->show_image)
  {
    if(iupStrEqualNoCase(value, "LEFT"))
    {
      mtxList->colorCol = IMTXL_FIRST_COLUMN;
      mtxList->itemCol  = IMTXL_SECOND_COLUMN;
    }
    else  /* RIGHT */
    {
      mtxList->itemCol  = IMTXL_FIRST_COLUMN;
      mtxList->colorCol = IMTXL_SECOND_COLUMN;
    }
  }
  else
  {
    if(iupStrEqualNoCase(value, "LEFT"))
    {
      mtxList->colorCol = IMTXL_FIRST_COLUMN;
      mtxList->imageCol = IMTXL_SECOND_COLUMN;
      mtxList->itemCol  = IMTXL_THIRD_COLUMN;
    }
    else if(iupStrEqualNoCase(value, "RIGHT"))
    {
      mtxList->itemCol  = IMTXL_FIRST_COLUMN;
      mtxList->imageCol = IMTXL_SECOND_COLUMN;
      mtxList->colorCol = IMTXL_THIRD_COLUMN;
    }
    else  /* MIDDLE */
    {
      mtxList->itemCol  = IMTXL_FIRST_COLUMN;
      mtxList->colorCol = IMTXL_SECOND_COLUMN;
      mtxList->imageCol = IMTXL_THIRD_COLUMN;
    }
  }

  return 0;
}

static char* iMatrixListGetOrderImageColAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;
  
  /* color column will not be shown */
  if (!mtxList->show_image)
    return 0;

  if (!mtxList->show_color)
  {
    if (mtxList->imageCol == IMTXL_FIRST_COLUMN)
      return "LEFT";
    else
      return "RIGHT";
  }
  else
  {
    if (mtxList->imageCol == IMTXL_FIRST_COLUMN)
      return "LEFT";
    else if (mtxList->imageCol == IMTXL_THIRD_COLUMN)
      return "RIGHT";
    else
      return "MIDDLE";
  }
}

static int iMatrixListSetOrderImageColAttrib(Ihandle *ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;
  
  /* image column will not be shown */
  if (!mtxList->show_image)
    return 0;

  if (!mtxList->show_color)
  {
    if(iupStrEqualNoCase(value, "LEFT"))
    {
      mtxList->imageCol = IMTXL_FIRST_COLUMN;
      mtxList->itemCol  = IMTXL_SECOND_COLUMN;
    }
    else  /* RIGHT */
    {
      mtxList->itemCol  = IMTXL_FIRST_COLUMN;
      mtxList->imageCol = IMTXL_SECOND_COLUMN;
    }
  }
  else
  {
    if(iupStrEqualNoCase(value, "LEFT"))
    {
      mtxList->imageCol = IMTXL_FIRST_COLUMN;
      mtxList->colorCol = IMTXL_SECOND_COLUMN;
      mtxList->itemCol  = IMTXL_THIRD_COLUMN;
    }
    else if(iupStrEqualNoCase(value, "RIGHT"))
    {
      mtxList->itemCol  = IMTXL_FIRST_COLUMN;
      mtxList->colorCol = IMTXL_SECOND_COLUMN;
      mtxList->imageCol = IMTXL_THIRD_COLUMN;
    }
    else  /* MIDDLE */
    {
      mtxList->itemCol  = IMTXL_FIRST_COLUMN;
      mtxList->imageCol = IMTXL_SECOND_COLUMN;
      mtxList->colorCol = IMTXL_THIRD_COLUMN;
    }
  }

  return 0;
}

static char* iMatrixListGetOrderItemColAttrib(Ihandle *ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;
  
  if (mtxList->itemCol == IMTXL_FIRST_COLUMN)
    return "LEFT";
  else
    return "RIGHT";
}

static int iMatrixListSetShowColorAttrib(Ihandle *ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    mtxList->show_color = 1;
  else
    mtxList->show_color = 0;

  /* Default order of this column */
  mtxList->colorCol = IMTXL_SECOND_COLUMN;    

  return 0;
}

static char* iMatrixListGetShowColorAttrib(Ihandle* ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  if(mtxList->show_color)
    return "YES";
  else
    return "NO";
}

static int iMatrixListSetShowImageAttrib(Ihandle *ih, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    mtxList->show_image = 1;
  else
    mtxList->show_image = 0;

  /* Default order of this column */
  if(mtxList->show_color)
    mtxList->imageCol = IMTXL_THIRD_COLUMN;
  else
    mtxList->imageCol = IMTXL_SECOND_COLUMN;

  return 0;
}

static char* iMatrixListGetShowImageAttrib(Ihandle* ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  if(mtxList->show_image)
    return "YES";
  else
    return "NO";
}

static int iMatrixListSetImageAttrib(Ihandle* ih, int lin, const char* value)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");

  if (mtxList->show_image && iupMatrixCheckCellPos(ih, lin, mtxList->imageCol))
  {
    ih->data->cells[lin][mtxList->imageCol].image = iupImageGetImage(value, ih, 0);

    /* Force a matrix redisplay */
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageUnmarkAttrib(Ihandle* ih, const char* value)
{
  //TODO Isso está errado!!!!!
  // Veja o que a iupImageGetImage retorna
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_UNMARK", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageMarkAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_MARK", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageDelAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_DEL", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageBlockUnmarkAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_BLOCK_UNMARK", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageBlockMarkAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_BLOCK_MARK", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageBlockUnselAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_DEL_UNSEL", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageBlockDelAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_BLOCK_DEL", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetImageAddAttrib(Ihandle* ih, const char* value)
{
  Ihandle *img = iupImageGetImage(value, ih, 0);
  if(img)
  {
    IupSetHandle("IMG_ADD", img);
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetColorAttrib(Ihandle* ih, int lin, const char* value)
{
  unsigned char r, g, b;
  
  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  if (mtxList->show_color && iupMatrixCheckCellPos(ih, lin, mtxList->colorCol))
  {
    ih->data->cells[lin][mtxList->colorCol].color = iupStrDup(value);

    /* Force a matrix redisplay */
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  return 0;
}

static int iMatrixListSetButtonActiveAttrib(Ihandle* ih, int lin, const char* value)
{
  /* Just checking */
  if(lin <= 0 || !mtxList->show_image)
    return 0;

  /* Set the new value */
  if(iupStrEqualNoCase(value, "ON"))
    ih->data->cells[lin][mtxList->imageCol].buttonActive = 1;
  else
    ih->data->cells[lin][mtxList->imageCol].buttonActive = 0;  /* OFF */

  /* Force a matrix redisplay */
  IupSetAttribute(ih, "REDRAW", "ALL");

  return 0;
}

static int iMatrixListSetLineActiveAttrib(Ihandle* ih, int lin, const char* value)
{
  int editMode = IupGetInt(ih, "EDIT_MODE");

  /* Just checking */
  if(lin <= 0)
    return 0;

  /* Set the new value */
  if(iupStrEqualNoCase(value, "ON"))
  {
    ih->data->cells[lin][mtxList->itemCol].lineActive = 1;

    if(editMode && mtxList->doubleClickDel)
      IupSetAttribute(ih, "ACTIVE", "YES");
  }
  else
  {
    ih->data->cells[lin][mtxList->itemCol].lineActive = 1;  /* OFF */

    if(editMode && mtxList->doubleClickDel)
      IupSetAttribute(ih, "ACTIVE", "NO");
  }

  /* Force a matrix redisplay */
  IupSetAttribute(ih, "REDRAW", "ALL");

  return 0;
}

static int iMatrixListSetIdValueAttrib(Ihandle* ih, int lin, const char* value)
{
  if (iupMatrixCheckCellPos(ih, lin, mtxList->itemCol))
    iupMatrixSetValue(ih, lin, mtxList->itemCol, value, 0);
  return 0;
}

static char* iMatrixListGetIdValueAttrib(Ihandle* ih, int lin)
{
  if (iupMatrixCheckCellPos(ih, lin, mtxList->itemCol))
    return iupMatrixGetValueString(ih, lin, mtxList->itemCol);
  return NULL;
}

static int iMatrixListSetValueAttrib(Ihandle* ih, const char* value)
{
  /* Is the focus cell a item column cell ? */
  if(ih->data->columns.focus_cell != mtxList->itemCol)
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
  /* Is the focus cell a item column cell ? */
  if(ih->data->columns.focus_cell != mtxList->itemCol)
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

/******************************************************************************
 Callbacks
******************************************************************************/
static int iMatrixListDraw_CB(Ihandle *ih, int lin, int col, int x1, int x2, int y1, int y2, cdCanvas *cnv)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  int numLines = IupGetInt(ih, "NUMLIN");
  int editMode = IupGetInt(ih, "EDIT_MODE");

  /* Just checking */
  if(lin <= 0 || col <= 0 || !cnv)
    return IUP_DEFAULT;

  /* Check if we have the color column */
  if(mtxList->colorCol && col == mtxList->colorCol)
  {
    /* Don't draw the color column on the empty line. */
    if((lin < numLines && editMode) || (lin <= numLines && !editMode))
    {
      IFniiiiiiC cb = (IFniiiiiiC)IupGetCallback(ih, "DRAWCOLORCOL_CB");
      
      /* If draw callback is defined, delegate the draw action. */ 
      if(cb)
        return cb(ih, lin, col, x1, x2, y1, y2, cnv);
      else
      {    
        char *color = ih->data->cells[lin][mtxList->colorCol].color;
        
        /* We have the color attribute ? */
        if(color)
        {
          static const int DX_BORDER = 2;
          static const int DY_BORDER = 3;
          static const int DX_FILL = 3;
          static const int DY_FILL = 4;
          unsigned char red = 255, green = 255, blue = 255;

          /* Fill the box with the color */
          iupStrToRGB(color, &red, &green, &blue);
          cdCanvasForeground(cnv, cdEncodeColor(red, green, blue));
          cdCanvasBox(cnv, x1 + DX_FILL, x2 - DX_FILL, y1 - DY_FILL, y2 + DY_FILL);

          /* Draw the border */
          if(iupStrEqualNoCase(IupGetAttribute(ih, IUP_ACTIVE), IUP_NO))
            cdCanvasForeground(cnv, CD_GRAY);
          else
            cdCanvasForeground(cnv, CD_BLACK);

          cdCanvasRect(cnv, x1 + DX_BORDER, x2 - DX_BORDER, y1 - DY_BORDER, y2 + DY_BORDER);
        }
      }
    }

    /* Don't need to do anything else */
    return IUP_IGNORE;
  }

  /* Process only the image column */
  if(col != mtxList->imageCol)
    return IUP_IGNORE;

  /* Don't draw the image to the empty line */
  if((lin < numLines && editMode) || (lin <= numLines && !editMode))
  {
    /* Find the image point */
    int x = x2 - x1 - IMTXL_IMG_WIDTH;
    int y = y1 - y2 - 1 - IMTXL_IMG_HEIGHT;
    x /= 2; x += x1;
    y /= 2; y += y2;

    /* Get the index image to display */
    /* Select the correct image icon to display */
    if(editMode && (!ih->data->cells[lin]->lineActive || !ih->data->cells[lin][mtxList->imageCol].buttonActive))
    {
      /* Put the icon */
      if(IupGetHandle("IMG_BLOCK_MARK") != NULL && IupGetHandle("IMG_BLOCK_UNMARK") != NULL)
        iMatrixListPutIconRGBA(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y);
      else
        iMatrixListPutIconMap(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y, lin);
    }
    else if(mtxList->doubleClickDel && !ih->data->cells[lin][mtxList->itemCol].lineActive)
    {
      if(IupGetHandle("IMG_BLOCK_DEL") != NULL)
        iMatrixListPutIconRGBA(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y);
      else
        iMatrixListPutIconMap(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y, lin);
    }
    else if(mtxList->doubleClickDel && ih->data->cells[lin][mtxList->itemCol].lineActive)
    {
      if(IupGetHandle("IMG_DEL") != NULL)
        iMatrixListPutIconRGBA(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y);
      else
        iMatrixListPutIconMap(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y, lin);
    }
    else
    {
      if(IupGetHandle("IMG_UNMARK") != NULL && IupGetHandle("IMG_MARK") != NULL)
        iMatrixListPutIconRGBA(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y);
      else
        iMatrixListPutIconMap(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y, lin);
    }
  }

  if(lin == numLines && IupGetHandle("IMG_ADD") != NULL && editMode)
  {
    int x = x2 - x1 - IMTXL_IMG_WIDTH;
    int y = y1 - y2 - 1 - IMTXL_IMG_HEIGHT;
    x /= 2; x += x1;
    y /= 2; y += y2;

    iMatrixListPutIconRGBA(ih, ih->data->cells[lin][mtxList->imageCol].image, x, y);
  }

  return IUP_DEFAULT;
}

static int iMatrixListEdition_CB(Ihandle *ih, int lin, int col, int mode, int update)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  int status   = IUP_DEFAULT;
  int numLines = IupGetInt(ih, "NUMLIN");
  int editMode = IupGetInt(ih, "EDIT_MODE");
  IFniii editCB = (IFniii)IupGetCallback(ih, "EDIT_CB");

  if(col != mtxList->itemCol)
    return IUP_IGNORE;

  if(editCB != NULL && ih->data->cells[lin][mtxList->itemCol].lineActive)
    status = editCB(ih, lin, col, mode);

  if(!editMode)
  {
    IFnis dbClickCB = (IFnis)IupGetCallback(ih, "DBLCLICK_CB");

    if(dbClickCB != NULL)
      status = dbClickCB(ih, lin, IupGetAttributeId(ih, "", lin));
    else
      status = IUP_IGNORE;
  }
  else
  {
    /* Update last action */
    IupSetInt(ih, "ACTION_TYPE", 1);  /* Edit action */

    /* Editing the item column ? */
    if(col == mtxList->itemCol)
    {
      if(mode == 1)
      {
        mtxList->lastSelLine = lin;
      }
      else if(mode == 0)
      {
        IFni insertCB = (IFni)IupGetCallback(ih, "INSERT_CB");

        if(update == 0)
          return IUP_IGNORE;

        if(numLines == lin)
        {
          int i;
          if(insertCB != NULL)
          {
            for(i = 1; i <= lin; i++)
              iMatrixListSetLineActiveAttrib(ih, i, "OFF");
            
            status = insertCB(ih, lin);
            
            for(i = 1; i <= lin; i++)
              iMatrixListSetLineActiveAttrib(ih, i, "ON");

            if(status == IUP_IGNORE)
            {
              IupSetAttributeId2(ih, "", lin, mtxList->itemCol, "");
              
              /* Force a matrix redisplay */
              IupSetAttribute(ih, "REDRAW", "ALL");
              
              return IUP_CONTINUE;
            }
            else
            {
              IFnii actionCB = (IFnii)IupGetCallback(ih, "ACTION_CB");

              /* Add the line */
              IupSetInt(ih, "ADDLIN", lin);
              
              /* Set it active */
              IupSetAttributeId2(ih, "", lin+1, mtxList->itemCol, "");
              ih->data->cells[lin+1][mtxList->itemCol].lineActive = 1;  /* ON */

              /* Check if we need to call a function */
              if(actionCB != NULL)
                actionCB(ih, mtxList->lastSelLine, mtxList->itemCol);
            }
          }
        }
      }
    }
    else
    {
      /* Don't allow to edit this cell */
      status = IUP_IGNORE;
    }

    /* Force a matrix redisplay */
    IupSetAttribute(ih, "REDRAW", "ALL");
  }

  /* Notify if the application is or is not in edit mode */
  if(status != IUP_IGNORE)
  {
    IFni notifyEditionCB = (IFni)IupGetCallback(ih, "NOTIFYEDITION_CB");

    if(notifyEditionCB != NULL)
      status = notifyEditionCB(ih, mode);
  }

  /* Return the action */
  return status;
}

static int iMatrixListClick_CB(Ihandle *ih, int lin, int col, char *status)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  static int lock = 0;
  IFnis clickCB = (IFnis)IupGetCallback(ih, "CLICK_CB");
  int ret = IUP_DEFAULT;
  char buffer[30];
  (void)col;

  /* The line is locked ? */
  if(lock || lin <= 0)
    return IUP_IGNORE;

  /* Lock line */
  lock = 1;

  /* Select the clicked line */
  sprintf(buffer, "MARK%d:0", lin);
  IupSetAttribute(ih, buffer, "YES");

  /* Set the cell focus */
  sprintf(buffer, "%d:%d", lin, mtxList->itemCol);
  IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));

  /* Set the last selected line */
  mtxList->lastSelLine = lin;

  /* Force a matrix redisplay */
  IupSetAttribute(ih, "REDRAW", "ALL");

  /* Check if we need to call a function */
  if(clickCB != NULL && ih->data->cells[lin][mtxList->itemCol].lineActive)
    ret = clickCB(ih, lin, status);

  /* Release line */
  lock = 0;

  return ret;
}

static int iMatrixListLeave_CB(Ihandle *ih, int lin, int col)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  int ret = IUP_DEFAULT;
  int numLines = IupGetInt(ih, "NUMLIN");
  int editMode = IupGetInt(ih, "EDIT_MODE");
  int lastAction = IupGetInt(ih, "ACTION_TYPE");
  char buffer[30];

  /* Return if not editable */
  if(editMode)
    return IUP_IGNORE;

  if(lin == numLines && lastAction)  /* last action = edition */
  {
    /* Get the cell value */
    char *data = IupGetAttributeId2(ih, "", lin, col);
    if(data != NULL && data[0] != '\0')
    {
      /* Add new line */
      IupSetInt(ih, "ADDLIN", lin);

      /* Activate the new line */
      ih->data->cells[lin+1][mtxList->itemCol].lineActive = 1;  /* ON */

      /* Show delete button ? */
      if(mtxList->doubleClickDel)
      {
        sprintf(buffer, "%d:%d", lin, mtxList->itemCol);
        IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));
      }

      /* Reset values the line created */
      IupStoreAttributeId2(ih, "", lin+1, mtxList->itemCol, "");

      /* Update the last select line */
      mtxList->lastSelLine = lin;

      /* Force a matrix redisplay */
      IupSetAttribute(ih, "REDRAW", "ALL");
    }
  }

  /* Update last matrix state */
  IupSetInt(ih, "ACTION_TYPE", 0);  /* Action none */

  return ret;
}

static int iMatrixListEditLabel(Ihandle *ih, ImatrixListData* mtxList, int c, int lin, int col, int active, char* after)
{
  int line = lin;
  char buffer[30];
  char *index;
  int numLines = IupGetInt(ih, IUP_NUMLIN);
  IFnii actionCB = (IFnii)IupGetCallback(ih, "ACTION_CB");
  int ret = IUP_DEFAULT;
  (void)active;

  if(!IupGetInt(ih, IUP_ACTIVE))  /* return = NO */
    return IUP_IGNORE;

  if(((c == K_DOWN) || (c == K_UP)) && mtxList->doubleClickDel)
  {
    int linSel = 0;

    if(c == K_DOWN)
      linSel = (lin == numLines ?  numLines : lin + 1);
    else if(c == K_UP)
      linSel = (lin == 1 ? 1 : lin - 1);
  }

  /* Get matrix atrributes */
  index = IupGetName((Ihandle*)ih->data->cells[lin][mtxList->imageCol].image);

  if(col == mtxList->itemCol || col == mtxList->imageCol)
  {
    char *data = after;

    /* Is the item column ? */
    if(col != mtxList->itemCol)
    {
      /* Set the cell focus */
      sprintf(buffer, "%d:%d", lin, mtxList->itemCol);
      IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));

      /* Get the cell value */
      data = IupGetAttributeId2(ih, "", lin, mtxList->itemCol);
    }

    if(((c == K_CR) &&  (iupStrEqualNoCase(IupGetAttribute(ih, "EDITNEXT"), "NONE"))) || c == K_UP || c == K_DOWN)
    {
      if(c == K_UP && lin != 1)
      {
        line = lin - 1;

        /* Check if we need to call a function */
        if(actionCB != NULL)
          ret = actionCB(ih, line, mtxList->itemCol);

        mtxList->lastSelLine = line;
      }
      else if((c == K_DOWN && lin != numLines) || 
               ((c == K_CR &&  (iupStrEqualNoCase(IupGetAttribute(ih, "EDITNEXT"), "NONE"))) && data != NULL && data[0] != '\0'))
      {
        line = lin + 1;

        /* Check if we need to call a function */
        if(actionCB != NULL)
          ret = actionCB(ih, line, mtxList->itemCol);

        /* Save the selected line */
        mtxList->lastSelLine = line;
      }
    }
  }

  /* Return the status */
  return ret;
}

static int iMatrixListSelectLine(Ihandle *ih, ImatrixListData* mtxList, int c, int lin, int col, int active, char* after)
{
  int i;
  int line = lin;
  int ret = IUP_DEFAULT;
  int numLines = IupGetInt(ih, "NUMLIN");
  IFnii actionCB = (IFnii)IupGetCallback(ih, "ACTION_CB");
  char buffer[30];
  (void)after;
  (void)active;

  if(mtxList->itemCol != col)
    return IUP_DEFAULT;

  /* Process the key */
  if(c == K_UP)
  {
    line = mtxList->lastSelLine - 1;

    for(i = line; i >= 1; i--)
    {
      /* Unset the current selected line */
      sprintf(buffer, "MARK%d:0", mtxList->lastSelLine);
      IupSetAttribute(ih, buffer, "NO");

      /* Set the new selected line */
      sprintf(buffer, "MARK%d:0", i);
      IupSetAttribute(ih, buffer, "YES");

      /* Set the new focus */
      sprintf(buffer, "%d:%d", i, mtxList->itemCol);
      IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));

      /* Save the last selected line */
      mtxList->lastSelLine = i;

      if(ih->data->cells[i][mtxList->itemCol].lineActive)
      {
        /* Check if we need to call a function */
        if(actionCB != NULL)
          ret = actionCB(ih, i, mtxList->itemCol);

        /* Stop the loop */
        break;
      }
    }
  }
  else if(c == K_DOWN)
  {
    line = mtxList->lastSelLine + 1;

    for(i = line; i <= numLines; i++)
    {
      /* Unset the current selected line */
      sprintf(buffer, "MARK%d:0", mtxList->lastSelLine);
      IupSetAttribute(ih, buffer, "NO");

      /* Set the new selected line */
      sprintf(buffer, "MARK%d:0", i);
      IupSetAttribute(ih, buffer, "YES");

      /* Set the new focus */
      sprintf(buffer, "%d:%d", i, mtxList->itemCol);
      IupSetAttribute(ih, "FOCUS_CELL", iupStrDup(buffer));

      /* Save the last selected line */
      mtxList->lastSelLine = i;

      if(ih->data->cells[i][mtxList->itemCol].lineActive)
      {
        /* Check if we need to call a function */
        if(actionCB != NULL)
          ret = actionCB(ih, i, mtxList->itemCol);

        /* Stop the loop */
        break;
      }
    }
  }
  else if(c == K_CR)
  {
    IFnis dbClickCB = (IFnis)IupGetCallback(ih, "DBLCLICK_CB");

    /* The line is active ? */
    if(ih->data->cells[lin][mtxList->itemCol].lineActive)
    {
      /* Check if we need to call a function */
      if(dbClickCB != NULL)
        ret = dbClickCB(ih, lin, IupGetAttributeId(ih, "", lin));
      else
        ret = IUP_IGNORE;
    }
  }

  /* Return the status */
  return ret;
}

static int iMatrixListAction_CB(Ihandle *ih, int c, int lin, int col, int active, char* after)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  int ret = IUP_DEFAULT;
  int editMode = IupGetInt(ih, "EDIT_MODE");

  /* The matrix is editable ? */
  if(editMode)
    ret = iMatrixListEditLabel(ih, mtxList, c, lin, col, active, after);
  else
    ret = iMatrixListSelectLine(ih, mtxList, c, lin, col, active, after);

  /* Force a matrix redisplay */
  IupSetAttribute(ih, "REDRAW", "ALL");

  return ret;
}

/******************************************************************************
 Methods
******************************************************************************/
static void iMatrixListUnMapMethod(Ihandle* ih)
{
  ImatrixListData* mtxList = (ImatrixListData*)iupAttribGet(ih, "_IUP_MATRIXLIST_DATA");
  free(mtxList);
}

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
  mtxList->itemCol  = IMTXL_FIRST_COLUMN;
  mtxList->imageCol = IMTXL_INVALID_COLUMN;
  mtxList->colorCol = IMTXL_INVALID_COLUMN;
  mtxList->doubleClickDel = 0;  /* Disable double-click to delete */
  mtxList->editModeToggle = 0;  /* Start toggle no editable       */
  mtxList->selectedLine   = 0;  /* Start with no selected line    */
  mtxList->lastSelLine    = 0;
  mtxList->show_image     = 0;
  mtxList->show_color     = 0;

  /* starting array of default images */
  iMatrixListInitializeImages();

  /* change the IupCanvas default values */
  iupAttribSet(ih, "SCROLLBAR", "VERTICAL");
  iupAttribSet(ih, "BORDER", "NO");
  iupAttribSet(ih, "CURSOR", "ARROW");

  /* Change the IupMatrix default values */
  iupAttribSet(ih, "HIDEFOCUS", "YES");  /* Hide the matrix focus feedback */
  iupAttribSet(ih, "EXPAND", "NO");      /* Disable the expand option */
  iupAttribSet(ih, "MARKMODE", "LIN");   /* Select the entire line */

  /* iMatrix callbacks */
  IupSetCallback(ih, "DRAW_CB",  (Icallback)iMatrixListDraw_CB);
  IupSetCallback(ih, "CLICK_CB", (Icallback)iMatrixListClick_CB);
  //IupSetCallback(ih, "EDITION_CB",   (Icallback)iMatrixListEdition_CB);
  //IupSetCallback(ih, "LEAVEITEM_CB", (Icallback)iMatrixListLeave_CB);
  IupSetCallback(ih, "ACTION_CB",    (Icallback)iMatrixListAction_CB);
  IupSetCallback(ih, "VALUE_CB",     NULL);  /* Force the matrix to use the VALUE attribute */

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
  ic->has_attrib_id  = 1;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->New    = iupMatrixListNewClass;
  ic->Create = iMatrixListCreateMethod;
  ic->Map    = iMatrixListMapMethod;
  ic->UnMap  = iMatrixListUnMapMethod;

  /* iMatrixList Callbacks */
  iupClassRegisterCallback(ic, "DBLCLICK_CB", "is");
  iupClassRegisterCallback(ic, "INSERT_CB", "i");
  iupClassRegisterCallback(ic, "NOTIFYEDITION_CB", "i");
  iupClassRegisterCallback(ic, "EDIT_CB", "iii");
  iupClassRegisterCallback(ic, "DRAWCOLORCOL_CB", "iiiiiiv");

  iupClassRegisterReplaceAttribDef(ic, "CURSOR", IUPAF_SAMEASSYSTEM, "ARROW");

  /* iMatrixList Attributes */
  iupClassRegisterAttributeId(ic, "IDVALUE", iMatrixListGetIdValueAttrib, iMatrixListSetIdValueAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iMatrixListGetValueAttrib, iMatrixListSetValueAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "ORDERCOLORCOL", iMatrixListGetOrderColorColAttrib, iMatrixListSetOrderColorColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORDERIMAGECOL", iMatrixListGetOrderImageColAttrib, iMatrixListSetOrderImageColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORDERITEMCOL",  iMatrixListGetOrderImageColAttrib, iMatrixListSetOrderImageColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "SHOWCOLOR", iMatrixListGetShowColorAttrib, iMatrixListSetShowColorAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWIMAGE", iMatrixListGetShowImageAttrib, iMatrixListSetShowImageAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "IMAGE", NULL, iMatrixListSetImageAttrib, IUPAF_IHANDLENAME|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR", NULL, iMatrixListSetColorAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGEUNMARK", NULL, iMatrixListSetImageUnmarkAttrib, IUPAF_SAMEASSYSTEM, "IMG_UNMARK", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEMARK",   NULL, iMatrixListSetImageMarkAttrib,   IUPAF_SAMEASSYSTEM, "IMG_MARK",   IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEDEL",    NULL, iMatrixListSetImageDelAttrib,    IUPAF_SAMEASSYSTEM, "IMG_DEL",    IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEADD",    NULL, iMatrixListSetImageAddAttrib,    IUPAF_SAMEASSYSTEM, "IMG_ADD",    IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBLOCKUNMARK", NULL, iMatrixListSetImageBlockUnmarkAttrib, IUPAF_SAMEASSYSTEM, "IMG_BLOCK_UNMARK", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBLOCKMARK",   NULL, iMatrixListSetImageBlockMarkAttrib,   IUPAF_SAMEASSYSTEM, "IMG_BLOCK_MARK",   IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBLOCKUNSEL",  NULL, iMatrixListSetImageBlockUnselAttrib,  IUPAF_SAMEASSYSTEM, "IMG_BLOCK_UNSEL",  IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBLOCKDEL",    NULL, iMatrixListSetImageBlockDelAttrib,    IUPAF_SAMEASSYSTEM, "IMG_BLOCK_DEL",    IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "BUTTONACTIVE", NULL, iMatrixListSetButtonActiveAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "LINEACTIVE",   NULL, iMatrixListSetLineActiveAttrib,   IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Does nothing... this control defines automatically the number of columns to be used */
  iupClassRegisterAttribute(ic, "ADDCOL", NULL, iMatrixListSetAddColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DELCOL", NULL, iMatrixListSetDelColAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupMatrixList(void)
{
  return IupCreate("matrixlist");
}

#if 0
  A Matrixlist é para ser mais isolada em relação a Matrix. 
  Nesse sentido não pode ter a sua própria estrutura dentro da Matrix,
  tem que ser declarada apenas aqui.
  Assim, mudei o código mas não terminei todas as mudanças.

// TODO: como evitar que o atributo "NUMCOL" seja definido (set) pelo usuário?
  Da mesma forma que em ADDCOL/DELCOL, 
  mas tem que substituir no código da MatrixList o set de NUMCOL, 
  para algo interno da Matrix
// TODO: não está pegando o atributo "EDIT_MODE" definido na aplicação do usuário... usar iupAttribGet dentro do controle?
  Não entendi
// TODO: como desenhar uma imagem no canvas, a partir de uma IupImage? (iMatrixListDraw_CB)
  Usando o atributo WID da IupImage. Isso vai restringir o tipo de IupImage que pode ser usado
  mas isso não é um problema dentro do contexto do MatrixList
  Por sinal, os atributos de imagem estão todos incorretos. Eles a principio não precisam ter método de set, depois te explico.
// TODO: como permitir a edição, considerando os códigos de iMatrixListEdition_CB, iMatrixListLeave_CB e iMatrixListAction_CB? Ou eles são desnecessários?
// TODO: chamar novas callbacks de ACTION_CB dentro da própria Action? (pelo que entendi, o controle do pessoal do Recon criava novas callbacks...)
  Depois vamos ver isso.
#endif
