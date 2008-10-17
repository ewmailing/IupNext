m=iup.matrix{
  numlin = 2,
  numcol = 3,
  drop_cb = function (self, drop, lin,col)
    if col == 1  and lin == 2 then
      drop[1] = "A" 
      drop[2] = "B"
      return iup.DEFAULT
    else
      return iup.IGNORE
    end
  end,
  dropselect_cb = function (self,lin,col,drop,t,i,v)
    print("dropselect_cb(",lin,col,drop,t,i,v,")")
    return iup.DEFAULT
  end,
}
dlg=iup.dialog{m;title="teste"}
dlg:show()
