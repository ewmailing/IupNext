-- IupMultiline Advanced Example in IupLua 
-- Shows a dialog with a multiline, a text, a list and some buttons. 
-- You can test the multiline attributes by clicking on the buttons.
-- Each button is related to an attribute.
-- Select if you want to set or get an attribute using the dropdown list. 
-- The value in the text will be used as value when a button is pressed.. 

-- Creates a multiline and turn on expand
mltline = iupmultiline {expand = "YES"}

-- Creates a multiline and turn on horizontal expand
text = iuptext {expand = "HORIZONTAL"}

-- Creates a list and sets its items and dropdown
list = iuplist {"SET", "GET" ; dropdown = "YES"}

-- Creates buttons
btn_append = iupbutton {title = "Append"}
btn_insert = iupbutton {title = "Insert"}
btn_border = iupbutton {title = "Border"}
btn_caret = iupbutton {title = "Caret"}
btn_readonly = iupbutton {title = "Read only"}
btn_selection = iupbutton {title = "Selection"}
btn_selectedtext = iupbutton {title = "Selected Text"}
btn_nc = iupbutton {title = "Number of characters"}
btn_value = iupbutton {title = "Value"}

-- Creates dialog 
dlg = iupdialog {iupvbox {mltline, 
                 iuphbox {text, list},
                 iuphbox {btn_append, btn_insert, btn_border, btn_caret, btn_readonly, 
                          btn_selection, btn_selectedtext, btn_nc, btn_value},
                 } ; title = "IupMultiLine Advanced Example", size = "FULLxQUARTER"}

-- Shows dialog in the center of the screen 
dlg:showxy (IUP_CENTER, IUP_CENTER)

function btn_append:action()
  if list.value == "1" then
    mltline.append = text.value
    IupMessage("Set attribute", "Attribute APPEND set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute APPEND get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_insert:action()
  if list.value == "1" then
    mltline.append = text.value
    IupMessage("Set attribute", "Attribute INSERT set with value "..text.value);
  else
    text.value = mltline.insert
    IupMessage("Get attribute", "Attribute INSERT get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_border:action()
  if list.value == "1" then
    mltline.border = text.value
    IupMessage("Set attribute", "Attribute BORDER set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute BORDER get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_caret:action()
  if list.value == "1" then
    mltline.caret = text.value
    IupMessage("Set attribute", "Attribute CARET set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute CARET get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_readonly:action()
  if list.value == "1" then
    mltline.readonly = text.value
    IupMessage("Set attribute", "Attribute READONLY set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute READONLY get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_selection:action()
  if list.value == "1" then
    mltline.selection = text.value
    IupMessage("Set attribute", "Attribute SELECTION set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute SELECTION get with value "..text.value);
  end

  return IUP_DEFAULT
end                                                       

 function btn_selectedtext:action()
  if list.value == "1" then
    mltline.selectedtext = text.value
    IupMessage("Set attribute", "Attribute SELECTEDTEXT set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute SELECTEDTEXT get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_nc:action()
  if list.value == "1" then
    mltline.nc = text.value
    IupMessage("Set attribute", "Attribute NC set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute NC get with value "..text.value);
  end

  return IUP_DEFAULT
end

function btn_value:action()
  if list.value == "1" then
    mltline.value = text.value
    IupMessage("Set attribute", "Attribute VALUE set with value "..text.value);
  else
    text.value = mltline.append
    IupMessage("Get attribute", "Attribute VALUE get with value "..text.value);
  end

  return IUP_DEFAULT
end