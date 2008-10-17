dlg = iup.dialog
{
        iup.vbox
        {
          iup.button{title="Button Very Long Text"},
          iup.button{title="short"},
          iup.button{title="Mid Button"}
          ;size="100x"
--    ;gap="5",alignment="ARIGHT",margin="5x5"
        }
  ;title="IupDialog", font="HELVETICA_BOLD_14" 
}

dlg:show()
