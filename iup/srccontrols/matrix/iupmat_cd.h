/** \file
 * \brief iupmatrix. CD help macros.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_CD_H 
#define __IUPMAT_CD_H

#include <cd.h>
#include <cdiup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INVY(y) (ih->data->YmaxC - (y))

#define IsCanvasSet(ih,err)  do {                         \
                               if(ih->data->cddbuffer)    \
                                 err = CD_OK;             \
                               else                       \
                                 err = CD_ERROR;          \
                             } while(0)

#define CdLine(x1,y1,x2,y2)  cdCanvasLine(ih->data->cddbuffer,x1,INVY(y1),x2,INVY(y2))

#define CdVertex(x,y)        cdCanvasVertex(ih->data->cddbuffer,x,INVY(y))

#define CdBox(xmin,xmax,ymin,ymax)                               \
  cdCanvasBox(ih->data->cddbuffer,xmin < xmax ? xmin : xmax,     \
              xmin < xmax ? xmax : xmin,                         \
              INVY(ymin) < INVY(ymax) ? INVY(ymin) : INVY(ymax), \
              INVY(ymin) < INVY(ymax) ? INVY(ymax) : INVY(ymin))

#define CdRect(xmin,xmax,ymin,ymax)                              \
  cdCanvasRect(ih->data->cddbuffer,xmin < xmax ? xmin : xmax,    \
              xmin < xmax ? xmax : xmin,                         \
              INVY(ymin) < INVY(ymax) ? INVY(ymin) : INVY(ymax), \
              INVY(ymin) < INVY(ymax) ? INVY(ymax) : INVY(ymin))

#define CdClipArea(x1,x2,y1,y2)  cdCanvasClipArea(ih->data->cddbuffer,x1,x2,INVY(y2),INVY(y1));

#define CdPutText(x,y,s,m)       do {                                              \
                                   cdCanvasTextAlignment(ih->data->cddbuffer,m);   \
                                   cdCanvasText(ih->data->cddbuffer,x,INVY(y),s);  \
                                  } while(0)

#define CdRestoreBgColor()        cdCanvasForeground(ih->data->cddbuffer,oldbgc);

void iupMatrixCDSetCdFrameColor(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
