require("iuplua")
require("iupluaimglib")

function str_find(str, str_to_find, casesensitive, start)
  if (not casesensitive) then
    return str_find(string.lower(str), string.lower(str_to_find), true, start)
  end

  return string.find(str, str_to_find, start)
end

lbl_statusbar = iup.label{title = "Lin 1, Col 1", expand = "HORIZONTAL", padding = "10x5"}

multitext = iup.text{
	multiline = "YES",
	expand = "YES"
}

item_open = iup.item{title="Open..."}
item_saveas = iup.item{title="Save As..."}
item_font = iup.item{title="Font..."}
item_about = iup.item{title="About..."}
item_find = iup.item{title="Find..."}
item_goto = iup.item{title="Go To..."}
item_exit = iup.item{title="Exit"}

function multitext:caret_cb(lin, col)
  lbl_statusbar.title = "Lin "..lin..", Col "..col
end

function item_open:action()
  local filedlg = iup.filedlg{
    dialogtype = "OPEN", 
    filter = "*.txt", 
    filterinfo = "Text Files", 
    pareintaldialog=iup.GetDialog(self)
    }

  filedlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

  if (tonumber(filedlg.status) ~= -1) then
    local filename = filedlg.value
    local ifile = io.open(filename, "r")
    if (ifile) then
      local str = ifile:read("*a")
      ifile:close()
      if (str) then
        multitext.value = str
      else
        iup.Message("Error", "Fail when reading from file: " .. filename)
      end
    else
      iup.Message("Error", "Can't open file: " .. filename)
    end
  end
  filedlg:destroy()
end

function item_saveas:action()
  local filedlg = iup.filedlg{
    dialogtype = "SAVE", 
    filter = "*.txt", 
    filterinfo = "Text Files", 
    parentaldialog=iup.GetDialog(self)
    }

  filedlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

  if (tonumber(filedlg.status) ~= -1) then
    local filename = filedlg.value
    local ifile = io.open(filename, "w")
    if (ifile) then
      if (not ifile:write(multitext.value)) then
        iup.Message("Error", "Fail when writing to file: " .. filename)
      end
      ifile:close()
    else
      iup.Message("Error", "Can't open file: " .. filename)
    end
  end
  filedlg:destroy()
end

function item_exit:action()
	return iup.CLOSE
end

function item_goto:action()
  local line_count = multitext.linecount
  local lbl_goto = iup.label{title = "Line Number [1-"..line_count.."]:"}
  local txt_goto = iup.text{mask = iup.MASK_UINT, visiblecolumns = 20} --unsigned integer numbers only

  local bt_goto_ok = iup.button{title = "OK", text_linecount = 0, padding = "10x2"} 
  bt_goto_ok.text_linecount = line_count
  function bt_goto_ok:action()
    local line_count = tonumber(self.text_linecount)
    local line = tonumber(txt_goto.value)
    if (line < 1 or line >= line_count) then
      iup.Message("Error", "Invalid line number.")
      return
    end
    goto_dlg.status = 1
    return iup.CLOSE
  end

  local bt_goto_cancel = iup.button{title = "Cancel", padding = "10x2"}
  function bt_goto_cancel:action()
    goto_dlg.status = 0
    return iup.CLOSE
  end

  local box = iup.vbox{
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
  goto_dlg = iup.dialog{
    box,
    title = "Go To Line", 
    dialogframe = "Yes", 
    defaultenter = bt_goto_ok, 
    defaultesc = bt_goto_cancel,
    parentdialog = iup.GetDialog(self)
  }

  goto_dlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

  if (tonumber(goto_dlg.status) == 1) then
    local line = txt_goto.value
    local pos = iup.TextConvertLinColToPos(multitext, line, 0)
    multitext.caretpos = pos
    multitext.scrolltopos = pos
  end

  goto_dlg:destroy()
end

function item_find:action()
  local find_dlg = self.find_dialog
  if (not find_dlg) then
    local txt_find = iup.text{visiblecolumns = "20"}
    local tgl_find_case = iup.toggle{title = "Case Sensitive"}
    local bt_find_next = iup.button{title = "Find Next", padding = "10x2"}
    local bt_find_close = iup.button{title = "Close", padding = "10x2"}

    function bt_find_next:action()
      local find_pos = multitext.find_pos
      local str_to_find = txt_find.value

      local casesensitive = tgl_find_case.value

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

        iup.SetFocus(multitext)
        multitext.selectionpos = pos..":"..end_pos

        local lin, col = iup.TextConvertPosToLinCol(multitext, pos)
        local pos = iup.TextConvertLinColToPos(multitext, lin, 0)  -- position at col=0, just scroll lines
        multitext.scrolltopos = pos
      else
        multitext.find_pos = -1
        iup.Message("Warning", "Text not found.")
      end
    end

    function bt_find_close:action()
      iup.Hide(iup.GetDialog(self))  -- do not destroy, just hide
    end

    box = iup.vbox{
      iup.label{title = "Find What:"},
      txt_find,
      tgl_find_case,
      iup.hbox{
        iup.fill{},
        bt_find_next,
        bt_find_close,
        normalizesize="HORIZONTAL", 
      },
      margin = "10x10", 
      gap = "5",
    }

    find_dlg = iup.dialog{
      box, 
      title = "Find", 
      dialogframe = "Yes", 
      defaultenter = bt_next, 
      defaultesc = bt_close,
      parentdialog = iup.GetDialog(self)
      }

    -- Save the dialog to reuse it 
    self.find_dialog = find_dlg -- from the main dialog */
  end

  -- centerparent first time, next time reuse the last position
  find_dlg:showxy(iup.CURRENT, iup.CURRENT)
end

function item_font:action()
  local font = multitext.font
  local fontdlg = iup.fontdlg{value = font, parentdialog=iup.GetDialog(self)}

  fontdlg:popup(iup.CENTERPARENT, iup.CENTERPARENT)

  if (tonumber(fontdlg.status) == 1) then
    multitext.font = fontdlg.value
  end

  fontdlg:destroy()
end

function item_about:action()
  iup.Message("About", "   Simple Notepad\n\nAutors:\n   Gustavo Lyrio\n   Antonio Scuri")
end

file_menu = iup.menu{item_open,item_saveas,iup.separator{},item_exit}
edit_menu = iup.menu{item_find, item_goto}
format_menu = iup.menu{item_font}
help_menu = iup.menu{item_about}
sub_menu_file = iup.submenu{file_menu, title = "File"}
sub_menu_edit = iup.submenu{edit_menu, title = "Edit"}
sub_menu_format = iup.submenu{format_menu, title = "Format"}
sub_menu_help = iup.submenu{help_menu, title = "Help"}

menu = iup.menu{
  sub_menu_file, 
  sub_menu_edit, 
  sub_menu_format, 
  sub_menu_help,
  }

btn_open = iup.button{image = "IUP_FileOpen", flat = "Yes", action = item_open.action }
btn_save = iup.button{image = "IUP_FileSave", flat = "Yes", action = item_saveas.action}
btn_find = iup.button{image = "IUP_EditFind", flat = "Yes", action = item_find.action}

toolbar_hb = iup.hbox{
  btn_open,
  btn_save,
  iup.label{separator="VERTICAL"},
  btn_find,
  margin = "5x5",
  gap = 2,
  }

vbox = iup.vbox{
  toolbar_hb,
	multitext,
  lbl_statusbar,
}

dlg = iup.dialog{
	vbox,
	title = "Simple Notepad",
	size = "HALFxHALF",
	menu = menu
}

-- parent for pre-defined dialogs in closed functions (IupMessage)
iup.SetGlobal("PARENTDIALOG", iup.SetHandleName(dlg))

dlg:showxy(iup.CENTER,iup.CENTER)
dlg.usersize = nil

-- to be able to run this script inside another context
if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
  iup.Close()
end
