require("iuplua")
require("iupluaimglib")


--********************************** Utilities *****************************************


function str_find(str, str_to_find, casesensitive, start)
  if (not casesensitive) then
    return str_find(string.lower(str), string.lower(str_to_find), start)
  end

  return string.find(str, str_to_find, start)
end

function new_file(ih)
  local dlg = iup.GetDialog(ih)
  local multitext = iup.GetDialogChild(dlg, "MULTITEXT")

  dlg.title = "Untitled - Simple Notepad"
  multitext.filename = nil
  multitext.dirty = "NO"
  multitext.value = ""
end

function open_file(ih, filename)
  local ifile = io.open(filename, "r")
  local str = ifile:read("*a")
  if (str) then
    local dlg = iup.GetDialog(ih)
    local multitext = iup.GetDialogChild(dlg, "MULTITEXT")
    local config = multitext.CONFIG
    dlg.title = filename.." - Simple Notepad"
    multitext.filename = filename
    multitext.dirty = "NO"
    multitext.value = str
    config:RecentUpdate(filename)
  end
  ifile:close()
end

function save_check(ih)
  local multitext = iup.GetDialogChild(ih, "MULTITEXT")
  if (multitext.dirty == "YES") then
    local resp = iup.Alarm("Warning", "File not saved! Save it now?", "Yes", "No", "Cancel")
    if resp == 1 then -- save the changes and continue
      item_save:action()
    elseif resp == 3 then  -- cancel
      return 0
    else  -- ignore the changes and continue
    end
  end
  return 1
end


--********************************** Callbacks *****************************************--


function item_goto:action()
  local multitext = iup.GetDialogChild(item_goto, "MULTITEXT")
  local line_count = multitext.linecount

  local lbl_goto = iup.label{title = "Line Number [1-"..line_count.."]:"}
  local txt_goto = iup.text{mask = iup.MASK_UINT, name = "LINE_TEXT", visiblecolumns = 20} --unsigned integer number
  local bt_goto_ok = iup.button{title = "OK", text_linecount = line_count, padding = "10x2", action = goto_ok_action}
  local bt_goto_cancel = iup.button{title = "Cancel", padding = "10x2", action = goto_cancel_action}

  function bt_goto_ok:action()
    local line_count = tonumber(self.text_linecount)
    local txt = iup.GetDialogChild(self, "LINE_TEXT")
    local line = tonumber(txt.value)
    if (line < 1 or line >= line_count) then
      iup.Message("Error", "Invalid line number.")
    end
    local goto_dlg = iup.GetDialog(self)
    goto_dlg.status =  "1"
    return iup.CLOSE
  end

  function bt_goto_cancel:action()
    local goto_dlg = iup.GetDialog(self)
    goto_dlg.status =  "0"
    return iup.CLOSE
  end

  box = iup.vbox{
    lbl_goto,
    txt_goto,
    iup.hbox{
      iup.fill{},
      bt_goto_ok,
      bt_goto_cancel, 
      normalizesize="HORIZONTAL",
    }, 
    margin = "10x10", 
    gap = "5",
  }
  local dlg = iup.dialog{
    box, 
    title = "Go To Line", 
    dialogframe = "Yes", 
    defaultenter = bt_goto_ok, 
    defaultesc = bt_goto_cancel,
    parentdialog = iup.GetDialog(self),
  }

  dlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

  if (dlg.status == 1) then
    local line = txt_goto.value
    local pos = iup.TextConvertLinColToPos(multitext, line, 0)
    multitext.caretpos = pos
    multitext.scrolltopos = pos
  end

  dlg:destroy()
end

function item_find_next:action()
  local find_dlg = self.find_dialog
  if (find_dlg) then
    local multitext = find_dlg.multitext
    local find_pos = multitext.find_pos
    local txt = iup.GetDialogChild(find_dlg, "FIND_TEXT")
    local str_to_find = txt.value

    local tgl_case = iup.GetDialogChild(find_dlg, "FIND_CASE")
    local casesensitive = tgl_case.value

    -- test again, because it can be called from the hot key
    if (not str_to_find) then
      return
    end
    if (not find_pos) or (find_pos == -1) then
      find_pos = 0
    end

    local str = multitext.value

    local pos, end_pos = str_find(str, str_to_find, casesensitive, find_pos)

    if (not pos) then
      local pos, end_pos = str_find(str, str_to_find, casesensitive)  -- try again from the start
    end

    if (pos) and (pos >= 0) then
      pos = pos - 1
      multitext.find_pos = end_pos
      multitext.selectionpos = pos..":"..end_pos
      multitext.find_selection = pos..":"..end_pos

      local lin, col = iup.TextConvertPosToLinCol(multitext, pos)
      local pos = iup.TextConvertLinColToPos(multitext, lin, 0)  -- position at col=0, just scroll lines 
      multitext.scrolltopos = pos
    else
      multitext.find_pos = -1
      iup.Message("Warning", "Text not found.")
    end
  end
end

function set_find_replace_visibility(find_dlg, show_replace)
  local replace_txt = iup.GetDialogChild(find_dlg, "REPLACE_TEXT")
  local  replace_lbl = iup.GetDialogChild(find_dlg, "REPLACE_LABEL")
  local replace_bt = iup.GetDialogChild(find_dlg, "REPLACE_BUTTON")

  if (show_replace) then
    replace_txt.visible = "Yes"
    replace_lbl.visible = "Yes"
    replace_bt.visible = "Yes"
    replace_txt.floating = "No"
    replace_lbl.floating = "No"
    replace_bt.floating  = "No"
  else
    replace_txt.floating = "Yes"
    replace_lbl.floating = "Yes"
    replace_bt.floating = "Yes"
    replace_txt.visible = "No"
    replace_lbl.visible = "No"
    replace_bt.visible = "No"
  end

  iup.SetAttribute(iup.GetDialog(replace_txt), "SIZE", nil)  -- force a dialog resize
  iup.Refresh(replace_txt)
end

function create_find_dialog()
  local txt_find = iup.text{name = "FIND_TEXT", visiblecolumns = "20"}
  local txt_find_replace = iup.text{name = "REPLACE_TEXT", visiblecolumns = "20"}
  local tgl_find_case = iup.toggle{title = "Case Sensitive", name = "FIND_CASE"}
  local bt_find_next = iup.button{title = "Find Next", padding = "10x2", action = find_next_action_cb}
  local bt_find_replace = iup.button{title = "Replace", padding = "10x2", action = find_replace_action_cb, name = "REPLACE_BUTTON"}
  local bt_find_close = iup.button{title = "Close", action = find_close_action_cb, padding = "10x2"}

  function bt_find_replace:action()
    local find_dlg = self.find_dialog
    local find_pos = multitext.find_pos
    local selectionpos = multitext.selectionpos
    local find_selection = multitext.find_selection

    if (find_pos == -1 or not (selectionpos) or not (find_selection) or (selectionpos ~= find_selection)) then
      item_find_next:action()
    else
      local replace_txt = iup.GetDialogChild(find_dlg, "REPLACE_TEXT")
      local str_to_replace = replace_txt.value
      multitext.selectedtext = str_to_replace

      -- then find next
      item_find_next:action()
    end
  end

  function bt_find_close:action()
    iup.Hide(iup.GetDialog(self))  -- do not destroy, just hide
  end

  local box = iup.vbox{
    iup.label{title = "Find What:"},
    txt_find,
    iup.label{title = "Replace with:", name="REPLACE_LABEL"},
    txt_find_replace,
    tgl_find_case,
    iup.hbox{
      iup.fill{},
      bt_find_next,
      bt_find_replace,
      bt_find_close,
      normalizesize="HORIZONTAL"
    },
    margin = "10x10", 
    gap = "5"
  }

  local find_dlg = iup.dialog{
    box, 
    title = "Find", 
    dialogframe = "Yes", 
    defaultenter = bt_next, 
    defaultesc = bt_close,
    parentdialog = iup.GetDialog(multitext)
    }

  -- Save the multiline to access it from the callbacks
  find_dlg.multitext = multitext

  -- Save the dialog to reuse it 
  find_dlg.find_dialog = find_dlg  -- from itself
  iup.SetAttribute(iup.GetDialog(multitext), "FIND_DIALOG", find_dlg) -- from the main dialog */

  return find_dlg
end

function item_find:action()
  local find_dlg = self.find_dialog
  local multitext = iup.GetDialogChild(self, "MULTITEXT")

  if (not find_dlg) then
    find_dlg = create_find_dialog()
  end
  set_find_replace_visibility(find_dlg, 0)

  -- centerparent first time, next time reuse the last position
  find_dlg:showxy(find_dlg, iup.CURRENT, iup.CURRENT)

  local str = multitext.selectedtext
  if (str) then
    txt = iup.GetDialogChild(find_dlg, "FIND_TEXT")
    txt.value = str
  end
end

function btn_find:action()
  item_find:action()
end

function item_replace:action()
  local find_dlg = self.find_dialog
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  if (not find_dlg) then
    find_dlg = create_find_dialog()
  end
  set_find_replace_visibility(1)

  -- centerparent first time, next time reuse the last position
  find_dlg:showxy(iup.CURRENT, iup.CURRENT)

  local str = multitext.selectedtext
  if (str) then
    local txt = iup.GetDialogChild(find_dlg, "FIND_TEXT")
    txt.value = str
  end
end

function selection_find_next:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local str = multitext.selectedtext
  if (str) then
    if (not find_dlg) then
      local find_dlg = create_find_dialog()
    end
    local txt = iup.GetDialogChild(find_dlg, "FIND_TEXT")
    txt.value = str
    find_next_action_cb()
  end
end

function toggle_visibility(item, bar)
  if (item.value == "ON") then
    bar.floating = "YES"
    bar.visible = "NO"
    item.value = "OFF"
  else
    bar.floating = "NO"
    bar.visible = "YES"
    item.value = "ON"
  end
  iup.Refresh(bar)  -- refresh the dialog layout
end

function item_toolbar:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local toolbar = iup.GetChild(iup.GetParent(multitext), 0)
  local config = multitext.config

  toggle_visibility(self, toolbar_hb)
  config:SetVariableStr("MainWindow", "Toolbar", item_toolbar.value)
end

function item_statusbar:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local statusbar = iup.GetChild(iup.GetParent(multitext), 0)
  local config = multitext.config
  toggle_visibility(self, lbl_statusbar)
  config:SetVariableStr("MainWindow", "Statusbar", item_statusbar.value)
end

function item_font:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local font = multitext.font
  local fontdlg = iup.fontdlg{value = font, parentdialog = iup.GetDialog(self)}

  fontdilg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

  if ((fontdlg.status) == "1") then
    local config = multitext.config
    multitext.font = fontdlg.value
    config:SetVariableStr("MainWindow", "Font", font)
  end

  fontdlg:destroy()
end

function item_copy:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local clipboard = iup.clipboard{text = multitext.selectedtext}
  clipboard:destroy()
end

function btn_copy:action()
  item_copy:action()
end

function item_paste:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local clipboard = iup.clipboard{}
  multitext.insert = clipboard.text
  clipboard:destroy()
  return iup.IGNORE  -- avoid system processing for hot keys, to correctly parse line feed
end

function btn_paste:action()
  return item_paste:action()
end

function item_cut:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  local clipboard = iup.clipboard{text = multitext.selectedtext}
  multitext.selectedtext = ""
  clipboard:destroy()
end

function btn_cut:action()
  return item_cut:action()
end

function item_delete:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  multitext.selectedtext = ""
end

function item_select_all:action()
  local multitext = iup.GetDialogChild(self, "MULTITEXT")
  iup.SetFocus(multitext)
  multitext.selection = "ALL"
end

function item_help:action()
  iup.Help("http://www.tecgraf.puc-rio.br/iup")
end

function item_about:action()
  iup.Message("About", "   Simple Notepad\n\nAutors:\n   Gustavo Lyrio\n   Antonio Scuri")
end


function create_main_dialog()
  local multitext = iup.text{
    multiline="YES", 
    expand="YES", 
    name="multitext", 
    dirty="NO"
    }
  function multitext:dropfiles_cb(filename)
    if (save_check(self)) then
  	  open_file(self, filename)
    end
  end
  function multitext:valuechanged_cb()
    self.dirty = "YES"
  end
  function multitext:caret_cb(lin, col, pos)
    lbl_statusbar.title = "Lin "..lin..", Col "..col
  end

  local lbl_statusbar = iup.label{title = "Lin 1, Col 1", name = "STATUSBAR", expand = "HORIZONTAL", padding = "10x5"}

  local item_new = iup.item{title = "New\tCtrl+N", image = "IUP_FileNew"}
  function item_new:action()
    if save_check(self) then
  	  new_file()
    end
  end

  local btn_new = iup.button{image = "IUP_FileNew", flat = "Yes"}
  function btn_new:action()
    item_new:action()
  end

  local item_open = iup.item{title = "&Open...\tCtrl+O", image = "IUP_FileOpen"}
  function item_open:action()
    local multitext = iup.GetDialogChild(self, "MULTITEXT")
    if save_check(self) then
      local filedlg = iup.filedlg{dialogtype = "OPEN", filter = "*.txt", filterinfo = "Text Files", parentidialog = iup.GetDialog(self)}
      filedlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)
      if (filedlg.status ~= "-1") then
        multitext.dirty = false
        open_file(filedlg.value)
      end
      filedlg:destroy()
    end
  end

  local btn_open = iup.button{imagem = "IUP_FileOpen", flat = "Yes"}
  function btn_open:action()
    item_open:action()
  end

  local item_save = iup.item{title = "Save\tCtrl+S", name = "ITEM_SAVE", image = "IUP_FileSave"}
  function item_save:action()
    local multitext = iup.GetDialogChild(self, "MULTITEXT")
    if (not multitext.filename) then
      item_saveas:action()
    else
      -- test again because in can be called using the hot key
      if (multitext.dirty) then
        local ifile = io.open(multitext.filename, "w")
        ifile:write(multitext.value)
        ifile:close()
        multitext.dirty = "NO"
      end
    end
  end

  local btn_save = iup.button{image = "IUP_FileSave", flat = "Yes"}
  function btn_save:action()
    item_save:action()
  end

  local item_saveas = iup.item{title = "Save &As...", name = "ITEM_SAVEAS"}
  function item_saveas:action()
    local multitext = iup.GetDialogChild(self, "MULTITEXT")
    local filedlg = iup.filedlg{dialogtype = "SAVE", filter = "*.txt", filterinfo = "Text Files", parentdialog = iup.GetDialog(self)}

    filedlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

    if (filedlg.status ~= "-1") then
      local ifile = io.open(filedlg.value, "w")
      local config = multitext.config
      ifile:write(multitext.value)
      ifile:close()
      config:RecentUpdate(filedlg.value)
      multitext.dirty = "NO"
    end
    filedlg:destroy()
  end

  local item_revert = iup.item{title = "Revert", name = "ITEM_REVERT"}
  function item_revert:action()
    open_file(multitext.filename)
  end

  local item_exit = iup.item{title = "E&xit"}
  function item_exit:action()
    local dlg = iup.GetDialog(self)
    local config = dlg.config

    if (not save_check(self)) then
      return iup.IGNORE  -- to abort the CLOSE_CB callback
    end
    config:DialogClosed(dlg, "MainWindow")
    config:Save()
    config:destroy()
    return iup.CLOSE
  end

  local item_find = iup.item{title = "&Find...\tCtrl+F", image = "IUP_EditFind"}
  local item_find_next = iup.item{title = "Find &Next\tF3", name = "ITEM_FINDNEXT"}
  local btn_find = iup.button{image = "IUP_EditFind", flat = "Yes"}
  local item_cut = iup.item{title = "Cut\tCtrl+X", name = "ITEM_CUT", image = "IUP_EditCut"}
  local item_copy = iup.item{title = "Copy\tCtrl+C", name = "ITEM_CUT", image = "IUP_EditCopy"}
  local item_paste = iup.item{title = "Paste\tCtrl+V", name = "ITEM_PASTE", image = "IUP_EditPaste"}
  local item_delete = iup.item{title = "Delete\tDel", image = "IUP_EditErase", name = "ITEM_DELETE"}
  local item_select_all = iup.item{title = "Select All\tCtrl+A"}

  local btn_cut = iup.button{image = "IUP_EditCut", flat = "Yes"}
  local btn_copy = iup.button{image =  "IUP_EditCopy", flat = "Yes"}
  local btn_paste = iup.button{image = "IUP_EditPaste", flat = "Yes"}

  local toolbar_hb = iup.hbox{
    btn_new,
    btn_open,
    btn_save,
    iup.label{separator="VERTICAL"},
    btn_cut,
    btn_copy,
    btn_paste,
    iup.label{separator="VERTICAL"},
    btn_find}

  local item_toolbar = iup.item{title="&Toobar..."}
  local item_statusbar = iup.item{title="&Statusbar..."}
  local item_goto = iup.item{title="&Go To...\tCtrl+G"}
  local item_font = iup.item{title="&Font..."}
  local item_help = iup.item{title="&Help..."}
  local item_about = iup.item{title="&About..."}

  local recent_menu = iup.menu{}

  local file_menu = iup.menu{
    item_new,
    item_open,
    item_save,
    item_saveas,
    item_revert,
    iup.separator{},
    iup.submenu{title="Recent &Files", recent_menu},
    item_exit
    }

  function file_menu:open_cb()
    --local item_revert = iup.GetDialogChild(self, "ITEM_REVERT")
    --local item_save = iup.GetDialogChild(self, "ITEM_SAVE")
    --local multitext = iup.GetDialogChild(self, "MULTITEXT")
    if (multitext.dirty) then
      item_save.active = "YES"
    else
      item_save.active = "NO"
    end
    if (multitext.dirty and multitext.filename) then
      item_revert.active = "YES"
    else
      item_revert.active = "NO"
    end
  end

  local edit_menu = iup.menu{
    item_cut,
    item_copy,
    item_paste,
    item_delete,
    iup.separator{},
    item_find,
    item_find_next,
    item_replace,
    item_goto,
    iup.separator{},
    item_select_all
    }

  function edit_menu:open_cb()
    local find_dlg = self.find_dialog
    local item_paste = iup.GetDialogChild(self, "ITEM_PASTE")
    local item_cut = iup.GetDialogChild(self, "ITEM_CUT")
    local item_delete = iup.GetDialogChild(self, "ITEM_DELETE")
    local item_copy = iup.GetDialogChild(self, "ITEM_COPY")
    local item_find_next = iup.GetDialogChild(self, "ITEM_FINDNEXT")
    local multitext = iup.GetDialogChild(self, "MULTITEXT")
    local clipboard = iup.clipboard{}

    if (not clipboard.textavailable) then
      item_paste.active = "NO"
    else
      item_paste.active = "YES"
    end

    if (not multitext.selectedtext) then
      item_cut.active = "NO"
      item_delete.active = "NO"
      item_copy.active = "NO"
    else
      item_cut.active = "YES"
      item_delete.active = "YES"
      item_copy.active = "YES"
    end

    if (find_dlg) then
      local txt = iup.GetDialogChild(find_dlg, "FIND_TEXT")
      if (txt) and (txt.value ~= "") then
        item_find_next.active = "YES"
      else
        item_find_next.active = "NO"
      end
    else
      item_find_next.active = "NO"
    end

    clipboard:destroy()
  end

  local format_menu = iup.menu{
    item_font}
  local view_menu = iup.menu{
    item_toolbar,
    item_statusbar}
  local help_menu = iup.menu{
    item_help,
    item_about}

  local sub_menu_file = iup.submenu{title = "&File", file_menu}
  local sub_menu_edit = iup.submenu{title = "&Edit", edit_menu}
  local sub_menu_format = iup.submenu{title = "F&ormat", format_menu}
  local sub_menu_view = iup.submenu{title = "&View", view_menu}
  local sub_menu_help = iup.submenu{title = "&Help", help_menu}

  local menu = iup.menu{
    sub_menu_file,
    sub_menu_edit,
    sub_menu_format,
    sub_menu_view,
    sub_menu_help}

  local vbox = iup.vbox{
    toolbar_hb,
    multitext,
    lbl_statusbar}

  local dlg = iup.dialog{
    vbox, menu=menu}

  function dlg:dropfiles_cb(filename)
    if (save_check(self)) then
  	  open_file(self, filename)
    end
  end
  function dlg:k_any(c)
    if (c == iup.K_cN) then
      item_new:action()
    elseif (c == iup.K_cO) then
      item_open:action()
    elseif (c == iup.K_cS) then
      item_save:action()
    elseif (c == iup.K_cF) then
      item_find:action()
    elseif (c == iup.K_cG) then
      item_goto:action()
    elseif (c == iup.K_F3) then
      find_next_action_cb()
    elseif (c == iup.K_cF3) then
      selection_find_next_action_cb()
    elseif (c == iup.K_cV) then
      item_paste:action()
    end
    return iup.DEFAULT
  end

  return dlg
end

config = iup.config{}
config.app_name = "simple_notepad"
config:Load()

function config:recent_cb()
  if save_check(self) then
    open_file(item_recent.title)
  end
end

local dlg = create_main_dialog()

dlg:showrxy(iup.CENTERPARENT, iup.CENTERPARENT)
dlg.usersize = nil -- remove minimum size restriction

-- initialize the current file
new_file(dlg)

-- open a file from the command line (allow file association in Windows)
if (argc > 1 and argv[1]) then
  filename = argv[1]
  open_file(filename)
end

if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
  iup.Close()
end
