/** \file
 * \brief Internal Pattern Matching.
 *
 * See Copyright Notice in iup.h
 * $Id: imask.h,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */
 
#ifndef __IMASK_H 
#define __IMASK_H

#ifdef __cplusplus
extern "C" {
#endif

#define MASKSET    0
#define MASKCHAR   1
#define MASKAUTOFILL   2
#define MASKNOAUTOFILL 3

#define MASKALLCHARS 0

typedef struct Imask_elem {
   int    min, max;
   int    type;
   void  *elem;
   struct Imask_elem *next;
} ImaskElem;

typedef struct Imask_set {
  char str[30];
  int  len;
  int  not;
} ImaskSet;

typedef struct Imask_char {
   char ch;
} ImaskChar;

ImaskElem* iupCreateMask(char *mask);
int iupCheckMask(ImaskElem *me, char *str, char *fill);
   
typedef struct {
   ImaskElem *mask;
   int status;
} ITextInfo;

#ifdef __cplusplus
}
#endif

#endif
