------------------------------------------------------------------------------
-- MatrixList class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "matrixlist",
  parent = iup.WIDGET,
  creation = "",
  callback = {
    imagevaluechanged_cb = "nn",
    listclick_cb = "nns",
    listrelease_cb = "nns",
    listinsert_cb = "n",
    listremove_cb = "n",
--    listdraw_cb = "nnnnnnn",  implemented by matrix
    listedition_cb = "nnnn",
  },
  funcname = "MatrixList",
  include = "iupcontrols.h",
--  extrafuncs = 1,
}

function ctrl.createElement(class, param)
   return iup.MatrixList()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
