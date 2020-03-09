------------------------------------------------------------------------------
-- Tree class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "flattree",
  parent = iup.WIDGET,
  subdir = "elem",
  funcname = "FlatTree",
  creation = "",
  callback = {
--    selection_cb = "nn",
--    multiselection_cb = "nn",  -- fake definition to be replaced by treefuncs module
--    multiunselection_cb = "nn",  -- fake definition to be replaced by treefuncs module
--    branchopen_cb = "n",
--    branchclose_cb = "n",
--    executeleaf_cb = "n",
--    noderemoved_cb = "s",  -- fake definition to be replaced by treefuncs module
--    rename_cb = "ns",
--    showrename_cb = "n",
--    rightclick_cb = "n",
--    dragdrop_cb = "nnnn",
--    togglevalue_cb = "nn",
  },
--  extrafuncs = 1,
}

--iup.FLATTREEREFTABLE={}

function ctrl.createElement(class, param)
  return iup.FlatTree()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
