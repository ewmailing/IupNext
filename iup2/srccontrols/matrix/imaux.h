/** \file
 * \brief iupmatrix control
 * auxiliary functions.
 *
 * See Copyright Notice in iup.h
 * $Id: imaux.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMAUX_H 
#define __IMAUX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*Iitemmousemove)    (Ihandle *, int, int);
typedef int (*Iitemrightclickcb)    (Ihandle *, int, int, char *);

int  iupmatIsCellVisible(Ihandle *h, int lin, int col);
int  iupmatGetCellDim   (Ihandle *h, int lin, int col, int *x, int *y,
                         int *dx, int *dy);

void  iupmatGetPos            (Ihandle *h, int m);
void  iupmatGetLastWidth      (Ihandle *h, int m);

int   iupmatGetColumnWidth    (Ihandle *h, int col);
int   iupmatGetLineHeight     (Ihandle *h, int lin);
void  iupmatFillWidthVec      (Ihandle *h);
void  iupmatFillHeightVec     (Ihandle *h);

int   iupmatGetTitlelineSize  (Ihandle *h);
int   iupmatGetTitlecolumnSize(Ihandle *h);
int   iupmatGetLineCol        (Ihandle *h, int x, int y, int *l, int *c);
void  iupmatUpdateCellValue   (Ihandle *h);
char* iupmatGetCellValue      (Ihandle *h, int lin, int col);

int   iupmatCallLeavecellCb   (Ihandle *h);
void  iupmatCallEntercellCb   (Ihandle *h);
int   iupmatCallEditionCbLinCol(Ihandle *h, int lin, int col, int modo);

int   iupmatTextHeight        (Ihandle *h, char *text, int *totalh, int *lineh, int *spacing);
void  iupmatTextWidth         (Ihandle *h, char *text, int *width);

#ifdef __cplusplus
}
#endif

#endif
