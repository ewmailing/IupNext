------------------------------------------------------------------------------
-- Menu class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "menu",
  parent = BOX,
  creation = "-",
  callback = {
    open_cb = "",
    menuclose_cb = "",
  }
}

function ctrl.popup(handle, x, y)
  Popup(handle, x, y)
end

function ctrl.append(handle, elem)
  Append(handle, elem)
end

function ctrl.createElement(class, param)
  local n = #param
  for i=1,n do
    if type(param[i]) == "table" then 
      itemarg = {}
      for u,v in pairs(param[i]) do
        if type(u) ~= "number" then
          itemarg[u] = v
        end
      end
      if type(param[i][1]) == "string" and (type(param[i][2]) == "function" or type(param[i][2]) == "string") then
        itemarg.title = param[i][1]
        itemarg.action = param[i][2]
        param[i] = item(itemarg)
      elseif type(param[i][1]) == "string" and type(param[i][2]) == "userdata" then
        itemarg[1] = param[i][2]
        itemarg.title = param[i][1]
        param[i] = submenu(itemarg)
      end
    end
  end
   return Menu()
end

function ctrl.showxy(handle, x, y)
  return ShowXY(handle, x, y)
end

function ctrl.destroy(handle)
  return Destroy(handle)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
