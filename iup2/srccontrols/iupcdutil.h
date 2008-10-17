/** \file
 * \brief iupcdutil. CD and IUP utilities for the IupControls
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __CDIUPUTIL_H 
#define __CDIUPUTIL_H 

#ifdef __cplusplus
extern "C" {
#endif

#include <cd.h>

void cdIupCalcShadows(long bgcolor, long *light_shadow, long *mid_shadow, long *dark_shadow);
long cdIupConvertColor(char *color);
void cdIupDrawSunkenRect(cdCanvas *c, int x1, int y1, int x2, int y2,  
                         long light_shadow, long mid_shadow, long dark_shadow);
void cdIupDrawRaisenRect(cdCanvas *c, int x1, int y1, int x2, int y2,  
                         long light_shadow, long mid_shadow, long dark_shadow);
void cdIupDrawVertSunkenMark(cdCanvas *c, int x, int y1, int y2, long light_shadow, long dark_shadow);
void cdIupDrawHorizSunkenMark(cdCanvas *c, int x1, int x2, int y, long light_shadow, long dark_shadow);
void cdIupDrawFocusRect(Ihandle* h, cdCanvas *c, int x1, int y1, int x2, int y2);

#define cdIupInvertYAxis(_y, _h) ((_h) - (_y) - 1);


#ifdef __cplusplus
}
#endif

#endif
