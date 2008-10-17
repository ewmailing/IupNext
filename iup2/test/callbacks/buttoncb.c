#include <stdio.h>
#include <iup.h>


int TesteCB(Ihandle* self)
{
        return IUP_DEFAULT;
}


int TesteBtCB(Ihandle* self, int but, int pressed, 
        int x, int y, char* status)
{
        if (but != IUP_BUTTON3)
                return IUP_DEFAULT;


        if (pressed == 1)
                ;// IupMessage("Mensagem", "Pressionado");
        else
                IupMessage("Mensagem", "Não pressionado");


        return IUP_DEFAULT;
}


int PontoCB(Ihandle* self)
{
        return IUP_DEFAULT;
}


int LinhaCB(Ihandle* self)
{
        return IUP_DEFAULT;
}


int AreaCB(Ihandle* self)
{
        return IUP_DEFAULT;
}


int main()
{
        Ihandle* dlg = NULL;
        Ihandle* btn = NULL;
        Ihandle* mnu = NULL;
        
        IupOpen();

        btn = IupButton("Teste", "TesteCB");
        IupSetAttribute(btn, IUP_BUTTON_CB, "TesteBtCB");
        IupSetFunction("TesteCB", (Icallback) TesteCB);
        IupSetFunction("TesteBtCB", (Icallback) TesteBtCB);


        mnu = IupMenu(IupItem("Ponto", "PontoCB"),
                IupItem("Linha", "LinhaCB"),
                IupItem("Área", "AreaCB"),
                IupSeparator(),
                IupItem("Cancelar", "CancelarCB"),
                NULL);
        IupSetFunction("PontoCB", (Icallback) PontoCB);
        IupSetFunction("LinhaCB", (Icallback) LinhaCB);
        IupSetFunction("AreaCB", (Icallback) AreaCB);


        dlg = IupDialog(btn);
        IupPopup(dlg, IUP_CENTER, IUP_CENTER);


        IupClose();

        return 0;
} 
