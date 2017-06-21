-- main

local l_main_x = 33
g_main_x = 44

local t = {1, "xx"}

for i = 1, 10 do
  l_main_x = l_main_x + 1
  g_main_x = g_main_x - 1
end

local function f2(z)
  z = z + 1
  return z
end

function f1(x)
  local y = x
  x = "Hello " .. x 
  y = false
  y = f2(3)
  return x .. " " .. y
end

function f3()
  g_main_x = g_main_x + 50
  l_main_x = l_main_x + 5
end

local ret = f1("IUP")
print(ret)

f3()

g_main_x = g_main_x + 1

