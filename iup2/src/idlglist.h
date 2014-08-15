/** \file
 * \brief list of all created dialogs
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IDLGLIST_H 
#define __IDLGLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Adds a dialog to the list. Used only in IupDialog.
 */
void iupDlgListAdd(Ihandle *ih);

/* Removes a dialog from the list. Used only in IupDestroy.
 */
void iupDlgListDelete(Ihandle *ih);

/** Starts a loop for all the created dialogs. 
 */
Ihandle* iupDlgListFirst(void);

/** Retrieve the next dialog on the list. Must call iupDlgListFirst first.
 */
Ihandle* iupDlgListNext(void);

#ifdef __cplusplus
}
#endif

#endif
