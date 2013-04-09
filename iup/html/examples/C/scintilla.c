/*
 * IupScintilla sample
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iup.h>
#include <iup_scintilla.h>

const char* sampleCode = {
  "#include<stdio.h>\n#include<iup.h>\n\nvoid SampleTest() {\n  printf(\"Printing float: %f\\n\", 12.5);\n}\n\n"
  "int main(int argc, char **argv) {\n"
  "  // Start up the IupScintilla\n"
  "  IupOpen(&argc, &argv);\n"
  "  IupSetGlobal(\"SINGLEINSTANCE\", \"Iup Sample\");\n\n"
  "  if(!IupGetGlobal(\"SINGLEINSTANCE\")) {\n"
  "    IupClose(); \n"
  "    return EXIT_SUCCESS; \n  }\n\n"
  "  SampleTest();\n"
  "  printf(\"Printing an integer: %d\\n\", 37);\n\n"
  "  IupMainLoop();\n"
  "  IupClose();\n"
  "  return EXIT_SUCCESS;\n}\n"
};

int marginclick_cb(Ihandle *self, int margin, int line)
{
  char buffer[100], str[100];

  sprintf(buffer, "%d", line);
  sprintf(str, "FOLDLEVEL%d", line);

  printf("MARGINCLICK_CB = Margin: %d, Line: %d\n", margin, line);
  printf("Fold Level = %s\n", IupGetAttribute(self, str));

  IupSetAttribute(self, "TOGGLEFOLD", buffer);

  return IUP_DEFAULT;
}

int doubleclick_cb(Ihandle *self, int mod, int line)
{
  char buffer[100]; 
  sprintf(buffer, "%d", line);

  IupSetAttribute(self, "TOGGLEFOLD", buffer);

  printf("DBLCLICK_CB = Modifier: %d, Line: %d\n", mod, line);
  if(mod == 1) printf("SHIFT pressed\n\n");
  else if(mod == 2) printf("CTRL pressed\n\n");
  else if(mod == 4) printf("ALT pressed\n\n");
  else if(mod == 6) printf("ALT GR pressed\n\n");
  else printf("\n\n");

  (void)self;
  return IUP_DEFAULT;
}

int hotspotclick_cb(Ihandle *self, int mod, int line)
{
  char buffer[100], str[100];
  char *text;
  
  sprintf(buffer, "%d", line);
  sprintf(str, "LINE%d", line);
  
  text = IupGetAttribute(self, str);

  printf("HOTSPOTCLICK_CB = Modifier: %d, Line: %d\n", mod, line);
  printf("Getting line text = %s\n", text);

  return IUP_DEFAULT;
}

int button_cb(Ihandle* self, int button, int pressed, int x, int y, char* status)
{
  printf("BUTTON_CB = button: %d, pressed: %d, x: %d, y: %d, status: %s\n", button, pressed, x, y, status);
  (void)self;
  return IUP_DEFAULT;
}

int motion_cb(Ihandle *self, int x, int y, char *status)
{
  printf("MOTION_CB = x: %d, y: %d, status: %s\n", x, y, status);
  (void)self;
  return IUP_DEFAULT;
}

int caret_cb(Ihandle *self, int lin, int col, int pos)
{
  printf("CARET_CB = lin: %d, col: %d, pos: %d\n", lin, col, pos);
  (void)self;
  return IUP_DEFAULT;
}

int valuechanged_cb(Ihandle *self)
{
  printf("VALUECHANGED_CB\n");
  (void)self;
  return IUP_DEFAULT;
}

int action_cb(Ihandle *self, int key, char *txt)
{
  printf("ACTION = key: %d, text: %s\n", key, txt);
  (void)self;
  return IUP_IGNORE;
}

void set_attribs (Ihandle *sci)
{
  IupSetAttribute(sci, "CLEARALL", "");
  IupSetAttribute(sci, "LEXER", "CPP");

  IupSetAttribute(sci, "KEYWORDS0", "int char double float");

  IupSetAttribute(sci, "STYLEFGCOLOR", "0 255 0");
  IupSetAttribute(sci, "STYLEFGCOLOR2", "0 255 0");
  IupSetAttribute(sci, "STYLEFGCOLOR4", "255 255 0");
  IupSetAttribute(sci, "STYLEFGCOLOR5", "255 0 0");
  IupSetAttribute(sci, "STYLEFGCOLOR6", "255 0 255");

  IupSetAttribute(sci, "STYLEBOLD10", "YES");
  IupSetAttribute(sci, "STYLEHOTSPOT6", "YES");
  IupSetAttribute(sci, "STYLEFONT32", "Courier New");
  IupSetAttribute(sci, "STYLEFONTSIZE32", "12");

  IupSetAttribute(sci, "INSERT0", sampleCode);

  IupSetAttribute(sci, "MARGINWIDTHN0", "50");

  if (1)
  {
    IupSetAttribute(sci, "PROPERTY", "fold,1");
    IupSetAttribute(sci, "PROPERTY", "fold.compact,0");
    IupSetAttribute(sci, "PROPERTY", "fold.comment,1");
    IupSetAttribute(sci, "PROPERTY", "fold.preprocessor,1");

    IupSetAttribute(sci, "MARGINWIDTHN1", "0");
    IupSetAttribute(sci, "MARGINTYPEN1",  "MARGIN_SYMBOL");
    IupSetAttribute(sci, "MARGINMASKN1",  "MASK_FOLDERS");
    IupSetAttribute(sci, "MARGINWIDTHN1", "20");

    IupSetAttribute(sci, "MARKERDEFINE", "FOLDER,PLUS");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDEROPEN,MINUS");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDEREND,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDERMIDTAIL,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDEROPENMID,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDERSUB,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDERTAIL,EMPTY");

    IupSetAttribute(sci, "FOLDFLAGS", "LINEAFTER_CONTRACTED");

    IupSetAttribute(sci, "MARGINSENSITIVEN1", "YES");
  }

  printf("Number of chars in this text: %s\n", IupGetAttribute(sci, "COUNT"));
  printf("Number of lines in this text: %s\n", IupGetAttribute(sci, "LINECOUNT"));
  printf("%s\n", IupGetAttribute(sci, "LINEVALUE"));
}

void ScintillaTest(void)
{
  Ihandle *dlg, *sci;

  IupScintillaOpen();        

  // Creates an instance of the Scintilla control
  sci = IupScintilla();
  IupSetAttribute(sci, "VISIBLECOLUMNS", "80");
  IupSetAttribute(sci, "VISIBLELINES", "40");
  //IupSetAttribute(sci, "SCROLLBAR", "NO");
  IupSetAttribute(sci, "BORDER", "NO");

  IupSetCallback(sci, "MARGINCLICK_CB", (Icallback)marginclick_cb);
  IupSetCallback(sci, "DBLCLICK_CB", (Icallback)doubleclick_cb);
  IupSetCallback(sci, "HOTSPOTCLICK_CB", (Icallback)hotspotclick_cb);
  IupSetCallback(sci, "BUTTON_CB", (Icallback)button_cb);
  IupSetCallback(sci, "MOTION_CB", (Icallback)motion_cb);
  IupSetCallback(sci, "CARET_CB", (Icallback)caret_cb);
  IupSetCallback(sci, "VALUECHANGED_CB", (Icallback)valuechanged_cb);
  IupSetCallback(sci, "ACTION", (Icallback)action_cb);

  // Creates a dialog containing the control
  dlg = IupDialog(IupVbox(sci, NULL));
  IupSetAttribute(dlg, "TITLE", "IupScintilla");
  IupSetAttribute(dlg, "RASTERSIZE", "680x510");
  IupSetAttribute(dlg, "MARGIN", "10x10");

  // Shows dialog
  IupShow(dlg);

  set_attribs(sci);
}

int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  ScintillaTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
