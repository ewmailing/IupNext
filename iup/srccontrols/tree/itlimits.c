/** \file
 * \brief iuptree control
 * Functions used to verify limits between specified canvas regions
 *
 * See Copyright Notice in iup.h
 */

#include <cd.h>

#include "itlimits.h"


/* Determines if a pixel is inside a region
   c   : canvas 
   x   : pixel horizontal coordinate
   y   : pixel vertical coordinate
   x0  : region left border
   y0  : region top border
   w   : region width
   h   : region height
*/
int iTreeLimitsInsideRegion(cdCanvas* c, int x, int y, int x0, int y0, int w, int h )
{
  cdCanvasUpdateYAxis(c, &y);
  y++;
  
  return (x >= x0 && x <= x0 + w && y >= y0 && y <= y0 + h);
}
