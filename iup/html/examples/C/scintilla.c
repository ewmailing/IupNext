/*
 * IupScintilla sample
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iup.h>
#include <iup_scintilla.h>

const char* sampleCode = {
  "/* Block comment */\n"
  "#include<stdio.h>\n#include<iup.h>\n\n"
  "void SampleTest() {\n  printf(\"Printing float: %f\\n\", 12.5);\n}\n\n"
  "void SampleTest2() {\n  printf(\"Printing char: %c\\n\", 'c');\n}\n\n"
  "int main(int argc, char **argv) {\n"
  "  // Start up IUP\n"
  "  IupOpen(&argc, &argv);\n"
  "  IupSetGlobal(\"SINGLEINSTANCE\", \"Iup Sample\");\n\n"
  "  if(!IupGetGlobal(\"SINGLEINSTANCE\")) {\n"
  "    IupClose(); \n"
  "    return EXIT_SUCCESS; \n  }\n\n"
  "  SampleTest();\n"
  "  SampleTest2();\n"
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
  IupSetAttribute(sci, "LEXERLANGUAGE", "cpp");

  IupSetAttribute(sci, "KEYWORDS0", "void struct union enum char short int long double float signed unsigned const static extern auto register volatile bool class private protected public friend inline template virtual asm explicit typename mutable"
                                    "if else switch case default break goto return for while do continue typedef sizeof NULL new delete throw try catch namespace operator this const_cast static_cast dynamic_cast reinterpret_cast true false using"
                                    "typeid and and_eq bitand bitor compl not not_eq or or_eq xor xor_eq");

  //IupSetAttribute(sci, "STYLEFONT32", "Courier New");
  IupSetAttribute(sci, "STYLEFONT32", "Consolas");
  IupSetAttribute(sci, "STYLEFONTSIZE32", "11");
  IupSetAttribute(sci, "STYLECLEARALL", "Yes");  /* sets all styles to have the same attributes as 32 */

  IupSetAttribute(sci, "STYLEFGCOLOR1", "0 128 0");    // 1-C comment 
  IupSetAttribute(sci, "STYLEFGCOLOR2", "0 128 0");    // 2-C++ comment line 
  IupSetAttribute(sci, "STYLEFGCOLOR4", "128 0 0");    // 4-Number 
  IupSetAttribute(sci, "STYLEFGCOLOR5", "0 0 255");    // 5-Keyword 
  IupSetAttribute(sci, "STYLEFGCOLOR6", "160 20 20");  // 6-String 
  IupSetAttribute(sci, "STYLEFGCOLOR7", "128 0 0");    // 7-Character 
  IupSetAttribute(sci, "STYLEFGCOLOR9", "0 0 255");    // 9-Preprocessor block 
  IupSetAttribute(sci, "STYLEFGCOLOR10", "255 0 255"); // 10-Operator 
  IupSetAttribute(sci, "STYLEBOLD10", "YES");
  // 11-Identifier  

  //IupSetAttribute(sci, "STYLEHOTSPOT6", "YES");
  
  IupSetAttribute(sci, "INSERT0", sampleCode);

  IupSetAttribute(sci, "MARGINWIDTHN0", "50");

  if (0)
  {
    IupSetAttribute(sci, "PROPERTY", "fold,1");
    IupSetAttribute(sci, "PROPERTY", "fold.compact,0");
    IupSetAttribute(sci, "PROPERTY", "fold.comment,1");
    IupSetAttribute(sci, "PROPERTY", "fold.preprocessor,1");

    IupSetAttribute(sci, "MARGINWIDTH1", "0");
    IupSetAttribute(sci, "MARGINTYPE1",  "MARGIN_SYMBOL");
    IupSetAttribute(sci, "MARGINMASK1",  "MASK_FOLDERS");
    IupSetAttribute(sci, "MARGINWIDTH1", "20");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDER,PLUS");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDEROPEN,MINUS");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDEREND,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDERMIDTAIL,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDEROPENMID,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDERSUB,EMPTY");
    IupSetAttribute(sci, "MARKERDEFINE", "FOLDERTAIL,EMPTY");

    IupSetAttribute(sci, "FOLDFLAGS", "LINEAFTER_CONTRACTED");

    IupSetAttribute(sci, "MARGINSENSITIVE1", "YES");
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
//  IupSetAttribute(sci, "VISIBLECOLUMNS", "80");
//  IupSetAttribute(sci, "VISIBLELINES", "40");
  //IupSetAttribute(sci, "SCROLLBAR", "NO");
//  IupSetAttribute(sci, "BORDER", "NO");
  IupSetAttribute(sci, "EXPAND", "Yes");

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
  IupSetAttribute(dlg, "RASTERSIZE", "700x500");
  IupSetAttribute(dlg, "MARGIN", "10x10");

  // Shows dialog
  IupShow(dlg);
  IupSetAttribute(dlg, "RASTERSIZE", NULL);

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

