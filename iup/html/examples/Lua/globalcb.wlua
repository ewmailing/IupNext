
function gbutton(but, press, x, y, status)
  print(but, press, x, y, status)
end

iup.SetGlobal("INPUTCALLBACKS", "Yes")
iup.SetGlobalCallback("GLOBALBUTTON_CB", gbutton)
