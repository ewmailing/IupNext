require"iuplua"
require"iupluaimglib"

butt = iup.button{image= "IUP_IMGBUT_NEW"}
dlg = iup.dialog{butt,title="test"}

function dlg:close_cb()
  iup.ExitLoop()
  dlg:destroy()
  return iup.IGNORE
end

dlg:show()

iup.MainLoop()
