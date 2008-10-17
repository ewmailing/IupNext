/** \file
 * \brief Windows Driver windows procedures.
 *
 * See Copyright Notice in iup.h
 * $Id: winproc.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WINPROC_H 
#define __WINPROC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Prototipos de funcoes exportadas */

#define WM_TRAY_NOTIFICATION (WM_USER+102)
#define ID_TRAYICON     1000

void iupwinFinish(void);
void iupwinSetFocus(Ihandle* n);
LRESULT CALLBACK iupwinDialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK iupwinCanvasProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int iupwinButtonCallCb(HWND hwnd, Ihandle *n);
void iupwinCallEnterLeaveWindow(Ihandle *h, int enter);
int iupwinCloseChildren(Ihandle* client);

/* in winedit.c */
LRESULT CALLBACK iupwinEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

/* in wincombo.c */
LRESULT CALLBACK iupwinComboListProc(HWND hWnd, UINT uMessage, WPARAM uParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif
