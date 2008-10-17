$debug
------------------------------------------------
-- Projeto Prea3d
--
-- File: dlg-linedb.lua
--
-- Dialogo para selecao de caracteristicas dos
-- segmentos
--
------------------------------------------------
---------------------------------------
-- $Id: dlg-linedb.lua,v 1.1 2008-10-17 06:21:41 scuri Exp $
--
-- $Revision: 1.1 $
---------------------------------------

-- Funcoes para manipulacao do dialogo  
--                                     
-- 1) Parametros das funcoes:

-- material   : indice na lista de materiais
--              (o indice corresponde a ordem dos materiais no arquivo)
-- forceUnit  : lineDB.kN, lineDB.N, lineDB.kgf, lineDB.ton
-- lengthUnit : lineDB.in, lineDB.cm, lineDB.mm, lineDB.m
-- sortedBy   : indice correspondente a uma das propriedades utilizadas para ordenar o BD
--              Nominal Diameter   1
--              Min. Breaking Load 2
--              EA                 3
--              WIA                4
--              WIW                5
-- value      : valor numerico usado para a procura

-- 2) Prototipo das funcoes:
--
-- lineDB_updateDialog( material, forceUnit, lengthUnit, sortedBy, value )
--
-- material, forceUnit, lengthUnit, sortedBy, value = lineDB_getLastUpdate( )
--
-- n = lineDB_getMaterialNumber()
--
-- diamNom, diamHid, wia, wiw, cd, cm, ea, mbl, fric_s, fric_d = 
-- lineDB_findEntry( material, forceUnit, lengthUnit, sortedBy, value )
--
--

------------------------------------------------

lineDB                 = {}
lineDB.lastLine        = 1
lineDB.lastCol         = 2
lineDB.strings         = {}

-- constantes * NAO MODIFICAR * 
-- podem ser usadas com as funcoes exportadas para Lua

lineDB.kN  = 1
lineDB.N   = 2
lineDB.kgf = 3
lineDB.ton = 4
lineDB.in  = 4
lineDB.cm  = 2
lineDB.mm  = 3
lineDB.m   = 1
--lineDB.propconv = {"1"=2,"2"=6,"3"=7,"4"=4,"5"=5}

------------------------------------------------

lineDB.oldK = k;

k = lineDB;

------------------------------------------------
-- Funcoes

function lineDB_getMaterialName( index )
   local str
   str = lineDB.material[tostring(index+1)]

   if str == nil then 
      error("lineDB_getMaterialName: invalid parameter.")
   end

   return str
end

function lineDB_materialNameToIndex( name )
   return lineDB.materialToIndex[name]
end
------------------------------------------------

k.dataMatrix = iupmatrix
               {
                  height0=20,
                  width0=25,  alignment0=right,
                  width1=40,  alignment1=right,
                  width2=40,  alignment2=right,
                  width3=40,  alignment3=right,
                  width4=45,  alignment4=right,
                  width5=45,  alignment5=right,
                  width6=65,  alignment6=right,
                  width7=65,  alignment7=right,
                  width8=45,  alignment8=right,
                  width9=45,  alignment9=right,
                  width10=35, alignment10=right,
                  width11=35, alignment11=right,
                  bgcolor="255 255 255",
                  numcol=11,
                  numcol_visible = 9,
                  numlin=0,
                  numlin_visible = 11,
                  scrollbar = "YES",
                  cursor="ARROW",
                  expand = "NO"
               }

-- Diam Nom - Diam Hid - Wia - Wiw - MBL - EA - Cd - Cm - AT_e - AT_d

--k.dataMatrix:setcell( 0, 1, "Name"       )
--k.dataMatrix:setcell( 0, 2, "Nom. Diam." )
--k.dataMatrix:setcell( 0, 3, "Hid. Diam." )
--k.dataMatrix:setcell( 0, 4, "WIA"        )
--k.dataMatrix:setcell( 0, 5, "WIW"        )
--k.dataMatrix:setcell( 0, 6, "MBL"        )
--k.dataMatrix:setcell( 0, 7, "EA"         )
--k.dataMatrix:setcell( 0, 8, "CD"         )
--k.dataMatrix:setcell( 0, 9, "CM"         )
--k.dataMatrix:setcell( 0, 10, "Fric_S"    )
--k.dataMatrix:setcell( 0, 11, "Fric_D"    )

k.LSIZE     = "40x"
k.TSIZE     = "120x"
k.TSIZEL    = "90x"
k.TSIZELL   = "50x"
k.BUTTON_SZ = "60X20"
k.White = "255 255 255"
k.Gray  = "150 150 150"

k.propertyLabel = iuplabel{ size = k.LSIZE, title = "Sort By:" }

k.property = iuplist
             {
                "Nominal Diameter",
                "Min. Breaking Load",
                "Axil Rigidity (EA)",
                "Weight in Air",
                "Weight in Water";
                dropdown = IUP_YES,
                size = k.TSIZEL
             }

k.propertyVLabel = iuplabel{ size = k.LSIZE, title = "Value:" }

k.propertyValue = iuptext{ size = k.TSIZEL }

k.materialLabel = iuplabel { size = k.LSIZE, title = "Material:"    }

k.material = iuplist 
             {
                "EMPTY LIST";
                dropdown = IUP_YES,
                size = k.TSIZE
             }


k.unitsLabel = iuplabel { size = k.LSIZE, title = "Units:" }

-- forca:  N, kN, kgf, ton
-- compr:  mm, cm, m

lineDB.lastUnitForce  = 1;
lineDB.lastUnitLength = 1;

k.unitForce = iuplist
              {
                 "kN",
                 "N", 
                 "kgf",
                 "ton";
                  dropdown = IUP_YES,
                  size = k.TSIZELL
              }

k.unitLength = iuplist
               {
                  "m",
                  "cm",
                  "mm", 
                  "in"; 
                  dropdown = IUP_YES,
                  size = k.TSIZELL
               }

k.Ok     = iupbutton { size = k.BUTTON_SZ, title = "Apply"  }
k.Cancel = iupbutton { size = k.BUTTON_SZ, title = "Cancel" }


------------------------------------------------
-- Inicializando as variaveis do dialogo

lineDB.material.value = "1"
lineDB.property.value = "1"
lineDB.propertyValue.value = "1"
lineDB.unitForce.value = "1"
lineDB.unitLength.value = "1"

------------------------------------------------

k.box = iuphbox
        {
           iupfill{ size = 5 },
           iupvbox
           {
              iupfill{ size = 5 },
              iupframe
              {
                 iupvbox
                 {
                    iupfill{ size = 5 },
                    iuphbox
                    {
                       iupfill{ size = 5 },
                       k.dataMatrix,
                       iupfill{ size = 5 }
                    },
                    iupfill{ size = 5 }                 
                 };
                 title = "Properties"
              },
              iupfill{ size = 5 },
              iuphbox
              {
                 iupframe
                 {
                    iuphbox
                    {
                       iuphbox
                       {
                          iupfill{ size = 3 },
                          iupframe{iupvbox
                          {
                            iuphbox{k.propertyVLabel,iupfill{},k.propertyValue},
                            iuphbox{k.propertyLabel, iupfill{},k.property};
                          }},
                          iuphbox
                          {
                           iupframe{iuphbox
			   {
			      k.unitsLabel,
			      iupvbox{ k.unitForce,
			               iupfill{ size = 3 },
			               k.unitLength
                                     };
			   }},
                           iupframe{iuphbox
			   {
			      k.materialLabel,
			      k.material;
			   }};
                          },
                          iupfill{ size = 5 }
                       }
                    };
                    title = "Search"
                 },
                 iupfill{ size = 5 },
                 iupvbox
                 {
                    iupframe
                    {
                       iupvbox
                       {
                          iupfill{ size = 2 },
                          iuphbox
                          {
                             iupfill{ size = 30 },
                             iuplabel{ fgcolor="0 0 255",
                                       title = "* Petrobras Standard" },
                             iupfill{}
                          },
                          iupfill{}
                       };
                       title = "Legend"
                    },
                    iupfill{ size = 3 },
                    iuphbox
                    {
                       iupfill{},
                       k.Ok,
                       iupfill{ size = 30 },
                       k.Cancel,
                    }
                 },
              },
              iupfill{ size = 5 }
           },
           iupfill{ size = 5 }
        }

k.dlg = iupdialog
        {
           k.box;
           title = "Line Segment Properties",
           resize = IUP_NO,
           minbox = IUP_NO,
           maxbox = IUP_NO,
           icon="bitcenpes",
           parent="linesdlg"
        }

------------------------------------------------

function loadMaterialTypes( numTypes )
   local t, i
    
    t = lineDB.strings
    lineDB.materialToIndex = {}
    
    i = 0
    while i < numTypes do
       lineDB.material[ i+1 ] = t[ i ]
       lineDB.materialToIndex[ t[i] ] = i
       i = i+1
    end
end

k.property.action = function ( self, t, i, v ) 
   if v ==  1 then 
      lineDBHighlightColumn( )
      SincronizePropertyValue(lineDB.lastLine)

--      cfUpdateLineDB( tonumber( lineDB.material.value )-1, 
--                      i, 
--                      lineDB.dataMatrix,
--                      tonumber( lineDB.propertyValue.value ),
--                      tonumber( lineDB.unitForce.value ),
--                      tonumber( lineDB.unitLength.value ) );
   end
end

function lineDBHighlightColumn()
   local col
   if lineDB.property.value=="1" then		-- Nominal Diameter
    col=2
   elseif lineDB.property.value=="2" then	-- Min. Breaking Load
    col=6
   elseif lineDB.property.value=="3" then	-- EA
    col=7
   elseif lineDB.property.value=="4" then	-- WIA
    col=4
   elseif lineDB.property.value=="5" then	-- WIW
    col=5
   end

   local lin=1
   while (lin<=tonumber(lineDB.dataMatrix.numlin)) do
    if (lin~=lineDB.lastLine) then
     lineDB.dataMatrix[format("bgcolor%d:%d",lin,lineDB.lastCol)] = lineDB.White
     lineDB.dataMatrix[format("bgcolor%d:%d",lin,col)]            = lineDB.Gray
    end
    lin=lin+1
   end
   lineDB.lastCol = col;   
end

k.unitForce.action = function ( self, t, i, v ) 
   if v ==  1 then

      num = cfUpdtSearchValue( tonumber( lineDB.propertyValue.value ),
                               lineDB.lastUnitForce,
                               lineDB.lastUnitLength,
                               tonumber( lineDB.property.value ),
                               tonumber( lineDB.unitForce.value     ),
                               tonumber( lineDB.unitLength.value    ) );
                                     
      lineDB.lastUnitForce  = tonumber( lineDB.unitForce.value );
      lineDB.lastUnitLength = tonumber( lineDB.unitLength.value);

      lineDB.propertyValue.value = num;

      cfUpdateLineDB( tonumber( lineDB.material.value )-1, 
                      lineDB.property.value, 
                      lineDB.dataMatrix,
                      tonumber( lineDB.propertyValue.value ),
                      i,
                      tonumber( lineDB.unitLength.value ) );
      lineDBHighlightColumn()
   end
end

k.unitLength.action = function ( self, t, i, v ) 

   if v ==  1 then 

      num = cfUpdtSearchValue( tonumber( lineDB.propertyValue.value ),
                               lineDB.lastUnitForce,
                               lineDB.lastUnitLength,
                               tonumber( lineDB.property.value ),
                               tonumber( lineDB.unitForce.value     ),
                               tonumber( lineDB.unitLength.value    ) );
                                     
      lineDB.lastUnitForce  = tonumber( lineDB.unitForce.value );
      lineDB.lastUnitLength = tonumber( lineDB.unitLength.value);

      lineDB.propertyValue.value = num;


      cfUpdateLineDB( tonumber( lineDB.material.value )-1, 
                      lineDB.property.value, 
                      lineDB.dataMatrix,
                      tonumber( lineDB.propertyValue.value ),
                      tonumber( lineDB.unitForce.value ) ,
                      i );
      lineDBHighlightColumn()
   end
end

k.material.action = function ( self, t, i, v )

   if v ==  1 then 
      print("material.action: "..i )
      cfUpdateLineDB( i-1, 
                      lineDB.property.value, 
                      lineDB.dataMatrix,
                      tonumber( lineDB.propertyValue.value ),
                      tonumber( lineDB.unitForce.value ),
                      tonumber( lineDB.unitLength.value ) );
      lineDBHighlightColumn()
   end
end

k.propertyValue.action = function( self, c, after)

   --if c == K_CR then 
      --lineDB.propertyValue.value=tonumber(after) 
      if (tonumber(lineDB.propertyValue.value)~=
          tonumber(after)) then
       cfUpdateLineDB( tonumber( lineDB.material.value )-1, 
                       lineDB.property.value, 
                       lineDB.dataMatrix,
                       tonumber( after ),
                       tonumber( lineDB.unitForce.value ),
                       tonumber( lineDB.unitLength.value ) );
       lineDBHighlightColumn()
     end
   ----end
end

k.dataMatrix.edition = function ( self, lin, col, modo )
   return IUP_IGNORE
end

function SincronizePropertyValue(lin)
    if lineDB.property.value=="1" then		-- Nominal Diameter
     lineDB.propertyValue.value = lineDB.dataMatrix:getcell(lin,2)
    elseif lineDB.property.value=="2" then	-- Min. Breaking Load
     lineDB.propertyValue.value = lineDB.dataMatrix:getcell(lin,6)
    elseif lineDB.property.value=="3" then	-- EA
     lineDB.propertyValue.value = lineDB.dataMatrix:getcell(lin,7)
    elseif lineDB.property.value=="4" then	-- WIA
     lineDB.propertyValue.value = lineDB.dataMatrix:getcell(lin,4)
    elseif lineDB.property.value=="5" then	-- WIW
     lineDB.propertyValue.value = lineDB.dataMatrix:getcell(lin,5)
    end
end
    
k.dataMatrix.enteritem = function ( self, lin, col )
   --if lin == lineDB.lastLine then
   --   return IUP_IGNORE
   --end

   SincronizePropertyValue(lin)
   lineDBHighlight( lin )
   return IUP_DEFAULT
end

function lineDBHighlight( line )
   local col=1
   while (col<=tonumber(lineDB.dataMatrix.numcol)) do
    if (col~=lineDB.lastCol) then
     lineDB.dataMatrix[format("bgcolor%d:%d",lineDB.lastLine,col)]=lineDB.White
     lineDB.dataMatrix[format("bgcolor%d:%d",line,col)]           =lineDB.Gray
    end
    col=col+1
   end
   lineDB.lastLine = line;   
end

-- Atualiza o banco a matriz de sgemntos
k.Ok.action = function ()
	lineDB.canceled = 0
   return IUP_CLOSE
end
     
k.Cancel.action = function ()
   lineDB.canceled = 1
   return IUP_CLOSE
end

-- funcao de chamada do dialog e retorno dos valores da linha selecionada
-------------------------------------------------------------------------
lineDB.showDlg = function ()

	local l_idx

   --cfUpdateLineDB( 0, 0, lineDB.dataMatrix, 0, 1, 1 );

   cfUpdateLineDB( tonumber( lineDB.material.value ), 
                   tonumber( lineDB.property.value ), 
                   lineDB.dataMatrix,
                   tonumber( lineDB.propertyValue.value ),
                   tonumber( lineDB.unitForce.value ),
                   tonumber( lineDB.unitLength.value ) );

   lineDBHighlightColumn( )
   

	lineDB.dlg:popup(IUP_CENTER,IUP_CENTER)

	l_idx = lineDB.lastLine


   if lineDB.canceled == 1 then -- se o usuario cancelou
		return nil
   else
      return 1,
      tonumber( lineDB.dataMatrix:getcell(l_idx,1) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,2) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,3) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,4) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,5) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,6) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,7) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,8) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,9) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,10) ),
      tonumber( lineDB.dataMatrix:getcell(l_idx,11) ),
      lineDB.material[ lineDB.material.value ];

	end
end

lineDB.preaShowDlg = function ()
	local l_idx

   lineDBHighlightColumn( )
   
	lineDB.dlg:popup(IUP_CENTER,IUP_CENTER)

	l_idx = lineDB.lastLine

   if lineDB.canceled == 1 then -- se o usuario cancelou
		return nil
   else
      local mat, force, length, sort, v, data

      mat, force, length, sort, v = lineDB_getLastUpdate()
      
      -- lineDB_getEntry tem o segundo parametro indexado de 0!
      data = lineDB_getEntry( mat, l_idx-1 )

      return 1, 
      data.diamNom, 
      data.diamHid, 
      data.wia, 
      data.wiw, 
      data.mbl, 
      data.ea, 
      data.cd,
      data.cm,
      data.fric_s,
      data.fric_d,
      tonumber(lineDB.material.value)-1;
             
      -- sequencia de operacoes para colocar dados no banco de dados
      --lineDB_getLastUpdate()
      --lineDB_findEntry()
      --lineDB_userUnits() -- prea nao precisa disso ainda! so seta nas unidades default do prea
	end
end


------------------------------------------------

function MapLineDBDialog()
 IupMap(lineDB.dlg)
end

k = lineDB.oldK;
lineDB.oldK = nil;
lineDB.dataMatrix["bgcolor*:2"]="150 150 150"
--IupMap(lineDB.dlg);
