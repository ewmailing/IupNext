

local button = iup.button {
  title = "Botão",
  active = "NO",
}
local button2 = iup.button {
  title = "Botão2",
}
local item = iup.item {
  title = "O conteúdo da célula tem que ser 'OK'",
}
local menu = iup.menu { item }

local function action()
  print("Action!")
  return iup.DEFAULT
end

button.action = action
button2.action = action
item.action = action

local dialog = iup.dialog {
  iup.hbox {
    iup.matrix {
      numcol = 1,
      numlin = 1,
      numcol_visible = 1,
      numlin_visible = 1,
      click_cb = function(self, lin, col, but)
        if iup.isbutton3(but) then
          if self[lin..":"..col] == "OK" then
            button.active = "YES"
            item.active = "YES"
          else
            button.active = "NO"
            item.active = "NO"
          end
          menu:popup(iup.MOUSEPOS, iup.MOUSEPOS)
        end
        return iup.DEFAULT
      end,
    },
    button2,
    button,
  },
}
dialog:show() 
