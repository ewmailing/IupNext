require("iuplua")

function btn_exit_cb(self)
	-- Exits the main loop
	return iup.CLOSE	
end

label = iup.label{title = "Hello world from IUP."}
button = iup.button{title = "OK"}

vbox = iup.vbox{
	label,
	button;
	alignment = "acenter",
	gap = "10",
	margin = "10x10"
}
dlg = iup.dialog{
	vbox,
	title = "Hello World 5"
}
button.action = btn_exit_cb

dlg:showxy(iup.CENTER,iup.CENTER)

-- to be able to run this script inside another context
if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
  iup.Close()
end
