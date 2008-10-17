
im = iup.image 
{
{2,1,1,1,1,1,1,2},
{1,2,1,1,1,1,2,1},
{1,1,2,1,1,2,1,1},
{1,1,1,2,2,1,1,1},
{1,1,1,2,2,1,1,1},
{1,1,2,1,1,2,1,1},
{1,2,1,1,1,1,2,1},
{2,1,1,1,1,1,1,2};
 colors = 
 {
 "192 192 192",
 "255 255 255",
 }
}

lb = iup.label
     { 
       title = "", 
       image = im 
     }

hb = iup.hbox{ iup.fill{}, lb, iup.fill{}; tabtitle = "Teste" }
tb = iup.tabs{ hb }

dl = iup.dialog{ tb }

dl : popup( iup.CENTER, iup.CENTER )

