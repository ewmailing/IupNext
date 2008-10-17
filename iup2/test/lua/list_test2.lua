list = iup.list{
        "primeiro",
        "segundo",
        "terceiro",
        dropdown = "YES",
        visible_items = 5,
        editbox = "YES",
        expand = "HORIZONTAL",
}

function list:edit_cb(c, after)
  print(c, after)
end

function show_bug()
        list[1] = "primeiro"
        list[2] = "segundo"
        list[3] = "terceiro"
        list.value = "terceiro"
        print(list.value == "terceiro", "O valor do atributo é '"..list.value.."'")
end


local dialog = iup.dialog{
        title = "IupList Bug";
        iup.hbox{
                list,
                iup.button{
                        title = "Show Bug!",
                        action = show_bug,
                },
        },
}

dialog:popup(iup.CENTER, iup.CENTER) 