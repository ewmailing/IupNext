dlg_list = {}
dlg_list_count = 0


function btn_popup_action()
  local dlg = new_dlg2()
  dlg:popup(iup.CENTER, iup.CENTER)
end

function btn_close_cb(self)
  self:destroy()
  return iup.IGNORE
end  

function btn_execute(self)
  local dlg = iup.GetDialog(self)
  local txt = dlg[1][1][1]
  local i = 1
  while i < 100000 do
   i = i + 1
   txt.value = tostring(i/100000)
   iup.LoopStep()
  end
  return iup.CLOSE

end

function new_dlg()
  local dlg = iup.dialog
  {
     iup.vbox
     {
        iup.hbox
        {
          iup.text{ size=200},
        },
        iup.hbox
        {
          iup.button{ title = "Popup", action = btn_popup_action},
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

function new_dlg2()
  local dlg = iup.dialog
  {
     iup.vbox
     {
        iup.hbox
        {
          iup.gauge{ size=200},
          iup.button{ title = "Execute", action = btn_execute}
        },
        iup.hbox
        {
          iup.button{ title = "Popup", action = btn_popup_action},
          iup.button{ title = "Close", action = "return iup.CLOSE"},
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
maindlg:popup(iup.CENTER, iup.CENTER) 