/******************************************************************************
*
*  IUP Speech
*
*  Mark Stroetzel Glasberg - June 18, 2003
*
* Attributes: 
*  NUMBER -> Grammar number associated with the Ihandle
*  GRAMMAR -> Takes a xml file with grammar definition
*  SAY -> Synthetizes a word/sentence.
*         
******************************************************************************/

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <atlbase.h>
#include <assert.h>

#include <sphelper.h> /* sapi function */

#include "iup.h"
#include "iupcpi.h"
#include "iupcbs.h"
#include "istrutil.h"
#include "iupspeech.h"

#define NUM_MAX_GRAMMARS 100
#define WM_RECOEVENT    WM_USER+190 /* Arbitrary user defined message for reco callback */

static Ihandle *ids[NUM_MAX_GRAMMARS];

static ISpVoice *pVoice = NULL;  /* Pointer to our voice synthetizer */

static CComPtr<ISpRecoContext> g_cpRecoCtxt; /* Pointer to our recognition context */
static CComPtr<ISpRecognizer>	 g_cpEngine;   /* Pointer to our recognition engine instance */

static HWND speech_hwnd = NULL;

LRESULT CALLBACK SpeechProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static int assignId(Ihandle *h)
{
  int i;
  for(i=0; i<NUM_MAX_GRAMMARS; i++)
  {
    if(ids[i] == NULL)
    {
      ids[i] = h;
      IupSetAttribute(h, "NUMBER", (char*) i);
      return i;
    }
  }
  /* iupError("id could not be set, more timers than allowed"); */
  return -1;
}

static Ihandle *iupSpeechCreate(Iclass *ic)
{
  Ihandle *s;
  assert(ic != NULL);
  if(ic == NULL)
    return NULL;

  s = IupUser();
  assignId(s);

  return s;
}

static void iupSpeechDestroy(Ihandle* n)
{
  ISpRecoGrammar *grammar = (ISpRecoGrammar *) IupGetAttribute(n, "GRAMMARCTX");
  if(grammar) 
    grammar->Release();
}

static void iupSpeechSetAttr(Ihandle *n, char *attr, char *value)
{
  if(attr == NULL)
    return;

  if(iupStrEqual(attr, "GRAMMAR"))
  {
    ISpRecoGrammar *grammar;

    if(value == NULL)
    {
      grammar = (ISpRecoGrammar *) IupGetAttribute(n, "GRAMMARCTX");
      if(grammar) 
      {
        grammar->Release();
        grammar = NULL;
      }
      return;
    }

    size_t len = strlen(value);
    wchar_t *filename = (wchar_t*) malloc(sizeof(wchar_t)*(len+1));
    mbstowcs(filename, value, len+1);

    grammar = (ISpRecoGrammar *) IupGetAttribute(n, "GRAMMARCTX");
    if(grammar) 
    {
      grammar->Release();
      grammar = NULL;
    }

    HRESULT hr = g_cpRecoCtxt->CreateGrammar((int) IupGetAttribute(n, "NUMBER"), &grammar);
    assert(SUCCEEDED(hr));

    /* Disabled before compiling */
    hr = grammar->SetGrammarState(SPGS_DISABLED);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
      return;

    /* Loading grammar file */
    hr = grammar->LoadCmdFromFile(filename, SPLO_STATIC);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
      return;

    /* Re-enabled after compiling */
    hr = grammar->SetGrammarState(SPGS_ENABLED);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
      return;

    /* Set rules to active, we are now listening for commands */
    hr = grammar->SetRuleState(NULL, NULL, SPRS_ACTIVE );
    assert(SUCCEEDED(hr));

    IupSetAttribute(n, "GRAMMARCTX", (char*) grammar);
  } 
  else if(iupStrEqual(attr, "SAY"))  
  {
    size_t len = strlen(value);
    wchar_t *line = (wchar_t*) malloc(sizeof(wchar_t)*(len+1));
    mbstowcs(line, value, len+1);
    char *sync = IupGetAttribute(n, "SYNC");
    HRESULT hr;
    if(sync && iupStrEqual(sync, "YES"))
      hr = pVoice->Speak(line, SPF_DEFAULT, NULL);
    else
      hr = pVoice->Speak(line, SPF_ASYNC, NULL);
    assert(SUCCEEDED(hr));
    free(line);
  }
}

Ihandle *IupSpeech(void)
{
  return IupCreate("speech");
}

int IupSpeechOpen(void)
{
  HINSTANCE hinstance = GetModuleHandle(NULL);

  /* create dummy window */
  WNDCLASSEX wcex;
  wcex.cbSize         = sizeof(WNDCLASSEX); 
  wcex.style			    = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc	  = (WNDPROC)SpeechProc;
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
  if(FAILED(hr)) return 0;

  speech_hwnd = CreateWindow("IUPSPEECH", "title", WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, 0, 540, 480, NULL, NULL, hinstance, NULL);
  if (!speech_hwnd) return 0;

  /* create a recognition engine */
#if 0
  // not working
  hr = g_cpEngine.CoCreateInstance(CLSID_SpInprocRecognizer);
  hr = g_cpEngine->SetInput(NULL, TRUE);
#else
  hr = g_cpEngine.CoCreateInstance(CLSID_SpSharedRecognizer);
#endif
  if(FAILED(hr)) return 0;

  /*  Init speech reproduction */
  hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
  if(FAILED(hr))
    return 0;

  /* create the command recognition context */
  hr = g_cpEngine->CreateRecoContext(&g_cpRecoCtxt);
  if(FAILED(hr)) return 0;

  /* Let SR know that window we want it to send event information to, and using what message */
  hr = g_cpRecoCtxt->SetNotifyWindowMessage(speech_hwnd, WM_RECOEVENT, 0, 0);
  assert(SUCCEEDED(hr));

  /* Tell SR what types of events interest us. Here we only care about command recognition */
  hr = g_cpRecoCtxt->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
  assert(SUCCEEDED(hr));

  Iclass *ICSpeech = iupCpiCreateNewClass("speech", NULL);
  iupCpiSetClassMethod(ICSpeech, ICPI_SETATTR, (Imethod) iupSpeechSetAttr);
  iupCpiSetClassMethod(ICSpeech, ICPI_DESTROY, (Imethod) iupSpeechDestroy);
  iupCpiSetClassMethod(ICSpeech, ICPI_CREATE,  (Imethod) iupSpeechCreate);

  return 1;
}

void IupSpeechClose(void)
{
  /* Uninit speech reproduction */
  pVoice->WaitUntilDone(3000); /* let's wait for 3secs (or it will crash) */
#if 0
  /* It is crashing with this, shouldn't be... */
  pVoice->Release();
  pVoice = NULL;
#endif

  if(g_cpEngine)     g_cpEngine.Release();

  /* Release recognition context, if created */
  if(g_cpRecoCtxt)
  {
    g_cpRecoCtxt->SetNotifySink(NULL);
    g_cpRecoCtxt.Release();
  }

  DestroyWindow(speech_hwnd);
  speech_hwnd = NULL;
}

static void call_action_cb(Ihandle *n, char *string, int rule, int val)
{
  if(n)
  {
    IFnsii cb = (IFnsii)IupGetCallback(n, "ACTION_CB");
    if(cb(n, string, rule, val) == IUP_CLOSE)
      IupExitLoop();
  }
}

static void ExecuteCommand(ISpPhrase *pPhrase, HWND hwnd)
{
  SPPHRASE *pElements;

  /* Get the phrase elements, one of which is the rule id we specified in
     the grammar.  Switch on it to figure out which command was recognized. */
  if (SUCCEEDED(pPhrase->GetPhrase(&pElements)))
  {        
    Ihandle *n = ids[pElements->ullGrammarID];
    assert(n);
    if(n)
    {
      WCHAR *wv;
      HRESULT hr = pPhrase->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &wv, NULL);
      assert(SUCCEEDED(hr));
      if(FAILED(hr))
        return;

      size_t len = wcslen(wv)+1;
      char *v = (char*) malloc(sizeof(char)*(len+1));
      wcstombs(v, wv, len);
      call_action_cb(n, v, pElements->Rule.ulId, pElements->pProperties->vValue.ulVal);
      free(v);
    }

    /* Free the pElements memory which was allocated for us */
    ::CoTaskMemFree(pElements);
  }
}

static void ProcessRecoEvent(HWND hwnd)
{
  CSpEvent event;

  /* Loop processing events while there are any in the queue */
  while(event.GetFrom(g_cpRecoCtxt) == S_OK)
  {
    /* Look at recognition event only */
    switch (event.eEventId)
    {
      case SPEI_RECOGNITION:
        ExecuteCommand(event.RecoResult(), hwnd);
        break;

    }
  }
}

static LRESULT CALLBACK SpeechProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) 
	{
    case WM_RECOEVENT:
      ProcessRecoEvent(hwnd);
      break;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
   }
   return 0;
}


