------------------------------------------------------------------------------
-- Dialog class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "dialog",
  parent = iup.WIDGET,
  creation = "I",
  callback = {
    map_cb = "",
    unmap_cb = "",
    close_cb = "",
    show_cb = "n",
    trayclick_cb = "nnn",
    dropfiles_cb = "snnn",
  }
}

function ctrl.createElement(class, arg)
   return iup.Dialog(arg[1])
end

function ctrl.popup(handle, x, y)
  iup.Popup(handle,x,y)
end

function ctrl.showxy(handle, x, y)
  return iup.ShowXY(handle, x, y)
end

function ctrl.destroy(handle)
  return iup.Destroy(handle)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
