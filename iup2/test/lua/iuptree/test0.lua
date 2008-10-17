dofile("TableTree.lua")

-- The first element of each table must be the title of the new branch
TableTree({
            "ROOT",
            {
               "2D",
               "pentagono",
               "quadrado",
               {
                 "triangulo","isosceles","escaleno","equilatero"
               },
               "retangulo" ,
               "losango"
            },
            {
              "3D",
              "Paraboloide hiperbolico",
              "Esfera"
            }
          })
