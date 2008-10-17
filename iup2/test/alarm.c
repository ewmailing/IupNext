/* IupAlarm Example */

/* IUP libraries include */

#include <stdlib.h>
#include <stdio.h>

#include "iup.h"

/* Main program */
int main(int argc, char **argv)
{
  /* Initializes IUP */
  IupOpen(&argc, &argv);

  /* Executes IupAlarm */
  switch (IupAlarm ("IupAlarm Example",
    "File not saved! Save it now?", "Yes", "No", "Cancel"))
  {
    /* Shows a message for each selected button */
    case 1:
      IupMessage ("Save file", "File saved sucessfully - leaving program") ;
    break ;

    case 2:
      IupMessage ("Save file", "File not saved - leaving program anyway") ;
    break ;

    case 3:
      IupMessage ("Save file", "Operation canceled") ;
    break ;
  }
  
  /* Initializes IUP main loop */
  IupMainLoop () ;

  /* Finishes IUP */
  IupClose () ;

  /* Program finished sucessfully */
  return 0 ;
}


