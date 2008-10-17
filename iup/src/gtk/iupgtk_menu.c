/** \file
 * \brief Menu Resources
 *
 * See Copyright Notice in iup.h
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_attrib.h"
#include "iup_dialog.h"
#include "iup_str.h"
#include "iup_label.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_image.h"
#include "iup_menu.h"

#include "iupgtk_drv.h"


typedef struct _ImenuPos
{
  int x, y;
} ImenuPos;

static void gtkMenuPositionFunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, ImenuPos *menupos)
{
  *x = menupos->x;
  *y = menupos->y;
  *push_in = FALSE;
  (void)menu;
}

int iupdrvMenuPopup(Ihandle* ih, int x, int y)
{
  ImenuPos menupos;
  menupos.x = x;
  menupos.y = y;
  gtk_menu_popup((GtkMenu*)ih->handle, NULL, NULL, (GtkMenuPositionFunc)gtkMenuPositionFunc,
                 (gpointer)&menupos, 0, gtk_get_current_event_time());
  gtk_main();
  return IUP_NOERROR;
}

int iupdrvMenuGetMenuBarSize(Ihandle* ih)
{
  int ch;
  iupdrvFontGetCharSize(ih, NULL, &ch);
#ifdef WIN32
  return 3 + ch + 3;
#else
  return 4 + ch + 4;
#endif
}

static void gtkItemUpdateImage(Ihandle* ih, const char* value, const char* image, const char* impress)
{
  GdkPixbuf* pixbuf;

  if (!impress || !iupStrBoolean(value))
    pixbuf = iupImageGetImage(image, ih, 0, "IMAGE");
  else
    pixbuf = iupImageGetImage(impress, ih, 0, "IMPRESS");

  if (pixbuf)
  {
    GtkWidget* image_label = gtk_image_menu_item_get_image((GtkImageMenuItem*)ih->handle);
    if (!image_label)
    {
      image_label = gtk_image_new();
      gtk_image_menu_item_set_image((GtkImageMenuItem*)ih->handle, image_label);
    }

    if (pixbuf != gtk_image_get_pixbuf((GtkImage*)image_label))
      gtk_image_set_from_pixbuf((GtkImage*)image_label, pixbuf);
  }
  else
    gtk_image_menu_item_set_image((GtkImageMenuItem*)ih->handle, NULL);
}


/*******************************************************************************************/


static void gtkMenuMap(GtkWidget *widget, Ihandle* ih)
{
  Icallback cb = IupGetCallback(ih, "OPEN_CB");
  if (cb)
    cb(ih);

  (void)widget;
}

static void gtkMenuUnMap(GtkWidget *widget, Ihandle* ih)
{
  Icallback cb = IupGetCallback(ih, "MENUCLOSE_CB");
  if (cb)
    cb(ih);

  (void)widget;
}

static void gtkPopupMenuUnMap(GtkWidget *widget, Ihandle* ih)
{
  gtkMenuUnMap(widget, ih);

  /* quit the popup loop */
  gtk_main_quit();
}

static void gtkItemSelect(GtkWidget *widget, Ihandle* ih)
{
  Icallback cb = IupGetCallback(ih, "HIGHLIGHT_CB");
  if (cb)
    cb(ih);

  cb = IupGetCallback(ih, "HELP_CB");
  if (cb)
    gtk_menu_set_active((GtkMenu*)ih->parent->handle, IupGetChildPos(ih->parent, ih));

  (void)widget;
}

static void gtkItemActivate(GtkWidget *widget, Ihandle* ih)
{
  Icallback cb;

  if (GTK_IS_CHECK_MENU_ITEM(ih->handle) && !iupAttribGetInt(ih, "AUTOTOGGLE") && !iupAttribGetInt(ih->parent, "RADIO"))
  {
    /* GTK by default will do autotoggle */
    g_signal_handlers_block_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkItemActivate), ih);
    gtk_check_menu_item_set_active((GtkCheckMenuItem*)ih->handle, !gtk_check_menu_item_get_active((GtkCheckMenuItem*)ih->handle));
    g_signal_handlers_unblock_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkItemActivate), ih);
  }

  if (GTK_IS_IMAGE_MENU_ITEM(ih->handle))
  {
    if (iupAttribGetInt(ih, "AUTOTOGGLE"))
    {
      if (iupStrBoolean(iupAttribGetStr(ih, "VALUE")))
        iupAttribSetStr(ih, "VALUE", "OFF");
      else
        iupAttribSetStr(ih, "VALUE", "ON");

      gtkItemUpdateImage(ih, iupAttribGetStr(ih, "VALUE"), iupAttribGetStr(ih, "IMAGE"), iupAttribGetStr(ih, "IMPRESS"));
    }
  }

  cb = IupGetCallback(ih, "ACTION");
  if (cb && cb(ih)==IUP_CLOSE)
    IupExitLoop();

  (void)widget;
}

static gboolean gtkMenuKeyPressEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  if (evt->keyval == GDK_F1)
  {
    Ihandle* child;
    GtkWidget* active = gtk_menu_get_active((GtkMenu*)widget);
    for (child=ih->firstchild; child; child=child->brother)
    {
      if (child->handle == active)
        iupgtkShowHelp(NULL, NULL, child);
    }
  }

  (void)widget;
  (void)evt;
  return FALSE;
}


/*******************************************************************************************/


static int gtkMenuMapMethod(Ihandle* ih)
{
  if (iupMenuIsMenuBar(ih))
  {
    /* top level menu used for MENU attribute in IupDialog (a menu bar) */

    ih->handle = gtk_menu_bar_new();
    if (!ih->handle)
      return IUP_ERROR;

    iupgtkBaseAddToParent(ih);
  }
  else
  {
    ih->handle = gtk_menu_new();
    if (!ih->handle)
      return IUP_ERROR;

    if (ih->parent)
    {
      /* parent is a submenu */
      gtk_menu_item_set_submenu((GtkMenuItem*)ih->parent->handle, ih->handle);

      g_signal_connect(G_OBJECT(ih->handle), "map", G_CALLBACK(gtkMenuMap), ih);
      g_signal_connect(G_OBJECT(ih->handle), "unmap", G_CALLBACK(gtkMenuUnMap), ih);
    }
    else
    {
      /* top level menu used for IupPopup */
      iupAttribSetStr(ih, "_IUPGTK_POPUP_MENU", "1");

      g_signal_connect(G_OBJECT(ih->handle), "map", G_CALLBACK(gtkMenuMap), ih);
      g_signal_connect(G_OBJECT(ih->handle), "unmap", G_CALLBACK(gtkPopupMenuUnMap), ih);
    }
  }

  gtk_widget_add_events(ih->handle, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(ih->handle), "key-press-event", G_CALLBACK(gtkMenuKeyPressEvent), ih);

  ih->serial = iupMenuGetChildId(ih); 
  gtk_widget_show(ih->handle);

  return IUP_NOERROR;
}

void iupdrvMenuInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkMenuMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iupdrvBaseSetBgColorAttrib, NULL, IUP_MAPPED, IUP_INHERIT);
}


/*******************************************************************************************/

static int gtkItemSetTitleImageAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_IMAGE_MENU_ITEM(ih->handle))
  {
    gtkItemUpdateImage(ih, NULL, value, NULL);
    return 1;
  }
  else
    return 0;
}

static int gtkItemSetImageAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_IMAGE_MENU_ITEM(ih->handle))
  {
    gtkItemUpdateImage(ih, iupAttribGetStr(ih, "VALUE"), value, iupAttribGetStr(ih, "IMPRESS"));
    return 1;
  }
  else
    return 0;
}

static int gtkItemSetImpressAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_IMAGE_MENU_ITEM(ih->handle))
  {
    gtkItemUpdateImage(ih, iupAttribGetStr(ih, "VALUE"), iupAttribGetStr(ih, "IMAGE"), value);
    return 1;
  }
  else
    return 0;
}

static int gtkItemSetTitleAttrib(Ihandle* ih, const char* value)
{
  char *str;
  GtkWidget* label;

  if (!value) value = "     ";
  str = iupMenuGetTitle(ih, value);

  label = gtk_bin_get_child((GtkBin*)ih->handle);

  iupgtkSetMnemonicTitle(ih, (GtkLabel*)label, str);

  if (str != value) free(str);
  return 1;
}

static int gtkItemSetValueAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_CHECK_MENU_ITEM(ih->handle))
  {
    if (iupAttribGetInt(ih->parent, "RADIO"))
      value = "ON";

    g_signal_handlers_block_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkItemActivate), ih);
    gtk_check_menu_item_set_active((GtkCheckMenuItem*)ih->handle, iupStrBoolean(value));
    g_signal_handlers_unblock_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkItemActivate), ih);
    return 0;
  }
  else
  {
    gtkItemUpdateImage(ih, value, iupAttribGetStr(ih, "IMAGE"), iupAttribGetStr(ih, "IMPRESS"));
    return 1;
  }
}

static char* gtkItemGetValueAttrib(Ihandle* ih)
{
  if (GTK_IS_CHECK_MENU_ITEM(ih->handle))
  {
    if (gtk_check_menu_item_get_active((GtkCheckMenuItem*)ih->handle))
      return "ON";
    else
      return "OFF";
  }
  else
    return NULL;
}

static int gtkItemMapMethod(Ihandle* ih)
{
  int pos;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupMenuIsMenuBar(ih->parent))
    ih->handle = gtk_menu_item_new_with_label("");
  else
  {
    if (iupAttribGetStr(ih, "IMAGE")||iupAttribGetStr(ih, "TITLEIMAGE"))
      ih->handle = gtk_image_menu_item_new_with_label("");
    else if (iupAttribGetInt(ih->parent, "RADIO"))
    {
      GtkRadioMenuItem* last_tg = (GtkRadioMenuItem*)iupAttribGetStr(ih->parent, "_IUPGTK_LASTRADIOITEM");
      if (last_tg)
        ih->handle = gtk_radio_menu_item_new_with_label_from_widget(last_tg, "");
      else
        ih->handle = gtk_radio_menu_item_new_with_label(NULL, "");
      iupAttribSetStr(ih->parent, "_IUPGTK_LASTRADIOITEM", (char*)ih->handle);
    }
    else
      ih->handle = gtk_check_menu_item_new_with_label("");
  }

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupMenuGetChildId(ih); 

  g_signal_connect(G_OBJECT(ih->handle), "select", G_CALLBACK(gtkItemSelect), ih);
  g_signal_connect(G_OBJECT(ih->handle), "activate", G_CALLBACK(gtkItemActivate), ih);

  pos = IupGetChildPos(ih->parent, ih);
  gtk_menu_shell_insert((GtkMenuShell*)ih->parent->handle, ih->handle, pos);
  gtk_widget_show(ih->handle);

  iupUpdateStandardFontAttrib(ih);

  return IUP_NOERROR;
}

void iupdrvItemInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkItemMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Common */
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iupdrvSetStandardFontAttrib, IupGetGlobal("DEFAULTFONT"), IUP_NOT_MAPPED, IUP_INHERIT);  /* use inheritance to retrieve standard fonts */

  /* Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupBaseSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);

  /* IupItem only */
  iupClassRegisterAttribute(ic, "VALUE", gtkItemGetValueAttrib, gtkItemSetValueAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, gtkItemSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEIMAGE", NULL, gtkItemSetTitleImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, gtkItemSetImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMPRESS", NULL, gtkItemSetImpressAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}


/*******************************************************************************************/


static int gtkSubmenuSetImageAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_IMAGE_MENU_ITEM(ih->handle))
  {
    gtkItemUpdateImage(ih, NULL, value, NULL);
    return 1;
  }
  else
    return 0;
}

static int motSubmenuMapMethod(Ihandle* ih)
{
  int pos;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupMenuIsMenuBar(ih->parent))
    ih->handle = gtk_menu_item_new_with_label("");
  else
    ih->handle = gtk_image_menu_item_new_with_label("");

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupMenuGetChildId(ih); 

  pos = IupGetChildPos(ih->parent, ih);
  gtk_menu_shell_insert((GtkMenuShell*)ih->parent->handle, ih->handle, pos);
  gtk_widget_show(ih->handle);

  g_signal_connect(G_OBJECT(ih->handle), "select", G_CALLBACK(gtkItemSelect), ih);

  iupUpdateStandardFontAttrib(ih);

  return IUP_NOERROR;
}

void iupdrvSubmenuInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motSubmenuMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Common */
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iupdrvSetStandardFontAttrib, IupGetGlobal("DEFAULTFONT"), IUP_NOT_MAPPED, IUP_INHERIT);  /* use inheritance to retrieve standard fonts */

  /* Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupBaseSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);

  /* IupSubmenu only */
  iupClassRegisterAttribute(ic, "TITLE", NULL, gtkItemSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, gtkSubmenuSetImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}


/*******************************************************************************************/


static int gtkSeparatorMapMethod(Ihandle* ih)
{
  int pos;

  if (!ih->parent)
    return IUP_ERROR;

  ih->handle = gtk_separator_menu_item_new();
  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupMenuGetChildId(ih); 

  pos = IupGetChildPos(ih->parent, ih);
  gtk_menu_shell_insert((GtkMenuShell*)ih->parent->handle, ih->handle, pos);
  gtk_widget_show(ih->handle);

  return IUP_NOERROR;
}

void iupdrvSeparatorInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkSeparatorMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;
}
