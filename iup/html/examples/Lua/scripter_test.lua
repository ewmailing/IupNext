-- main

local l_main_x = 33
g_main_x = 44

for i = 1, 10 do
  l_main_x = l_main_x + 1
  g_main_x = g_main_x - 1
end

function f1(x)
  local y = x
  x = "Hello" .. x
  y = nil
  return x
end

local ret = f1("IUP")
print(ret)

