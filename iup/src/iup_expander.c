/** \file
 * \brief iupexpander control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_childtree.h"
#include "iup_draw.h"
#include "iup_image.h"


static Ihandle* load_image_arrowup_highlight(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 175, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 211, 116, 116, 116, 255, 0, 0, 0, 211, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 116, 116, 116, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 171, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 171, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 159, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 0, 0, 0, 143, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowdown_highlight(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 116, 116, 116, 255, 0, 0, 0, 143, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 159, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 171, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 171, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 116, 116, 116, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 211, 116, 116, 116, 255, 0, 0, 0, 211, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 175, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowleft_highlight(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 159, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 171, 116, 116, 116, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 116, 116, 116, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 211, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 211, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 116, 116, 116, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 171, 116, 116, 116, 255, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 159, 116, 116, 116, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowright_highlight(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 0, 0, 0, 159, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 116, 116, 116, 255, 0, 0, 0, 171, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 211, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 175, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 255, 255, 255, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 211, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 255, 255, 255, 255, 116, 116, 116, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 116, 116, 116, 255, 0, 0, 0, 171, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 116, 116, 255, 0, 0, 0, 159, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowup(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 175, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 211, 0, 0, 0, 255, 0, 0, 0, 211, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 171, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 171, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 159, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowleft(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 159, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 171, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 211, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 211, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 171, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 159, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowright(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 159, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 171, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 211, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 175, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 211, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 171, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 159, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static Ihandle* load_image_arrowdown(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 159, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 171, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 171, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 195, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 195, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 211, 0, 0, 0, 255, 0, 0, 0, 211, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 175, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Ihandle* image = IupImageRGBA(15, 15, imgdata);
  return image;
}

static void iExpanderLoadImages(void)
{
  IupSetHandle("IupArrowUp", load_image_arrowup());
  IupSetHandle("IupArrowLeft", load_image_arrowleft());
  IupSetHandle("IupArrowRight", load_image_arrowright());
  IupSetHandle("IupArrowDown", load_image_arrowdown());
  IupSetHandle("IupArrowUpHighlight", load_image_arrowup_highlight());
  IupSetHandle("IupArrowDownHighlight", load_image_arrowdown_highlight());
  IupSetHandle("IupArrowLeftHighlight", load_image_arrowleft_highlight());
  IupSetHandle("IupArrowRightHighlight", load_image_arrowright_highlight());
}

enum { IEXPANDER_LEFT, IEXPANDER_RIGHT, IEXPANDER_TOP, IEXPANDER_BOTTOM };
enum { IEXPANDER_CLOSE, IEXPANDER_OPEN, IEXPANDER_OPEN_FLOAT };

struct _IcontrolData
{
  /* attributes */
  int position;
  int state;
  int bar_size;
  int extra_buttons;
  int auto_show;

  /* aux */
  int highlight;
  Ihandle* timer;
};


static void iExpanderUpdateStateImage(Ihandle* ih)
{
  /* expander -> bar -> box -> (expand_button, ...) */
  Ihandle* box = ih->firstchild->firstchild;
  Ihandle* expand_button = box->firstchild;

  char* image = iupAttribGetStr(ih, "IMAGE");
  if (image)
  {
    if (ih->data->state != IEXPANDER_CLOSE)
    {
      char* imopen = iupAttribGetStr(ih, "IMAGEOPEN");
      if (imopen) image = imopen;

      if (ih->data->highlight)
      {
        char* imhighlight = iupAttribGetStr(ih, "IMAGEOPENHIGHLIGHT");
        if (imhighlight) image = imhighlight;
      }
    }
    else
    {
      if (ih->data->highlight)
      {
        char* imhighlight = iupAttribGetStr(ih, "IMAGEHIGHLIGHT");
        if (imhighlight) image = imhighlight;
      }
    }

    IupSetAttribute(expand_button, "IMAGE", image);
  }
  else
  {
    char* title;

    /* the arrow points in the direction of the action */

    switch (ih->data->position)
    {
    case IEXPANDER_LEFT:
      if (ih->data->state == IEXPANDER_CLOSE)
        image = "IupArrowRight";
      else
        image = "IupArrowLeft";
      break;
    case IEXPANDER_RIGHT:
      if (ih->data->state == IEXPANDER_CLOSE)
        image = "IupArrowLeft";
      else
        image = "IupArrowRight";
      break;
    case IEXPANDER_BOTTOM:
      if (ih->data->state == IEXPANDER_CLOSE)
        image = "IupArrowUp";
      else
        image = "IupArrowDown";
      break;
    default: /* IEXPANDER_TOP */
      title = iupAttribGetStr(ih, "TITLE");
      if (title)
      {
        if (ih->data->state == IEXPANDER_CLOSE)
          image = "IupArrowRight";
        else
          image = "IupArrowDown";
      }
      else
      {
        if (ih->data->state == IEXPANDER_CLOSE)
          image = "IupArrowDown";
        else
          image = "IupArrowUp";
      }
      break;
    }

    if (ih->data->highlight)
      IupSetfAttribute(expand_button, "IMAGE", "%sHighlight", image);
    else
      IupSetAttribute(expand_button, "IMAGE", image);
  }
}

static void iExpanderOpenCloseChild(Ihandle* ih, int refresh, int callcb, int state)
{
  Ihandle* child = ih->firstchild->brother;

  if (callcb)
  {
    IFni cb = (IFni)IupGetCallback(ih, "OPENCLOSE_CB");
    if (cb)
    {
      int ret = cb(ih, state);
      if (ret == IUP_IGNORE)
        return;
    }
  }

  ih->data->state = state;
  iExpanderUpdateStateImage(ih);

  if (child)
  {
    if (ih->data->state == IEXPANDER_CLOSE)
      IupSetAttribute(child, "VISIBLE", "NO");
    else
      IupSetAttribute(child, "VISIBLE", "YES");

    if (refresh)
      IupRefresh(child); /* this will recompute the layout of the hole dialog */
  }

  if (callcb)
  {
    IFn cb = IupGetCallback(ih, "ACTION");
    if (cb)
      cb(ih);
  }
}

static int iExpanderGetBarSize(Ihandle* ih)
{
  int bar_size;
  if (ih->data->bar_size == -1)
  {
    Ihandle* box = ih->firstchild->firstchild;

    iupBaseComputeNaturalSize(box);

    if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
      bar_size = box->naturalwidth;
    else
      bar_size = box->naturalheight;
  }
  else
    bar_size = ih->data->bar_size;

  return bar_size;
}

static void iExpanderUpdateTitle(Ihandle* ih)
{
  Ihandle* box = ih->firstchild->firstchild;
  Ihandle* expand_button = box->firstchild;
  Ihandle* label = expand_button->brother;
  char* title = iupAttribGetStr(ih, "TITLE");
  if (title)
  {
    IupSetAttribute(box, "MARGIN", "0x1");
    IupSetAttribute(box, "GAP", "1");
    IupSetStrAttribute(label, "VISIBLE", "Yes");
    IupSetStrAttribute(label, "TITLE", title);
    IupSetAttribute(expand_button, "EXPAND", "NO");
  }
  else
  {
    IupSetAttribute(box, "MARGIN", "2x2");
    IupSetAttribute(box, "GAP", "0");
    IupSetStrAttribute(label, "VISIBLE", "No");
    IupSetStrAttribute(label, "TITLE", NULL);
    IupSetAttribute(expand_button, "EXPAND", "HORIZONTAL");
  }

  if (ih->handle)
    iExpanderUpdateStateImage(ih);
}

static void iExpanderUpdateExtraButtonImage(Ihandle* ih, Ihandle* extra_button, int pressed)
{
  int button = iupAttribGetInt(extra_button, "EXTRABUTTON_NUMBER");
  char* image = iupAttribGetId(ih, "IMAGEEXTRA", button);
  if (!image)
    return;

  if (pressed)
  {
    char* impress = iupAttribGetId(ih, "IMAGEEXTRAPRESS", button);
    if (impress) image = impress;
  }
  else if (ih->data->highlight)
  {
    char* imhighlight = iupAttribGetId(ih, "IMAGEEXTRAHIGHLIGHT", button);
    if (imhighlight) image = imhighlight;
  }

  IupSetStrAttribute(extra_button, "IMAGE", image);
}

static int iExpanderExtraButtonButton_CB(Ihandle* extra_button, int button, int pressed, int x, int y, char* status);
static int iExpanderExtraButtonEnterWindow_cb(Ihandle* extra_button);
static int iExpanderExtraButtonLeaveWindow_cb(Ihandle* extra_button);

static void iExpanderAddExtraButton(Ihandle* ih, Ihandle* extra_box, int number)
{
  Ihandle* extra_button = IupLabel(NULL);
  IupSetInt(extra_button, "EXTRABUTTON_NUMBER", number);
  IupSetCallback(extra_button, "BUTTON_CB", (Icallback)iExpanderExtraButtonButton_CB);
  IupSetCallback(extra_button, "ENTERWINDOW_CB", (Icallback)iExpanderExtraButtonEnterWindow_cb);
  IupSetCallback(extra_button, "LEAVEWINDOW_CB", (Icallback)iExpanderExtraButtonLeaveWindow_cb);

  iExpanderUpdateExtraButtonImage(ih, extra_button, 0);

  IupAppend(extra_box, extra_button);
}

static void iExpanderUpdateExtraButtons(Ihandle* ih)
{
  Ihandle* box = ih->firstchild->firstchild;
  Ihandle* extra_box = box->firstchild->brother->brother;  /* (expand_button, label, extra_box) */

  if (extra_box)
    IupDestroy(extra_box);

  if (ih->data->extra_buttons)
  {
    extra_box = IupHbox(NULL);
    IupSetAttribute(box, "MARGIN", "0x0");
    IupSetAttribute(box, "GAP", "2");
    IupAppend(box, extra_box);

    iExpanderAddExtraButton(ih, extra_box, ih->data->extra_buttons);
    if (ih->data->extra_buttons > 1)
      iExpanderAddExtraButton(ih, extra_box, ih->data->extra_buttons-1);
    if (ih->data->extra_buttons > 2)
      iExpanderAddExtraButton(ih, extra_box, ih->data->extra_buttons-2);
  }
}

static int iExpanderExpandButtonButton_CB(Ihandle* expand_button, int button, int pressed, int x, int y, char* status);
static int iExpanderExpandButtonEnterWindow_cb(Ihandle* expand_button);
static int iExpanderExpandButtonLeaveWindow_cb(Ihandle* expand_button);

static void iExpanderUpdateBox(Ihandle* ih)
{
  Ihandle* bar = ih->firstchild;
  Ihandle* box, *expand_button;

  if (bar->firstchild)
    IupDestroy(bar->firstchild);

  expand_button = IupLabel(NULL);
  IupSetAttribute(expand_button, "ALIGNMENT", "ACENTER:ACENTER");
  IupSetCallback(expand_button, "BUTTON_CB", (Icallback)iExpanderExpandButtonButton_CB);
  IupSetCallback(expand_button, "ENTERWINDOW_CB", (Icallback)iExpanderExpandButtonEnterWindow_cb);
  IupSetCallback(expand_button, "LEAVEWINDOW_CB", (Icallback)iExpanderExpandButtonLeaveWindow_cb);

  if (ih->data->position == IEXPANDER_TOP)
  {
    box = IupHbox(expand_button, IupLabel(NULL), NULL);
  }
  else if (ih->data->position == IEXPANDER_BOTTOM)
  {
    box = IupHbox(expand_button, NULL);
    IupSetAttribute(expand_button, "EXPAND", "HORIZONTAL");
  }
  else
  {
    box = IupVbox(expand_button, NULL);
    IupSetAttribute(expand_button, "EXPAND", "VERTICAL");
  }

  IupSetAttribute(box, "MARGIN", "2x2");
  IupSetAttribute(box, "GAP", "0");
  IupSetAttribute(box, "ALIGNMENT", "ACENTER");

  IupAppend(bar, box);

  if (ih->data->position == IEXPANDER_TOP)
    iExpanderUpdateTitle(ih);

  iExpanderUpdateStateImage(ih);
}


/*****************************************************************************\
|* Internal Callbacks                                                        *|
\*****************************************************************************/


static int iExpanderGlobalMotion_cb(int x, int y)
{
  int child_x, child_y;
  Ihandle* ih = (Ihandle*)IupGetGlobal("_IUP_EXPANDER_GLOBAL");
  Ihandle* bar = ih->firstchild;
  Ihandle* child = bar->brother;

  if (ih->data->state != IEXPANDER_OPEN_FLOAT)
  {
    IupSetGlobal("_IUP_EXPANDER_GLOBAL", NULL);
    IupSetFunction("GLOBALMOTION_CB", IupGetFunction("_IUP_OLD_GLOBALMOTION_CB"));
    IupSetFunction("_IUP_OLD_GLOBALMOTION_CB", NULL);
    IupSetGlobal("INPUTCALLBACKS", "No");
    return IUP_DEFAULT;
  }

  child_x = 0, child_y = 0;
  iupdrvClientToScreen(bar, &child_x, &child_y);
  if (x > child_x && x < child_x + bar->currentwidth &&
      y > child_y && y < child_y + bar->currentheight)
    return IUP_DEFAULT;  /* ignore if inside the bar */

  child_x = 0, child_y = 0;
  iupdrvClientToScreen(child, &child_x, &child_y);
  if (x < child_x || x > child_x+child->currentwidth ||
      y < child_y || y > child_y+child->currentheight)
  {
    iExpanderOpenCloseChild(ih, 0, 1, IEXPANDER_CLOSE);

    IupSetGlobal("_IUP_EXPANDER_GLOBAL", NULL);
    IupSetFunction("GLOBALMOTION_CB", IupGetFunction("_IUP_OLD_GLOBALMOTION_CB"));
    IupSetFunction("_IUP_OLD_GLOBALMOTION_CB", NULL);
    IupSetGlobal("INPUTCALLBACKS", "No");
  }

  return IUP_DEFAULT;
}

static int iExpanderTimer_cb(Ihandle* timer)
{
  Ihandle* ih = (Ihandle*)iupAttribGet(timer, "_IUP_EXPANDER");
  Ihandle* child = ih->firstchild->brother;

  /* run timer just once each time */
  IupSetAttribute(timer, "RUN", "No");

  /* just show child on top,
     that's why child must be a native container when using autoshow. */
  iExpanderOpenCloseChild(ih, 0, 1, IEXPANDER_OPEN_FLOAT);
  IupRefreshChildren(ih);
  IupSetAttribute(child, "ZORDER", "TOP"); 

  /* now monitor mouse move */
  IupSetGlobal("INPUTCALLBACKS", "Yes");
  IupSetFunction("_IUP_OLD_GLOBALMOTION_CB", IupGetFunction("GLOBALMOTION_CB"));
  IupSetGlobal("_IUP_EXPANDER_GLOBAL", (char*)ih);
  IupSetFunction("GLOBALMOTION_CB", (Icallback)iExpanderGlobalMotion_cb);
  return IUP_DEFAULT;
}

static int iExpanderExpandButtonLeaveWindow_cb(Ihandle* expand_button)
{
  /* expander -> bar -> box -> (expand_button, ...) */
  Ihandle* ih = IupGetParent(IupGetParent(IupGetParent(expand_button)));

  if (ih->data->highlight)
  {
    ih->data->highlight = 0;

    iExpanderUpdateStateImage(ih);

    if (ih->data->auto_show)
    {
      if (IupGetInt(ih->data->timer, "RUN"))
        IupSetAttribute(ih->data->timer, "RUN", "No");
    }
  }
  return IUP_DEFAULT;
}

static int iExpanderExpandButtonEnterWindow_cb(Ihandle* expand_button)
{
  /* expander -> bar -> box -> (expand_button, ...) */
  Ihandle* ih = IupGetParent(IupGetParent(IupGetParent(expand_button)));

  if (!ih->data->highlight)
  {
    Ihandle* child = ih->firstchild->brother;

    ih->data->highlight = 1;

    iExpanderUpdateStateImage(ih);

    if (ih->data->auto_show &&
        child &&
        ih->data->state == IEXPANDER_CLOSE)
      IupSetAttribute(ih->data->timer, "RUN", "Yes");
  }
  return IUP_DEFAULT;
}

static int iExpanderExpandButtonButton_CB(Ihandle* expand_button, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1 && pressed)
  {
    /* expander -> bar -> box -> (expand_button, ...) */
    Ihandle* ih = IupGetParent(IupGetParent(IupGetParent(expand_button)));

    if (ih->data->auto_show)
    {
      if (IupGetInt(ih->data->timer, "RUN"))
        IupSetAttribute(ih->data->timer, "RUN", "No");
    }

    iExpanderOpenCloseChild(ih, 1, 1, ih->data->state == IEXPANDER_OPEN ? IEXPANDER_CLOSE : IEXPANDER_OPEN);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iExpanderExtraButtonLeaveWindow_cb(Ihandle* extra_button)
{
  /* expander -> bar -> box -> (expand_button, label, extra_box(extra_button)) */
  Ihandle* ih = IupGetParent(IupGetParent(IupGetParent(IupGetParent(extra_button))));

  if (ih->data->highlight)
  {
    ih->data->highlight = 0;

    iExpanderUpdateExtraButtonImage(ih, extra_button, 0);
  }
  return IUP_DEFAULT;
}

static int iExpanderExtraButtonEnterWindow_cb(Ihandle* extra_button)
{
  /* expander -> bar -> box -> (expand_button, label, extra_box(extra_button)) */
  Ihandle* ih = IupGetParent(IupGetParent(IupGetParent(IupGetParent(extra_button))));

  if (!ih->data->highlight)
  {
    ih->data->highlight = 1;

    iExpanderUpdateExtraButtonImage(ih, extra_button, 0);
  }
  return IUP_DEFAULT;
}

static int iExpanderExtraButtonButton_CB(Ihandle* extra_button, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1)
  {
    /* expander -> bar -> box -> (expand_button, label, extra_box(extra_button)) */
    Ihandle* ih = IupGetParent(IupGetParent(IupGetParent(IupGetParent(extra_button))));

    IFnii cb = (IFnii)IupGetCallback(ih, "EXTRABUTTON_CB");
    if (cb)
    {
      button = IupGetInt(extra_button, "EXTRABUTTON_NUMBER");
      cb(ih, button, pressed);
    }

    iExpanderUpdateExtraButtonImage(ih, extra_button, pressed);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}


/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/


static char* iExpanderGetClientSizeAttrib(Ihandle* ih)
{
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    width -= bar_size;
  else
    height -= bar_size;

  if (width < 0) width = 0;
  if (height < 0) height = 0;
  return iupStrReturnIntInt(width, height, 'x');
}

static int iExpanderSetPositionAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle)
    return 0; /* can be changed only before map */

  if (iupStrEqualNoCase(value, "LEFT"))
    ih->data->position = IEXPANDER_LEFT;
  else if (iupStrEqualNoCase(value, "RIGHT"))
    ih->data->position = IEXPANDER_RIGHT;
  else if (iupStrEqualNoCase(value, "BOTTOM"))
    ih->data->position = IEXPANDER_BOTTOM;
  else  /* Default = TOP */
    ih->data->position = IEXPANDER_TOP;

  iExpanderUpdateBox(ih);

  return 0;  /* do not store value in hash table */
}

static char* iExpanderGetPositionAttrib(Ihandle* ih)
{
  if (ih->data->position == IEXPANDER_LEFT)
    return "LEFT";
  else if (ih->data->position == IEXPANDER_RIGHT)
    return "RIGHT";
  else if (ih->data->position == IEXPANDER_BOTTOM)
    return "BOTTOM";
  else  /* Default = TOP */
    return "TOP";
}

static int iExpanderSetBarSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    ih->data->bar_size = -1;
  else
    iupStrToInt(value, &ih->data->bar_size);  /* must manually update layout */
  return 0; /* do not store value in hash table */
}

static char* iExpanderGetBarSizeAttrib(Ihandle* ih)
{
  int bar_size = iExpanderGetBarSize(ih);
  return iupStrReturnInt(bar_size);
}

static int iExpanderSetStateAttrib(Ihandle* ih, const char* value)
{
  int state;
  if (iupStrEqualNoCase(value, "OPEN"))
    state = IEXPANDER_OPEN;
  else
    state = IEXPANDER_CLOSE;

  if (ih->data->state == state)
    return 0;

  iExpanderOpenCloseChild(ih, 1, 0, state);

  return 0; /* do not store value in hash table */
}

static char* iExpanderGetStateAttrib(Ihandle* ih)
{
  if (ih->data->state)
    return "OPEN";
  else
    return "CLOSE";
}

static int iExpanderSetForeColorAttrib(Ihandle* ih, const char* value)
{
  Ihandle* bar = ih->firstchild;
  IupSetStrAttribute(bar, "FGCOLOR", value);
  return 1;
}

static int iExpanderSetBackColorAttrib(Ihandle* ih, const char* value)
{
  Ihandle* bar = ih->firstchild;
  IupSetStrAttribute(bar, "BGCOLOR", value);
  return 1;
}

static int iExpanderSetImageAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->state == IEXPANDER_CLOSE)
  {
    Ihandle* box = ih->firstchild->firstchild;
    Ihandle* expand_button = box->firstchild;
    IupSetStrAttribute(expand_button, "IMAGE", value);
  }
  return 1;
}

static int iExpanderSetImageOpenAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->state != IEXPANDER_CLOSE)
  {
    Ihandle* box = ih->firstchild->firstchild;
    Ihandle* expand_button = box->firstchild;
    IupSetStrAttribute(expand_button, "IMAGE", value);
  }
  return 1;
}

static int iExpanderSetImageExtra(Ihandle* ih, const char* value, int button)
{
  if (ih->data->extra_buttons > button - 1)
  {
    Ihandle* box = ih->firstchild->firstchild;
    Ihandle* extra_box = box->firstchild->brother->brother;  /* (expand_button, label, extra_box) */

    if (extra_box)
    {
      Ihandle* extra_button = IupGetChild(extra_box, ih->data->extra_buttons - button);
      iupAttribSetStrId(ih, "IMAGEEXTRA", button, value);
      iExpanderUpdateExtraButtonImage(ih, extra_button, 0);
    }
  }

  return 1;
}

static int iExpanderSetImageExtraPress(Ihandle* ih, const char* value, int button)
{
  if (ih->data->extra_buttons > button - 1)
  {
    Ihandle* box = ih->firstchild->firstchild;
    Ihandle* extra_box = box->firstchild->brother->brother;  /* (expand_button, label, extra_box) */

    if (extra_box)
    {
      Ihandle* extra_button = IupGetChild(extra_box, ih->data->extra_buttons - button);
      iupAttribSetStrId(ih, "IMAGEEXTRAPRESS", button, value);
      iExpanderUpdateExtraButtonImage(ih, extra_button, 0);
    }
  }

  return 1;
}

static int iExpanderSetImageExtraHighlight(Ihandle* ih, const char* value, int button)
{
  if (ih->data->extra_buttons > button - 1)
  {
    Ihandle* box = ih->firstchild->firstchild;
    Ihandle* extra_box = box->firstchild->brother->brother;  /* (expand_button, label, extra_box) */

    if (extra_box)
    {
      Ihandle* extra_button = IupGetChild(extra_box, ih->data->extra_buttons - button);
      iupAttribSetStrId(ih, "IMAGEEXTRAHIGHLIGHT", button, value);
      iExpanderUpdateExtraButtonImage(ih, extra_button, 0);
    }
  }

  return 1;
}

static int iExpanderSetImageExtra1Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtra(ih, value, 1);
}

static int iExpanderSetImageExtraPress1Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtraPress(ih, value, 1);
}

static int iExpanderSetImageExtraHighlight1Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtraHighlight(ih, value, 1);
}

static int iExpanderSetImageExtra2Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtra(ih, value, 2);
}

static int iExpanderSetImageExtraPress2Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtraPress(ih, value, 2);
}

static int iExpanderSetImageExtraHighlight2Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtraHighlight(ih, value, 2);
}

static int iExpanderSetImageExtra3Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtra(ih, value, 3);
}

static int iExpanderSetImageExtraPress3Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtraPress(ih, value, 3);
}

static int iExpanderSetImageExtraHighlight3Attrib(Ihandle* ih, const char* value)
{
  return iExpanderSetImageExtraHighlight(ih, value, 3);
}

static int iExpanderSetTitleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->position == IEXPANDER_TOP)
  {
    iupAttribSetStr(ih, "TITLE", value);
    iExpanderUpdateTitle(ih);
  }

  return 1;
}

static int iExpanderSetAutoShowAttrib(Ihandle* ih, const char* value)
{
  ih->data->auto_show = iupStrBoolean(value);
  if (ih->data->auto_show)
  {
    if (!ih->data->timer)
    {
      ih->data->timer = IupTimer();
      IupSetAttribute(ih->data->timer, "TIME", "1000");  /* 1 second */
      IupSetCallback(ih->data->timer, "ACTION_CB", iExpanderTimer_cb);
      iupAttribSet(ih->data->timer, "_IUP_EXPANDER", (char*)ih);  /* 1 second */
    }
  }
  else
  {
    if (ih->data->timer)
      IupSetAttribute(ih->data->timer, "RUN", "NO");
  }
  return 0; /* do not store value in hash table */
}

static char* iExpanderGetAutoShowAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->auto_show);
}

static int iExpanderSetExtraButtonsAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle)
    return 0; /* can be changed only before map */

  if (!value)
    ih->data->extra_buttons = 0;
  else
  {
    iupStrToInt(value, &(ih->data->extra_buttons));
    if (ih->data->extra_buttons < 0)
      ih->data->extra_buttons = 0;
    else if (ih->data->extra_buttons > 3)
      ih->data->extra_buttons = 3;
  }

  iExpanderUpdateExtraButtons(ih);

  return 0; /* do not store value in hash table */
}

static char* iExpanderGetExtraButtonsAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->extra_buttons);
}


/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/


static void iExpanderComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int child_expand = 0,
      natural_w, natural_h;
  Ihandle* bar = ih->firstchild;
  Ihandle* child = bar->brother;
  Ihandle* box = bar->firstchild;
  int bar_size = iExpanderGetBarSize(ih);

  iupBaseComputeNaturalSize(box);

  /* bar */
  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    natural_w = bar_size;
    natural_h = box->naturalheight;
  }
  else
  {
    natural_w = box->naturalwidth;
    natural_h = bar_size;
  }

  if (child)
  {
    /* update child natural bar_size first */
    iupBaseComputeNaturalSize(child);

    if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    {
      if (ih->data->state == IEXPANDER_OPEN)  /* only open, not float */
        natural_w += child->naturalwidth;
      natural_h = iupMAX(natural_h, child->naturalheight);
    }
    else
    {
      natural_w = iupMAX(natural_w, child->naturalwidth);
      if (ih->data->state == IEXPANDER_OPEN)  /* only open, not float */
        natural_h += child->naturalheight;
    }

    if (ih->data->state == IEXPANDER_OPEN)
      child_expand = child->expand;
    else
    {
      if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
        child_expand = child->expand & IUP_EXPAND_HEIGHT;  /* only vertical allowed */
      else
        child_expand = child->expand & IUP_EXPAND_WIDTH;  /* only horizontal allowed */
    }
  }

  *children_expand = child_expand;
  *w = natural_w;
  *h = natural_h;
}

static void iExpanderSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  Ihandle* bar = ih->firstchild;
  Ihandle* child = bar->brother;
  Ihandle* box = bar->firstchild;
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    bar->currentwidth = bar_size;
    bar->currentheight = ih->currentheight;

    if (ih->currentwidth < bar_size)
      ih->currentwidth = bar_size;

    width = ih->currentwidth - bar_size;
  }
  else /* IEXPANDER_TOP OR IEXPANDER_BOTTOM */
  {
    bar->currentwidth = ih->currentwidth;
    bar->currentheight = bar_size;

    if (ih->currentheight < bar_size)
      ih->currentheight = bar_size;

    height = ih->currentheight - bar_size;
  }

  iupBaseSetCurrentSize(box, bar->currentwidth, bar->currentheight, shrink);

  if (child)
  {
    if (ih->data->state == IEXPANDER_OPEN)
      iupBaseSetCurrentSize(child, width, height, shrink);
    else if (ih->data->state == IEXPANDER_OPEN_FLOAT)  /* simply set to the natural size */
      iupBaseSetCurrentSize(child, child->currentwidth, child->currentheight, shrink);
  }
}

static void iExpanderSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  Ihandle* bar = ih->firstchild;
  Ihandle* child = bar->brother;
  int bar_size = iExpanderGetBarSize(ih);

  /* always position bar */
  if (ih->data->position == IEXPANDER_LEFT)
  {
    iupBaseSetPosition(bar, x, y);
    x += bar_size;
  }
  else if (ih->data->position == IEXPANDER_RIGHT)
    iupBaseSetPosition(bar, x + ih->currentwidth - bar_size, y);
  else if (ih->data->position == IEXPANDER_BOTTOM)
    iupBaseSetPosition(bar, x, y + ih->currentheight - bar_size);
  else /* IEXPANDER_TOP */
  {
    iupBaseSetPosition(bar, x, y);
    y += bar_size;
  }

  if (child)
  {
    if (ih->data->state == IEXPANDER_OPEN)
      iupBaseSetPosition(child, x, y);
    else if (ih->data->state == IEXPANDER_OPEN_FLOAT)
    {
      if (ih->data->position == IEXPANDER_RIGHT)
        x -= child->currentwidth;
      else if (ih->data->position == IEXPANDER_BOTTOM)
        y -= child->currentheight;

      iupBaseSetPosition(child, x, y);
    }
  }
}

static void iExpanderChildAddedMethod(Ihandle* ih, Ihandle* child)
{
  iExpanderOpenCloseChild(ih, 0, 0, ih->data->state);
  (void)child;
}

static int iExpanderCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->position = IEXPANDER_TOP;
  ih->data->state = IEXPANDER_OPEN;
  ih->data->bar_size = -1;

  bar = IupBackgroundBox(NULL);
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */
  bar->flags |= IUP_INTERNAL;

  iExpanderUpdateBox(ih);

  IupSetAttribute(bar, "CANFOCUS", "NO");
  IupSetAttribute(bar, "BORDER", "NO");

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  return IUP_NOERROR;
}

static void iExpanderDestroyMethod(Ihandle* ih)
{
  if (ih->data->timer)
    IupDestroy(ih->data->timer);
}

Iclass* iupExpanderNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "expander";
  ic->format = "h";   /* one ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY+2;  /* canvas+child */
  ic->is_interactive = 0;

  /* Class functions */
  ic->New     = iupExpanderNewClass;
  ic->Create  = iExpanderCreateMethod;
  ic->Destroy = iExpanderDestroyMethod;
  ic->Map     = iupBaseTypeVoidMapMethod;
  ic->ChildAdded = iExpanderChildAddedMethod;

  ic->ComputeNaturalSize     = iExpanderComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iExpanderSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition    = iExpanderSetChildrenPositionMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "ACTION", "");
  iupClassRegisterCallback(ic, "OPENCLOSE_CB", "i");
  iupClassRegisterCallback(ic, "EXTRABUTTON_CB", "ii");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iExpanderGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);

  /* IupExpander only */
  iupClassRegisterAttribute(ic, "BARPOSITION", iExpanderGetPositionAttrib, iExpanderSetPositionAttrib, IUPAF_SAMEASSYSTEM, "TOP", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BARSIZE", iExpanderGetBarSizeAttrib, iExpanderSetBarSizeAttrib, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STATE", iExpanderGetStateAttrib, iExpanderSetStateAttrib, IUPAF_SAMEASSYSTEM, "OPEN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FORECOLOR", NULL, iExpanderSetForeColorAttrib, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKCOLOR", NULL, iExpanderSetBackColorAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, iExpanderSetTitleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AUTOSHOW", iExpanderGetAutoShowAttrib, iExpanderSetAutoShowAttrib, IUPAF_SAMEASSYSTEM, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXTRABUTTONS", iExpanderGetExtraButtonsAttrib, iExpanderSetExtraButtonsAttrib, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGE", NULL, iExpanderSetImageAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEOPEN", NULL, iExpanderSetImageOpenAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEOPENHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGEEXTRA1", NULL, iExpanderSetImageExtra1Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAPRESS1", NULL, iExpanderSetImageExtraPress1Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAHIGHLIGHT1", NULL, iExpanderSetImageExtraHighlight1Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRA2", NULL, iExpanderSetImageExtra2Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAPRESS2", NULL, iExpanderSetImageExtraPress2Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAHIGHLIGHT2", NULL, iExpanderSetImageExtraHighlight2Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRA3", NULL, iExpanderSetImageExtra3Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAPRESS3", NULL, iExpanderSetImageExtraPress3Attrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEEXTRAHIGHLIGHT3", NULL, iExpanderSetImageExtraHighlight3Attrib, NULL, NULL, IUPAF_NO_INHERIT);

  if (!IupGetHandle("IupArrowUp") || !IupGetHandle("IupArrowDown"))
    iExpanderLoadImages();

  return ic;
}

Ihandle* IupExpander(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("expander", children);
}
