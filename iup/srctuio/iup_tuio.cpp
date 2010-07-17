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


using namespace TUIO;

class IupTuioListener : public TuioListener 
{
  int changed;
  TuioClient* client;
  Ihandle* ih;

  public:
    int debug;

    IupTuioListener(Ihandle* ih, TuioClient* _client);

    void addTuioObject(TuioObject *tobj);
    void updateTuioObject(TuioObject *tobj);
    void removeTuioObject(TuioObject *tobj);

    void addTuioCursor(TuioCursor *tcur);
    void updateTuioCursor(TuioCursor *tcur);
    void removeTuioCursor(TuioCursor *tcur);

    void refresh(TuioTime frameTime);
};

IupTuioListener::IupTuioListener(Ihandle* _ih, TuioClient* _client)
  :debug(0), changed(0), ih(_ih), client(_client)
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
  this->changed = 1;

  IFniiiis cb = (IFniiiis)IupGetCallback(this->ih, "CURSOR_CB");
  if (cb)
  {
    int w, h;
    IupGetIntInt(NULL, "FULLSIZE", &w, &h);
    cb(this->ih, tcur->getCursorID(), tcur->getSessionID(), tcur->getScreenX(w), tcur->getScreenY(h), "ADD");
  }

  if (this->debug)
  {
    int w, h;
    IupGetIntInt(NULL, "FULLSIZE", &w, &h);
    printf("IupTuioClient-AddCursor(id=%d sid=%d x=%d y=%d)\n",tcur->getCursorID(), tcur->getSessionID(), tcur->getScreenX(w), tcur->getScreenY(h));
  }
}

void IupTuioListener::updateTuioCursor(TuioCursor *tcur) 
{
  this->changed = 1;

  IFniiiis cb = (IFniiiis)IupGetCallback(this->ih, "CURSOR_CB");
  if (cb)
  {
    int w, h;
    IupGetIntInt(NULL, "FULLSIZE", &w, &h);
    cb(this->ih, tcur->getCursorID(), tcur->getSessionID(), tcur->getScreenX(w), tcur->getScreenY(h), "UPDATE");
  }

  if (this->debug)
  {
    int w, h;
    IupGetIntInt(NULL, "FULLSIZE", &w, &h);
    printf("IupTuioClient-UpdateCursor(id=%d sid=%d x=%d y=%d)\n",tcur->getCursorID(), tcur->getSessionID(), tcur->getScreenX(w), tcur->getScreenY(h));
  }
}

void IupTuioListener::removeTuioCursor(TuioCursor *tcur) 
{
  this->changed = 1;

  IFniiiis cb = (IFniiiis)IupGetCallback(this->ih, "CURSOR_CB");
  if (cb)
  {
    int w, h;
    IupGetIntInt(NULL, "FULLSIZE", &w, &h);
    cb(this->ih, tcur->getCursorID(), tcur->getSessionID(), tcur->getScreenX(w), tcur->getScreenY(h), "REMOVE");
  }

  if (this->debug)
  {
    int w, h;
    IupGetIntInt(NULL, "FULLSIZE", &w, &h);
    printf("IupTuioClient-RemoveCursor(id=%d sid=%d x=%d y=%d)\n",tcur->getCursorID(), tcur->getSessionID(), tcur->getScreenX(w), tcur->getScreenY(h));
  }
}

void  IupTuioListener::refresh(TuioTime frameTime) 
{
  if (this->changed)
  {
    std::list<TuioCursor*>::iterator iter;
    int i, w, h, x, y;
    Ihandle* ih_canvas = NULL;
    IFniIII cb = NULL;
    int *px=NULL, *py=NULL, *id=NULL;

    if (this->debug)
      printf("IupTuioClient-RefreshChanged(time=%d)\n", frameTime.getTotalMilliseconds());

    ih_canvas = IupGetAttributeHandle(this->ih, "TARGETCANVAS");
    if (!ih_canvas || ih_canvas->iclass->nativetype != IUP_TYPECANVAS)
      return;

    cb = (IFniIII)IupGetCallback(ih_canvas, "MULTITOUCH_CB");
    if (!cb)
      return;

    this->changed = 0;
    this->client->lockCursorList();

    IupGetIntInt(NULL, "FULLSIZE", &w, &h);

    std::list<TuioCursor*> cursorList = this->client->getTuioCursors();
    int count = cursorList.size();

    px = new int[count];
    py = new int[count];
    id = new int[count];

    for (i = 0, iter = cursorList.begin(); i<count; iter++,i++) 
    {
      TuioCursor *tcur = (*iter);

      x = tcur->getScreenX(w);
      y = tcur->getScreenY(h);

      iupdrvScreenToClient(ih_canvas, &x, &y);

      px[i] = x;
      py[i] = y;
      id[i] = (((int)tcur->getSessionID()) << 16) || tcur->getCursorID();
    }

    if (cb(ih_canvas, count, id, px, py)==IUP_CLOSE)
      IupExitLoop();
     
    delete[] px;
    delete[] py;
    delete[] id;

    this->client->unlockCursorList();
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
    port = (int)(params[0]);
  
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
