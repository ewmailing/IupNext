/** \file
 * \brief IUP CPI pre-defined controls.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ICPI_H 
#define __ICPI_H

#ifdef __cplusplus
extern "C" {
#endif

void IupTimerOpen(void);
void IupTimerClose(void);
void IupFileDlgOpen(void);
void IupSpinOpen(void);
void IupSpinClose(void);
void IupCboxOpen(void);
void IupSboxOpen(void);

/* declared in <iup.h>
Ihandle *IupFileDlg(void);
Ihandle *IupTimer(void);
*/

#ifdef __cplusplus
}
#endif

#endif
