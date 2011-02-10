/** \file
 * \brief global attributes enviroment
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>      
#include <stdio.h>      
#include <string.h>      

#include "iup.h" 

#include "iup_table.h"
#include "iup_globalattrib.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_drvinfo.h"
#include "iup_assert.h"
#include "iup_str.h"
#include "iup_strmessage.h"


static Itable *iglobal_table = NULL;

void iupGlobalAttribInit(void)
{
  iglobal_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupGlobalAttribFinish(void)
{
  iupTableDestroy(iglobal_table);
  iglobal_table = NULL;
}

static int iGlobalChangingDefaultColor(const char *name)
{
  if (iupStrEqual(name, "DLGBGCOLOR") ||
      iupStrEqual(name, "DLGFGCOLOR") ||
      iupStrEqual(name, "MENUBGCOLOR") ||
      iupStrEqual(name, "MENUFGCOLOR") ||
      iupStrEqual(name, "TXTBGCOLOR") ||
      iupStrEqual(name, "TXTFGCOLOR"))
  {
    char str[50] = "_IUP_USER_DEFAULT_";
    strcat(str, name);
    iupTableSet(iglobal_table, str, (void*)"1", IUPTABLE_POINTER);  /* mark as changed by the User */
    return 1;
  }
  return 0;
}

int iupGlobalDefaultColorChanged(const char *name)
{
  char str[50] = "_IUP_USER_DEFAULT_";
  strcat(str, name);
  return iupTableGet(iglobal_table, str) != NULL;
}

void iupGlobalSetDefaultColorAttrib(const char* name, int r, int g, int b)
{
  if (!iupGlobalDefaultColorChanged(name))
  {
    char value[50];
    sprintf(value, "%3d %3d %3d", r, g, b);
    iupTableSet(iglobal_table, name, (void*)value, IUPTABLE_STRING);
  }
}

static void iGlobalTableSet(const char *name, const char *value, int store)
{
  if (!value)
    iupTableRemove(iglobal_table, name);
  else if (store)
    iupTableSet(iglobal_table, name, (void*)value, IUPTABLE_STRING);
  else
    iupTableSet(iglobal_table, name, (void*)value, IUPTABLE_POINTER);
}

static void iGlobalSet(const char *name, const char *value, int store)
{
  iupASSERT(name!=NULL);
  if (!name) return;

  if (iupStrEqual(name, "DEFAULTFONTSIZE"))
  {
    iupSetDefaultFontSizeGlobalAttrib(value);
    return;
  }
  if (iupStrEqual(name, "KEYPRESS"))
  {
    int key;
    if (iupStrToInt(value, &key))
      iupdrvSendKey(key, 0x01);
    return;
  }
  if (iupStrEqual(name, "KEYRELEASE"))
  {
    int key;
    if (iupStrToInt(value, &key))
      iupdrvSendKey(key, 0x02);
    return;
  }
  if (iupStrEqual(name, "KEY"))
  {
    int key;
    if (iupStrToInt(value, &key))
      iupdrvSendKey(key, 0x03);
    return;
  }
  if (iupStrEqual(name, "LANGUAGE"))
  {
    iupStrMessageUpdateLanguage(value);
    iGlobalTableSet(name, value, store);
    return;
  }
  if (iupStrEqual(name, "CURSORPOS"))
  {
    int x, y;
    if (iupStrToIntInt(value, &x, &y, 'x') == 2)
      iupdrvWarpPointer(x, y);
    return;
  }
  if (iupStrEqual(name, "MOUSEBUTTON"))
  {
    int x, y, status;
    char bt; 
    if (sscanf(value, "%dx%d %c %d", &x, &y, &bt, &status) == 4)
      iupdrvSendMouse(x, y, bt, status);
    return;
  }

  if (iGlobalChangingDefaultColor(name) || 
      iupdrvSetGlobal(name, value))
    iGlobalTableSet(name, value, store);
}

void IupSetGlobal(const char *name, const char *value)
{
  iGlobalSet(name, value, 0);
}

void IupStoreGlobal(const char *name, const char *value)
{
  iGlobalSet(name, value, 1);
}

char *IupGetGlobal(const char *name)
{
  char* value;
  
  iupASSERT(name!=NULL);
  if (!name) 
    return NULL;

  if (iupStrEqual(name, "DEFAULTFONTSIZE"))
    return iupGetDefaultFontSizeGlobalAttrib();
  if (iupStrEqual(name, "CURSORPOS"))
  {
    char *str = iupStrGetMemory(50);
    int x, y;
    iupdrvGetCursorPos(&x, &y);
    sprintf(str, "%dx%d", (int)x, (int)y);
    return str;
  }
  if (iupStrEqual(name, "SHIFTKEY"))
  {
    char key[5];
    iupdrvGetKeyState(key);
    if (key[0] == 'S')
      return "ON";
    else
      return "OFF";
  }
  if (iupStrEqual(name, "CONTROLKEY"))
  {
    char key[5];
    iupdrvGetKeyState(key);
    if (key[1] == 'C')
      return "ON";
    else
      return "OFF";
  }
  if (iupStrEqual(name, "MODKEYSTATE"))
  {
    char *str = iupStrGetMemory(5);
    iupdrvGetKeyState(str);
    return str;
  }
  if (iupStrEqual(name, "SCREENSIZE"))
  {
    char *str = iupStrGetMemory(50);
    int w, h;
    iupdrvGetScreenSize(&w, &h);
    sprintf(str, "%dx%d", w, h);
    return str;
  }
  if (iupStrEqual(name, "FULLSIZE"))
  {
    char *str = iupStrGetMemory(50);
    int w, h;
    iupdrvGetFullSize(&w, &h);
    sprintf(str, "%dx%d", w, h);
    return str;
  }
  if (iupStrEqual(name, "SCREENDEPTH"))
  {
    char *str = iupStrGetMemory(50);
    int bpp = iupdrvGetScreenDepth();
    sprintf(str, "%d", bpp);
    return str;
  }

  value = iupdrvGetGlobal(name);

  if (!value)
    value = (char*)iupTableGet(iglobal_table, name);

  return value;
}

int iupGlobalIsPointer(const char* name)
{
  static struct {
    const char *name;
  } ptr_table[] = {
#ifndef GTK_MAC
  #ifdef WIN32
    {"HINSTANCE"},
  #else
    {"XDISPLAY"},
    {"XSCREEN"},
    {"APPSHELL"},
  #endif
#endif
  };
#define PTR_TABLE_SIZE ((sizeof ptr_table)/(sizeof ptr_table[0]))

  if (name)
  {
    int i;
    for (i = 0; i < PTR_TABLE_SIZE; i++)
    {
      if (iupStrEqualNoCase(name, ptr_table[i].name))
        return 1;
    }
  }

  return 0;
}

#include "iup_key.h"
#include <time.h>

static FILE* irec_file = NULL;
static int irec_lastclock = 0;

static int iRecClock(void)
{
  return (int)(clock()*1000/CLOCKS_PER_SEC); /* time in miliseconds */
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

void IupRecordInput(const char* filename)
{
  //int ret = IUP_NOERROR;
  if (irec_file)
    fclose(irec_file);

  if (filename)
    irec_file = fopen(filename, "wb");
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
  //return ret;
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
      break;
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
    // if ""
    file = (FILE*)IupGetAttribute(timer, "__IUP_PLAYFILE");

    fclose(file);
    IupSetAttribute(timer, "RUN", "NO");
    IupDestroy(timer);
    IupSetGlobal("_IUP_PLAYTIMER", NULL);
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
