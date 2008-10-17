text = iup.multiline {
 size = "100x40",
 expand = "YES",
 nc = 256,
 action = function(self,c,after)
   print("after:", after)
 end,
}

dlgMulti = iup.dialog {
  iup.vbox{ text },  
  ; title = "Teste de multiline"
}

dlgMulti:show()
