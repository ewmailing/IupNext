-- Creates boxes
vboxA = iup.vbox{iup.label{title="TABS AAA"}} vboxB = iup.vbox{iup.label{title="TABS BBB", size="100x"}}

-- Sets titles of the vboxes
vboxA.tabtitle = "AAAAAA"
vboxB.tabtitle = "BBBBBB"

-- Creates tabs
tabs = iup.tabs{vboxA, vboxB}

hbox = iup.hbox{ iup.fill{}, iup.button{title="ok"}, iup.button{title="cancel"}, iup.fill{} }

-- Creates dialog
dlg = iup.dialog{ iup.vbox{tabs, hbox}; title="Test IupTabs"}

-- Shows dialog in the center of the screen 
dlg:showxy(iup.CENTER, iup.CENTER)
