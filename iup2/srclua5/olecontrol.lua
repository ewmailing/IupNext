------------------------------------------------------------------------------
-- OleControl class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "olecontrol",
  parent = WIDGET,
  creation = "s",
  funcname = "OleControl",
  callback = {},
  include = "iupole.h",
  extracode = [[ 
int iupolelua_open(lua_State* L)
{
  if (iuplua_opencall_internal(L))
    IupOleControlOpen();
    
  iuplua_changeEnv(L);
  iupolecontrollua_open(L);
  iuplua_returnEnv(L);
  return 0;
}

/* obligatory to use require"iupluaole" */
int luaopen_iupluaole(lua_State* L)
{
  return iupolelua_open(L);
}

/* obligatory to use require"iupluaole51" */
int luaopen_iupluaole51(lua_State* L)
{
  return iupolelua_open(L);
}

]]
}

function ctrl.createElement(class, arg)
  local ctl = OleControl(arg[1])
   
  -- if luacom is loaded, use it to access methods and properties
  -- of the control
  if luacom then
    local punk = ctl.iunknown
    if punk then
      ctl.com = luacom.MakeLuaCOM(luacom.MakeIUnknown(punk))
    end     
  end
   
  return ctl
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
