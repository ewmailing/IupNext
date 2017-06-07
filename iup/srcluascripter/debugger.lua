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

  currentFuncLevel = 0,
  stepFuncLevel = 0,
  stepFuncState = 0,
  startLevel = 0,

  currentFile = nil,
  currentLine = nil,
}

------------------------------------- User Interface State -------------------------------------

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
  local stop, step, pause, cont, run, dbg, curline

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
    curline = "NO"
  elseif st == DEBUG_ACTIVE or st == DEBUG_STEP_INTO or st == DEBUG_STEP_OVER or st == DEBUG_STEP_OUT then
    local btn_continue = iup.GetDialogChild(main_dialog, "BTN_CONTINUE")
    zbox.value = btn_continue

    stop = "YES"
    step = "NO"
    cont = "NO"
    run = "NO"
    pause = "YES"
    dbg = "NO"
    curline = "NO"

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
    curline = "Yes"
  else -- st == DEBUG_INACTIVE
    local btn_debug = iup.GetDialogChild(main_dialog, "BTN_DEBUG")
    zbox.value = btn_debug

    stop = "NO"
    step = "NO"
    cont = "NO"
    run = "YES"
    pause = "NO"
    dbg = "YES"
    curline = "NO"

    multitext.readonly = "No"
    debuggerClearLocalVariablesList()
    debuggerClearStackList()
  end
    
  debugger.debug_state = st

  multitext.markerdeleteall = 2 -- current line highlight
  multitext.markerdeleteall = 3 -- current line arrow

  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_RUN"), "ACTIVE", run)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_CONTINUE"), "ACTIVE", cont)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_STEPOUT"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "ITM_CURRENTLINE"), "ACTIVE", curline)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_RUN"), "ACTIVE", run)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_CONTINUE"), "ACTIVE", cont)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_STEPOUT"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_CURRENTLINE"), "ACTIVE", curline)
end                   

function debuggerHighlightLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  debugger.currentLine = currentline

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  local pos = iup.TextConvertLinColToPos(multitext, currentline-1, 0) -- line here starts at 0
  multitext.caretpos = pos
  multitext.markerdeleteall = 2
  multitext["markeradd"..currentline-1] = 2
  multitext.markerdeleteall = 3
  multitext["markeradd"..currentline-1] = 3
end

function debuggerShowCurrentLine()
  if debugger.currentLine then
    local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
    local pos = iup.TextConvertLinColToPos(multitext, debugger.currentLine-1, 0) -- line here starts at 0
    multitext.caretpos = pos
  end
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

function debuggerNewBreakpoint()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  local fname = multitext.filename
  if (not fname) then fname = "" end
  local status, filename, line = iup.GetParam("New Breakpoint", nil, "Filename: %s\nLine: %i\n", fname, 1)

  if (status) then
    iup.SetAttributeId(multitext, "MARKERADD", line - 1, 1)-- in user interface line starts at 1, in Scintilla starts at 0
    debuggerUpdateBreakpointsList()
  end
end

function debuggerRemoveAllBreakpoints()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
  multitext.markerdeleteall = 1
  
  debuggerUpdateBreakpointsList()
end

function debuggerUpdateBreakpointsList()
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  local list_break = iup.GetDialogChild(main_dialog, "LIST_BREAK")
  iup.SetAttribute(list_break, "REMOVEITEM", "ALL")

  local breakpoints = debuggerGetBreakpoints(multitext)
  local filename = multitext.filename
  -- TODO add for all open files

  for index, line in pairs(breakpoints) do
    iup.SetAttribute(list_break, index, "Line "..line.." of "..filename)
    iup.SetAttribute(list_break, "LINE"..index, line)
    iup.SetAttribute(list_break, "FILENAME"..index, filename)
  end
end

function debuggerBreaksListAction(index)
  local list_break = iup.GetDialogChild(main_dialog, "LIST_BREAK")
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")

  local line = iup.GetAttribute(list_break, "LINE"..index)
  local filename = iup.GetAttribute(list_break, "FILENAME"..index)
  
  if multitext.filename == filename then
    debuggerSelectLine(tonumber(line))
  end
end

function debuggerRemoveBreakpoint(index)
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  local list_break = iup.GetDialogChild(main_dialog, "LIST_BREAK")

  local line = iup.GetAttribute(list_break, "LINE"..index)
  local filename = iup.GetAttribute(list_break, "FILENAME"..index)
  
  if multitext.filename == filename then
    iup.SetAttributeId(main_dialog, "TOGGLEMARKER", line - 1, 2)
  else
    -- TODO update other filenames ???
    debuggerUpdateBreakpointsList()
  end
end

function debuggerHasLineBreak(filename, currentline)
  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  return debuggerHasBreakpoint(multitext, currentline - 1)
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
  local defined = iup.GetAttribute(list_stack, "DEFINED"..index)

  print(list_stack[index] .. "  (level="..level..")")
  if defined then
    print("    [" .. defined .."]")
  end
end

function debuggerPrintStack()
  local list_stack = iup.GetDialogChild(main_dialog, "LIST_STACK")
  local count = list_stack.count

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local level = index + debugger.startLevel - 1
    local defined = iup.GetAttribute(list_stack, "DEFINED"..index)

    print(list_stack[index] .. "  (level="..level..")")
    if defined then
      print("    [" .. defined .."]")
    end
  end
end

function debuggerUpdateStackList()
  local info, name, defined
  local level = debugger.startLevel
  
  debuggerClearStackList()

  local list_stack = iup.GetDialogChild(main_dialog, "LIST_STACK")
  
  info = debug.getinfo(level, "Snl") -- name, what, currentline
  while  info ~= nil do
    if info.what == "main" then
      name = "<main>"
    elseif info.name and info.name ~= "" then
      name = info.name

      if info.what == "C" then
        name = name .. " (C)"
      end
    else
      name = "<noname>"
    end
    if info.namewhat == "l" then
        name = name .. " Local"
    end
    if info.currentline > 0 then
       name = name .. ", at line " .. info.currentline
    end

    defined = nil
    if info.what ~= "C" then    
      local s = string.sub(info.source, 1, 1)
      if s == "@" then
        local filename = string.sub(info.source, 2)
        defined = "Defined in the file: " .. filename
      else
        defined = "Defined in a string."
      end
    end

    local index = level - debugger.startLevel + 1
    iup.SetAttribute(list_stack, index, name)
    iup.SetAttribute(list_stack, "DEFINED"..index, defined)

    level = level + 1
    if info.what == "main" then
      break
    end

    info = debug.getinfo(level, "Snl") -- name, what, currentline
  end
  
  if level > debugger.startLevel then
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "YES")

    list_stack.value = 1 -- select first item on list
    debuggerUpdateLocalVariablesList(debugger.startLevel + 1, debugger.startLevel + 1) -- this is the level of the local variables inside that function
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

function debuggerUpdateState(filename, currentline)
  if debugger.debug_state == DEBUG_STEP_OUT then
    if debugger.stepFuncState == FUNC_STATE_OUTSIDE then
      debuggerSetState(DEBUG_PAUSED)
      
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = debugger.currentFuncLevel
    end
  elseif debugger.debug_state == DEBUG_STEP_INTO or
      (debugger.debug_state == DEBUG_STEP_OVER and debugger.stepFuncState == FUNC_STATE_OUTSIDE) or
      (debugger.debug_state ~= DEBUG_PAUSED and debuggerHasLineBreak(filename, currentline)) then
      debuggerSetState(DEBUG_PAUSED)
  end
end

function debuggerLineHook(filename, currentline)
  debugger.currentFuncLevel = debuggerGetDebugLevel()

  debuggerUpdateState(filename, currentline)
  
  if debugger.debug_state == DEBUG_PAUSED then
  
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
    return         -- TODO for now, ignore strings
  end
  local filename = string.sub(info.source, 2)
  if debugger.currentFile ~= filename then
    return         -- TODO for now, ignore other files
  end

  if debugger.debug_state ~= DEBUG_INACTIVE then
    if event == "call" then
      debuggerCallHook()
    elseif event == "return" then
      debuggerReturnHook(info.what)
    elseif event == "line" then
      debuggerLineHook(filename, currentline)
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
end

function debuggerEndDebug(stop)
  debug.sethook() -- turns off the hook

  debuggerSetState(DEBUG_INACTIVE)

  local debugtabs = iup.GetDialogChild(main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  if stop then
    print("-- Debug stop\n")
    error() -- abort processing, no error message
  else
    print("-- Debug finish\n")
  end
end

function debuggerExit()
  if debugger.debug_state ~= DEBUG_INACTIVE then
    debuggerEndDebug(true)
  end
end
