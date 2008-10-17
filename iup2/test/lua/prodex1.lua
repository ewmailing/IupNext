--
--  
--  vinicius@tecgraf.puc-rio.br
--

-- FUNCTION I AQUI OU NO TESTE???

function I(s)
  return s
end

----------------------- INICIALIZAÇÃO DO MENU -----------------------
local item_bloco = iup.item {title = I"Bloco"}
local item_campo = iup.item {title = I"Campo"}
local item_oport = iup.item {title = I"Oportunidade"}
local item_exit  = iup.item {title = I"Sair"}
local item_load  = iup.item {title = I"Carregar Blocos"}
local item_help  = iup.item {title = I"Ajuda"}

local menu_new    = iup.menu {item_bloco, item_campo, item_oport}
local submenu_new = iup.submenu {menu_new; title = I"Novo"}

local menu_file       = iup.menu {submenu_new, iup.separator{}, item_exit}
local menu_tools      = iup.menu {item_load}
local submenu_file    = iup.submenu {menu_file; title = I"Arquivo"}
local submenu_tools   = iup.submenu {menu_tools; title = I"Ferramentas"}

local menu = iup.menu {submenu_file, submenu_tools, item_help}

----------------------- FUNÇÕES DO MENU -----------------------
function item_bloco:action ()
   return iup.CLOSE
end 

function item_campo:action ()
   return iup.CLOSE
end
 
function item_oport:action ()
   return iup.CLOSE
end 

function item_exit:action ()
   return iup.CLOSE
end 
 
function item_load:action ()
   return iup.CLOSE
end 

function item_help:action ()
   return iup.CLOSE
end 


----------------------- CRIAÇÃO DA JANELA -----------------------
local dlg = iup.dialog
{
   iup.hbox {
--      iup.fill {size = 300}
      iup.button {title="Test", active = "NO"}
   };
   menu = menu
}

----------------------- VARIÁVEL GLOBAL -----------------------
   dlg:popup (iup.CENTER, iup.CENTER)
