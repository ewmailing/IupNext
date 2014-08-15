/** \file
 * \brief Global definitions.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IGLOBAL_H 
#define __IGLOBAL_H

#include <stdlib.h>      /* malloc, realloc, free, NULL */

#include "iup.h"
#include "iupcpi.h"
#include "iupcbs.h"
#include "istrutil.h"
#include "itable.h"
#include "inames.h"
#include "ilex.h"
#include "imacros.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ABS
#define ABS(a) ((a)>0?(a):-(a))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define FRAME_MARGEM_X 2
#define FRAME_MARGEM_Y 2	/* xview 12 */	/* windows =  18 */

#define REALLOC(p,s)	((p)==NULL?malloc(s):realloc(p,s))

#define STRBLOCK	20
#define SPACE	        32

#define NOSTR   "NO"
#define YESSTR  "YES"
#define NO      0
#define YES     1
#define NOTDEF -1

/************************************************************************/
/*              Control types                                           */
/************************************************************************/

extern char *UNKNOWN_;
extern char *IMAGE_;
extern char *BUTTON_;
extern char *CANVAS_;
extern char *DIALOG_;
extern char *FILL_;
extern char *FRAME_;
extern char *HBOX_;
extern char *ITEM_;
extern char *SEPARATOR_;
extern char *SUBMENU_;
extern char *KEYACTION_;
extern char *LABEL_;
extern char *LIST_;
extern char *MENU_;
extern char *RADIO_;
extern char *TEXT_;
extern char *TOGGLE_;
extern char *VBOX_;
extern char *ZBOX_;
extern char *MULTILINE_;
extern char *USER_;
extern char *CONTROL_;

typedef struct Ikeyact_
{
  Itable* list;
} Ikeyact;

typedef struct Ilist_
{
  void* data;
  void* dddata;
} Ilist;

typedef struct Isize_
{
 int width;
 int height;
} Isize;

typedef union Iattrib_
{
  void *data;
  Ikeyact k;
  Ilist lst;
} Iattrib;

struct Ihandle_
{
  int serialnumber;		 /* serial number */
  char* type;		       /* type of interface element */
  Itable* enviroment;
  Iclass* iclass;
  void* handle;        /* native handle */
  int usersize;        /* the configuration of expand and fill */
  int posx,posy;       /* upper-left corner */
  Isize natural;       /* the calculated size based in the control contents */
  Isize current;       /* the actual size of the control */
  Iattrib attrib;
  Ihandle* brother;    /* next control in the container */
  Ihandle* firstchild;
  Ihandle* parent;
};

/* ienv.c */
int   iupCheck     (Ihandle *n, const char *a);   /* this returns a non zero value if YES or NOTDEF, and zero if NO */
                                            /* the use of the return values indicates also the default value  */
                                            /* if the attribute is not defined.                               */
                                            /* (iupCheck() or !iupCheck()) -> default=YES                     */
                                            /* (iupCheck()!=YES or iupCheck()==YES) -> default=NO             */
int   iupIsInternal(const char* name);
int   iupIsPointer(const char *attr);
int   iupIsInheritable(const char *attr);
void  iupSetEnv    (Ihandle* n, const char* a, const char* v);
void  iupStoreEnv  (Ihandle* n, const char* a, const char* v);
char* iupGetEnvRaw (Ihandle* n, const char* a);   /* no inheritance */
char* iupGetEnv    (Ihandle* n, const char* a);
int   iupGetEnvInt (Ihandle* n, const char* a);
int   iupGetWidth  (Itable* t);
int   iupGetHeight (Itable* t);
int   iupGetColor  (Ihandle* n, const char *a, unsigned int *r, unsigned int *g, unsigned int *b);
int   iupGetRGB (const char *color, unsigned int *r, unsigned int *g, unsigned int *b);
int   iupGetSize   (Ihandle *n, int* w, int *h);
char *iupGetBgColor(Ihandle* self);
char *iupGetParentBgColor (Ihandle* self);
/* Other functions from <iup.h> and implemented in env.c :
char* IupGetAttributes (Ihandle *n)
Ihandle* IupSetAttributes (Ihandle *n, char* e)
void IupSetAttribute (Ihandle *n, char* a, char *v)
void IupStoreAttribute (Ihandle *n, char* a, char *v)
char* IupGetAttribute (Ihandle *n, char* a)
float IupGetFloat (Ihandle *n, char* a)
int IupGetInt (Ihandle *n, char* a)
void IupSetfAttribute (Ihandle *n, char* a, char* f, ...)
*/

/* icreate.c */
Ihandle* iupMkHbox      (Ihandle* box);
Ihandle* iupMkMenu      (Ihandle* menu);
Ihandle* iupMkVbox      (Ihandle* box);
Ihandle* iupMkZbox      (Ihandle* box);
Ihandle* iupMkKeyAction (Itable* list);
/* Other functions from <iup.h> and implemented in create.c :
Ihandle* IupImage (unsigned int width, unsigned int height, char *pixmap)
Ihandle* IupButton (char* label, char* action)
Ihandle* IupCanvas (char* repaint)
Ihandle* IupDialog (Ihandle* exp)
Ihandle* IupFill (void)
Ihandle* IupFrame (Ihandle* exp)
Ihandle* IupUser ()
Ihandle *IupHbox (Ihandle *first, ...)
Ihandle* IupItem (char* label, char* action)
Ihandle* IupSubmenu (char* label, Ihandle* exp)
Ihandle* IupLabel (char* label)
Ihandle* IupList (char* action)
Ihandle *IupMenu (Ihandle *first, ...)
Ihandle* IupRadio (Ihandle* exp)
Ihandle* IupText (char* action)
Ihandle* IupMultiLine (char* action)
Ihandle* IupToggle (char *label, char *action)
Ihandle* IupSeparator (void)
Ihandle *IupVbox (Ihandle* first, ...)
Ihandle *IupZbox (Ihandle* first, ...)
Ihandle *IupCreatep(char *name, Ihandle* first, ...)
Ihandle* IupCreate(char *name)
Ihandle* IupCreatev(char *name, Ihandle **params)
*/

/* icalcsize.c */
int  iupSetSize        (Ihandle *root);
void iupSetCurrentSize (Ihandle *n, int w, int h);
int  iupSetNaturalSize (Ihandle *n);
void iupSetPosition (Ihandle *n, int x, int y);
void iupListGetItemSize(Ihandle *e, int *w, int *h, int isdropdown);

/* ikey.c */
int iupKeyCanCaps(int iupcode);
char *iupKeyEncode(int c);
int   iupKeyDecode (char *c);
void  iupKeyForEach(void (*func)(char *name, int code, void* user_data), void* user_data);

/* ifocus.c */
int iupAcceptFocus    (Ihandle *h);
/* Other functions from <iup.h> and implemented in focus.c :
Ihandle* IupPreviousField (Ihandle *h)
Ihandle *IupNextField (Ihandle *h)
*/

/* Funcoes de acesso `a estrutura de um elemento Iup (Ihandle) em icpi.c
  */
void  iupSetPosX(Ihandle* self, int x);
void  iupSetPosY(Ihandle* self, int y);
int   iupGetPosX(Ihandle* self);
int   iupGetPosY(Ihandle* self);
void  iupSetCurrentWidth(Ihandle* self, int w);
void  iupSetCurrentHeight(Ihandle* self, int h);
int   iupGetCurrentWidth(Ihandle* self);
int   iupGetCurrentHeight(Ihandle* self);
void* iupGetNativeHandle(Ihandle* self);
void  iupSetNativeHandle(Ihandle* self, void* handle);
void  iupSetNaturalWidth(Ihandle* self, int w);
void  iupSetNaturalHeight(Ihandle* self, int h);
int   iupGetNaturalWidth(Ihandle* self);
int   iupGetNaturalHeight(Ihandle* self);
void* iupGetImageData(Ihandle* self);

/* itraverse.c 
   Other functions from <iup.h> and implemented in itraverse.c :
     IupGetDialog, IupDetach, IupAppend, IupGetNextChild, IupGetBrother, IupGetParent
*/
Ihandle *iupGetRadio(Ihandle *n);


#ifdef __cplusplus
}
#endif

#endif
