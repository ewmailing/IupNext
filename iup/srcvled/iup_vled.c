#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iup.h>
#include <iup_scintilla.h>
#include <iup_config.h>

#include "iup_str.h"


#define FOLDING_MARGIN "20"

static Ihandle* get_config(Ihandle* ih)
{
  Ihandle* config = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "CONFIG_HANDLE");
  return config;
}

static int item_help_action_cb(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

static int item_about_action_cb(void)
{
  IupMessage("About", "   IUP Visual LED\n\nAuthors:\n   Camilo Freire\n   Antonio Scuri");
  return IUP_DEFAULT;
}

static char* getLedKeywords(void)
{
  return "IMAGE IMAGERGB IMAGERGBA TIMER USER BUTTON CANVAS FLATBUTTON FLATTOGGLE DIALOG FILL FILEDLG MESSAGEDLG COLORDLG FONTDLG PROGRESSBAR FRAME FLATFRAME HBOX ITEM LABEL FLATLABEL LIST "
    "SBOX SCROLLBOX FLATSCROLLBOX DETACHBOX BACKGROUNDBOX EXPANDER DROPBUTTON MENU MULTILINE RADIO SEPARATOR SUBMENU TEXT VAL TREE TABS FLATTABS TOGGLE VBOX ZBOX GRIDBOX NORMALIZER LINK "
    "CBOX FLATSEPARATOR SPACE SPIN SPINBOX SPLIT GAUGE COLORBAR COLORBROWSER DIAL ANIMATEDLABEL CELLS MATRIX MATRIXLIST MATRIXEX GLCANVAS GLBACKGROUNDBOX OLECONTROL PLOT MGLPLOT SCINTILLA "
    "WEBBROWSER GLCANVASBOX GLSUBCANVAS GLLABEL GLSEPARATOR GLBUTTON GLTOGGLE GLTEXT GLPROGRESSBAR GLVAL GLLINK GLFRAME GLEXPANDER GLSCROLLBOX GLSIZEBOX FLATMULTIBOX FLATLIST FLATVAL";
}

static const char *getLastNonAlphaNumeric(const char *text)
{
  int len = (int)strlen(text);
  const char *c = text + len - 1;
  if (*c == '\n')
    c--;
  for (; c != text; c--)
  {
    if (*c < 48 || (*c > 57 && *c < 65) || (*c > 90 && *c < 97) || *c > 122)
      return c + 1;
  }
  return NULL;
}

static char *filterList(const char *text, const char *list)
{
  char *filteredList[1024];
  char *retList;
  int count = 0;

  int i, len;
  const char *lastValue = list;
  const char *nextValue = iupStrNextValue(list, (int)strlen(list), &len, ' ');
  while (len != 0)
  {
    if ((int)strlen(text) <= len && iupStrEqualPartial(lastValue, text))
    {
      char *value = malloc(80);

      strncpy(value, lastValue, len);
      value[len] = 0;
      filteredList[count++] = value;
    }
    lastValue = nextValue;
    nextValue = iupStrNextValue(nextValue, (int)strlen(nextValue), &len, ' ');
  }

  retList = malloc(1024);
  retList[0] = 0;
  for (i = 0; i < count; i++)
  {
    if (i == 0)
    {
      strcpy(retList, filteredList[i]);
      strcat(retList, " ");
    }
    else
    {
      strcat(retList, filteredList[i]);
      strcat(retList, " ");
    }
  }

  for (i = 0; i < count; i++)
    free(filteredList[i]);

  return retList;
}

static int multitext_valuechanged_cb(Ihandle* multitext)
{
  if (IupGetInt(multitext, "AUTOCOMPLETION"))
  {
    const char *t;
    int pos = IupGetInt(multitext, "CARETPOS");
    char *text = IupGetAttribute(multitext, "VALUE");
    text[pos + 1] = '\0';
    t = getLastNonAlphaNumeric(text);
    if (t != NULL && *t != '\n' && *t != 0)
    {
      char *fList = filterList(t, getLedKeywords());
      if (strlen(fList) > 0)
        IupSetAttributeId(multitext, "AUTOCSHOW", (int)strlen(t) - 1, fList);
      free(fList);
    }

    return IUP_DEFAULT;
  }

  return IUP_CONTINUE;
}

static int multitext_kesc_cb(Ihandle *multitext)
{
  if (!IupGetInt(multitext, "AUTOCOMPLETION"))
    return IUP_CONTINUE;

  if (IupGetInt(multitext, "AUTOCACTIVE"))
    IupSetAttribute(multitext, "AUTOCCANCEL", "YES");

  return IUP_CONTINUE;
}


static int multitext_map_cb(Ihandle* multitext)
{
  Ihandle* config = get_config(multitext);
  const char *value;

  IupSetAttribute(multitext, "LEXERLANGUAGE", "led");
  IupSetAttribute(multitext, "KEYWORDS0", getLedKeywords());

  IupSetAttribute(multitext, "STYLEFGCOLOR1", "0 128 0");    /* 1-Led comment */
  IupSetAttribute(multitext, "STYLEFGCOLOR2", "255 128 0");  /* 2-Number  */
  IupSetAttribute(multitext, "STYLEFGCOLOR3", "0 0 255");    /* 3-Keyword  */
  IupSetAttribute(multitext, "STYLEFGCOLOR4", "164 0 164");  /* 4-String  */
  IupSetAttribute(multitext, "STYLEFGCOLOR5", "164 0 164");  /* 5-Character  */
  IupSetAttribute(multitext, "STYLEFGCOLOR6", "164 0 0");   /* 6-Operator  */
  /* 3, 8 and 9 - are not used */
  IupSetAttribute(multitext, "STYLEBOLD10", "YES");

  IupSetAttribute(multitext, "MARKERHIGHLIGHT", "YES");

  IupSetAttributeId(multitext, "MARKERBGCOLOR", 25, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDEREND */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 25, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDEREND */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 26, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDEROPENMID */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 26, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDEROPENMID */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 27, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDERMIDTAIL */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 27, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDERMIDTAIL */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 28, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDERTAIL */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 28, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDERTAIL */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 29, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDERSUB */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 29, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDERSUB */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 30, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDER */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 30, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDER */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 31, "0 0 0");         /* BGCOLOR para SC_MARKNUM_FOLDEROPEN */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 31, "255 255 255");   /* FGCOLOR para SC_MARKNUM_FOLDEROPEN */

  IupSetAttribute(multitext, "PROPERTY", "fold=1");
  IupSetAttribute(multitext, "PROPERTY", "fold.compact=0"); /* avoid folding of blank lines */
  IupSetAttribute(multitext, "_IUP_FOLDDING", "1");

  /* Folding margin=3 */
  IupSetAttribute(multitext, "MARGINWIDTH3", FOLDING_MARGIN);
  IupSetAttribute(multitext, "MARGINMASKFOLDERS3", "Yes");
  IupSetAttribute(multitext, "MARGINSENSITIVE3", "YES");

  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDER=BOXPLUS");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDEROPEN=BOXMINUS");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDEREND=BOXPLUSCONNECTED");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDERMIDTAIL=TCORNER");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDEROPENMID=BOXMINUSCONNECTED");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDERSUB=VLINE");
  IupSetAttribute(multitext, "MARKERDEFINE", "FOLDERTAIL=LCORNER");

  IupSetAttribute(multitext, "FOLDFLAGS", "LINEAFTER_CONTRACTED");

  value = IupConfigGetVariableStr(config, "LedScripter", "CommentColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR1", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "CommentColor", IupGetAttribute(multitext, "STYLEFGCOLOR1"));

  value = IupConfigGetVariableStr(config, "LedScripter", "NumberColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR2", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "NumberColor", IupGetAttribute(multitext, "STYLEFGCOLOR4"));

  value = IupConfigGetVariableStr(config, "LedScripter", "KeywordColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR3", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "KeywordColor", IupGetAttribute(multitext, "STYLEFGCOLOR5"));

  value = IupConfigGetVariableStr(config, "LedScripter", "StringColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR4", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "StringColor", IupGetAttribute(multitext, "STYLEFGCOLOR6"));

  value = IupConfigGetVariableStr(config, "LedScripter", "CharacterColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR5", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "CharacterColor", IupGetAttribute(multitext, "STYLEFGCOLOR7"));

  value = IupConfigGetVariableStr(config, "LedScripter", "OperatorColor");
  if (value)
    IupSetStrAttribute(multitext, "STYLEFGCOLOR6", value);
  else
    IupConfigSetVariableStr(config, "LedScripter", "OperatorColor", IupGetAttribute(multitext, "STYLEFGCOLOR10"));

  value = IupConfigGetVariableStr(config, "LedScripter", "AutoCompletion");
  if (value)
    IupSetStrAttribute(multitext, "AUTOCOMPLETION", value);

  value = IupConfigGetVariableStr(config, "LedScripter", "Folding");
  if (iupStrBoolean(value))
  {
    IupSetAttribute(multitext, "MARGINWIDTH3", FOLDING_MARGIN);
    IupSetAttribute(multitext, "PROPERTY", "fold=1");
    IupSetAttribute(multitext, "_IUP_FOLDDING", "1");
  }
  else
  {
    IupSetAttribute(multitext, "MARGINWIDTH3", "0");
    IupSetAttribute(multitext, "PROPERTY", "fold=0");
    IupSetAttribute(multitext, "_IUP_FOLDDING", NULL);
  }
  IupSetAttribute(multitext, "FOLDALL", "EXPAND");

  return IUP_DEFAULT;
}

static int newtext_cb(Ihandle* ih, Ihandle *multitext)
{
  (void)ih;
  /* this is called before the multitext is mapped */
  IupSetCallback(multitext, "VALUECHANGED_CB", (Icallback)multitext_valuechanged_cb);
  IupSetCallback(multitext, "K_ESC", (Icallback)multitext_kesc_cb);
  IupSetCallback(multitext, "MAP_CB", (Icallback)multitext_map_cb);

  IupSetAttribute(multitext, "AUTOCOMPLETION", "OFF");

#ifdef WIN32
  IupSetAttribute(multitext, "FONT", "Consolas, 11");
#else
  IupSetAttribute(multitext, "FONT", "Monospace, 12");
  /* Other alternatives for "Consolas" in Linux: "DejaVu Sans Mono", "Liberation Mono", "Nimbus Mono L", "FreeMono" */
#endif

  return IUP_DEFAULT;
}

static int configload_cb(Ihandle *ih, Ihandle* config)
{
  const char* value;

  value = IupConfigGetVariableStr(config, "LedScripter", "AutoCompletion");
  if (value)
  {
    Ihandle* ih_item = IupGetDialogChild(ih, "ITM_AUTOCOMPLETE");
    IupSetStrAttribute(ih_item, "VALUE", value);
  }

  value = IupConfigGetVariableStr(config, "LedScripter", "Folding");
  if (value)
  {
    Ihandle* ih_item = IupGetDialogChild(ih, "ITM_FOLDING");
    IupSetStrAttribute(ih_item, "VALUE", value);
  }

  return IUP_DEFAULT;
}

static int marker_changed_cb(Ihandle *ih, Ihandle *multitext, int lin, int margin)
{
  if (margin == 3)
    IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);

  (void)ih;
  return IUP_DEFAULT;
}

static Ihandle* get_current_multitext(Ihandle* ih)
{
  Ihandle* tabs = IupGetDialogChild(ih, "MULTITEXT_TABS");
  return (Ihandle*)IupGetAttribute(tabs, "VALUE_HANDLE");
}

static int led_menu_open_cb(Ihandle *ih_menu)
{
  Ihandle* menu_foldall = IupGetDialogChild(ih_menu, "ITM_FOLD_ALL");
  Ihandle* item_toggle_folding = IupGetDialogChild(ih_menu, "ITM_TOGGLE_FOLDING");
  Ihandle* item_folding = IupGetDialogChild(ih_menu, "ITM_FOLDING");
  Ihandle* item_comments = IupGetDialogChild(ih_menu, "ITM_COMMENTS");
  Ihandle* multitext = get_current_multitext(ih_menu);
  char *selpos = IupGetAttribute(multitext, "SELECTIONPOS");

  if (IupGetInt(item_folding, "VALUE"))
  {
    IupSetAttribute(item_toggle_folding, "ACTIVE", "Yes");
    IupSetAttribute(menu_foldall, "ACTIVE", "Yes");
  }
  else
  {
    IupSetAttribute(item_toggle_folding, "ACTIVE", "NO");
    IupSetAttribute(menu_foldall, "ACTIVE", "NO");
  }

  if (selpos)
    IupSetAttribute(item_comments, "ACTIVE", "Yes");
  else
    IupSetAttribute(item_comments, "ACTIVE", "NO");

  return IUP_DEFAULT;
}


static int item_autocomplete_action_cb(Ihandle* ih_item)
{
  Ihandle* tabs = IupGetDialogChild(ih_item, "MULTITEXT_TABS");
  int children_count = IupGetChildCount(tabs);
  Ihandle* multitext;
  Ihandle* config = get_config(ih_item);
  int i;

  if (IupGetInt(ih_item, "VALUE"))
  {
    IupSetAttribute(ih_item, "VALUE", "OFF");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);
      IupSetAttribute(multitext, "AUTOCOMPLETION", "OFF");
    }

  }
  else
  {
    IupSetAttribute(ih_item, "VALUE", "ON");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);
      IupSetAttribute(multitext, "AUTOCOMPLETION", "ON");
    }
  }

  IupConfigSetVariableStr(config, "LedScripter", "AutoCompletion", IupGetAttribute(ih_item, "VALUE"));

  return IUP_DEFAULT;
}


static int setparent_param_cb(Ihandle* param_dialog, int param_index, void* user_data)
{
  if (param_index == IUP_GETPARAM_MAP)
  {
    Ihandle* ih = (Ihandle*)user_data;
    IupSetAttributeHandle(param_dialog, "PARENTDIALOG", ih);
  }

  return 1;
}

static int item_style_config_action_cb(Ihandle* ih_item)
{
  Ihandle* tabs = IupGetDialogChild(ih_item, "MULTITEXT_TABS");
  int children_count = IupGetChildCount(tabs);
  Ihandle* multitext;
  Ihandle* config = get_config(ih_item);
  char commentColor[30], numberColor[30], keywordColor[30],
    stringColor[30], characterColor[30], operatorColor[30];
  int i;

  strcpy(commentColor, IupConfigGetVariableStr(config, "LedScripter", "CommentColor"));
  strcpy(numberColor, IupConfigGetVariableStr(config, "LedScripter", "NumberColor"));
  strcpy(keywordColor, IupConfigGetVariableStr(config, "LedScripter", "KeywordColor"));
  strcpy(stringColor, IupConfigGetVariableStr(config, "LedScripter", "StringColor"));
  strcpy(characterColor, IupConfigGetVariableStr(config, "LedScripter", "CharacterColor"));
  strcpy(operatorColor, IupConfigGetVariableStr(config, "LedScripter", "OperatorColor"));

  if (!IupGetParam("Syntax Colors", setparent_param_cb, IupGetDialog(ih_item),
    "Comment: %c\n"
    "Number: %c\n"
    "Keyword: %c\n"
    "String: %c\n"
    "Character: %c\n"
    "Operator: %c\n",
    commentColor, numberColor, keywordColor, stringColor, characterColor, operatorColor, NULL))
    return IUP_DEFAULT;

  IupConfigSetVariableStr(config, "LedScripter", "CommentColor", commentColor);
  IupConfigSetVariableStr(config, "LedScripter", "NumberColor", numberColor);
  IupConfigSetVariableStr(config, "LedScripter", "KeywordColor", keywordColor);
  IupConfigSetVariableStr(config, "LedScripter", "StringColor", stringColor);
  IupConfigSetVariableStr(config, "LedScripter", "CharacterColor", characterColor);
  IupConfigSetVariableStr(config, "LedScripter", "OperatorColor", operatorColor);

  for (i = 0; i < children_count; i++)
  {
    multitext = IupGetChild(tabs, i);

    IupSetStrAttribute(multitext, "STYLEFGCOLOR1", commentColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR2", numberColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR3", keywordColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR4", stringColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR5", characterColor);
    IupSetStrAttribute(multitext, "STYLEFGCOLOR6", operatorColor);
  }

  return IUP_DEFAULT;
}

static int item_folding_action_cb(Ihandle* ih)
{
  Ihandle* tabs = IupGetDialogChild(ih, "MULTITEXT_TABS");
  int children_count = IupGetChildCount(tabs);
  Ihandle* multitext;
  Ihandle* config = get_config(ih);
  int i;

  if (IupGetInt(ih, "VALUE"))
  {
    IupSetAttribute(ih, "VALUE", "OFF");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);

      IupSetAttribute(multitext, "PROPERTY", "fold=0");
      IupSetAttribute(multitext, "MARGINWIDTH3", "0");
      IupSetAttribute(multitext, "_IUP_FOLDDING", NULL);
    }
  }
  else
  {
    IupSetAttribute(ih, "VALUE", "ON");

    for (i = 0; i < children_count; i++)
    {
      multitext = IupGetChild(tabs, i);

      IupSetAttribute(multitext, "PROPERTY", "fold=1");
      IupSetAttribute(multitext, "MARGINWIDTH3", FOLDING_MARGIN);
      IupSetAttribute(multitext, "_IUP_FOLDDING", "1");
    }
  }

  IupSetAttribute(multitext, "FOLDALL", "EXPAND");

  IupConfigSetVariableStr(config, "LedScripter", "Folding", IupGetAttribute(ih, "VALUE"));

  return IUP_DEFAULT;
}

static int item_toggle_folding_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);
  int pos = IupGetInt(multitext, "CARETPOS");
  int lin, col;

  /* must test again because it can be called by the hot key */
  if (!IupGetInt(multitext, "_IUP_FOLDDING"))
    return IUP_DEFAULT;

  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);

  if (!IupGetIntId(multitext, "FOLDLEVELHEADER", lin))
  {
    lin = IupGetIntId(multitext, "FOLDPARENT", lin);
    if (lin < 0)
      return IUP_DEFAULT;
  }

  IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
  IupSetfAttribute(multitext, "CARET", "%d:0", lin);

  return IUP_DEFAULT;
}

static int item_fold_collapse_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "CONTRACT");

  return IUP_DEFAULT;
}

static int item_fold_expand_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "EXPAND");

  return IUP_DEFAULT;
}

static int item_fold_toggle_action_cb(Ihandle* ih)
{
  Ihandle* multitext = get_current_multitext(ih);

  IupSetAttribute(multitext, "FOLDALL", "TOGGLE");

  return IUP_DEFAULT;
}

static int item_fold_level_action_cb(Ihandle* ih_item)
{
  int level = 0, action = 0;
  Ihandle* config = get_config(ih_item);

  const char* value = IupConfigGetVariableStr(config, "LedScripter", "FoldAllLevel");
  if (value) iupStrToInt(value, &level);

  value = IupConfigGetVariableStr(config, "LedScripter", "FoldAllLevelAction");
  if (value) iupStrToInt(value, &action);

  if (IupGetParam("Fold All by Level", setparent_param_cb, IupGetDialog(ih_item),
    "Level: %i\n"
    "Options: %o|Collapse|Expand|Toggle|\n",
    &level, &action, NULL))
  {
    Ihandle* multitext = get_current_multitext(ih_item);
    int lin, count = IupGetInt(multitext, "LINECOUNT");

    IupConfigSetVariableInt(config, "LuaScripter", "FoldAllLevel", level);
    IupConfigSetVariableInt(config, "LuaScripter", "FoldAllLevelAction", action);

    for (lin = 0; lin < count; lin++)
    {
      if (IupGetIntId(multitext, "FOLDLEVELHEADER", lin))
      {
        int foldLevel = IupGetIntId(multitext, "FOLDLEVEL", lin);
        if (foldLevel + 1 == level) /* level at header is different from child */
        {
          switch (action)
          {
            case 0: /* Collapse */
              if (IupGetIntId(multitext, "FOLDEXPANDED", lin))
                IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
              break;
            case 1: /* Expand */
              if (!IupGetIntId(multitext, "FOLDEXPANDED", lin))
                IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
              break;
            case 2: /* Toggle */
              IupSetfAttribute(multitext, "FOLDTOGGLE", "%d", lin);
              break;
          }
        }
      }
    }
  }

  return IUP_DEFAULT;
}

static int item_linescomment_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = get_current_multitext(ih_item);
  char *sel = IupGetAttribute(multitext, "SELECTION");
  char *text_line;
  int lin, col, lin1, lin2, col1, col2;

  if (!sel)
    return IUP_DEFAULT;

  sscanf(sel, "%d,%d:%d,%d", &lin1, &col1, &lin2, &col2);

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  for (lin = lin1; lin <= lin2; lin++)
  {
    int len, pos;
    text_line = IupGetAttributeId(multitext, "LINE", lin);
    len = (int)strlen(text_line);

    for (col = 0; col < len; col++)
    {
      char c = text_line[col];
      if (c != ' ' && c != '\t')
        break;
    }

    IupTextConvertLinColToPos(multitext, lin, col, &pos);
    IupSetAttributeId(multitext, "INSERT", pos, "# ");
  }

  IupSetAttribute(multitext, "UNDOACTION", "END");

  IupSetStrf(multitext, "SELECTION", "%d,0:%d,999", lin1, lin2);

  return IUP_DEFAULT;
}

static int item_linesuncomment_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = get_current_multitext(ih_item);
  char *sel = IupGetAttribute(multitext, "SELECTION");
  char *text_line;
  int lin, col, lin1, lin2, col1, col2;

  if (!sel)
    return IUP_DEFAULT;

  sscanf(sel, "%d,%d:%d,%d", &lin1, &col1, &lin2, &col2);

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  for (lin = lin1; lin <= lin2; lin++)
  {
    int len, pos, nChar;
    text_line = IupGetAttributeId(multitext, "LINE", lin);
    len = (int)strlen(text_line);
    nChar = 0;

    for (col = 0; col < len; col++)
    {
      if (text_line[col] == '#')
      {
        nChar = 1;
        if (text_line[col + 2] == ' ')
          nChar++;
        break;
      }
    }

    if (nChar == 0)
      continue;

    IupTextConvertLinColToPos(multitext, lin, col, &pos);
    IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, nChar);
  }

  IupSetAttribute(multitext, "UNDOACTION", "END");

  IupSetStrf(multitext, "SELECTION", "%d,0:%d,999", lin1, lin2);

  return IUP_DEFAULT;
}

static Ihandle* buildLedMenu(void)
{
  Ihandle *item_autocomplete, *item_style_config, *item_expand, *item_toggle, *item_level,
    *item_folding, *item_toggle_folding, *ledMenu, *item_collapse,
    *item_linescomment, *item_linesuncomment;

  item_autocomplete = IupItem("Auto Completion", NULL);
  IupSetAttribute(item_autocomplete, "NAME", "ITM_AUTOCOMPLETE");
  IupSetCallback(item_autocomplete, "ACTION", (Icallback)item_autocomplete_action_cb);

  item_style_config = IupItem("Syntax Colors...", NULL);
  IupSetAttribute(item_style_config, "NAME", "ITM_STYLE");
  IupSetCallback(item_style_config, "ACTION", (Icallback)item_style_config_action_cb);

  item_folding = IupItem("Folding", NULL);
  IupSetAttribute(item_folding, "NAME", "ITM_FOLDING");
  IupSetCallback(item_folding, "ACTION", (Icallback)item_folding_action_cb);

  item_toggle_folding = IupItem("Toggle Fold\tF8", NULL);
  IupSetAttribute(item_toggle_folding, "NAME", "ITM_TOGGLE_FOLDING");
  IupSetCallback(item_toggle_folding, "ACTION", (Icallback)item_toggle_folding_action_cb);

  item_collapse = IupItem("Collapse", NULL);
  IupSetAttribute(item_collapse, "NAME", "ITM_COLLAPSE");
  IupSetCallback(item_collapse, "ACTION", (Icallback)item_fold_collapse_action_cb);

  item_expand = IupItem("Expand", NULL);
  IupSetAttribute(item_expand, "NAME", "ITM_EXPAND");
  IupSetCallback(item_expand, "ACTION", (Icallback)item_fold_expand_action_cb);

  item_toggle = IupItem("Toggle", NULL);
  IupSetAttribute(item_toggle, "NAME", "ITM_TOGGLE");
  IupSetCallback(item_toggle, "ACTION", (Icallback)item_fold_toggle_action_cb);

  item_level = IupItem("by Level...", NULL);
  IupSetAttribute(item_level, "NAME", "ITM_LEVEL");
  IupSetCallback(item_level, "ACTION", (Icallback)item_fold_level_action_cb);

  item_linescomment = IupItem("Lines Comment", NULL);
  IupSetCallback(item_linescomment, "ACTION", (Icallback)item_linescomment_action_cb);

  item_linesuncomment = IupItem("Lines Uncomment", NULL);
  IupSetCallback(item_linesuncomment, "ACTION", (Icallback)item_linesuncomment_action_cb);

  ledMenu = IupMenu(
    item_folding,
    item_toggle_folding,
    IupSubmenu("Fold All",
    IupSetAttributes(IupMenu(
    item_collapse,
    item_expand,
    item_toggle,
    item_level,
    NULL), "NAME=ITM_FOLD_ALL")),
    IupSeparator(),
    IupSubmenu("Comments",
    IupSetAttributes(IupMenu(
    item_linescomment,
    item_linesuncomment,
    NULL), "NAME=ITM_COMMENTS")),
    IupSeparator(),
    item_autocomplete,
    IupSeparator(),
    item_style_config,
    NULL);

  IupSetCallback(ledMenu, "OPEN_CB", (Icallback)led_menu_open_cb);

  return IupSubmenu("&Led", ledMenu);
}

int main(int argc, char **argv)
{
  Ihandle *main_dialog;
  Ihandle *config;
  Ihandle *menu;
  Ihandle *ledMenu;
  int i;

  IupOpen(&argc, &argv);
  IupImageLibOpen();
  IupScintillaOpen();

#ifdef _DEBUG
  IupSetGlobal("GLOBALLAYOUTDLGKEY", "Yes");
#endif

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "iupvled");
  IupConfigLoad(config);

  main_dialog = IupScintillaDlg();

  ledMenu = buildLedMenu();
  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupInsert(menu, IupGetChild(menu, IupGetChildCount(menu) - 1), ledMenu);

  IupSetAttribute(main_dialog, "SUBTITLE", "IUP Visual LED");
  IupSetAttributeHandle(main_dialog, "CONFIG", config);

  IupSetAttribute(main_dialog, "EXTRAFILTERS", "Led Files|*.led|");

  IupSetCallback(main_dialog, "NEWTEXT_CB", (Icallback)newtext_cb);
  IupSetCallback(main_dialog, "CONFIGLOAD_CB", (Icallback)configload_cb);
  IupSetCallback(main_dialog, "MARKERCHANGED_CB", (Icallback)marker_changed_cb);

  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupAppend(menu, IupSubmenu("&Help", IupMenu(
    IupSetCallbacks(IupItem("&Help...", NULL), "ACTION", (Icallback)item_help_action_cb, NULL),
    IupSetCallbacks(IupItem("&About...", NULL), "ACTION", (Icallback)item_about_action_cb, NULL),
    NULL)));

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, main_dialog, IupGetAttribute(main_dialog, "SUBTITLE"));

  /* open a file from the command line (allow file association in Windows) */
  for (i = 1; i < argc; i++)
  {
    const char* filename = argv[i];
    IupSetStrAttribute(main_dialog, "OPENFILE", filename);
  }

  /* Call NEW_TEXT_CB because the first tab was already created */
  newtext_cb(main_dialog, get_current_multitext(main_dialog));

  IupMainLoop();

  IupDestroy(config);
  IupDestroy(main_dialog);

  IupClose();
  return EXIT_SUCCESS;
}

