/* IupMessage Example */

/* IUP libraries include */
#include "iup.h"

/* Main program */
int main(int argc, char **argv)
{
  /* Initializes IUP */
  IupOpen(&argc, &argv);
  
  /* Executes IupMessage */
  IupMessage("IupMessage Example", "Press the button") ;

  /* Finishes IUP */
  IupClose () ;

  /* Program finished sucessfully */
  return 0 ;
}