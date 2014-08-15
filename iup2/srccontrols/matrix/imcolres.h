/** \file
 * \brief iupmatrix column resize.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMCOLRES_H 
#define __IMCOLRES_H

#ifdef __cplusplus
extern "C" {
#endif

int  iupmatColresTry         (Ihandle *h, int x, int y);
void iupmatColresFinish      (Ihandle *h, int x);
void iupmatColresMove        (Ihandle *h, int x);
void iupmatColresChangeCursor(Ihandle *h, int x, int y);
int  iupmatColresResizing    (void);

void  iupmatColresSet (Ihandle *h, char* value, int col, int m, int pixels);
char *iupmatColresGet (Ihandle *h, int col, int m, int pixels);

#ifdef __cplusplus
}
#endif

#endif
