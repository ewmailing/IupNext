/** \file
 * \brief List Control
 *
 * See Copyright Notice in "iup.h"
 */

// First implement iupdrvListGetCount()
// Next is iupdrvListAppendItem()
// Also insert, remove, and removeall, getvalueattribute and setvalueattribute

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_mask.h"
#include "iup_key.h"
#include "iup_image.h"
#include "iup_list.h"

#include "iupemscripten_drv.h"

typedef enum
{
  IUPEMSCRIPTENLISTSUBTYPE_UNKNOWN = -1,
  IUPEMSCRIPTENLISTSUBTYPE_DROPDOWN,
  IUPEMSCRIPTENLISTSUBTYPE_EDITBOXDROPDOWN,
  IUPEMSCRIPTENLISTSUBTYPE_EDITBOX,
  IUPEMSCRIPTENLISTSUBTYPE_MULTIPLELIST,
  IUPEMSCRIPTENLISTSUBTYPE_SINGLELIST // Not official, but could be useful for mobile
} IupEmscriptenListSubType;

extern int emjsList_CreateList(IupEmscriptenListSubType subtype, int32_t arr[], size_t arr_size);
extern int emjsList_GetCount(int handleID, IupEmscriptenListSubType subType);
extern void emjsList_AppendItem(int handleID, IupEmscriptenListSubType subType, const char* value);
extern char* emjsListCreateIdValueAttrib(int handleID, int pos);

void iupdrvListAddItemSpace(Ihandle* ih, int *h)
{
  (void)ih;
  /* FROM:
    gtk_tree_view_column_cell_get_size
      height = text_height + 2*focus_line_width;
    gtk_widget_style_get(ih->handle, "focus-line-width", &focus_line_width, NULL);
      returns always 1
  */
  *h += 2;
}

// Yes, this is marked with KEEPALIVE because it is called from the font.js. (But maybe we can remove this because it is not clear if we need it for "direct".)
// And yes, this is actually an Ihandle* ih as a parameter coming from JavaScript.
// This is using a different Emscripten compiler feature we uncovered ("direct" invocation using leading _ underscore).
EMSCRIPTEN_KEEPALIVE IupEmscriptenListSubType emscriptenListGetSubType(Ihandle* ih)
{
	if(ih->data->is_dropdown) {
      if(ih->data->has_editbox) {
          return IUPEMSCRIPTENLISTSUBTYPE_EDITBOXDROPDOWN;
      }
      else {
          return IUPEMSCRIPTENLISTSUBTYPE_DROPDOWN;
      }
  }
	else {
      if(ih->data->has_editbox) {
          return IUPEMSCRIPTENLISTSUBTYPE_EDITBOX;
      }
      else if(ih->data->is_multiple) {
          return IUPEMSCRIPTENLISTSUBTYPE_MULTIPLELIST;

      }
      else {
          return IUPEMSCRIPTENLISTSUBTYPE_SINGLELIST;
      }
  }
	return IUPEMSCRIPTENLISTSUBTYPE_UNKNOWN;
}

int iupdrvListGetCount(Ihandle* ih)
{
  IupEmscriptenListSubType sub_type = emscriptenListGetSubType(ih);
  int count = emjsList_GetCount(ih->handle->handleID, sub_type);
  return count;
}

void iupdrvListAddBorders(Ihandle* ih, int *x, int *y)
{
/*   int border_size = 2*5; */
/*   (*x) += border_size; */
/*   (*y) += border_size; */

/*   if (ih->data->is_dropdown) { */
/* #ifdef HILDON */
/*     (*x) += 9; /\* extra space for the dropdown button *\/ */
/* #else */
/*     (*x) += 5; /\*  space for the dropdown button *\/ */
/* #endif */

/*     if (ih->data->has_editbox) */
/*       (*x) += 5; /\* another extra space for the dropdown button *\/ */
/*     else { */
/*       (*y) += 4; /\* extra padding space *\/ */
/*       (*x) += 4; /\* extra padding space *\/ */
/*     } */
/*   } */
/*   else { */
/*     if (ih->data->has_editbox) */
/*       (*y) += 2*3; /\* internal border between editbox and list *\/ */
/*   } */
}
void iupdrvListAppendItem(Ihandle* ih, const char* value)
{
    IupEmscriptenListSubType sub_type = emscriptenListGetSubType(ih);
    // need to call to js function to add item; should pass sub_type so it knows how to process

    iupEmscripten_Log("HandleID: %p", ih->handle->handleID);
    emjsList_AppendItem(ih->handle->handleID, sub_type, value);
}

void iupdrvListInsertItem(Ihandle* ih, int pos, const char* value)
{

}

void iupdrvListRemoveItem(Ihandle* ih, int pos)
{

}

void iupdrvListRemoveAllItems(Ihandle* ih)
{

}

void* iupdrvListGetImageHandle(Ihandle* ih, int id)
{

    return NULL;
}

int iupdrvListSetImageHandle(Ihandle* ih, int id, void* hImage)
{

  return 0;
}

static char* emscriptenListGetIdValueAttrib(Ihandle* ih, int id_value)
{
	int pos = iupListGetPosAttrib(ih, id_value);
	if(pos >= 0)
	{
    char* c_str = emjsListCreateIdValueAttrib(ih->handle->handleID, pos);
    char* iup_str = iupStrReturnStr(c_str);
    free(c_str);
    return iup_str;
	}
	return NULL;
}



static int emscriptenListMapMethod(Ihandle* ih)
{
  int elem_count = 0;
  InativeHandle* new_handle = NULL;

  IupEmscriptenListSubType sub_type = emscriptenListGetSubType(ih);
  new_handle = (InativeHandle*)calloc(1, sizeof(InativeHandle));
  elem_count = emjsList_CreateList(sub_type, new_handle->compoundHandleIDArray, IUP_EMSCRIPTEN_MAX_COMPOUND_ELEMENTS);

  if (elem_count > 1) {
    new_handle->isCompound = true;
    new_handle->numElemsIfCompound = elem_count;
    new_handle->handleID = new_handle->compoundHandleIDArray[1]; // NOTE: We currently only expect 2 elements in this case
  }
  else {
    new_handle->handleID = new_handle->compoundHandleIDArray[0];
  }

  ih->handle = new_handle;
  iupEmscripten_SetIntKeyForIhandleValue(new_handle->handleID, ih);
  iupEmscripten_Log("Handle ID is %d for ih_pointer %p", new_handle->handleID, ih);

#if 0
	NSView* the_view;
	NSPopUpButton* popup_button = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(30, 30, 190, 40)];


	if (ih->data->is_dropdown) {

    if (ih->data->has_editbox) {

    }
		else {
//			ih->handle = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
		}

		if(ih->data->show_image) {

		}

    if (ih->data->has_editbox) {

      if (!iupAttribGetBoolean(ih, "CANFOCUS")) {
//				iupgtkSetCanFocus(ih->handle, 0);
			}

		}
		else
		{

			if (!iupAttribGetBoolean(ih, "CANFOCUS")) {

			}
			else {

			}
		}

	}
	else {

		if (ih->data->has_editbox) {

//			iupgtkSetCanFocus(ih->handle, 0);  /* focus goes only to the edit box */
			if (!iupAttribGetBoolean(ih, "CANFOCUS")) {
//				iupgtkSetCanFocus(entry, 0);
			}

		}
		else {

			if (!iupAttribGetBoolean(ih, "CANFOCUS")) {
//				iupgtkSetCanFocus(ih->handle, 0);
			}
		}

		if (ih->data->show_image) {

		}

		if (ih->data->sb) {
			if (iupAttribGetBoolean(ih, "AUTOHIDE")) {
//				scrollbar_policy = GTK_POLICY_AUTOMATIC;
			}
			else {
//				scrollbar_policy = GTK_POLICY_ALWAYS;
			}
		}
		else {
//			scrollbar_policy = GTK_POLICY_NEVER;
		}


		if (!ih->data->has_editbox && ih->data->is_multiple) {

		}
		else {
		}

	}

	/* Enable internal drag and drop support */
	if(ih->data->show_dragdrop && !ih->data->is_dropdown && !ih->data->is_multiple)
	{

	}

	if (iupAttribGetBoolean(ih, "SORT"))
	{
//		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), IUPGTK_LIST_TEXT, GTK_SORT_ASCENDING);
	}
	/* add to the parent, all GTK controls must call this. */
//	iupgtkAddToParent(ih);


	ih->handle = the_view;

#if 0
	// I'm using objc_setAssociatedObject/objc_getAssociatedObject because it allows me to avoid making subclasses just to hold ivars.
	objc_setAssociatedObject(the_toggle, IHANDLE_ASSOCIATED_OBJ_KEY, (id)ih, OBJC_ASSOCIATION_ASSIGN);
	// I also need to track the memory of the buttion action receiver.
	// I prefer to keep the Ihandle the actual NSView instead of the receiver because it makes the rest of the implementation easier if the handle is always an NSView (or very small set of things, e.g. NSWindow, NSView, CALayer).
	// So with only one pointer to deal with, this means we need our Toggle to hold a reference to the receiver object.
	// This is generally not good emscripten as Toggles don't retain their receivers, but this seems like the best option.
	// Be careful of retain cycles.
	IupemscriptenToggleReceiver* toggle_receiver = [[IupemscriptenToggleReceiver alloc] init];
	[the_toggle setTarget:toggle_receiver];
	[the_toggle setAction:@selector(myToggleClickAction:)];
	// I *think* is we use RETAIN, the object will be released automatically when the Toggle is freed.
	// However, the fact that this is tricky and I had to look up the rules (not to mention worrying about retain cycles)
	// makes me think I should just explicitly manage the memory so everybody is aware of what's going on.
	objc_setAssociatedObject(the_toggle, IUP_emscripten_TOGGLE_RECEIVER_OBJ_KEY, (id)toggle_receiver, OBJC_ASSOCIATION_ASSIGN);

#endif
	// All emscripten views shoud call this to add the new view to the parent view.
	iupemscriptenAddToParent(ih);



	/* configure for DRAG&DROP */
	if (IupGetCallback(ih, "DROPFILES_CB"))
	{
//		iupAttribSet(ih, "DROPFILESTARGET", "YES");
	}

//	IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)gtkListConvertXYToPos);


	/* update a mnemonic in a label if necessary */
//	iupgtkUpdateMnemonic(ih);

#endif

  iupListSetInitialItems(ih);
  iupEmscripten_AddWidgetToParent(ih);

	return IUP_NOERROR;
}


static void emscriptenListUnMapMethod(Ihandle* ih)
{
#if 0
	id the_view = ih->handle;
	/*
	 id text_receiver = objc_getAssociatedObject(the_view, IUP_emscripten_TOGGLE_RECEIVER_OBJ_KEY);
	 objc_setAssociatedObject(the_view, IUP_emscripten_TOGGLE_RECEIVER_OBJ_KEY, nil, OBJC_ASSOCIATION_ASSIGN);
	 [text_receiver release];
	 */
	[the_view release];
	ih->handle = NULL;
#endif

}


void iupdrvListInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
	ic->Map = emscriptenListMapMethod;
	ic->UnMap = emscriptenListUnMapMethod;


  iupClassRegisterAttributeId(ic, "IDVALUE", emscriptenListGetIdValueAttrib, iupListSetIdValueAttrib, IUPAF_NO_INHERIT);
  /* iupClassRegisterAttribute(ic, "VALUE", gtkListGetValueAttrib, gtkListSetValueAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT); */
  /* iupClassRegisterAttribute(ic, "SHOWDROPDOWN", NULL, gtkListSetShowDropdownAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT); */

#if 0

  /* Driver Dependent Attribute functions */

  /* Overwrite Common */
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, gtkListSetStandardFontAttrib, IUPAF_SAMEASSYSTEM, "DEFAULTFONT", IUPAF_NO_SAVE|IUPAF_NOT_MAPPED);

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, gtkListSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTBGCOLOR", IUPAF_DEFAULT);

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, gtkListSetFgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTFGCOLOR", IUPAF_DEFAULT);

  /* IupList only */
  iupClassRegisterAttributeId(ic, "IDVALUE", gtkListGetIdValueAttrib, iupListSetIdValueAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", gtkListGetValueAttrib, gtkListSetValueAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWDROPDOWN", NULL, gtkListSetShowDropdownAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TOPITEM", NULL, gtkListSetTopItemAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPACING", iupListGetSpacingAttrib, gtkListSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);

  iupClassRegisterAttribute(ic, "PADDING", iupListGetPaddingAttrib, gtkListSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "0x0", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "SELECTEDTEXT", gtkListGetSelectedTextAttrib, gtkListSetSelectedTextAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTION", gtkListGetSelectionAttrib, gtkListSetSelectionAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTIONPOS", gtkListGetSelectionPosAttrib, gtkListSetSelectionPosAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CARET", gtkListGetCaretAttrib, gtkListSetCaretAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CARETPOS", gtkListGetCaretPosAttrib, gtkListSetCaretPosAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "INSERT", NULL, gtkListSetInsertAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "APPEND", NULL, gtkListSetAppendAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "READONLY", gtkListGetReadOnlyAttrib, gtkListSetReadOnlyAttrib, NULL, NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "NC", iupListGetNCAttrib, gtkListSetNCAttrib, NULL, NULL, IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "CLIPBOARD", NULL, gtkListSetClipboardAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTO", NULL, gtkListSetScrollToAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTOPOS", NULL, gtkListSetScrollToPosAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "IMAGE", NULL, gtkListSetImageAttrib, IUPAF_IHANDLENAME|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Not Supported */
  iupClassRegisterAttribute(ic, "VISIBLE_ITEMS", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_SUPPORTED);
  iupClassRegisterAttribute(ic, "DROPEXPAND", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_SUPPORTED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AUTOREDRAW", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_SUPPORTED|IUPAF_NO_INHERIT);
#endif
}
