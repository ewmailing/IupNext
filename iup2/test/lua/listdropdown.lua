l = iup.list{
    "A", "B", "C";
--    dropdown = "yes",
     multiple = "yes",
    action = function(self, t, i, v)
print("action")
      --print("t="..t.." i="..i.." v="..v.." value="..self.value)
      --print(self.value .." = " .. self[self.value])
    end
}

a = iup.vbox{iup.fill{},iup.hbox{iup.fill{},l,iup.fill{}},iup.fill{}}

a.multiselect_cb = function(ih, s)
      print("s="..s) --.." value="..ih.value)
      --print(self.value .." = " .. self[self.value])
    end

d = iup.dialog{ a }

d:show()
