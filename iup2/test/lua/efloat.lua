IUPMASK_EFLOAT = "[+/-]?(/d+/.?/d*|/./d+)([eE][+/-]?/d+)?"

txt = iuptext{expand="YES", SIZE="200x"}
iupmaskSet (txt, IUPMASK_EFLOAT, 0, 1) ;
dg = iupdialog{txt}
dg:show()
