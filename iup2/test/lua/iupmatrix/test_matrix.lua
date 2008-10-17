-----------------------------------------------------

REZONE = { }
REZONE = { label = "teste", tesc = 1, step = 1, gamma = 1.81 }

-----------------------------------------------------

function REZONE : dialog( )

 local mtx = iup.matrix{ numlin    = 3,  
                        numcol    = 2,  
                        scrollbar = "NO",
                        alignment0= "ACENTER",
                        widthdef  = 60, 
                        width2    = 80,
                        expand    = "NO",
                        numcol_visible = 2, 
                        numlin_visible = 3 }

 mtx : setcell(1, 1, "Time")       mtx : setcell(1, 2, self.tesc)
 mtx : setcell(2, 1, "Step")       mtx : setcell(2, 2, self.step)
 mtx : setcell(3, 1, "Mud Weight") mtx : setcell(3, 2, self.gamma)

function mtx:action_cb(c,line,col,active,after)
  print("action",c,line,col,active,after)

  -- append string "more" into current cell
  self.value = after.."more"

  -- alway put carat at beginning
  self.caret = 1

  -- force redraw
  self.redraw = "ALL"

  return iup.DEFAULT
end

 local bt = iup.button{ title  = " Close ", 
                       size   = "50x12", 
                       action = "return iup.CLOSE" }

 return iup.vbox 
        {
          iup.hbox 
          {
            iup.fill{}, 
            iup.frame{ 
              iup.vbox { iup.fill{}, iup.frame{ mtx }, iup.fill{} }; 
              title = "Info" 
            }, 
            iup.fill{} 
          };
          title  = "Rezone",
          margin = "5x5",
          gap    = "3x3",
          button = bt
        }
end

-----------------------------------------------------

function REZONE : popup( f )

  local h = f.button
  local v = iup.vbox{ f, iup.hbox{ h } }

  local dlg = iup.dialog
             {
                iup.frame{ v };
                title   = f.title,
                font    = iup.TIMES_NORMAL_10,
                gap     = f.gap,
                margin  = f.margin,
                resize  = "NO"
             }

  dlg : map()
  dlg : popup( iup.CENTER, iup.CENTER )

end

-----------------------------------------------------


v = REZONE : dialog( )
    REZONE : popup( v )

-----------------------------------------------------
