/** \file
 * \brief Attributes Environment Management
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_ATTRIB_H 
#define __IUP_ATTRIB_H

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup attrib Attribute Environment 
 * \par
 * The attributes that are not stored at the control, 
 * are stored in a hash table (see \ref table).
 * \par
 * As a general rule use:
 * - IupGetAttribute, IupSetAttribute, ... : when care about control implementation, inheritance, hash table and default value
 * - iupAttribGetStr, iupAttribSetStr, ... : when NOT care, ONLY access the hash table
 * - iupAttribGetStrDefault                : same as iupAttribGetStr, but checks the default value
 * These different functions have very different performances and results. So use them wiselly.
 * \par
 * See \ref iup_attrib.h 
 * \ingroup cpi */


/** Returns true if the attribute name if in the internal format "_IUP...".
 * \ingroup attrib */
int iupAttribIsInternal(const char* name);

/** Returns true if the attribute name is a known pointer.
 * \ingroup attrib */
int iupAttribIsPointer(const char *name);

/** Returns true if the attribute is inheritable. 
 * valid only for a small group of common attributes.
 * Not used internally. The inheritance in handled directly by 
 * the Attribute Function registration in the \ref iclass.
 * \ingroup attrib */
int iupAttribIsInheritable(const char *name);

/** Sets the attribute only in the attribute environment as a pointer.
 * It ignores children.
 * \ingroup attrib */
void iupAttribSetStr(Ihandle* ih, const char* name, const char* value);

/** Sets the attribute only in the attribute environment as a string. 
 * The string is internally duplicated.
 * It ignores children.
 * \ingroup attrib */
void iupAttribStoreStr(Ihandle* ih, const char* name, const char* value);

/** Sets the attribute only in the attribute environment as a string. 
 * The string is internally duplicated. Use same format as sprintf.
 * It ignores children.
 * \ingroup attrib */
void iupAttribSetStrf(Ihandle *ih, const char* name, const char* format, ...);

/** Sets an integer attribute only in the attribute environment.
 * It will be stored as a string.
 * It ignores children.
 * \ingroup attrib */
void iupAttribSetInt(Ihandle *ih, const char* name, int num);

/** Sets an floating point attribute only in the attribute environment.
 * It will be stored as a string.
 * It ignores children.
 * \ingroup attrib */
void iupAttribSetFloat(Ihandle *ih, const char* name, float num);

/** Returns the attribute from the attribute environment. 
 * It ignores inheritance.
 * \ingroup attrib */
char* iupAttribGetStr(Ihandle* ih, const char* name);   

/** Returns the attribute from the attribute environment, 
 * but if not defined then returns the registered default value if any.
 * It ignores inheritance. 
 * \ingroup attrib */
char* iupAttribGetStrDefault(Ihandle* ih, const char* name);

/** Returns the attribute from the attribute environment, 
 * but if not defined then checks in the parent and goes recursive in the child tree.
 * \ingroup attrib */
char* iupAttribGetStrInherit(Ihandle* ih, const char* name);

/** Returns the attribute from the attribute environment, 
 * but checks in the native parents.
 * \ingroup attrib */
char* iupAttribGetStrNativeParent(Ihandle* ih, const char* name);

/** Returns the attribute from the attribute environment as an integer. 
 * It ignores inheritance.
 * \ingroup attrib */
int iupAttribGetInt(Ihandle* ih, const char* name);

/** Returns the attribute from the attribute environment as an integer, 
 * but if not defined then returns the registered default value if any.
 * It ignores inheritance.
 * \ingroup attrib */
int iupAttribGetIntDefault(Ihandle* ih, const char* name);

/** Returns the attribute from the attribute environment as a floating point. 
 * It ignores inheritance.
 * \ingroup attrib */
float iupAttribGetFloat(Ihandle* ih, const char* name);

/** Returns the attribute from the attribute environment as a floating point, 
 * but if not defined then returns the registered default value if any.
 * It ignores inheritance.
 * \ingroup attrib */
float iupAttribGetFloatDefault(Ihandle* ih, const char* name);

/** Set an internal name to a handle.
 * \ingroup attrib */
void iupAttribSetHandleName(Ihandle *ih);


/* For all attributes in the evironment, call the class SetAttribute only.
 * Called only after the element is mapped. */
void iupAttribUpdate(Ihandle* ih); 

/* For all registered inherited attributes, checks the parent tree and 
 * call the class SetAttribute if the attribute is defined.
 * Called only after the element is mapped. */
void iupAttribUpdateFromParent(Ihandle* ih);

/* Recursive function to notify the attribute change to the children 
   BUT does NOT change their environment. 
   Notify ONLY if not defined in the child. */
void iupAttribNotifyChildren(Ihandle *ih, const char* name, const char *value);



/* Other functions declared in <iup.h> and implemented here. 
IupGetAllAttributes
IupGetAttributes
IupSetAttributes
IupSetAttribute
IupStoreAttribute
IupGetAttribute
IupGetFloat
IupGetInt
IupGetInt2
IupSetfAttribute
IupSetAttributeHandle
IupGetAttributeHandle
*/


#ifdef __cplusplus
}
#endif

#endif
