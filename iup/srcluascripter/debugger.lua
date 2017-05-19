DEBUG_INACTIVE = 1
DEBUG_ACTIVE = 2
DEBUG_RUN = 3
DEBUG_STEP_INTO = 4
DEBUG_STEP_OVER = 5
DEBUG_STEP_OUT = 6
DEBUG_PAUSED = 7
DEBUG_STOPPED = 8

FUNC_INSIDE = 1
FUNC_OUTSIDE = 2

LUA_COPYRIGHT = "Copyright (C) 1994-2008 Lua.org, PUC-Rio"
LUA_AUTHORS = "R. Ierusalimschy, L. H. de Figueiredo & W. Celes"

local debug_state = DEBUG_INACTIVE

local breakpoints = {}

local initialStackLevel = 2

local currentFuncLevel = 0
local stepFuncLevel = 0
local stepFuncState = 0

function readFile(filename)

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

function update_title(multitext, filename, is_dirty)

	local subtitle = iup.GetAttribute(multitext, "SUBTITLE")
	 local dirty_sign = ""

	if is_dirty then
		dirty_sign = "*"
	end

	if filename == nil then filename = "Untitled" end

	--iup.SetAttribute(multitext, "TITLE", string.format("%s%s - %s", str_filetitle(filename), dirty_sign, subtitle))
end

function reloadFile(filename)

	  local str = readFile(filename)
	  if (str) then
		local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
		
		local fname = string.sub(filename, 2)

		iup.SetAttribute(multitext, "FILENAME", fname)
		iup.SetAttribute(multitext, "DIRTY", "NO")
		iup.SetAttribute(multitext, "VALUE", str)

		update_title(multitext, fname, 0)
		
		setCurrentFile(fname)
		
		iup.SetAttribute(multitext, "MARKERDELETEALL", -1)

		for i = 1, #breakpoints do
			local item = breakpoints[i]
			if item.filename == fname then
				iup.SetAttribute(main_dialog, "DONTTOGGLE", 1)
				iup.SetAttributeId(main_dialog, "TOGGLEMARKER", item.line-1, 2)
				iup.SetAttribute(main_dialog, "DONTTOGGLE", 0)
			end
		end
	end
end

function showVersionInfo()
  consoleEnterMessagef("IupLuaScripter 0.0\n"..
   "%s  %s\n"..
   "  (written by %s)\n"..
   "IUP %s\n"..
   "  (copyright (c) 1995-2008 Tecgraf/PUC-Rio - Petrobrás)\n\n",
   _VERSION, LUA_COPYRIGHT, LUA_AUTHORS, iup.GetGlobal("VERSION")
  )
  iup.SetAttribute(console.tbsDebug, "VALUE", iup.GetHandle("tabOutput"))
  iup.SetFocus(console.txtCmdLine)
end

function consoleListFuncAction()

	consoleEnterCommandStr("consoleListFunc()")
	
	consoleListFunc()
	
end

function consoleListVarAction()

	consoleEnterCommandStr("consoleListVar()")
	
	consoleListVar()
	
end

function consoleClearAction()

  iup.SetAttribute(console.mtlOutput, "VALUE", "")
	
end

function removeAllBreakpoints()
	local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
	
	iup.SetAttribute(multitext, "MARKERDELETEALL", 1)
	
	breakpoints = {}
	
	updateBreakpointsList()
end

function listStackAction(level)
	local info = debug.getinfo(level+4, "Snl")
	
	updateSourceLine(info)
	
	updateLocal(level+4)
end

function toggleBreakpoint(line, mark, value)

	local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
	
	if iup.GetAttribute(main_dialog, "DONTTOGGLE") then
		return
	end

	if value > 0 then
		insertBreakpoint(iup.GetAttribute(multitext, "FILENAME"), line)
	else
		removeBreakpoint(iup.GetAttribute(multitext, "FILENAME"), line)
	end
	
	--for i, k in pairs(breakpoints) do
		--table.sort(k)
	--end
	
	updateBreakpointsList()

end

function updateBreakpointsList()
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_BREAK"), "1", nil)
	for i = 1, #breakpoints do
		local item = breakpoints[i]
		iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_BREAK"), i, "Line "..item.line.." of "..item.filename)
	end
end

function debug_set_state(st)
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
	elseif st == DEBUG_ACTIVE or
		   st == DEBUG_RUN or
		   st == DEBUG_STEP_INTO or
		   st == DEBUG_STEP_OVER or
		   st == DEBUG_STEP_OUT then
		 iup.SetAttribute(iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE"), "VALUE", "BTN_CONTINUE")
		 stop = "YES"
		 step = "NO"
		 contin = "NO"
		 run = "NO"
		 pause = "YES"
		 curline = "NO"
		 if st == DEBUG_STEP_OUT then
			--stepFuncLevel = getDebugLevel(0)
			stepFuncLevel = currentFuncLevel
			stepFuncState = FUNC_INSIDE
		 else
			stepFuncLevel = 0
			stepFuncState = FUNC_OUTSIDE
		 end
	elseif st == DEBUG_PAUSED then
		iup.SetAttribute(iup.GetDialogChild(main_dialog, "ZBOX_DEBUG_CONTINUE"), "VALUE", "BTN_CONTINUE")
		breakAtCaret = nil
		stop = "YES"
		step = "YES"
		contin = "YES"
		run = "NO"
		pause = "NO"
		curline = "YES"
	end
	  
	debug_state = st

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
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_CURLINE"), "ACTIVE", curline)
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "BTN_CURLINEBREAK"), "ACTIVE", curline)
end                   

function insertBreakpoint(filename, line)

	local item = {}
	item.filename = filename
	item.line = line
	table.insert(breakpoints, item)
    
end

function removeBreakpointFromList(index)

	local item = breakpoints[index]
	
	if iup.GetAttribute(iup.GetDialogChild(main_dialog, "MULTITEXT"), "FILENAME") == item.filename then
		--toggleBreakpoint(item.line, 1, 2)
		iup.SetAttributeId(iup.GetDialogChild(main_dialog, "MULTITEXT"), "TOGGLEMARKER", item.line, 2)
	else
		removeBreakpoint(item.filename, item.line)
	end
	
	updateBreakpointsList()
end

function removeBreakpoint(filename, line)

    local pos = nil
	
	for i = 1, #breakpoints do
		local item = breakpoints[i]
		if item.filename == filename and item.line == line then
			table.remove(breakpoints, i)
			break
		end
	end
    
end

function highlightLine(multitext, line)
   local pos = iup.TextConvertLinColToPos(multitext, line, 0)
   iup.SetAttribute(multitext, "CARETPOS", pos)
   iup.SetAttribute(multitext, "MARKERDELETEALL", 2)
   iup.SetAttributeId(multitext, "MARKERADD", line, 2)
end

function updateSource(filename)

	reloadFile(filename)
	
end

function updateSourceLine(info)

  local multitext = iup.GetDialogChild(main_dialog, "MULTITEXT")
  
	if info.currentline == nil or info.currentline <= 0 then
		return
	end

	iup.SetAttribute(multitext, "CARET", string.format("%d,7", info.currentline))
	--iup.SetAttribute(multitext, "SELECTION", string.format("%d,1:%d,7", info.currentline, info.currentline))
	highlightLine(multitext, info.currentline-1)

end

function hasLineBreak(filename, line)

	if breakAtCaret and breakAtCaret == line then
		breakAtCaret = nil
		return true
	end

	for i = 1, #breakpoints do
		local item = breakpoints[i]
		if item.filename == filename and item.line == line then
			return true
		end
	end
    
    return false
end

function clearLocal()
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LOCAL"), "ACTIVE", "NO")
	--iup.SetAttribute(iup.GetDialogChild(main_dialog, "SET_LOCAL"), "ACTIVE", "NO")
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "1", nil)
end

function clearStack()
	clearLocal()
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "NO")
	--iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "NO")
	iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_STACK"), "1", nil)
end

function getObject(name)
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

function SetLocal(index)

	local level = iup.GetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "LEVEL"..index)
	local pos = iup.GetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "POS"..index)
	
	local name, value = debug.getlocal(level, pos)
	
	local state, newValue = iup.GetParam("Set Local", nil, name.." = ".."%s\n", value)

	debug.setlocal(level, pos, newValue)
	
	updateLocal(level)
end

function updateLocal(level)
	local name, value
	local pos = 1
	local val_key = 1
	clearLocal()
	name, value = debug.getlocal(level+1, pos)
	while name ~= nil do
		if string.sub(name, 1, 1) ~= "(" then
			iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), val_key, name.." = "..getObject(value))
			iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "POS"..val_key, pos)
			iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_LOCAL"), "LEVEL"..val_key, level+1)
			val_key = val_key + 1
		end
		pos = pos + 1
		name, value = debug.getlocal(level+1, pos)
	end
end

function updateStack()

	local info, name
	local level = 4
	
	clearStack()
	
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
	
	if level> initialStackLevel then
		iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_LEVEL"), "ACTIVE", "YES")
		iup.SetAttribute(iup.GetDialogChild(main_dialog, "PRINT_STACK"), "ACTIVE", "YES")
		iup.SetAttribute(iup.GetDialogChild(main_dialog, "LIST_STACK"), "VALUE", "1")
		updateLocal(4)
	end
	
end

function getDebugLevel(offset)

	local level = -1
	
	repeat 
		level = level+1
	until debug.getinfo(level, "Snl") == nil
	
	return level - offset
end

function updateState(filename, line)

	if debug_state == DEBUG_STEP_OUT then
		if stepFuncState == FUNC_OUTSIDE then
			local level = getDebugLevel(0)
			
			debug_set_state(DEBUG_PAUSED)
			
			stepFuncState = FUNC_OUTSIDE
			stepFuncLevel = currentFuncLevel
		end
	elseif debug_state == DEBUG_STEP_INTO or
			(debug_state == DEBUG_STEP_OVER and stepFuncState == FUNC_OUTSIDE) or
			(debug_state ~= DEBUG_PAUSED and hasLineBreak(filename, line)) then
			debug_set_state(DEBUG_PAUSED)
	end

end

function hookFunction(event, line)
   
	local info = debug.getinfo(initialStackLevel, "Snl")
	
		local s = string.sub(info.source, 1, 1)
		if s ~= "@" then
			return
		end
	
	if debug_state ~= DEBUG_INACTIVE then
		if event == "call" then
			callHook(info)
		elseif event == "return" then
			returnHook(info)
		elseif event == "line" then
			lineHook(info, line)
		end
	end

end

function lineHook(info, line)

	currentFuncLevel = getDebugLevel(0)

	updateState(string.sub(info.source, 2), line)
	
	if debug_state == DEBUG_PAUSED then
	
		if currentFile ~= string.sub(info.source, 2) then
			updateSource(info.source)
		end
	
		updateSourceLine(info)
		
		updateStack()
		
		debug.sethook()
		
		while debug_state == DEBUG_PAUSED do
			iup.LoopStep()
		end
		
		debug.sethook(hookFunction, "lcr")
		
	elseif iup.LoopStep() == IUP_CLOSE then
		debug_set_state(DEBUG_STOPPED)
	end
	
	if debug_state == DEBUG_STOPPED then
		debug_set_state(DEBUG_ACTIVE)
		debug.sethook()
		error("debug: Last command interrupted!", 0)
	end
end

function callHook(info)

	local level = getDebugLevel(0)

	if debug_state == DEBUG_STEP_OVER then
		if stepFuncLevel == 0 then
			stepFuncState = FUNC_INSIDE
			stepFuncLevel = level
		end
	end
	
end

function returnHook(info)

	local level = getDebugLevel(0)

	if level == startLevel+1 and info.what == "main" then
		debug_set_state(DEBUG_INACTIVE)
	elseif debug_state == DEBUG_STEP_OUT or debug_state == DEBUG_STEP_OVER then
		if stepFuncLevel == level then
			stepFuncState = FUNC_OUTSIDE
			stepFuncLevel = 0
		end
	end
	
end

function setCurrentFile(filename)
	currentFile = filename
end

function startDebug(filename, runMode)

	currentFile = filename
	
	startLevel = getDebugLevel(0)
	
	if runMode == false or runMode == nil then
		debug.sethook(hookFunction, "lcr")
	end
	
	local ok, msg = pcall(dofile, currentFile)
	
	debug_set_state(DEBUG_INACTIVE)
	
	if ok==false then
		print(msg)
	end
end
