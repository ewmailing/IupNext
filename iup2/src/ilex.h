/** \file
 * \brief lexical analysis manager for LED.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __ILEX_H 
#define __ILEX_H

#ifdef __cplusplus
extern "C" {
#endif

/* TOKENS */
#define TK_END          -1
#define TK_BEGP         1
#define TK_ENDP         2
#define TK_ATTR         3
#define TK_STR          4
#define TK_NAME         5
#define TK_NUMB         6
#define TK_SET          7
#define TK_COMMA        8
#define TK_FUNC         9
#define TK_ENDATTR     10

/* ERRORS */
#define FILENOTOPENED   1
#define NOTMATCH        2
#define NOTENDATTR      3
#define INVALID_NUMBER  4
#define PARSEERROR      5

char*   iupLexGetError   (void);
void    iupLexInit       (void);
void    iupLexFinish     (void);
int     iupLexStart      (const char *filename);
void    iupLexClose      (void);
int     iupLexLookAhead  (void);
int     iupLexAdvance    (void);
int     iupLexFollowedBy (int t);
int     iupLexMatch      (int t);
int     iupLexSeenMatch  (int t, int *erro);
int     iupLexInt        (void);
char*   iupLexGetName    (void);
char*   iupLexName       (void);
float   iupLexGetNumber  (void);
char*   iupLexGetFunc    (void);
int     iupLexError      (int n, ...);
Iclass* iupLexGetClass   (void);
void    iupLexRegisterLED(const char *name, Iclass *ic);

#ifdef __cplusplus
}
#endif

#endif
