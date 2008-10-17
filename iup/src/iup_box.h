/** \file
 * \brief Base for box Controls.
 *
 * See Copyright Notice in iup.h
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


#ifdef __cplusplus
}
#endif

#endif
