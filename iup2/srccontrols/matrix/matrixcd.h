/** \file
 * \brief iupmatrix. CD help macros.
 *
 * See Copyright Notice in iup.h
 * $Id: matrixcd.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __MATRIXCD_H 
#define __MATRIXCD_H

#include <cd.h>
#include <cdiup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INVY(y) (YmaxCanvas(mat) - (y))

#define cdcv(mat) (mat)->cddbuffer

#define IsCanvasSet(mat,err)  do {if((mat)->cddbuffer) \
                                  err = CD_OK; \
                                else                            \
                                  err = CD_ERROR;               \
                              } while(0)

#define CdLine(x1,y1,x2,y2)         cdCanvasLine(mat->cddbuffer,x1,INVY(y1),x2,INVY(y2))
#define CdVertex(x,y)               cdCanvasVertex(mat->cddbuffer,x,INVY(y))
#define CdBox(xmin,xmax,ymin,ymax) \
	cdCanvasBox(mat->cddbuffer,xmin < xmax ? xmin : xmax, \
      	      xmin < xmax ? xmax : xmin,\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymin) : INVY(ymax),\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymax) : INVY(ymin))
#define CdRect(xmin,xmax,ymin,ymax) \
	cdCanvasRect(mat->cddbuffer,xmin < xmax ? xmin : xmax, \
      	      xmin < xmax ? xmax : xmin,\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymin) : INVY(ymax),\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymax) : INVY(ymin))
#define CdClipArea(x1,x2,y1,y2)     cdCanvasClipArea(mat->cddbuffer,x1,x2,INVY(y2),INVY(y1));

#define CdPutText(x,y,s,m)       do {cdCanvasTextAlignment(mat->cddbuffer,m);       \
                                   cdCanvasText(mat->cddbuffer,x,INVY(y),s);      \
                                  } while(0)

#define CdRestoreBgColor()  cdCanvasForeground(mat->cddbuffer,oldbgc);

void iupmatSetCdFrameColor(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
