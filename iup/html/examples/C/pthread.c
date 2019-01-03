/**************************************************************************************************
 * IUP program that runs a "long" function in a second thread when the user clicks the button.    *
 * cc iup_pthread.c -o iup_pthread -W -pedantic -ansi -std=c99 -I/usr/include/iup -liup -lpthread *
 * Contribution by Daniel G.                                                                      *
 **************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iup.h>

pthread_t thread1;
int  iret1;

void *worker_function( void *ptr )
{
	/**************************************************************************************
	 * Do not call IUP functions from other threads as IUP 3 is currently not thread-safe *
	 **************************************************************************************/
	for (int i = 0; i < 100; i++) {
		sleep(5);
		printf("%s \n", "worker function");
	}
  
  // TODO: show lock/unlock sample
  
	return NULL;
}

int btn_cb( Ihandle *self )
{
	/************************************************************************************
	 * Do not use variables created on the callback function as they will lose context  *
	 * as soon as the callback function reach the return point.                         *
	 * Use global variables instead if you need to pass arguments to the second thread. *
   * But the acess to there variable must be thread safe using a lock,                *
   * for instance pthread_mutex_lock and pthread_mutex_unlock                         * 
	 ************************************************************************************/
	iret1 = pthread_create( &thread1, NULL, worker_function, NULL);
	pthread_detach(thread1);
	return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *button, *label, *vbox;

  IupOpen(&argc, &argv);
  
  label =  IupLabel("Start second thread:");
  button = IupButton("Start", NULL);
  vbox = IupVbox(
    label,
    button,
    NULL);
  IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
  IupSetAttribute(vbox, "GAP", "10");
  IupSetAttribute(vbox, "MARGIN", "10x10");
  
  dlg = IupDialog(vbox);
  IupSetAttribute(dlg, "TITLE", "pthread test");

  /* Registers callbacks */
  IupSetCallback(button, "ACTION", (Icallback) btn_cb);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();

  IupClose();
  return EXIT_SUCCESS;
}
