/** \file
 * \brief iupmatrix. CD help macros.
 *
 * See Copyright Notice in iup.h
 * $Id: matrixcd.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */
 
#ifndef __MATRIXCD_H 
#define __MATRIXCD_H

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

void iMatrixSetCdFrameColor(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
