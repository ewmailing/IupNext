/** \file
 * \brief global attributes enviroment
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>      
#include <stdio.h>      
#include <string.h>      
#include <time.h>

#include "iup.h" 
#include "iup_key.h"

#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_drvinfo.h"
#include "iup_str.h"


static FILE* irec_file = NULL;
static int irec_lastclock = 0;

static int iRecClock(void)
{
  return (int)((clock()*1000)/CLOCKS_PER_SEC); /* time in miliseconds */
}

static void iRecInputWheelCB(float p1, int p2, int p3, char* p4)
{
  (void)p4;
  if (irec_file)
  {
    int time = iRecClock() - irec_lastclock;
    fwrite("WHE", 1, 3, irec_file);
    fwrite(&time, sizeof(int), 1, irec_file);
    fwrite(&p1, sizeof(float), 1, irec_file);
    fwrite(&p2, sizeof(int), 1, irec_file);
    fwrite(&p3, sizeof(int), 1, irec_file);
    /* fwrite(p4, 1, IUPKEY_STATUS_SIZE, irec_file); */
    fwrite("\n", 1, 1, irec_file);
    irec_lastclock = iRecClock();
  }
}

static void iRecInputButtonCB(int p1, int p2, int p3, int p4, char* p5)
{
  (void)p5;
  if (irec_file)
  {
    char p1c = (char)p1;
    char p2c = (char)p2;
    int time = iRecClock() - irec_lastclock;
    if (p2==1 && iup_isdouble(p5)) p2c = 2;
    fwrite("BUT", 1, 3, irec_file);
    fwrite(&time, sizeof(int), 1, irec_file);
    fwrite(&p1c, 1, 1, irec_file);
    fwrite(&p2c, 1, 1, irec_file);
    fwrite(&p3, sizeof(int), 1, irec_file);
    fwrite(&p4, sizeof(int), 1, irec_file);
    /* fwrite(p5, 1, IUPKEY_STATUS_SIZE, irec_file); */
    fwrite("\n", 1, 1, irec_file);
    irec_lastclock = iRecClock();
  }
}

static void iRecInputMotionCB(int p1, int p2, char* p3)
{
  (void)p3;
  if (irec_file)
  {
    char p3c = 0;
    int time = iRecClock() - irec_lastclock;
    fwrite("MOV", 1, 3, irec_file);
    fwrite(&time, sizeof(int), 1, irec_file);
    fwrite(&p1, sizeof(int), 1, irec_file);
    fwrite(&p2, sizeof(int), 1, irec_file);
    /* fwrite(p3, 1, IUPKEY_STATUS_SIZE, irec_file); */
    if (iup_isbutton1(p3)) p3c = '1';
    if (iup_isbutton2(p3)) p3c = '2';
    if (iup_isbutton3(p3)) p3c = '3';
    if (iup_isbutton4(p3)) p3c = '4';
    if (iup_isbutton5(p3)) p3c = '5';
    fwrite(&p3c, 1, 1, irec_file);
    fwrite("\n", 1, 1, irec_file);
    irec_lastclock = iRecClock();
  }
}

static void iRecInputKeyPressCB(int p1, int p2)
{
  if (irec_file)
  {
    char p2c = (char)p2;
    int time = iRecClock() - irec_lastclock;
    fwrite("KEY", 1, 3, irec_file);
    fwrite(&time, sizeof(int), 1, irec_file);
    fwrite(&p1, sizeof(int), 1, irec_file);
    fwrite(&p2c, 1, 1, irec_file);
    fwrite("\n", 1, 1, irec_file);
    irec_lastclock = iRecClock();
  }
}

int IupRecordInput(const char* filename)
{
  if (irec_file)
    fclose(irec_file);

  if (filename)
  {
    irec_file = fopen(filename, "wb");
    if (!irec_file)
      return IUP_ERROR;
  }
  else
    irec_file = NULL;

  if (irec_file)
  {
    fwrite("IUPINPUT\n", 1, 9, irec_file);
    irec_lastclock = iRecClock();

    IupSetGlobal("INPUTCALLBACKS", "Yes");
    IupSetFunction("GLOBALWHEEL_CB", (Icallback)iRecInputWheelCB);
    IupSetFunction("GLOBALBUTTON_CB", (Icallback)iRecInputButtonCB);
    IupSetFunction("GLOBALMOTION_CB", (Icallback)iRecInputMotionCB);
    IupSetFunction("GLOBALKEYPRESS_CB", (Icallback)iRecInputKeyPressCB);
  }
  else
  {
    IupSetGlobal("INPUTCALLBACKS", "No");
    IupSetFunction("GLOBALWHEEL_CB", NULL);
    IupSetFunction("GLOBALBUTTON_CB", NULL);
    IupSetFunction("GLOBALMOTION_CB", NULL);
    IupSetFunction("GLOBALKEYPRESS_CB", NULL);
  }

  return IUP_NOERROR;
}

static int iPlayTimer_CB(Ihandle* timer)
{
  FILE* file = (FILE*)IupGetAttribute(timer, "__IUP_PLAYFILE");
  if(feof(file) || ferror(file))
  {
    fclose(file);
    IupSetAttribute(timer, "RUN", "NO");
    IupDestroy(timer);
    IupSetGlobal("_IUP_PLAYTIMER", NULL);
    return IUP_IGNORE;
  }
  else
  {
    /* char status[IUPKEY_STATUS_SIZE]; */
    char action[4] = "XXX";
    char eol;
    int time;

    fread(action, 1, 3, file);
    fread(&time, sizeof(int), 1, file);
    if (ferror(file)) return IUP_DEFAULT;

    time -= iRecClock() - irec_lastclock;
    if (time < 0) time = 0;
    if (time)
      iupdrvSleep(time);

    switch (action[0])
    {
    case 'B':
      {
        char button, status;
        int x, y;
        fread(&button, 1, 1, file);
        fread(&status, 1, 1, file);
        fread(&x, sizeof(int), 1, file);
        fread(&y, sizeof(int), 1, file);
        /* fread(status, 1, IUPKEY_STATUS_SIZE, file); */
        fread(&eol, 1, 1, file);
        if (ferror(file)) return IUP_DEFAULT;

        /*IupSetfAttribute(NULL, "MOUSEBUTTON", "%dx%d %c %d", x, y, button, (int)status);*/
        iupdrvSendMouse(x, y, (int)button, (int)status);
        break;
      }
    case 'M':
      {
        char button;
        int x, y;
        fread(&x, sizeof(int), 1, file);
        fread(&y, sizeof(int), 1, file);
        fread(&button, 1, 1, file);
        /* fread(status, 1, IUPKEY_STATUS_SIZE, file); */
        fread(&eol, 1, 1, file);
        if (ferror(file)) return IUP_DEFAULT;

        /* IupSetfAttribute(NULL, "CURSORPOS", "%dx%d", x, y); */
        iupdrvSendMouse(x, y, (int)button, -1);
        break;
      }
    case 'K':
      {
        int key;
        char pressed;
        fread(&key, sizeof(int), 1, file);
        fread(&pressed, 1, 1, file);
        fread(&eol, 1, 1, file);
        if (ferror(file)) return IUP_DEFAULT;

        if (pressed)
          /* IupSetfAttribute(NULL, "KEYPRESS", "%d", key); */
          iupdrvSendKey(key, 0x01);
        else
          /* IupSetfAttribute(NULL, "KEYRELEASE", "%d", key); */
          iupdrvSendKey(key, 0x02);
        break;
      }
    case 'W':
      {
        float delta;
        int x, y;
        fread(&delta, sizeof(float), 1, file);
        fread(&x, sizeof(int), 1, file);
        fread(&y, sizeof(int), 1, file);
        /* fread(status, 1, IUPKEY_STATUS_SIZE, file); */
        fread(&eol, 1, 1, file);
        if (ferror(file)) return IUP_DEFAULT;

        /*IupSetfAttribute(NULL, "MOUSEBUTTON", "%dx%d %c %d", x, y, 'W', (int)delta);*/
        iupdrvSendMouse(x, y, 'W', (int)delta);
        break;
      }
    default:
      {
        fclose(file);
        IupSetAttribute(timer, "RUN", "NO");
        IupDestroy(timer);
        IupSetGlobal("_IUP_PLAYTIMER", NULL);
        return IUP_IGNORE;
      }
    }

    IupFlush();
    irec_lastclock = iRecClock();
  }

  return IUP_DEFAULT;
}

int IupPlayInput(const char* filename)
{
  Ihandle* timer = (Ihandle*)IupGetGlobal("_IUP_PLAYTIMER");
  FILE* file;
  char sig[10];

  if (timer)
  {
    if (filename && filename[0]==0)
    {
      if (IupGetInt(timer, "RUN"))
        IupSetAttribute(timer, "RUN", "NO");
      else
        IupSetAttribute(timer, "RUN", "Yes");
      return IUP_NOERROR;
    }

    file = (FILE*)IupGetAttribute(timer, "__IUP_PLAYFILE");

    fclose(file);
    IupSetAttribute(timer, "RUN", "NO");
    IupDestroy(timer);
    IupSetGlobal("_IUP_PLAYTIMER", NULL);
  }
  else
  {
    if (!filename || filename[0]==0)
      return IUP_ERROR;
  }

  if (!filename)
    return IUP_NOERROR;

  file = fopen(filename, "rb");
  if (!file)
    return IUP_ERROR;

  fread(sig, 1, 9, file); sig[9] = 0;
  if (ferror(file)) return IUP_ERROR;

  if (!iupStrEqual(sig, "IUPINPUT\n"))
  {
    fclose(file);
    return IUP_ERROR;
  }

  irec_lastclock = iRecClock();

  timer = IupTimer();
  IupSetCallback(timer, "ACTION_CB", (Icallback)iPlayTimer_CB);
  IupSetAttribute(timer, "TIME", "20");
  IupSetAttribute(timer, "__IUP_PLAYFILE", (char*)file);
  IupSetAttribute(timer, "RUN", "YES");

  IupSetGlobal("_IUP_PLAYTIMER", (char*)timer);
  return IUP_NOERROR;
}
