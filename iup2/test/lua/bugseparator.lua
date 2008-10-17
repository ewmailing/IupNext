-- quick test use of vertical label as separator for buttons
bButton1 = iup.button{ title = "1", font="Courier::8"}
bButton2 = iup.button{ image = "IUP_IMGBUT_NEW"}
bButton3 = iup.button{ title = "3"}
bButton4 = iup.button{ title = "4"}
bButton5 = iup.button{ title = "",size="2x15",active="NO"}
bButton6 = iup.button{ title = "6"}
bButton7 = iup.button{ title = "7"}
lSep1 = iup.label{separator="VERTICAL",title=""}
lSep2 = iup.label{separator="VERTICAL",title=""}

dlg =
  iup.dialog
    {
    iup.vbox
      {
      iup.hbox {
               bButton1 ,bButton2, bButton3,
               lSep1,
               bButton4 ,bButton5,
               lSep2,
               bButton6 ,bButton7,
               alignment="ACENTER",
               gap="5",
               margin="5x5" ,
               size=25,
               },
      iup.label{separator="HORIZONTAL",title=""},
      size="25",
      },
    title="TEST",
    size="440x125",
    font="Times New Roman::12",
    }

dlg:map()

lSep1.expand="NO"
lSep2.expand="NO"
lSep1.rastersize="2x25"
lSep2.rastersize="2x25"

-- Shows dialog in the center of the screen 
dlg:showxy(iup.CENTER, iup.CENTER)
