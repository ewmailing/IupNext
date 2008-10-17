-- Creates boxes
vboxA = iup.vbox{iup.label{title="TABS AAA",font="COURIER_NORMAL_14"}, iup.button{title="AAA"}} 
vboxB = iup.vbox{iup.label{title="TABS BBB"}, iup.button{title="BBB"}}

-- Sets titles of the vboxes
vboxA.tabtitle = "AAAAAA"
vboxB.tabtitle = "BBBBBB"

-- Creates tabs
tabs = iup.tabs{vboxA, vboxB}

-- Creates dialog
dlg = iup.dialog{tabs; title="Test IupTabs", size="200x80"}

-- Shows dialog in the center of the screen dlg:showxy(iup.CENTER, iup.CENTER)
dlg:show()