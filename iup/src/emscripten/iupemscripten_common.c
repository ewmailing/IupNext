/** \file
 * \brief Emscripten Base Functions
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>              
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdarg.h>

#include "iup.h"
#include "iup_varg.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_key.h"
#include "iup_str.h"
#include "iup_class.h"
#include "iup_attrib.h"
#include "iup_focus.h"
#include "iup_key.h"
#include "iup_image.h"
#include "iup_drv.h"

#include "iupemscripten_drv.h"


static Itable* s_integerIdToIhandleMap = NULL;

void iupEmscripten_InitializeInternalGlobals()
{
	if(!s_integerIdToIhandleMap)
	{
		// FIXME: Is there a place to free the memory?
		s_integerIdToIhandleMap = iupTableCreate(IUPTABLE_POINTERINDEXED);
	}
} 
void iupEmscripten_DestroyInternalGlobals()
{

	if(s_integerIdToIhandleMap)
	{
		iupTableDestroy(s_integerIdToIhandleMap);
		s_integerIdToIhandleMap = NULL;
	}
}
void iupEmscripten_SetIntKeyForIhandleValue(int handle_id, Ihandle* ih)
{
	iupTableSet(s_integerIdToIhandleMap, (const char*)((intptr_t)handle_id), ih, IUPTABLE_POINTER);
}
void iupEmscripten_RemoveIntKeyFromIhandleMap(int handle_id)
{
	iupTableRemove(s_integerIdToIhandleMap, (const char*)((intptr_t)handle_id));
}
Ihandle* iupEmscripten_GetIhandleValueForKey(int handle_id)
{
	Ihandle* ih = (Ihandle*)((intptr_t)iupTableGet(s_integerIdToIhandleMap, (const char*)((intptr_t)handle_id)));
	return ih;
}


extern void emjsCommon_Log(char* message);
void iupEmscripten_Log(const char* restrict format, ...) {
  va_list argList;

  va_start(argList, format);
  char *my_string;

  vasprintf (&my_string, format, argList); // this mallocs under the hood - need to free later
  emjsCommon_Log(my_string);
  /* printf(format, argList); */
  va_end(argList);
  free(my_string);
}

extern void emjsCommon_IupLog(int priority, char* message);
void IupLogV(const char* type, const char* format, va_list arglist)
{
	enum IUPLOG_LEVEL
	{
		IUPLOG_LEVEL_LOG = 0,
		IUPLOG_LEVEL_DEBUG,
		IUPLOG_LEVEL_INFO,
		IUPLOG_LEVEL_WARN,
		IUPLOG_LEVEL_ERROR
	};
	enum IUPLOG_LEVEL priority = IUPLOG_LEVEL_LOG;

	if (iupStrEqualNoCase(type, "DEBUG"))
	{
		priority = IUPLOG_LEVEL_DEBUG;
	}
	else if (iupStrEqualNoCase(type, "ERROR"))
	{
		priority = IUPLOG_LEVEL_ERROR;
	}
	else if (iupStrEqualNoCase(type, "WARNING"))
	{
		priority = IUPLOG_LEVEL_WARN;
	}
	else if (iupStrEqualNoCase(type, "INFO"))
	{
		priority = IUPLOG_LEVEL_INFO;
	}

	char *my_string;
	vasprintf(&my_string, format, arglist); // this mallocs under the hood - need to free later

	emjsCommon_IupLog((int)priority, my_string);

	free(my_string);
	
}

void IupLog(const char* type, const char* format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  IupLogV(type, format, arglist);
  va_end(arglist);
}

extern void emjsCommon_AddWidgetToDialog(int parent_id, int child_id);
extern void emjsCommon_AddCompoundToDialog(int parent_id, int32_t elem_array[], size_t num_elems);
extern void emjsCommon_AddWidgetToWidget(int parent_id, int child_id);
extern void emjsCommon_AddCompoundToWidget(int parent_id, int32_t elem_array[], size_t num_elems);
extern void emjsCommon_SetPosition(int handle_id, int x, int y, int height, int width);
void iupEmscripten_AddWidgetToParent(Ihandle* ih)
{
	Ihandle* parent_ih = iupChildTreeGetNativeParent(ih);
	//InativeHandle* parent_native_handle = iupChildTreeGetNativeParentHandle(ih);
	// No parent? Probably need to assert here.
	if(!parent_ih)
	{
		return;
	}

	InativeHandle* parent_native_handle = parent_ih->handle;
	InativeHandle* child_handle = ih->handle;

	int32_t parent_id = 0;
	int32_t child_id = 0;
	_Bool parent_is_dialog = false;
	if(parent_native_handle)
	{
		parent_id = parent_native_handle->handleID;
		if(parent_ih->iclass->nativetype == IUP_TYPEDIALOG)
		{
			parent_is_dialog = true;
		}
	}
	if(child_handle)
	{
		child_id = child_handle->handleID;
	}
	if(parent_is_dialog)
	{

    /* emjsCommon_Alert("is compound"); */
    /* emjsCommon_Alert(child_handle->isCompound); */
		if (child_handle->isCompound) {
      /* emjsCommon_Alert("compound dialog"); */
      emjsCommon_AddCompoundToDialog(parent_id, child_handle->compoundHandleIDArray, child_handle->numElemsIfCompound);
    }
    else {
      /* emjsCommon_Alert("single dialog"); */
      emjsCommon_AddWidgetToDialog(parent_id, child_id);
    }
	}
	else
	{
		if (child_handle->isCompound) {
      /* emjsCommon_Alert("compound widget"); */
      emjsCommon_AddCompoundToWidget(parent_id, child_handle->compoundHandleIDArray, child_handle->numElemsIfCompound);
    }
    else {
      /* emjsCommon_Alert("single widget"); */
      emjsCommon_AddWidgetToWidget(parent_id, child_id);
    }
	}

}

extern void emjsCommon_SetFgColor(int handle_id, unsigned char r, unsigned char g, unsigned char b);
  // matzy: send to JS?
  

#if 0
  GdkRGBA rgba;

  iupgdkRGBASet(&rgba, r, g, b);

  gtk_widget_override_color(handle, GTK_STATE_FLAG_NORMAL, &rgba);
  gtk_widget_override_color(handle, GTK_STATE_ACTIVE, &rgba);
  gtk_widget_override_color(handle, GTK_STATE_PRELIGHT, &rgba);
  
  GtkRcStyle *rc_style;  
  GdkColor color;

  iupgdkColorSet(&color, r, g, b);

  rc_style = gtk_widget_get_modifier_style(handle);  

  rc_style->fg[GTK_STATE_ACTIVE] = rc_style->fg[GTK_STATE_NORMAL] = rc_style->fg[GTK_STATE_PRELIGHT] = color;
  rc_style->text[GTK_STATE_ACTIVE] = rc_style->text[GTK_STATE_NORMAL] = rc_style->text[GTK_STATE_PRELIGHT] = color;

  rc_style->color_flags[GTK_STATE_NORMAL] |= GTK_RC_TEXT | GTK_RC_FG;
  rc_style->color_flags[GTK_STATE_ACTIVE] |= GTK_RC_TEXT | GTK_RC_FG;
  rc_style->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_TEXT | GTK_RC_FG;

  /* do not set at CHILD_CONTAINER */
  gtk_widget_modify_style(handle, rc_style);
#endif
//}

void iupdrvActivate(Ihandle* ih)
{

}

void iupdrvReparent(Ihandle* ih)
{

	
}


void iupdrvBaseLayoutUpdateMethod(Ihandle *ih)
{
//	iupEmscripten_Log("x:%d, y:%d, w:%d, h:%d, id: %d", ih->x,ih->y,ih->currentwidth,ih->currentheight,ih->handle->handleID);

	if (ih->handle->isCompound)
	{
		for (int i = 0; i < ih->handle->numElemsIfCompound; i++)
		{
			// Our JS implementation uses elem which doesn't exist for Windows (Dialogs). Avoid for safety.
			if(ih->iclass->nativetype != IUP_TYPEDIALOG)
			{
				// Set element's position on screen
				emjsCommon_SetPosition(ih->handle->compoundHandleIDArray[i],ih->x,ih->y,ih->currentwidth,ih->currentheight);
			}
		}
	}
	else
	{
		// Our JS implementation uses elem which doesn't exist for Windows (Dialogs). Avoid for safety.
		if(ih->iclass->nativetype != IUP_TYPEDIALOG)
		{
			// Set element's position on screen
			emjsCommon_SetPosition(ih->handle->handleID,ih->x,ih->y,ih->currentwidth,ih->currentheight);
		}
	}


  //TODO Calculate size and return to ih
  


  /*******************BEGIN IF 0 ***/
	/* id parent_native_handle = iupChildTreeGetNativeParentHandle(ih); */
#if 0
	NSView* parent_view = nil;
	if([parent_native_handle isKindOfClass:[NSWindow class]])
	{
		NSWindow* parent_window = (NSWindow*)parent_native_handle;
		parent_view = [parent_window contentView];
	}
	else if([parent_native_handle isKindOfClass:[NSView class]])
	{
		parent_view = (NSView*)parent_native_handle;
	}
	else
	{
		NSCAssert(1, @"Unexpected type for parent widget");
		@throw @"Unexpected type for parent widget";
	}



	id child_handle = ih->handle;
	NSView* the_view = nil;
	if([child_handle isKindOfClass:[NSView class]])
	{
		the_view = (NSView*)child_handle;
	}
	else if([child_handle isKindOfClass:[CALayer class]])
	{
		NSCAssert(1, @"CALayer not implemented");
		@throw @"CALayer not implemented";
	}
	else
	{
		NSCAssert(1, @"Unexpected type for parent widget");
		@throw @"Unexpected type for parent widget";
	}
	
	
//	iupgtkNativeContainerMove((GtkWidget*)parent, widget, x, y);

//	iupgtkSetPosSize(GTK_CONTAINER(parent), widget, ih->x, ih->y, ih->currentwidth, ih->currentheight);

	/*
	CGSize fitting_size = [the_view fittingSize];
	ih->currentwidth = fitting_size.width;
	ih->currentheight = fitting_size.height;
*/
	
	NSRect parent_rect = [parent_view frame];

	NSRect the_rect = NSMakeRect(
		ih->x,
		// Need to invert y-axis, and also need to shift/account for height of widget because that is also lower-left instead of upper-left.
		parent_rect.size.height - ih->y - ih->currentheight,
		ih->currentwidth,
		ih->currentheight
	);
	[the_view setFrame:the_rect];
//	[the_view setBounds:the_rect];
	
	
#endif
  /***** END IF ***********************************/




}

void iupdrvBaseUnMapMethod(Ihandle* ih)
{
	// Why do I need this when everything else has its own UnMap method?
	//NSLog(@"iupdrvBaseUnMapMethod not implemented. Might be leaking");
}

void iupdrvDisplayUpdate(Ihandle *ih)
{
	// call ViewGroup.invalidate()

}

void iupdrvDisplayRedraw(Ihandle *ih)
{
	iupdrvDisplayUpdate(ih);
}

void iupdrvScreenToClient(Ihandle* ih, int *x, int *y)
{
}



int iupdrvBaseSetZorderAttrib(Ihandle* ih, const char* value)
{
  return 0;
}

void iupdrvSetVisible(Ihandle* ih, int visible)
{
}

int iupdrvIsVisible(Ihandle* ih)
{
	return 1;
}

int iupdrvIsActive(Ihandle *ih)
{
  return 1;
}

void iupdrvSetActive(Ihandle* ih, int enable)
{
}

char* iupdrvBaseGetXAttrib(Ihandle *ih)
{
  return NULL;
}

char* iupdrvBaseGetYAttrib(Ihandle *ih)
{

  return NULL;
}

/*
char* iupdrvBaseGetClientSizeAttrib(Ihandle *ih)
{

    return NULL;

}
 */

int iupdrvBaseSetBgColorAttrib(Ihandle* ih, const char* value)
{

	

  /* DO NOT NEED TO UPDATE GTK IMAGES SINCE THEY DO NOT DEPEND ON BGCOLOR */

  return 1;
}


int iupdrvBaseSetFgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;
  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;
  emjsCommon_SetFgColor(ih->handle->handleID, r, g, b);

  return 1;
}

int iupdrvBaseSetCursorAttrib(Ihandle* ih, const char* value)
{

  return 0;
}


int iupdrvGetScrollbarSize(void)
{

  return 0;
}



void iupdrvBaseRegisterCommonAttrib(Iclass* ic)
{
	/*
#ifndef GTK_MAC
  #ifdef WIN32                                 
    iupClassRegisterAttribute(ic, "HFONT", iupgtkGetFontIdAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  #else
    iupClassRegisterAttribute(ic, "XFONTID", iupgtkGetFontIdAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  #endif
#endif
  iupClassRegisterAttribute(ic, "PANGOFONTDESC", iupgtkGetPangoFontDescAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
*/
}

void iupdrvBaseRegisterVisualAttrib(Iclass* ic)
{
	
}

void iupdrvClientToScreen(Ihandle* ih, int *x, int *y)
{
	
}

void iupdrvPostRedraw(Ihandle *ih)
{

}

void iupdrvRedrawNow(Ihandle *ih)
{

}
void iupdrvSendKey(int key, int press)
{
	
}
void iupdrvSendMouse(int x, int y, int bt, int status)
{
	
}
void iupdrvSleep(int time)
{
	
}
void iupdrvWarpPointer(int x, int y)
{
	
}
