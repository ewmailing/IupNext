/** \file
 * \brief iuptree control
 * Functions to set and get attributes.
 *
 * See Copyright Notice in iup.h
 * $Id: itgetset.h,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */
 
#ifndef __ITGETSET_H 
#define __ITGETSET_H

#include "iuptree.h"

#ifdef __cplusplus
extern "C" {
#endif

int treegetsetAddExpanded (Ihandle *h, char* value);
int treegetsetAddNode (Ihandle *h, char* id_string, int kind);
int treegetsetDelNode (Ihandle *h, char* id_string, char* mode);

int treegetsetSetName (Ihandle* h, char* id_string, char* name);
int treegetsetSetPath (Ihandle *h, char* path);
int treegetsetSetValue (Ihandle *h, char* mode, int call_cb);
int treegetsetSetStarting (Ihandle *h, char* value);
int treegetsetSetState (Ihandle *h, char* id_string, char* mode);
int treegetsetSetDepth (Ihandle *h, char* id_string, char* dep);
int treegetsetSetCtrl (Ihandle *h, char* mode);
int treegetsetSetShift (Ihandle *h, char* mode);
int treegetsetSetImage (char* name, unsigned char* image, unsigned long int* colors, unsigned long int* selected, int range, Node node);

char* treegetsetGetName (Ihandle *h, char* id_string);
char* treegetsetGetKind (Ihandle *h, char* id_string);
char* treegetsetGetState (Ihandle *h, char* id_string);
char* treegetsetGetValue (Ihandle *h);
char* treegetsetGetStarting (Ihandle *h);
char* treegetsetGetParent (Ihandle *h, char* id_string);
char* treegetsetGetDepth (Ihandle *h, char* id_string);
char* treegetsetGetMarked (Ihandle *h, char* id_string);
char* treegetsetGetCtrl (Ihandle *h);
char* treegetsetGetShift (Ihandle *h);
void  treegetsetMark(Ihandle *h, char *node, char *attr);

int treegetsetSetColor(Ihandle *h, char* id, char* color);
char *treegetsetGetColor(Ihandle *h, char* id);

/* Auxiliar Functions */
int treegetsetCreateNode (TtreePtr tree, Node node, int kind);
void treegetsetSetMarked (Ihandle *h, int mode, int call_cb);
int treegetsetInvertSelection (Ihandle* h, char* id_string, int call_cb);
int treegetsetSetStateOfNode(Ihandle *h, Node node, char* mode);

#ifdef __cplusplus
}
#endif

#endif
