local options = {
        "Option 1",
        "Option 2",
        "Option 3",
        "Option 4",
        "Option 5",
}
local r = IupListDialog(1, "Title", 5, options, 0, 25, 10, { 1, 0, 0, 0, 0 })
print(r, type(r))
if type(r) == "table" then
    for key, value in pairs(r) do
      print("[" .. tostring(key) .. "] = " .. tostring(value))
    end
end
