/** \file
 * \brief Ihandle Class Interface
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUP_CLASS_H 
#define __IUP_CLASS_H

#include "iup_table.h"

#ifdef __cplusplus
extern "C" {
#endif


/** \defgroup iclass Ihandle Class
 * \par
 * See \ref iup_class.h
 * \ingroup cpi */

/** Known native types.
 * \ingroup iclass */
typedef enum _InativeType {
  IUP_TYPEVOID,     /**< No native representation - HBOX, VBOX, ZBOX, FILL, RADIO (handle==(void*)-1 always) */
  IUP_TYPECONTROL,  /**< Native controls - BUTTON, LABEL, TOGGLE, LIST, TEXT, MULTILINE, FRAME, others */
  IUP_TYPECANVAS,   /**< Drawing canvas, also used as a base control for custom controls. */ 
  IUP_TYPEDIALOG,   /**< DIALOG */
  IUP_TYPEIMAGE,    /**< IMAGE */
  IUP_TYPEMENU      /**< MENU, SUBMENU, ITEM, SEPARATOR */
} InativeType;

/** Possible number of children.
 * \ingroup iclass */
typedef enum _IchildType {
  IUP_CHILDNONE, 
  IUP_CHILD_ONE, 
  IUP_CHILDMANY
} IchildType;

typedef struct Iclass_ Iclass;

/** Ihandle Class Structure
 * \ingroup iclass */
struct Iclass_
{
  /* Class configuration parameters. */
  char* name;     /**< class name. No default, must be initialized. */
  char* format;   /**< Creation parameters format of the class when specified. \n
                   * It can have none, one or more of the following.
                   * - "b" = (unsigned char) - byte
                   * - "c" = (unsigned char*) - array of byte
                   * - "i" = (int) - integer
                   * - "j" = (int*) - array of integer
                   * - "f" = (float) - real
                   * - "s" = (char*) - string 
                   * - "a" = (char*) - name of an action
                   * - "h" = (Ihandle*) - element handle
                   * - "g" = (Ihandle**) - array of element handle
                   * If upper case then it is optional. Default is no parameters. */
  InativeType nativetype; /**< native type. Default is IUP_TYPEVOID. */
  IchildType childtype;   /**< children count enum: none, one, or many. Default is IUP_CHILDNONE. */
  int is_interactive;     /**< keyboard interactive boolean, 
                            * true if the class can have the keyboard input focus. Default is false. */
  int has_attrib_id;  /**< boolean to indicate if any attribute is numbered. Default is false. */

  Iclass* parent; /**< class parent to implement inheritance.
                   * Class name must be different.
                   * Creation parameters should be the same or repace the parents creation function.
                   * Native type should be the same.
                   * Child type should be a more restrictive or equal type (many->one->none).
                   * Attribute functions will have only one common table.
                   * All methods can be changed, set to NULL, switched, etc. */

  Itable* attrib_func; /**< table of functions to handle attributes, only one per class tree */

  /* Class methods. */

  /** Method that release the memory allocated by the class.
   * Called only once at \ref iupClassRelease.
   */
  void (*Release)(Iclass* ic);



  /** Method that creates the element and process the creation parameters. \n
   * Called only from IupCreate. \n
   * The parameters can be NULL for all the controls. \n
   * The control should also depend on attributes set before IupMap. \n
   * Must return IUP_NOERROR or IUP_ERROR. \n
   * Can be NULL, like all methods.
   */
  int (*Create)(Ihandle* ih, void** params);

  /** Method that map (create) the control to the native system. \n
   * Called only from IupMap. \n
   * Must return IUP_NOERROR or IUP_ERROR.
   */
  int (*Map)(Ihandle* ih);

  /** Method that unmap (destroy) the control from the native system. \n
   * Called only from IupDestroy if the control is mapped. \n
   * Must return IUP_NOERROR or IUP_ERROR.
   */
  void (*UnMap)(Ihandle* ih);

  /** Method that destroys the element. \n
   * Called only from IupDestroy. Always called even if the control is not mapped.
   */
  void (*Destroy)(Ihandle* ih);



  /** Returns the actual parent to add a child. The default implementation returns itself. \n
   * Called only from IupAppend or IupReparent. \n
   * This allows IUP elements to be a combination of other IUP elements in a single IUP element.
   */
  Ihandle* (*GetInnerContainer)(Ihandle* ih);

  /** Returns the internal native parent. The default implementation returns the handle of itself. \n
    * Called from \ref iupChildTreeGetNativeParentHandle. \n
    * This allows native elements to have an internal container
    * that will be the actual native parent, or in other words allows native elements to be a combination of 
    * other native elements in a single IUP element. The actual native parent may depend on the child tree.
   */
  void* (*GetInnerNativeContainerHandle)(Ihandle* ih, Ihandle* child);

  /** Notifies the element that a child was appended using IupAppend. \n
   * Called only from IupAppend or IupReparent. The child is not mapped, but the parent can be mapped.
   */
  void (*ChildAdded)(Ihandle* ih, Ihandle* child);

  /** Notifies the element that a child was removed using IupDetach. \n
   * Called only from IupDetach or IupReparent. The child is not mapped, but the parent can be mapped.
   */
  void (*ChildRemoved)(Ihandle* ih, Ihandle* child);


  /** Method that update size and position of the native control. \n
   * Called only from \ref iupLayoutUpdate and if the element is mapped.
   */
  void (*LayoutUpdate)(Ihandle* ih);



  /** Method that computes the natural size based on the user size and the actual natural size. \n
   * Should update expand, but does NOT depends on expand to compute the natural size. \n
   * Must call the same method for each children.
   * First calculate the native size for the children, then for the element. \n
   * Also called before the element is mapped, so it must be independent of the native control.
   * First call done at \ref iupLayoutCompute for the dialog.
   */
  void (*ComputeNaturalSize)(Ihandle* ih);

  /** Method that calculates and updates the current size based on the given size,
   * the natural size and the expand configuration. \n
   * The given size is the space available in the parent for the element. \n
   * Must call the same method for each children. 
   * First calculate the current size of the element, then for the children.\n
   * shrink is the dialog attribute passed here for optimization. \n
   * Also called before the element is mapped, so it must be independent of the native control.
   * First call done at \ref iupLayoutCompute for the dialog.
   */
  void (*SetCurrentSize)(Ihandle* ih, int w, int h, int shrink);

  /** Method that calculates and updates the position relative to the parent. \n
   * Must call the same method for each children.
   * First calculate the position of the element, then for the children.\n
   * Also called before the element is mapped, so it must be independent of the native control.
   * First call done at \ref iupLayoutCompute for the dialog.
   */
  void (*SetPosition)(Ihandle* ih, int x, int y);



  /** Method that shows a popup dialog. Called only for native pre-defined dialogs. \n
   * The element is not mapped. \n
   * Must return IUP_ERROR or IUP_NOERROR. \n
   * Called only from \ref iupDialogPopup.
   */
  int (*DlgPopup)(Ihandle* ih, int x, int y);   
};



/** Allocates memory for the Iclass structure and 
 * initializes the attribute handling functions table.
 * \ingroup iclass */
Iclass* iupClassNew(Iclass* ic_parent);

/** Release the memory allocated by the class.
 *  Calls the \ref Iclass::Release method. \n
 *  Called from iupRegisterFinish.
 * \ingroup iclass */
void iupClassRelease(Iclass* ic);



/** GetAttribute called for a specific attribute.
 * Used by \ref iupClassRegisterAttribute.
 * \ingroup iclass */
typedef char* (*IattribGetFunc)(Ihandle* ih);

/** GetAttribute called for a specific attribute when has_attrib_id is true. \n
 * Same as IattribGetFunc but handle attribute names with number ids at the end. \n
 * When calling iupClassRegisterAttribute just use a typecast. \n
 * Pure numbers are translated into IDVALUEid.
 * Used by \ref iupClassRegisterAttribute.
 * \ingroup iclass */
typedef char* (*IattribGetIdFunc)(Ihandle* ih, const char* name_id);

/** SetAttribute called for a specific attribute. \n
 * If returns 0, the attribute will not be stored in the attribute environment
 * (except inheritble attributes that are always stored in the hash table). \n
 * When IupSetAttribute is called using value=NULL, the default_value is passed to this function.
 * Used by \ref iupClassRegisterAttribute.
 * \ingroup iclass */
typedef int (*IattribSetFunc)(Ihandle* ih, const char* value);

/** SetAttribute called for a specific attribute when has_attrib_id is true. \n
 * Same as IattribSetFunc but handle attribute names with number ids at the end. \n
 * When calling iupClassRegisterAttribute just use a typecast. \n
 * Pure numbers are translated into IDVALUEid, ex: "1" = "IDVALUE1".
 * Used by \ref iupClassRegisterAttribute.
 * \ingroup iclass */
typedef int (*IattribSetIdFunc)(Ihandle* ih, const char* name_id, const char* value);

/** Attribute map state dependency.
 * Used by \ref iupClassRegisterAttribute.
 * \ingroup iclass */
typedef enum _Imap{
  IUP_NOT_MAPPED, /**< can call the set/get functions without being mapped */
  IUP_MAPPED      /**< will call the set/get functions only if mapped */
} Imap;

/** Attribute inheritance dependency.
 * Used by \ref iupClassRegisterAttribute.
 * \ingroup iclass */
typedef enum _Iinherit {
  IUP_NO_INHERIT, /**< is not inheritable */
  IUP_INHERIT     /**< is inherited by children and from parent */
} Iinherit;

/** Register attribute handling functions. get, set and default_value can be NULL.
 * default_value should point to a constant string. It will not be duplicated internally.
 * Notice that when an attribute is not defined then default_value=NULL and inherit=IUP_INHERIT.
 * Since there is only one attribute function table per class tree, 
 * if you register the same attribute than a parent class, then it will be replaced.
 * \ingroup iclass */
void iupClassRegisterAttribute(Iclass* ic, const char* name, 
                                           IattribGetFunc get, 
                                           IattribSetFunc set, 
                                           const char* default_value, 
                                           Imap mapped,
                                           Iinherit inherit);

/** Returns the attribute handling functions.
 * \ingroup iclass */
void iupClassRegisterGetAttribute(Iclass* ic, const char* name, 
                                           IattribGetFunc *get, 
                                           IattribSetFunc *set, 
                                           const char* *default_value, 
                                           Imap *mapped,
                                           Iinherit *inherit);

/** Register the parameters of a callback.
 * Used by language bindings.
 * format follows the format specification of the class creation parameters format, 
 * but it adds the "double" option and remove array options.
 * It can have none, one or more of the following. \n
 * - "b" = (unsigned char) - byte
 * - "i" = (int) - integer
 * - "f" = (float) - real
 * - "d" = (double) - real
 * - "s" = (char*) - string 
 * - "v" = (void*) - generic pointer 
 * - "h" = (Ihandle*) - element handle
 * The default return value for all callbacks is "i" (int). 
 * But the return value can be specified using one of the above parameters, 
 * after all parameters using "=" to separate it from them.
 * \ingroup iclass */
void iupClassRegisterCallback(Iclass* ic, const char* name, const char* format);

/** Returns the format of the parameters of a registered callback. 
 * If NULL then the default callback definition is assumed.
 * \ingroup iclass */
char* iupClassCallbackGetFormat(Iclass* ic, const char* name);



/** \defgroup iclassobject Class Object Functions
 * \par
 * Stubs for the class methods. They implement inheritance and check if method is NULL.
 * \par
 * See \ref iup_class.h
 * \ingroup iclass
 */

/** Calls \ref Iclass::Create method. 
 * \ingroup iclassobject
 */
int iupClassObjectCreate(Ihandle* ih, void** params);

/** Calls \ref Iclass::Map method. 
 * \ingroup iclassobject
 */
int iupClassObjectMap(Ihandle* ih);

/** Calls \ref Iclass::UnMap method. 
 * \ingroup iclassobject
 */
void iupClassObjectUnMap(Ihandle* ih);

/** Calls \ref Iclass::Destroy method. 
 * \ingroup iclassobject
 */
void iupClassObjectDestroy(Ihandle* ih);

/** Calls \ref Iclass::GetInnerContainer method.
 * The parent class is ignored. If necessary the child class must handle the parent class internally.
 * \ingroup iclassobject
 */
Ihandle* iupClassObjectGetInnerContainer(Ihandle* ih);

/** Calls \ref Iclass::GetInnerNativeContainerHandle method. 
 * The parent class is ignored. If necessary the child class must handle the parent class internally.
 * \ingroup iclassobject
 */
void* iupClassObjectGetInnerNativeContainerHandle(Ihandle* ih, Ihandle* child);

/** Calls \ref Iclass::ChildAdded method. 
 * \ingroup iclassobject
 */
void iupClassObjectChildAdded(Ihandle* ih, Ihandle* child);

/** Calls \ref Iclass::ChildRemoved method. 
 * \ingroup iclassobject
 */
void iupClassObjectChildRemoved(Ihandle* ih, Ihandle* child);

/** Calls \ref Iclass::LayoutUpdate method. 
 * \ingroup iclassobject
 */
void iupClassObjectLayoutUpdate(Ihandle* ih);

/** Calls \ref Iclass::ComputeNaturalSize method. 
 * \ingroup iclassobject
 */
void iupClassObjectComputeNaturalSize(Ihandle* ih);

/** Calls \ref Iclass::SetCurrentSize method. 
 * \ingroup iclassobject
 */
void iupClassObjectSetCurrentSize(Ihandle* ih, int w, int h, int shrink);

/** Calls \ref Iclass::SetPosition method. 
 * \ingroup iclassobject
 */
void iupClassObjectSetPosition(Ihandle* ih, int x, int y);

/** Calls \ref Iclass::DlgPopup method. 
 * \ingroup iclassobject
 */
int iupClassObjectDlgPopup(Ihandle* ih, int x, int y);



/* Handle attributes, but since the attribute function table is shared by the class hierarchy,
 * the attribute function is retrieved only from the current class.
 * Set is called from iupAttribUpdate (IupMap), IupStoreAttribute and IupSetAttribute.
 * Get is called only from IupGetAttribute.
 */
int iupClassObjectSetAttribute(Ihandle* ih, const char* name, const char* value, int *inherit);
char* iupClassObjectGetAttribute(Ihandle* ih, const char* name, char* *def_value, int *inherit);
char* iupClassObjectGetAttributeDefault(Ihandle* ih, const char* name);

/* Used only in iupAttribUpdateFromParent */
int iupClassCurIsInherit(Iclass* ic);


/* Other functions declared in <iup.h> and implemented here. 
IupGetClassType
IupGetClassName
*/


#ifdef __cplusplus
}
#endif

#include "iup_classbase.h"

#endif
