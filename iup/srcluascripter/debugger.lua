local DEBUG_INACTIVE = 1 -- debug is inactive, hooks are not set
local DEBUG_ACTIVE = 2 -- debug should be active and running, until we found a breakpoint or the program ends
local DEBUG_STOPPED = 3 -- debug should be active and running, we are waiting to stop in the next opportunity, we have to abort
local DEBUG_STEP_INTO = 4 -- debug should be active and running, we are waiting until its steps into the function to pause, or we found a breakpoint
local DEBUG_STEP_OVER = 5 -- debug should be active and running, we are waiting until its steps over the function to pause, or we found a breakpoint
local DEBUG_STEP_OUT = 6 -- debug should be active and running, we are waiting until its steps out of the function to pause, or we found a breakpoint
local DEBUG_PAUSED = 7 -- debug should be active, but paused

local FUNC_INSIDE = 1
local FUNC_OUTSIDE = 2

debugger = {
  debug_state = DEBUG_INACTIVE,

  breakpoints = {},

  initialStackLevel = 2,

  currentFuncLevel = 0,
  stepFuncLevel = 0,
  stepFuncState = 0,
  startLevel = 0,

  currentFile = nil,
}

------------------------------------- User Interface State -------------------------------------

function debuggerReadFile(filename)
  local s = string.sub(filename, 1, 1)
  if s ~= "@" then
    return nil
  end

  local f = io.open(string.sub(filename, 2), "r")
  if f == nil then
    return nil
  end
  local t = f:read("*a")
  f:close()
  
  return t
end

function debuggerUpdateTitle(multitext, filename, is_dirty)
  local subtitle = multitext.subtitle
  local dirty_sign = ""

  if is_dirty then
    dirty_sign = "*"
  end

  if filename == nil then filename = "Untitled" end

  --iup.SetAttribute(multitext, "TITLE", string.format("%s%s - %s", str_filetitle(filename), dirty_sign, subtitle))
end

function debuggerReloadFile(filename)
-- TODO usar função pronta para isso
  local str = debuggerReadFile(filename)
  if (str) then
    local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
    
    local fname = string.sub(filename, 2)

    iup.SetAttribute(multitext, "FILENAME", fname)
    iup.SetAttribute(multitext, "DIRTY", "NO")
    iup.SetAttribute(multitext, "VALUE", str)

    debuggerUpdateTitle(multitext, fname, 0)
    
    debugger.currentFile = fname
    
    multitext.markerdeleteall = -1 -- all markers

    for i = 1, #debugger.breakpoints do
      local item = debugger.breakpoints[i]
      if item.filename == fname then
        main_dialog.ignore_toglebreakpoint = 1
        iup.SetAttributeId(main_dialog, "TOGGLEMARKER", item.line-1, 2)
        main_dialog.ignore_toglebreakpoint = nil
      end
    end
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
  local stop, step, pause, contin, curline

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
    contin = "NO"
    run = "NO"
    pause = "NO"
    curline = "NO"
  elseif st == DEBUG_ACTIVE or st == DEBUG_STEP_INTO or st == DEBUG_STEP_OVER or st == DEBUG_STEP_OUT then
    local btn_continue = iup.GetDialogChild(main_dialog, "BTN_CONTINUE")
    zbox.value = btn_continue

    stop = "YES"
    step = "NO"
    contin = "NO"
    run = "NO"
    pause = "YES"
    curline = "NO"

    if st == DEBUG_STEP_OUT then
      debugger.stepFuncLevel = debugger.currentFuncLevel
      debugger.stepFuncState = FUNC_INSIDE
    else
      debugger.stepFuncLevel = 0
      debugger.stepFuncState = FUNC_OUTSIDE
    end
  elseif st == DEBUG_PAUSED then
    local btn_continue = iup.GetDialogChild(main_dialog, "BTN_CONTINUE")
    zbox.value = btn_continue

    stop = "YES"
    step = "YES"
    contin = "YES"
    run = "NO"
    pause = "NO"
    curline = "YES"
  else -- st == DEBUG_INACTIVE
    local btn_debug = iup.GetDialogChild(main_dialog, "BTN_DEBUG")
    zbox.value = btn_debug

    stop = "NO"
    step = "NO"
    contin = "NO"
    run = "YES"
    pause = "NO"
    curline = "NO"

    multitext.readonly = "No"
    debuggerClearLocalVariablesList()
    debuggerClearStackList()
  end
    
  debugger.debug_state = st

  multitext.markerdeleteall = 2

  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_CONTINUE"), "ACTIVE", contin)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPOUT"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_CONTINUE"), "ACTIVE", contin)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPOUT"), "ACTIVE", step)
end                   

function debuggerHighlightLine(multitext, line)
   local pos = iup.TextConvertLinColToPos(multitext, line, 0)
   multitext.caretpos = pos
   multitext.markerdeleteall = 2
   multitext["markeradd"..line] = 2
end

function debuggerUpdateSourceLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  debuggerHighlightLine(multitext, currentline-1)
end


------------------------------------- Breakpoints -------------------------------------


function debuggerRemoveAllBreakpoints()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  multitext.markerdeleteall = 1
  
  debugger.breakpoints = {}
  
  debuggerUpdateBreakpointsList()
end

function debuggerToggleBreakpoint(line, mark, value)

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  if main_dialog.ignore_toglebreakpoint then
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
  if (index == 0) then
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
    debuggerUpdateLocalVarialesList(level)
  end
end

function debuggerUpdateLocalVarialesList(level)
  local name, value
  local pos = 1
  local val_key = 1

  debuggerClearLocalVariablesList()

  name, value = debug.getlocal(level+1, pos)
  while name ~= nil do
    if string.sub(name, 1, 1) ~= "(" then
      local local_list = iup.GetDialogChild(main_dialog, "LIST_LOCAL")
      iup.SetAttribute(local_list, val_key, name.." = "..debuggerGetObjectType(value))
      iup.SetAttribute(local_list, "POS"..val_key, pos)
      iup.SetAttribute(local_list, "LEVEL"..val_key, level+1)
      val_key = val_key + 1
    end
    pos = pos + 1
    name, value = debug.getlocal(level+1, pos)
  end

  if (val_key > 1) then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LOCAL"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "SET_LOCAL"), "ACTIVE", "Yes")
  end
end

------------------------------------- Stack -------------------------------------

function debuggerStackListAction(level)
  local info = debug.getinfo(level+4, "l") -- currentline
  
  debuggerUpdateSourceLine(info.currentline)
  
  debuggerUpdateLocalVarialesList(level+4)
end

function debuggerClearStackList()
  debuggerClearLocalVariablesList()

  iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_STACK"), "REMOVEITEM", "ALL")
end

function debuggerUpdateStackList()
  local info, name
  local level = 4
  
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
    iup.SetAttribute(list_stack, level-3, name)
    if info.what == "main" then
      break
    end
    level = level + 1
    info = debug.getinfo(level, "Sn") -- name, what
  end
  
  level = level-1
  
  if level > debugger.initialStackLevel then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "YES")
    list_stack.value = 1
    debuggerUpdateLocalVarialesList(4)
  end
  
end


----------------------------  Debug State       --------------------------


function debuggerGetDebugLevel()
  local level = -1
  repeat 
    level = level+1
  until debug.getinfo(level, "l") == nil  -- only current line, default is all info
  return level
end

function debuggerUpdateState(filename, line)
  if debugger.debug_state == DEBUG_STEP_OUT then
    if debugger.stepFuncState == FUNC_OUTSIDE then
      debuggerSetState(DEBUG_PAUSED)
      
      debugger.stepFuncState = FUNC_OUTSIDE
      debugger.stepFuncLevel = debugger.currentFuncLevel
    end
  elseif debugger.debug_state == DEBUG_STEP_INTO or
      (debugger.debug_state == DEBUG_STEP_OVER and debugger.stepFuncState == FUNC_OUTSIDE) or
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
      debuggerReloadFile(source)
    end
  
    debuggerUpdateSourceLine(currentline)
    
    debuggerUpdateStackList()
    
    debug.sethook() -- turns off the hook
    
    while debugger.debug_state == DEBUG_PAUSED do
      iup.LoopStep()
    end
    
    debug.sethook(debuggerHookFunction, "lcr") -- restore the hook
  end
  
  if debugger.debug_state == DEBUG_STOPPED then
    debuggerSetState(DEBUG_INACTIVE)
    debug.sethook() -- turns off the hook
    error("-- Debug stop") -- abort processing
  end
end

function debuggerCallHook()
  if debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == 0 then
      local level = debuggerGetDebugLevel()
      debugger.stepFuncState = FUNC_INSIDE
      debugger.stepFuncLevel = level
    end
  end
end

function debuggerReturnHook(what)
  local level = debuggerGetDebugLevel()

  if level == debugger.startLevel+1 and what == "main" then
    debuggerSetState(DEBUG_INACTIVE)
  elseif debugger.debug_state == DEBUG_STEP_OUT or debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == level then
      debugger.stepFuncState = FUNC_OUTSIDE
      debugger.stepFuncLevel = 0
    end
  end
end

function debuggerHookFunction(event)
  local info = debug.getinfo(debugger.initialStackLevel, "Sl") -- what, source, currentline
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
      debuggerLineHook(info.source, info.currentline)
    end
  end

end

function debuggerStartDebug(filename)
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  debugger.currentFile = multitext.filename
  debugger.startLevel = debuggerGetDebugLevel()
  
  print("-- Debug start")
  debuggerSetState(DEBUG_ACTIVE)

  debug.sethook(debuggerHookFunction, "lcr")

  local ok, msg = pcall(dofile, debugger.currentFile)
  
  debuggerSetState(DEBUG_INACTIVE)
  print("-- Debug finish")
  
  if not ok then
    print(msg)
  end
end

function debuggerRun()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  iup.dostring(multitext.value) 
end


---- TODO: ---- 
-- debuggerReloadFile, quando terminar restaura original!!!
-- debug string????
