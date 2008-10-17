dlg_list = {}
dlg_list_count = 0

function btn_show_action()
  local dlg = new_dlg()
  dlg:showxy(iup.CENTER, iup.CENTER)
end

function btn_popup_action()
  local dlg = new_dlg()
  dlg:popup(iup.CENTER, iup.CENTER)
end                         

function btn_close_cb(self)
  self:destroy()
  return iup.IGNORE
end

function btn_hide_action(self)
  local dlg = iup.GetDialog(self)
  dlg:hide()
end

function btn_execute(self)
  local dlg = iup.GetDialog(self)
  local txt = dlg[1][1][1]
  iup.dostring(txt.value)
end

function new_dlg()
  local dlg = iup.dialog
  {
     iup.vbox
     {
        iup.hbox
        {
          iup.text{ size=200},
          iup.button{ title = "Execute", action = btn_execute}
        },
        iup.hbox
        {
          iup.button{ title = "Show", action = btn_show_action},
          iup.button{ title = "Popup", action = btn_popup_action},
          iup.button{ title = "Close", action = "return iup.CLOSE"},
          iup.button{ title = "Hide", action = btn_hide_action}
        };
        margin = "10x10"
     };
     title = "Modality Test",
     close_cb = btn_close_cb
  }
  dlg_list_count = dlg_list_count + 1
  dlg_list[dlg_list_count] = dlg
  dlg.title = "#"..dlg_list_count.." "..dlg.title
  return dlg
end

maindlg = new_dlg()
maindlg:showxy(iup.CENTER, iup.CENTER) 
