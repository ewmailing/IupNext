local menu = {}
for i = 1, 10 do
	local submenu = {}
	for i = 1, 10 do
		local subsubmenu = {}
		for i = 1, 5 do
			subsubmenu[i] = iup.item{
			  title = "MENU ITEM",
				action = function() print "I'm a menu item!" end,
			}
		end
		submenu[i] = iup.submenu{
			title = "SUBSUBMENU",
			iup.menu(subsubmenu),
		}
	end
	menu[i] = iup.submenu{
		title = "SUBMENU",
		iup.menu(submenu),
	}
end
menu = iup.menu(menu)

menu:popup(-5, -5)
menu:destroy()

if (not last) then
last = 0
end

collectgarbage("count")
local new = gcinfo()
print(new - last)
last = new
