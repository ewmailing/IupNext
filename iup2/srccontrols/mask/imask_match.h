/** \file
 * \brief imask.
 *
 * See Copyright Notice in iup.h
 * $Id: imask_match.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMASK_MATCH_H 
#define __IMASK_MATCH_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
  NULL_CMD=1,
  ANY_CMD=2,
  CHAR_CMD=3,
  SPC_CMD=4,
  CLASS_CMD=5,
  BEGIN_CMD=6,
  END_CMD=7,
  CAP_OPEN_CMD=71,
  CAP_CLOSE_CMD=72,
  NEG_OPEN_CMD=81,
  NEG_CLOSE_CMD=82
};

enum
{
  CLASS_CMD_RANGE=50,
  CLASS_CMD_CHAR=51
};

typedef enum 
{
  NORMAL_MATCH, 
  NO_CHAR_MATCH, 
  NO_MATCH
} match_t;

typedef struct _function_st
{
  char ch;
  match_t (*function) (const char *, long);
} function_st;

enum
{
  CAPTURE=100,
  NOCAPTURE=101
};

#ifdef __cplusplus
}
#endif

#endif
