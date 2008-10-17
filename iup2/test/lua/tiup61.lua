-- No Windows, a matriz nao aparece corretamente quando o dialogo tem

-- o atributo shrink = "YES", e o diálogo nao se ajusta aos tamanhos dos

-- outros elementos.


local box1 = iup.canvas{size="300x300", tabtitle = "Canvas"}

local box2 = iup.hbox{

  -- Com esse frame, o iuplua5 sai....

  iup.frame{

    iup.matrix{

      numlin = 10,

      numcol = 3,

      expand = "VERTICAL"

    }

    ; expand = "YES"

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

  ; title = "Teste", resize = "YES", shrink = "YES", size = "200x200"

}

dlg:show()


