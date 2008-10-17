dlg = iup.dialog
{
 iup.hbox
 {
  iup.matrix
  {
    numlin=10,
    numcol=3,
    numcol_visible=3,
    height0=10,
    widthdef=30,
    scrollbar="VERTICAL",
  },
  iup.fill{},
  iup.button{title ="Esse é um teste"},
  iup.fill{},
 }
 ; title = "teste"
}
dlg:show()

