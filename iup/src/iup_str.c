/** \file
 * \brief String Utilities
 *
 * See Copyright Notice in "iup.h"
 */

 
#include <string.h>  
#include <ctype.h>   
#include <stdlib.h>  
#include <stdio.h>  
#include <limits.h>
#include <stdarg.h>

#include "iup_str.h"


/* Line breaks can be:
\n - UNIX
\r - Mac
\r\n - DOS/Windows
*/


int iupStrEqual(const char* str1, const char* str2) 
{
  if (str1 == str2) return 1;
  if (!str1 || !str2 || *str1 != *str2) return 0;
  return (strcmp(str1, str2)==0)? 1: 0;
}

static char iStrUpper(char c)
{
  if (c >= 'a' && c <= 'z')  /* is lower */
    return (c - 'a') + 'A';
  return c;
}

static char iStrLower(char c)
{
  if (c >= 'A' && c <= 'Z')  /* is upper */
    return (c - 'A') + 'a';
  return c;
}

int iupStrEqualNoCaseNoSpace(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2 || 
      iStrLower(*str1) != iStrLower(*str2)) 
    return 0;       

  while (str1[i] && str2[i] && iStrLower(str1[i])==iStrLower(str2[i])) 
  {
    if (str1[i] == ' ')  /* also ignore spaces */
      i++;
    i++;
  }
  if (str1[i] == str2[i]) return 1;  /* check also for terminator */

  return 0;
}

int iupStrEqualNoCase(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2) 
    return 0;

  while (str1[i] && str2[i] && iStrLower(str1[i])==iStrLower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1;  /* check also for terminator */

  return 0;
}

int iupStrEqualNoCasePartial(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2) 
    return 0;

  while (str1[i] && str2[i] && iStrLower(str1[i])==iStrLower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1;  /* check also for terminator */
  if (str2[i] == 0) return 1;  /* is second string is at terminator, then it is partially equal */

  return 0;
}

int iupStrEqualPartial(const char* str1, const char* str2) 
{
  if (str1 == str2) return 1;
  if (!str1 || !str2 || *str1 != *str2) return 0;
  return (strncmp(str1, str2, strlen(str2))==0)? 1: 0;
}

int iupStrFalse(const char* str)
{
  if (!str || str[0]==0) return 0;
  if (str[0]=='0' && str[1]==0) return 1;
  if (iupStrEqualNoCase(str, "NO")) return 1;
  if (iupStrEqualNoCase(str, "OFF")) return 1;
  return 0;
}

int iupStrBoolean(const char* str)
{
  if (!str || str[0]==0) return 0;
  if (str[0]=='1' && str[1]==0) return 1;
  if (iupStrEqualNoCase(str, "YES")) return 1;
  if (iupStrEqualNoCase(str, "ON")) return 1;
  return 0;
}

void iupStrUpper(char* dstr, const char* sstr)
{
  for (; *sstr; sstr++, dstr++)
    *dstr = (char)iStrUpper(*sstr);
  *dstr = 0;
}

void iupStrLower(char* dstr, const char* sstr)
{
  for (; *sstr; sstr++, dstr++)
    *dstr = (char)iStrLower(*sstr);
  *dstr = 0;
}

int iupStrHasSpace(const char* str)
{
  while(*str)
  {
    if (*str == ' ')
      return 1;
    str++;
  }
  return 0;
}

char *iupStrDup(const char *str)
{
  if (str)
  {
    int size = strlen(str)+1;
    char *newstr = malloc(size);
    if (newstr) memcpy(newstr, str, size);
    return newstr;
  }
  return NULL;
}

const char* iupStrNextLine(const char* str, int *len)
{
  *len = 0;

  while(*str!=0 && *str!='\n' && *str!='\r') 
  {
    (*len)++;
    str++;
  }

  if (*str=='\r' && *(str+1)=='\n')   /* DOS line end */
    return str+2;
  else if (*str=='\n' || *str=='\r')   /* UNIX or MAC line end */
    return str+1;
  else 
    return str;  /* no next line */
}

const char* iupStrNextValue(const char* str, int str_len, int *len, char sep)
{
  *len = 0;

  while(*str!=0 && *str!=sep && *len<str_len) 
  {
    (*len)++;
    str++;
  }

  if (*str==sep)
    return str+1;
  else 
    return str;  /* no next value */
}

int iupStrLineCount(const char* str)
{
  int num_lin = 1;

  if (!str)
    return num_lin;

  while(*str != 0)
  {
    while(*str!=0 && *str!='\n' && *str!='\r')
      str++;

    if (*str=='\r' && *(str+1)=='\n')   /* DOS line end */
    {
      num_lin++;
      str+=2;
    }
    else if (*str=='\n' || *str=='\r')   /* UNIX or MAC line end */
    {
      num_lin++;
      str++;
    }
  }

  return num_lin;
}

int iupStrCountChar(const char *str, char c)
{
  int n;
  if (!str) return 0;
  for (n=0; *str; str++)
  {
    if (*str==c)
      n++;
  }
  return n;
}

void iupStrCopyN(char* dst_str, int dst_max_size, const char* src_str)
{
  int size = strlen(src_str)+1;
  if (size > dst_max_size) size = dst_max_size;
  memcpy(dst_str, src_str, size-1);
  dst_str[size-1] = 0;
}

char* iupStrDupUntil(char **str, char c)
{
  char *p_str,*new_str;
  if (!str || *str==NULL)
    return NULL;

  p_str=strchr(*str,c);
  if (!p_str) 
    return NULL;
  else
  {
    int i;
    int sl = (int)(p_str - (*str));

    new_str = (char *)malloc(sl + 1);
    if (!new_str) return NULL;

    for (i = 0; i < sl; ++i)
      new_str[i] = (*str)[i];

    new_str[sl] = 0;

    *str = p_str+1;
    return new_str;
  }
}

static char *iStrDupUntilNoCase(char **str, char c)
{
  char *p_str,*new_str;
  if (!str || *str==NULL)
    return NULL;

  p_str=strchr(*str,c); /* usually the lower case is enough */
  if (!p_str && isalpha(c)) 
    p_str=strchr(*str, iStrUpper(c));  /* but check also for upper case */

  /* if both fail, then abort */
  if (!p_str) 
    return NULL;
  else
  {
    int i;
    int sl=(int)(p_str - (*str));

    new_str = (char *) malloc (sl + 1);
    if (!new_str) return NULL;

    for (i = 0; i < sl; ++i)
      new_str[i] = (*str)[i];

    new_str[sl] = 0;

    *str = p_str+1;
    return new_str;
  }
}

char *iupStrGetLargeMem(int *size)
{
#define LARGE_MAX_BUFFERS 10
#define LARGE_SIZE SHRT_MAX
  static char buffers[LARGE_MAX_BUFFERS][LARGE_SIZE];
  static int buffers_index = -1;
  char* ret_str;

  /* init buffers array */
  if (buffers_index == -1)
  {
    int i;

    memset(buffers, 0, sizeof(char*)*LARGE_MAX_BUFFERS);
    buffers_index = 0;

    /* clear all memory only once */
    for (i=0; i<LARGE_MAX_BUFFERS; i++)
      memset(buffers[i], 0, sizeof(char)*LARGE_SIZE);
  }

  /* DON'T clear memory everytime because the buffer is too large */
  ret_str = buffers[buffers_index];
  ret_str[0] = 0;

  buffers_index++;
  if (buffers_index == LARGE_MAX_BUFFERS)
    buffers_index = 0;

  if (size) *size = LARGE_SIZE;
  return ret_str;
#undef LARGE_MAX_BUFFERS
#undef LARGE_SIZE 
}

static char* iupStrGetSmallMem(void)
{
#define SMALL_MAX_BUFFERS 100
#define SMALL_SIZE 80  /* maximum for iupStrReturnFloat */
  static char buffers[SMALL_MAX_BUFFERS][SMALL_SIZE];
  static int buffers_index = -1;
  char* ret_str;

  /* init buffers array */
  if (buffers_index == -1)
  {
    memset(buffers, 0, sizeof(char*)*SMALL_MAX_BUFFERS);
    buffers_index = 0;
  }

  /* always clear memory before returning a new buffer */
  memset(buffers[buffers_index], 0, SMALL_SIZE);
  ret_str = buffers[buffers_index];

  buffers_index++;
  if (buffers_index == SMALL_MAX_BUFFERS)
    buffers_index = 0;

  return ret_str;
#undef SMALL_MAX_BUFFERS
#undef SMALL_SIZE 
}

char *iupStrGetMemory(int size)
{
#define MAX_BUFFERS 50
  static char* buffers[MAX_BUFFERS];
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
    char* ret_str;

    /* init buffers array */
    if (buffers_index == -1)
    {
      memset(buffers, 0, sizeof(char*)*MAX_BUFFERS);
      memset(buffers_sizes, 0, sizeof(int)*MAX_BUFFERS);
      buffers_index = 0;
    }

    /* first alocation */
    if (!(buffers[buffers_index]))
    {
      buffers_sizes[buffers_index] = size+1;
      buffers[buffers_index] = (char*)malloc(buffers_sizes[buffers_index]);
    }
    else if (buffers_sizes[buffers_index] < size+1)  /* reallocate if necessary */
    {
      buffers_sizes[buffers_index] = size+1;
      buffers[buffers_index] = (char*)realloc(buffers[buffers_index], buffers_sizes[buffers_index]);
    }

    /* always clear memory before returning a new buffer */
    memset(buffers[buffers_index], 0, buffers_sizes[buffers_index]);
    ret_str = buffers[buffers_index];

    buffers_index++;
    if (buffers_index == MAX_BUFFERS)
      buffers_index = 0;

    return ret_str;
  }
#undef MAX_BUFFERS
}

char* iupStrReturnStrf(const char* format, ...)
{
  char* value = iupStrGetMemory(1024);
  va_list arglist;
  va_start(arglist, format);
  vsnprintf(value, 1024, format, arglist);
  va_end(arglist);
  return value;
}

char* iupStrReturnStr(const char* str)
{
  if (str)
  {
    int size = strlen(str)+1;
    char* ret_str = iupStrGetMemory(size);
    memcpy(ret_str, str, size);
    return ret_str;
  }
  else
    return NULL;
}

char* iupStrReturnBoolean(int b)
{
  if (b)
    return "YES";
  else
    return "NO";
}

char* iupStrReturnChecked(int check)
{
  if (check == -1)
    return "NOTDEF";
  else if (check)
    return "ON";
  else
    return "OFF";
}

char* iupStrReturnInt(int i)
{
  char* str = iupStrGetSmallMem();  /* 20 */
  sprintf(str, "%d", i);
  return str;
}

char* iupStrReturnFloat(float f)
{
  char* str = iupStrGetSmallMem();  /* 80 */
  sprintf(str, "%.9f", f);  /* maximum float precision */
  return str;
}

char* iupStrReturnRGB(unsigned char r, unsigned char g, unsigned char b)
{
  char* str = iupStrGetSmallMem();  /* 3*20 */
  sprintf(str, "%d %d %d", (int)r, (int)g, (int)b);
  return str;
}

char* iupStrReturnStrStr(const char *str1, const char *str2, char sep)
{
  if (str1 || str2)
  {
    char* ret_str;
    int size1=0, size2=0;
    if (str1) size1 = strlen(str1);
    if (str2) size2 = strlen(str2);
    ret_str = iupStrGetMemory(size1+size2+2);
    if (str1 && size1) memcpy(ret_str, str1, size1);
    ret_str[size1] = sep;
    if (str2 && size2) memcpy(ret_str+size1+1, str2, size2);
    ret_str[size1+1+size2] = 0;
    return ret_str;
  }
  else
    return NULL;
}

char* iupStrReturnIntInt(int i1, int i2, char sep)
{
  char* str = iupStrGetSmallMem();  /* 2*20 */
  sprintf(str, "%d%c%d", i1, sep, i2);
  return str;
}

int iupStrToRGB(const char *str, unsigned char *r, unsigned char *g, unsigned char *b)
{
  unsigned int ri = 0, gi = 0, bi = 0;
  if (!str) return 0;
  if (str[0]=='#')
  {
    str++;
    if (sscanf(str, "%2X%2X%2X", &ri, &gi, &bi) != 3) return 0;
  }
  else
  {
    if (sscanf(str, "%u %u %u", &ri, &gi, &bi) != 3) return 0;
  }
  if (ri > 255 || gi > 255 || bi > 255) return 0;
  *r = (unsigned char)ri;
  *g = (unsigned char)gi;
  *b = (unsigned char)bi;
  return 1;
}

int iupStrToInt(const char *str, int *i)
{
  if (!str) return 0;
  if (sscanf(str, "%d", i) != 1) return 0;
  return 1;
}

int iupStrToIntInt(const char *str, int *i1, int *i2, char sep)
{
  if (!str) return 0;

  if (*str == sep || (isalpha(sep) && *str == iStrUpper(sep))) /* no first value */
  {
    str++; /* skip separator */
    if (sscanf(str, "%d", i2) != 1) return 0;
    return 1;
  }
  else 
  {
    char* p_str = iStrDupUntilNoCase((char**)&str, sep);
    
    if (!p_str)   /* no separator means no second value */
    {        
      if (sscanf(str, "%d", i1) != 1) return 0;
      return 1;
    }
    else if (*str==0)  /* separator exists, but second value empty, also means no second value */
    {        
      int ret = sscanf(p_str, "%d", i1);
      free(p_str);
      if (ret != 1) return 0;
      return 1;
    }
    else
    {
      int ret = 0;
      if (sscanf(p_str, "%d", i1) == 1) ret++;
      if (sscanf(str, "%d", i2) == 1) ret++;
      free(p_str);
      return ret;
    }
  }
}

int iupStrToFloat(const char *str, float *f)
{
  if (!str) return 0;
  if (sscanf(str, "%f", f) != 1) return 0;
  return 1;
}

int iupStrToFloatFloat(const char *str, float *f1, float *f2, char sep)
{
  if (!str) return 0;

  if (*str == sep || (isalpha(sep) && *str == iStrUpper(sep))) /* no first value */
  {
    str++; /* skip separator */
    if (sscanf(str, "%f", f2) != 1) return 0;
    return 1;
  }
  else 
  {
    char* p_str = iStrDupUntilNoCase((char**)&str, sep);
    
    if (!p_str)   /* no separator means no second value */
    {        
      if (sscanf(str, "%f", f1) != 1) return 0;
      return 1;
    }
    else if (*str==0)    /* separator exists, but second value empty, also means no second value */
    {        
      int ret = sscanf(p_str, "%f", f1);
      free(p_str);
      if (ret != 1) return 0;
      return 1;
    }
    else
    {
      int ret = 0;
      if (sscanf(p_str, "%f", f1) != 1) ret++;
      if (sscanf(str, "%f", f2) != 1) ret++;
      free(p_str);
      return ret;
    }
  }
}

int iupStrToStrStr(const char *str, char *str1, char *str2, char sep)
{
  if (!str) return 0;

  if (*str == sep || (isalpha(sep) && *str == iStrUpper(sep))) /* no first value */
  {
    str++; /* skip separator */
    strcpy(str2, str);
    return 1;
  }
  else 
  {
    char* p_str = iStrDupUntilNoCase((char**)&str, sep);
    
    if (!p_str)   /* no separator means no second value */
    {        
      strcpy(str1, str);
      return 1;
    }
    else if (*str==0)    /* separator exists, but second value empty, also means no second value */
    {        
      strcpy(str1, p_str);
      free(p_str);
      return 1;
    }
    else
    {
      strcpy(str1, p_str);
      strcpy(str2, str);
      free(p_str);
      return 2;
    }
  }
}

char* iupStrFileGetPath(const char *file_name)
{
  /* Starts at the last character */
  int len = strlen(file_name) - 1;
  while (len != 0)
  {
    if (file_name[len] == '\\' || file_name[len] == '/')
    {
      len++;
      break;
    }

    len--;
  }

  if (len == 0)
    return NULL;

  {
    char* path = malloc(len+1);
    memcpy(path, file_name, len);
    path[len] = 0;

    return path;
  }
}

char* iupStrFileGetTitle(const char *file_name)
{
  /* Starts at the last character */
  int len = strlen(file_name);
  int offset = len - 1;
  while (offset != 0)
  {
    if (file_name[offset] == '\\' || file_name[offset] == '/')
    {
      offset++;
      break;
    }

    offset--;
  }

  {
    int title_size = len - offset + 1;
    char* file_title = malloc(title_size);
    memcpy(file_title, file_name + offset, title_size);
    return file_title;
  }
}

char* iupStrFileGetExt(const char *file_name)
{
  /* Starts at the last character */
  int len = strlen(file_name);
  int offset = len - 1;
  while (offset != 0)
  {
    /* if found a path separator stop. */
    if (file_name[offset] == '\\' || file_name[offset] == '/')
      return NULL;

    if (file_name[offset] == '.')
    {
      offset++;
      break;
    }

    offset--;
  }

  if (offset == 0) 
    return NULL;

  {
    int ext_size = len - offset + 1;
    char* file_ext = (char*)malloc(ext_size);
    memcpy(file_ext, file_name + offset, ext_size);
    return file_ext;
  }
}

char* iupStrFileMakeFileName(const char* path, const char* title)
{
  int size_path = strlen(path);
  int size_title = strlen(title);
  char *filename = malloc(size_path + size_title + 2);
  memcpy(filename, path, size_path);

  if (path[size_path-1] != '/')
  {
    filename[size_path] = '/';
    size_path++;
  }

  memcpy(filename+size_path, title, size_title);
  filename[size_path+size_title] = 0;

  return filename;
}

void iupStrFileNameSplit(const char* filename, char *path, char *title)
{
  int i, n = strlen(filename);

  /* Look for last folder separator and split title from path */
  for (i=n-1;i>=0; i--)
  {
    if (filename[i] == '\\' || filename[i] == '/') 
    {
      if (path)
      {
        strncpy(path, filename, i+1);
        path[i+1] = 0;
      }

      if (title)
      {
        strcpy(title, filename+i+1);
        title[n-i] = 0;
      }

      return;
    }
  }
}

int iupStrReplace(char* str, char src, char dst)
{
  int i = 0;
  while (*str)
  {
    if (*str == src)
    {
      *str = dst;
      i++;
    }
    str++;
  }
  return i;
}

void iupStrToUnix(char* str)
{
  char* pstr = str;

  if (!str) return;
  
  while (*str)
  {
    if (*str == '\r')
    {
      if (*(str+1) != '\n')  /* MAC line end */
        *pstr++ = '\n';
      str++;
    }
    else
      *pstr++ = *str++;
  }
  
  *pstr = *str;
}

void iupStrToMac(char* str)
{
  char* pstr = str;

  if (!str) return;
  
  while (*str)
  {
    if (*str == '\r')
    {
      if (*(++str) == '\n')  /* DOS line end */
        str++;
      *pstr++ = '\r';
    }
    else if (*str == '\n')  /* UNIX line end */
    {
      str++;
      *pstr++ = '\r';
    }
    else
      *pstr++ = *str++;
  }
  
  *pstr = *str;
}

char* iupStrToDos(const char* str)
{
	char *auxstr, *newstr;
	int num_lin;

  if (!str) return NULL;

  num_lin = iupStrLineCount(str);
  if (num_lin == 1)
    return (char*)str;

	newstr = malloc(num_lin + strlen(str) + 1);
  auxstr = newstr;
	while(*str)
	{
		if (*str == '\r' && *(str+1)=='\n')  /* DOS line end */
    {
      *auxstr++ = *str++;
      *auxstr++ = *str++;
    }
    else if (*str == '\r')   /* MAC line end */
    {
		  *auxstr++ = *str++;
			*auxstr++ = '\n';
    }
    else if (*str == '\n')  /* UNIX line end */
    {
			*auxstr++ = '\r';
		  *auxstr++ = *str++;
    }
    else
		  *auxstr++ = *str++;
	}
	*auxstr = 0;

	return newstr;	
}

#define IUP_ISRESERVED(_c) (_c=='\n' || _c=='\r' || _c=='\t')

char* iupStrConvertToC(const char* str)
{
  char* new_str, *pnstr;
  const char* pstr = str;
  int len, count=0;
  while(*pstr)
  {
    if (IUP_ISRESERVED(*pstr))
      count++;
    pstr++;
  }
  if (!count)
    return (char*)str;

  len = pstr-str;
  new_str = malloc(len+count+1);
  pstr = str;
  pnstr = new_str;
  while(*pstr)
  {
    if (IUP_ISRESERVED(*pstr))
    {
      *pnstr = '\\';
      pnstr++;

      switch(*pstr)
      {
      case '\n':
        *pnstr = 'n';
        break;
      case '\r':
        *pnstr = 'r';
        break;
      case '\t':
        *pnstr = 't';
        break;
      }
    }
    else
      *pnstr = *pstr;

    pnstr++;
    pstr++;
  }
  *pnstr = 0;
  return new_str;
}

char* iupStrProcessMnemonic(const char* str, char *c, int action)
{
  int i = 0, found = 0;
  char* new_str, *orig_str = (char*)str;

  if (!str) 
    return NULL;

  if (!strchr(str, '&'))
    return (char*)str;

  new_str = malloc(strlen(str)+1);
  while (*str)
  {
    if (*str == '&')
    {
      if (*(str+1) == '&') /* remove & from the string, add next & to the string */
      {
        found = -1;

        str++;
        new_str[i++] = *str;
      }
      else if (found!=1) /* mnemonic found */
      {
        found = 1;

        if (action == 1) /* replace & by c */
          new_str[i++] = *c;
        else if (action == -1)  /* remove & and return in c */
          *c = *(str+1);  /* next is mnemonic */
        /* else -- only remove & */
      }
    }
    else
    {
      new_str[i++] = *str;
    }

    str++;
  }
  new_str[i] = 0;

  if (found==0)
  {
    free(new_str);
    return orig_str;
  }

  return new_str;
}

int iupStrFindMnemonic(const char* str)
{
  int c = 0, found = 0;

  if (!str) 
    return 0;

  if (!strchr(str, '&'))
    return 0;

  while (*str)
  {
    if (*str == '&')
    {
      if (*(str+1) == '&')
      {
        found = -1;
        str++;
      }
      else if (found!=1) /* mnemonic found */
      {
        found = 1;
        c = *(str+1);  /* next is mnemonic */
      }
    }

    str++;
  }

  if (found==0)
    return 0;
  else
    return c;
}

#define ISTR_UTF8_BASE(_x)  (_x & 0xC0)  /* 11XX XXXX */
#define ISTR_UTF8_COUNT(_x) ((_x & 0xE0)? 3: ((_x & 0xF0)? 4: 2 ))
                            /* 1110 XXXX */  /* 1111 0XXX */  /* 110X XXXX */
#define ISTR_UTF8_SEQ(_x)   (_x & 0x80)  /* 10XX XXXX */

static void iStrFixPosUTF8(const char* value, int *start, int *end)
{
  int p = 0, i = 0, find = 0;
  while(value[i])
  {
    if (find==0 && p == *start)
    {
      *start = i;
      find = 1;
    }
    if (find==1 && p == *end)
    {
      *end = i;
      return;
    }

    if (ISTR_UTF8_BASE(value[i+1]) != 0x80)  /* is 1o byte of a sequence or is 1 byte only */
      p++;
    i++;
  }
}

void iupStrRemove(char* value, int start, int end, int dir, int utf8)
{
  if (start == end) 
  {
    if (dir==1)
      end++;
    else if (start == 0) /* there is nothing to remove before */
      return;
    else
      start--;
  }

  if (utf8)
    iStrFixPosUTF8(value, &start, &end);

  value += start;
  end -= start;
  while (*value)
  {
    *value = *(value+end);
    value++;
  }
}

char* iupStrInsert(const char* value, const char* insert_value, int start, int end, int utf8)
{
  char* new_value = (char*)value;
  int insert_len = strlen(insert_value);
  int len = strlen(value);

  if (utf8)
    iStrFixPosUTF8(value, &start, &end);

  if (end==start || insert_len > end-start)
  {
    new_value = malloc(len - (end-start) + insert_len + 1);
    memcpy(new_value, value, start);
    memcpy(new_value+start, insert_value, insert_len);
    memcpy(new_value+start+insert_len, value+end, len-end+1);
  }
  else
  {
    memcpy(new_value+start, insert_value, insert_len);
    memcpy(new_value+start+insert_len, value+end, len-end+1);
  }

  return new_value;
}
