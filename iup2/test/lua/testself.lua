local mtx = iup.matrix{
  numlin = 4,
  numcol = 4,
  edition_cb = function(self, lin, col, mode)
   print("edition:", self)
  end,
}

local txt = iup.text{
  action = function(self, c, after)
    print("text:", self)
    if (not self.a) then
      self.a = 3
    print("init a")
    end
    print("text.a", self.a)
  end,
}

local dlg = iup.dialog{
  iup.vbox{mtx, txt},
  ; title = "Test Matrix",
}
dlg:popup(iup.CENTER, iup.CENTER)