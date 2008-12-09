/** \file
 * \brief Base for box Controls.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_BOX_H 
#define __IUP_BOX_H

#ifdef __cplusplus
extern "C" {
#endif


struct _IcontrolData 
{
  int alignment,
      margin_x,
      margin_y,
      gap;
};

Iclass* iupBoxClassBase(void);

/* Implemented in iup_normalizer.c */
void iupNormalizeSizeBoxChild(Ihandle *ih, int children_natural_maxwidth, int children_natural_maxheight);


#ifdef __cplusplus
}
#endif

#endif
