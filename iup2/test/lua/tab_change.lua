-- Creates boxes
vboxA = iup.vbox{iup.label{title="TABS AAA"}, iup.button{title="AAA"}}
vboxB = iup.vbox{iup.label{title="TABS BBB"}, iup.button{title="BBB"}}

-- Sets titles of the vboxes
vboxA.tabtitle = "AAAAAA"
vboxB.tabtitle = "BBBBBB"


function tab_change(self, old_tab, new_tab)
   print("TAB_CHANGE", old_tab)
end


-- Creates tabs 
tabs = iup.tabs
{
   vboxA,
   vboxB;
   tabchange_cb = tab_change
}

-- Creates dialog
dlg = iup.dialog{tabs; title="Test IupTabs", size="200x80"}

-- Shows dialog in the center of the screen
dlg:showxy(iup.CENTER, iup.CENTER)