/** \file
 * \brief IupTuioClient control
 *
 * See Copyright Notice in "iup.h"
 */

#include "TuioListener.h"
#include "TuioClient.h"

#include "iup.h"
#include "iuptuio.h"
#include "iupcbs.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "iup_object.h"
#include "iup_assert.h"
#include "iup_str.h"
#include "iup_register.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"


using namespace TUIO;

class IupTuioListener : public TuioListener 
{
  int changed;
  TuioClient* client;
  Ihandle* ih;

  void processCursor(TuioCursor *tcur, const char* state, const char* action);

  public:
    int debug;

    IupTuioListener(Ihandle* _ih, TuioClient* _client);

    void addTuioObject(TuioObject *tobj);
    void updateTuioObject(TuioObject *tobj);
    void removeTuioObject(TuioObject *tobj);

    void addTuioCursor(TuioCursor *tcur);
    void updateTuioCursor(TuioCursor *tcur);
    void removeTuioCursor(TuioCursor *tcur);

    void refresh(TuioTime frameTime);
};

IupTuioListener::IupTuioListener(Ihandle* _ih, TuioClient* _client)
  :changed(0), client(_client), ih(_ih), debug(0)
{
}

void IupTuioListener::addTuioObject(TuioObject *tobj) 
{
  /* unused */
  (void)tobj;
}

void IupTuioListener::updateTuioObject(TuioObject *tobj) 
{
  /* unused */
  (void)tobj;
}

void IupTuioListener::removeTuioObject(TuioObject *tobj) 
{
  /* unused */
  (void)tobj;
}

void IupTuioListener::addTuioCursor(TuioCursor *tcur) 
{
  IupTuioListener::processCursor(tcur, "DOWN", "AddCursor"); 
}

void IupTuioListener::updateTuioCursor(TuioCursor *tcur) 
{
  IupTuioListener::processCursor(tcur, "MOVE", "UpdateCursor"); 
}

void IupTuioListener::removeTuioCursor(TuioCursor *tcur) 
{
  IupTuioListener::processCursor(tcur, "UP", "RemoveCursor"); 
}

void IupTuioListener::processCursor(TuioCursor *tcur, const char* state, const char* action) 
{
  int has_canvas = 0;
  this->changed = 1;

  Ihandle* ih_canvas = IupGetAttributeHandle(this->ih, "TARGETCANVAS");
  if (ih_canvas)
    has_canvas = 1;
  else
    ih_canvas = this->ih;

  IFniiis cb = (IFniiis)IupGetCallback(ih_canvas, "TOUCH_CB");
  if (cb)
  {
    int w, h, x, y, id;
    iupdrvGetFullSize(&w, &h);

    x = tcur->getScreenX(w);
    y = tcur->getScreenY(h);
    id = (((int)tcur->getSessionID()) << 16) | tcur->getCursorID();

    if (has_canvas)
      iupdrvScreenToClient(ih_canvas, &x, &y);

    if (cb(ih_canvas, id, x, y, (char*)state)==IUP_CLOSE)
      IupExitLoop();
  }

  if (this->debug)
    printf("IupTuioClient-%s(id=%d sid=%d x=%d y=%d)\n", action, tcur->getCursorID(), (int)tcur->getSessionID(), (int)tcur->getX(), (int)tcur->getY());
}

void  IupTuioListener::refresh(TuioTime frameTime) 
{
  if (this->changed)
  {
    std::list<TuioCursor*>::iterator iter;
    int i, w, h, x, y;
    IFniIII cb = NULL;
    int *px=NULL, *py=NULL, *id=NULL;
    int has_canvas = 0;

    this->changed = 0;

    Ihandle* ih_canvas = IupGetAttributeHandle(this->ih, "TARGETCANVAS");
    if (ih_canvas)
      has_canvas = 1;
    else
      ih_canvas = this->ih;

    cb = (IFniIII)IupGetCallback(ih_canvas, "MULTITOUCH_CB");
    if (cb)
    {
      this->client->lockCursorList();

      iupdrvGetFullSize(&w, &h);

      std::list<TuioCursor*> cursorList = this->client->getTuioCursors();
      int count = cursorList.size();

      px = new int[count?count:1];
      py = new int[count?count:1];
      id = new int[count?count:1];

      for (i = 0, iter = cursorList.begin(); i<count; iter++,i++) 
      {
        TuioCursor *tcur = (*iter);

        x = tcur->getScreenX(w);
        y = tcur->getScreenY(h);

        if (has_canvas)
          iupdrvScreenToClient(ih_canvas, &x, &y);

        px[i] = x;
        py[i] = y;
        id[i] = (((int)tcur->getSessionID()) << 16) | tcur->getCursorID();
      }

      if (cb(ih_canvas, count, id, px, py)==IUP_CLOSE)
        IupExitLoop();
       
      delete[] px;
      delete[] py;
      delete[] id;

      this->client->unlockCursorList();
    }

    if (this->debug)
      printf("IupTuioClient-RefreshChanged(time=%d)\n", (int)frameTime.getTotalMilliseconds());
  }
}

/**************************************************************************************/

struct _IcontrolData 
{
  IupTuioListener* listener;
  TuioClient* client;
};

static int iTuioSetConnectAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->client->isConnected())
    ih->data->client->disconnect();
    
  if (iupStrEqualNoCase(value, "YES"))
    ih->data->client->connect(false);
  else if (iupStrEqualNoCase(value, "LOCKED"))
    ih->data->client->connect(true);
    
  return 0;
}

static char* iTuioGetConnectAttrib(Ihandle *ih)
{
  if (ih->data->client->isConnected())
    return "Yes";
  else
    return "No";
}

static int iTuioSetDebugAttrib(Ihandle* ih, const char* value)
{
  ih->data->listener->debug = iupStrBoolean(value);
  return 0;
}

static char* iTuioGetDebugAttrib(Ihandle *ih)
{
  if (ih->data->listener->debug)
    return "Yes";
  else
    return "No";
}

static int iTuioCreateMethod(Ihandle* ih, void** params)
{
  int port = 3333;
  if (params && params[0])
    port = (int)(long)(params[0]); /* must cast to long first to avoid 64bit compiler error */
  
  ih->data = iupALLOCCTRLDATA();
  
  ih->data->client = new TuioClient(port);
  ih->data->listener = new IupTuioListener(ih, ih->data->client);
  ih->data->client->addTuioListener(ih->data->listener);

  return IUP_NOERROR;
}

static void iTuioDestroyMethod(Ihandle* ih)
{
  delete ih->data->client;
  delete ih->data->listener;
}

Ihandle* IupTuioClient(int port)
{
  void *params[2];
  params[0] = (void*)port;
  params[1] = NULL;
  return IupCreatev("tuioclient", params);
}

static Iclass* iTuioGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "tuioclient";
  ic->format = "i";  /* (int) */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;
  
  ic->Create = iTuioCreateMethod;
  ic->Destroy = iTuioDestroyMethod;

  iupClassRegisterCallback(ic, "TOUCH_CB", "iiis");
  iupClassRegisterCallback(ic, "MULTITOUCH_CB", "iIII");

  iupClassRegisterAttribute(ic, "CONNECT", iTuioGetConnectAttrib, iTuioSetConnectAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DEBUG", iTuioGetDebugAttrib, iTuioSetDebugAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

int IupTuioOpen(void)
{
  if (IupGetGlobal("_IUP_TUIO_OPEN"))
    return IUP_OPENED;

  iupRegisterClass(iTuioGetClass());

  IupSetGlobal("_IUP_TUIO_OPEN", "1");
  return IUP_NOERROR;
}
