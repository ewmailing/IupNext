/** \file
 * \brief iuptree control
 * Functions to set and get attributes.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ITGETSET_H 
#define __ITGETSET_H

#ifdef __cplusplus
extern "C" {
#endif

int iTreeGSAddExpanded(Ihandle* ih, const char* value);
int iTreeGSAddNode(Ihandle* ih, const char* id_string, int kind);
int iTreeGSDelNode(Ihandle* ih, const char* id_string, const char* mode);

int iTreeGSSetName(Ihandle* ih, const char* id_string, const char* name);
int iTreeGSSetPath(Ihandle* ih, const char* path);
int iTreeGSSetValue(Ihandle* ih, const char* mode, int call_cb);
int iTreeGSSetStarting(Ihandle* ih, const char* value);
int iTreeGSSetState(Ihandle* ih, const char* id_string, const char* mode);
int iTreeGSSetDepth(Ihandle* ih, const char* id_string, const char* dep);
int iTreeGSSetCtrl(Ihandle* ih, const char* mode);
int iTreeGSSetShift(Ihandle* ih, const char* mode);
int iTreeGSSetImage(const char* name, unsigned char* image, unsigned long int* colors, unsigned long int* selected, int range, Node node);

char* iTreeGSGetName(Ihandle* ih, const char* id_string);
char* iTreeGSGetKind(Ihandle* ih, const char* id_string);
char* iTreeGSGetState(Ihandle* ih, const char* id_string);
char* iTreeGSGetValue(Ihandle* ih);
char* iTreeGSGetStarting(Ihandle* ih);
char* iTreeGSGetParent(Ihandle* ih, const char* id_string);
char* iTreeGSGetDepth(Ihandle* ih, const char* id_string);
char* iTreeGSGetMarked(Ihandle* ih, const char* id_string);
char* iTreeGSGetCtrl(Ihandle* ih);
char* iTreeGSGetShift(Ihandle* ih);
void  iTreeGSMark(Ihandle* ih, const char* node, const char* attr);

int   iTreeGSSetColor(Ihandle* ih, const char* id, const char* color);
char* iTreeGSGetColor(Ihandle* ih, const char* id);

/* Auxiliary Functions */
int  iTreeGSCreateNode(Ihandle* ih, Node node, int kind);
void iTreeGSSetMarked(Ihandle* ih, int mode, int call_cb);
int  iTreeGSInvertSelection(Ihandle* ih, const char* id_string, int call_cb);
int  iTreeGSSetStateOfNode(Ihandle* ih, Node node, const char* mode);

#ifdef __cplusplus
}
#endif

#endif
