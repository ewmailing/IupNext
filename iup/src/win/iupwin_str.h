/** \file
 * \brief Windows String Processing
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPWIN_STR_H 
#define __IUPWIN_STR_H

#ifdef __cplusplus
extern "C" {
#endif


int iupwinSetUTF8Mode(void);

char* iupwinStrFromSystem(const TCHAR* str);
TCHAR* iupwinStrToSystem(const char* str);
TCHAR* iupwinStrToSystemLen(const char* str, int len);

void* iupwinStrGetMemory(int size);

WCHAR* iupwinStrChar2Wide(const char* str);
char* iupwinStrWide2Char(const WCHAR* wstr);

void iupwinStrCopy(TCHAR* dst_wstr, const char* src_str, int max_size);

int iupwinGetWindowText(HWND hWnd, LPSTR lpString, int nMaxCount);


#ifdef __cplusplus
}
#endif

#endif
