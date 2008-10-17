-- Creates boxes
vboxA = iup.vbox{iup.label{title="TABS A"}, iup.button{title="A"}}
vboxB = iup.vbox{iup.label{title="TABS B"}, iup.button{title="B"}}
vboxC = iup.vbox{iup.label{title="TABS C"}, iup.button{title="C"}; active = "NO"}
vboxD = iup.vbox{iup.label{title="TABS D"}, iup.button{title="D"}}
vboxE = iup.vbox{iup.label{title="TABS E"}, iup.button{title="E"};active = "NO"}
vboxF = iup.vbox{iup.label{title="TABS F"}, iup.button{title="F"} }

-- Sets titles of the vboxes
vboxA.tabtitle = "AAAAA"
vboxB.tabtitle = "BBBBB"
vboxC.tabtitle = "CCCCC"
vboxD.tabtitle = "DDDDD"
vboxE.tabtitle = "EEEEE"
vboxF.tabtitle = "FFFFF"

-- Creates tabs 
tabs = iup.tabs{vboxA, vboxB,vboxC, vboxD,vboxE, vboxF}

-- Creates dialog
dlg = iup.dialog{tabs; title="Test IupTabs", size="200x80"}

-- Shows dialog in the center of the screen
dlg:showxy(iup.CENTER, iup.CENTER)
