/** \file
 * \brief imask parser
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>

#include "imask.h"
#include "imask_match.h"

/*
 * Table of characters (customizaveis atraves de iMaskSetChar)
 */

static char *pm_chars = "|*+()[]-^/.?^${}~";
/*                       01234567890123456     */

#define OR_CH        pm_chars[0]  /* OR CHaracter      */
#define CL_CH        pm_chars[1]  /* CLosure CHaracter    */
#define OOM_CH       pm_chars[2]  /* One Or More CHaracter  */
#define OPGR_CH      pm_chars[3]  /* OPen  GRoup CHaracter  */
#define CLGR_CH      pm_chars[4]  /* CLose GRoup CHaracter  */
#define OPCL_CH      pm_chars[5]  /* OPen CLass CHaracter    */
#define CLCL_CH      pm_chars[6]  /* CLose CLass CHaracter  */
#define SEPCL_CH     pm_chars[7]  /* SEParate CLass CHaracter  */
#define NEGCL_CH     pm_chars[8]  /* NEGation CLass CHaracter  */
#define SPC_CH       pm_chars[9]  /* SPeCial function CHaracter  */
#define ANY_CH       pm_chars[10]  /* ANY CHaracter    */
#define ONE_CH       pm_chars[11]  /* ONE or no CHaracter    */
#define BEGIN_CH     pm_chars[12]  /* BEGINning of a line CHaracter*/
#define END_CH       pm_chars[13]  /* END of a line CHaracter  */
#define CAP_OPEN_CH  pm_chars[14]  /* CAPture OPEN CHaracter  */
#define CAP_CLOSE_CH pm_chars[15]  /* CAPture CLOSE CHaracter  */
#define NEG_CH       pm_chars[16]  /* NEGation CHaracter    */

#define SPC2_CH  '\\'    /* SPeCial 2 CHaracter */

#define isvalid(c) (c != 0 && c != OR_CH && c != OPGR_CH && c != CLGR_CH &&\
                         c != CL_CH && c != OPCL_CH && c != CLCL_CH &&\
                         c != CAP_OPEN_CH && c != CAP_CLOSE_CH && c != OOM_CH)

#define STATE_BLOCK        30

extern function_st imk_match_functions[];

typedef struct vars_st
{
  const char *string;
  int state, j, num_states;
  iMask *fsm;
  short capture[30];
  short size;
  char nextcap;
  jmp_buf env;
}
vars_st;

static int pm_expression (vars_st * vars);
static int pm_term (vars_st * vars);
static int pm_factor (vars_st * vars);
static void pm_error (vars_st *vars);
static void pm_newstate (vars_st * vars);
static void pm_setstate (vars_st * vars, int state, char ch, char command, int next1, int next2);

int iMaskSetChar (int char_number, char new_char)
{
  if ((char_number < 0) || (char_number > (int)strlen(pm_chars)))
    return 0;

  pm_chars[char_number] = new_char;

  return 1;
}

    /*
     * Funcao de interface, recebe padrao e retorna array contendo as finite
     * state machines (fsm) construidas a partir do padrao
     */

int iMaskParse (const char *text, iMask ** fsm)
{
  int t;
  vars_st vars;

  /* inicializacao das variaveis */

  vars.state = 1;
  vars.j = 0;
  vars.num_states = 0;
  vars.size = 0;
  vars.nextcap = 0;
  vars.string = text;

  if ((vars.fsm = (iMask *) malloc (STATE_BLOCK * sizeof (iMask))) == NULL)
    return IMK_MEM_ERROR;

  vars.num_states = STATE_BLOCK;

  /* a principio, nao ha captura. Se ocorrer uma, ele e setado
     para CAPTURE */

  vars.fsm[0].ch = NOCAPTURE;

  if (setjmp (vars.env) == 0)
    t = pm_expression (&vars);

  else
  {
    free (vars.fsm);
    return IMK_PARSE_ERROR;
  }

  /* seta os estados inicial e final, guardando no inicial
     (fsm[0].next1) o tamanho da maquina */

  pm_setstate (&vars, 0, vars.fsm[0].ch, NULL_CMD, t, vars.state + 1);
  pm_setstate (&vars, vars.state, 0, NULL_CMD, 0, 0);

  *fsm = vars.fsm;

  return IMK_PARSE_OK;
}

static int pm_expression (vars_st * vars)
{
  int r, t1;
  int last_state = vars->state - 1;

  t1 = pm_term (vars);
  r = t1;

  if (vars->string[vars->j] == OR_CH)
  {
    int t2 = vars->state;
    int t3;

    r = t2;

    vars->j++;
    pm_newstate (vars);

    t3 = pm_expression (vars);  /* pega o 2o ramo do OR */

    /* faz o primeiro state antes do OR apontar para o state de entrada
     * do OR */

    if (vars->fsm[last_state].next1 == t1)
      vars->fsm[last_state].next1 = t2;

    if (vars->fsm[last_state].next2 == t1)
      vars->fsm[last_state].next2 = t2;

    /* faz o ultimo state do primeiro ramo do OR apontar para o
     * state de saida do OR */

    if (vars->fsm[t2 - 1].next1 == t2)
      vars->fsm[t2 - 1].next1 = vars->state;

    if (vars->fsm[t2 - 1].next2 == t2)
      vars->fsm[t2 - 1].next2 = vars->state;

    pm_setstate (vars, t2, 0, NULL_CMD, t1, t3);
    pm_setstate (vars, vars->state, 0, NULL_CMD, vars->state + 1,
        vars->state + 1);

    pm_newstate (vars);
  }
  return r;
}

static int pm_term (vars_st * vars)
{
  int r;

  r = pm_factor (vars);

  if ((vars->string[vars->j] == OPGR_CH) ||
      (isvalid (vars->string[vars->j])) ||
      (vars->string[vars->j] == OPCL_CH) ||
      (vars->string[vars->j] == CAP_OPEN_CH) ||
      (vars->string[vars->j] == NEG_CH))
    pm_term (vars);

  if (!((vars->string[vars->j] == OR_CH) ||
        (vars->string[vars->j] == CLGR_CH) ||
        (vars->string[vars->j] == '\0') ||
        (vars->string[vars->j] == CAP_CLOSE_CH)))
    pm_error (vars);

  return r;
}

static int pm_factor (vars_st * vars)
{
  int r, t1, t2 = 0;

  t1 = vars->state;

  if (vars->string[vars->j] == OPGR_CH)
  {
    vars->j++;
    t2 = pm_expression (vars);

    if (vars->string[vars->j] == CLGR_CH)
      vars->j++;
    else
      pm_error (vars);
  }

  else if (vars->string[vars->j] == CAP_OPEN_CH)
  {
    vars->fsm[0].ch = CAPTURE;
    pm_setstate (vars, vars->state, vars->nextcap, 
                 CAP_OPEN_CMD, vars->state + 1, vars->state + 1);
    t2 = vars->state;
    pm_newstate (vars);
    vars->capture[++vars->size] = vars->nextcap++;
    vars->j++;

    pm_expression (vars);

    if (vars->string[vars->j] == CAP_CLOSE_CH)
    {
      pm_setstate (vars, vars->state, (char)vars->capture[vars->size--],
                   CAP_CLOSE_CMD, vars->state + 1, vars->state + 1);

      pm_newstate (vars);
      vars->j++;
    }
    else
      pm_error (vars);

  }

  else if (vars->string[vars->j] == ANY_CH)
  {
    pm_setstate (vars, vars->state, 1, ANY_CMD, vars->state + 1, vars->state + 1);
    t2 = vars->state;
    vars->j++;
    pm_newstate (vars);
  }

  else if (vars->string[vars->j] == NEG_CH)
  {
    int t6;
    t2 = vars->state;
    vars->j++;
    pm_newstate (vars);
    t6 = pm_factor (vars);
    pm_setstate (vars, t2, 1, NEG_OPEN_CMD, t6, vars->state);
    pm_setstate (vars, vars->state, 1, NEG_CLOSE_CMD, vars->state + 1, vars->state + 1);
    pm_newstate (vars);
  }

  else if (vars->string[vars->j] == BEGIN_CH)
  {
    pm_setstate (vars, vars->state, 1, BEGIN_CMD, vars->state + 1,
      vars->state + 1);
    t2 = vars->state;
    vars->j++;
    pm_newstate (vars);
  }
  else if (vars->string[vars->j] == END_CH)
  {
    pm_setstate (vars, vars->state, 1, END_CMD, vars->state + 1,
      vars->state + 1);
    t2 = vars->state;
    vars->j++;
    pm_newstate (vars);
  }

  else if (isvalid (vars->string[vars->j]) && (vars->string[vars->j]
  != SPC_CH) && (vars->string[vars->j] != ANY_CH))
  {
    pm_setstate (vars, vars->state, vars->string[vars->j], 
                 CHAR_CMD, vars->state + 1, vars->state + 1);
    t2 = vars->state;
    vars->j++;
    pm_newstate (vars);
  }

  else if (vars->string[vars->j] == OPCL_CH)
  {
    vars->j++;
    pm_setstate (vars, vars->state, 0, CLASS_CMD, 0, 0);

    if (vars->string[vars->j] == NEGCL_CH)
    {
      vars->fsm[vars->state].next2 = 1;
      vars->j++;
    }

    t2 = vars->state;
    pm_newstate (vars);

    if (vars->string[vars->j] == SEPCL_CH)
      pm_error (vars);

    while ((vars->string[vars->j] != CLCL_CH) && (vars->string[vars->j] != '\n')
      && (vars->string[vars->j] != '\0'))
    {
      if (vars->string[vars->j] == SPC_CH)
      {
        char temp;

        vars->j++;
        switch (vars->string[vars->j])
        {
        case 'n':
          temp = '\n';
          break;

        case 't':
          temp = '\t';
          break;

        case 'e':
          temp = 27;
          break;

        default:
          temp = vars->string[vars->j];
        }
        pm_setstate (vars, vars->state, temp, CLASS_CMD_CHAR, 0, 0);
        vars->j++;
        pm_newstate (vars);
      }
      else if (vars->string[vars->j] == SEPCL_CH)
      {
        char temp = 0;

        vars->j++;

        if (vars->string[vars->j] == SPC_CH)
        {
          vars->j++;
          switch (vars->string[vars->j])
          {
          case 'n':
            temp = '\n';
            break;

          case 't':
            temp = '\t';
            break;

          case 'e':
            temp = 27;
            break;

          default:
            temp = vars->string[vars->j];
          }
        }
        else if (vars->string[vars->j] != CLCL_CH)
          temp = vars->string[vars->j];

        else
          pm_error (vars);

        pm_setstate (vars, vars->state - 1, vars->fsm[vars->state - 1].ch,
                     CLASS_CMD_RANGE, temp, 0);
        vars->j++;
      }
      else if (vars->string[vars->j] == BEGIN_CH)
      {
        pm_setstate (vars, vars->state, 1, BEGIN_CMD, vars->state + 1, vars->state + 1);
        t2 = vars->state;
        vars->j++;
        pm_newstate (vars);
      }
      else if (vars->string[vars->j] == END_CH)
      {
        pm_setstate (vars, vars->state, 1, END_CMD, vars->state + 1, vars->state + 1);
        t2 = vars->state;
        vars->j++;
        pm_newstate (vars);
      }

      else
      {
        pm_setstate (vars, vars->state, vars->string[vars->j], CLASS_CMD_CHAR, 0, 0);
        vars->j++;
        pm_newstate (vars);
      }

    }
    if (vars->string[vars->j] != CLCL_CH)
      pm_error (vars);
    pm_setstate (vars, vars->state, 0, NULL_CMD, vars->state + 1, vars->state + 1);
    vars->fsm[t2].next1 = vars->state;
    vars->j++;
    pm_newstate (vars);

  }

  else if (vars->string[vars->j] == SPC_CH)
  {
    int loop1 = 0;

    vars->j++;

    while (imk_match_functions[loop1].ch != '\0' && 
           imk_match_functions[loop1].ch != vars->string[vars->j])
      loop1++;

    if (imk_match_functions[loop1].ch == '\0')
    {
      int temp;

      switch (vars->string[vars->j])
      {
      case 'n':
        temp = '\n';
        break;

      case 't':
        temp = '\t';
        break;

      case 'e':
        temp = 27;
        break;

      case 'x':
        vars->j++;
        sscanf (&vars->string[vars->j], "%2x", &temp);
        vars->j++;
        break;

      case 'o':
        vars->j++;
        sscanf (&vars->string[vars->j], "%3o", &temp);
        vars->j += 2;
        break;

      default:
        if (isdigit (vars->string[vars->j]))
        {
          sscanf (&vars->string[vars->j], "%3d", &temp);
          if (temp > 255)
          {
            pm_error (vars);
          }
          vars->j += 2;
        }
        else
          temp = vars->string[vars->j];
      }
      pm_setstate (vars, vars->state, (char)temp, CHAR_CMD, vars->state + 1, vars->state + 1);
    }

    else
    {
      pm_setstate (vars, vars->state, (char)loop1, SPC_CMD, vars->state + 1, vars->state + 1);
    }

    t2 = vars->state;
    vars->j++;
    pm_newstate (vars);
  }
  else
    pm_error (vars);

  if (vars->string[vars->j] == CL_CH)
  {
    pm_setstate (vars, vars->state, 0, NULL_CMD, vars->state + 1, t2);
    r = vars->state;

    if (vars->fsm[t1 - 1].next1 == t1)
      vars->fsm[t1 - 1].next1 = vars->state;

    if (vars->fsm[t1 - 1].next2 == t1)
      vars->fsm[t1 - 1].next2 = vars->state;

    vars->j++;
    pm_newstate (vars);
  }

  else if (vars->string[vars->j] == ONE_CH)
  {
    pm_setstate (vars, vars->state, 0, NULL_CMD, vars->state + 1, t2);
    r = vars->state;

    if (vars->fsm[t1 - 1].next1 == t1)
      vars->fsm[t1 - 1].next1 = vars->state;

    if (vars->fsm[t1 - 1].next2 == t1)
      vars->fsm[t1 - 1].next2 = vars->state;

    if (vars->fsm[vars->state - 1].next1 == vars->state)
      vars->fsm[vars->state - 1].next1 = vars->state + 1;

    if (vars->fsm[vars->state - 1].next2 == vars->state)
      vars->fsm[vars->state - 1].next2 = vars->state + 1;

    vars->j++;
    pm_newstate (vars);

    pm_setstate (vars, vars->state, 0, NULL_CMD, vars->state + 1, vars->state + 1);

    pm_newstate (vars);
  }
  else if (vars->string[vars->j] == OOM_CH)
  {
    pm_setstate (vars, vars->state, 0, NULL_CMD, vars->state + 1, t2);
    r = t2;

    vars->j++;

    pm_newstate (vars);

  }
  else
    r = t2;

  return r;
}

static void pm_error (vars_st * vars)
{
  longjmp (vars->env, 1);
}

static void pm_newstate (vars_st * vars)
{

  if (vars->state >= vars->num_states - 1)
  {
    iMask *new_fsm = (iMask*) realloc (vars->fsm, (vars->num_states + STATE_BLOCK) * sizeof (iMask));
    vars->fsm = new_fsm;
    vars->num_states += STATE_BLOCK;
  }

  vars->state++;
}

static void pm_setstate (vars_st * vars, int state, char ch, char command, int next1, int next2)
{
  vars->fsm[state].ch = ch;
  vars->fsm[state].command = command;
  vars->fsm[state].next1 = next1;
  vars->fsm[state].next2 = next2;
}
