/** \file
 * \brief Driver Interface. Each driver must export the symbols defined here.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IDRV_H 
#define __IDRV_H

#ifdef __cplusplus
extern "C" {
#endif

int      iupdrvCanvasSize     (Ihandle* e, int *w, int *h);
int      iupdrvGetScrollSize(void);
void     iupdrvGetCharSize(Ihandle* e, int *w, int *h);
void     iupdrvStringSize(Ihandle* n, const char* text, int *w, int *h);
void     iupdrvDestroyHandle(Ihandle* n);
void     iupdrvCreateNativeObject(Ihandle *n);
void     iupdrvCreateObject(Ihandle* self, Ihandle* parent );
void     iupdrvCreateObjects(Ihandle *n);
void     iupdrvResizeObjects(Ihandle *n);
void     iupdrvUpdateAttrs(Ihandle* n);
void     iupdrvSetIdleFunction(Icallback f);
char*    iupdrvGetDefault(Ihandle* n, const char *a);
char*    iupdrvGetAttribute(Ihandle* n, const char *a);
void     iupdrvSetAttribute(Ihandle* n, const char *a, const char *v);
int      iupdrvWindowSizeX(Ihandle* n, int scale);
int      iupdrvWindowSizeY(Ihandle* n, int scale);
int      iupdrvStoreAttribute(Ihandle* n, const char* attr);
int      iupdrvSetGlobal(const char* name, const char* value);
char*    iupdrvGetGlobal(const char* name);
void     iupdrvGetTextLineBreakSize(Ihandle* n, const char* text, int *w, int *h);
int      iupdrvGetBorderSize(Ihandle* n);

#ifdef __cplusplus
}
#endif

#endif
