/** \file
 * \brief winmain for iupstub
 *
 * See Copyright Notice in iup.h
 */

#include <windows.h>
#include <stdlib.h>

/*  this works for the MSVCRT.DLL
    __argc, __argv are exported in the <stdlib.h>
    main is an application defined function
*/

extern int main(int, char **);

int PASCAL WinMain (HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int ncmdshow)
{
 return main(__argc, __argv);
}
