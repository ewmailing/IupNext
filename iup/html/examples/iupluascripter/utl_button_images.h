/*
 * daVinci - IUP Button Images Utilities
 *
 * Copyright (C) 1999-2008, Antonio E. Scuri, Mauro Charão, Tecgraf/PUC-Rio.
 */


#ifndef _UTL_IUP_H
#define _UTL_IUP_H

#ifdef __cplusplus
extern "C" {
#endif

void utlCreateButtonImages(void);
void utlKillButtonImages(void);
void utlCreateIupImage(int w, int h, unsigned char *bits, char *colors[], char *name);

#ifdef __cplusplus
}
#endif

#endif
