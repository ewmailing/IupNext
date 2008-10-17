--IupZbox Example in IupLua 
--An application of a zbox could be a program requesting several entries from the user according to a previous selection. In this example, a list of possible layouts ,each one consisting of an element, is presented, and according to the selected option the dialog below the list is changed. 


-- IupZbox Example 


fill = iupfill {}
text = iuptext {value = "Enter your text here", expand = "YES"}
lbl  = iuplabel {title = "This element is a label"}
btn  = iupbutton {title = "This button does nothing"}


mat = iupmatrix {numcol=2, numlin=3,numcol_visible=2, numlin_visible=3, widthdef=34}
mat:setcell(0,0,"Inflation")
mat:setcell(1,0,"Medicine")
mat:setcell(2,0,"Food")
mat:setcell(3,0,"Energy")
mat:setcell(0,1,"January 2000")
mat:setcell(0,2,"February 2000")
mat:setcell(1,1,"5.6")
mat:setcell(2,1,"2.2")
mat:setcell(3,1,"7.2")
mat:setcell(1,2,"4.6")
mat:setcell(2,2,"1.3")
mat:setcell(3,2,"1.4")


zbox = iupzbox
{
  fill,
  text,
  lbl,
  btn,
  mat;
  alignment = IUP_ACENTER, value=text
}


list = iuplist { "fill", "text", "lbl", "btn", "mat"; value="2"}
ilist = {fill, text, lbl, btn, mat}


function list:action (t, o, selected)
  if selected == 1 then
    -- Sets the value of the zbox to the selected element 
    zbox.value=ilist[o]
  end
  
  return IUP_DEFAULT
end


frm = iupframe
{
  iuphbox
  {
    iupfill{},
    list,
    iupfill{}
  } ;
  title = "Select an element"
}


dlg = iupdialog
{
  iupvbox
  {
    frm,
    iupframe
    {
      zbox;
      title = "Elements"
    }
  } ;
  size = "QUARTER",
  title = "IupZbox Example"
}


dlg:showxy (0, 0)
