/** \file
 * \brief iupmask functions.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPMASK_H 
#define __IUPMASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* pre-defined masks */
#define IUP_MASK_FLOAT    "[+/-]?(/d+/.?/d*|/./d+)"
#define IUP_MASK_UFLOAT   "(/d+/.?/d*|/./d+)"
#define IUP_MASK_EFLOAT   "[+/-]?(/d+/.?/d*|/./d+)([eE][+/-]?/d+)?"
#define IUP_MASK_INT      "[+/-]?/d+"
#define IUP_MASK_UINT     "/d+"
#define IUPMASK_FLOAT     IUP_MASK_FLOAT
#define IUPMASK_UFLOAT    IUP_MASK_UFLOAT
#define IUPMASK_EFLOAT    IUP_MASK_EFLOAT
#define IUPMASK_INT	      IUP_MASK_INT
#define IUPMASK_UINT      IUP_MASK_UINT


int  iupmaskSet     (Ihandle *h, const char *mask, int autofill, int casei);
int  iupmaskSetInt  (Ihandle *h, int autofill, int   min , int   max);
int  iupmaskSetFloat(Ihandle *h, int autofill, float min , float max);
void iupmaskRemove   (Ihandle *h);

int  iupmaskCheck   (Ihandle *h);

int  iupmaskGet      (Ihandle *h, char   **val);
int  iupmaskGetFloat (Ihandle *h, float  *fval);
int  iupmaskGetDouble(Ihandle *h, double *dval);
int  iupmaskGetInt   (Ihandle *h, int    *ival);



int  iupmaskMatSet     (Ihandle *h, const char *mask, int autofill, int casei, int lin, int col);
int  iupmaskMatSetInt  (Ihandle *h, int autofill, int   min, int   max, int lin, int col);
int  iupmaskMatSetFloat(Ihandle *h, int autofill, float min, float max, int lin, int col);
void iupmaskMatRemove  (Ihandle *h, int lin, int col);

int  iupmaskMatCheck   (Ihandle *h, int lin, int col);

int  iupmaskMatGet      (Ihandle *h, char   **val, int lin, int col);
int  iupmaskMatGetFloat (Ihandle *h, float  *fval, int lin, int col);
int  iupmaskMatGetDouble(Ihandle *h, double *dval, int lin, int col);
int  iupmaskMatGetInt   (Ihandle *h, int    *ival, int lin, int col);

#ifdef __cplusplus
}
#endif

#endif
