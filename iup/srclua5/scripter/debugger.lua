local DEBUG_INACTIVE = 1 -- debug is inactive, hooks are not set
local DEBUG_ACTIVE = 2 -- debug should be active and running, until we found a breakpoint or the program ends
local DEBUG_STOPPED = 3 -- debug should be active and running, we are waiting to stop in the next opportunity, we have to abort
local DEBUG_STEP_INTO = 4 -- debug should be active and running, we are waiting until its steps into the function to pause, or we found a breakpoint
local DEBUG_STEP_OVER = 5 -- debug should be active and running, we are waiting until its steps over the function to pause, or we found a breakpoint
local DEBUG_STEP_OUT = 6 -- debug should be active and running, we are waiting until its steps out of the function to pause, or we found a breakpoint
local DEBUG_PAUSED = 7 -- debug should be active, but paused

local FUNC_STATE_INSIDE = 1
local FUNC_STATE_OUTSIDE = 2

local debugger = {
  debug_state = DEBUG_INACTIVE,

  currentFuncLevel = 0,
  stepFuncLevel = 0,
  stepFuncState = 0,
  startLevel = 0,

  currentFile = nil,
  currentLine = nil,

  main_dialog = nil,
}

function iupDebuggerInit(main_dialog)
  debugger.main_dialog = main_dialog
end


------------------------------------- User Interface State -------------------------------------

function iupDebuggerSetStateString(state)
  local map_state = {
    DEBUG_INACTIVE = DEBUG_INACTIVE,
    DEBUG_ACTIVE = DEBUG_ACTIVE,
    DEBUG_STOPPED = DEBUG_STOPPED,
    DEBUG_STEP_INTO = DEBUG_STEP_INTO,
    DEBUG_STEP_OVER = DEBUG_STEP_OVER,
    DEBUG_STEP_OUT = DEBUG_STEP_OUT,
    DEBUG_PAUSED = DEBUG_PAUSED,
  }

  iupDebuggerSetState(map_state[state])
end

function iupDebuggerSetState(st)
  local stop, step, pause, cont, run, dbg, curline

  if debugger.debug_state == st then
    return
  end

  local zbox = iup.GetDialogChild(debugger.main_dialog, "ZBOX_DEBUG_CONTINUE")
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  multitext.readonly = "Yes"

  if st == DEBUG_STOPPED then
    local btn_debug = iup.GetDialogChild(debugger.main_dialog, "BTN_DEBUG")
    zbox.value = btn_debug

    stop = "NO"
    step = "NO"
    cont = "NO"
    run = "NO"
    pause = "NO"
    dbg = "NO"
    curline = "NO"
  elseif st == DEBUG_ACTIVE or st == DEBUG_STEP_INTO or st == DEBUG_STEP_OVER or st == DEBUG_STEP_OUT then
    local btn_continue = iup.GetDialogChild(debugger.main_dialog, "BTN_CONTINUE")
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
    local btn_continue = iup.GetDialogChild(debugger.main_dialog, "BTN_CONTINUE")
    zbox.value = btn_continue

    stop = "YES"
    step = "YES"
    cont = "YES"
    run = "NO"
    pause = "NO"
    dbg = "NO"
    curline = "Yes"
  else -- st == DEBUG_INACTIVE
    local btn_debug = iup.GetDialogChild(debugger.main_dialog, "BTN_DEBUG")
    zbox.value = btn_debug

    stop = "NO"
    step = "NO"
    cont = "NO"
    run = "YES"
    pause = "NO"
    dbg = "YES"
    curline = "NO"

    multitext.readonly = "No"
    iupDebuggerClearLocalVariablesList()
    iupDebuggerClearStackList()
  end
    
  debugger.debug_state = st

  multitext.markerdeleteall = 2 -- current line highlight
  multitext.markerdeleteall = 3 -- current line arrow

  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_RUN"), "ACTIVE", run)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_CONTINUE"), "ACTIVE", cont)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_STEPOUT"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "ITM_CURRENTLINE"), "ACTIVE", curline)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_RUN"), "ACTIVE", run)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_STOP"), "ACTIVE", stop)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_PAUSE"), "ACTIVE", pause)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_CONTINUE"), "ACTIVE", cont)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_DEBUG"), "ACTIVE", dbg)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_STEPINTO"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_STEPOVER"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_STEPOUT"), "ACTIVE", step)
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_CURRENTLINE"), "ACTIVE", curline)
end                   

function iupDebuggerHighlightLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  debugger.currentLine = currentline

  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local pos = iup.TextConvertLinColToPos(multitext, currentline-1, 0) -- line here starts at 0
  multitext.caretpos = pos
  multitext.markerdeleteall = 2
  multitext["markeradd"..currentline-1] = 2
  multitext.markerdeleteall = 3
  multitext["markeradd"..currentline-1] = 3
end

function iupDebuggerShowCurrentLine()
  if debugger.currentLine then
    local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
    local pos = iup.TextConvertLinColToPos(multitext, debugger.currentLine-1, 0) -- line here starts at 0
    multitext.caretpos = pos
  end
end

function iupDebuggerSelectLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local pos = iup.TextConvertLinColToPos(multitext, currentline-1, 0) -- line here starts at 0
  multitext.caretpos = pos
  multitext.selection = currentline-1 .. ",0:" .. currentline-1 .. ",9999"
end


------------------------------------- Breakpoints -------------------------------------

function iupDebuggerNewBreakpoint()
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local fname = multitext.filename
  if (not fname) then fname = "" end
  local status, filename, line = iup.GetParam("New Breakpoint", nil, "Filename: %s\nLine: %i\n", fname, 1)

  if (status) then
    iup.SetAttributeId(multitext, "MARKERADD", line - 1, 1)-- in user interface line starts at 1, in Scintilla starts at 0
    iupDebuggerUpdateBreakpointsList()
  end
end

function iupDebuggerRemoveAllBreakpoints()
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  
  multitext.markerdeleteall = 1
  
  iupDebuggerUpdateBreakpointsList()
end

function iupDebuggerUpdateBreakpointsList()
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local list_break = iup.GetDialogChild(debugger.main_dialog, "LIST_BREAK")
  iup.SetAttribute(list_break, "REMOVEITEM", "ALL")

  local breakpoints = iupDebuggerGetBreakpoints(multitext)
  local filename = multitext.filename
  -- TODO add for all open files
  local has_breakpoints = false

  for index, line in pairs(breakpoints) do
    iup.SetAttribute(list_break, index, "Line "..line.." of "..filename)
    iup.SetAttribute(list_break, "LINE"..index, line)
    iup.SetAttribute(list_break, "FILENAME"..index, filename)
    has_breakpoints = true
  end

  if has_breakpoints then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_REMOVE"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_REMOVEALL"), "ACTIVE", "Yes")
    list_break.value = 1 -- select first item on list
  else
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_REMOVE"), "ACTIVE", "NO")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "BTN_REMOVEALL"), "ACTIVE", "NO")
  end
end

function iupDebuggerBreaksListAction(index)
  local list_break = iup.GetDialogChild(debugger.main_dialog, "LIST_BREAK")
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")

  local line = iup.GetAttribute(list_break, "LINE"..index)
  local filename = iup.GetAttribute(list_break, "FILENAME"..index)
  
  if multitext.filename == filename then
    iupDebuggerSelectLine(tonumber(line))
  end
end

function iupDebuggerRemoveBreakpoint(index)
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local list_break = iup.GetDialogChild(debugger.main_dialog, "LIST_BREAK")

  local line = iup.GetAttribute(list_break, "LINE"..index)
  local filename = iup.GetAttribute(list_break, "FILENAME"..index)
  
  if multitext.filename == filename then
    iup.SetAttributeId(debugger.main_dialog, "TOGGLEMARKER", line - 1, 2)
  else
    -- TODO update other filenames ???
    iupDebuggerUpdateBreakpointsList()
  end
end

function iupDebuggerHasLineBreak(filename, currentline)
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  return iupDebuggerHasBreakpoint(multitext, currentline - 1)
end

------------------------------------- Locals -------------------------------------

function iupDebuggerClearLocalVariablesList()
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_ALLLOCALS"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LOCAL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL"), "REMOVEITEM", "ALL")
end

function iupDebuggerSetLocalVariable()
  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")
  local index = local_list.value
  if (not index or tonumber(index) == 0) then
    iup.MessageError(debugger.main_dialog, "Select a variable on the list.")
    return
  end

  local level = iup.GetAttribute(local_list, "LEVEL"..index)
  local pos = iup.GetAttribute(local_list, "POS"..index)
  
  local name, value = debug.getlocal(level, pos)
  if (value == nil) then value = "" end
  local valueType = type(value)
  if valueType ~= "string" and valueType ~= "number" and valueType ~= "boolean" then
    iup.MessageError(debugger.main_dialog, "Can edit only strings, booleans and numbers.")
    return
  end

  local status, newValue
  if valueType == "string" then
    status, newValue = iup.GetParam("Set Local", nil, name.." = ".."%s\n", tostring(value))
  elseif valueType == "number" then
    status, newValue = iup.GetParam("Set Local", nil, name.." = ".."%R\n", tonumber(value))
  elseif valueType == "boolean" then
    if value then value = 1 else value = 0 end
    status, newValue = iup.GetParam("Set Local", nil, name.." = ".."%b[false,true]\n", value)
  end

  if (status) then
    if valueType == "string" then
      newValue = tostring(newValue)
    elseif valueType == "number" then
      newValue = tonumber(newValue)
    elseif valueType == "boolean" then
      newValue = tonumber(newValue)
      if newValue == 1 then 
        newValue = true
      else
        newValue = false
      end
    end
    
    debug.setlocal(level, pos, newValue)
    iup.SetAttribute(local_list, index, name.." = "..tostring(newValue).." <"..type(newValue)..">")
  end
end

function iupDebuggerPrintLocalVariable()
  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")
  local index = local_list.value

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  local level = iup.GetAttribute(local_list, "LEVEL"..index)
  local pos = iup.GetAttribute(local_list, "POS"..index)
  local name, value = debug.getlocal(level, pos)

  iupConsolePrint(local_list[index] .. "  (level="..level..", pos="..pos..")")
  iupConsolePrintValue(value)
end

function iupDebuggerPrintAllLocalVariables()
  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")
  local count = local_list.count

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local level = iup.GetAttribute(local_list, "LEVEL"..index)
    local pos = iup.GetAttribute(local_list, "POS"..index)
    local name, value = debug.getlocal(level, pos)

    iupConsolePrint(local_list[index] .. "  (level="..level..", pos="..pos..")")
    iupConsolePrintValue(value)
  end
end

function iupDebuggerUpdateLocalVariablesList(level, actual_level)
  local name, value
  local pos = 1
  local index = 1

  iupDebuggerClearLocalVariablesList()

  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")

  name, value = debug.getlocal(level, pos)
  while name ~= nil do
    if string.sub(name, 1, 1) ~= "(" then  -- do not include internal variables (loop control variables, temporaries, etc).
      iup.SetAttribute(local_list, index, name.." = "..tostring(value).." <"..type(value)..">")
      iup.SetAttribute(local_list, "POS"..index, pos)
      iup.SetAttribute(local_list, "LEVEL"..index, actual_level)
      index = index + 1
    end
    pos = pos + 1
    name, value = debug.getlocal(level, pos)
  end

  -- vararg (only for Lua >= 5.2, ignored in Lua 5.1)
  pos = -1
  name, value = debug.getlocal(level, pos)
  while name ~= nil do
    iup.SetAttribute(local_list, index, "vararg["..-pos.."] = "..tostring(value).." <"..type(value)..">")
    iup.SetAttribute(local_list, "POS"..index, pos)
    iup.SetAttribute(local_list, "LEVEL"..index, actual_level)
    index = index + 1
    pos = pos - 1
    name, value = debug.getlocal(level, pos)
  end

  if (index > 1) then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LOCAL"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_ALLLOCALS"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "Yes")

    local_list.value = 1 -- select first item on list
  end
end

function iupDebuggerLocalVariablesListAction(local_list, index)
  local level = iup.GetAttribute(local_list, "LEVEL"..index)
  local pos = iup.GetAttribute(local_list, "POS"..index)
  local name, value = debug.getlocal(level, pos)
  local valueType = type(value)
  if valueType == "string" or valueType == "number" or valueType == "boolean" then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "Yes")
  else
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "No")
  end
end

------------------------------------- Stack -------------------------------------

function iupDebuggerStackListAction(index)
  local level = index + debugger.startLevel - 1  -- this is level of the function
  level = level + 1 -- must fix the level because we called a Lua function
  local info = debug.getinfo(level, "Sl") -- source, currentline
  
  local filename = string.sub(info.source, 2)
  if debugger.currentFile == filename then
    iupDebuggerSelectLine(info.currentline)
  end
  
  iupDebuggerUpdateLocalVariablesList(level + 1, level) -- this is the level of the local variables inside that function
                                                     -- the actual level does not includes the fix
end

function iupDebuggerClearStackList()
  iupDebuggerClearLocalVariablesList()

  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LEVEL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_STACK"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "LIST_STACK"), "REMOVEITEM", "ALL")
end

function iupDebuggerPrintStackLevel()
  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  local index = list_stack.value

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  local level = index + debugger.startLevel - 1
  local defined = iup.GetAttribute(list_stack, "DEFINED"..index)

  iupConsolePrint(list_stack[index] .. "  (level="..level..")")
  iupConsolePrint(defined)
end

function iupDebuggerPrintStack()
  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  local count = list_stack.count

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local level = index + debugger.startLevel - 1
    local defined = iup.GetAttribute(list_stack, "DEFINED"..index)

    iupConsolePrint(list_stack[index] .. "  (level="..level..")")
    iupConsolePrint(defined)
  end
end

function iupDebuggerUpdateStackList()
  local info, desc, defined
  local level = debugger.startLevel
  
  iupDebuggerClearStackList()

  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  
  info = debug.getinfo(level, "Snl") -- source, name, namewhat, what, currentline, linedefined
  while  info ~= nil do
    if info.what == "main" then
      desc = "<main>"
    elseif info.name and info.name ~= "" then
      desc = info.name
    else
      desc = "<noname>"
    end
    if _VERSION ~= "Lua 5.1" then
      local call = debug.getinfo(level, "uf")
      local params = ""
      local pos = 1
      local name, value = debug.getlocal(call.func, pos)
      while name ~= nil do
        params = params .. name .. ", "
        pos = pos + 1
        name, value = debug.getlocal(call.func, pos)
      end
      if params ~= "" then 
        if call.isvararg then
          params = params .. "..."
        else
          params = string.sub(params, 1, -3) -- remove last ", "
        end
        desc = desc .. "(" .. params .. ")"
      end
    end
    if info.namewhat ~= "" then
        desc = desc .. " <".. info.namewhat .. ">"
    end
    if info.currentline > 0 then
       desc = desc .. " at line " .. info.currentline
    end

    if info.what == "C" then    
      defined = "   [Defined in C.]"
    else
      local s = string.sub(info.source, 1, 1)
      if s == "@" then
        local filename = string.sub(info.source, 2)
        defined = "   [Defined in the file: \"" .. filename .. "\" at line " .. info.linedefined .. ".]"
      else
        defined = "   [Defined in a string.]"
      end
    end

    local index = level - debugger.startLevel + 1
    iup.SetAttribute(list_stack, index, desc)
    iup.SetAttribute(list_stack, "DEFINED"..index, defined)

    level = level + 1
    if info.what == "main" then
      break
    end

    info = debug.getinfo(level, "Snl") -- source, name, namewhat, what, currentline, linedefined
  end
  
  if level > debugger.startLevel then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_STACK"), "ACTIVE", "YES")

    list_stack.value = 1 -- select first item on list
    iupDebuggerUpdateLocalVariablesList(debugger.startLevel + 1, debugger.startLevel + 1) -- this is the level of the local variables inside that function
  end
  
end


----------------------------  Debug State       --------------------------

function iupDebuggerGetDebugLevel()
  local level = -1
  repeat 
    level = level+1
  until debug.getinfo(level, "l") == nil  -- only current line, default is all info
  return level
end

function iupDebuggerUpdateState(filename, currentline)
  if debugger.debug_state == DEBUG_STEP_OUT then
    if debugger.stepFuncState == FUNC_STATE_OUTSIDE then
      iupDebuggerSetState(DEBUG_PAUSED)
      
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = debugger.currentFuncLevel
    end
  elseif debugger.debug_state == DEBUG_STEP_INTO or
      (debugger.debug_state == DEBUG_STEP_OVER and debugger.stepFuncState == FUNC_STATE_OUTSIDE) or
      (debugger.debug_state ~= DEBUG_PAUSED and iupDebuggerHasLineBreak(filename, currentline)) then
      iupDebuggerSetState(DEBUG_PAUSED)
  end
end

function iupDebuggerLineHook(filename, currentline)
  debugger.currentFuncLevel = iupDebuggerGetDebugLevel()

  iupDebuggerUpdateState(filename, currentline)
  
  if debugger.debug_state == DEBUG_PAUSED then
  
    iupDebuggerHighlightLine(currentline)
    
    iupDebuggerUpdateStackList()
    
    debug.sethook() -- turns off the hook
    
    while debugger.debug_state == DEBUG_PAUSED do
      iup.LoopStep()
    end
    
    debug.sethook(iupDebuggerHookFunction, "lcr") -- restore the hook
  end
  
  if debugger.debug_state == DEBUG_STOPPED then
    iupDebuggerEndDebug(true)
  end
end

function iupDebuggerCallHook()
  if debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == 0 then
      local level = iupDebuggerGetDebugLevel()
      debugger.stepFuncState = FUNC_STATE_INSIDE
      debugger.stepFuncLevel = level
    end
  end
end

function iupDebuggerReturnHook(what)
  local level = iupDebuggerGetDebugLevel()

  if what == "main" then
    iupDebuggerSetState(DEBUG_INACTIVE)
  elseif debugger.debug_state == DEBUG_STEP_OUT or debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == level then
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = 0
    end
  end
end

function iupDebuggerHookFunction(event, currentline)
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
      iupDebuggerCallHook()
    elseif event == "return" then
      iupDebuggerReturnHook(info.what)
    elseif event == "line" then
      iupDebuggerLineHook(filename, currentline)
    end
  end
end

function iupDebuggerStartDebug(filename)
  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  debugger.currentFile = multitext.filename
  debugger.startLevel = iupDebuggerGetDebugLevel() + 1 -- usually 3+1=4
  
  iupConsolePrint("-- Debug start")
  iupDebuggerSetState(DEBUG_ACTIVE)
  debugtabs.valuepos = 1

  debug.sethook(iupDebuggerHookFunction, "lcr")
end

function iupDebuggerEndDebug(stop)
  debug.sethook() -- turns off the hook

  iupDebuggerSetState(DEBUG_INACTIVE)

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  if stop then
    iupConsolePrint("-- Debug stop!")
    error() -- abort processing, no error message
  else
    iupConsolePrint("-- Debug finish")
  end
end

function iupDebuggerExit()
  if debugger.debug_state ~= DEBUG_INACTIVE then
    iupDebuggerEndDebug(true)
  end
end
