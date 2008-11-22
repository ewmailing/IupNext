/*IUP_IDLE_ACTION Example in C
Creating a program that computes a number’s factorial using the idle function.*/

#include <stdio.h>
#include "iup.h"

/************************************************************************************ 
* structure that stores the computation’s state
************************************************************************************/
static struct
{
  int passo; /* iteration step */
  double fatorial; /* last computed value */
} calc;

/************************************************************************************ 
* end of computation: defines the function associated to the idle function as NULL
************************************************************************************/
static int fim_calculo (void)
{
  IupSetFunction (IUP_IDLE_ACTION, (Icallback) NULL);
  return IUP_DEFAULT;
}

/************************************************************************************ 
* computes one iteration step of the computation; this function will be 
* called every time the user is not interacting with the application
************************************************************************************/
static int idle_function (void)
{
  static char str[80]; /* must be static because will be passed*/

  /* "TITLE" value for IUP */
  calc.passo++; /* next iteration step */
  calc.fatorial *= calc.passo; /* executes one computation step */

  /* feedback to the user on the current step and the last computed value */
  sprintf (str, "%d -> %10.4g",calc.passo,calc.fatorial);
  IupStoreAttribute (IupGetHandle("mens"), "VALUE", str);

  if (calc.passo == 100) /* computation ends when step = 100 */
   fim_calculo();

  return IUP_DEFAULT;
}

/************************************************************************************ 
* begin computation: initializes initial values and sets idle function
************************************************************************************/
static int inicio_calculo (void)
{
  calc.passo = 0;
  calc.fatorial = 1.0;
  IupSetFunction (IUP_IDLE_ACTION, (Icallback) idle_function);
  IupSetAttribute (IupGetHandle("mens"), "VALUE", "Computing...");
  return IUP_DEFAULT;
}

/************************************************************************************ 
* main program
************************************************************************************/
void main(int argc, char **argv)
{
  Ihandle *text, *bt, *dg;
  IupOpen(&argc, &argv);
 
  text = IupText("");
  IupSetAttribute(text, "SIZE", "100x20");
  IupSetHandle("mens", text);
  bt = IupButton("Calcular", "calcula");

  dg = IupDialog(IupVbox(text, bt, NULL));

  IupSetFunction ("calcula",(Icallback) inicio_calculo);
  IupShowXY(dg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupDestroy(dg);
  IupClose();
}
