local dados={
                { 1,2,3 },
                { 4,5,6 },
                { 7,8,9 },
                { 10,11,12 },
                { 13,14,15 },
             }
local active={ true,true,true,true,true }
            
function mtx_valuecb(h,lin,col)
 local l=tonumber(lin)
 local c=tonumber(col)
 if l==0 and c==0 then
        return ""
 elseif l==0 then
        return "Coluna"..c
 elseif c==0 then
        return l
 else
  --print("l=",l," c=",c," dados=",dados[l][c])
  if dados[l] and dados[l][c] then
   return dados[l][c]
  else
   return ""
  end
 end
end

function mtx_value_edit(handle,lin,col,newvalue)
print("newvalue=",newvalue)
if not dados[lin] then
 dados[lin] = {}
end
dados[lin][col]=newvalue
 return IUP_DEFAULT
end

function mtx_edition(h,l,c)
 if  c==1 then
        return IUP_IGNORE
 end
 return IUP_DEFAULT
end


local item_insert_before = iupitem{title="Inserir antes",}
local item_insert_after  = iupitem{title="Inserir depois",}
local item_remove        = iupitem{title="Remover",}
local item_clear         = iupitem{title="Limpar",}
local item_active        = iupitem{title="Inativar",}

local mtx 
local popupmenu=iupmenu{
                 item_insert_before,
                 item_insert_after,
                 item_remove,
                 item_clear;
                 action=function(self)
                         
                        end
                 }
                              
mtx = 
      iupmatrix{
             numcol = 3,
             numcol_visible = 3,
             numlin = 5,
             numlin_visible = 5,
             seltype = "SELLIN",
             valuecb=mtx_valuecb,
             popup=popupmenu,
             value_edit=mtx_value_edit,
             edition=mtx_edition
            }

mtx["WIDTH0"] = 10
mtx["HEIGHT0"] = 10
mtx.scrollbar = "YES"
function item_insert_before:action()
 
end

function item_insert_after:action()
 
end            

function item_remove:action()
 
end

function item_clear:action()
 
end

function item_active:action()
 
end

local file_menu = iupmenu{iupitem{title="Sair"}}
local menubar = iupmenu{iupsubmenu{file_menu;title="Arquivo"}}
local lb = iuplabel{title="seila"} --glbcolor.Create{color="white"}
local dlg = iupdialog{
                iupvbox{
                 iupfill{size=10},
                 iupframe{mtx},
                 iupfill{size=10},
                 iuphbox{
                 lb,
                 iupbutton{title="Sel",action=function()
                                       
                                        end
                          }
                 },
                 iupfill{size=10},
                 iuptext{value="AAAAA",size=100},
                };
                title="teste",
                menu=menubar
            }


dlg:show()