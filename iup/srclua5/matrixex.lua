------------------------------------------------------------------------------
-- MatrixEx class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "matrixex",
  parent = iup.WIDGET,
  creation = "",
  funcname = "MatrixEx",
  callback = {
    pastesize_cb = "nn",
    busy_cb = "nns",
  },
  include = "iupmatrixex.h",
  extrafuncs = 1,
}

function ctrl.createElement(class, param)
   return iup.MatrixEx(param.action)
end

function ctrl.setcell(handle, l, c, val)
  iup.MatSetAttribute(handle,"",l,c,val)
end

function ctrl.getcell(handle, l, c)
  return iup.MatGetAttribute(handle,"",l,c)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
