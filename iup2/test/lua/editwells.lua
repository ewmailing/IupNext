local btn_visible
local btn_n_visible
local btn_color_well
local btn_color_comp
local btn_none
local btn_all
local matrix

local load_colors

local t -- the current data table
local t_line   --data table where the line values are stored

local click = function(self,l,c,r)
   --local n, sel = gmatrix.MarkedLines(matrix)
   if n==0 then
      btn_visible.active = "NO"
      btn_n_visible.active = "NO"
      btn_color_well.active = "NO"
      btn_color_comp.active = "NO"
      btn_none.active = "NO"
      btn_all.active = "YES"
   elseif n==matrix.numlin then
      btn_visible.active = "YES"
      btn_n_visible.active = "YES"
      btn_color_well.active = "YES"
      btn_color_comp.active = "YES"
      btn_none.active = "YES"
      btn_all.active = "NO"
   else
      btn_visible.active = "YES"
      btn_n_visible.active = "YES"
      btn_color_well.active = "YES"
      btn_color_comp.active = "YES"
      btn_none.active = "YES"
      btn_all.active = "YES"
   end
   load_colors()
end


btn_visible = iup.button{title = I"Visível" , size = 60}
btn_n_visible = iup.button{title = I"Não Visível"}
btn_color_well = iup.button{title = I"Cor Poço"}
btn_color_comp = iup.button{title = I"Cor Completação"}
btn_all = iup.button{title = I"Todos"}
btn_none = iup.button{title = I"Nenhum"}
local btn_invert = iup.button{title = I"Inverter"}
local btn_ok = Config:GetOKButton()
local btn_cancel = Config:GetCancelButton()
local btn_apply = iup.button{title=I"Aplicar",size=60}

matrix = --gmatrix.Create{ 
         iup.matrix{ HEIGTH0 = 10, numcol = 4 , numlin = 10 , numcol_visible = 4 , numlin_visible = 10,
                            width1 = 80 , width2 = 50 , width3 = 50 , width4 = 50 , click_cb = click,
                            alignment1 = "ALEFT" , alignment2 = "ACENTER"
}

matrix['SELON*:3'] = 'NO'
matrix:setcell(0,1,I"Poço")
matrix:setcell(0,2,I"Visível")
matrix:setcell(0,3,I"Poço")
matrix:setcell(0,4,I"Completação")


-- well name font / font size
local well_font = iup.list{'';multiple="NO", dropdown="YES", font=Config:GetDropDownListFont(), bgcolor='255 255 255'}
local well_font_size = iup.list{'';multiple="NO", dropdown="YES", font=Config:GetDropDownListFont(), bgcolor='255 255 255'}
local well_font_styles = iup.list{'';multiple="NO", dropdown="YES", font=Config:GetDropDownListFont(), bgcolor='255 255 255'}

local s_update_table = function()
  local font = well_font[tonumber(well_font.value)]
  local size = tonumber(well_font_size[tonumber(well_font_size.value)])
  local styles = well_font_styles[tonumber(well_font_styles.value)]
  local bold = (styles == I'Negrito' or styles == I'Negrito/Itálico')
  local italic = (styles == I'Itálico' or styles == I'Negrito/Itálico')
  t.name_font = font
  t.name_font_size = size
  t.name_font_bold = bold
  t.name_font_italic = italic
end

well_font.action = function(self, text, item, select)
  if select == 0 then
    return
  end
  local fntsizelist = Config:GetGlutFontsSizes()[text]
  for k,v in ipairs(fntsizelist) do
    well_font_size[k] = v
    well_font_size[k+1] = nil
  end
  local fntstylelist = Config:GetGlutFontsStyles()[text]
  for k,v in ipairs(fntstylelist) do
    well_font_styles[k] = v
    well_font_styles[k+1] = nil
  end
  well_font_size.value = 1
  well_font_styles.value = 1
  well_font_styles.active = table.getn(fntstylelist) > 1 and "YES" or "NO"
  s_update_table()
end

well_font_size.action = function(self, text, item, select)
  s_update_table()
end

well_font_styles.action = function(self, text, item, select)
  s_update_table()
end

local dialog = giup.dialog{
  iup.frame{
    iup.vbox{
      iup.frame{
        iup.hbox{
          iup.vbox{
            iup.hbox{
              iup.fill{size = 14},
              iup.frame{matrix}
            },
            iup.hbox{
              iup.fill{},
              btn_all,
              btn_none,
              btn_invert,
              iup.fill{}
              ;
              alignment = "ACENTER",
              gap=10,
            }
          },
          iup.fill{size=14},
          iup.vbox{
            iup.fill{size=2},
            btn_visible,
            btn_n_visible,
            btn_color_well,
            btn_color_comp,
            iup.fill{size=2}, 
            iup.vbox{
              iup.hbox{
                iup.label{
                  title=I"Fonte:",
                  size = "40x",
                },
                iup.fill{size=2},
                well_font,
                ;
              },
              iup.hbox{
                iup.label{
                  title=I"Tamanho:",
                  size = "40x",
                },
                iup.fill{size=2},
                well_font_size,
                ;
              },
              iup.hbox{
                iup.label{
                  title=I"Estilo:",
                  size = "40x",
                },
                iup.fill{size=2},
                well_font_styles,
                ;
              },
              ;
              alignment = "ALEFT",
            },
            ;
            gap=5,
            alignment = "ACENTER",
          },
          iup.fill{size=14}
        }
        ;
        margin = "2x2"
      },
      iup.fill{size=5},
      iup.frame{
        iup.hbox{
          iup.fill{},
          btn_ok,
          btn_cancel,
          btn_apply
          ;
          alignment = "ACENTER",
          gap=10,
        }
        ;
        margin = "2x2",
      },
      iup.fill{size=5},
    }
    ;
    margin = "2x2"
  }
  ;
  title = I"Edição de Poços"
}


dialog:map()
local tabela1 ={btn_visible,btn_n_visible,btn_color_well,btn_color_comp,btn_all,btn_none,btn_invert,btn_ok,btn_cancel}
gsize.Set (tabela1)


function load_colors()
   for k,v in ipairs(t) do
      matrix["BGCOLOR"..tostring(k)..":".."3"] = tostring((t[k].r*255)).." "..tostring((t[k].g*255)).." "..tostring((t[k].b*255))
      matrix["BGCOLOR"..tostring(k)..":".."4"] = tostring((t[k].cr*255)).." "..tostring((t[k].cg*255)).." "..tostring((t[k].cb*255))
   end
end


function matrix:edition_cb(l,c,mode)
   return iup.IGNORE
end


function matrix:scroll_cb(lin,col)
   matrix.redar = 'ALL'
   --matrix.redraw = 'ALL'
end


local load = function(t)
   btn_visible.active = "NO"
   btn_n_visible.active = "NO"
   btn_color_well.active = "NO"
   btn_color_comp.active = "NO"
   btn_none.active = "NO"
   btn_all.active = "YES"
   
   local i=0
   for k,v in ipairs(t) do
      matrix:setcell(k,1,t[k].name)
      if t[k].visible then
         matrix:setcell(k,2,I"Sim")
      else
         matrix:setcell(k,2,I"Não")
      end
      matrix["BGCOLOR"..tostring(k)..":".."3"] = tostring((t[k].r*255)).." "..tostring((t[k].g*255)).." "..tostring((t[k].b*255))
      matrix["BGCOLOR"..tostring(k)..":".."4"] = tostring((t[k].cr*255)).." "..tostring((t[k].cg*255)).." "..tostring((t[k].cb*255))
      i = i+1
   end
   
	 --[[
   matrix.numlin = i
   matrix.sm.numlin = i
   if i<=10 then
      matrix.numlin_visible=i
   else
      matrix.numlin_visible=10
   end
	 --]]
end


local ok = true
local apply_func

function btn_ok:action()
   ok = true
   return iup.CLOSE
end


function btn_cancel:action()
   ok = false
   return iup.CLOSE
end


function btn_apply:action()
   apply_func(t)
end

function btn_all:action()
   --gmatrix.MarkAllLines(matrix)
   btn_visible.active = "YES"
   btn_n_visible.active = "YES"
   btn_color_well.active = "YES"
   btn_color_comp.active = "YES"
   btn_all.active= "NO"
   btn_none.active = "YES"
   load_colors()
   --matrix.redraw='ALL'
end


function btn_none:action()
   print'unmark all lines'
   --gmatrix.UnmarkAllLines(matrix)
   btn_visible.active = "NO"
   btn_n_visible.active = "NO"
   btn_color_well.active = "NO"
   btn_color_comp.active = "NO"
   btn_all.active= "YES"
   btn_none.active = "NO"
   load_colors()
end


function btn_invert:action()
   local i,sel
   --i,sel=gmatrix.MarkedLines(matrix)
   if i==tonumber(matrix.numlin) then      --todas as linhas marcadas
      btn_visible.active = "NO"
      btn_n_visible.active = "NO"
      btn_color_well.active = "NO"
      btn_color_comp.active = "NO"
      btn_all.active= "YES"
      btn_none.active = "NO"
   else
      --antes do gmatrix.InvertSelection era feito assim
      --local c=1
      --for j=1,matrix.numlin do
      --   if sel[c]==j then
      --      gmatrix.UnMarkLin(matrix,j)
      --      c=c+1
      --   else
      --      gmatrix.MarkLin(matrix,j)
      --   end
      --end
      if i==0 then               --nenhuma linha marcada
         btn_visible.active = "YES"
         btn_n_visible.active = "YES"
         btn_color_well.active = "YES"
         btn_color_comp.active = "YES"
         btn_all.active= "NO"
         btn_none.active = "YES"
      else
         btn_visible.active = "YES"
         btn_n_visible.active = "YES"
         btn_color_well.active = "YES"
         btn_color_comp.active = "YES"
         btn_all.active = "YES"
         btn_none.active = "YES"
      end
   end
   --gmatrix.InvertSelection(matrix)
   load_colors()
end


function btn_color_well:action()
   local i, sel
   --i e o numero de linhas selecionadas
   --sel[j]=l é igual ao número da linha marcada
   --i , sel= gmatrix.MarkedLines(matrix)
   local r, g, b
   if not sel[1] then
   else
      r, g, b = iup.GetColor(iup.CENTER, iup.CENTER, t[sel[1]].r * 255, t[sel[1]].g * 255, t[sel[1]].b * 255)
   end
   
   if not r then
   else
      for j=1,i do
         matrix["BGCOLOR"..tostring(sel[j])..":".."3"] = tostring(r).." "..tostring(g).." "..tostring(b)
         t[sel[j]].r=r/255
         t[sel[j]].g=g/255
         t[sel[j]].b=b/255
      end
   end
end


function btn_color_comp:action()
   local i, sel
   --i e o numero de linhas selecionadas
   --sel[j]=l é igual ao número da linha marcada
   --i , sel= gmatrix.MarkedLines(matrix)
   local r, g, b
   if not sel[1] then
   else
      r, g, b = iup.GetColor(iup.CENTER, iup.CENTER, t[sel[1]].cr * 255, t[sel[1]].cg * 255, t[sel[1]].cb * 255)
   end
   
   if not r then
   else
      for j=1,i do
         matrix["BGCOLOR"..tostring(sel[j])..":".."4"] = tostring(r).." "..tostring(g).." "..tostring(b)
         t[sel[j]].cr=r/255
         t[sel[j]].cg=g/255
         t[sel[j]].cb=b/255
      end
   end
end


function btn_visible:action()
   local i, sel
   --i,sel=gmatrix.MarkedLines(matrix)
   
   if not sel[1] then
   else
      for j=1,i do
         matrix:setcell(sel[j],2,I"Sim")
         t[sel[j]].visible=true
      end
   end
end


function btn_n_visible:action()
   local i, sel
   --i,sel=gmatrix.MarkedLines(matrix)
   
   if not sel[1] then
   else
      for j=1,i do
         matrix:setcell(sel[j],2,I"Não")
         t[sel[j]].visible=false
      end
   end
end


return function(tbl,apply)
  local fntlist = Config:GetGlutFontsDesc()
  local font = tbl.name_font
  local font_size = tbl.name_font_size
  local font_bold = tbl.name_font_bold
  local font_italic = tbl.name_font_italic
  local font_style
  if font_bold then
    if font_italic then
      font_style = I'Negrito/Itálico'
    else
      font_style = I'Negrito'
    end
  else
    if font_italic then
      font_style = I'Itálico'
    else
      font_style = I'Normal'
    end
  end
  for k,v in ipairs(fntlist) do
    well_font[k] = v
    well_font[k+1] = nil
    if font == v then
      well_font.value = k
    end
  end
  local fntsizelist = Config:GetGlutFontsSizes()[font]
  for k,v in ipairs(fntsizelist) do
    well_font_size[k] = v
    well_font_size[k+1] = nil
    if font_size == v then
      well_font_size.value = k
    end
  end
  local fntstylelist = Config:GetGlutFontsStyles()[font]
  for k,v in ipairs(fntstylelist) do
    well_font_styles[k] = v
    well_font_styles[k+1] = nil
    if font_style == v then
      well_font_styles.value = k
    end
  end
  well_font_styles.active = table.getn(fntstylelist) > 1 and "YES" or "NO"
  --gmatrix.SetNumLines(matrix, table.getn(tbl))
  t = tbl
  apply_func = apply
  --gmatrix.UnmarkAllLines(matrix)
  load(t)
  --aqui entra a definicao do dialogo que esta acima nessa janela
  dialog.size = nil
  dialog:popup(iup.CENTER,iup.CENTER)
  return ok
end

