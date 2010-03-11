------------------------------------------------------------------------------
-- Tabs class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "tabs",
  parent = WIDGET,
  creation = "v",
  callback = {
    tabchange_cb = "ii",
  },
  funcname = "Tabsv",
  createfunc = [[
static int Tabsv(lua_State *L)
{
  Ihandle **hlist = iuplua_checkihandle_array(L, 1);
  Ihandle *h = IupTabsv(hlist);
  iuplua_plugstate(L, h);
  iuplua_pushihandle_raw(L, h);
  free(hlist);
  return 1;
}

]],
}

function ctrl.createElement(class, param)
  return Tabsv(param)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
