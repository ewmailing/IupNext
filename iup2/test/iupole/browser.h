// browser.h

#ifndef browser_h
#define browser_h

#include "iup.h"
#ifdef _WIN32
#include "iupole.h"
#endif

class Browser
{
 public:
  Browser(char *title,char *filename);
  ~Browser();
  void Show();
  void Hide();
 private:
  int firstshow;
  int shown;
  int Navigate();
  void CreateDialog100();
  char fname[BUFSIZ];
  char title[BUFSIZ];
  Ihandle *browser;
  Ihandle *dialog;
  Ihandle *close;
};

#endif
