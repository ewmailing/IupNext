console = {}

console.create = function()

	io.write = consolePrint
	print = consolePrint
	
	console.cmdList = {}

	console.txtCmdLine = iup.GetDialogChild(main_dialog, "TXT_CMDLINE")
	console.mtlOutput = iup.GetDialogChild(main_dialog, "MTL_OUTPUT")
	
	console.currentListInd = 0
	console.hold_caret = false
	
	console.identLevel = 0
	
end

function consolePrint(...)

	local buffer = ""
	
	for i, v in ipairs{...} do
		if i > 1 then
			buffer = buffer.."\t"
		end
		if type(v) == "string" then
			buffer = buffer..v
		elseif type(v) == "number" then
			buffer = buffer..v
		elseif type(v) == "nil" then
			buffer = buffer.."nil"
		elseif type(v) == "table" then
			buffer = buffer.."<table>"
		elseif type(v) == "function" then
			buffer = buffer.."<function>"
		else
			buffer = buffer.."<unknown>"
		end
	end
	consoleEnterMessage(buffer, 1)
end

function consoleIndent()
	local level = console.identLevel
	if level == 0 then
		return ""
	elseif level == 1 then
		return "  "
	elseif level == 2 then
		return "    "
	elseif level == 3 then
		return "      "
	elseif level == 4 then
		return "        "
	elseif level == 5 then
		return "          "
	else
		return "            "
	end
end

function consoleCommandListInsert(text)
	table.insert(console.cmdList, text)
	console.currentListInd = #console.cmdList
end

function consoleCommandListFindStr(text)

	for i = 1, #console.cmdList do
		if text == console.cmdList[i] then
			return i
		end
	end
	return nil
end

function consolePrintObject(object, value)

	if type(value)=="string" then
		consoleEnterMessagef("%s%s = \"%s\"", consoleIndent(), object, value)
	elseif type(value)=="userdata" then
		consoleEnterMessagef("%s%s = <userdata> (type = %s)", consoleIndent(), object, type(value))
	elseif type(value)==nil then
		consoleEnterMessagef("%s%s = nil", consoleIndent(), object)
	elseif type(value)=="number" then
		local fmt
		if value == math.floor(value) then
			fmt = "%s%s = %d"
		else
			fmt = "%s%s = %f"
		end
		consoleEnterMessagef(fmt, consoleIndent(), object, value)
	elseif type(value) == "table" then
		if console.identLevel < 6 then
			consoleEnterMessagef("%s%s = ", consoleIndent(), object)
			consoleEnterMessagef("%s[", consoleIndent())
			console.identLevel = console.identLevel + 1
			local holdCaret = console.hold_caret
			if holdCaret==false then
				consoleHoldCaret(true)
			end
			consolePrintTable(value)
			if holdCaret==false then
				consoleHoldCaret(false)
			end
			console.identLevel = console.identLevel - 1
			consoleEnterMessagef("%s}", console.identLevel)
		else
			consoleEnterMessagef("%s{table too deep}", consoleIndent())
		end
	elseif type(value)=="function" then
		local info = debug.getinfo(object, "Snl")
		if info.name ~= nil then
			if string.sub(info.source, 1, 1) ~= "@" then
				consoleEnterMessagef("%s%s = <function> (\"%s\"%s) [defined in a string at line %d]", consoleIndent(), object,
					info.name, info.namewhat, info.linedefined)
			else
				consoleEnterMessagef("%s%s = <function> (\"%s\"%s) [defined in the file \"%s\" at line %d]", consoleIndent(),
					object, info.name, info.namewhat, info.source, info.linedefined)
			end
		elseif string.sub(info.source, 1, 1) ~= "@" then
			consoleEnterMessagef("%s%s = <function> [defined in a string at line %d]", consoleIndent(), object, info.linedefined)
		else
			consoleEnterMessagef("%s%s = <function> [defined in the file \"%s\" at line %d", consoleIndent(), object, info.source,
				info.linedefined)
		end
	else
		consoleEnterMessagef("%s%s = <unknown> (type = %s)", consoleIndent(), object, type(value))
	end
end

function consoleEnterCommandStr(text)
	local ind = consoleCommandListFindStr(text)
	
	if ind == nil then
		consoleCommandListInsert(text)
		console.currentListInd = #console.cmdList
	else
		console.currentListInd = ind
	end
	consoleEnterMessagef("> %s", text)
end

function consoleEnterCommandStrf(text, ...)
	local value = string.format(text, ...)
	console.cmdEnterCommandStr(value)
end

function consoleEnterMessagef(message, ...)
	local value = string.format(message, ...)
	consoleEnterMessage(value, 1)
end

function consoleEnterMessage(text, insert_new_line)

	local value  = console.mtlOutput.value
	value = value..text
	if insert_new_line == 1 then
		value = value.."\n"
	end
	console.mtlOutput.value = value
end

function consoleHoldCaret(hold)
	if hold==false then
		iup.SetAttribute(console.mtlOutput, "CARET", "65535,65535")
	end
	console.hold_caret = hold
end

function consoleEnterCommand()

	local command = console.txtCmdLine.value
	
	if command == nil then
		return
	end

	local holdCaret = console.holdCaret

	if holdCaret==false then
		consoleHoldCaret(true)
	end
	
	consoleEnterCommandStr(command)
	
	local value = _G[command]
	if value then
		console.identLevel = 0
		consolePrintObject(command, value)
	else
		consolePrintExpression(command)
	end

	if holdCaret==false then
		consoleHoldCaret(false)
	end
	
	console.txtCmdLine.value = ""
	iup.SetFocus(console.txtCmdLine)
end

function consoleKeyUpCommand()
	if #console.cmdList > 0 then
		if console.currentListInd >= 1 then
			console.txtCmdLine.value = console.cmdList[console.currentListInd]
			if console.currentListInd > 1 then
				console.currentListInd = console.currentListInd - 1
			end
		end
	end
end

function consoleKeyDownCommand()
	if #console.cmdList > 0 then
			print(tostring(console.currentListInd))
		if console.currentListInd <= #console.cmdList then
			console.txtCmdLine.value = console.cmdList[console.currentListInd]
			if console.currentListInd < #console.cmdList then
				console.currentListInd = console.currentListInd + 1
			end
		end
	end
end

function consolePrintResult(result)
  for i = 1, #result do
    if type(result[i]) ~= "nil" then
		local buf
      buf = string.format("<%dº return>", i)
      console.ind_level = 0
      consolePrintObject(buf, result[i])
    end
  end
end

function consolePrintExpression(command)

	local f = loadstring(command)

	if f then
		local result = {f()}
		consolePrintResult(result)
	else
		consoleEnterMessagef("lua: %s\n", "Invalid Expression")
	end

end

function consolePrintFuncVar(name, value)
	if name==nil or value==nil or type(name)~="string" or type(value)~="function" then
		error("consolePrintFuncVar: invalid parameters.")
	end
	
	if type(value)=="function" then
		consoleEnterMessagef("%s (C)", name)
	else
		consoleEnterMessage(name, 1)
	end
end

function consolePrintVar(var, value, table_flag)

local name
	if table_flag == nil then
		name = string.format("%s", tostring(var))
	elseif type(var) == "string" then
		name = string.format("[\"%s\"]", tostring(var))
	else
		name = string.format("[%s]", tostring(var))
	end
	consolePrintObject(name, value)
end

function consolePrintVarType(index, value, tableFlag)
	local name
	if tableFlag==nil then
		name = index..""
	elseif type(index)=="string" then
		name = "[\""..index.."\"]"
	else
		name = "["..index.."]"
	end
	consoleEnterMessagef("%s%s = \"%s\"", consoleIndent(), name, type(value))
end

function consolePrintTable(t)
  local n,v = next(t, nil)
  while n ~= nil do
    consolePrintVar(n, v, 1)
    n,v = next(t, n)
  end
end

function consoleListFuncs()
	 local global = getfenv(0)
	 local n,v = next(global, nil)
	 consoleHoldCaret(true)
	 while n ~= nil do
		if type(v) == "function" then
			consolePrintFuncVar(n, v)
		end
		n,v = next(global, n)
	end
	consoleHoldCaret(false)
end

function consoleListVars()
	local global = getfenv(0)
	local n,v = next(global, nil)
	consoleHoldCaret(true)
	while n ~= nil do
		if type(v) ~= "function" and n ~= "_G" then
			consolePrintVarType(n, v)
		end
		n,v = next(global, n)
	end
	consoleHoldCaret(false)
end

function consoleVersionInfo()
  consoleEnterMessage(
    _COPYRIGHT .. "\n"..
    "IUP " .. iup._VERSION .. "  " .. iup._COPYRIGHT .. "\n"
  )
end

function consoleClear()
  console.mtlOutput.value = ""
end

console.create()
