/*IupToggle Example in C 
Creates 9 toggles: the first one has an image and an associated callback; the second has an image and is deactivated; the third is regular; the fourth has its foreground color changed; the fifth has its background color changed; the sixth has its foreground and background colors changed; the seventh is deactivated; the eight has its font changed; the ninth has its size changed.. */


#include <stdio.h>
#include "iup.h"          
                           
static unsigned char img_1[] = 
{
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static unsigned char img_2[] = 
{
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,
  1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


void createimgs(void)
{ 
  Ihandle *img1, *img2;
  img1 = IupImage(16,16, img_1);
  img2 = IupImage(16,16, img_2);
  IupSetHandle ("img1", img1); 
  IupSetHandle ("img2", img2);
  IupSetAttribute (img1, "1", "255 255 255"); 
  IupSetAttribute (img1, "2", "0 192 0"); 
  IupSetAttribute (img2, "1", "255 255 255"); 
  IupSetAttribute (img2, "2", "0 192 0"); 
}

int toggle1cb(Ihandle *self, int v)
{
  if(v == 1)IupMessage("Toggle 1","pressed"); 
    else IupMessage("Toggle 1","released"); 

  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *toggles, *dlg, *caixa ;
  Ihandle *toggle1, *toggle2, *toggle3, *toggle4, *toggle5, *toggle6, *toggle7, *toggle8, *toggle9 ;
  IupOpen(&argc, &argv);
  createimgs();

  toggle1 = IupToggle("toggle with image", NULL);
  toggle2 = IupToggle("deactivated toggle with image", NULL);
  toggle3 = IupToggle("regular toggle", NULL);
  toggle4 = IupToggle("toggle with blue foreground color", NULL);
  toggle5 = IupToggle("toggle with red background color", NULL);
  toggle6 = IupToggle("toggle with black background color and green foreground color", NULL);
  toggle7 = IupToggle("deactivated toggle", NULL);
  toggle8 = IupToggle("toggle with Courier 14 Bold font", NULL);
  toggle9 = IupToggle("toggle with size EIGHTxEIGHT", NULL);
	
  IupSetCallback(toggle1, "ACTION", (Icallback) toggle1cb);
  
  IupSetAttribute(toggle1, IUP_IMAGE,   "img1");              /* Toggle 1 uses image                 */
  IupSetAttribute(toggle2, IUP_IMAGE,   "img2");              /* Toggle 2 uses image                 */
  IupSetAttribute(toggle2, IUP_ACTIVE,  IUP_NO);              /* Toggle 2 inactive                   */
  IupSetAttribute(toggle4, IUP_FGCOLOR, "0 0 255");           /* Toggle 4 has blue foreground color  */
  IupSetAttribute(toggle5, "BGCOLOR", "255 0 0");           /* Toggle 5 has red background color   */
  IupSetAttribute(toggle6, IUP_FGCOLOR, "0 255 0");           /* Toggle 6 has green foreground color */
  IupSetAttribute(toggle6, "BGCOLOR", "0 0 0");             /* Toggle 6 has black background color */
  IupSetAttribute(toggle7, IUP_ACTIVE,  IUP_NO);              /* Toggle 7 inactive                   */
  IupSetAttribute(toggle8, IUP_FONT,    IUP_COURIER_BOLD_14); /* Toggle 8 has Courier 14 Bold font   */
  IupSetAttribute(toggle9, "SIZE",    "EIGHTHxEIGHTH");     /* Toggle 9 has size EIGHTxEIGHT       */

  /* Creating box that contains the toggles */
  caixa = 
    IupVbox (
      toggle1, 
      toggle2, 
      toggle3, 
      toggle4, 
      toggle5, 
      toggle6, 
	  toggle7, 
      toggle8, 
      toggle9, 
      NULL  );

  toggles = IupRadio( caixa ); /* Grouping toggles */
  IupSetAttribute(toggles, "EXPAND", IUP_YES);

  /* Dialog */
  dlg = IupDialog(toggles);
  IupSetAttribute (dlg, "TITLE", "IupToggle"); 
  IupSetAttributes(dlg, "MARGIN=5x5, GAP=5, RESIZE=NO"); 

  /* Associates a menu to the dialog */
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER); 
  IupMainLoop();
  IupDestroy(dlg);
  IupDestroy(IupGetHandle ("img1")); 
  IupDestroy(IupGetHandle ("img2")); 
  IupClose();

  return 0 ;
}