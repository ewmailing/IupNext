/** \file
 * \brief Windows Unicode Encapsulation
 *
 * See Copyright Notice in "iup.h"
 */
#include <windows.h>
#include <commctrl.h>

#include <stdio.h>              
#include <stdlib.h>
#include <string.h>             

#include "iupwin_str.h"


/* From MSDN:
- Internally, the ANSI version translates the string to Unicode. 
- The ANSI versions are also less efficient, 
  because the operating system must convert the ANSI strings to Unicode at run time. 

  So, there is no point in doing a lib that calls both ANSI and Unicode API. 
  In the simple and ideal world we need only Unicode windows and handle the convertion when necessary.
  The same convertion would exist anyway if using the ANSI API.
- By default, file I/O functions use ANSI file names. 
  But if your files have Unicode names, then you may consider using UTF-8 
  so later the aplication can recover the original Unicode version.
*/
static int iupwin_utf8mode = 1;  // somente para facilitar o uso dos testes existentes, depois remover
//static int iupwin_utf8mode = 0;    /* default is NOT using UTF-8 */
static int iupwin_utf8mode_file = 0;  

/* TODOUTF8:
1) procurar no código do iupwin outras possiveis falhas
2) Testar arquivos com "çã" em FAT e NTFS
3) clipboard com UTF-8 ou Unicode?
4) Attributos CARET e SELECTION do IupText/IupList como indexam a string?
   Como é no GTK?
5) Tentar juntar a edição de texto do IupText e do IupList 
   usando funções genéricas que encapsulem ambos, 
   tentar passar como parametro coisas que são diferentes em ambos
-------------------------------
- outros controles
    iupglcanvas
    iupolecontrol
    iupscintilla
    iupweb
- suporte para UTF8 no driver Win32 do CD
- melhorar o suporte a teclas no IUP 
*/

int iupwinSetUTF8Mode(int utf8mode)
{
#ifdef UNICODE
  iupwin_utf8mode = utf8mode;
  return 1;
#else
  return 0;
#endif
}

int iupwinSetUTF8ModeFile(int utf8mode)
{
#ifdef UNICODE
  iupwin_utf8mode_file = utf8mode;
  return 1;
#else
  return 0;
#endif
}

static void winStrWide2Char(const WCHAR* wstr, char* str, int len)
{
  if (iupwin_utf8mode)
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, len, str, 2*len, NULL, NULL);  /* str must has a large buffer */
  else
    len = WideCharToMultiByte(CP_ACP, 0, wstr, len, str, 2*len, NULL, NULL);

  if (len>0)
    str[len] = 0;
}

static void winStrChar2Wide(const char* str, WCHAR* wstr, int len)
{
  if (iupwin_utf8mode)
    len = MultiByteToWideChar(CP_UTF8, 0, str, len, wstr, len);
  else
    len = MultiByteToWideChar(CP_ACP, 0, str, len, wstr, len);

  if (len>0)
    wstr[len] = 0;
}

WCHAR* iupwinStrChar2Wide(const char* str)
{
  if (str)
  {
    int len = (int)strlen(str);
    WCHAR* wstr = (WCHAR*)malloc((len+1) * sizeof(WCHAR));
    winStrChar2Wide(str, wstr, len);
    return wstr;
  }

  return NULL;
}

char* iupwinStrWide2Char(const WCHAR* wstr)
{
  if (wstr)
  {
    int len = (int)wcslen(wstr);
    char* str = (char*)malloc((2*len+1) * sizeof(char));   /* str must has a large buffer */
    winStrWide2Char(wstr, str, len);
    return str;
  }

  return NULL;
}

void iupwinStrCopy(TCHAR* dst_wstr, const char* src_str, int max_size)
{
  if (src_str)
  {
    TCHAR* src_wstr = iupwinStrToSystem(src_str);
    int len = lstrlen(src_wstr);
    if (len > max_size) len = max_size;
    lstrcpyn(dst_wstr, src_wstr, len);
  }
}

TCHAR* iupwinStrToSystemFilename(const char* str)
{
  TCHAR* wstr;
  int old_utf8mode = iupwin_utf8mode;
  iupwin_utf8mode = iupwin_utf8mode_file;
  wstr = iupwinStrToSystem(str);
  iupwin_utf8mode = old_utf8mode;
  return wstr;
}

char* iupwinStrFromSystemFilename(const TCHAR* wstr)
{
  char* str;
  int old_utf8mode = iupwin_utf8mode;
  iupwin_utf8mode = iupwin_utf8mode_file;
  str = iupwinStrFromSystem(wstr);
  iupwin_utf8mode = old_utf8mode;
  return str;
}

TCHAR* iupwinStrToSystem(const char* str)
{
#ifdef UNICODE
  if (str)
  {
    int len = (int)strlen(str);
    WCHAR* wstr = (WCHAR*)iupwinStrGetMemory((len+1) * sizeof(WCHAR));
    winStrChar2Wide(str, wstr, len);
    return wstr;
  }
  return NULL;
#else
  return (char*)str;
#endif
}

char* iupwinStrFromSystem(const TCHAR* wstr)
{
#ifdef UNICODE
  if (wstr)
  {
    int len = (int)wcslen(wstr);
    char* str = (char*)iupwinStrGetMemory((2*len+1) * sizeof(char));    /* str must has a large buffer */
    winStrWide2Char(wstr, str, len);
    return str;
  }
  return NULL;
#else
  return (char*)wstr;
#endif
}

TCHAR* iupwinStrToSystemLen(const char* str, int len)
{
#ifdef UNICODE
  if (str)
  {
    WCHAR* wstr = (WCHAR*)iupwinStrGetMemory((len+1) * sizeof(WCHAR));
    winStrChar2Wide(str, wstr, len);
    return wstr;
  }
  return NULL;
#else
  (void)len;
  return (char*)str;
#endif
}

void* iupwinStrGetMemory(int size)
{
#define MAX_BUFFERS 10
  static void* buffers[MAX_BUFFERS];
  static int buffers_sizes[MAX_BUFFERS];
  static int buffers_index = -1;

  int i;

  if (size == -1) /* Frees memory */
  {
    buffers_index = -1;
    for (i = 0; i < MAX_BUFFERS; i++)
    {
      if (buffers[i]) 
      {
        free(buffers[i]);
        buffers[i] = NULL;
      }
      buffers_sizes[i] = 0;
    }
    return NULL;
  }
  else
  {
    void* ret_buffer;

    /* init buffers array */
    if (buffers_index == -1)
    {
      memset(buffers, 0, sizeof(void*)*MAX_BUFFERS);
      memset(buffers_sizes, 0, sizeof(int)*MAX_BUFFERS);
      buffers_index = 0;
    }

    /* first alocation */
    if (!(buffers[buffers_index]))
    {
      buffers_sizes[buffers_index] = size+1;
      buffers[buffers_index] = malloc(buffers_sizes[buffers_index]);
    }
    else if (buffers_sizes[buffers_index] < size+1)  /* reallocate if necessary */
    {
      buffers_sizes[buffers_index] = size+1;
      buffers[buffers_index] = realloc(buffers[buffers_index], buffers_sizes[buffers_index]);
    }

    /* clear memory */
    memset(buffers[buffers_index], 0, buffers_sizes[buffers_index]);
    ret_buffer = buffers[buffers_index];

    buffers_index++;
    if (buffers_index == MAX_BUFFERS)
      buffers_index = 0;

    return ret_buffer;
  }
#undef MAX_BUFFERS
}
