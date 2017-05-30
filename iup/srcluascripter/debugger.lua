local DEBUG_INACTIVE = 1
local DEBUG_ACTIVE = 2
local DEBUG_STOPPED = 3
local DEBUG_STEP_INTO = 4
local DEBUG_STEP_OVER = 5
local DEBUG_STEP_OUT = 6
local DEBUG_PAUSED = 7

local FUNC_INSIDE = 1
local FUNC_OUTSIDE = 2

debugger = {
  debug_state = DEBUG_INACTIVE,

  breakpoints = {},

  initialStackLevel = 2,

  currentFuncLevel = 0,
  stepFuncLevel = 0,
  stepFuncState = 0,
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

  local subtitle = iup.GetAttribute(multitext, "SUBTITLE")
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
    
    currentFile = fname
    
    iup.SetAttribute(multitext, "MARKERDELETEALL", -1)

    for i = 1, #debugger.breakpoints do
      local item = debugger.breakpoints[i]
      if item.filename == fname then
        iup.SetAttribute(main_dialog, "DONTTOGGLE", 1)
        iup.SetAttributeId(main_dialog, "TOGGLEMARKER", item.line-1, 2)
        iup.SetAttribute(main_dialog, "DONTTOGGLE", 0)
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

  if st == DEBUG_STOPPED then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE"), "VALUE", "BTN_DEBUG")
    stop = "NO"
    step = "NO"
    contin = "NO"
    run = "NO"
    pause = "NO"
    curline = "NO"
  elseif st == DEBUG_INACTIVE then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE"), "VALUE", "BTN_DEBUG")
    stop = "NO"
    step = "NO"
    contin = "NO"
    run = "YES"
    pause = "NO"
    curline = "NO"
  elseif st == DEBUG_ACTIVE or st == DEBUG_STEP_INTO or st == DEBUG_STEP_OVER or st == DEBUG_STEP_OUT then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE"), "VALUE", "BTN_CONTINUE")
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
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE"), "VALUE", "BTN_CONTINUE")
    stop = "YES"
    step = "YES"
    contin = "YES"
    run = "NO"
    pause = "NO"
    curline = "YES"
  end
    
  debugger.debug_state = st

  iup.SetAttribute(iup.GetDialogChild(main_dialog, "MULTITEXT"), "MARKERDELETEALL", 2)

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
   iup.SetAttribute(multitext, "CARETPOS", pos)
   iup.SetAttribute(multitext, "MARKERDELETEALL", 2)
   iup.SetAttributeId(multitext, "MARKERADD", line, 2)
end

function debuggerUpdateSourceLine(info)

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  if info.currentline == nil or info.currentline <= 0 then
    return
  end

  iup.SetAttribute(multitext, "CARET", string.format("%d,7", info.currentline))

  debuggerHighlightLine(multitext, info.currentline-1)

end


------------------------------------- Breakpoints -------------------------------------


function debuggerRemoveAllBreakpoints()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  iup.SetAttribute(multitext, "MARKERDELETEALL", 1)
  
  debugger.breakpoints = {}
  
  debuggerUpdateBreakpointsList()
end

function debuggerToggleBreakpoint(line, mark, value)

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  if iup.GetAttribute(main_dialog, "DONTTOGGLE") then
    return
  end

  if value > 0 then
    debuggerInsertBreakpoint(iup.GetAttribute(multitext, "FILENAME"), line)
  else
    debuggerRemoveBreakpoint(iup.GetAttribute(multitext, "FILENAME"), line)
  end
  
  debuggerUpdateBreakpointsList()

end

function debuggerUpdateBreakpointsList()
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_BREAK"), "REMOVEITEM", "ALL")

  for i = 1, #debugger.breakpoints do
    local item = debugger.breakpoints[i]
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_BREAK"), i, "Line "..item.line.." of "..item.filename)
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
  
  if iup.GetAttribute(iup.GetDialogChild(main_dialog, "MULTITEXT"), "FILENAME") == item.filename then
    iup.SetAttributeId(iup.GetDialogChild(main_dialog, "MULTITEXT"), "TOGGLEMARKER", item.line, 2)
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

function debuggerGetObjectType(name)
  local nameType = type(name)
  if nameType == "string" or nameType == "number" then
    return name
  elseif nameType == "table" then
    return "<table>"
  elseif nameType == "function" then
    return "<function>"
  elseif nameType == "userdata" then
    return "<userdata>"
  elseif nameType == "nil" then
    return "<nil>"
  end
  return "<unknown>"
end

function debuggerSetLocalVariable(index)

  local level = iup.GetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "LEVEL"..index)
  local pos = iup.GetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "POS"..index)
  
  local name, value = debug.getlocal(level, pos)
  
  local status, newValue = iup.GetParam("Set Local", nil, name.." = ".."%s\n", value)

  if (status == 1) then
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
      iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), val_key, name.." = "..debuggerGetObjectType(value))
      iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "POS"..val_key, pos)
      iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "LEVEL"..val_key, level+1)
      val_key = val_key + 1
    end
    pos = pos + 1
    name, value = debug.getlocal(level+1, pos)
  end
end

------------------------------------- Stack -------------------------------------

function debuggerStackListAction(level)
  local info = debug.getinfo(level+4, "Snl")
  
  debuggerUpdateSourceLine(info)
  
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
  
  info = debug.getinfo(level, "Sn")
  while  info ~= nil do
    if info.what == "main" then
      name = "<main>"
    elseif info.name and info.name ~= "" then
      name = info.name
    else
      name = "<noname>"
    end
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_STACK"), level-3, name)
    if info.what == "main" then
      break
    end
    level = level + 1
    info = debug.getinfo(level, "Sn")
  end
  
  level = level-1
  
  if level> debugger.initialStackLevel then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_STACK"), "VALUE", "1")
    debuggerUpdateLocalVarialesList(4)
  end
  
end


----------------------------  Debug State       --------------------------


function debuggerGetDebugLevel(offset)

  local level = -1
  
  repeat 
    level = level+1
  until debug.getinfo(level, "Snl") == nil
  
  return level - offset
end

function debuggerUpdateState(filename, line)

  if debugger.debug_state == DEBUG_STEP_OUT then
    if debugger.stepFuncState == FUNC_OUTSIDE then
      local level = debuggerGetDebugLevel(0)
      
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

function debuggerHookFunction(event, line)
   
  local info = debug.getinfo(debugger.initialStackLevel, "Snl")
  
    local s = string.sub(info.source, 1, 1)
    if s ~= "@" then
      return
    end
  
  if debugger.debug_state ~= DEBUG_INACTIVE then
    if event == "call" then
      debuggerCallHook(info)
    elseif event == "return" then
      debuggerReturnHook(info)
    elseif event == "line" then
      debuggerLineHook(info, line)
    end
  end

end

function debuggerLineHook(info, line)

  debugger.currentFuncLevel = debuggerGetDebugLevel(0)

  debuggerUpdateState(string.sub(info.source, 2), line)
  
  if debugger.debug_state == DEBUG_PAUSED then
  
    if currentFile ~= string.sub(info.source, 2) then
      debuggerReloadFile(info.source)
    end
  
    debuggerUpdateSourceLine(info)
    
    debuggerUpdateStackList()
    
    debug.sethook()
    
    while debugger.debug_state == DEBUG_PAUSED do
      iup.LoopStep()
    end
    
    debug.sethook(debuggerHookFunction, "lcr")
    
  elseif iup.LoopStep() == IUP_CLOSE then
    debuggerSetState(DEBUG_STOPPED)
  end
  
  if debugger.debug_state == DEBUG_STOPPED then
    debuggerSetState(DEBUG_ACTIVE)
    debug.sethook()
    error("debug: Last command interrupted!", 0)
  end
end

function debuggerCallHook(info)

  local level = debuggerGetDebugLevel(0)

  if debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == 0 then
      debugger.stepFuncState = FUNC_INSIDE
      debugger.stepFuncLevel = level
    end
  end
  
end

function debuggerReturnHook(info)

  local level = debuggerGetDebugLevel(0)

  if level == startLevel+1 and info.what == "main" then
    debuggerSetState(DEBUG_INACTIVE)
  elseif debugger.debug_state == DEBUG_STEP_OUT or debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == level then
      debugger.stepFuncState = FUNC_OUTSIDE
      debugger.stepFuncLevel = 0
    end
  end
  
end

function debuggerStartDebug(filename, runMode)

  currentFile = filename
  
  startLevel = debuggerGetDebugLevel(0)
  
  if runMode == false or runMode == nil then
    debug.sethook(debuggerHookFunction, "lcr")
  end
  
  local ok, msg = pcall(dofile, currentFile)
  
  debuggerSetState(DEBUG_INACTIVE)
  
  if not ok then
    print(msg)
  end
end

-- TODO run
-- TODO error handler - lua_call
-- TODO readonly
-- TODO save before run
