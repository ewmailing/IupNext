------------------------------------------------------------------------------
-- ProgressBar class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "progressbar",
  parent = WIDGET,
  creation = "",
  funcname = "ProgressBar",
  callback = {}
} 

function ctrl.createElement(class, param)
   return ProgressBar()
end
   
iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
