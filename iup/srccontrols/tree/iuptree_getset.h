/** \file
 * \brief iuptree control
 * Functions to set and get attributes.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPTREE_GETSET_H 
#define __IUPTREE_GETSET_H

#ifdef __cplusplus
extern "C" {
#endif

int iupTreeGSAddExpanded(Ihandle* ih, const char* value);
int iupTreeGSAddNode(Ihandle* ih, const char* id_string, int kind);
int iupTreeGSDelNode(Ihandle* ih, const char* id_string, const char* mode);

int iupTreeGSSetName(Ihandle* ih, const char* id_string, const char* name);
int iupTreeGSSetPath(Ihandle* ih, const char* path);
int iupTreeGSSetValue(Ihandle* ih, const char* mode, int call_cb);
int iupTreeGSSetStarting(Ihandle* ih, const char* value);
int iupTreeGSSetState(Ihandle* ih, const char* id_string, const char* mode);
int iupTreeGSSetDepth(Ihandle* ih, const char* id_string, const char* dep);
int iupTreeGSSetCtrl(Ihandle* ih, const char* mode);
int iupTreeGSSetShift(Ihandle* ih, const char* mode);
int iupTreeGSSetImage(const char* name, unsigned char* image, unsigned long int* colors, unsigned long int* selected, int range, ItreeNodePtr node);

char* iupTreeGSGetName(Ihandle* ih, const char* id_string);
char* iupTreeGSGetKind(Ihandle* ih, const char* id_string);
char* iupTreeGSGetState(Ihandle* ih, const char* id_string);
char* iupTreeGSGetValue(Ihandle* ih);
char* iupTreeGSGetStarting(Ihandle* ih);
char* iupTreeGSGetParent(Ihandle* ih, const char* id_string);
char* iupTreeGSGetDepth(Ihandle* ih, const char* id_string);
char* iupTreeGSGetMarked(Ihandle* ih, const char* id_string);
char* iupTreeGSGetCtrl(Ihandle* ih);
char* iupTreeGSGetShift(Ihandle* ih);
void  iupTreeGSMark(Ihandle* ih, const char* node, const char* attr);

int   iupTreeGSSetColor(Ihandle* ih, const char* id, const char* color);
char* iupTreeGSGetColor(Ihandle* ih, const char* id);

/* Auxiliary Functions */
int  iupTreeGSCreateNode(Ihandle* ih, ItreeNodePtr node, int kind);
void iupTreeGSSetMarked(Ihandle* ih, int mode, int call_cb);
int  iupTreeGSInvertSelection(Ihandle* ih, const char* id_string, int call_cb);
int  iupTreeGSSetStateOfNode(Ihandle* ih, ItreeNodePtr node, const char* mode);

#ifdef __cplusplus
}
#endif

#endif
