------------------------------------------------------------------------------
-- Plot class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "plot",
  parent = iup.WIDGET,
  creation = "",
  funcname = "Plot",
  subdir = "ctrl",
  callback = {
    drawsample_cb = "nnddn",
    select_cb = "nnddn",
    selectbegin_cb = "",
    selectend_cb = "",
    predraw_cb = "n",   -- fake definitions to be replaced by plotfuncs module
    postdraw_cb = "n",  -- fake definitions to be replaced by plotfuncs module
    delete_cb = "nndd",
    deletebegin_cb = "",
    deleteend_cb = "",
    plotmotion_cb = "dd",
    plotbutton_cb = "nndds",
  },
  include = "iup_plot.h",
  extrafuncs = 1,
}

function ctrl.createElement(class, param)
   return iup.Plot(param.action)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
