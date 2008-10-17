-------------------------------------------------------------------------------
--                              IupMatrix example                            -- 
--        Author : Claudio Coutinho de Biasi                                 -- 
--   Description : Creates a matrix with two columns and three lines         --
--                 The dialog in which the matrix is located has an          --
--                 associated menu that allows changing titles, dimensions,  --
--                 colors, alignment, marking mode and edition mode, as well --
--                 as inserting or removing a line or column                 --
-------------------------------------------------------------------------------

-- number of columns and lines in the matrix
col = 2
lin = 3
-- maximum number of characters in the dimension and title dialogs
dimension_limit = 2
title_limit = 20

-- initializing elements

-- creating elements in the title menu
get_value_cancel = iupbutton { title = "Cancel" }
get_value_ok = iupbutton { title = "OK" }
get_value_text = iuptext { expand = IUP_YES, border = IUP_YES }
get_value = iupdialog { iupvbox{ get_value_text, get_value_ok, get_value_cancel; margin="10x10", gap=10} } 

-- initializing menu
-- creating the items that compose the ‘titles’ menu
titlegeneral = iupitem { title = "General" }
titlecol = iupitem { title = "Column" }
titlelin = iupitem { title = "Line" }

-- creating the items that compose the ‘add’ menu
addcolleft = iupitem{ title = "Column on the left" }
addcolright = iupitem{ title = "Column on the right" }
addlinabove = iupitem{ title = "Line above" }
addlinbelow = iupitem{ title = "Line below" }

-- creating the items that compose the ‘delete’ menu
deletecolleft = iupitem{ title = "Column on the left" }
deletecolright = iupitem{ title = "Column on the right" }
deletelinabove = iupitem{ title = "Line above" }
deletelinbelow = iupitem{ title = "Line below" }

-- creating the items that compose the ‘dimensions’ menu
height = iupitem{ title = "Height" }
width = iupitem{ title = "Width" }

-- creating the items that compose the ‘alignment’ menu
alinleft = iupitem{ title = "Left" }
alincent = iupitem{ title = "Centered" }
alinright = iupitem{ title = "Right" }

-- creating the items that compose the ‘mark’ menu
mark_multipla = iupitem{ title = "Mark multiple" }
mark_continua = iupitem{ title = "Mark continuous", value = IUP_ON }
size_editable = iupitem{ title = "Size editable" }

-- creating the items in the ‘color foreground’ menu
color_foreground_red = iupitem{ title = "Red" }
color_foreground_green = iupitem{ title = "Green" }
color_foreground_blue = iupitem{ title = "Blue" }
color_foreground_black = iupitem{ title = "Black" }
color_foreground_white = iupitem{ title = "White" }

-- creating the items in the ‘color background’ menu
color_background_red = iupitem{ title = "Red" }
color_background_green = iupitem{ title = "Green" }
color_background_blue = iupitem{ title = "Blue" }
color_background_black = iupitem{ title = "Black" }
color_background_white = iupitem{ title = "White" }

-- creating the ‘color foreground’ submenu
color_foreground_menu = iupmenu { color_foreground_red, color_foreground_green, 
  color_foreground_blue, color_foreground_black, color_foreground_white }
color_foreground = iupsubmenu{ color_foreground_menu ; title = "Foreground"}

-- creating the ‘color background’ submenu
color_background_menu = iupmenu {color_background_red, color_background_green, 
  color_background_blue, color_background_black, color_background_white}
color_background = iupsubmenu{ color_background_menu ; title = "Background"}

-- creating the ‘titles’ submenu
titles_menu = iupmenu { titlegeneral, titlecol, titlelin }
titles = iupsubmenu{ titles_menu ; title = "Titles" }

-- creating the ‘add’ submenu
add_menu = iupmenu { addcolleft, addcolright, addlinabove, addlinbelow }
add = iupsubmenu{ add_menu ; title = "Add" }

-- creating the ‘delete’ submenu
delete_menu = iupmenu { deletecolleft, deletecolright, deletelinabove, deletelinbelow }
delete = iupsubmenu{ delete_menu ; title = "Delete" }

-- creating the ‘dimensions’ submenu
dimensions_menu = iupmenu { height, width }
dimensions = iupsubmenu { dimensions_menu ; title = "Dimensions" }

-- creating the ‘alignment’ submenu
alignment_menu = iupmenu { alinleft, alincent, alinright }
alignment = iupsubmenu { alignment_menu ; title = "Alignment" }

-- creating the ‘mark’ submenu
configuration_menu = iupmenu { mark_multipla, mark_continua, size_editable }
configuration = iupsubmenu { configuration_menu ; title = "Configuration" }

-- creating the ‘change_color’ submenu
change_color_menu = iupmenu { color_foreground, color_background }
change_color_sbm = iupsubmenu { change_color_menu ; title = "Color" }

value_item = iupitem{ title = "Value" }
redraw_item = iupitem{ title = "Redraw" }

-- creating the ‘change’ menu
change_menu = iupmenu { value_item, titles, add, delete, dimensions, alignment, change_color_sbm, configuration, redraw_item }

-- creating the bar menu
change_submenu = iupsubmenu { change_menu ; title = "Change" }
main_menu = iupmenu { change_submenu }

-- initializing matrix
mtrx = iupmatrix{ numcol = 2, numlin = 3, numcol_visible = 2, numlin_visible = 3, mark_mode = "CELL" }
IupSetAttribute(mtrx,"0:0","Inflation")
IupSetAttribute(mtrx,"1:0","Medicine")
IupSetAttribute(mtrx,"2:0","Food")
IupSetAttribute(mtrx,"3:0","Energy")
IupSetAttribute(mtrx,"0:1","January 2000")
IupSetAttribute(mtrx,"0:2","February 2000")
IupSetAttribute(mtrx,"1:1",5.6)
IupSetAttribute(mtrx,"2:1",2.2)
IupSetAttribute(mtrx,"3:1",7.2)
IupSetAttribute(mtrx,"1:2",4.5)
IupSetAttribute(mtrx,"2:2",8.1)
IupSetAttribute(mtrx,"3:2",3.4)

-- places matrix inside a frame
moldura = iupframe { mtrx }

-- places matrix in the dialog
dlg1 = iupdialog{moldura ; title = "IupMatrix" } 
dlg2 = iupdialog{iuplabel{title="(use menu to change the matrix)"} ; title = "IupMatrix", menu = main_menu } 

-- shows the dialog
dlg1:showxy(600,200)
dlg2:showxy(200,200)

-- callback of the cancel button
function get_value_cancel:action()
  get_value.value = nil 
  return IUP_CLOSE
end

-- callback of the ok button in the title dialog
function get_value_ok:action()
  get_value.value = get_value_text.value 
  return IUP_CLOSE
end

-- function that changes the title
function change_title(mode)
  if mode == -1 then
    get_value_text.value = IupGetAttribute(mtrx,"0:0")
    get_value.title = "Title - 0:0"
    get_value:popup(IUP_CENTER,IUP_CENTER)
    if get_value.value ~= nil then
      IupSetAttribute(mtrx,"0:0", get_value.value)
    end
  elseif mtrx.marked == nil then
    IupMessage("Change title","There are no marked cells")
  else
    local i = 1
    local c = 1 
        
    if mode == 0 then
      get_value.title = "Title - Column"
    else
      get_value.title = "Title - Line"
    end
    get_value_text.value = ""
    get_value:popup(IUP_CENTER,IUP_CENTER)

    if get_value.value ~= nil then
      marked = mtrx.marked
      
      while i ~= nil do
        i = strfind(marked,1,i)
        if i ~= nil then
          if mode == 0 then
            str = "0:"..(i - col*floor(i/lin))	
          else
            str = (floor(i/(col+1))+1)..":0"
          end
            
          IupSetAttribute(mtrx, str, get_value.value)
          i = i + 1
        end
      end  
    end  
  end
end

-- callback that changes the overall title
function titlegeneral:action()
  change_title(-1)
  return IUP_DEFAULT
end

-- callback that changes the column title
function titlecol:action()
  change_title(0)
  return IUP_DEFAULT
end

-- callback that changes the line title
function titlelin:action()
  change_title(1) 
  return IUP_DEFAULT
end

function redraw_item:action()
  mtrx.redraw = "YES"
  return IUP_DEFAULT
end

function value_item:action()
  get_value_text.value = mtrx.value
  get_value.title = "Value"
  get_value:popup(IUP_CENTER,IUP_CENTER)
  if get_value.value ~= nil then
    mtrx.value = get_value.value
  end
  return IUP_DEFAULT
end

-- function that changes the column
function changecol(mode, pos)
  if mtrx.marked == nil then 
    IupMessage("Change column","There are no marked cells")
  elseif mode == "DELCOL" and col == 1 then
    IupMessage("Change column","Last column cannot be deleted")
  else
    local marked = mtrx.marked
    local i = strfind(marked,1,1)
    local c = i - col*floor(i/lin) + pos - 1
        
    if(mode == "DELCOL" and ((pos == 0 and c == 0) or (pos == 1 and c == col))) then
      IupMessage("Change column","Column out of matrix")
    else
      IupSetAttribute(mtrx,mode,c)
      if mode == "ADDCOL" then
        col = col + 1
 	    else
        col = col - 1
      end
    end
  end
end

-- callback that adds columns to the left
function addcolleft:action()
  changecol("ADDCOL", 0)
  return IUP_DEFAULT
end

-- callback that adds columns to the right
function addcolright:action()
  changecol("ADDCOL", 1)
  return IUP_DEFAULT
end

-- callback that deletes columns to the left
function deletecolleft:action()
  changecol("DELCOL", 0)
  return IUP_DEFAULT
end

-- callback that deletes columns to the right
function deletecolright:action()
  changecol("DELCOL", 1)
  return IUP_DEFAULT
end

-- function that changes the line
function changelin(mode, pos)
  if mtrx.marked == nil then
    IupMessage("Change line","There are no marked cells")
  elseif mode == "DELLIN" and lin == 1 then
    IupMessage("Change line","Last line cannot be deleted")
  else
    local marked = mtrx.marked
    local i = strfind(marked,1,1)
    local l = floor(i/(col+1))+pos

    if(mode == "DELLIN" and ((pos == 0 and l == 0) or (pos == 1 and l == lin))) then
      IupMessage("Change line","Line out of matrix")
    else
	    IupSetAttribute(mtrx,mode,l)
      if mode == "ADDLIN" then
	      lin = lin + 1
      else
        lin = lin - 1
      end
    end
  end
end

-- callback that adds lines above
function addlinabove:action()
  changelin("ADDLIN", 0)
  return IUP_DEFAULT
end

-- callback that adds lines below
function addlinbelow:action()
  changelin("ADDLIN", 1)
  return IUP_DEFAULT
end

-- callback that deletes lines above
function deletelinabove:action()
  changelin("DELLIN", 0)
  return IUP_DEFAULT
end

-- callback that deletes lines below
function deletelinbelow:action()
  changelin("DELLIN", 1)
  return IUP_DEFAULT
end

-- function that changes the dimension
function change_dimension(mode)
  if mtrx.marked == nil then
    IupMessage("Change dimension","There are no marked cells")
  else
    local i = 1
    local c = 1
        
    if mode == "HEIGHT" then
      get_value.title = "Height"
    else
      get_value.title = "Width"
    end
    get_value_text.value = ""
    get_value:popup(IUP_CENTER,IUP_CENTER)
    
    if get_value.value ~= nil then
      marked = mtrx.marked
       
      while i ~= nil do  
        i = strfind(marked, 1, i)
        if i ~= nil then
          c = i - col*floor(i/lin)
          if mode == "HEIGHT" then
            str = mode..(floor(i/(col+1))+1)
          else
            str = mode..c
          end
          IupSetAttribute(mtrx, str, get_value.value)
          i = i + 1
        end
      end
    end
  end
end

-- callback that changes the height of a cell
function height:action()
  change_dimension("HEIGHT")
  return IUP_DEFAULT
end

-- callback that changes the width of a cell
function width:action()
  change_dimension("WIDTH")
  return IUP_DEFAULT
end

-- callback that changes the alignment of a column
function alin(pos)
  if mtrx.marked == nil then
    IupMessage("Alignment","There are no marked cells")
  else
    local i = 1
    marked = mtrx.marked
    
    while i ~= nil do
      i = strfind(marked,1,i)
      if i ~= nil then
        local c = i - col*floor(i/lin)
        str = "ALIGNMENT"..c
        IupSetAttribute(mtrx,str,pos);
        i = i + 1
      end
    end
  end
end

-- callback that sets the alignment of a column to the left
function alinleft:action()
  alin("ALEFT")
  return IUP_DEFAULT
end

-- callback that sets the alignment of a column to the center
function alincent:action()
  alin("ACENTER")
  return IUP_DEFAULT
end

-- callback that sets the alignment of a column to the right
function alinright:action()
  alin("ARIGHT")
  return IUP_DEFAULT
end

-- callback that shifts between continuous and non-continuous marking
function mark_continua:action()
  if mtrx.area == "CONTINUOUS" then
    mark_continua.value = IUP_OFF
    mtrx.area = "NOT_CONTINUOUS"
  else
    mark_continua.value = IUP_ON
    mtrx.area = "CONTINUOUS"
  end
  return IUP_DEFAULT
end

-- callback that shifts between multiple and single marking
function mark_multipla:action()
  if mtrx.multiple == IUP_YES then
    mtrx.multiple = IUP_NO
    mark_multipla.value = IUP_OFF
  else
    mtrx.multiple = IUP_YES
    mark_multipla.value = IUP_ON
  end
  return IUP_DEFAULT
end

-- callback that shifts between editable and non-editable size
function size_editable:action()
  if mtrx.resizematrix == IUP_YES then
    mtrx.resizematrix = IUP_NO
    size_editable.value = IUP_OFF
  else
    mtrx.resizematrix = IUP_YES
    size_editable = IUP_ON
  end
  return IUP_DEFAULT
end

-- callback that shifts the color of selected cells
function change_color(mode,color)
  if mtrx.marked == nil then
    IupMessage("Change color","There are no marked cells")
  else
    local i = 1
    
    marked = mtrx.marked
            
    while i ~= nil do
      i = strfind(marked,1,i)
          
      if i ~= nil then        
        local c = i - col*floor(i/lin)
        local l = (floor(i/(col+1))+1)
        local str = mode..l..":"..c
        IupSetAttribute(mtrx, str, color)
        i = i + 1
      end
    end
  end
end

-- callback that shifts the foreground color to red
function color_foreground_red:action()
  change_color("FGCOLOR","255 0 0")
  return IUP_DEFAULT
end
 
-- callback that shifts the foreground color to green
function color_foreground_green:action()
  change_color("FGCOLOR","0 255 0")
  return IUP_DEFAULT
end

-- callback that shifts the foreground color to blue
function color_foreground_blue:action()
  change_color("FGCOLOR","0 0 255")
  return IUP_DEFAULT
end

-- callback that shifts the foreground color to black
function color_foreground_black_cb()
  change_color("FGCOLOR","0 0 0")
  return IUP_DEFAULT
end

-- callback that shifts the foreground color to white
function color_foreground_white:action()
  change_color("FGCOLOR","255 255 255")
  return IUP_DEFAULT
end

-- callback that shifts the background color to red
function color_background_red:action()
  change_color("BGCOLOR","255 0 0")
  return IUP_DEFAULT
end

-- callback that shifts the background color to green
function color_background_green:action(void)
  change_color("BGCOLOR","0 255 0")
  return IUP_DEFAULT
end

-- callback that shifts the background color to blue
function color_background_blue:action()
  change_color("BGCOLOR","0 0 255")
  return IUP_DEFAULT
end

-- callback that shifts the background color to black
function color_background_black:action()
  change_color("BGCOLOR","0 0 0")
  return IUP_DEFAULT
end

-- callback that shifts the background color to white
function color_background_white:action()
  change_color("FGCOLOR","255 255 255")
  return IUP_DEFAULT
end
