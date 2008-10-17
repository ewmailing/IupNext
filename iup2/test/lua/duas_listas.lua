local l1 = iuplist{ dropdown="YES" }
l1["1"] = "l1a"
l1["2"] = "l1b"
l1["3"] = "l1c"

local l2 = iuplist
{ 
   dropdown = "YES", 
   action = function(iupself,t,i,v) 
      print("l2: ",v, "    ", %l1[%l1.value]) 
   end 
}

l2["1"] = "l2a"
l2["2"] = "l2b"
l2["3"] = "l2c"

l1.action = function(iupself,t,i,v) 
  print("l1:",v, "   ",%l2[%l2.value]) 
end

dlg = iupdialog{
   iuphbox{ 
      l1,
      l2,
   };
   title = "teste"
}

IupShow(dlg)

