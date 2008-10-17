/** \file
 * \brief Keyboard Focus navigation
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUP_FOCUS_H 
#define __IUP_FOCUS_H

#ifdef __cplusplus
extern "C" {
#endif


/** \defgroup focus Keyboard Focus
 * \par
 * See \ref iup_focus.h
 * \ingroup cpi */


/** Utility to check if a control can have the keyboard input focus.
 * Checks the control class, active state and visible.
 * \ingroup focus */
int iupFocusCanAccept(Ihandle *ih);

/** Call GETFOCUS_CB and FOCUS_CB.
 * \ingroup focus */
void iupCallGetFocusCb(Ihandle *ih);

/** Call KILLFOCUS_CB and FOCUS_CB.
 * \ingroup focus */
void iupCallKillFocusCb(Ihandle *ih);

/** Returns the next element that should receive the focus.
 * \ingroup focus */
Ihandle* iupGetNextFocus(Ihandle *ih);

/** Returns the next element that could receive the focus.
 * \ingroup focus */
Ihandle* iupFocusNextInteractive(Ihandle *ih);


/* Other functions declared in <iup.h> and implemented here. 
IupPreviousField
IupNextField
*/


#ifdef __cplusplus
}
#endif

#endif
