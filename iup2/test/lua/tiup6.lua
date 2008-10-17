-- No Windows, a matriz nao aparece corretamente quando o dialogo tem
-- o atributo shrink = "YES", e o diálogo nao se ajusta aos tamanhos dos
-- outros elementos.

local box1 = iup.canvas{size="300x300", tabtitle = "Canvas"}
box1.bgcolor = "0 255 0"
local box2 = iup.hbox{
   iup.matrix{
     bgcolor = "255 0 0",
     numlin = 10,
     numcol = 3,
     expand = "VERTICAL"
   }
   ; tabtitle = "Matriz"
}

local dlg = iup.dialog{
  iup.vbox{
    iup.tabs{
     box1,
     box2,
     ; tabtile = "TOP"
    }
  }
  ; size="300x300", title = "Teste", resize = "YES", shrink = "YES"
}
dlg:map()
dlg:show()

