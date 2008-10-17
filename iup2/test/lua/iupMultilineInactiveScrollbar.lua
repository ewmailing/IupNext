-- Iup Multiline Inactive Scrollbar Bug


mline = iup.multiline{
  size = "1x1",
  expand = "YES"
}

n_text = iup.text{
  value = "10",
  expand = "NO",
  rastersize = "32x"
}

writeNLines_bttn = iup.button{
  title = "Escrever Linhas",
  action = function(self)
    n = tonumber(n_text.value)
    for i = 1, n do
      mline.append = "linha "..tostring(i).." de "..tostring(n)
    end
  end
}

clearLines_bttn = iup.button{
  title = "Limpar",
  action = function(self)
    mline.value = ""
  end
}

tstBox = iup.hbox{
  clearLines_bttn,
  iup.fill{},
  n_text, writeNLines_bttn;
  alignment = "ACENTER"
}

dlg = iup.dialog
{
  iup.vbox{
    tstBox,
    mline,
  };
  rastersize = "400x300",
  title="Multiline Bug",
}

dlg:show()

iup.MainLoop()




