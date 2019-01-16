#include <stdlib.h>
#include <stdio.h>
#include <iup.h>
#include <iupim.h>

int quit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dialog, *quit_bt, *vbox, *image;

  IupOpen(&argc, &argv);

//  image = IupLoadImage("D:\\tecgraf\\iup\\html\\examples\\C\\balloon.png");
  image = IupLoadImage("/tecgraf/iup/html/examples/C/balloon.png");
//  image = IupLoadImage("/tecgraf/iup/bin/Debug_64/balloon.png");

  /* Creating the button */ 
  quit_bt = IupButton("Quit", 0);
  IupSetCallback(quit_bt, "ACTION", (Icallback)quit_cb);

  /* the container with a label and the button */
  vbox = IupVbox(
           IupSetAttributes(IupFlatLabel("Very Long Text Label"), "EXPAND=YES, ALIGNMENT=ACENTER, FONTSIZE=28"), 
           quit_bt,
           NULL);
  IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
  IupSetAttribute(vbox, "MARGIN", "200x200");
  IupSetAttribute(vbox, "GAP", "5");

  vbox = IupBackgroundBox(vbox);
  IupSetAttribute(vbox, "RASTERSIZE", "804x644");
//  IupSetAttribute(vbox, "BACKCOLOR", "255 255 255");
  IupSetAttribute(vbox, "BGCOLOR", "255 255 255");
  IupSetAttributeHandle(vbox, "BACKIMAGE", image);

  /* Creating the dialog */ 
  dialog = IupDialog(vbox);
  IupSetAttributeHandle(dialog, "DEFAULTESC", quit_bt);

  IupSetAttribute(dialog, "BORDER", "NO");
  IupSetAttribute(dialog, "RESIZE", "NO");
  IupSetAttribute(dialog, "MINBOX", "NO");
  IupSetAttribute(dialog, "MAXBOX", "NO");
  IupSetAttribute(dialog, "MENUBOX", "NO");
  IupSetAttributeHandle(dialog, "SHAPEIMAGE", image);
//  IupSetAttributeHandle(dialog, "OPACITYIMAGE", image);
//  IupSetAttribute(dialog, "OPACITY", "128");

  IupShow(dialog);

  IupMainLoop();
  
  IupDestroy(dialog);
  IupClose();

  return EXIT_SUCCESS;
}
