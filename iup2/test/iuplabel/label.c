
#include <stdio.h>
#include <iup.h>

int main2 (void) 
{
  Ihandle *hdlg;
  IupOpen(&argc, &argv);

   hdlg = IupDialog(IupVbox(
                              IupFrame(
                                     IupVbox(
                                       IupSetAttributes(IupFill(), "SIZE=3"),
                                       IupSetAttributes(IupLabel(
                                                "SAAAP\n"
                                                "\n"
                                                "Sistema de Avaliação Ambiental\n"
                                                "de Alternativas de Percurso\n"
                                                "\n"
                                                "\n"
                                                "Versão 3.0 beta\n"
                                                "\n\n\n 2005"), "EXPAND = YES, ALIGNMENT=ACENTER"),
//                                       IupLabel("2004"),
                                       IupSetAttributes(IupFill(), "SIZE=3"),
                                       NULL)
                              ),
                              IupSetAttributes(IupFill(), "SIZE=3"),
                              IupHbox(
                                IupFill(),
                                IupSetAttributes(IupButton("OK", "AboutOkCB"), "EXPAND=NO, SIZE=50x14"),
                                IupFill(),
                                NULL
                              ),
                              NULL
                           ));
    IupSetAttribute(hdlg, IUP_MINBOX, IUP_NO);
    IupSetAttribute(hdlg, IUP_MAXBOX, IUP_NO);
    IupSetAttribute(hdlg, IUP_RESIZE, IUP_NO);
    IupSetAttribute(hdlg, IUP_MARGIN, "2x2");
    IupSetAttribute(hdlg, IUP_EXPAND, IUP_YES);
    IupSetAttribute(hdlg, IUP_SIZE, "150");

  IupShow(hdlg);
  
  IupMainLoop ();
  IupDestroy(hdlg);
  IupClose ();
  return 0;
}

int main(int argc, char **argv) 
{
  Ihandle *dlg, *lb1, *lb2, *lb3, *lb4;
  IupOpen(&argc, &argv);

  lb1 = IupLabel("Esse é um texto\nCheio de Enters\nSó pra dar problema");
  lb2 = IupLabel("abba");
  lb3 = IupLabel("a\nb\n");
  lb4 = IupLabel("\nnormal\n\n");
  dlg = IupDialog(IupVbox(IupFrame(lb1), IupFrame(lb2), IupFrame(lb3), IupFrame(lb4), NULL));
  IupShow(dlg);
  
  IupMainLoop ();
  IupDestroy(dlg);
  IupClose ();
  return 0;
}

