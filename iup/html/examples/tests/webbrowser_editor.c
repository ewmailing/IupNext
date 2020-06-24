/*
 * IupWebBrowser Editor sample
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iup.h>
#include <iupweb.h>


static int wb_navigate_cb(Ihandle* self, char* url)
{
  printf("NAVIGATE_CB: %s\n", url);
  (void)self;
  if (strstr(url, "download")!=NULL)
    return IUP_IGNORE;
  return IUP_DEFAULT;
}
                   
static int wb_error_cb(Ihandle* self, char* url)
{
  printf("ERROR_CB: %s\n", url);
  (void)self;
  return IUP_DEFAULT;
}

static int wb_completed_cb(Ihandle* self, char* url)
{
  printf("COMPLETED_CB: %s\n", url);
  printf("CANGOBACK: %s\n", IupGetAttribute(self, "CANGOBACK"));
  printf("CANGOFORWARD: %s\n", IupGetAttribute(self, "CANGOFORWARD"));
  (void)self;
  return IUP_DEFAULT;
}

static int wb_updatecommands_cb(Ihandle* self)
{
  printf("UPDATECOMMANDS_CB\n");
  (void)self;
  return IUP_DEFAULT;
}

static int wb_newwindow_cb(Ihandle* self, char* url)
{
  printf("NEWWINDOW_CB: %s\n", url);
  IupSetAttribute(self, "VALUE", url);
  return IUP_DEFAULT;
}

static int open_cb(Ihandle* self)
{
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute(filedlg, "EXTFILTER", "HTML Files|*.html;*.htm;*.mhtml|All Files|*.*|");

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "OPENFILE", filename);
  }

  IupDestroy(filedlg);

  return IUP_DEFAULT;
}

static int save_cb(Ihandle* self)
{
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
  IupSetAttribute(filedlg, "EXTFILTER", "HTML Files|*.html;*.htm;*.mhtml|All Files|*.*|");

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "SAVEFILE", filename);
  }

  IupDestroy(filedlg);

  return IUP_DEFAULT;
}

static int new_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "NEW", NULL);
  return IUP_DEFAULT;
}

static int print_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "PRINT", NULL);
  return IUP_DEFAULT;
}

static int cmd_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "EXECCOMMAND", IupGetAttribute(self, "CMD"));
  return IUP_DEFAULT;
}

static int enterwindow_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "COMMAND", IupGetAttribute(self, "CMD"));
  printf("COMMAND=%s\n", IupGetAttribute(web, "COMMAND"));
  printf("  COMMANDSTATE=%s\n", IupGetAttribute(web, "COMMANDSTATE"));
  printf("  COMMANDENABLED=%s\n", IupGetAttribute(web, "COMMANDENABLED"));
  return IUP_DEFAULT;
}

static int enterwindow2_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "COMMAND", IupGetAttribute(self, "CMD2"));
  printf("COMMAND=%s\n", IupGetAttribute(web, "COMMAND"));
  //printf("  COMMANDTEXT=%s\n", IupGetAttribute(web, "COMMANDTEXT")); returned NULL for all
  printf("  COMMANDVALUE=%s\n", IupGetAttribute(web, "COMMANDVALUE"));
  printf("  %s=%s\n", IupGetAttribute(web, "COMMAND"), IupGetAttribute(web, IupGetAttribute(web, "COMMAND")));
  return IUP_DEFAULT;
}

static int insertimage_cb(Ihandle* self)
{
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute(filedlg, "EXTFILTER", "Image Files|*.jpeg;*.jpg;*.png;*.gif;*.svg|All Files|*.*|");

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "INSERTIMAGEFILE", filename);
  }

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

static int fgcolor_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  unsigned char r = 0, g = 0, b = 0;
  IupGetRGB(web, "FORECOLOR", &r, &g, &b);
  if (IupGetColor(IUP_MOUSEPOS, IUP_MOUSEPOS, &r, &g, &b) == 1)
    IupSetStrf(web, "FORECOLOR", "#%02X%02X%02X", (int)r, (int)g, (int)b);
  return IUP_DEFAULT;
}

static int bgcolor_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  unsigned char r = 0, g = 0, b = 0;
  IupGetRGB(web, "BACKCOLOR", &r, &g, &b);
  if (IupGetColor(IUP_MOUSEPOS, IUP_MOUSEPOS, &r, &g, &b) == 1)
    IupSetStrf(web, "BACKCOLOR", "#%02X%02X%02X", (int)r, (int)g, (int)b);
  return IUP_DEFAULT;
}

static int inserttext_cb(Ihandle* self)
{
  char text[1024] = "";
  if (IupGetText("Insert Text", text, 1024))
  {
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "INSERTTEXT", text);
  }
  return IUP_DEFAULT;
}

static int inserthtml_cb(Ihandle* self)
{
  char text[1024] = "";
  if (IupGetText("Insert HTML", text, 1024))
  {
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "INSERTHTML", text);
  }
  return IUP_DEFAULT;
}

static int fontname_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  char text[1024] = "";
  char* value = IupGetAttribute(web, "FONTNAME");
  if (value)
    strcpy(text, value);
  if (IupGetText("Font Name", text, 1024))
  {
    IupSetAttribute(web, "FONTNAME", text);
  }
  return IUP_DEFAULT;
}

static int fontsize_cb(Ihandle* self, char *text, int index, int state)
{
  if (state == 1)
  {
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetInt(web, "FONTSIZE", index);
  }
  (void)text;
  return IUP_DEFAULT;
}

static int formatblock_cb(Ihandle* self, char *text, int index, int state)
{
  if (state == 1)
  {
    const char* formats[9] = { "<H1>", "<H2>", "<H3>", "<H4>", "<H5>", "<H6>", "<P>", "<PRE>", "<BLOCKQUOTE>" };
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "FORMATBLOCK", formats[index-1]);
  }
  (void)text;
  return IUP_DEFAULT;
}

static int createlink_cb(Ihandle* self)
{
  char text[1024] = "";
  if (IupGetText("Create Link", text, 1024))
  {
    Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
    IupSetAttribute(web, "CREATELINK", text);
  }
  return IUP_DEFAULT;
}

static int html_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  char* html = IupGetAttribute(web, "HTML");
  if (html)
  {
    char text[10240];
    strcpy(text, html);
    if (IupGetText("HTML Text", text, 10240))
      IupSetAttribute(web, "HTML", text);
  }
  return IUP_DEFAULT;
}

static int editable_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");

  if (IupGetInt(web, "EDITABLE"))
    IupSetAttribute(web, "EDITABLE", "No");
  else
    IupSetAttribute(web, "EDITABLE", "Yes");

  return IUP_DEFAULT;
}

static int find_cb(Ihandle* self)
{
  Ihandle* web = (Ihandle*)IupGetAttribute(self, "MY_WEB");
  IupSetAttribute(web, "FIND", NULL);
  return IUP_DEFAULT;
}

char *iupKeyCodeToName(int code);  /* for debugging purposes only */

static int keypress_cb(Ihandle *ih, int c, int pressed)
{
  if (iup_isprint(c))
    printf("KEYPRESS_CB(%d = %s \'%c\' (%d))\n", c, iupKeyCodeToName(c), (char)c, pressed);
  else
    printf("KEYPRESS_CB(%d = %s (%d))\n", c, iupKeyCodeToName(c), pressed);
  return IUP_DEFAULT;
}

static int button_cb(Ihandle *ih, int but, int pressed, int x, int y, char* status)
{
  printf("BUTTON_CB(but=%c (%d), x=%d, y=%d [%s])\n", (char)but, pressed, x, y, status);
  return IUP_DEFAULT;
}


extern void create_layout_rt_editor_images(void);

void WebBrowserEditorTest(void)
{
  Ihandle *dlg, *web;
  Ihandle *btOpen, *btSave, *btEditable;

  IupWebBrowserOpen();              

  // Creates an instance of the WebBrowser control
  web = IupWebBrowser();
  IupSetCallback(web, "KEYPRESS_CB", (Icallback)keypress_cb);
  IupSetCallback(web, "BUTTON_CB", (Icallback)button_cb);

  if (!IupGetHandle("rt_editor_bold"))
    create_layout_rt_editor_images();

  // Creates a dialog containing the control
  dlg = IupDialog(IupVbox(IupHbox(
                            IupSetAttributes(IupSetCallbacks(IupButton("New", NULL), "ACTION", new_cb, NULL), "IMAGE=IUP_FileNew"),
                            IupSetAttributes(btOpen = IupButton("Open...", NULL), "IMAGE=IUP_FileOpen"),
                            IupSetAttributes(btSave = IupButton("Save As...", NULL), "IMAGE=IUP_FileSave"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Print", NULL), "ACTION", print_cb, NULL), "IMAGE=IUP_Print"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Undo", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=UNDO, IMAGE=IUP_EditUndo"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Redo", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=REDO, IMAGE=IUP_EditRedo"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Cut", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=CUT, IMAGE=IUP_EditCut"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Copy", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=COPY, IMAGE=IUP_EditCopy"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Paste", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=PASTE, IMAGE=IUP_EditPaste"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Select All", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=SELECTALL"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Find", NULL), "ACTION", find_cb, NULL), "IMAGE=IUP_EditFind"),
                            IupSetAttributes(btEditable = IupButton("Editable", NULL), "IMAGE=rt_editor_pencil"),
                            NULL), 
                          IupHbox(
                            IupSetAttributes(IupSetCallbacks(IupButton("Bold", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=BOLD, IMAGE=rt_editor_bold"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Italic", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=ITALIC, IMAGE=rt_editor_italic"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Strikethrough", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=STRIKETHROUGH, IMAGE=rt_editor_strikethrough"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Underline", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=UNDERLINE, IMAGE=rt_editor_underline"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Left", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=JUSTIFYLEFT, IMAGE=rt_editor_left"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Center", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=JUSTIFYCENTER, IMAGE=rt_editor_center"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Right", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=JUSTIFYRIGHT, IMAGE=rt_editor_right"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Justified", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=JUSTIFYFULL, IMAGE=rt_editor_justified"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Indent", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=INDENT, IMAGE=rt_editor_indent"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Outdent", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=OUTDENT, IMAGE=rt_editor_outdent"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Subscript", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=SUBSCRIPT, IMAGE=rt_editor_subscript"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Superscript", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=SUPERSCRIPT, IMAGE=rt_editor_superscript"),
                            NULL),
                          IupHbox(
                            IupSetAttributes(IupSetCallbacks(IupButton("Remove Format", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=REMOVEFORMAT, IMAGE=rt_editor_erasure"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Delete", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=DELETE, IMAGE=IUP_EditErase"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Insert Ordered List", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=INSERTORDEREDLIST, IMAGE=rt_editor_numbered"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Insert Unordered List", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=INSERTUNORDEREDLIST, IMAGE=rt_editor_bullets"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Foreground Color...", NULL), "ACTION", fgcolor_cb, "ENTERWINDOW_CB", enterwindow2_cb, NULL), "CMD2=FORECOLOR, IMAGE=rt_editor_fgcolor"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Background Color...", NULL), "ACTION", bgcolor_cb, "ENTERWINDOW_CB", enterwindow2_cb, NULL), "CMD2=BACKCOLOR, IMAGE=rt_editor_bgcolor"),
                            NULL),
                          IupHbox(
                            IupSetAttributes(IupSetCallbacks(IupButton("Insert Image...", NULL), "ACTION", insertimage_cb, NULL), "IMAGE=rt_editor_picture"),
                            IupSetCallbacks(IupButton("Insert Text...", NULL), "ACTION", inserttext_cb, NULL), 
                            IupSetCallbacks(IupButton("Insert HTML...", NULL), "ACTION", inserthtml_cb, NULL), 
                            IupSetAttributes(IupSetCallbacks(IupButton("Create Link...", NULL), "ACTION", createlink_cb, NULL), "IMAGE=rt_editor_link"),
                            IupSetAttributes(IupSetCallbacks(IupButton("Remove Link", NULL), "ACTION", cmd_cb, "ENTERWINDOW_CB", enterwindow_cb, NULL), "CMD=UNLINK"),
                            IupSetCallbacks(IupButton("Edit Source...", NULL), "ACTION", html_cb, NULL),
                            NULL),
                          IupHbox(
                            IupSetAttributes(IupSetCallbacks(IupButton("Font Name...", NULL), "ACTION", fontname_cb, "ENTERWINDOW_CB", enterwindow2_cb, NULL), "CMD2=FONTNAME"),
                            IupLabel("Font Size:"),
                            IupSetAttributes(IupSetCallbacks(IupList(NULL), "ACTION", (Icallback)fontsize_cb, "ENTERWINDOW_CB", enterwindow2_cb, NULL), "1=x-small, 2=small, 3=medium, 4=large, 5=x-large, 6=xx-large, 7=xx-large, DROPDOWN=Yes, CMD2=FONTSIZE"),
                            IupLabel("Format Block:"),
                            IupSetAttributes(IupSetCallbacks(IupList(NULL), "ACTION", (Icallback)formatblock_cb, "ENTERWINDOW_CB", enterwindow2_cb, NULL), "1=\"Heading 1\", 2 = \"Heading 2\", 3 = \"Heading 3\", 4 = \"Heading 4\", 5 = \"Heading 5\", 6 = \"Heading 6\", 7 = \"Paragraph\", 8 = \"Preformatted\", 9 = \"Block Quote\", DROPDOWN=Yes, CMD2=FORMATBLOCK"),
                            NULL),
                          web, NULL));
  IupSetAttribute(dlg, "TITLE", "IupWebBrowser");
  IupSetAttribute(dlg, "MY_WEB", (char*)web);
  IupSetAttribute(dlg, "RASTERSIZE", "x600");
  IupSetAttribute(dlg, "MARGIN", "5x5");
  IupSetAttribute(dlg, "GAP", "5");


  IupSetAttribute(web, "HTML", "<html><body><p><b>Hello</b> World! (חדפ)</p></body></html>");

  IupSetAttribute(web, "EDITABLE", "Yes");

  IupSetCallback(btOpen, "ACTION", (Icallback)open_cb);
  IupSetCallback(btSave, "ACTION", (Icallback)save_cb);
  IupSetCallback(btEditable, "ACTION", (Icallback)editable_cb);

  IupSetCallback(web, "NEWWINDOW_CB", (Icallback)wb_newwindow_cb);
  IupSetCallback(web, "NAVIGATE_CB", (Icallback)wb_navigate_cb);
  IupSetCallback(web, "ERROR_CB", (Icallback)wb_error_cb);
  IupSetCallback(web, "COMPLETED_CB", (Icallback)wb_completed_cb);
  IupSetCallback(web, "UPDATECOMMANDS_CB", (Icallback)wb_updatecommands_cb);

  // Shows dialog
  IupShow(dlg);
}

// IupNext will require EntryPoint
#ifndef USE_ENTRY_POINT
  #if defined(__APPLE__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    #define USE_ENTRY_POINT
  #endif
#endif

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

#ifdef USE_ENTRY_POINT
  // Platforms that require ENTRY_POINT must wait to start their IUP code until the entry point callback.
  IupSetFunction("ENTRY_POINT", (Icallback)WebBrowserEditorTest);
#else
  WebBrowserEditorTest();
#endif

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
