/* IupList: Example in C 
   Creates a dialog with three frames, each one containing a list.
   The first is a simple list, the second one is a multiple list and the last one is a drop-down list.
   The second list has a callback associated.
*/

#include <stdio.h>
#include <iup.h>

int list_multiple_cb (Ihandle *self, char *t, int i, int v)
{
  char message [40] ;

  sprintf (message, "Item %d - %s - %s", i, t, v == 0 ? "deselected" : "selected" );

  IupMessage ("Competed in", message);

  return IUP_DEFAULT;
}

int main(int argc, char **argv) 
{
  Ihandle *dlg;
  Ihandle *list, *list_multiple, *list_dropdown;
  Ihandle *frm_medal, *frm_sport, *frm_prize;

  IupOpen(&argc, &argv);

  list = IupList ("list_act");
  IupSetAttributes (list, "1=Gold, 2=Silver, 3=Bronze, 4=None,"
                          "VALUE=4, SIZE=EIGHTHxEIGHTH");

  frm_medal = IupFrame (list);
  IupSetAttribute (frm_medal, "TITLE", "Best medal");
  list_multiple = IupList(NULL);
  
  IupSetAttributes (list_multiple, "1=\"100m dash\", 2=\"Long jump\", 3=\"Javelin throw\", 4=\"110m hurdlers\", 5=\"Hammer throw\",6=\"High jump\","
                                   "MULTIPLE=YES, VALUE=\"+--+--\", SIZE=EIGHTHxEIGHTH");

  IupSetCallback(list_multiple, "ACTION", (Icallback)list_multiple_cb);
  
  frm_sport = IupFrame (list_multiple);
  
  IupSetAttribute (frm_sport, "TITLE", "Competed in");

  list_dropdown = IupList (NULL);
  
  IupSetAttributes (list_dropdown, "1=\"Less than US$ 1000\", 2=\"US$ 2000\", 3=\"US$ 5000\", 4=\"US$ 10000\", 5=\"US$ 20000\", 6=\"US$ 50000\", 7=\"More than US$ 100000\","
                                   "DROPDOWN=YES, VISIBLE_ITEMS=5");
  
  frm_prize = IupFrame (list_dropdown);
  IupSetAttribute (frm_prize, "TITLE", "Prizes won");

  dlg = IupDialog (IupHbox (frm_medal, frm_sport, frm_prize, NULL));
  IupSetAttribute (dlg, "TITLE", "IupList Example");
  IupShowXY (dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop ();
  IupDestroy(dlg);
  IupClose ();
  return 0;
}