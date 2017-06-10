
local console = {}

if (not loadstring) then
  loadstring = load
end

function iupConsoleInit(txt_cmdline, mtl_output)
  console.cmdList = {}
  console.currentListInd = 0

  console.txtCmdLine = txt_cmdline 
  console.mtlOutput = mtl_output   
  
  console.hold_caret = false

  console.mtlOutput.value = _COPYRIGHT .. "\n" ..
                            "IUP " .. iup._VERSION .. "  " .. iup._COPYRIGHT
end


--------------------- Command History ---------------------


function iupConsoleKeyUpCommand()
  if #console.cmdList > 0 then
    if console.currentListInd >= 1 then
      console.txtCmdLine.value = console.cmdList[console.currentListInd]
      if console.currentListInd > 1 then
        console.currentListInd = console.currentListInd - 1
      end
    end
  end
end

function iupConsoleKeyDownCommand()
  if #console.cmdList > 0 then
    if console.currentListInd <= #console.cmdList then
      console.txtCmdLine.value = console.cmdList[console.currentListInd]
      if console.currentListInd < #console.cmdList then
        console.currentListInd = console.currentListInd + 1
      end
    end
  end
end

function iupConsoleEnterCommandStr(text)
  table.insert(console.cmdList, text)
  console.currentListInd = #console.cmdList
  print("> " .. text)
end

function iupConsoleEnterCommand()

  local command = console.txtCmdLine.value
  if command == nil or command == "" then
    return
  end

  iupConsoleEnterCommandStr(command)

  local cmd, msg = loadstring(command)
  if (not cmd) then
    cmd = loadstring("return " .. command)
  end
  if (not cmd) then
    print("Error: ".. msg) -- the original error message
  else
    local result = {cmd()}
    for i = 1, #result do
      iupConsolePrintValue(result[i])
    end
  end

  console.txtCmdLine.value = ""
end


--------------------- Print Replacement ---------------------


print_old = print
function print(...)
  local param = {...}
  local str = ""
  if (#param == 0) then
    str = "nil"
  else
    for i, k in ipairs(param) do 
      if (i > 1) then str = str .."\t" end
      str = str .. tostring(k)
    end
  end
  console.mtlOutput.append = str
  if (not console.hold_caret) then
    console.mtlOutput.scrollto = "99999999:1"
  end
end
write_old = io.write
io.write = function(...)
  console.mtlOutput.appendnewline="No"
  print(...)
  console.mtlOutput.appendnewline="Yes"
end

function iupConsoleValueToString(v)
  if (type(v) == "string") then
    return "\"" .. v .. "\""
  else 
    return tostring(v)
  end
end

function iupConsolePrintTable(t)
  local str = "{\n"
  local tmp = {}
  for i, k in ipairs(t) do 
    str = str .. "  "..iupConsoleValueToString(k)..",\n"
    tmp[i] = true
  end
  for i, k in pairs(t) do 
    if (not tmp[i]) then
      str = str .. "  "..tostring(i).. " = "..iupConsoleValueToString(k)..",\n"
    end
  end
  str = str .. "}"
  print(str)
end

function iupConsolePrintValue(v)
  if (type(v) == "table") then 
    iupConsolePrintTable(v)
  else
    print(iupConsoleValueToString(v))
  end
end


--------------------- Utilities ---------------------


function iupConsoleListFuncs()
  console.hold_caret = true

  local global = _G
  local n,v = next(global, nil)
  while n ~= nil do
    if type(v) == "function" then
      print(n)
    end
    n,v = next(global, n)
  end

  console.hold_caret = false
  console.mtlOutput.scrollto = "99999999:1"
end

function iupConsoleListVars()
  console.hold_caret = true

  local global = _G
  local n,v = next(global, nil)
  while n ~= nil do
    if type(v) ~= "function" and n ~= "_G" then
      print(n)
    end
    n,v = next(global, n)
  end

  console.hold_caret = false
  console.mtlOutput.scrollto = "99999999:1"
end

