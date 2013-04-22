------------------------------------------------------------------------------
-- Scintilla class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "scintilla",
  parent = iup.WIDGET,
  creation = "",
  callback = {
  savepoint_cb = "n",
--  valuechanged_cb = "",
--  caret_cb = "nnn", 
  marginclick_cb = "nns",
  hotspotclick_cb = "nnns",
--  button_cb = "nnnns",
--  motion_cb = "nns",
  action = "nnns",
  },
  include = "iup_scintilla.h",
}

function ctrl.createElement(class, param)
  return iup.Scintilla()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
