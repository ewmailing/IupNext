local dlg = iup.dialog
{
  iup.vbox
  {
     iup.button{title="ok"},
     iup.button{title="cancel"}
  }
  ; 
  size="300x300", 
  title = "Teste", 
  resize = "print(1)", 
--  shrink = "YES"
}
dlg:map()
dlg:show()

