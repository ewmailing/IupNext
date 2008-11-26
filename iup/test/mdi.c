#include <stdlib.h>
#include <stdio.h>
#include <iup.h>


static int mdi_tilehoriz(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "TILEHORIZONTAL");
  return IUP_DEFAULT;
}

static int mdi_tilevert(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "TILEVERTICAL");
  return IUP_DEFAULT;
}

static int mdi_cascade(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "CASCADE");
  return IUP_DEFAULT;
}

static int mdi_icon(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "ICON");
  return IUP_DEFAULT;
}

static int mdi_next(Ihandle* self)
{
  IupSetAttribute(self, "MDIACTIVATE", "NEXT");
  return IUP_DEFAULT;
}

static int mdi_previous(Ihandle* self)
{
  IupSetAttribute(self, "MDIACTIVATE", "PREVIOUS");
  return IUP_DEFAULT;
}

static int mdi_closeall(Ihandle* self)
{
  IupSetAttribute(self, "MDICLOSEALL", NULL);
  return IUP_DEFAULT;
}

static int mdi_activate(Ihandle* self)
{
  printf("mdi_activate(%s)\n", IupGetName(self));
  return IUP_DEFAULT;
}

static int mdi_new(Ihandle* self)
{
  Ihandle *box, *cnv, *dlg;

  cnv = IupCanvas(NULL);
  IupSetAttribute(cnv,"BGCOLOR","128 255 0");

  box = IupVbox(cnv, NULL);
  IupSetAttribute(box,"MARGIN","5x5");

  dlg = IupDialog(box);
  IupSetAttribute(dlg,"TITLE","MDI Child");
  IupSetAttribute(dlg,"MDICHILD","YES");
  IupSetAttribute(dlg,"MDICLIENT","mdiClient");
  IupSetCallback(dlg,"MDIACTIVATE_CB",(Icallback)mdi_activate);
//  IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");

  IupShow(dlg);

  return IUP_DEFAULT;
}

static void createMenu(void)
{
  Ihandle* winmenu;
  Ihandle* mnu = IupMenu(
  IupSubmenu("MDI",IupMenu(
      IupItem("New", "mdi_new"), 
      NULL)),
  winmenu = IupSubmenu("Window", IupMenu(
      IupItem("Tile Horizontal", "mdi_tilehoriz"), 
      IupItem("Tile Vertical", "mdi_tilevert"), 
      IupItem("Cascade", "mdi_cascade"), 
      IupItem("Icon Arrange", "mdi_icon"), 
      IupItem("Close All", "mdi_closeall"), 
      IupSeparator(),
      IupItem("Next", "mdi_next"), 
      IupItem("Previous", "mdi_previous"), 
      NULL)),
    NULL);
  IupSetHandle("mnu",mnu);
  IupSetHandle("mdiMenu",winmenu);

  IupSetFunction("mdi_new", (Icallback)mdi_new);
  IupSetFunction("mdi_tilehoriz", (Icallback)mdi_tilehoriz);
  IupSetFunction("mdi_tilevert", (Icallback)mdi_tilevert);
  IupSetFunction("mdi_cascade", (Icallback)mdi_cascade);
  IupSetFunction("mdi_icon", (Icallback)mdi_icon);
  IupSetFunction("mdi_next", (Icallback)mdi_next);
  IupSetFunction("mdi_previous", (Icallback)mdi_previous);
  IupSetFunction("mdi_closeall", (Icallback)mdi_closeall);
}

static Ihandle* createFrame(void)
{
  Ihandle *dlg, *cnv;
  cnv = IupCanvas( NULL);
  IupSetAttribute(cnv,"MDICLIENT","YES");
  IupSetHandle("mdiClient", cnv);

  dlg = IupDialog(cnv);
  IupSetAttribute(dlg,"TITLE","MDI Frame");
//  IupSetAttribute(dlg,"MDIMENU","mdiMenu");
  IupSetAttribute(dlg,"RASTERSIZE","800x600");

  return dlg;
}

int main(int argc, char* argv[])
{
  Ihandle* dlg;

  IupOpen(&argc, &argv);      

//  createMenu();

  dlg = createFrame();
//  IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");
//  IupSetAttribute(dlg,IUP_MENU,"mnu");
  IupShow(dlg);
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();  
  return 0;
}
