local tr1 = iup.tree{
  rastersize = "200x200",
  expand = "YES"
}
local tr2 = iup.tree{
  rastersize = "200x100",
  expand = "YES"
}
local canvas = iup.canvas{
  rastersize = "500x500",
  expand = "YES"
}
local dlg = iup.dialog{
  iup.hbox{
    iup.sbox{
      iup.vbox{
        tr1,
        iup.sbox{
          tr2,
          ; direction = "NORTH"
        },
      },
      ; direction = "EAST"
    },
    canvas,
  },
  ; title = "IupTreeTest", 
  margin = "5x5", gap = "5"
}

dlg:show()
