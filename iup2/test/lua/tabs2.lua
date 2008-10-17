$debug

-----------------------------------------------------  

function dlgdescriptor( )
  
 local sizet = "65x"
 
 self = {}

 self.Label = iuptext{ value   = self.label, 
                       fgcolor = "0 0 0", size = "70x", expand = "NO" }

 self.Curr1 = iuplabel  { title     = "",
                          fgcolor   = "000 000 000",
                          alignment = "ACENTER",
                          size      = sizet }

 self.Curr2 = iuplabel  { title     = "",
                          fgcolor   = "000 000 000",
                          alignment = "ACENTER",
                          size      = sizet }

 self.LS1 = iuplist { obj = self }
 self.LS2 = iuplist { obj = self }

 self.BT1 = iupbutton { title  = "Create Time Functions...",
                        obj    = self }

 IupSetAttribute( self.LS1, "FONT", IUP_TIMES_NORMAL_12 )
 IupSetAttribute( self.LS1, "BGCOLOR", "255 255 255" )
 IupSetAttribute( self.LS1, "FGCOLOR", "000 000 000" )
 IupSetAttribute( self.LS1, "DROPDOWN","NO" )
 IupSetAttribute( self.LS1, "SIZE",    "140x100" )

 IupSetAttribute( self.LS2, "FONT", IUP_TIMES_NORMAL_12 )
 IupSetAttribute( self.LS2, "BGCOLOR", "255 255 255" )
 IupSetAttribute( self.LS2, "FGCOLOR", "000 000 000" )
 IupSetAttribute( self.LS2, "DROPDOWN","NO" )
 IupSetAttribute( self.LS2, "SIZE",    "140x100" )

 self.Tab1=iupvbox{ iupfill{},
                    iuphbox { iupfill{}, self.LS1, iupfill{} },
                    iupfill{}; tabtitle="Time functions(X)" }
 self.Tab2=iupvbox{ iupfill{},
                    iuphbox { iupfill{}, self.LS2, iupfill{} },
                    iupfill{}; tabtitle="Time functions(Y)" }

 self.Tabs = iuptabs{ self.Tab1, self.Tab2; tabtype = "TOP" }

-- AUXF : settabs_( self.Tabs, self.Tab1 )

-- IupSetAttribute ( self.Tabs, "REPAINT", "YES" )

 -- sets starting state of tabs.
-- self : setTabs( )

 self.mtx = iupmatrix { numlin         = 2,  
                        numcol         = 1,  
                        width0         = 30,
                        alignment0     = "ACENTER",
                        scrollbar      = "NO",
                        bgcolor        = "255 255 255",
                        fgcolor        = "000 000 000",
                        numcol_visible = 1, 
                        numlin_visible = 2 }

 self.mtx:setcell(1, 0, "Gx")
 self.mtx:setcell(1, 1, self.gx)
 self.mtx:setcell(2, 0, "Gy")
 self.mtx:setcell(2, 1, self.gy)

 self.hbxp = iuphbox 
             { 
               iupfill{}, 
               iupvbox 
               {  
                 iupfill{},
                 iupframe
                 { 
                    iuphbox{ iupfill{}, self.Label, iupfill{} }; 
                    title = "Label" 
                 },
                 iupfill{},
                 iupframe { self.mtx; expand = "NO", title = "Values:" }, 
                 iupfill{},
                 iupframe
                 {
                   iuphbox { iupfill{},
                             iupvbox { iupfill{}, self.Curr1, iupfill{} },
                             iupfill{} };
                   title   = "Current time function( X ):",
                   size    = "120x",
                   fgcolor = "000 000 255"
                 },
                 iupfill{},
                 iupframe
                 {
                   iuphbox { iupfill{},
                             iupvbox { iupfill{}, self.Curr2, iupfill{} },
                             iupfill{} };
                   title   = "Current time function( Y ):",
                   size    = "120x",
                   fgcolor = "000 000 255"
                 },
                 iupfill{},
               },
               iupfill{},
               iupvbox 
               {  
                 iupfill{},
                 iuphbox { iupfill{}, self.Tabs, iupfill{} },
                 iupfill{},
                 iuphbox { self.BT1, iupfill{} },
                 iupfill{},
               },
               iupfill{};
             }

 return iupvbox
        {  
          iupframe 
          {
            iupvbox 
            { 
              iupfill{},
              self.hbxp,
              iupfill{}
            }
          }; 
          title = "Inertial",
        }  
end  

local dlg = iupdialog
{
  dlgdescriptor()
	;
	title = "TESTE",
	margin = "2x2",
	gap = 2
}

dlg:show()
