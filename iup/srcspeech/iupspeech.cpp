/** \file
 * \brief Speech Control.
 *
 * See Copyright Notice in iup.ih
 */


#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <atlbase.h>

#include <sphelper.h> /* sapi function */

#include "iup.h"
#include "iupspeech.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_register.h"
#include "iup_assert.h"


#define ISPEECH_NUM_MAX_GRAMMARS 100
#define WM_RECOEVENT    WM_USER+190 /* Arbitrary user defined message for reco callback */

static Ihandle* ispeech_ids[ISPEECH_NUM_MAX_GRAMMARS];
static ISpVoice *ispeech_voice = NULL;                  /* Pointer to our voice synthetizer */
static CComPtr<ISpRecoContext> ispeech_context = NULL;  /* Pointer to our recognition context */
static CComPtr<ISpRecognizer>	 ispeech_engine = NULL;   /* Pointer to our recognition engine instance */
static HWND ispeech_hwnd = NULL;

struct _IcontrolData
{
  ISpRecoGrammar *grammar;
};


static int iSpeechSetGrammarAttrib(Ihandle *ih, const char *value)
{
  if (ih->data->grammar) 
    ih->data->grammar->Release();
  ih->data->grammar = NULL;

  if (!value)
    return 0;

  HRESULT hr = ispeech_context->CreateGrammar(ih->serial, &(ih->data->grammar));
  iupASSERT(SUCCEEDED(hr));

  /* Disabled before compiling */
  hr = ih->data->grammar->SetGrammarState(SPGS_DISABLED);
  iupASSERT(SUCCEEDED(hr));
  if(FAILED(hr))
    return 0;

  size_t len = strlen(value);
  wchar_t *wfilename = (wchar_t*)malloc(sizeof(wchar_t)*(len+1));
  mbstowcs(wfilename, value, len+1);

  /* Loading grammar file */
  hr = ih->data->grammar->LoadCmdFromFile(wfilename, SPLO_STATIC);
  free(wfilename);
  iupASSERT(SUCCEEDED(hr));
  if(FAILED(hr))
    return 0;

  /* Re-enabled after compiling */
  hr = ih->data->grammar->SetGrammarState(SPGS_ENABLED);
  iupASSERT(SUCCEEDED(hr));
  if(FAILED(hr))
    return 0;

  /* Set rules to active, we are now listening for commands */
  hr = ih->data->grammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
  iupASSERT(SUCCEEDED(hr));

  return 1;
}

static int iSpeechSetSayAttrib(Ihandle *ih, const char *value)
{
  HRESULT hr;
  if (!value)
    return 0;

  size_t len = strlen(value);
  wchar_t *line = (wchar_t*) malloc(sizeof(wchar_t)*(len+1));
  mbstowcs(line, value, len+1);

  char *sync = iupAttribGetStr(ih, "SYNC");
  if(iupStrBoolean(sync))
    hr = ispeech_voice->Speak(line, SPF_DEFAULT, NULL);
  else
    hr = ispeech_voice->Speak(line, SPF_ASYNC, NULL);
  iupASSERT(SUCCEEDED(hr));
  free(line);
  return 0;
}

static void iSpeechCallActionCb(Ihandle *ih, char *text, int rule_id, int prop_id)
{
  IFnsii cb = (IFnsii)IupGetCallback(ih, "ACTION_CB");
  if (cb(ih, text, rule_id, prop_id) == IUP_CLOSE)
    IupExitLoop();
}

static void iSpeechExecuteCommand(ISpPhrase *pPhrase)
{
  SPPHRASE *pElements;

  /* Get the phrase elements, one of which is the rule id we specified in
     the grammar.  Switch on it to figure out which command was recognized. */
  if (SUCCEEDED(pPhrase->GetPhrase(&pElements)))
  {        
    Ihandle *ih = ispeech_ids[pElements->ullGrammarID];
    iupASSERT(ih);
    if (ih)
    {
      WCHAR *wv;
      HRESULT hr = pPhrase->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &wv, NULL);
      iupASSERT(SUCCEEDED(hr));
      if(FAILED(hr))
        return;

      size_t len = wcslen(wv)+1;
      char *v = (char*) malloc(sizeof(char)*(len+1));
      wcstombs(v, wv, len);
      iSpeechCallActionCb(ih, v, pElements->Rule.ulId, (pElements->pProperties->vValue.vt == VT_UI4)? pElements->pProperties->vValue.ulVal: 0);
      free(v);
    }

    /* Free the pElements memory which was allocated for us */
    ::CoTaskMemFree(pElements);
  }
}

static void iSpeechProcessRecoEvent(void)
{
  CSpEvent cevent;

  /* Loop processing events while there are any in the queue */
  while(cevent.GetFrom(ispeech_context) == S_OK)
  {
    /* Look at recognition event only */
    if (cevent.eEventId == SPEI_RECOGNITION)
      iSpeechExecuteCommand(cevent.RecoResult());
  }
}

static LRESULT CALLBACK iSpeechProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) 
	{
    case WM_RECOEVENT:
      iSpeechProcessRecoEvent();
      break;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
   }
   return 0;
}


/*********************************************************************************/


static int iSpeechAssignId(Ihandle *ih)
{
  int i;
  for(i=0; i<ISPEECH_NUM_MAX_GRAMMARS; i++)
  {
    if (ispeech_ids[i] == NULL)
    {
      ispeech_ids[i] = ih;
      ih->serial = i;
      return i;
    }
  }
  return -1;
}

static int iSpeechCreateMethod(Ihandle* ih, void** params)
{
  (void)params;
  ih->data = iupALLOCCTRLDATA();
  iSpeechAssignId(ih);
  return IUP_NOERROR;
}

static void iSpeechDestroyMethod(Ihandle* ih)
{
  if (ih->data->grammar) 
    ih->data->grammar->Release();
  ih->data->grammar = NULL;
}

static void iSpeechReleaseMethod(Iclass* ic)
{
  (void)ic;

  if (!ispeech_hwnd)
    return;

  /* Uninit speech reproduction */
  if (ispeech_voice)
    ispeech_voice->WaitUntilDone(3000); /* let's wait for 3secs (or it will crash) */
#if 0
  ispeech_voice->Release();  /* It is crashing with this, shouldn't be... */
#endif
  ispeech_voice = NULL;

  if (ispeech_engine)     
    ispeech_engine.Release();
  ispeech_engine = NULL;

  /* Release recognition context, if created */
  if (ispeech_context)
  {
    ispeech_context->SetNotifySink(NULL);
    ispeech_context.Release();
    ispeech_context = NULL;
  }

  DestroyWindow(ispeech_hwnd);
  ispeech_hwnd = NULL;
}


/*********************************************************************************/


Ihandle *IupSpeech(void)
{
  return IupCreate("speech");
}

static Iclass* iSpeechGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "speech";
  ic->format = ""; /* no paramters */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = iSpeechCreateMethod;
  ic->Destroy = iSpeechDestroyMethod;
  ic->Release = iSpeechReleaseMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "ACTION_CB", "sii");

  /* Speech Only */
  iupClassRegisterAttribute(ic, "GRAMMAR", NULL, iSpeechSetGrammarAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SAY", NULL, iSpeechSetSayAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}

int IupSpeechOpen(void)
{
  HINSTANCE hinstance = GetModuleHandle(NULL);

  if (ispeech_hwnd)
    return IUP_NOERROR;

  memset(ispeech_ids, 0, ISPEECH_NUM_MAX_GRAMMARS*sizeof(Ihandle*));

  /* create dummy window */
  WNDCLASSEX wcex;
  wcex.cbSize         = sizeof(WNDCLASSEX); 
  wcex.style			    = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc	  = (WNDPROC)iSpeechProc;
  wcex.cbClsExtra		  = 0;
  wcex.cbWndExtra		  = 0;
  wcex.hInstance		  = hinstance;
  wcex.hIcon			    = NULL;
  wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground	= NULL;
  wcex.lpszMenuName	  = NULL;
  wcex.lpszClassName	= "IUPSPEECH";
  wcex.hIconSm		    = NULL;
  HRESULT hr = RegisterClassEx(&wcex);
  if(FAILED(hr)) return IUP_ERROR;

  ispeech_hwnd = CreateWindow("IUPSPEECH", "title", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, 540, 480, NULL, NULL, hinstance, NULL);
  if (!ispeech_hwnd) return IUP_ERROR;

  /* create a recognition engine */
#if 0
  // not working
  hr = ispeech_engine.CoCreateInstance(CLSID_SpInprocRecognizer);
  hr = ispeech_engine->SetInput(NULL, TRUE);
#else
  hr = ispeech_engine.CoCreateInstance(CLSID_SpSharedRecognizer);
#endif
  if(FAILED(hr)) return IUP_ERROR;

  /*  Init speech reproduction */
  hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&ispeech_voice);
  if(FAILED(hr))
    return IUP_ERROR;

  /* create the command recognition context */
  hr = ispeech_engine->CreateRecoContext(&ispeech_context);
  if(FAILED(hr)) 
    return IUP_ERROR;

  /* Let SR know that window we want it to send event information to, and using what message */
  hr = ispeech_context->SetNotifyWindowMessage(ispeech_hwnd, WM_RECOEVENT, 0, 0);
  iupASSERT(SUCCEEDED(hr));

  /* Tell SR what types of events interest us. Here we only care about command recognition */
  hr = ispeech_context->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
  iupASSERT(SUCCEEDED(hr));

  iupRegisterClass(iSpeechGetClass());

  return IUP_NOERROR;
}
