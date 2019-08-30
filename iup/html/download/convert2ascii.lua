-- Converts file encoding to pure ASCii
-- Including Comments
-- For instance: "Atenção!" would be "Aten\xE7\xE3o!". 
-- But actually will be "Aten\xE7""\xE3""o!" to properly encapsulate the hexadecinal representation
-- Solves the problem of some compilers when detecting the file encoding
-- Based on a contribution form Could Wu
-- Run this from the iup base folder
-- It will replace the original files
-- Works on Lua 5.2 or 5.3 (not working on 5.1, must check gsub behavior)
-- Usage: iup> lua convert2ascii.lua

local function toascii(c)
  local ascii = string.byte(c)
  local hex = string.format("\\x%X", ascii)
  return hex .. '"' .. ( c:sub(-1) == '"' and "" or '"')
end

local function convert(filename)
  print(filename)
	local f = assert(io.open(filename,"rb"))
	local text = f:read("*a")
	f:close()

  text = text:gsub('[\x80-\xff]"?', toascii) -- from 128 to 255
  
	f = assert(io.open(filename, "wb"))
	f:write(text)
	f:close()
end

print("Converting to ASCii:\n")

convert("src/iup_lng_portuguese.h")
convert("src/iup_lng_spanish.h")
convert("srccontrols/matrixex/iup_lng_portuguese_matrix.h")
convert("srccontrols/matrixex/iup_lng_spanish_matrix.h")
convert("srcplot/iup_lng_portuguese_plot.h")
convert("srcplot/iup_lng_spanish_plot.h")

print("\nDone")
