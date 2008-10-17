-- IupFrame Example in IupLua
-- Draws a frame around a button. Note that FGCOLOR is added to the frame but
-- it is inherited by the button.

-- Creates frame with a label
frame = iup.frame
          {
            iup.hbox
            {
              iup.fill{},
              iup.label{title="IupFrame Test"},
              iup.fill{},
              NULL
            }
          } ;

-- Sets label's attributes
frame.fgcolor = "255 0 0"
frame.size    = EIGHTHxEIGHTH
frame.title   = "This is the frame"
frame.margin  = "10x10"
frame.bgcolor = "0 255 255"

-- Creates dialog
dialog = iup.dialog{frame};
dialog:show()