#include <stdio.h>
#include <stdlib.h>
#include <iup.h>

int str_goto(char *orig, int line) 
{
  int currline = 1;
  int c = 0;
  for (c = 0; c < strlen(orig); c++) 
  {
    if (currline == line)
      return c;
    else if (orig[c] == '\n')
      currline++;
  }
  return -1;
}

void str_find(char *orig, char *find, char *start, char *sel) 
{
  char *p_str = strstr(orig, find);
  if (p_str != NULL) 
  {
    int index = (int)(p_str - orig);
    snprintf(start, sizeof(orig), "%d", index);
    sprintf(sel, "%d:%d", index, index + (int)strlen(find));
  }
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) 
{
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  int len_rep;  // length of rep
  int len_with; // length of with
  int len_front; // distance between rep and end of last rep
  int count;    // number of replacements

  if (!orig)
    return NULL;
  if (!rep)
    rep = "";
  len_rep = strlen(rep);
  if (!with)
    with = "";
  len_with = strlen(with);

  ins = orig;
  for (count = 0; tmp = strstr(ins, rep); ++count) 
  {
    ins = tmp + len_rep;
  }
  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; // move to next "end of rep"
  }
  strcpy(tmp, orig);
  return result;
}

char* read_file(const char* filename)
{
  int size;
  char* str;
  FILE* file = fopen(filename, "rb");
  if (!file) 
  {
    IupMessagef("Error", "Can't open file: %s", filename);
    return NULL;
  }

  /* calculate file size */
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  /* allocate memory for the file contents + nul terminator */
  str = malloc(size + 1);
  /* read all data at once */
  fread(str, size, 1, file);
  /* set the nul terminator */
  str[size] = 0;

  fclose(file);
  return str;
}

void write_file(const char* filename, const char* str, int count)
{
  FILE* file = fopen(filename, "w");
  if (!file) 
  {
    IupMessagef("Error", "Can't open file: %s", filename);
    return;
  }

  fwrite(str, 1, count, file);
  fclose(file);
}

int item_open_action_cb(Ihandle* item_open)
{
  Ihandle* multitext = IupGetDialogChild(item_open, "MULTITEXT");
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute(filedlg, "FILTER", "*.txt");
  IupSetAttribute(filedlg, "FILTERINFO", "Text Files");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    char* str = read_file(filename);
    if (str)
    {
      IupSetStrAttribute(multitext, "VALUE", str);
      free(str);
    }
  }

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

int item_save_action_cb(Ihandle* item_save)
{
  Ihandle* multitext = IupGetDialogChild(item_save, "MULTITEXT");
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
  IupSetAttribute(filedlg, "FILTER", "*.txt");
  IupSetAttribute(filedlg, "FILTERINFO", "Text Files");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    char* str = IupGetAttribute(multitext, "VALUE");
    int count = IupGetInt(multitext, "COUNT");
    write_file(filename, str, count);
  }

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

int goto_bt_cb (void) 
{
    Ihandle *gotodlg = IupGetHandle("gotodlg");
	Ihandle *multitext = IupGetHandle("multitext");
	char *value = IupGetAttribute(multitext, "VALUE");
	Ihandle *gototxt = IupGetHandle("gototxt");
	int linetarget = IupGetInt(gototxt, "VALUE");
	int line = 0;
	char indexpos[sizeof(value)];
	line = str_goto(value, linetarget);
	if (line == -1) {
    	IupDestroy(gotodlg);
		IupMessage("Error", "No such line.");
		return IUP_ERROR;
	}
	sprintf(indexpos, "%d", line); 
	IupSetAttribute(multitext, "CARETPOS", indexpos);
    IupDestroy(gotodlg);
	return IUP_DEFAULT;
}

int goto_cb(void) 
{
	Ihandle *gotodlg;
    Ihandle *goto_bt = IupButton("Go To", 0);
    Ihandle *gototxt = IupText(NULL);
    IupSetHandle("gototxt", gototxt);

    Ihandle *hbox = IupHbox(
        gototxt,
        goto_bt,
        NULL
    );
    IupSetAttribute(hbox, "MARGIN", "10x10");
    IupSetAttribute(hbox, "GAP", "5");

    IupSetAttribute(hbox, "SIZE", "30");

    gotodlg = IupDialog(hbox);
    IupSetHandle("gotodlg", gotodlg);
    IupSetAttribute(gotodlg, "TITLE", "Go To");

    IupSetCallback(goto_bt, "ACTION", (Icallback)goto_bt_cb);
    IupPopup(gotodlg, IUP_CENTER, IUP_CENTER);

	return IUP_DEFAULT;
}

int replace_bt_cb (void) 
{
	Ihandle *multitext = IupGetHandle("multitext");
    Ihandle *findtxt = IupGetHandle("findtxt");
    Ihandle *replacetxt = IupGetHandle("replacetxt");
    Ihandle *replacedlg = IupGetHandle("replacedlg");
    char *findstr = IupGetAttribute(findtxt, "VALUE");
    char *replacestr = IupGetAttribute(replacetxt, "VALUE");
    char *value = IupGetAttribute(multitext, "VALUE");
	

    IupSetAttribute(multitext, "VALUE", str_replace(value, findstr, replacestr));
    IupDestroy(replacedlg);
	return IUP_DEFAULT;	 
}

int replace_cb(void) 
{
  Ihandle* multitext = IupGetDialogChild(item_font, "MULTITEXT");
  Ihandle *replacedlg;
    Ihandle *replace_bt = IupButton("Replace", 0);
	Ihandle *findtxt = IupText(NULL);
	IupSetHandle("findtxt", findtxt);
    Ihandle *replacetxt = IupText(NULL);
    IupSetHandle("replacetxt", replacetxt);
    Ihandle *hbox = IupHbox(
		IupVbox(
			IupLabel("Find:"),
			findtxt,
			NULL
		),
		IupVbox(
			IupLabel("Replace:"),
        	replacetxt,
			NULL
		),
        replace_bt,
        NULL
    );
    IupSetAttribute(hbox, "MARGIN", "10x10");
    IupSetAttribute(hbox, "GAP", "5");

    IupSetAttribute(hbox, "SIZE", "30");

    replacedlg = IupDialog(hbox);
    IupSetHandle("replacedlg", replacedlg);
    IupSetAttribute(replacedlg, "TITLE", "Replace");

    IupSetCallback(replace_bt, "ACTION", (Icallback)replace_bt_cb);
    IupPopup(replacedlg, IUP_CENTER, IUP_CENTER);

	return IUP_DEFAULT;
}

int find_bt_cb(void)
{
  Ihandle* multitext = IupGetDialogChild(item_font, "MULTITEXT");
  Ihandle *findtxt = IupGetHandle("findtxt");
	Ihandle *finddlg = IupGetHandle("finddlg");
	char *findstr = IupGetAttribute(findtxt, "VALUE");
    char *value = IupGetAttribute(multitext, "VALUE");

    char indexpos[sizeof(value)];
    char selection[sizeof(findstr)];
	str_find(value, findstr, indexpos, selection);
    if (indexpos != NULL) {
        IupSetAttribute(multitext, "CARETPOS", indexpos);
        IupSetAttribute(multitext, "SELECTIONPOS", selection);
    }
    else {
        char* msg;
        sprintf(msg, "Could not find %s in text.\n", findstr);
        IupMessage("Warning", msg);
    }
    IupDestroy(finddlg);
}

int find_cb(void)
{
  Ihandle* multitext = IupGetDialogChild(item_font, "MULTITEXT");
  Ihandle *finddlg;
	Ihandle *find_bt = IupButton("Find", 0);
	Ihandle *findtxt = IupText(NULL);
	IupSetHandle("findtxt", findtxt);
	Ihandle *hbox = IupHbox(
		findtxt,
		find_bt,
		NULL
	);
	IupSetAttribute(hbox, "MARGIN", "10x10");
	IupSetAttribute(hbox, "GAP", "5");

	IupSetAttribute(hbox, "SIZE", "30");

	finddlg = IupDialog(hbox);
  	IupSetHandle("finddlg", finddlg);
  	IupSetAttribute(finddlg, "TITLE", "Find");

  	IupSetCallback(find_bt, "ACTION", (Icallback)find_bt_cb);
	IupPopup(finddlg, IUP_CENTER, IUP_CENTER);
		
	return IUP_DEFAULT;
}

int item_font_action_cb(Ihandle* item_font)
{
  Ihandle* multitext = IupGetDialogChild(item_font, "MULTITEXT");
  Ihandle* fontdlg = IupFontDlg();
  char* font = IupGetAttribute(multitext, "FONT");
  IupSetStrAttribute(fontdlg, "VALUE", font);
  IupSetAttribute(fontdlg, "TITLE", "IupFontDlg Test");

  IupPopup(fontdlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(fontdlg, "STATUS") == 1)
  {
    char* font = IupGetAttribute(fontdlg, "VALUE");
    IupSetStrAttribute(multitext, "FONT", font);
  }

  IupDestroy(fontdlg);
  return IUP_DEFAULT;
}

int about_cb(void) 
{
  IupMessage("About", "   IUP Tutorial\n\nAutors:\n   Gustavo Lyrio\n   Antonio Scuri");
  return IUP_DEFAULT;
}

int exit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *vbox, *multitext, *menu;
  Ihandle *sub_menu_file, *file_menu, *item_exit, *item_open, *item_save;
  Ihandle *sub_menu_edit, *edit_menu, *item_find, *item_replace, *item_goto;
  Ihandle *sub_menu_format, *format_menu, *item_font;
  Ihandle *sub_menu_help, *help_menu, *item_about;

  IupOpen(&argc, &argv);

  multitext = IupText(NULL);
  IupSetAttribute(multitext, "MULTILINE", "YES");
  IupSetAttribute(multitext, "EXPAND", "YES");
  IupSetAttribute(multitext, "NAME", "MULTITEXT");

  item_open = IupItem("Open...", NULL);
  item_save = IupItem("Save...", NULL);
  item_exit = IupItem("Exit", NULL);
  item_find = IupItem ("Find..", NULL);
  item_replace = IupItem ("Replace..", NULL);
  item_goto = IupItem ("Go To...", NULL);
  item_font = IupItem("Font...", NULL);
  item_about = IupItem("About...", NULL);

  IupSetCallback(item_open, "ACTION", (Icallback)open_cb);
  IupSetCallback(item_save, "ACTION", (Icallback)save_cb);
  IupSetCallback(item_exit, "ACTION", (Icallback)exit_cb);
  IupSetCallback(item_find, "ACTION", (Icallback)find_cb);
  IupSetCallback(item_replace, "ACTION", (Icallback)replace_cb);
  IupSetCallback(item_goto, "ACTION", (Icallback)goto_cb);
  IupSetCallback(item_font, "ACTION", (Icallback)item_font_action_cb);
  IupSetCallback(item_about, "ACTION", (Icallback)about_cb);

  file_menu = IupMenu(
    item_open,
    item_save,
    IupSeparator(),
    item_exit,
    NULL);
	edit_menu = IupMenu(
    item_find,
    item_replace,
    item_goto,
    NULL);
  format_menu = IupMenu(item_font,
    NULL);
  help_menu = IupMenu(item_about,
    NULL);

  sub_menu_file = IupSubmenu("File", file_menu);
	sub_menu_edit = IupSubmenu("Edit", edit_menu);
  sub_menu_format = IupSubmenu("Format", format_menu);
  sub_menu_help = IupSubmenu("Help", help_menu);

  menu = IupMenu(
    sub_menu_file, 
    sub_menu_edit, 
    sub_menu_format, 
    sub_menu_help, 
    NULL);
	IupSetAttribute(menu, "_APP_MULTITEXT", (char*)multitext);

  vbox = IupVbox(
    multitext,
    NULL);

  dlg = IupDialog(vbox);
  IupSetAttributeHandle(dlg, "MENU", menu);
  IupSetAttribute(dlg, "TITLE", "Simple Notepad");
  IupSetAttribute(dlg, "SIZE", "QUARTERxQUARTER");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupSetAttribute(dlg, "USERSIZE", NULL);

  IupMainLoop();

  IupClose();
  return EXIT_SUCCESS;
}
