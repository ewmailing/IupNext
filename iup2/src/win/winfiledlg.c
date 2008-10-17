/** \file
 * \brief Windows Driver IupFileDlg
 *
 * See Copyright Notice in iup.h
 * $Id: winfiledlg.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "iup.h"
#include "iupcpi.h"
#include "iglobal.h"
#include "icpi.h"
#include "winproc.h"
#include "winhandle.h"
#include "win.h"
#include "winhook.h"


#define MAX_FILENAME_SIZE 65000


/********************************************************************
*** Permite a selecao de diretorios, retornando ao usuario o valor
*** selecionado atraves do atributo IUP_VALUE. O dialogo sempre 
*** inicia no diretorio raiz do desktop. O atributo IUP_TITLE e
*** utilizado para mostrar um texto explicativo no interior do 
*** dialogo.
filedlg = iupfiledlg {dialogtype = "DIR", title = "Select Image Folder", directory = "d:\\"} 
filedlg:popup (x, y)
********************************************************************/

static INT CALLBACK winFileDlgBrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  if (uMsg == BFFM_INITIALIZED)
  {
    Ihandle* h = (Ihandle*)lpData;
    char* directory = IupGetAttribute(h, IUP_DIRECTORY);
    int x = IupGetInt(h, "_SH_X");
    int y = IupGetInt(h, "_SH_Y");
    RECT rect;
    GetWindowRect(hwnd, &rect);
    iupwinAdjustPos(h, &x, &y, rect.right-rect.left, rect.bottom-rect.top);
    SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE); 
    SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)directory);
  }
  return 0;
}

static void winFileDlgGetFolder(Ihandle *h)
{
  char dirname[MAX_FILENAME_SIZE+1];
  BROWSEINFO bi;
  HWND hwnd;
  char *buffer = NULL;
  ITEMIDLIST *selecteditem;
  char *title = iupStrDup( IupGetAttribute(h, IUP_TITLE) );
  
  hwnd = GetActiveWindow();

  buffer = (char*)malloc(sizeof(char)*MAX_PATH);
  assert(buffer);
  if(!buffer)
    return;

  ZeroMemory(&bi, sizeof(BROWSEINFO));
  bi.hwndOwner = hwnd;
  bi.lpszTitle = title;
  bi.pszDisplayName = buffer; 
  bi.lpfn = winFileDlgBrowseCallback;
  bi.lParam = (LPARAM)h;
  bi.ulFlags = BIF_NEWDIALOGSTYLE;

  selecteditem = SHBrowseForFolder(&bi);

  free(buffer); 

  if(!selecteditem)
  {
    IupStoreAttribute(h, IUP_VALUE, NULL);
    IupStoreAttribute(h, IUP_STATUS, "-1");
    if(title) free(title);
    return;
  }

  SHGetPathFromIDList(selecteditem, dirname);
  IupStoreAttribute(h, IUP_VALUE, dirname);
  IupStoreAttribute(h, IUP_STATUS, "0");

  if(title) free(title);
}

#define IUP_PREVIEWCANVAS 3000

static UINT_PTR CALLBACK winFileDlgSimpleHook(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uiMsg)
  {
  case WM_INITDIALOG:
    {
      OPENFILENAME* ofn = (OPENFILENAME*)lParam;
      Ihandle* h = (Ihandle*)ofn->lCustData;
      int x = IupGetInt(h, "_SH_X");
      int y = IupGetInt(h, "_SH_Y");
      RECT rect;
      GetWindowRect(GetParent(hDlg), &rect);
      iupwinAdjustPos(h, &x, &y, rect.right-rect.left, rect.bottom-rect.top);
      SetWindowPos(GetParent(hDlg), HWND_TOP, x, y, 0, 0, SWP_NOSIZE); 
      break;
    }
  }
  return 0;
}

static UINT_PTR CALLBACK winFileDlgHook(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uiMsg)
  {
  case WM_INITDIALOG:
    {
      OPENFILENAME* ofn = (OPENFILENAME*)lParam;
      Ihandle* h = (Ihandle*)ofn->lCustData;
      HWND hWnd = GetDlgItem(hDlg, IUP_PREVIEWCANVAS);
      int x = IupGetInt(h, "_SH_X");
      int y = IupGetInt(h, "_SH_Y");
      RECT rect;
      GetWindowRect(GetParent(hDlg), &rect);
      iupwinAdjustPos(h, &x, &y, rect.right-rect.left, rect.bottom-rect.top);
      SetWindowPos(GetParent(hDlg), HWND_TOP, x, y, 0, 0, SWP_NOSIZE); 
      if (hWnd)
      {
        RECT rect;
        GetClientRect(hWnd, &rect);

        IupSetfAttribute(h, "PREVIEWWIDTH", "%d", rect.right - rect.left);
        IupSetfAttribute(h, "PREVIEWHEIGHT", "%d", rect.bottom - rect.top);
      }
      SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)h);
      break;
    }
  case WM_DRAWITEM:
    {
      if (wParam == IUP_PREVIEWCANVAS)
      {
        LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
        Ihandle* h = (Ihandle*)GetWindowLongPtr(hDlg, DWLP_USER);
        IFnss FileCallback = (IFnss)IupGetCallback(h, "FILE_CB");
        char filename[MAX_PATH];
        IupSetAttribute(h, "PREVIEWDC", (char*)lpDrawItem->hDC);
        if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
            filename, MAX_PATH) <= MAX_PATH)
        {
          DWORD attrib = GetFileAttributes(filename);
          if (attrib & FILE_ATTRIBUTE_DIRECTORY)
            FileCallback(h, NULL, "PAINT");
          else
            FileCallback(h, filename, "PAINT");
        }
        else
          FileCallback(h, NULL, "PAINT");
        IupSetAttribute(h, "PREVIEWDC", NULL);
      }
      break;
    }
  case WM_SIZE:
    {
      HWND hWnd = GetDlgItem(hDlg, IUP_PREVIEWCANVAS);
      if (hWnd)
      {
        int old_width;
        Ihandle* h = (Ihandle*)GetWindowLongPtr(hDlg, DWLP_USER);
        old_width = IupGetInt(h, "_IUPWIN_OLD_WIDTH");
        if (old_width)
        {
          int height, width, ypos, xpos;
          RECT rect, dlgrect;
          GetWindowRect(hWnd, &rect);
          GetWindowRect(hDlg, &dlgrect);
          ypos = rect.top - dlgrect.top;
          xpos = rect.left - dlgrect.left;
          GetWindowRect(GetParent(hDlg), &dlgrect);
          width = rect.right-rect.left + (dlgrect.right-dlgrect.left)-old_width;
          height = rect.bottom-rect.top;
          SetWindowPos(hWnd, hDlg, xpos, ypos, width, height, SWP_NOZORDER);
          GetClientRect(hWnd, &rect);
          IupSetfAttribute(h, "PREVIEWWIDTH", "%d", rect.right-rect.left);
          IupSetfAttribute(h, "_IUPWIN_OLD_WIDTH", "%d", dlgrect.right-dlgrect.left);
          RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
        else
        {
          RECT dlgrect;
          GetWindowRect(GetParent(hDlg), &dlgrect);
          IupSetfAttribute(h, "_IUPWIN_OLD_WIDTH", "%d", dlgrect.right-dlgrect.left);
        }
      }
      break;
    }
  case WM_DESTROY:
    {
      Ihandle* h = (Ihandle*)GetWindowLongPtr(hDlg, DWLP_USER);
      IFnss FileCallback = (IFnss)IupGetCallback(h, "FILE_CB");
      FileCallback(h, NULL, "FINISH");
      break;
    }
  case WM_NOTIFY:
    {
      LPOFNOTIFY pofn = (LPOFNOTIFY)lParam;
      Ihandle* h = (Ihandle*)pofn->lpOFN->lCustData;
      IFnss FileCallback = (IFnss)IupGetCallback(h, "FILE_CB");
      switch (pofn->hdr.code)
      {
      case CDN_INITDONE:
        {
          HWND hWnd = GetDlgItem(hDlg, IUP_PREVIEWCANVAS);
          FileCallback(h, NULL, "INIT");
          if (hWnd) RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
          break;
        }
      case CDN_FILEOK:
      case CDN_SELCHANGE:
        {
          HWND hWnd = GetDlgItem(hDlg, IUP_PREVIEWCANVAS);
          char filename[MAX_PATH];
          if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg), filename, MAX_PATH) <= MAX_PATH)
          {
            int ret;
            char* file_msg;
            DWORD attrib = GetFileAttributes(filename);
            if (attrib & FILE_ATTRIBUTE_DIRECTORY)
              break;

            if (pofn->hdr.code == CDN_FILEOK)
              file_msg = "OK";
            else
              file_msg = "SELECT";

            ret = FileCallback(h, filename, file_msg);
            if (pofn->hdr.code == CDN_FILEOK && ret == IUP_IGNORE) 
            {
              SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 1L);
              return 1; /* will refuse the file */
            }
          }
          if (pofn->hdr.code == CDN_SELCHANGE && hWnd) 
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
          break;
        }
      }
      break;
    }
  }
  return 0;
}

static void checkPathSlash(char* name)
{
  /* check for "/" */
  int i, len = strlen(name);
  for (i = 0; i < len; i++)
  {
    if (name[i] == '/')
      name[i] = '\\';
  }
}

static int winFileDlgPopup(Ihandle *h, int x, int y)
{
  OPENFILENAME ofn;
  int result=0;
  char *buffer    = NULL,
       *filter    = NULL,
       *extfilter = NULL,
       *directory = NULL,
       *title     = NULL,
       *info      = NULL;
  char *type      = IupGetAttribute(h, IUP_DIALOGTYPE);
  int dialogtype = (type && iupStrEqual(type, IUP_SAVE))? 1: (type && iupStrEqual(type, IUP_DIR))? -1: 0;
  Ihandle* parent;

  IupSetfAttribute(h, "_SH_X", "%d", x);
  IupSetfAttribute(h, "_SH_Y", "%d", y);

  /* Escolha de diretorio */
  if( dialogtype == -1  )
  {
    iupwinUnhook();
    winFileDlgGetFolder(h);
    iupwinRehook();
    return IUP_NOERROR;
  }

  /* Escolha de arquivo */
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  parent = IupGetAttributeHandle(h, IUP_PARENTDIALOG);
  if (parent && handle(parent))
    ofn.hwndOwner = handle(parent);
  else
    ofn.hwndOwner = GetActiveWindow();

  filter    = iupStrDup( IupGetAttribute(h, IUP_FILTER)     );
  info      = iupStrDup( IupGetAttribute(h, IUP_FILTERINFO) );
  extfilter = IupGetAttribute(h, IUP_EXTFILTER);

  if(extfilter != NULL)
  {
    int i=0;
    buffer = (char *) malloc(strlen(extfilter)+2);
    assert(buffer != NULL);
    strcpy(buffer, extfilter);
    assert(buffer[strlen(extfilter)] == 0);

    while(1) /* Substitui os simbolos "|" pelo terminador "0" - exigencia do windows */
    {
      if(buffer[i] == 0)
      {
        buffer[i+1] = 0;
        break;
      }
      if(buffer[i] == '|')
        buffer[i] = 0;
      i++;
    }
    ofn.lpstrFilter = buffer;

  }
  else if(filter != NULL)
  {
    if(info == NULL)
      info = iupStrDup( filter );
     
    buffer = (char *) malloc(strlen(filter)+1+strlen(info)+2);
    assert(buffer != NULL);

    strcpy(buffer, info);
    strcpy(buffer+strlen(info)+1, filter);
    buffer[strlen(info)+1+strlen(filter)+1] = '\0';
    ofn.lpstrFilter = buffer;
  }

  if(ofn.lpstrFilter != NULL)
  {
    char *v = IupGetAttribute(h, IUP_FILTERUSED);
    if(v)
      ofn.nFilterIndex = atoi(v);
    else
      ofn.nFilterIndex = 1;
  }

  ofn.lpstrFile = (char *) malloc(MAX_FILENAME_SIZE+1);

  {
    char *file_name = IupGetAttribute(h, IUP_FILE);

    if (file_name)
    {
      strncpy(ofn.lpstrFile, file_name, MAX_FILENAME_SIZE);
      checkPathSlash(ofn.lpstrFile);
    }
    else
      ofn.lpstrFile[0] = '\0';
  }

  ofn.nMaxFile = MAX_FILENAME_SIZE;

  directory = iupStrDup( IupGetAttribute(h, IUP_DIRECTORY) );
  ofn.lpstrInitialDir = directory;
  if (directory)
    checkPathSlash((char*)ofn.lpstrInitialDir);

  title = iupStrDup( IupGetAttribute(h, IUP_TITLE) );
  ofn.lpstrTitle = title;

  ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

  if(iupCheck(h, IUP_NOOVERWRITEPROMPT)!=YES)
    ofn.Flags |= OFN_OVERWRITEPROMPT;

  switch(iupCheck(h, IUP_ALLOWNEW))
  {
  case YES:
    ofn.Flags |= OFN_CREATEPROMPT;
    break;
  case NO:
    ofn.Flags |= OFN_FILEMUSTEXIST;
    break;
  default:
    if (dialogtype == 0)
      ofn.Flags |= OFN_FILEMUSTEXIST;
    else
      ofn.Flags |= OFN_CREATEPROMPT;
     break;
  }

  if(iupCheck(h, IUP_NOCHANGEDIR))
    ofn.Flags |= OFN_NOCHANGEDIR;

  if(iupCheck(h, IUP_MULTIPLEFILES) == YES)
     ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;

  {
    char *showprev;
    IFnss FileCallback = (IFnss)IupGetCallback(h, "FILE_CB");
    ofn.lpfnHook = NULL;
    if (FileCallback)
      ofn.lpfnHook = winFileDlgHook;
    else if (x != IUP_CURRENT && y != IUP_CURRENT)
      ofn.lpfnHook = winFileDlgSimpleHook;

    if (ofn.lpfnHook)
    {
      ofn.Flags |= OFN_ENABLEHOOK | OFN_EXPLORER | OFN_ENABLESIZING;
      ofn.lCustData = (LPARAM)h;
    }

    showprev = IupGetAttribute(h, "SHOWPREVIEW");
    if (showprev && iupStrEqual(showprev, IUP_YES))
    {
      ofn.Flags |= OFN_ENABLETEMPLATE;
      ofn.hInstance = iupwin_hinstance;
      ofn.lpTemplateName = "iupPreviewDlg";
    }
  }

  {
    iupwinUnhook();

    if( dialogtype == 0 )
      result = GetOpenFileName(&ofn);
    else
      result = GetSaveFileName(&ofn);

    iupwinRehook();
  }

  if (result)
  {
    struct stat buf;

    if (stat(ofn.lpstrFile,&buf) == 0)  /* arquivo existe */
    {
      IupSetAttribute(h, IUP_FILEEXIST, IUP_YES);
      IupSetAttribute(h, IUP_STATUS, "0");
    }
    else
    {
      IupSetAttribute(h, IUP_FILEEXIST, IUP_NO);
      IupSetAttribute(h, IUP_STATUS, "1");
    }

    if(iupCheck(h, IUP_MULTIPLEFILES) == YES)
    {
      int i = 0;
      
      /* If we are talking about more than one file... */
      if(ofn.lpstrFile && ofn.lpstrFile[ofn.nFileOffset-1] == 0 && ofn.nFileOffset>0) 
      {
        while(ofn.lpstrFile[i] != 0 || ofn.lpstrFile[i+1] != 0)
        {
          if(ofn.lpstrFile[i]==0)
            ofn.lpstrFile[i] = '|';
          i++;
        }
        ofn.lpstrFile[i] = '|';
      }
    }

    IupStoreAttribute(h, IUP_VALUE, ofn.lpstrFile);

    IupSetfAttribute(h, IUP_FILTERUSED, "%d", (int) ofn.nFilterIndex);
  }
  else
  {
    IupSetAttribute(h, IUP_VALUE, NULL);
    IupSetAttribute(h, IUP_FILEEXIST, NULL);
    IupSetAttribute(h, IUP_STATUS, "-1");
  }

  if(buffer)        free(buffer);
  if(ofn.lpstrFile) free(ofn.lpstrFile);
  if(filter)        free(filter);
  if(info)          free(info);
  if(title)         free(title);
  if(directory)     free(directory);
  
  return IUP_NOERROR;
}

/*
 * Exported functions
 */

Ihandle *IupFileDlg(void)
{
  return IupCreate("filedlg");
}

void IupFileDlgOpen(void)
{
  Iclass *ic = iupCpiCreateNewClass("filedlg", NULL);
  iupCpiSetClassMethod(ic, ICPI_POPUP, (Imethod) winFileDlgPopup);
}
