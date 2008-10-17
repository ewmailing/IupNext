/** \file
 * \brief String Utilities
 *
 * See Copyright Notice in iup.h
 * $Id: istrutil.h,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

 
#ifndef __ISTRUTIL_H 
#define __ISTRUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/** Returns a copy of the given string.
 * If str is NULL it will return NULL.
 */
char* iupStrDup(const char* str); 

/** Returns a non zero value if the two strings are equal.
 * str1 or str2 can be NULL.
 */
int iupStrEqual(const char* str1, const char* str2);

/** Returns a non zero value if the two strings are equal but ignores case.
 * str1 or str2 can be NULL.
 */
int iupStrEqualNoCase(const char* str1, const char* str2);

/** Returns a non zero value if the two strings are equal 
 * up to a number of characters defined by the strlen of the second string.
 * str1 or str2 can be NULL.
 */
int iupStrEqualPartial(const char* str1, const char* str2);

/** Returns the number of lines and the size of the largest line in a string.
 * Detects also CR/LF pairs.
 */
void  iupStrLineCount(const char* str, int *max_len, int *num_lin);

/** Returns the a pointer to the next line and the size of the current line.
 * Detects also CR/LF pairs.
 * If str is NULL it will return NULL.
 */
const char* iupStrNextLine(const char* str, int *len);

/** Returns the number of repetitions of the character occours in the string.
 */
int iupStrCountChar(const char *str, int c);

/** Returns a new string containing a copy of the string up to the character.
 * The string is then incremented to after the position of the character.
 */
char *iupStrCopyUntil(char **str, int c);

/** Returns a buffer with the specified size+1. \n
 * The buffer is resused after 10 calls. It must NOT be freed.
 * Use size=-1 to free all the internal buffers.
 */
char *iupStrGetMemory(int size);

int iupStrToIntInt(const char *str, int *i1, int *i2, char c);


#ifdef __cplusplus
}
#endif

#endif
