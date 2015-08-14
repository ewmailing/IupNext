#include <stdlib.h>
#include <stdio.h>

#include "simple_paint_util.h"

#include <iup.h>
#include <iupim.h>


static int SplashTimer_CB(Ihandle* timer)
{
  if (IupGetAttribute(timer, "_FIRST_STAGE"))
  {
    IupSetAttribute(timer, "RUN", "NO");
    IupSetAttribute(timer,"TIME", "1000");
    IupSetAttribute(timer, "RUN", "YES");
    IupSetAttribute(timer, "_FIRST_STAGE", NULL);
  }
  else
  {
    Ihandle* dlg = (Ihandle*)IupGetAttribute(timer, "_DIALOG");
    IupSetAttribute(timer, "RUN", "NO");
    IupDestroy(dlg);
    IupDestroy(timer);
  }
  return IUP_DEFAULT;
}

void IupShowSplashDlg(const char* image)
{
  Ihandle *dlg, *timer;

#if _OLD_
  Ihandle* lbl = IupLabel(NULL);
  IupSetAttribute(lbl, "IMAGE", image);

  dlg = IupDialog(lbl);
  IupSetAttribute(dlg, "BGCOLOR", "255 255 255");
#else
  dlg = IupDialog(NULL);
  IupSetAttribute(dlg, "RASTERSIZE", "256x256");
  IupSetAttribute(dlg, "BORDER", "NO");
#endif

  IupSetAttribute(dlg,"RESIZE", "NO");
  IupSetAttribute(dlg,"MINBOX", "NO");
  IupSetAttribute(dlg,"MAXBOX", "NO");
  IupSetAttribute(dlg,"MENUBOX", "NO");
  IupSetAttribute(dlg,"TOPMOST", "YES");

  /* show the splash for 1 second without other windows,
     then start to show the main window. */

  timer = IupTimer();
  IupSetAttribute(timer,"TIME", "1000");
  IupSetCallback(timer, "ACTION_CB", SplashTimer_CB);
  IupSetAttribute(timer, "_DIALOG", (char*)dlg);
  IupSetAttribute(timer, "_FIRST_STAGE", "YES");
  IupSetAttribute(timer, "RUN", "YES");

#ifndef WIN32  // Set before map in GTK
  IupSetAttribute(dlg, "OPACITYIMAGE", image);
#endif

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

#ifdef WIN32   // Set after show in Windows
  IupSetAttribute(dlg, "OPACITYIMAGE", image);
#endif

  while(IupGetAttribute(timer, "_FIRST_STAGE"))
    IupLoopStep();
}

void SimplePaintSplash(const char* argv0)
{
  char* file_path = str_filepath(argv0);
  char filename[10240];
  Ihandle* image;

  if (file_path)
    sprintf(filename, "%s/%s", file_path, "TecgrafLogo.png");
  else
    sprintf(filename, "%s", "TecgrafLogo.png");

  image = IupLoadImage(filename);
  if (!image && file_path)
  {
    char* up_file_path = str_filepath(file_path);
    if (up_file_path)
    {
      sprintf(filename, "%s/%s", up_file_path, "TecgrafLogo.png");
      image = IupLoadImage(filename);
      delete up_file_path;
    }
  }

  if (file_path) delete file_path;

  if (image)
  {                                
    IupSetAttribute(image, "AUTOSCALE", "1");
    IupSetHandle("SPLASH", image);
    IupShowSplashDlg("SPLASH");
  }
}
