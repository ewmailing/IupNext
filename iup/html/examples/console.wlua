require( "iuplua" )

-- Utilities
iupluacmd = {}

function iupluacmd.printtable(t)
  local n,v = next(t, nil)
  print("--printtable Start--")
  while n ~= nil do
    print(tostring(n).."="..tostring(v))
    n,v = next(t, n)
  end
  print("--printtable End--")
end

function iupluacmd.print_version_info()
  if (im) then print("IM " .. im._VERSION .. " " .. im._COPYRIGHT) end
  if (cd) then print("CD " .. cd._VERSION .. " " .. cd._COPYRIGHT) end
  print("IUP " .. iup._VERSION .. " " .. iup._COPYRIGHT)
  print("")
  print("IUP Info")
  print("  System: " .. iup.GetGlobal("SYSTEM"))
  print("  System Version: " .. iup.GetGlobal("SYSTEMVERSION"))
  local mot = iup.GetGlobal("MOTIFVERSION")
  if (mot) then print("  Motif Version: ", mot) end
  print("  Screen Size: " .. iup.GetGlobal("SCREENSIZE"))
  print("  Screen Depth: " .. iup.GetGlobal("SCREENDEPTH"))
  if (iup.GL_VENDOR) then print("  OpenGL Vendor: " .. iup.GL_VENDOR) end
  if (iup.GL_RENDERER) then print("  OpenGL Renderer: " .. iup.GL_RENDERER) end
  if (iup.GL_VERSION) then print("  OpenGL Version: " .. iup.GL_VERSION) end
end

-- IUPLUA Full Application  

iupluacmd.lastfilename = nil -- Last file open
iupluacmd.mlCode = iup.multiline{expand="YES", size="200x120", font="COURIER_NORMAL_10"}   
iupluacmd.lblPosition = iup.label{title="0:0", size="50x"} 
iupluacmd.lblFileName = iup.label{title="", size="50x", expand="HORIZONTAL"} 

function iupluacmd.mlCode:caret_cb(lin, col)
   iupluacmd.lblPosition.title = lin..":"..col
end

iupluacmd.butExecute = iup.button{size="50x15", title="Execute", 
                                  action="iup.dostring(iupluacmd.mlCode.value)"}
iupluacmd.butClearCommands = iup.button{size="50x15", title="Clear", 
                                        action="iupluacmd.mlCode.value=''  iupluacmd.lblFileName.title = ''  iupluacmd.lastfilename = nil"}
iupluacmd.butLoadFile = iup.button{size="50x15", title="Load..."}
iupluacmd.butSaveasFile = iup.button{size="50x15", title="Save As..."}
iupluacmd.butSaveFile = iup.button{size="50x15", title="Save"}

iupluacmd.butSaveFile.action = function()
   if (iupluacmd.lastfilename == nil) then
      iupluacmd.butSaveasFile:action() 
   else
      newfile = io.open(iupluacmd.lastfilename, "w+")
      if (newfile ~= nil) then
         newfile:write(iupluacmd.mlCode.value)
         newfile:close() 
      else
         error ("Cannot Save file "..filename)
      end
   end
end

iupluacmd.butSaveasFile.action = function()
   local fd = iup.filedlg{dialogtype="SAVE", title="Save File", 
                          filter="*.lua", filterinfo="Lua files",allownew=yes}
   fd:popup(iup.LEFT, iup.LEFT)
   local status = fd.status
   iupluacmd.lastfilename = fd.value
   iupluacmd.lblFileName.title = iupluacmd.lastfilename
   fd:destroy()
   if status ~= "-1" then 
      if (iupluacmd.lastfilename == nil) then
         error ("Cannot Save file "..filename)
      end
      local newfile=io.open(iupluacmd.lastfilename, "w+")
      if (newfile ~= nil) then
         newfile:write(iupluacmd.mlCode.value)
         newfile:close(newfile)
      else
         error ("Cannot Save file")
      end
   end
end

iupluacmd.butLoadFile.action = function ()
   local fd=iup.filedlg{dialogtype="OPEN", title="Load File", 
                        filter="*.lua;*.wlua", filterinfo="Lua Files", allownew="NO"}
   fd:popup(iup.CENTER, iup.CENTER)
   local status = fd.status
   local filename = fd.value
   fd:destroy()
   if (status == "-1") or (status == "1") then 
      if (status == "1") then
         error ("Cannot load file "..filename)
      end
   else
      local newfile = io.open (filename, "r")
      if (newfile == nil) then
         error ("Cannot load file "..filename)
      else
         iupluacmd.mlCode.value=newfile:read("*a") 
         newfile:close (newfile) 
         iupluacmd.lastfilename = filename
         iupluacmd.lblFileName.title = iupluacmd.lastfilename
      end
   end
end

iupluacmd.vbxConsole = iup.vbox 
{
   iup.frame{iup.hbox{iup.vbox{iupluacmd.butLoadFile, 
                               iupluacmd.butSaveFile, 
                               iupluacmd.butSaveasFile, 
                               iupluacmd.butClearCommands, 
                               iupluacmd.butExecute; 
                               margin="0x0", gap="10"}, 
                      iup.vbox{iupluacmd.lblFileName, 
                               iupluacmd.mlCode, 
                               iupluacmd.lblPosition; 
                               alignment = "ARIGHT"}; 
                      alignment="ATOP"}; title="Commands"}
   ;alignment="ACENTER", margin="5x5", gap="5" 
}

-- Main Menu Definition.

iupluacmd.mnuMain = iup.menu
{
   iup.submenu
   {
      iup.menu
      {
          iup.item{title="Exit", action="return iup.CLOSE"}
      }; title="File"
   },
   iup.submenu{iup.menu
   {
      iup.item{title="Print Version Info...", action=iupluacmd.print_version_info},
      iup.item{title="About...", action="iupluacmd.dlgAbout:popup(iup.CENTER, iup.CENTER)"}
   };title="Help"}
}

-- Main Dialog Definition.

iupluacmd.dlgMain = iup.dialog{iupluacmd.vbxConsole; 
                               title="IupLua Console", 
                               menu=iupluacmd.mnuMain, 
                               defaultenter=iupluacmd.butExecute,
                               close_cb = "return iup.CLOSE"}

-- About Dialog Definition.

iupluacmd.dlgAbout = iup.dialog 
{
   iup.vbox
   {
      iup.label{title="IupLua5 Console"}, 
      iup.fill{size="5"},
      iup.fill{size="5"},
      iup.frame
      {
          iup.vbox
          {
              iup.label{title="Tecgraf/PUC-Rio"},
              iup.label{title="iup@tecgraf.puc-rio.br"} 
          }
      },
      iup.fill{size="5"},
      iup.button{title="OK", action="return iup.CLOSE", size="50X20"} 
      ;margin="10x10", alignment="ACENTER" 
   }
   ;maxbox="NO", minbox="NO", resize="NO", title="About"
}

-- Displays the Main Dialog 

iupluacmd.dlgMain:show()
iup.SetFocus(iupluacmd.mlCode)

iup.MainLoop()

iupluacmd.dlgMain:destroy()
iupluacmd.dlgAbout:destroy()
