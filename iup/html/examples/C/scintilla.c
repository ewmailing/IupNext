/*
 * IupScintilla sample
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "iup.h"
#include "iup_scintilla.h"

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
  "  IupClose();\n\n"
  "  return EXIT_SUCCESS;\n}\n"
};

int margin_click_cb(Ihandle *self, int margin, int line)
{
  char buffer[100], str[100];
  
  _itoa (line, buffer, 10);
  sprintf(str, "FOLDLEVEL%d", line);

  printf("MARGINCLICK_CB = Margin: %d, Line: %d\n", margin, line);
  printf("Fold Level = %s\n", IupGetAttribute(self, str));

  IupSetAttribute(self, "TOGGLEFOLD", buffer);

  return IUP_DEFAULT;
}

int double_click_cb(Ihandle *self, int mod, int line)
{
  char buffer[100]; 
  _itoa (line, buffer, 10);

  IupSetAttribute(self, "TOGGLEFOLD", buffer);

  printf("DOUBLECLICK_CB = Modifier: %d, Line: %d\n", mod, line);
  if(mod == 1) printf("SHIFT pressed\n\n");
  else if(mod == 2) printf("CTRL pressed\n\n");
  else if(mod == 4) printf("ALT pressed\n\n");
  else if(mod == 6) printf("ALT GR pressed\n\n");
  else printf("\n\n");

  (void)self;
  return IUP_DEFAULT;
}

int hotspot_click_cb(Ihandle *self, int mod, int line)
{
  char buffer[100], str[100];
  char *text;
  
  _itoa (line, buffer, 10);
  sprintf(str, "LINE%d", line);
  
  text = IupGetAttribute(self, str);

  printf("HOTSPOTCLICK_CB = Modifier: %d, Line: %d\n", mod, line);
  printf("Getting line text = %s\n", text);

  return IUP_DEFAULT;
}

int dwell_start_cb(Ihandle *self, int x, int y, int line)
{
  printf("DWELLSTART_CB = x: %d, y: %d, line: %d\n", x, y, line);
  (void)self;
  return IUP_DEFAULT;
}

int dwell_end_cb(Ihandle *self, int x, int y, int line)
{
  printf("DWELLEND_CB = x: %d, y: %d, line: %d\n", x, y, line);
  (void)self;
  return IUP_DEFAULT;
}

void set_attribs (Ihandle *sci)
{
  IupSetAttribute(sci, "CLEARALL", "");
  IupSetAttribute(sci, "LEXER", "CPP");

  IupSetAttribute(sci, "KEYWORDS0", "int char double float");

  IupSetAttribute(sci, "FGCOLORSTYLE1", "0 255 0");
  IupSetAttribute(sci, "FGCOLORSTYLE2", "0 255 0");
  IupSetAttribute(sci, "FGCOLORSTYLE4", "255 255 0");
  IupSetAttribute(sci, "FGCOLORSTYLE5", "255 0 0");
  IupSetAttribute(sci, "FGCOLORSTYLE6", "255 0 255");

  IupSetAttribute(sci, "BOLDSTYLE10", "YES");
  IupSetAttribute(sci, "HOTSPOT6", "YES");
  IupSetAttribute(sci, "INSERTTEXT0", sampleCode);
  IupSetAttribute(sci, "FONT32", "Courier New");
  IupSetAttribute(sci, "FONTSIZE32", "12");

  IupSetAttribute(sci, "MARGINWIDTHN0", "50");

  if (0)
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
}

void ScintillaTest(void)
{
  Ihandle *dlg, *sci;

  IupScintillaOpen();        

  // Creates an instance of the Scintilla control
  sci = IupScintilla();
  IupSetAttribute(sci, "VISIBLECOLUMNS", "580");
  IupSetAttribute(sci, "VISIBLELINES", "400");
  //IupSetAttribute(sci, "SCROLLBAR", "NO");

  IupSetCallback(sci, "MARGINCLICK_CB", (Icallback)margin_click_cb);
  IupSetCallback(sci, "DOUBLECLICK_CB", (Icallback)double_click_cb);
  IupSetCallback(sci, "HOTSPOTCLICK_CB", (Icallback)hotspot_click_cb);
  IupSetCallback(sci, "DWELLSTART_CB", (Icallback)dwell_start_cb);
  IupSetCallback(sci, "DWELLEND_CB", (Icallback)dwell_end_cb);

  // Creates a dialog containing the control
  dlg = IupDialog(IupVbox(sci, NULL));
  IupSetAttribute(dlg, "TITLE", "IupScintilla");
  IupSetAttribute(dlg, "RASTERSIZE", "640x480");
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
