/** \file
 * \brief GTK Driver iupdrvSetGlobal
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"

#include "iupgtk_drv.h"


int iupgtk_utf8autoconvert = 1;

int iupdrvSetGlobal(const char *name, const char *value)
{
  if (iupStrEqual(name, "UTF8AUTOCONVERT"))
  {
    if (!value || iupStrBoolean(value))
      iupgtk_utf8autoconvert = 1;
    else
      iupgtk_utf8autoconvert = 0;
    return 0;
  }
  return 1;
}

int iupdrvCheckMainScreen(int *w, int *h)
{
  GdkScreen *screen = gdk_screen_get_default();
  int monitors_count = gdk_screen_get_n_monitors(screen);
  if (monitors_count > 1)
  {
    GdkRectangle rect;
    gdk_screen_get_monitor_geometry(screen, gdk_screen_get_monitor_at_point(screen, 0, 0), &rect);
    *w = rect.width;
    *h = rect.height;
    return 1;
  }
  return 0;
}

char *iupdrvGetGlobal(const char *name)
{
  if (iupStrEqual(name, "VIRTUALSCREEN"))
  {
    char *str = iupStrGetMemory(50);
    GdkScreen *screen = gdk_screen_get_default();
    GdkWindow *root = gdk_screen_get_root_window(gdk_screen_get_default());
    int x = 0;
    int y = 0;
    int w = gdk_screen_get_width(screen); 
    int h = gdk_screen_get_height(screen);
    gdk_window_get_root_origin(root, &x, &y);
    sprintf(str, "%d %d %d %d", x, y, w, h);
    return str;
  }
  if (iupStrEqual(name, "MONITORSINFO"))
  {
    int i;
    GdkScreen *screen = gdk_screen_get_default();
    int monitors_count = gdk_screen_get_n_monitors(screen);
    char *str = iupStrGetMemory(monitors_count*50);
    char* pstr = str;
    GdkRectangle rect;

    for (i=0; i < monitors_count; i++)
    {
      gdk_screen_get_monitor_geometry(screen, i, &rect);
      pstr += sprintf(pstr, "%d %d %d %d\n", rect.x, rect.y, rect.width, rect.height);
    }

    return str;
  }
  if (iupStrEqual(name, "TRUECOLORCANVAS"))
  {
    if (gdk_visual_get_best_depth() > 8)
      return "YES";
    else
      return "NO";
  }
  if (iupStrEqual(name, "UTF8AUTOCONVERT"))
  {
    if (iupgtk_utf8autoconvert)
      return "YES";
    else
      return "NO";
  }
  return NULL;
}
