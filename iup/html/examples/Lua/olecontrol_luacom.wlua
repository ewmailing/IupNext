-- IpuOle and LuaCom example contributed by Kommit

require "iuplua"
require "iupluaole"
require "luacom"

local control = iup.olecontrol{"COMCTL.ListViewCtrl"} -- this control works, but lacks many function
--local control = iup.olecontrol{"MsComCtlLib.ListViewCtrl"}

-- connect it to LuaCOM
control:CreateLuaCOM()

-- Sets production mode
control.designmode= "NO"

control.bgcolor = "0 0 0"  -- if control is not showing, you will see a black window...

-- Create a dialog containing the OLE control

local dlg = iup.dialog{
    title="IupOle",
    size="HALFxHALF",
    iup.vbox{
    control,
    }
}

-- Show the dialog and run the main loop
dlg:show()

control.com.View = 3  -- Report View

-- control.com.FullRowSelect = true -- this property is only support by MsComCtlLib
control.com.ColumnHeaders:Add(nil, nil, "Header 1")
control.com.ColumnHeaders:Add(nil, nil, "Header 2")
control.com.ListItems:Add(nil, nil, "Value 1")
control.com.ListItems:Add(nil, nil, "Value 3")
-- following operations are only support by MsComCtlLib
-- control.com.ListItems:Item(1).ListSubItems:Add(nil, nil, "Value2")
-- control.com.ListItems:Item(2).ListSubItems:Add(nil, nil, "Value4")


-- add events to to the control
list_events = {}

function list_events:ColumnClick(column)
    control.com.Sorted = 1

    iCur = column.Index - 1
    if iCur == iLast then
        if control.com.SortOrder == 0 then
            control.com.SortOrder = 1
        else
            control.com.SortOrder = 0
        end
    end
    control.com.SortKey = iCur
    iLast = iCur
end

luacom.Connect(control.com, list_events)



if (not iup.MainLoopLevel or iup.MainLoopLevel()==0) then
  iup.MainLoop()
end

