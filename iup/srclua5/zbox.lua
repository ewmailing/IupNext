------------------------------------------------------------------------------
-- ZBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "zbox",
  parent = BOX,
  creation = "-",
  callback = {}
}

function ctrl.append (handle, elem)
  ihandle_setname(elem)
  Append(handle, elem)
end

function ctrl.SetChildrenNames(obj)
  if obj then
    local i = 1
    while obj[i] do
      ihandle_setname(obj[i])
      i = i+1
    end
  end
end

function ctrl.createElement(class, param)
   ctrl.SetChildrenNames(param)
   return Zbox()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
