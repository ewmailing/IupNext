x1 = iup.timer{
	time = 1000,
	action_cb = function() print("lost") end,
}

x1.run = "YES"

mine = iup.timer{
	time = 1000,
	action_cb = function() print("mine") end,
}

local dlg = iup.dialog {
	iup.hbox {
		iup.button {
			title = "Start Timer",
			action = function() mine.run = "YES" end,
		},
		iup.button {
			title = "Stop Timer",
			action = function() mine.run = "NO" end,
		},
	},
}

dlg:show()
