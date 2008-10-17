local cnv = iup.canvas{
  size = "400x400",
}

local lst = iup.list{
  value = "1",
  dropdown = "YES",
  size = "100x",
  expand = "NO",
  action = function(self, t, i, v)
    print("list action (t,i,v): ", t, i, v)
  end,
}
lst["1"] = "valor1"
lst["2"] = "valor2"
lst["3"] = nil

local function FillList()
print "FillList()"

--  lst[1] = "novo valor1"
--  lst[2] = "novo valor2"
--  lst[3] = "novo valor3"
--  lst[4] = nil

  -- Esta chamando a callback aqui!!!
  lst["1"] = nil

  iup.SetIdle(nil)
  -- Se chamar sem parametro (ver manual), o programa cai...
  --iup.SetIdle()
end

local bt = iup.button{
  title = "bt1", alignment = "ARIGHT",
  action = function(self)
--[[
    -- aqui funciona corretamente (nao chama callback de action)
    lst[1] = "novo valor1"
    lst[2] = "novo valor2"
    lst[3] = "novo valor3"
    lst[4] = nil
    lst.value = "1"
--]]
    -- Atualizando a lista na idle, da problema.
    iup.SetIdle(FillList)
  end,
}

local dlg = iup.dialog{
  iup.vbox{lst,cnv,bt}
  ; title = "Test", expand = "YES", 
  fgcolor = "0 0 0",
  bgcolor = "200 200 200",
}
-- aqui da segmentation fault..
--dlg.font = nil 
iup.Show(dlg)

