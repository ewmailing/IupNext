        Curvas = iuptree {
                expand = "VERTICAL",
                rastersize = "200x",
                ctrl = "YES",
                shift = "YES"
        }

        -- Variável temporária para construir a IupTree:
        local cTree = {
                {
                        {
                                "teste",
                                "teste2",
                                "teste3";
                                branchname = "Bo"
                        },
                        {
                                branchname = "Bg"
                        },
                        {
                                branchname = "uo"
                        };
                        branchname = "Plug"
                }
        }
        TreeSetValue( Curvas, cTree )
        
        -- CALLBACK:
        Curvas.selection = function ( self, id, status )
                print( id )
        end
        
        Curvas.rightclick = function ( self, id, status )
                print( id )
        end

        Curvas.branchopen = function ( self, id, status )
                print( id )
        end

        
        -- DIÁLOGO:
        dlg = iupdialog {
                Curvas
        }
        
        dlg:show()
        
        IupMainLoop()
        
        dlg:destroy()