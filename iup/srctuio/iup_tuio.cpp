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

struct iTuioCursorEnvent{
  int id;
  float x, y;
  char state;

  iTuioCursorEnvent(int _id, float _x, float _y, char _state);
};

iTuioCursorEnvent::iTuioCursorEnvent(int _id, float _x, float _y, char _state)
:id(_id), x(_x), y(_y), state(_state)
{
}

class IupTuioListener : public TuioListener 
{
  int changed, locked;
  TuioClient* client;
  Ihandle* ih;
  std::list<iTuioCursorEnvent> cursor_events;

  void processCursor(TuioCursor *tcur, const char* state, const char* action);

  static int timer_action_cb(Ihandle *timer);

  public:
    int debug;
    Ihandle* timer;

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
  :changed(0), locked(0), client(_client), ih(_ih), debug(0)
{
  timer = IupTimer();
  IupSetAttribute(timer, "TIME", "50");
  IupSetAttribute(timer, "_IUP_TUIOLISTENER", (char*)this);
  IupSetCallback(timer, "ACTION_CB", timer_action_cb);
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
  float x = tcur->getX();
  float y = tcur->getY();
  int id = (((int)tcur->getSessionID()) << 16) | tcur->getCursorID();
  cursor_events.push_back(iTuioCursorEnvent(id, x, y, state[0]));

  this->changed = 1;

  if (this->debug)
    printf("IupTuioClient-%s(id=%d sid=%d x=%d y=%d)\n", action, tcur->getCursorID(), (int)tcur->getSessionID(), (int)tcur->getX(), (int)tcur->getY());
}

void  IupTuioListener::refresh(TuioTime frameTime) 
{
  if (this->changed)
  {
    Ihandle* ih_canvas = IupGetAttributeHandle(this->ih, "TARGETCANVAS");
    if (!ih_canvas)
      ih_canvas = this->ih;

    this->changed = 0;

    IFniiis cb = (IFniiis)IupGetCallback(ih_canvas, "TOUCH_CB");
    IFniIIII mcb = (IFniIIII)IupGetCallback(ih_canvas, "MULTITOUCH_CB");
    if (cb || mcb)
    {
      this->client->lockCursorList();
      this->locked = 1;
    }

    if (this->debug)
      printf("IupTuioClient-RefreshChanged(time=%d)\n", (int)frameTime.getTotalMilliseconds());
  }
}

int IupTuioListener::timer_action_cb(Ihandle *timer)
{
  IupTuioListener* listener = reinterpret_cast<IupTuioListener*>(IupGetAttribute(timer, "_IUP_TUIOLISTENER"));

  if (!listener->locked)
    return IUP_DEFAULT;

  int has_canvas = 0;
  Ihandle* ih_canvas = IupGetAttributeHandle(listener->ih, "TARGETCANVAS");
  if (ih_canvas)
    has_canvas = 1;
  else
    ih_canvas = listener->ih;

  IFniiis cb = (IFniiis)IupGetCallback(ih_canvas, "TOUCH_CB");
  IFniIIII mcb = (IFniIIII)IupGetCallback(ih_canvas, "MULTITOUCH_CB");

  int w, h, x, y;
  iupdrvGetFullSize(&w, &h);

  int count = listener->cursor_events.size();

  int *px=NULL, *py=NULL, *pid=NULL, *pstate=NULL;
  if (mcb)
  {
    px = new int[count?count:1];
    py = new int[count?count:1];
    pid = new int[count?count:1];
    pstate = new int[count?count:1];
  }

  for (int i = 0; i<count; i++) 
  {
    const iTuioCursorEnvent& evt = listener->cursor_events.front();

    char* state = (evt.state=='D')? "DOWN": ((evt.state=='U')? "UP": "MOVE");
    x = (int)floor(evt.x*w+0.5f);
    y = (int)floor(evt.y*h+0.5f);

    if (has_canvas)
      iupdrvScreenToClient(ih_canvas, &x, &y);

    if (cb)
    {
      if (cb(ih_canvas, evt.id, x, y, state)==IUP_CLOSE)
        IupExitLoop();
    }

    if (mcb)
    {
      px[i] = x;
      py[i] = y;
      pid[i] = evt.id;
      pstate[i] = state[0];
    }

    listener->cursor_events.pop_front();
  }

  if (mcb)
  {
    if (mcb(ih_canvas, count, pid, px, py, pstate)==IUP_CLOSE)
      IupExitLoop();
     
    delete[] px;
    delete[] py;
    delete[] pid;
    delete[] pstate;
  }

  listener->locked = 0;
  listener->client->unlockCursorList();
  return IUP_DEFAULT;
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
  {
    IupSetAttribute(ih->data->listener->timer, "RUN", "NO");
    ih->data->client->disconnect();
  }

  if (iupStrEqualNoCase(value, "YES"))
  {
    ih->data->client->connect(false);
    IupSetAttribute(ih->data->listener->timer, "RUN", "YES");
  }
  else if (iupStrEqualNoCase(value, "LOCKED"))
  {
    ih->data->client->connect(true);
    IupSetAttribute(ih->data->listener->timer, "RUN", "YES");
  }
    
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
  IupDestroy(ih->data->listener->timer);
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
  iupClassRegisterCallback(ic, "MULTITOUCH_CB", "iIIII");

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
