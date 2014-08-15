-------------------------------------------------------------------------------
--         Projeto Prea3D  -- TeCGraf / PUC-Rio  --
--
-------------------------------------------------------------------------------

p3dDlg.offset = {}

p3dDlg.offset.oldK = k

k = p3dDlg.offset

$debug

-- Definicao do Metodo de calculo do
-- Minimize
k.KEEP_LINES   = 0
k.KEEP_ANCHORS = 1

-- Definicao do Metodo de calculo do
-- OffSet
k.CHC      = 0
k.CATENARY = 1

----------------------------------------------------------------


k.InitCase = function( i )
   local t = p3dDlg.offset
   t.case[i] = {}
   
   t.case[i].fx = 0
   t.case[i].fy = 0
   t.case[i].mz = 0
   
   t.case[i].offsetDx  = 0
   t.case[i].offsetDy  = 0
   t.case[i].offsetDrz = 0
   t.case[i].offsetDist = 0
   t.case[i].offsetStatus = "--"
   t.case[i].offsetErrorMsg = ""
   t.case[i].firstDamDx = 0
   t.case[i].firstDamDy = 0
   t.case[i].firstDamDrz = 0
   t.case[i].firstDamDist = 0
   t.case[i].firstDamStatus = "--"
   t.case[i].firstDamErrorMsg = ""
   t.case[i].secondDamDx = 0
   t.case[i].secondDamDy = 0
   t.case[i].secondDamDrz = 0
   t.case[i].secondDamDist = 0
   t.case[i].secondDamStatus = "--"
   t.case[i].secondDamErrorMsg = ""
   
   t.miniDx   = 0
   t.miniDy   = 0
   t.miniDist = 0
   t.miniDrz  = 0
end



k.InitOffsetTable = function( )
   local t = p3dDlg.offset
   t.data = {}
   t.case = {}
   
   t.data.npoints = 40
   t.data.ndepths = 1
   t.data.delta = 0
   t.data.factor = fmt_f(1)
   t.data.methodCC = 1
   t.data.methodFA = 1
   
   t.InitCase( 1 )
   
   t.miniDx   = 0
   t.miniDy   = 0
   t.miniDist = 0
   t.miniDrz  = 0
   
   t.minimizeType = t.KEEP_LINES
   t.numCase = 1
end


k.InitOffsetTable( )


k.ResetCases = function( i )
   local t = p3dDlg.offset
   
   t.case[i].offsetDx  = 0
   t.case[i].offsetDy  = 0
   t.case[i].offsetDrz = 0
   t.case[i].offsetDist = 0
   t.case[i].offsetStatus = "--"
   t.case[i].offsetErrorMsg = ""
   t.case[i].firstDamDx = 0
   t.case[i].firstDamDy = 0
   t.case[i].firstDamDrz = 0
   t.case[i].firstDamDist = 0
   t.case[i].firstDamStatus = "--"
   t.case[i].firstDamErrorMsg = ""
   t.case[i].secondDamDx = 0
   t.case[i].secondDamDy = 0
   t.case[i].secondDamDrz = 0
   t.case[i].secondDamDist = 0
   t.case[i].secondDamStatus = "--"
   t.case[i].secondDamErrorMsg = ""
   
   t.minimize_button.active = IUP_NO
   t.apply_button.active = IUP_NO
      
   t.resultTab.value = t.resultBox
   t.matrixResult.numlin = 0
   t.matrixResultFirstDam.numlin  = 0
   t.matrixResultSecondDam.numlin = 0
   t.labelFirstDamaged.title = "Damaged Line:"
   t.labelSecondDamaged.title = "Damaged Line:"
   t.resultTab.value = t.resultBox
   offsetLineGauge.value = 0.0     
   co_deleteCase( i - 1 )
   t.viewCharCuves.active = IUP_NO
end





--
-- Callback de edicao da matriz de numero de casos
--
EditioncaseNumMat = function( self, lin, col, modo )
   local t = p3dDlg.offset
   if ( modo == 0 ) then
      local num = tonumber( self:getcell( lin, col ) )
      
      -- Adicionando Casos
      if ( t.numCase < num ) then
         local i = t.numCase + 1
         while( i <= num ) do
            t.InitCase( i )
            t.SetData( i )
            i = i + 1
         end
         t.Matrix.numlin = num
         t.numCase = num
      
      -- Retirando Casos
      elseif( num < t.numCase ) then
         local i = t.numCase
         t.Matrix.numlin = num
         t.numCase = num
         while( i > num ) do
            co_deleteCase( i - 1 )
            i = i - 1
         end
      end
      
      t.DrawMatrix( )
      
      CleanMatrixColor(  )
      
      LineMatrixColor( 1 )
                  
      return IUP_DEFAULT
   end
end



k.caseNumMat = iupmatrix
               {
                  numlin = 1,
                  numcol = 1,
                  numlin_visible = 1,
                  numcol_visible = 1,
                  width0 = 120,
                  width1 = 40,
                  expand = IUP_NO,
                  scrollbar = IUP_NO,
                  alignment0 = IUP_ACENTER,
                  edition = EditioncaseNumMat
               }
               
k.caseNumMat:setcell( 1, 0, "# of External Load Cases" )

k.viewCharCuves = iupbutton{ title = "View Characteristic Curves", size = 130 }

k.viewCharCuves.action = function()
    local lineCount, rawData = co_getCCurveRawData()
    ccurveplotOpen(lineCount, rawData)
end

k.catenary_toggle = iuptoggle{ title = "Catenary" }
k.chc_toggle      = iuptoggle{ title = "Characteristic Curves" }

k.catenary_toggle.action = function( self, status )
   local t = p3dDlg.offset
   t.offsetType = t.CATENARY
   
   local i = 1
   while( i <= t.numCase ) do
      t.ResetCases( i )
      t.SetData( i )
      i = i + 1
   end
   
   t.minimize_button.active    = IUP_NO
   t.apply_button.active       = IUP_NO
   t.resultFirstDamBox.active  = IUP_NO
   t.resultSecondDamBox.active = IUP_NO
   t.matrixResult.numlin = 0
   t.matrixResultFirstDam.numlin  = 0
   t.matrixResultSecondDam.numlin = 0
   t.labelFirstDamaged.title = "Damaged Line:"
   t.labelSecondDamaged.title = "Damaged Line:"
   t.resultTab.value = t.resultBox
   t.resultTab.repaint = IUP_YES
    
   
   t.chcFrame.active = IUP_NO
   t.CalcMatrix.active = IUP_NO
end

k.chc_toggle.action = function( self, status )
   local t = p3dDlg.offset
   t.offsetType = t.CHC
   
   local i = 1
   while( i <= t.numCase ) do
      t.ResetCases( i )
      t.SetData( i )
      i = i + 1
   end
   
   t.minimize_button.active    = IUP_NO
   t.apply_button.active       = IUP_NO
   t.resultFirstDamBox.active  = IUP_NO
   t.resultSecondDamBox.active = IUP_NO
   t.matrixResult.numlin = 0
   t.matrixResultFirstDam.numlin  = 0
   t.matrixResultSecondDam.numlin = 0
   t.labelFirstDamaged.title = "Damaged Line:"
   t.labelSecondDamaged.title = "Damaged Line:"
   t.resultTab.value = t.resultBox
   t.resultTab.repaint = IUP_YES
   
   t.chcFrame.active = IUP_YES
   t.CalcMatrix.active = IUP_YES
end

k.offset_radio = iupradio
                 {
                    iupvbox
                    {
                       iupfill{ size = 5 },
                       iuphbox
                       {  
                          iupfill{ size = 5 },
                           k.chc_toggle,
                          iupfill{ size = 5 },
                          k.catenary_toggle,
                          iupfill{ size = 5 }
                       },
                       iupfill{ size = 5 }
                    }
                 }



k.enteritemCalcMatrix = function( self, lin, col )
   return IUP_DEFAULT
end


k.leaveitemCalcMatrix = function( self, lin, col )
   return IUP_DEFAULT
end

k.editionCalcMatrix = function( self, lin, col, modo )
   
   if ( modo == 0 ) then
      local t = p3dDlg.offset
      local v = p3dUnit.getValue( t.CalcMatrix, lin, col )

      if( col == 1 ) then
         t.data.npoints = v

      elseif( col == 2 ) then
         if ( mod( v, 2 ) == 0 ) then
            IupMessage( "Prea3D", "Depths must be an odd number" )
            return IUP_IGNORE
         else
            t.data.ndepths = v
         end

      elseif( col == 3 ) then
         t.data.delta = v

      elseif ( col == 4 ) then
         p3dUnit.setValue( t.CalcMatrix, lin, col, fmt_f(v) )
         t.data.factor = fmt_f(v)

      elseif( col == 5 ) then
         if( v == "Catenary" ) then
            t.data.methodCC = 1
         elseif( v == "FEM" ) then
            t.data.methodCC = 2
         end
      end

      local i = 1
      while( i <= t.numCase ) do
         t.ResetCases( i )
         t.SetData( i )
         i = i + 1
      end

      co_changeDataOri( t.data.npoints, t.data.ndepths, t.data.delta, 
                        t.data.factor, t.data.methodCC, t.data.methodFA )
   end
   return IUP_DEFAULT
end


k.dropeditionCalcMatrix = function( self, drop, lin, col )
   if ( col == 5 or col == 6 ) then
      local t = p3dDlg.offset
      if ( col == 5 ) then
         local l = drop.previousvalue
      
         if ( l == "Catenary" ) then
            IupSetAttribute( drop, 1, "Catenary" )
            IupSetAttribute( drop, 2, "FEM" )
            drop.value = 1
         else
            IupSetAttribute( drop, 1, "Catenary" )
            IupSetAttribute( drop, 2, "FEM" )
            drop.value = 2
         end
      elseif ( col == 6 ) then
         local l = drop.previousvalue
      
         if ( l == "Catenary" ) then
            IupSetAttribute( drop, 1, "Catenary" )
            IupSetAttribute( drop, 2, "FEM" )
            drop.value = 1
         elseif ( l == "FEM" ) then
            IupSetAttribute( drop, 1, "Catenary" )
            IupSetAttribute( drop, 2, "FEM" )
            drop.value = 2
         else
            IupSetAttribute( drop, 1, "Catenary" )
            IupSetAttribute( drop, 2, "FEM" )
            drop.value = 3
         end
      end
      local i = 1
      while( i <= t.numCase ) do
         t.ResetCases( i )
         t.SetData( i )
         i = i + 1
      end
      return IUP_DEFAULT
   else
      return IUP_IGNORE
   end
end


k.CallbackMousemove = function( self, lin, col )
   if( lin == 0 ) then
      if( col == 3 ) then
         self.cursor = IUP_ARROW
         self.tip = "Variation of the Depth Value"
      elseif( col == 4 ) then
         self.cursor = IUP_ARROW
         self.tip = "Factor for Multiplying the MBL Value"
      elseif( col == 5 ) then
         self.cursor = IUP_ARROW
         self.tip = "Method for generating Characteristic Curves"
      elseif( col == 6 ) then
         self.cursor = IUP_ARROW
         self.tip = "Method for Computing Anchor Positions"
      else
         self.cursor = nil
         self.tip = ""
      end
      return IUP_DEFAULT
   else
      return IUP_IGNORE
   end
end


k.CalcMatrix = unitmatrix
               {
                  numlin = 1,
                  numcol = 5,
                  numlin_visible = 1,
                  numcol_visible = 5,
                  widthdef = 60,
                  width2 = 65,
                  scrollbar = IUP_VERTICAL,
                  expand = IUP_NO,
                  alignment0 = IUP_CENTER,
                  alignmentdef = IUP_CENTER,
              
                  mousemove = k.CallbackMousemove,
                  enteritem = k.enteritemCalcMatrix,
                  leaveitem = k.leaveitemCalcMatrix,
                  edition = k.editionCalcMatrix,
                  drop = k.dropeditionCalcMatrix,
              
                  units = 
                  {
                     orientation = "col";
                     {"# of Points", nil, "%d", p3dUnit.mask.int },
                     {"# of Diff Depths", nil, "%d", p3dUnit.mask.int },
                     {"Delta", p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                     {"MBL Factor", nil, "%d", p3dUnit.mask.int },
                     {"Curve Gen", nil, "%s", nil },
                  }
               }
           


function CleanMatrixColor( )
   local t = p3dDlg.offset
   t.Matrix["BGCOLOR"..t.caseSelected..":1"] = "255 255 255"
   t.Matrix["FGCOLOR"..t.caseSelected..":1"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":2"] = "255 255 255"
   t.Matrix["FGCOLOR"..t.caseSelected..":2"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":3"] = "255 255 255"
   t.Matrix["FGCOLOR"..t.caseSelected..":3"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":4"] = NoEditBcolor
   t.Matrix["FGCOLOR"..t.caseSelected..":4"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":5"] = NoEditBcolor
   t.Matrix["FGCOLOR"..t.caseSelected..":5"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":6"] = NoEditBcolor
   t.Matrix["FGCOLOR"..t.caseSelected..":6"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":7"] = NoEditBcolor
   t.Matrix["FGCOLOR"..t.caseSelected..":7"] = "0 0 0"
   t.Matrix["BGCOLOR"..t.caseSelected..":8"] = NoEditBcolor
   t.Matrix["FGCOLOR"..t.caseSelected..":8"] = "0 0 0"
end


k.DrawMatrix = function( )
   local t = p3dDlg.offset
   local i = 1
   while( i <= t.numCase ) do
      t.Matrix["BGCOLOR"..i..":1"] = "255 255 255"
      t.Matrix["FGCOLOR"..i..":1"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":2"] = "255 255 255"
      t.Matrix["FGCOLOR"..i..":2"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":3"] = "255 255 255"
      t.Matrix["FGCOLOR"..i..":3"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":4"] = NoEditBcolor
      t.Matrix["FGCOLOR"..i..":4"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":5"] = NoEditBcolor
      t.Matrix["FGCOLOR"..i..":5"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":6"] = NoEditBcolor
      t.Matrix["FGCOLOR"..i..":6"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":7"] = NoEditBcolor
      t.Matrix["FGCOLOR"..i..":7"] = "0 0 0"
      t.Matrix["BGCOLOR"..i..":8"] = NoEditBcolor
      t.Matrix["FGCOLOR"..i..":8"] = "0 0 0"
      i = i + 1
   end
end



function LineMatrixColor( lin )
   local t = p3dDlg.offset
   t.Matrix["BGCOLOR"..lin..":1"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":2"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":3"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":4"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":5"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":6"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":7"] = ActiveBcolor
   t.Matrix["BGCOLOR"..lin..":8"] = ActiveBcolor
   t.Matrix["FGCOLOR"..lin..":1"] = ActiveFcolor
   t.Matrix["FGCOLOR"..lin..":2"] = ActiveFcolor
   t.Matrix["FGCOLOR"..lin..":3"] = ActiveFcolor
   t.Matrix["FGCOLOR"..lin..":4"] = NoEditFcolor
   t.Matrix["FGCOLOR"..lin..":5"] = NoEditFcolor
   t.Matrix["FGCOLOR"..lin..":6"] = NoEditFcolor
   t.Matrix["FGCOLOR"..lin..":7"] = NoEditFcolor
   t.Matrix["FGCOLOR"..lin..":8"] = NoEditFcolor
end

k.displayResult = function( resultType, matrix, caseCurr )
   local t = p3dDlg.offset
   
   local tbl = co_resultLines( resultType, caseCurr )
   
   if( tbl == nil ) then
      return
   end
   if( resultType == 3 ) then
      t.labelFirstDamaged.title = "Damaged Line: Line "..tbl.exLine.id.."( "..tbl.exLine.name.." )"
      t.labelFirstDamaged.redraw = IUP_YES
   elseif( resultType == 4 ) then
      t.labelSecondDamaged.title = "Damaged Line: Line "..tbl.exLine.id.."( "..tbl.exLine.name.." )"
      t.labelSecondDamaged.redraw = IUP_YES
   end
      
   local i = 1   
   matrix.numlin = tbl.nLines
   matrix.lineNames = {}

   while( i <= tbl.nLines ) do
      p3dUnit.setValue( matrix, i, 0, tbl[i].id      )
      p3dUnit.setValue( matrix, i, 1, tbl[i].topTen  )
      p3dUnit.setValue( matrix, i, 2, tbl[i].newTopTen  )
      p3dUnit.setValue( matrix, i, 3, tbl[i].topHTen )
      p3dUnit.setValue( matrix, i, 4, tbl[i].topVTen )
      p3dUnit.setValue( matrix, i, 5, tbl[i].topAng  )
      p3dUnit.setValue( matrix, i, 6, tbl[i].az      )
      p3dUnit.setValue( matrix, i, 7, tbl[i].paidOut )
      matrix:setcell( i, 8, format( "%.2f", tbl[i].mblFac  ) )
      matrix.lineNames[i] = tbl[i].name
      
      i = i + 1
   end
end


k.enteritemMatrix = function( self, lin, col )
   local t = p3dDlg.offset

   CleanMatrixColor(  )
   
   LineMatrixColor( lin )
   
   t.caseSelected = lin
   
   if( t.resultTab.value == t.resultBox ) then
      if ( t.case[t.caseSelected].offsetStatus ~= "Ok" ) then
         t.minimize_button.active    = IUP_NO
         t.apply_button.active       = IUP_NO
         t.resultFirstDamBox.active  = IUP_NO
         t.resultSecondDamBox.active = IUP_NO
         t.matrixResult.numlin = 0
         t.matrixResultFirstDam.numlin  = 0
         t.matrixResultSecondDam.numlin = 0
         t.labelFirstDamaged.title = "Damaged Line:"
         t.labelSecondDamaged.title = "Damaged Line:"
         t.resultTab.value = t.resultBox
         t.resultTab.repaint = IUP_YES
      else
         t.minimize_button.active = IUP_YES
         t.apply_button.active    = IUP_YES
         t.resultFirstDamBox.active  = IUP_YES
         t.resultSecondDamBox.active = IUP_YES
         t.displayResult( 2, t.matrixResult, t.caseSelected-1 )
         t.resultTab.repaint = IUP_YES
      end

   elseif( t.resultTab.value == t.resultFirstDamBox ) then
      if ( t.case[t.caseSelected].firstDamStatus ~= "Ok" ) then
         t.matrixResultFirstDam.numlin  = 0
      else
         t.displayResult( 3, t.matrixResultFirstDam, t.caseSelected-1 )
      end
   elseif( t.resultTab.value == t.resultSecondDamBox ) then
      if ( t.case[t.caseSelected].secondDamStatus ~= "Ok" ) then
         t.matrixResultFirstDam.numlin  = 0
      else
         t.displayResult( 4, t.matrixResultSecondDam, t.caseSelected-1 )
      end
   end
   
   
   return IUP_DEFAULT      
end


k.leaveitemMatrix = function( self, lin, col )
   return IUP_DEFAULT
end


k.editionMatrix = function( self, lin, col, modo )
   if( col >= 4 ) then
      return IUP_IGNORE
   elseif ( modo == 0 ) then
      local t = p3dDlg.offset
      if ( col >= 1 and col <= 3 ) then
         t.case[lin].fx = p3dUnit.getValue( self, lin, 1 )
         t.case[lin].fy = p3dUnit.getValue( self, lin, 2 )
         t.case[lin].mz = p3dUnit.getValue( self, lin, 3 )
      end

      local _
      t.case[lin].fx, t.case[lin].fy, _  = cfpTransfInv( t.case[lin].fx, t.case[lin].fy, 0)
       _ , _ , t.case[lin].mz = cfpTransfInv(0, 0, t.case[lin].mz)

      t.ResetCases( lin )
      t.SetData( lin )
   end

   return IUP_DEFAULT
end

k.Mousemove = function( self, lin, col )
   local t = p3dDlg.offset
   if( lin == 0 ) then
      if( col == 1 ) then
         self.cursor = IUP_ARROW
         self.tip = "External Force in Global X Direction"
      elseif( col == 2 ) then
         self.cursor = IUP_ARROW
         self.tip = "External Force in Global Y Direction"
      elseif( col == 3 ) then
         self.cursor = IUP_ARROW
         self.tip = "External Moment in Global Mz Direction"
      elseif( col == 4 ) then
         self.cursor = IUP_ARROW
         self.tip = "Offset Component in Global X Direction"
      elseif( col == 5 ) then
         self.cursor = IUP_ARROW
         self.tip = "Offset Component in Global Y Direction"
      elseif( col == 6 ) then
         self.cursor = IUP_ARROW
         self.tip = "Delta Distance"
      elseif( col == 7 ) then
         self.cursor = IUP_ARROW
         self.tip = "Rotation in Global Z Direction"
      end
      return IUP_DEFAULT
   elseif( col == 8 ) then
      self.cursor = IUP_ARROW
      if( t.case[lin] ~= nil ) then
         if( t.resultTab.value == t.resultBox ) then
            self.tip = t.case[lin].offsetErrorMsg
         elseif( t.resultTab.value == t.resultFirstDamBox ) then
            self.tip = t.case[lin].firstDamErrorMsg
         elseif( t.resultTab.value == t.resultSecondDamBox ) then
            self.tip = t.case[lin].secondDamErrorMsg
         end
      end

   else
      return IUP_IGNORE
   end
end


k.Matrix = unitmatrix
           {
              numlin = 1,
              numcol = 8,
              numlin_visible = 8,
              numcol_visible = 8,
              width0 = 30,
              widthdef = 60,
              scrollbar = IUP_VERTICAL,
              expand = IUP_NO,
              alignment0 = IUP_CENTER,
              alignmentdef = IUP_CENTER,
              
              mousemove = k.Mousemove,
              enteritem = k.enteritemMatrix,
              leaveitem = k.leaveitemMatrix,
              edition = k.editionMatrix,
                            
              units = 
              {
                 orientation = "col";
                 {"Fx", p3dUnit.force, "%.3f", p3dUnit.mask.float },
                 {"Fy", p3dUnit.force, "%.3f", p3dUnit.mask.float },
                 {"Mz", p3dUnit.moment, "%.3f", p3dUnit.mask.float },
                 {"Delta X" , p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                 {"Delta Y" , p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                 {"Delta Distance" , p3dUnit.length, "%.3f", p3dUnit.mask.float },
                 {"Delta Rz" , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
                 {"Status", nil, "%s", nil },
              }
           }


k.Matrix:setcell( 0, 0, "Case" )

k.chcFrame = iupframe
             {
                iuphbox
                {
                   iupfill{ size = 5 },
                   iupvbox
                   {
                      iupfill{ size = 5 },
                      k.CalcMatrix,
                      iupfill{ size = 5 }
                   },
                   iupfill{ size = 5 }
                };
                title = "Characteristic Curve Data"
             }


k.CalcBox = iuphbox
            {
               iupfill{},
               iupvbox
               {
                  iupfill{},
                  k.chcFrame,
                  iupfill{}
               },
               iupfill{}
            }
            

k.dataBox = iuphbox
              {
                 iupfill{},
                 iupvbox
                 {
                    iupfill{},
                    iupframe{ k.Matrix },
                    iupfill{}
                 },
                 iupfill{}
              }
              
offsetOpGauge   = iupgauge{ size = "40x12", expand = IUP_HORIZONTAL }
offsetLineGauge = iupgauge{ size = "40x12", expand = IUP_HORIZONTAL }

k.close_button       = iupbutton{ title = "Close"   , size = BUTTON_SZ }
k.apply_button       = iupbutton{ title = "Apply"   , size = BUTTON_SZ }
k.reportHTML_button  = iupbutton{ title = "Report"  , size = BUTTON_SZ }
k.run_button         = iupbutton{ title = "Run"     , size = BUTTON_SZ }
k.minimize_button    = iupbutton{ title = "Minimize", size = BUTTON_SZ, active = IUP_NO }
k.canvas_button      = iupbutton{ title = "Top View"  , size = BUTTON_SZ }

k.MinRun_button    = iupbutton{ title = "Run", size = BUTTON_SZ }
k.MinOk_button     = iupbutton{ title = "Ok", size = BUTTON_SZ }
k.MinCancel_button = iupbutton{ title = "Cancel", size = BUTTON_SZ }


k.canvas_button.action = function ( self )
   local t = p3dDlg.offset
   
   if( t.resultTab.value == t.resultBox ) then
      co_getResultDisp( 1 )
   
   elseif( t.resultTab.value == t.resultFirstDamBox ) then
      co_getResultDisp( 2 )
   
   elseif( t.resultTab.value == t.resultSecondDamBox ) then
      co_getResultDisp( 3 )   
   end
   
   t.setListCanvas()
   t.setMatrixResultCanvas( t.listCanvas[1] )
   
   -- setando transparencia
   local trans = OffsetGetTransparency( )
   t.transparencyVal.value = 1 - trans
   OffsetRedraw()
   
   t.canvasResult:popup( IUP_CENTER, IUP_CENTER  )
   
end


k.close_button.action = function( self )
   local t = p3dDlg.offset
   local i = 1
   if( offsetLineGauge.CALC == "1" ) then
       offsetLineGauge.CALC = "0"
       return IUP_DEFAULT
   end
   
   while( i <= t.numCase ) do
      t.ResetCases( i )      
      i = i + 1
   end
   return IUP_CLOSE
end

k.apply_button.action = function()
   co_applyOffset( p3dDlg.offset.caseSelected-1 )
   return IUP_CLOSE
end

k.reportHTML_button.action = function()
   GenReportOffset()
   --return IUP_CLOSE
end

k.minimize_button.action = function()
   local t = p3dDlg.offset

   t.MinOk_button.active = IUP_NO
   
   t.miniDx   = 0.0
   t.miniDy   = 0.0
   t.miniDist = 0.0
   t.miniDrz  = 0.0
   
   t.CaseMatrix:setcell( 1, 1, t.caseSelected )
   t.displayResult( 0, t.matrixResultMinimize, t.caseSelected-1 )

   local _, dx, dy, drz, fx, fy, mz
   
   fx, fy, _  = cfpTransf( t.case[t.caseSelected].fx, t.case[t.caseSelected].fy, 0)
   _ , _, mz  = cfpTransf( 0, 0, t.case[t.caseSelected].mz )
   dx, dy, _  = cfpTransf( t.case[t.caseSelected].offsetDx, t.case[t.caseSelected].offsetDy, 0)
   _ , _, drz = cfpTransf( 0, 0, t.case[t.caseSelected].offsetDrz )
   
   p3dUnit.setValue( t.MinForceMatrix, 1, 1, fx )
   p3dUnit.setValue( t.MinForceMatrix, 1, 2, fy )
   p3dUnit.setValue( t.MinForceMatrix, 1, 3, mz )
   
   p3dUnit.setValue( t.MinOutMatrix, 1, 1, dx )
   p3dUnit.setValue( t.MinOutMatrix, 1, 2, dy )
   p3dUnit.setValue( t.MinOutMatrix, 1, 3, t.case[t.caseSelected].offsetDist )
   p3dUnit.setValue( t.MinOutMatrix, 1, 4, drz )
   
   p3dUnit.setValue( t.MinOutMatrix, 2, 1, t.miniDx   )
   p3dUnit.setValue( t.MinOutMatrix, 2, 2, t.miniDy   )
   p3dUnit.setValue( t.MinOutMatrix, 2, 3, t.miniDist )
   p3dUnit.setValue( t.MinOutMatrix, 2, 4, t.miniDrz  )
   
   if ( t.minimizeType == t.KEEP_ANCHORS )then
      t.minimize_radio = k.anchor_toggle
   elseif( t.minimizeType ==  t.KEEP_LINES ) then
      t.minimize_radio = k.lines_toggle
   end
   
   t.ZBox.value = t.MinimizeBox

end



k.run_button.action = function()
   local t = p3dDlg.offset
   local methodCC, methodFA
   
   t.run_button.active        = IUP_NO
   t.minimize_button.active   = IUP_NO
   t.canvas_button.active     = IUP_NO
   t.reportHTML_button.active = IUP_NO
   
   
   offsetLineGauge.CALC = "1"
   t.close_button.title = "Cancel"
      
   if( t.resultTab.value == t.resultBox ) then
      
      local i = 1
      while ( i <= tonumber(t.numCase) ) do
         if ( t.case[i].offsetStatus ~= "Ok" ) then

            local ret, errorMsg
               
            ret, errorMsg, t.case[i].offsetDx,
            t.case[i].offsetDy, t.case[i].offsetDrz = co_calcOffset( t.case[i].fx, t.case[i].fy, t.case[i].mz, i - 1, t.offsetType )

            if ret == 1 then 
               t.case[i].offsetStatus = "Ok"
               t.case[i].offsetErrorMsg = errorMsg
               t.case[i].offsetDist = sqrt( t.case[i].offsetDx*t.case[i].offsetDx + t.case[i].offsetDy*t.case[i].offsetDy )
            else
               if( offsetLineGauge.CALC == "0" ) then
                  t.case[i].offsetStatus = "Canceled"
               else
                  t.case[i].offsetStatus = "Error"
               end
               t.case[i].offsetErrorMsg = errorMsg
               offsetLineGauge.value = 0.0
            end
            t.SetData( i )
         end
         i = i + 1
      end

      if( t.case[t.caseSelected].offsetStatus ~= "Ok" ) then
         t.minimize_button.active = IUP_NO
         t.apply_button.active = IUP_NO
      else
         t.minimize_button.active = IUP_YES
         t.resultTab.repaint = IUP_YES
         t.apply_button.active = IUP_YES
      end
   elseif( t.resultTab.value == t.resultFirstDamBox ) then
      
      local i = 1
      while ( i <= tonumber(t.numCase) ) do
         if ( t.case[i].firstDamStatus ~= "Ok" ) then

            local errorMsg
               
            errorMsg, t.case[i].firstDamDx, 
            t.case[i].firstDamDy, t.case[i].firstDamDrz = co_calcFirstDam( i - 1, t.offsetType )
                              
            if errorMsg == nil then 
               t.case[i].firstDamStatus = "Ok"
               t.case[i].firstDamErrorMsg = errorMsg
               t.case[i].firstDamDist = sqrt( t.case[i].firstDamDx*t.case[i].firstDamDx + t.case[i].firstDamDy*t.case[i].firstDamDy )
            else
               if( offsetLineGauge.CALC == "0" ) then
                  t.case[i].firstDamStatus = "Canceled"
               else
                  t.case[i].firstDamStatus = "Error"
               end
               t.case[i].firstDamErrorMsg = errorMsg
               offsetLineGauge.value = 0.0
            end
            t.SetData( i )
	      end
	      i = i + 1
	   end
	   	   
	elseif( t.resultTab.value == t.resultSecondDamBox ) then
      
      local i = 1
      while ( i <= tonumber(t.numCase) ) do
         if ( t.case[i].secondDamStatus ~= "Ok" ) then

            local errorMsg
               
            errorMsg, t.case[i].secondDamDx, 
            t.case[i].secondDamDy, t.case[i].secondDamDrz = co_calcSecondDam( i - 1, t.offsetType )
                              
            if errorMsg == nil then 
               t.case[i].secondDamStatus = "Ok"
               t.case[i].secondDamErrorMsg = errorMsg
               t.case[i].secondDamDist = sqrt( t.case[i].secondDamDx*t.case[i].secondDamDx + t.case[i].secondDamDy*t.case[i].secondDamDy )
            else
               if( offsetLineGauge.CALC == "0" ) then
                  t.case[i].secondDamStatus = "Canceled"
               else
                  t.case[i].secondDamStatus = "Error"
               end
               t.case[i].secondDamErrorMsg = errorMsg
               offsetLineGauge.value = 0.0
            end
            t.SetData( i )
	      end
	      i = i + 1
	   end
	   
   end

   offsetLineGauge.CALC = "0"
   t.close_button.title = "Close"
      
   t.run_button.active        = IUP_YES
   t.canvas_button.active     = IUP_YES
   t.reportHTML_button.active = IUP_YES
   if (t.offsetType == t.CHC) then
        t.viewCharCuves.active = IUP_YES
   end
   
   t.Matrix:enteritem( t.caseSelected, 1 )
end


k.MinCancel_button.action = function( )
   local t = p3dDlg.offset
   t.ZBox.value = t.box
   
   t.miniDx  = 0.0
   t.miniDy  = 0.0
   t.miniDis = 0.0
   t.miniDrz = 0.0
      
   co_minimizeCancel()
   
   return IUP_DEFAULT
end

k.MinOk_button.action = function( )
   local t = p3dDlg.offset
   local i = 1
   while( i <= t.numCase ) do
      if ( i ~= t.caseSelected ) then
         t.ResetCases( i )
         t.SetData( i )
      end
      i = i + 1
   end
   
   t.case[t.caseSelected].offsetDx   = t.miniDx
   t.case[t.caseSelected].offsetDy   = t.miniDy
   t.case[t.caseSelected].offsetDist = t.miniDist
   t.case[t.caseSelected].offsetDrz  = t.miniDrz

   t.SetData( t.caseSelected )
   co_minimizeOk( t.caseSelected - 1 )
   
   t.Matrix:enteritem( t.caseSelected, 1 )
  
   t.ZBox.value = t.box
   
   return IUP_DEFAULT
end

k.MinRun_button.action = function( )
   local t = p3dDlg.offset

   t.miniDx = 0
   t.miniDy = 0
   t.miniDist = 0
   t.miniDrz = 0
   
   p3dUnit.setValue( t.MinOutMatrix, 2, 1, t.miniDx   )
   p3dUnit.setValue( t.MinOutMatrix, 2, 2, t.miniDy   )
   p3dUnit.setValue( t.MinOutMatrix, 2, 3, t.miniDist )
   p3dUnit.setValue( t.MinOutMatrix, 2, 4, t.miniDrz  )
   
   co_minimizeStart( t.caseSelected-1 )
   
   local ret, errorMsg
   errorMsg = ""
        
   ret, errorMsg, t.miniDx, t.miniDy, t.miniDrz = co_minimizeOffSet( t.caseSelected-1, t.minimizeType )
      
   if( ret == 1) then
      t.miniDist = sqrt( t.miniDx*t.miniDx + t.miniDy*t.miniDy )
      p3dUnit.setValue( t.MinOutMatrix, 2, 1, t.miniDx )
      p3dUnit.setValue( t.MinOutMatrix, 2, 2, t.miniDy )
      p3dUnit.setValue( t.MinOutMatrix, 2, 3, t.miniDist )
      p3dUnit.setValue( t.MinOutMatrix, 2, 4, t.miniDrz )
      t.MinOutMatrix["REDRAW"] = IUP_YES
      t.MinOk_button.active = IUP_YES
      
      t.displayResult( 1, t.matrixResultMinimize, 0 )
      
   else
      IupMessage( "Prea3D - Error", errorMsg )
      t.MinOk_button.active = IUP_NO
      
   end
  
   return IUP_DEFAULT
end


k.anchor_toggle = iuptoggle{ title = "Keeping Anchor Positions" }
k.lines_toggle  = iuptoggle{ title = "Keeping Line Lengths" }

k.anchor_toggle.action = function( self, status )
   local t = p3dDlg.offset
   t.minimizeType = t.KEEP_ANCHORS
end

k.lines_toggle.action = function( self, status )
   local t = p3dDlg.offset
   t.minimizeType = t.KEEP_LINES
end

k.minimize_radio = iupradio
                   {
                      iupvbox
                      {
                         k.lines_toggle,
                         iupfill{ size = 5 },
                         k.anchor_toggle
                         
                      }
                   }

k.CaseMatrix = iupmatrix
               {
                  numlin = 1,
                  numcol = 1,
                  numlin_visible = 1,
                  numcol_visible = 1,
                  width0 = 80,
                  width1 = 40,
                  expand = IUP_NO,
                  scrollbar = IUP_NO,
                  alignment0 = IUP_ACENTER
               }
               
k.CaseMatrix:setcell( 1, 0, "Selected Case" )

k.CaseMatrix.edition = function( self, lin, col, modo )
   return IUP_IGNORE
end
        

k.editionMinForceMatrix = function( self, lin, col, modo )
   return IUP_IGNORE
end



k.MinForceMatrix = unitmatrix
                   {
                      numlin = 1,
                      numcol = 3,
                      numlin_visible = 1,
                      numcol_visible = 3,
                      width1 = 60,
                      width2 = 60,
                      width3 = 60,
                      scrollbar = IUP_NO,
                      expand = IUP_NO,
                      edition = k.editionMinForceMatrix,
                      units = 
                      {
                         orientation = "col";
                         {"Fx", p3dUnit.force, "%.3f", p3dUnit.mask.float },
                         {"Fy", p3dUnit.force, "%.3f", p3dUnit.mask.float },
                         {"Mz", p3dUnit.moment, "%.3f", p3dUnit.mask.float },
                      }
                   }
                   
k.MinOutMatrix = unitmatrix
                 {
                    numlin = 2,
                    numcol = 4,
                    numlin_visible = 2,
                    numcol_visible = 4,
                    width0 = 60,
                    width1 = 60,
                    width2 = 60,
                    scrollbar = IUP_NO,
                    expand = IUP_NO,
                    units = 
                    {
                       orientation = "col";
                       {"Delta X"   , p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                       {"Delta Y"   , p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                       {"Delta Dist", p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                       {"Delta Rz"  , p3dUnit.angle       , "%.3f", p3dUnit.mask.float },
                    }
                 }
                 
p3dUnit.setValue( k.MinOutMatrix, 1, 0, "Previous Offset" )
p3dUnit.setValue( k.MinOutMatrix, 2, 0, "Current Offset" )


k.MinOutMatrix.edition = function( self, lin, col, modo )
   return IUP_IGNORE
end


--
-- Callback de Edicao da Matriz de Resultado
--
-- Todas as matrizes de resultado usam essa funcao
--
k.matrixResultEdition = function( self, lin, col )
   return IUP_IGNORE
end


--
-- Callback de Mousemove da Matriz de Resultado
--
-- Todas as matrizes de resultado usam essa funcao
--
k.matrixResultMousemove = function( self, lin, col )
   if( lin == 0 ) then
      if( col == 0 ) then
         self.cursor = IUP_ARROW
         self.tip = "Line Id"
      elseif( col == 1 ) then
         self.cursor = IUP_ARROW
         self.tip = "Previous Top Tension"
      elseif( col == 2 ) then
         self.cursor = IUP_ARROW
         self.tip = "Top Tension"
      elseif( col == 3 ) then
         self.cursor = IUP_ARROW
         self.tip = "Top Horizontal Tension"
      elseif( col == 4 ) then
         self.cursor = IUP_ARROW
         self.tip = "Top Vertical Tension"
      elseif( col == 5 ) then
         self.cursor = IUP_ARROW
         self.tip = "Top Angle"
      elseif( col == 6 ) then
         self.cursor = IUP_ARROW
         self.tip = "Line Azimuth"
      elseif( col == 7 ) then
         self.cursor = IUP_ARROW
         self.tip = "Percentage of Top Tension w.r.t. MBL"
      end
      return IUP_DEFAULT
   elseif( col == 0 and lin > 0 ) then
      self.cursor = IUP_ARROW
      self.tip = self.lineNames[lin]
      return IUP_DEFAULT
   else
      self.cursor = nil
      self.tip = nil
      return IUP_IGNORE
   end
end

--
-- Matriz de Resultado Principal
--
k.matrixResult = unitmatrix
                 {
                    numlin = 1,
                    numcol = 8,
                    numlin_visible = 10,
                    numcol_visible = 8,
                    widthdef = 60,
                    width0 = 30,
                    width1 = 75,
                    width3 = 70,
                    width4 = 65,
                    width5 = 70,
                    expand = IUP_NO,
                    resize = IUP_NO,
                    edition = k.matrixResultEdition,
                    mousemove = k.matrixResultMousemove,
                    units = 
                    {
                       orientation = "col";
                       {"Prev TopTen"    , p3dUnit.force , "%.3f", p3dUnit.mask.float },
                       {"TopTen"         , p3dUnit.force , "%.3f", p3dUnit.mask.float },
                       {"TopHTen"        , p3dUnit.force , "%.3f", p3dUnit.mask.float },
                       {"TopVTen"        , p3dUnit.force , "%.3f", p3dUnit.mask.float },
                       {"Top Angle"      , p3dUnit.angle , "%.3f", p3dUnit.mask.float },
                       {"Line Az."       , p3dUnit.angle , "%.3f", p3dUnit.mask.float },
                       {"On Board"       , p3dUnit.length, "%.3f", p3dUnit.mask.float },
                       {"% MBL"          , nil           , "%.2f", nil },
                    }
                 }

k.matrixResult:setcell( 0, 0, "Line Id" )


--
-- Matriz de Resultado do Minimize
--
k.matrixResultMinimize = unitmatrix
{
   numlin = 0,
   numcol = 8,
   numlin_visible = 5,
   numcol_visible = 8,
   widthdef = 60,
   width0 = 30,
   width1 = 75,
   width3 = 70,
   width4 = 65,
   width5 = 70,
   expand = IUP_NO,
   scrollbar = IUP_VERTICAL,
   edition = k.matrixResultEdition,
   mousemove = k.matrixResultMousemove,
   units = 
   {
      orientation = "col";
      {"Prev TopTen"    , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopTen"         , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopHTen"        , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopVTen"        , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"Top Angle"      , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
      {"Line Az."       , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
      {"Paid Out"       , p3dUnit.length, "%.3f", p3dUnit.mask.float },
      {"% MBL"          , nil           , "%.2f", nil },
   }
}
k.matrixResultMinimize:setcell( 0, 0, "Line Id" )


--
-- Matriz de Resultado do First Damaged
--
k.matrixResultFirstDam = unitmatrix
{
   numlin = 1,
   numcol = 8,
   numlin_visible = 10,
   numcol_visible = 8,
   widthdef = 60,
   width0 = 30,
   width1 = 75,
   width3 = 70,
   width4 = 65,
   width5 = 70,
   expand = IUP_NO,
   resize = IUP_NO,
   edition = k.matrixResultEdition,
   mousemove = k.matrixResultMousemove,
   units = 
   {
      orientation = "col";
      {"Prev TopTen"    , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopTen"         , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopHTen"        , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopVTen"        , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"Top Angle"      , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
      {"Line Az."       , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
      {"On Board"       , p3dUnit.length, "%.3f", p3dUnit.mask.float },
      {"% MBL"          , nil          , "%.2f", nil },
   }
}

k.matrixResultFirstDam:setcell( 0, 0, "Line Id" )
k.labelFirstDamaged = iuplabel{ title ="Damaged Line:", size = "300x", alignment = IUP_ACENTER}

--
-- Matriz de Resultado do Second Damaged
--
k.matrixResultSecondDam = unitmatrix
{
   numlin = 1,
   numcol = 8,
   numlin_visible = 10,
   numcol_visible = 8,
   widthdef = 60,
   width0 = 30,
   width1 = 75,
   width3 = 70,
   width4 = 65,
   width5 = 70,
   expand = IUP_NO,
   resize = IUP_NO,
   edition = k.matrixResultEdition,
   mousemove = k.matrixResultMousemove,
   units = 
   {
      orientation = "col";
      {"Prev TopTen"    , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopTen"         , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopHTen"        , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"TopVTen"        , p3dUnit.force, "%.3f", p3dUnit.mask.float },
      {"Top Angle"      , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
      {"Line Az."       , p3dUnit.angle, "%.3f", p3dUnit.mask.float },
      {"On Board"       , p3dUnit.length, "%.3f", p3dUnit.mask.float },
      {"% MBL"          , nil          , "%.2f", nil },
   }
}

k.matrixResultSecondDam:setcell( 0, 0, "Line Id" )
k.labelSecondDamaged = iuplabel{ title ="Damaged Line: ", size = "300x", alignment = IUP_ACENTER}

k.MinTypeBox = iupvbox
               {
                  iupfill{},
                  iuphbox
                  {
                     iupfill{},
                     iupframe
                     {
                        iupvbox
                        {
                           iupfill{ size = 10 },
                           iuphbox
                           {
                              iupfill{ size = 10 },
                              k.minimize_radio,
                              iupfill{ size = 10 }
                           },
                           iupfill{ size = 10 }
                        };
                        title = "Type of Minimization"
                     },
                     iupfill{},
                  },
                  iupfill{}
               }


                
k.MinButtonBox = iuphbox
                 {
                    iupfill{},
                    k.MinRun_button,
                    iupfill{ size = 20 },
                    k.MinOk_button,
                    iupfill{ size = 20 },
                    k.MinCancel_button,
                    iupfill{}
                 }


k.MinimizeBox = iuphbox
                {
                   iupfill{ },
                   iupvbox
                   {
                      iupfill{ },
                      iuphbox
                      {
                         iupfill{},
                         iupframe{ k.CaseMatrix },
                         iupfill{}
                      },
                      iupfill{ size = 10 },
                      k.MinTypeBox,
                      iupfill{ size = 10 },
                      iuphbox
                      {
                         iupfill{},
                         iupframe{ k.MinForceMatrix },
                         iupfill{}
                      },
                      iupfill{ size = 10 },
                      iupvbox
                      {
                         iupfill{},
                         iuphbox
                         {
                            iupfill{},
                            iupframe{ k.MinOutMatrix },
                            iupfill{}
                         },
                         iupfill{}
                      },
                      iupframe
                      { 
                         k.matrixResultMinimize;
                         title = "Results"
                      },
                      iupfill{ size = 20 },
                      k.MinButtonBox,
                      iupfill{ }
                   },
                   iupfill{}
                }

k.buttonBox = iuphbox
               {
                  iupfill{ },
                  k.run_button,
                  iupfill{ size = 30 },
                  k.minimize_button,
                  iupfill{ size = 30 },
                  k.apply_button,
                  iupfill{ size = 30 },
                  k.reportHTML_button,
                  iupfill{ size = 30 },
                  k.canvas_button,
                  iupfill{ size = 30 },
                  k.close_button,
                  iupfill{ }
               }


k.resultBox = iuphbox
              {
                 iupfill{},
                 k.matrixResult,
                 iupfill{};
                 tabtitle = "Offset"
              }

k.resultFirstDamBox = iuphbox
                      {
                         iupfill{},
                         iupvbox
                         {
                            iupfill{},
                            iuphbox
                            {
                               iupfill{},
                               k.labelFirstDamaged,
                               iupfill{},
                            },
                            iupfill{ size = 5 },
                            k.matrixResultFirstDam,
                            iupfill{}
                         },
                         iupfill{};
                         tabtitle = "First Damaged"
                      }

k.resultSecondDamBox = iuphbox
                       {
                          iupfill{},
                          iupvbox
                          {
                             iupfill{},
                             iuphbox
                             {
                                iupfill{},
                                k.labelSecondDamaged,
                                iupfill{},
                             },
                             iupfill{ size = 5 },
                             k.matrixResultSecondDam,
                             iupfill{}
                          },
                          iupfill{};
                          tabtitle = "Second Damaged"
                       }

k.resultTabChange = function( self, new_tab, old_tab )
   local t = p3dDlg.offset

   if( self.value == new_tab ) then
      return IUP_DEFAULT
   end
   
   self.value = new_tab

   if( new_tab == t.resultFirstDamBox ) then
      
      t.minimize_button.active = IUP_NO
      t.apply_button.active = IUP_NO

      local i = 1
      while ( i <= tonumber(t.numCase) ) do      
         t.SetData( i )
         i = i + 1
      end
      t.displayResult( 3, t.matrixResultFirstDam, (t.caseSelected - 1) )

   elseif( new_tab == t.resultSecondDamBox ) then
      t.minimize_button.active = IUP_NO
      t.apply_button.active = IUP_NO

      local i = 1
      while ( i <= tonumber(t.numCase) ) do      
         t.SetData( i )
         i = i + 1
      end
      t.displayResult( 4, t.matrixResultSecondDam, (t.caseSelected - 1) )
   elseif( new_tab == t.resultBox ) then

      local i = 1
      while ( i <= tonumber(t.numCase) ) do
         t.SetData( i )
         i = i + 1
      end

      t.minimize_button.active = IUP_YES
      t.apply_button.active = IUP_YES
      t.displayResult( 2, t.matrixResult, (t.caseSelected - 1) )

   end
   return IUP_DEFAULT
end

k.resultTab = iuptabs
              {
                 k.resultBox,
                 k.resultFirstDamBox,
                 k.resultSecondDamBox;
                 type = IUP_TOP,
                 value = k.resultBox,
                 tabchange = k.resultTabChange
              }


k.box = iuphbox
        {
           iupfill{ size = 8 },
           iupvbox
           {
              iupfill{size = 8},
              iuphbox
              {
                 iupfill{},
                 iupvbox
                 {
                    iupfill{},
                    iupframe
                    {
                       k.offset_radio;
                       title = "Calculation Method"
                    },
                    iupfill{}
                 },
                 iupfill{ size = 10 },
                 iupvbox
                 { 
                    iupfill{},
                    k.CalcBox,
                    iupfill{}
                 },
                 iupfill{}
              },
              iupfill{ size = 8 },
              iuphbox
              {
                 iupfill{},
                 iupframe{ k.caseNumMat },
                 iupfill{size = 15},
                 k.viewCharCuves,
                 iupfill{size = 60}
              },
              iupfill{ size = 5 },
              k.dataBox,
              iupfill{ size = 5 },
              iupframe
              {
              
                 k.resultTab;
                 title = "Results"
              },
              iupfill{ size = 5 },
              offsetLineGauge,
              iupfill{ size = 10 },
              k.buttonBox,
              iupfill{ size = 10 }
           },
           iupfill{ size = 8 }
        }
           
k.ZBox = iupzbox{ k.box, k.MinimizeBox }

k.dlg = iupdialog
        {
           k.ZBox;
           title = "Compute Offset",
           minbox = IUP_NO,
           maxbox = IUP_NO,
           resize = IUP_NO,
           icon = icon_dialog,
           parentdialog = MainDlg,
        }

k.dlg.close = function()
   return IUP_CLOSE
end

function co_saveObject( index )
   cfUndoPar( Objects[index]:save() )
end


function co_generalDataToC()
   local t = p3dDlg.offset
   return t.data.npoints, t.data.ndepths, t.data.delta, t.data.factor, t.data.methodCC, t.data.methodFA
end


function showOffsetDlgFromC()
   local t = p3dDlg.offset
      
   t.run_button.active = IUP_YES
   t.close_button.active = IUP_YES
   
   t.caseNumMat:setcell( 1, 1, t.numCase )
      
   t.Matrix.numlin = t.numCase

   local i = 1
   while( i <= t.numCase ) do
      t.ResetCases( i )
      t.SetData( i )
      i = i + 1   
   end
      
   p3dUnit.setValue( t.CalcMatrix, 1, 1, t.data.npoints )
   p3dUnit.setValue( t.CalcMatrix, 1, 2, t.data.ndepths )
   p3dUnit.setValue( t.CalcMatrix, 1, 3, t.data.delta   )
   p3dUnit.setValue( t.CalcMatrix, 1, 4, t.data.factor  )

   if ( t.data.methodCC == 1 ) then
      p3dUnit.setValue( t.CalcMatrix, 1, 5, "Catenary" )
   else
      p3dUnit.setValue( t.CalcMatrix, 1, 5, "FEM" )
   end
     
   if ( t.data.methodFA == 1 ) then
      p3dUnit.setValue( t.CalcMatrix, 1, 6, "Catenary" )
   else
      p3dUnit.setValue( t.CalcMatrix, 1, 6, "FEM" )
   end

   t.ZBox.value = t.box
   
   t.Matrix:enteritem( 1, 1 )
      
   offsetLineGauge.value = 0.0
      
   t.dlg:popup( IUP_CENTER, IUP_CENTER )
   
   return IUP_DEFAULT
   
end


k.SetData = function( i )
   local t = p3dDlg.offset
   local _, dx, dy, drz, fx, fy, mz, dist, status
   
   if( t.resultTab.value == t.resultBox ) then
      dx  = t.case[i].offsetDx
      dy  = t.case[i].offsetDy
      drz = t.case[i].offsetDrz
      dist = t.case[i].offsetDist
      status = t.case[i].offsetStatus
   elseif( t.resultTab.value == t.resultFirstDamBox ) then
      dx  = t.case[i].firstDamDx
      dy  = t.case[i].firstDamDy
      drz = t.case[i].firstDamDrz
      dist = t.case[i].firstDamDist
      status = t.case[i].firstDamStatus
   elseif( t.resultTab.value == t.resultSecondDamBox ) then
      dx  = t.case[i].secondDamDx
      dy  = t.case[i].secondDamDy
      drz = t.case[i].secondDamDrz
      dist = t.case[i].secondDamDist
      status = t.case[i].secondDamStatus
   else
      dx  = 0
      dy  = 0
      drz = 0
      dist = 0
      status = "--"
   end
   
   fx, fy, _  = cfpTransf( t.case[i].fx, t.case[i].fy, 0)
   _ , _, mz  = cfpTransf( 0, 0, t.case[i].mz )
   dx, dy, _  = cfpTransf( dx, dy, 0)
   _ , _, drz = cfpTransf( 0, 0, drz )
   
   p3dUnit.setValue( t.Matrix, i, 0, i   )
   p3dUnit.setValue( t.Matrix, i, 1, fx  )
   p3dUnit.setValue( t.Matrix, i, 2, fy  )
   p3dUnit.setValue( t.Matrix, i, 3, mz  )
   p3dUnit.setValue( t.Matrix, i, 4, dx  )
   p3dUnit.setValue( t.Matrix, i, 5, dy  )
   p3dUnit.setValue( t.Matrix, i, 6, dist )
   p3dUnit.setValue( t.Matrix, i, 7, drz  )
   p3dUnit.setValue( t.Matrix, i, 8, status )
   
   t.Matrix.redraw = IUP_YES
end


-- Carregando as unidades da matrix
-----------------------------------
k.unitCalcMatrix      = {}
k.unitMatrix          = {}
k.unitMinForceMatrix  = {}
k.unitMinOutMatrix    = {}
k.unitMatrixResult    = {}
k.unitMatrixResultMin = {}
k.unitMatrixResultFirstDam  = {}
k.unitMatrixResultSecondDam = {}

p3dUnit.initObject( k.unitCalcMatrix )
p3dUnit.initObject( k.unitMatrix )
p3dUnit.initObject( k.unitMinForceMatrix )
p3dUnit.initObject( k.unitMinOutMatrix )
p3dUnit.initObject( k.unitMatrixResult )
p3dUnit.initObject( k.unitMatrixResultMin )
p3dUnit.initObject( k.unitMatrixResultFirstDam )
p3dUnit.initObject( k.unitMatrixResultSecondDam )

p3dUnit.setSelectedUnits( k.CalcMatrix, k.unitCalcMatrix )
p3dUnit.setSelectedUnits( k.Matrix, k.unitMatrix )
p3dUnit.setSelectedUnits( k.MinForceMatrix, k.unitMinForceMatrix )
p3dUnit.setSelectedUnits( k.MinOutMatrix, k.unitMinOutMatrix )
p3dUnit.setSelectedUnits( k.matrixResult, k.unitMatrixResult )
p3dUnit.setSelectedUnits( k.matrixResultMinimize, k.unitMatrixResultMin )
p3dUnit.setSelectedUnits( k.matrixResultFirstDam , k.unitMatrixResultFirstDam  )
p3dUnit.setSelectedUnits( k.matrixResultSecondDam, k.unitMatrixResultSecondDam )

p3dUnit.setValue( k.CalcMatrix, 0, 0, "" )
p3dUnit.setValue( k.Matrix, 0, 0, "" )
p3dUnit.setValue( k.MinForceMatrix, 0, 0, "" )
p3dUnit.setValue( k.MinOutMatrix, 0, 0, "" )



p3dDlg.offset.dlg:map()
k.caseSelected = 1
k.offsetType = k.CHC
k.resultTab.value = k.resultBox

------------------------------------------------------------
-- Canvas de resultado
------------------------------------------------------------


k.ZoomIn  = iupbutton
{
   title = "ZI",
   image = icon_zoomin,
   impress = icon_zoominPress,
   tip = "Zoom In",
   color = BUTTON_CL,
   action = function( self )
      OffsetZoomIn()
   end
}

k.ZoomOut = iupbutton
{
   title = "ZO",
   image = icon_zoomout,
   impress = icon_zoomoutPress,
   tip = "Zoom Out",
   color = BUTTON_CL,
   action = function( self )
      OffsetZoomOut()
   end
}

k.ZoomWnd = iupbutton
            {
               title = "ZW",
               image = icon_zoomwin,
               impress = icon_zoomwinPress,
               tip = "Zoom Window",
               color = BUTTON_CL,
               action = function( self )
                  OffsetZoomWnd()
               end
            }

k.Fit = iupbutton
        {
           title = "FIT",
           image = icon_fit,
           impress = icon_fitPress,
           tip = "Fit",
           color = BUTTON_CL,
           action = function( self )
              OffsetFit()
           end
        }


k.closeCanvasButton = iupbutton{ title = "Close"   , size = BUTTON_SZ }

k.closeCanvasButton.action = function( self )
   return IUP_CLOSE
end

k.matrixResultCanvas = unitmatrix
           {
              numlin = 1,
              numcol = 7,
              numlin_visible = 1,
              numcol_visible = 7,
              width0 = 30,
              widthdef = 60,
              width6 = 65,
              scrollbar = IUP_NO,
              expand = IUP_NO,
              alignment0 = IUP_CENTER,
              alignmentdef = IUP_CENTER,
              BGCOLOR = NoEditBcolor,
              units = 
              {
                 orientation = "col";
                 {"Fx", p3dUnit.force, "%.3f", p3dUnit.mask.float },
                 {"Fy", p3dUnit.force, "%.3f", p3dUnit.mask.float },
                 {"Mz", p3dUnit.moment, "%.3f", p3dUnit.mask.float },
                 {"Delta X" , p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                 {"Delta Y" , p3dUnit.displacement, "%.3f", p3dUnit.mask.float },
                 {"Delta Distance" , p3dUnit.length, "%.3f", p3dUnit.mask.float },
                 {"Delta Rz" , p3dUnit.angle, "%.3f", p3dUnit.mask.float }
              },
              edition = function( self, lin, col, mode )
                 return IUP_IGNORE
              end
           }


k.matrixResultCanvas:setcell( 0, 0, "Case" )

k.unitMatrixResultCanvas = {}
p3dUnit.initObject( k.unitMatrixResultCanvas )
p3dUnit.setSelectedUnits( k.matrixResultCanvas, k.unitMatrixResultCanvas )

k.listCanvas = iuplist{ title="Select Case", dropdown = IUP_YES, size="60x" }
k.labelListCanvas = iuplabel{ title="Select Case:"  }

k.listCanvas.action = function( self, s, i, v )
   local  t = p3dDlg.offset
   if( v == 1 ) then
      t.setMatrixResultCanvas( s )
   end   
end

k.setListCanvas = function ()
   local t = p3dDlg.offset
   local i = 1
   local j = 1
   t.listCanvas[1] = nil
   while( i <= tonumber(t.numCase) ) do
      if( t.case[i].offsetStatus == "Ok" ) then
         t.listCanvas[j] = "Case "..i
         j = j + 1
      end
      i = i + 1
   end
end

k.setMatrixResultCanvas = function( str )
   local t = p3dDlg.offset
   local _, dx, dy, drz, fx, fy, mz, dist
   local i
   
   if( str == nil or type(str) ~= "string" ) then
      p3dUnit.setValue( t.matrixResultCanvas, 1, 0,  0  )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 1, 0.0  )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 2, 0.0  )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 3, 0.0  )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 4, 0.0  )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 5, 0.0  )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 6, 0.0 )
      p3dUnit.setValue( t.matrixResultCanvas, 1, 7, 0.0  )
      t.matrixResultCanvas.redraw = IUP_YES
      return
   end
   
   _, _, i = strfind( str, "(%d+)" )
   i = tonumber(i)
   
   OffsetSetSelectCase( i )
   
   fx, fy, _  = cfpTransf( t.case[i].fx, t.case[i].fy, 0)
   _ , _, mz  = cfpTransf( 0, 0, t.case[i].mz )
   dx, dy, _  = cfpTransf( t.case[i].offsetDx, t.case[i].offsetDy, 0)
   _ , _, drz = cfpTransf( 0, 0, t.case[i].offsetDrz )
   
   p3dUnit.setValue( t.matrixResultCanvas, 1, 0, i   )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 1, fx  )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 2, fy  )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 3, mz  )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 4, dx  )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 5, dy  )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 6, t.case[i].offsetDist )
   p3dUnit.setValue( t.matrixResultCanvas, 1, 7, drz  )
   
   t.matrixResultCanvas.redraw = IUP_YES
   
end

k.boxListCanvas = iupvbox
                  {
                     iupfill{},
                     iuphbox
                     {
                        iupvbox
                        {
                           iupfill{},
                           k.labelListCanvas,
                           iupfill{}
                        },
                        iupfill{ size = 5 },
                        k.listCanvas
                     },
                     iupfill{}
                  }

k.transparencyVal = iupval
                   {
                      IUP_HORIZONTAL;
                      min = 0,
                      max = 1,
                      value = 0.2,
                      mousemove = function( self, val )
                         local t = 1 - val
                         OffsetSetTransparency( t )
                      end,
                      buttonrelease = function( self, val )
                         local t = 1 - val
                         OffsetSetTransparency( t )
                      end
                   }

k.transparencyLabel = iuplabel{ title = "Transparency:" }

k.boxTransparency = iupvbox
                    {
                       iupfill{},
                       iuphbox
                       {
                          iupvbox
                          {
                              iupfill{},
                              k.transparencyLabel,
                              iupfill{}
                          },
                          iupfill{ size = 5 },
                          k.transparencyVal
                       },
                       iupfill{}
                    }

k.canvasButtonsBox = iuphbox
                     {
                        iupfill{},
                        iupframe{ k.boxTransparency },
                        iupfill{ size = 10},
                        iupframe{ k.boxListCanvas },
                        iupfill{ size = 10 },
                        k.ZoomIn,
                        k.ZoomOut,
                        k.ZoomWnd,
                        k.Fit,
                        iupfill{ size = 15 },
                        k.closeCanvasButton,
                        iupfill{ size = 5 }
                     }

function offsetCreateCanvas( cnv )
   local dlg
   local t = p3dDlg.offset
   dlg = iupdialog
         {
            iupvbox
            {
               iupfill{ size = 2 },
               t.canvasButtonsBox,
               iupfill{ size = 2 },
               cnv,
               iupfill{ size = 2 },
               iupframe
               {
                  t.matrixResultCanvas
               }
            };
            minbox = IUP_NO,
            icon = icon_dialog,
            parentdialog = t.dlg,
            title = "Offset - Top View"
         }
   IupMap( dlg )
   t.canvasResult = dlg   
end



k = p3dDlg.offset.oldK

