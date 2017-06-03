local DEBUG_INACTIVE = 1 -- debug is inactive, hooks are not set
local DEBUG_ACTIVE = 2 -- debug should be active and running, until we found a breakpoint or the program ends
local DEBUG_STOPPED = 3 -- debug should be active and running, we are waiting to stop in the next opportunity, we have to abort
local DEBUG_STEP_INTO = 4 -- debug should be active and running, we are waiting until its steps into the function to pause, or we found a breakpoint
local DEBUG_STEP_OVER = 5 -- debug should be active and running, we are waiting until its steps over the function to pause, or we found a breakpoint
local DEBUG_STEP_OUT = 6 -- debug should be active and running, we are waiting until its steps out of the function to pause, or we found a breakpoint
local DEBUG_PAUSED = 7 -- debug should be active, but paused

local FUNC_STATE_INSIDE = 1
local FUNC_STATE_OUTSIDE = 2

debugger = {
  debug_state = DEBUG_INACTIVE,

  breakpoints = {},

  currentFuncLevel = 0,
  stepFuncLevel = 0,
  stepFuncState = 0,
  startLevel = 0,

  currentFile = nil,
  restore_value = false,
}

------------------------------------- User Interface State -------------------------------------

function debuggerReadFile(filename)
  local f = io.open(filename, "r")
  if f == nil then
    return nil
  end
  local t = f:read("*a")
  f:close()
  return t
end

function debuggerReloadFile(filename)
  local str = debuggerReadFile(filename)
  if (str) then
    local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
    
    iup.SetAttribute(multitext, "VALUE", str)
    debugger.restore_value = true

    debugger.currentFile = filename
    
    multitext.markerdeleteall = -1 -- all markers

    for i = 1, #debugger.breakpoints do
      local item = debugger.breakpoints[i]
      if item.filename == filename then
        main_dialog.ignore_toglebreakpoint = 1
        iup.SetAttributeId(main_dialog, "TOGGLEMARKER", item.line-1, 2)
        main_dialog.ignore_toglebreakpoint = nil
      end
    end
  else
    iup.Message("Warning!", "Failed to read file:\n  "..filename)
    debuggerEndDebug(true)
  end
end

function debuggerSetStateString(state)
  local map_state = {
    DEBUG_INACTIVE = DEBUG_INACTIVE,
    DEBUG_ACTIVE = DEBUG_ACTIVE,
    DEBUG_STOPPED = DEBUG_STOPPED,
    DEBUG_STEP_INTO = DEBUG_STEP_INTO,
    DEBUG_STEP_OVER = DEBUG_STEP_OVER,
    DEBUG_STEP_OUT = DEBUG_STEP_OUT,
    DEBUG_PAUSED = DEBUG_PAUSED,
  }

  debuggerSetState(map_state[state])
end

function debuggerSetState(st)
  local stop, step, pause, cont, run, dbg

  if debugger.debug_state == st then
    return
  end

  local zbox = iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE")
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  multitext.readonly = "Yes"

  if st == DEBUG_STOPPED then
    local btn_debug = iup.GetDialogChild(main_dialog, "BTN_DEBUG")
    zbox.value = btn_debug

    stop = "NO"
    step = "NO"
    cont = "NO"
    run = "NO"
    pause = "NO"
    dbg = "NO"
  elseif st == DEBUG_ACTIVE or st == DEBUG_STEP_INTO or st == DEBUG_STEP_OVER or st == DEBUG_STEP_OUT then
    local btn_continue = iup.GetDialogChild(main_dialog, "BTN_CONTINUE")
    zbox.value = btn_continue

    stop = "YES"
    step = "NO"
    cont = "NO"
    run = "NO"
    pause = "YES"
    dbg = "NO"

    if st == DEBUG_STEP_OUT then
      debugger.stepFuncLevel = debugger.currentFuncLevel
      debugger.stepFuncState = FUNC_STATE_INSIDE
    else
      debugger.stepFuncLevel = 0
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
    end
  elseif st == DEBUG_PAUSED then
    local btn_continue = iup.GetDialogChild(main_dialog, "BTN_CONTINUE")
    zbox.value = btn_continue

    stop = "YES"
    step = "YES"
    cont = "YES"
    run = "NO"
    pause = "NO"
    dbg = "NO"
  else -- st == DEBUG_INACTIVE
    local btn_debug = iup.GetDialogChild(main_dialog, "BTN_DEBUG")
    zbox.value = btn_debug

    stop = "NO"
    step = "NO"
    cont = "NO"
    run = "YES"
    pause = "NO"
    dbg = "YES"

    multitext.readonly = "No"
    debuggerClearLocalVariablesList()
    debuggerClearStackList()
  end
    
  debugger.debug_state = st

  multitext.markerdeleteall = 2

  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_RUN"), "ACTIVE", run)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_CONTINUE"), "ACTIVE", cont)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPOUT"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_RUN"), "ACTIVE", run)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_CONTINUE"), "ACTIVE", cont)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPOUT"), "ACTIVE", step)
end                   

function debuggerHighlightLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  local pos = iup.TextConvertLinColToPos(multitext, currentline-1, 0) -- line here starts at 0
  multitext.caretpos = pos
  multitext.markerdeleteall = 2
  multitext["markeradd"..currentline-1] = 2
end

function debuggerSelectLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  local pos = iup.TextConvertLinColToPos(multitext, currentline-1, 0) -- line here starts at 0
  multitext.caretpos = pos
  multitext.selection = currentline-1 .. ",0:" .. currentline-1 .. ",9999"
end


------------------------------------- Breakpoints -------------------------------------


function debuggerRemoveAllBreakpoints()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  multitext.markerdeleteall = 1
  
  debugger.breakpoints = {}
  
  debuggerUpdateBreakpointsList()
end

function debuggerToggleBreakpoint(line, marker, value)

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  if main_dialog.ignore_toglebreakpoint then
    return
  end
  if not multitext.filename then
    iup.SetAttributeId(multitext, "MARKERDELETE", line - 1, marker)
    iup.Message("Warning!", "Must have a filename to add a breakpoint.")
    return
  end

  if value > 0 then
    debuggerInsertBreakpoint(multitext.filename, line)
  else
    debuggerRemoveBreakpoint(multitext.filename, line)
  end
  
  debuggerUpdateBreakpointsList()
end

function debuggerUpdateBreakpointsList()
  local list_break = iup.GetDialogChild(main_dialog, "LIST_BREAK")
  iup.SetAttribute(list_break, "REMOVEITEM", "ALL")

  for i = 1, #debugger.breakpoints do
    local item = debugger.breakpoints[i]
    iup.SetAttribute(list_break, i, "Line "..item.line.." of "..item.filename)
  end
end

function debuggerInsertBreakpoint(filename, line)
  local item = {}
  item.filename = filename
  item.line = line
  table.insert(debugger.breakpoints, item)
end

function debuggerRemoveBreakpointFromList(index)
  local item = debugger.breakpoints[index]
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  if multitext.filename == item.filename then
    iup.SetAttributeId(multitext, "TOGGLEMARKER", item.line, 2)
  else
    debuggerRemoveBreakpoint(item.filename, item.line)
  end
  
  debuggerUpdateBreakpointsList()
end

function debuggerRemoveBreakpoint(filename, line)
  for i = 1, #debugger.breakpoints do
    local item = debugger.breakpoints[i]
    if item.filename == filename and item.line == line then
      table.remove(debugger.breakpoints, i)
      break
    end
  end
    
end

function debuggerHasLineBreak(filename, line)
  for i = 1, #debugger.breakpoints do
    local item = debugger.breakpoints[i]
    if item.filename == filename and item.line == line then
      return true
    end
  end
    
  return false
end

------------------------------------- Locals -------------------------------------

function debuggerClearLocalVariablesList()
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_ALLLOCALS"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LOCAL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "SET_LOCAL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "REMOVEITEM", "ALL")
end

function debuggerGetObjectType(value)
  local valueType = type(value)
  if valueType == "string" or valueType == "number" then
    return value
  elseif valueType == "table" then
    return "<table>"
  elseif valueType == "function" then
    return "<function>"
  elseif valueType == "userdata" then
    return "<userdata>"
  elseif valueType == "nil" then
    return "<nil>"
  end
  return "<unknown>"
end

function debuggerSetLocalVariable()
  local local_list = iup.GetDialogChild(main_dialog, "LIST_LOCAL")
  local index = local_list.value
  if (not index or tonumber(index) == 0) then
    iup.Message("Warning!", "Select a variable on the list.")
    return
  end

  local level = iup.GetAttribute(local_list, "LEVEL"..index)
  local pos = iup.GetAttribute(local_list, "POS"..index)
  
  local name, value = debug.getlocal(level, pos)
  if (value == nil) then value = "" end
  local valueType = type(value)
  if valueType ~= "string" and valueType ~= "number" then
    iup.Message("Warning!", "Can edit only strings and numbers.")
    return
  end

  local status, newValue = iup.GetParam("Set Local", nil, name.." = ".."%s\n", tostring(value))

  if (status) then
    debug.setlocal(level, pos, newValue)
    iup.SetAttribute(local_list, index, name.." = "..newValue)
  end
end

function debuggerPrintLocalVariable()
  local local_list = iup.GetDialogChild(main_dialog, "LIST_LOCAL")
  local index = local_list.value

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  local level = iup.GetAttribute(local_list, "LEVEL"..index)
  local pos = iup.GetAttribute(local_list, "POS"..index)

  print(local_list[index] .. "  (level="..level..", pos="..pos..")")
end

function debuggerPrintAllLocalVariables()
  local local_list = iup.GetDialogChild(main_dialog, "LIST_LOCAL")
  local count = local_list.count

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local level = iup.GetAttribute(local_list, "LEVEL"..index)
    local pos = iup.GetAttribute(local_list, "POS"..index)

    print(local_list[index] .. "  (level="..level..", pos="..pos..")")
  end
end

function debuggerUpdateLocalVariablesList(level, actual_level)
  local name, value
  local pos = 1
  local index = 1

  debuggerClearLocalVariablesList()

  local local_list = iup.GetDialogChild(main_dialog, "LIST_LOCAL")

  name, value = debug.getlocal(level, pos)
  while name ~= nil do
    if string.sub(name, 1, 1) ~= "(" then
      iup.SetAttribute(local_list, index, name.." = "..debuggerGetObjectType(value))
      iup.SetAttribute(local_list, "POS"..index, pos)
      iup.SetAttribute(local_list, "LEVEL"..index, actual_level)
      index = index + 1
    end
    pos = pos + 1
    name, value = debug.getlocal(level, pos)
  end

  if (index > 1) then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LOCAL"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_ALLLOCALS"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "SET_LOCAL"), "ACTIVE", "Yes")

    local_list.value = 1 -- select first item on list
  end
end

------------------------------------- Stack -------------------------------------

function debuggerStackListAction(index)
  local level = index + debugger.startLevel - 1  -- this is level of the function
  level = level + 1 -- must fix the level because we called a Lua function
  local info = debug.getinfo(level, "Sl") -- source, currentline
  
  local filename = string.sub(info.source, 2)
  if debugger.currentFile == filename then
    debuggerSelectLine(info.currentline)
  end
  
  debuggerUpdateLocalVariablesList(level + 1, level) -- this is the level of the local variables inside that function
                                                     -- the actual level does not includes the fix
end

function debuggerClearStackList()
  debuggerClearLocalVariablesList()

  iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_STACK"), "REMOVEITEM", "ALL")
end

function debuggerPrintStackLevel()
  local list_stack = iup.GetDialogChild(main_dialog, "LIST_STACK")
  local index = list_stack.value

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  local level = index + debugger.startLevel - 1

  print(list_stack[index] .. "  (level="..level..")")
end

function debuggerPrintStack()
  local list_stack = iup.GetDialogChild(main_dialog, "LIST_STACK")
  local count = list_stack.count

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local level = index + debugger.startLevel - 1
    print(list_stack[index] .. "  (level="..level..")")
  end
end

function debuggerUpdateStackList()
  local info, name
  local level = debugger.startLevel
  
  debuggerClearStackList()

  local list_stack = iup.GetDialogChild(main_dialog, "LIST_STACK")
  
  info = debug.getinfo(level, "Sn") -- name, what
  while  info ~= nil do
    if info.what == "main" then
      name = "<main>"
    elseif info.name and info.name ~= "" then
      name = info.name
    else
      name = "<noname>"
    end
    local index = level - debugger.startLevel + 1
    iup.SetAttribute(list_stack, index, name)

    level = level + 1
    if info.what == "main" then
      break
    end

    info = debug.getinfo(level, "Sn") -- name, what
  end
  
  if level > debugger.startLevel then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "YES")

    list_stack.value = 1 -- select first item on list
    debuggerUpdateLocalVariablesList(debugger.startLevel + 1, debugger.startLevel + 1) -- this is the level of the local variables inside that function
  end
  
end


----------------------------  Debug State       --------------------------

function debuggerEndDebug(stop)
  debug.sethook() -- turns off the hook

  debuggerSetState(DEBUG_INACTIVE)

  if (debugger.restore_value) then 
    local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
    debuggerReloadFile(multitext.filename) 
    debugger.restore_value = false
  end

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  if stop then
    error("-- Debug stop\n") -- abort processing
  else
    print("-- Debug finish\n")
  end
end

function debuggerGetDebugLevel()
  local level = -1
  repeat 
    level = level+1
  until debug.getinfo(level, "l") == nil  -- only current line, default is all info
  return level
end

function debuggerUpdateState(filename, line)
  if debugger.debug_state == DEBUG_STEP_OUT then
    if debugger.stepFuncState == FUNC_STATE_OUTSIDE then
      debuggerSetState(DEBUG_PAUSED)
      
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = debugger.currentFuncLevel
    end
  elseif debugger.debug_state == DEBUG_STEP_INTO or
      (debugger.debug_state == DEBUG_STEP_OVER and debugger.stepFuncState == FUNC_STATE_OUTSIDE) or
      (debugger.debug_state ~= DEBUG_PAUSED and debuggerHasLineBreak(filename, line)) then
      debuggerSetState(DEBUG_PAUSED)
  end
end

function debuggerLineHook(source, currentline)
  debugger.currentFuncLevel = debuggerGetDebugLevel()

  local filename = string.sub(source, 2)

  debuggerUpdateState(filename, currentline)
  
  if debugger.debug_state == DEBUG_PAUSED then
  
    if debugger.currentFile ~= filename then
      local s = string.sub(source, 1, 1)
      if s == "@" then
        debuggerReloadFile(filename)
      end
    end
  
    debuggerHighlightLine(currentline)
    
    debuggerUpdateStackList()
    
    debug.sethook() -- turns off the hook
    
    while debugger.debug_state == DEBUG_PAUSED do
      iup.LoopStep()
    end
    
    debug.sethook(debuggerHookFunction, "lcr") -- restore the hook
  end
  
  if debugger.debug_state == DEBUG_STOPPED then
    debuggerEndDebug(true)
  end
end

function debuggerCallHook()
  if debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == 0 then
      local level = debuggerGetDebugLevel()
      debugger.stepFuncState = FUNC_STATE_INSIDE
      debugger.stepFuncLevel = level
    end
  end
end

function debuggerReturnHook(what)
  local level = debuggerGetDebugLevel()

  print_old("debuggerReturnHook(level="..level..", what="..what..")")

  --TODO if level == debugger.startLevel+1 and what == "main" then  ???
  if what == "main" then
    debuggerSetState(DEBUG_INACTIVE)
  elseif debugger.debug_state == DEBUG_STEP_OUT or debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == level then
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = 0
    end
  end
end

function debuggerHookFunction(event, currentline)
  -- Inside a hook, you can call getinfo with level 2 to get more information about the running function
  local info = debug.getinfo(2, "S") -- what, source
  local s = string.sub(info.source, 1, 1)
  if s ~= "@" then
    return
  end

  if debugger.debug_state ~= DEBUG_INACTIVE then
    if event == "call" then
      debuggerCallHook()
    elseif event == "return" then
      debuggerReturnHook(info.what)
    elseif event == "line" then
      debuggerLineHook(info.source, currentline)
    end
  end

end

function debuggerStartDebug(filename)
  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  debugger.currentFile = multitext.filename
  debugger.startLevel = debuggerGetDebugLevel() + 1 -- usually 3+1=4
  
  print("-- Debug start\n")
  debuggerSetState(DEBUG_ACTIVE)
  debugtabs.valuepos = 1

  debug.sethook(debuggerHookFunction, "lcr")

  local ok, msg = pcall(dofile, debugger.currentFile)
  if not ok then
    print(msg)
  end
  
  debuggerEndDebug(false)
end

function debuggerRun()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  iup.dostring(multitext.value) 
end
