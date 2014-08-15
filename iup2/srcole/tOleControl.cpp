// tOleControl.cpp: implementation of the tOleControl class.
//
//////////////////////////////////////////////////////////////////////


#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "iup.h"
#include "iupcpi.h"
#include "iglobal.h"

#include "tOleControl.h"

Iclass *tOleControl::iupclass = NULL;

///////////////
//           //
// Callbacks //
//           //
///////////////

// Callback de resize do canvas container

static int resize_cb(Ihandle *self, int w, int h)
{
  tOleControl *ctl = (tOleControl *)
    tIupCtl::GetObjFromIhandle(self);

  if (ctl == NULL) 
   return IUP_DEFAULT;

  if(!ctl->created)
    return IUP_DEFAULT;

  ctl->olehandler->OnShow();

  return IUP_DEFAULT;
}


//////////////////////////////////////////////
// Stubs que transformam chamadas da Iup em //
// chamadas a metodos                       //
//////////////////////////////////////////////

// Stub de criacao

static Ihandle *stub_create(Iclass *iupclass, void **params)
{
  char *control_progID = NULL;
  tOleControl *ctl = NULL;
  
  control_progID = (char *) params[0];

  ctl = new tOleControl(control_progID);

  assert(ctl != NULL);

  if(ctl->created == false)
  {
    delete ctl;


    // bug: iup nao suporta que se retorne NULL
    // no metodo de create

    return IupCanvas(""); 
  }
  else
    return ctl->get_ihandle();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tOleControl::tOleControl(char *progID)
{
  int retval = 0;

  created = false;
  initialized = false;

  retval = OleInitialize(NULL);

  if(retval != S_OK && retval != S_FALSE)
  {
    initialized = false;
    return;
  }
  else
    initialized = true;

  olehandler = NULL;

  {
    wchar_t* wcProgId = NULL;
    HRESULT hr;

    /* converte ProgID para OLESTR */
    wcProgId = (wchar_t*) malloc( (strlen(progID) + 1) * sizeof(wchar_t));
    mbstowcs(wcProgId, progID, strlen(progID)+1);

    hr = CLSIDFromProgID(wcProgId, &m_clsid);

    free(wcProgId);
    wcProgId = NULL;

    if(FAILED(hr))
      return;
  }

  olehandler = new tOleHandler();

  if(olehandler->Create(&m_clsid) != CREATE_FAILED)
    created = true;
  else
  {
    delete olehandler;
    created = false;
  }

  if(created)
  {
    handle = IupCanvas("ole_control_repaint");
    IupSetCallback(handle, IUP_RESIZE_CB,(Icallback) resize_cb);
    IupSetAttribute(handle, IUP_BORDER, IUP_NO);

    initIhandle();
  }
}

tOleControl::~tOleControl()
{
  if(created)
  {
    olehandler->Close(true);
    delete olehandler;
  }

  if(initialized)
    OleUninitialize();
}

void tOleControl::Initialize()
{
  tOleControl::iupclass = 
    tIupCtl::RegisterIupClass("OLECONTROL", "olecontrol", "s", stub_create);
}

/////////////////
//             //
// Metodos CPI //
//             //
/////////////////

Ihandle *tOleControl::CreateFromArray(const char *ProgID)
{
  return IupCreatep("olecontrol", (Ihandle *) ProgID, NULL);
}

void tOleControl::map(Ihandle * parent)
{
  iupCpiDefaultMap(handle, parent);

  if(!created)
    return;

  olehandler->m_hWnd = (HWND)iupGetNativeHandle(handle);
}

char * tOleControl::getattr(const char * attr)
{
  if(!created)
    return NULL;

  assert(attr != NULL);
  if(attr == NULL)
    return NULL;

  if(!strcmp(attr, "IUNKNOWN"))
  {
    // retorna um ponteiro para a implementacao de
    // IUnknown do objeto
    IUnknown *punk = NULL;
    olehandler->ObjectGet(&punk);
    return (char *) punk;
  }
  else if(!strcmp(attr, "IUPOLECONTROL"))
    return IUP_YES;

  else
    return NULL;
}

void tOleControl::setnaturalsize()
{
  int w = 0, h = 0;
  long objwidth = 0, objheight = 0;

  if(!created)
  {
    iupCpiDefaultSetNaturalSize(handle);
    return;
  }

  iupGetSize(handle, &w, &h);

  olehandler->GetNaturalSize(&objwidth, &objheight);

  if (w <= 0)
    w = objwidth;

  if (h <= 0)
    h = objheight;


  iupSetNaturalWidth(handle, w);
  iupSetNaturalHeight(handle, h);
}

void tOleControl::setcurrentsize(int w, int h)
{
  SIZEL szl;

  iupCpiDefaultSetCurrentSize(handle, w, h);

  if(!created)
    return;

  szl.cx = iupGetCurrentWidth(handle);
  szl.cy = iupGetCurrentHeight(handle);

  olehandler->SizeSet(&szl, TRUE, TRUE);
  olehandler->UpdateInPlaceObjectRects(NULL, TRUE);
}

void tOleControl::setattr(const char * attribute, const char * value)
{
  if(!created)
    return;

  assert(attribute != NULL);

  if(attribute == NULL)
    return;

  // Seta designmode/usermode
  if(!strcmp(attribute, "DESIGNMODE"))
  {
    if(value && !strcmp(value, IUP_YES))
      olehandler->m_ambientProp.setDesignMode(true, true);
    else
      olehandler->m_ambientProp.setDesignMode(false, true);
  }
  else if(!strcmp(attribute, "DESIGNMODE_DONT_NOTIFY"))
  {
    if(value && !strcmp(value, IUP_YES))
      olehandler->m_ambientProp.setDesignMode(true, false);
    else
      olehandler->m_ambientProp.setDesignMode(false, false);
  }
  else if(!strcmp(attribute, IUP_BGCOLOR))
  {
    // ignora IUP_BGCOLOR, pois se ele for setado no canvas
    // gera problemas no repaint dos controles, pois o canvas
    // tenta se desenhar com a cor de fundo.
  }
  else // seta no canvas
  {
    iupCpiDefaultSetAttr(handle, attribute, value);
  }
}
