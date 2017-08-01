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

  currentFile = nil,
  currentLine = nil,

  main_dialog = nil,
}

function iup.DebuggerInit(main_dialog)
  debugger.main_dialog = main_dialog
end


------------------------------------- User Interface State -------------------------------------

function iup.DebuggerSetStateString(state)
  local map_state = {
    DEBUG_INACTIVE = DEBUG_INACTIVE,
    DEBUG_ACTIVE = DEBUG_ACTIVE,
    DEBUG_STOPPED = DEBUG_STOPPED,
    DEBUG_STEP_INTO = DEBUG_STEP_INTO,
    DEBUG_STEP_OVER = DEBUG_STEP_OVER,
    DEBUG_STEP_OUT = DEBUG_STEP_OUT,
    DEBUG_PAUSED = DEBUG_PAUSED,
  }

  iup.DebuggerSetState(map_state[state])
end

function iup.DebuggerSetState(st)
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
    iup.DebuggerClearLocalVariablesList()
    iup.DebuggerClearStackList()
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

function iup.DebuggerHighlightLine(currentline)
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

function iup.DebuggerShowCurrentLine()
  if debugger.currentLine then
    local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
    local pos = iup.TextConvertLinColToPos(multitext, debugger.currentLine-1, 0) -- line here starts at 0
    multitext.caretpos = pos
  end
end

function iup.DebuggerSelectLine(currentline)
  if currentline == nil or currentline <= 0 then
    return
  end

  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local pos = iup.TextConvertLinColToPos(multitext, currentline-1, 0) -- line here starts at 0
  multitext.caretpos = pos
  multitext.selection = currentline-1 .. ",0:" .. currentline-1 .. ",9999"
end


------------------------------------- Breakpoints -------------------------------------

function iup.DebuggerNewBreakpoint()
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local fname = multitext.filename
  if (not fname) then fname = "" end
  local status, filename, line = iup.GetParam("New Breakpoint", nil, "Filename: %s\nLine: %i\n", fname, 1)

  if (status) then
    iup.SetAttributeId(multitext, "MARKERADD", line - 1, 1)-- in user interface line starts at 1, in Scintilla starts at 0
    iup.DebuggerUpdateBreakpointsList()
  end
end

function iup.DebuggerRemoveAllBreakpoints()
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  
  multitext.markerdeleteall = 1
  
  iup.DebuggerUpdateBreakpointsList()
end

function iup.DebuggerUpdateBreakpointsList()
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local list_break = iup.GetDialogChild(debugger.main_dialog, "LIST_BREAK")
  iup.SetAttribute(list_break, "REMOVEITEM", "ALL")

  local breakpoints = iup.DebuggerGetBreakpoints(multitext)
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

function iup.DebuggerBreaksListAction(index)
  local list_break = iup.GetDialogChild(debugger.main_dialog, "LIST_BREAK")
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")

  local line = iup.GetAttribute(list_break, "LINE"..index)
  local filename = iup.GetAttribute(list_break, "FILENAME"..index)
  
  if multitext.filename == filename then
    iup.DebuggerSelectLine(tonumber(line))
  end
end

function iup.DebuggerRemoveBreakpoint(index)
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  local list_break = iup.GetDialogChild(debugger.main_dialog, "LIST_BREAK")

  local line = iup.GetAttribute(list_break, "LINE"..index)
  local filename = iup.GetAttribute(list_break, "FILENAME"..index)
  
  if multitext.filename == filename then
    iup.SetAttributeId(debugger.main_dialog, "TOGGLEMARKER", line - 1, 2)
  else
    -- TODO update other filenames ???
    iup.DebuggerUpdateBreakpointsList()
  end
end

function iup.DebuggerHasLineBreak(filename, currentline)
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  return iup.DebuggerHasBreakpoint(multitext, currentline - 1)
end

------------------------------------- Locals -------------------------------------

function iup.DebuggerClearLocalVariablesList()
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_ALLLOCALS"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LOCAL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL"), "REMOVEITEM", "ALL")
end

function iup.DebuggerSetLocalVariable()
  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")
  local index = local_list.value
  if (not index or tonumber(index) == 0) then
    iup.MessageError(debugger.main_dialog, "Select a variable on the list.")
    return
  end

  local name = local_list[index]
  local s, e = string.find(name, " =", 1, true)
  name = string.sub(name, 1, s - 1)
  local value = local_list["VAL"..index]

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
  
    local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
    local index = tonumber(list_stack.value)

    -- here there are 4 levels on top of the stack: 
    --   1-DebuggerSetLocalVariable, 
    --   2-LoopStep
    --   3-DebuggerLineHook, 
    --   4-DebuggerHookFunction
    local startLevel = 5
    local level = index - 1 + startLevel  -- this is the level of the function
    
    iup.DebuggerSetLocal(local_list, level, index, newValue)
    iup.DebuggerSetLocalListItem(local_list, index, name, newValue) -- do not set pos
  end
end

function iup.DebuggerPrintLocalVariable()
  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")
  local index = local_list.value

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  local pos = iup.GetAttribute(local_list, "POS"..index)
  local value = local_list["VAL"..index]

  iup.ConsolePrint(local_list[index] .. "  (pos="..pos..")")
  iup.ConsolePrintValue(value)
end

function iup.DebuggerPrintAllLocalVariables()
  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")
  local count = local_list.count

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local pos = iup.GetAttribute(local_list, "POS"..index)
    local value = local_list["VAL"..index]

    iup.ConsolePrint(local_list[index] .. "  (pos="..pos..")")
    iup.ConsolePrintValue(value)
  end
end

function iup.DebuggerSetLocal(local_list, level, index, newValue)
  local pos = iup.GetAttribute(local_list, "POS"..index)
  local list_value = iup.GetAttribute(local_list, index)
  local s = string.sub(list_value, 1, 3)
  if s == ":: " then
    debug.setupvalue(local_list.func, pos, newValue)
  else
    level = level + 1 -- this is the level inside this function
    debug.setlocal(level, pos, newValue)
  end
end

function iup.DebuggerGetLocal(local_list, level, index)
  local name, value
  local pos = iup.GetAttribute(local_list, "POS"..index)
  local list_value = iup.GetAttribute(local_list, index)
  local s = string.sub(list_value, 1, 3)
  if s == ":: " then
    name, value = debug.getupvalue(local_list.func, pos)
  else
    level = level + 1 -- this is the level inside this function
    name, value = debug.getlocal(level, pos)
  end
  return name, value
end

function iup.DebuggerLocalVariablesListAction(local_list, index)
  local value = local_list["VAL"..index]
  local valueType = type(value)
  if valueType == "string" or valueType == "number" or valueType == "boolean" then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "Yes")
  else
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "No")
  end
end

function iup.DebuggerSetLocalListItem(local_list, index, name, value, pos)
  local valueType = type(value)
  if valueType ~= "string" and valueType ~= "number" and valueType ~= "boolean" then
    iup.SetAttribute(local_list, index, name.." = <"..tostring(value)..">")
  else
    iup.SetAttribute(local_list, index, name.." = "..tostring(value).." <"..valueType..">")
  end

  local_list["VAL"..index] = value

  if pos then
    iup.SetAttribute(local_list, "POS"..index, pos)
  end
end

function iup.DebuggerUpdateLocalVariablesList(level)
  local name, value
  local pos
  local index = 1

  level = level + 1 -- this is the level inside this function

  iup.DebuggerClearLocalVariablesList()

  local local_list = iup.GetDialogChild(debugger.main_dialog, "LIST_LOCAL")

  pos = 1
  name, value = debug.getlocal(level, pos)
  while name ~= nil do
    if string.sub(name, 1, 1) ~= "(" then  -- do not include internal variables (loop control variables, temporaries, etc).
      iup.DebuggerSetLocalListItem(local_list, index, name, value, pos)
      index = index + 1
    end

    pos = pos + 1
    name, value = debug.getlocal(level, pos)
  end

  -- vararg (only for Lua >= 5.2, ignored in Lua 5.1)
  pos = -1
  name, value = debug.getlocal(level, pos)
  while name ~= nil do
    name = "vararg[" .. -pos .. "]"
    iup.DebuggerSetLocalListItem(local_list, index, name, value, pos)
    index = index + 1

    pos = pos - 1
    name, value = debug.getlocal(level, pos)
  end

  local call = debug.getinfo(level, "uf")
  if call.nups > 0 then
    pos = 1
    local_list.func = call.func
    name, value = debug.getupvalue(call.func, pos)
    while name ~= nil do
      name = ":: " .. name
      iup.DebuggerSetLocalListItem(local_list, index, name, value, pos)
      index = index + 1

      pos = pos + 1
      name, value = debug.getupvalue(call.func, pos)
    end
  else
    local_list.func = nil
  end

  if (index > 1) then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LOCAL"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_ALLLOCALS"), "ACTIVE", "Yes")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "SET_LOCAL"), "ACTIVE", "Yes")

    local_list.value = 1 -- select first item on list
  end
end

function iup.DebuggerShowTip(word, line)

  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  local count = iup.GetAttribute(list_stack, "COUNT")
  local index = tonumber(list_stack.value)

  local filename = iup.GetAttribute(list_stack, "FILENAME"..index)
  
  if not filename or debugger.currentFile ~= filename then
    return
  end  

  -- here there are 4 levels on top of the stack: 
  --   1-DebuggerShowTip, 
  --   2-LoopStep
  --   3-DebuggerLineHook, 
  --   4-DebuggerHookFunction
  local startLevel = 5
  local level = index - 1 + startLevel  -- this is the level of the function

  local info = debug.getinfo(level, "SnlL") -- source, name, namewhat, what, currentline, linedefined
  if info.linedefined == 0 and info.lastlinedefined == 0 then
    if not info.activelines[line] then
      return
    end
  elseif line < info.linedefined or line > info.lastlinedefined then
    return
  end

  local pos = 1
  local name, value = debug.getlocal(level, pos)
  while name ~= nil do
    if name == word then
      return name, tostring(value)
    end
    pos = pos + 1
    name, value = debug.getlocal(level, pos)
  end
  
  for gname, gvalue in pairs(_G) do 
    if gname == word then
      return gname, tostring(gvalue)
    end
  end

end

------------------------------------- Stack -------------------------------------


function iup.DebuggerStackListAction(list_stack, index)
  
  -- here there are 4 levels on top of the stack: 
  --   1-DebuggerStackListAction, 
  --   2-LoopStep
  --   3-DebuggerLineHook, 
  --   4-DebuggerHookFunction
  local startLevel = 5
  local level = index - 1 + startLevel  -- this is the level of the function
  
  local filename = iup.GetAttribute(list_stack, "FILENAME"..index)
  if filename and debugger.currentFile == filename then
    local currentline = iup.GetAttribute(list_stack, "CURRENTLINE"..index)
    iup.DebuggerSelectLine(tonumber(currentline))
  end
  
  iup.DebuggerUpdateLocalVariablesList(level)
end

function iup.DebuggerClearStackList()
  iup.DebuggerClearLocalVariablesList()

  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LEVEL"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_STACK"), "ACTIVE", "NO")
  iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "LIST_STACK"), "REMOVEITEM", "ALL")
end

function iup.DebuggerPrintStackLevel()
  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  local index = list_stack.value

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  local defined = iup.GetAttribute(list_stack, "DEFINED"..index)

  iup.ConsolePrint(list_stack[index] .. "  (level="..index..")")
  iup.ConsolePrint(defined)
end

function iup.DebuggerPrintStack()
  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  local count = list_stack.count

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  for index = 1, count do
    local defined = iup.GetAttribute(list_stack, "DEFINED"..index)

    iup.ConsolePrint(list_stack[index] .. "  (level="..index..")")
    iup.ConsolePrint(defined)
  end
end

function iup.DebuggerUpdateStackList()
  local info, desc, defined
  
  -- here there are 3 levels on top of the stack: 
  --   1-DebuggerUpdateStackList, 
  --   2-DebuggerLineHook, 
  --   3-DebuggerHookFunction
  local startLevel = 4
  local level = startLevel

  iup.DebuggerClearStackList()

  local list_stack = iup.GetDialogChild(debugger.main_dialog, "LIST_STACK")
  
  info = debug.getinfo(level)--, "Snl") -- source, name, namewhat, what, currentline, linedefined
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

    local filename
    if info.what == "C" then    
      defined = "  [Defined in C"
    else
      local s = string.sub(info.source, 1, 1)
      if s == "@" then
        filename = string.sub(info.source, 2)
        defined = "  [Defined in the file: \"" .. filename .. "\""
      else
        local short_src = string.sub(info.short_src, 2, -2)
        defined = "  [Defined in a " .. short_src
      end
    end
    if info.linedefined > 0 then
       defined = defined .. ", line " .. info.linedefined .. "]"
    else
       defined = defined .. "]"
    end

    local index = (level - startLevel) + 1
    iup.SetAttribute(list_stack, index, desc)
    iup.SetAttribute(list_stack, "DEFINED"..index, defined)
    iup.SetAttribute(list_stack, "FILENAME"..index, filename)
    iup.SetAttribute(list_stack, "CURRENTLINE"..index, info.currentline)

    level = level + 1

    info = debug.getinfo(level)--, "Snl") -- source, name, namewhat, what, currentline, linedefined
  end
  
  if level > startLevel then
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
    iup.SetAttribute(iup.GetDialogChild(debugger.main_dialog, "PRINT_STACK"), "ACTIVE", "YES")

    list_stack.value = 1 -- select first item on list (startLevel)
    iup.DebuggerUpdateLocalVariablesList(startLevel)
  end
  
end


----------------------------  Debug State       --------------------------

function iup.DebuggerGetFuncLevel()
-- level 0 is the current function (getinfo itself); 
-- level 1 is the function that called getinfo (DebuggerGetFuncLevel)
  local func_level = 1
  repeat 
    func_level = func_level + 1
  until debug.getinfo(func_level, "l") == nil  -- only current line, default is all info
  return func_level - 1
end

function iup.DebuggerUpdateState(filename, currentline)
  if debugger.debug_state == DEBUG_STEP_OUT then
    if debugger.stepFuncState == FUNC_STATE_OUTSIDE then
      iup.DebuggerSetState(DEBUG_PAUSED)
      
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = debugger.currentFuncLevel
    end
  elseif debugger.debug_state == DEBUG_STEP_INTO or
      (debugger.debug_state == DEBUG_STEP_OVER and debugger.stepFuncState == FUNC_STATE_OUTSIDE) or
      (debugger.debug_state ~= DEBUG_PAUSED and iup.DebuggerHasLineBreak(filename, currentline)) then
      iup.DebuggerSetState(DEBUG_PAUSED)
  end
end

function iup.DebuggerLineHook(filename, currentline)
  debugger.currentFuncLevel = iup.DebuggerGetFuncLevel()

  iup.DebuggerUpdateState(filename, currentline)
  
  if debugger.debug_state == DEBUG_PAUSED then
  
    iup.DebuggerHighlightLine(currentline)
    
    iup.DebuggerUpdateStackList()
    
    debug.sethook() -- turns off the hook
    
    while debugger.debug_state == DEBUG_PAUSED do
      iup.LoopStep()
    end
    
    debug.sethook(iup.DebuggerHookFunction, "lcr") -- restore the hook
  end
  
  if debugger.debug_state == DEBUG_STOPPED then
    iup.DebuggerEndDebug(true)
  end
end

function iup.DebuggerCallHook()
  if debugger.debug_state == DEBUG_STEP_OVER then
    if debugger.stepFuncLevel == 0 then
      local func_level = iup.DebuggerGetFuncLevel()
      debugger.stepFuncState = FUNC_STATE_INSIDE
      debugger.stepFuncLevel = func_level
    end
  end
end

function iup.DebuggerReturnHook(what)
  if what == "main" then
    iup.DebuggerSetState(DEBUG_INACTIVE)
  elseif debugger.debug_state == DEBUG_STEP_OUT or debugger.debug_state == DEBUG_STEP_OVER then
    local func_level = iup.DebuggerGetFuncLevel()
    if debugger.stepFuncLevel == func_level then
      debugger.stepFuncState = FUNC_STATE_OUTSIDE
      debugger.stepFuncLevel = 0
    end
  end
end

function iup.DebuggerHookFunction(event, currentline)

  -- how many levels we have before the hook was invoked?
  -- hook is always at level 2 when called
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
      iup.DebuggerCallHook()
    elseif event == "return" then
      iup.DebuggerReturnHook(info.what)
    elseif event == "line" then
      iup.DebuggerLineHook(filename, currentline)
    end
  end
end

function iup.DebuggerStartDebug(filename)
  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  local multitext = iup.GetDialogChild(debugger.main_dialog, "MULTITEXT")
  debugger.currentFile = multitext.filename
  
  iup.ConsolePrint("-- Debug start")
  iup.DebuggerSetState(DEBUG_ACTIVE)
  debugtabs.valuepos = 1

  debug.sethook(iup.DebuggerHookFunction, "lcr")
end

function iup.DebuggerEndDebug(stop)
  debug.sethook() -- turns off the hook

  iup.DebuggerSetState(DEBUG_INACTIVE)

  local debugtabs = iup.GetDialogChild(debugger.main_dialog, "DEBUG_TABS")
  debugtabs.valuepos = 0

  if stop then
    iup.ConsolePrint("-- Debug stop!")
    error() -- abort processing, no error message
  else
    iup.ConsolePrint("-- Debug finish")
  end
end

function iup.DebuggerExit()
  if debugger.debug_state ~= DEBUG_INACTIVE then
    iup.DebuggerEndDebug(true) -- make a stop
  end
end
