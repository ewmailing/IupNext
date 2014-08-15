#ifndef __IUP_EXT_H 
#define __IUP_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

/* unofficial, added by fabraham. used on IUP3wrapper mechanism */

int       IupLoopStepWait  (void); 

void      IupSetExternalDialogCountFunc (int (*func)());                      
void      IupSetExternalIdleFunc (int (*func)());                             
void      IupSetExternalModalDialogEnterFunc (void (*func)(int popup_level)); 
void      IupSetExternalModalDialogLeaveFunc (void (*func)(int popup_level)); 

void      IupModalDialogEnter (Ihandle* ih, int popup_level);                 
void      IupModalDialogLeave (int popup_level);                              

#ifdef __cplusplus
}
#endif
