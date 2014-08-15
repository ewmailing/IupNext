/** \file
 * \brief iupmask imk_match_functions
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "imask.h"
#include "imask_match.h"

#define MIN_STACK_ELEMENTS 1000

typedef struct _capt
{
  struct _capt* next_one;
  enum
  {
    open, close
  }
  type;
  short which_one;
  long pos;
} capt;

typedef struct _vars_st
{
  const char *text;
  iMask *fsm;
  iMaskMatchFunc function;
  short *tested;
  void *user;
} vars_st;


typedef struct _Stack
{
  short *stack;
  short size;
} Stack;


/* macro para simplificar funcoes do pattern matching */
#define isalphanum(x) (isalnum(x) || (x == '_'))


/*****************************************************************
 * Funcoes correspondentes aos comandos das expressoes regulares *
 *****************************************************************/

static match_t match_blanks (const char *text, long j)
{
  return (((text[j] == '\t') || (text[j] == '\xff') || (text[j] == ' ') ||
     (text[j] == '\n'))) ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_non_blanks (const char *text, long j)
{
  return (!((text[j] == '\t') || (text[j] == '\xff') ||
      (text[j] == ' ') || (text[j] == '\n')))
    ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_alpha (const char *text, long j)
{
  return (isalpha (text[j])) ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_non_alpha (const char *text, long j)
{
  return (!isalpha (text[j]) && (text[j] != '\0'))
    ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_digit (const char *text, long j)
{
  return (isdigit (text[j])) ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_non_digit (const char *text, long j)
{
  return (!isdigit (text[j]) && (text[j] != '\0'))
    ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_alphanum (const char *text, long j)
{
  return isalphanum (text[j]) ? NORMAL_MATCH : NO_MATCH;
}

static match_t match_non_alphanum (const char *text, long j)
{
  return (isalphanum (text[j]) || (text[j] == '\0'))
    ? NO_MATCH : NORMAL_MATCH;
}

static match_t match_word_boundary (const char *text, long j)
{
  if ((j == 0) && isalphanum (text[j]))
    return NO_CHAR_MATCH;

  else if (isalphanum (text[j - 1]) && !isalphanum (text[j]))
    return NO_CHAR_MATCH;

  else if (isalphanum (text[j]) && !isalphanum (text[j - 1]))
    return NO_CHAR_MATCH;

  return NO_MATCH;

}

/* array com os comandos do pattern matching com os
 * respectivos ponteiros de funcao
 */

function_st imk_match_functions[] =
{
  {'w', &match_alphanum},
  {'W', &match_non_alphanum},
  {'d', &match_digit},
  {'D', &match_non_digit},
  {'S', &match_non_blanks},
  {'s', &match_blanks},
  {'b', &match_word_boundary},
  {'l', &match_alpha},
  {'L', &match_non_alpha},
  {'\0', NULL}
};

/**************************************************
 * Chama a funcao do usuario para cada captura ou *
 * organiza um array com elas caso o usuario nao  *
 * passado nenhuma                                *
 **************************************************/
static void capture_result (vars_st * vars, capt * capture)
{
  capt *next = NULL;

  while (capture != NULL)
  {
    capt *cap = capture->next_one;

    capture->next_one = next;
    next = capture;
    capture = cap;
  }

  capture = next;
  next = NULL;

  while (capture != NULL)
  {
    if (capture->type == open)
    {
      capt *cap = capture->next_one;

      capture->next_one = next;
      next = capture;
      capture = cap;
    }
    else
    {
      if (capture->pos >= next->pos)
        (*vars->function) (capture->which_one, next->pos, capture->pos,
        vars->text, vars->user);

      next = next->next_one;
      capture = capture->next_one;
    }
  }
}

/*********************************************
* funcao recursiva para percorrer a maquina *
*********************************************/
static long recursive_match (vars_st * vars, long j, int state, capt * capture, int size)
{
  switch (vars->fsm[state].command)
  {
  case NULL_CMD:
    if (vars->fsm[state].next1 == 0)    /*se chegou ao fim da maquina de estados */
    {
      if (vars->function != NULL)
        capture_result (vars, capture);         /* guarda capturas */

      return j;
    }

    /* verifica o estado atual ja foi avaliado antes */
    {
      int count;
      for (count = 0; count < size; count++)
        if (vars->tested[count] == state)
          return IMK_NOMATCH;
    }

    vars->tested[size++] = state;       /* indicada que o estado foi testado */

    /* se houverem dois ramos, chama a funcao recursivamente,
    retornando com o primeiro que completar a maquina */

    if (vars->fsm[state].next1 != vars->fsm[state].next2)
    {
      long a;

      a = recursive_match (vars, j, vars->fsm[state].next2, capture, size);

      if (a != IMK_NOMATCH)         /* se deu match */
        return a;

      a = recursive_match (vars, j, vars->fsm[state].next1, capture, size);

      return a;
    }
    break;

  case CAP_OPEN_CMD:
    {
      long a;
      capt new_cap;

      new_cap.next_one = capture;
      new_cap.type = open;
      new_cap.pos = j;
      new_cap.which_one = vars->fsm[state].ch;

      a = recursive_match (vars, j, vars->fsm[state].next1, &new_cap, size);

      return a;
    }
    break;

  case CAP_CLOSE_CMD:
    {
      long a;
      capt new_cap;

      new_cap.next_one = capture;
      new_cap.type = close;
      new_cap.pos = j - 1;
      new_cap.which_one = vars->fsm[state].ch;

      a = recursive_match (vars, j, vars->fsm[state].next1, &new_cap, size);

      return a;
    }

  case CLASS_CMD:
    {
      int temp, found = 0, negate;

      temp = vars->fsm[state].next1;
      negate = vars->fsm[state].next2;
      state++;

      while (vars->fsm[state].command != NULL_CMD)
      {
        if (vars->fsm[state].command == CLASS_CMD_RANGE)
        {
          if ((vars->text[j] >= vars->fsm[state].ch) &&
            (vars->text[j] <= vars->fsm[state].next1))
          {
            found = 1;
            break;
          }
        }

        else if ((vars->fsm[state].command == CLASS_CMD_CHAR) &&
          (vars->text[j] == vars->fsm[state].ch))
        {
          found = 1;
          break;
        };
        state++;
      }

      if (found ^ negate)
      {
        if (vars->text[j] == '\0')
          return IMK_NOMATCH;
        j++;
        vars->tested = &vars->tested[size + 1];
        size = 0;
        return recursive_match (vars, j, temp, capture, size);
      }
      else
        return IMK_NOMATCH;
    }

  case CHAR_CMD:
    if (vars->text[j] != vars->fsm[state].ch)
      return IMK_NOMATCH;
    j++;
    vars->tested = &vars->tested[size + 1];
    size = 0;
    break;

  case ANY_CMD:
    if ((vars->text[j] == '\0') || (vars->text[j] == '\n'))
      return IMK_NOMATCH;
    j++;
    vars->tested = &vars->tested[size + 1];
    size = 0;
    break;

  case SPC_CMD:
    {
      long a;

      a = (*imk_match_functions[(int) vars->fsm[state].ch].function) (vars->text, j);

      switch (a)
      {
      case NO_MATCH:
        return IMK_NOMATCH;

      case NORMAL_MATCH:
        j++;
        vars->tested = &vars->tested[size + 1];
        size = 0;
        break;

      case NO_CHAR_MATCH:;
        /* nao faz nada, continua normalmente */
      }
    }
    break;

  case BEGIN_CMD:
    if (!((vars->text[j - 1] == '\n') || (j == 0)))
      return IMK_NOMATCH;

    break;

  case END_CMD:
    if (!((vars->text[j] == '\n') || (vars->text[j] == '\0')))
      return IMK_NOMATCH;

    break;
  }

  return recursive_match (vars, j, vars->fsm[state].next1, capture, size);
}

/************************************************
* verifica se um estado esta presente na pilha *
************************************************/
static int inStack (Stack * stack, int state)
{
  int a;
  for (a = 0; a < stack->size; a++)
    if (stack->stack[a] == state)
      return 1;

  return 0;
}

/************************************
* Prepara uma pilha para ser usada *
************************************/
static void newStack (Stack * new_stack, short *stack)
{
  new_stack->size = 0;
  new_stack->stack = stack;
}

/************************************
* Coloca um novo elemento na pilha *
************************************/
static void pushStack (Stack * stack, short value)
{
  stack->stack[stack->size++] = value;
}

/***********************************************************
* Copia a pilha fonte na pilha destino, apagando a fonte *
***********************************************************/
static void moveStack (Stack * dest, Stack * source)
{
  short *temp = dest->stack;
  dest->stack = source->stack;
  source->stack = temp;

  dest->size = source->size;
  source->size = 0;
}

/************************
* Funcao nao recursiva *
************************/
static long local_match (const char *text, iMask * fsm, long start, char *addchar, int casei)
{
  int finished = IMK_NOMATCH;
  Stack now, next;
  short a1[MIN_STACK_ELEMENTS];
  short a2[MIN_STACK_ELEMENTS];
  int state;
  int j = 0;
  int pos;

  if (addchar) addchar[0] = 0;   

  j = start;

  newStack(&now, a1);
  newStack(&next, a2);

  pushStack (&now, fsm[0].next1);

  for (;;)
  {
    for (pos = 0; pos < now.size; pos++)
    {
      state = now.stack[pos];

      if (state == 0)
      {
        finished = j - start;
        continue;
      }

      if (fsm[state].command == NULL_CMD)
      {
        if(!inStack (&now, fsm[state].next2))
          pushStack (&now, fsm[state].next2);

        if(fsm[state].next1 != fsm[state].next2)
        {
          if(!inStack (&now, fsm[state].next1))
            pushStack (&now, fsm[state].next1);
        }
      }
      else if (text[j] == '\0');
      /* faz com que \0 seja ignorado */

      else if (((fsm[state].command == CHAR_CMD) &&
        ((!casei && fsm[state].ch == text[j]) ||
        (casei && tolower(fsm[state].ch) == tolower(text[j]))
        )
        ) ||
        ((fsm[state].command == ANY_CMD) &&
        (text[j] != '\n')
        )
        )
        pushStack (&next, fsm[state].next1);
      else if (fsm[state].command == SPC_CMD)
      {
        int ret;

        ret = (*(imk_match_functions[(int) fsm[state].ch].function))(text, j);
        switch (ret)
        {
        case NO_MATCH:
          break;

        case NORMAL_MATCH:
          pushStack (&next, fsm[state].next1);
          break;

        case NO_CHAR_MATCH:
          pushStack (&now, fsm[state].next1);
          break;
        }
      }
      else if (fsm[state].command == CLASS_CMD)
      {
        int temp, found = 0, negate;

        temp = fsm[state].next1;
        negate = fsm[state].next2;
        state++;

        while (fsm[state].command != NULL_CMD)
        {
          if (fsm[state].command == CLASS_CMD_RANGE)
          {
            if((!casei && (text[j]>=fsm[state].ch) &&
              (text[j]<=fsm[state].next1)
              ) ||
              (casei && (tolower(text[j])>=tolower(fsm[state].ch)) &&
              (tolower(text[j])<=tolower(fsm[state].next1))
              )
              )
            {
              found = 1;
              break;
            }
          }
          else if ((fsm[state].command == CLASS_CMD_CHAR) &&
            ((!casei && text[j] == fsm[state].ch) ||
            (casei && tolower(text[j]) == tolower(fsm[state].ch))
            )
            )
          {
            found = 1;
            break;
          }
          state++;
        }

        if(found ^ negate)
        {
          pushStack (&next, temp);
          state = temp;
        }
      }
      else if (fsm[state].command == BEGIN_CMD)
      {
        if (text[j - 1] == '\n' || j == 0)
          pushStack (&now, fsm[state].next1);
      }
      else if (fsm[state].command == END_CMD)
      {
        if (text[j] == '\n' || text[j] == '\0')
          pushStack (&now, fsm[state].next1);
      }
    }

    if (text[j] == '\0')
    {
      if(next.size == 0 && finished == j)
      {
        return finished;
      }
      else if(addchar)
      {
        int pos;

        for (pos = 0; pos < now.size; pos++)
        {
          state = now.stack[pos];
          if (fsm[state].command == CHAR_CMD)
          {
            pushStack (&next, state);
          }
          else if (fsm[state].command != NULL_CMD)
          {
            next.size = 0;
            break;
          }
          else
          {
            if (!inStack (&now, fsm[state].next2))
              pushStack (&now, fsm[state].next2);

            if (fsm[state].next1 != fsm[state].next2)
            {
              if (!inStack (&now, fsm[state].next1))
                pushStack (&now, fsm[state].next1);
            }
          }           
        }

        moveStack (&now, &next);

        if (now.size == 1)
        {
          int inx=0;
          state = now.stack[0];
          while(fsm[state].next1 == fsm[state].next2)
          {
            if(fsm[state].command == CHAR_CMD)
              addchar[inx++] = fsm[state].ch;
            else if(fsm[state].command != NULL_CMD)
              break;

            state = fsm[state].next1;
          }
          addchar[inx]=0;
        }
      }
      return IMK_PARTIALMATCH;
    }

    j++;

    if (next.size == 0)
    {
      if (finished > IMK_NOMATCH)
        return finished;

      return IMK_NOMATCH;
    }

    moveStack (&now, &next);
  }
}

/******************************************************/
/* Decide qual funcao percorrera a maquina de estados */
/* (recursiva ou nao-recursiva)                       */
/******************************************************/
int iMaskMatch (const char *text, iMask * fsm, long start, iMaskMatchFunc function, void *user, char *addchar, int icase)
{
  long ret;
  short tested[10000];          /* vai ser eliminado */
  vars_st vars;

  /* se nao houver capturas no padrao, chama funcao nao-recursiva */

  if (fsm[0].ch == NOCAPTURE)
    return local_match (text, fsm, start, addchar, icase);

  /* dados usados pela funcao recursiva */

  vars.text = text;
  vars.fsm = fsm;
  vars.tested = tested;
  vars.function = function;

  vars.user = user;

  ret = recursive_match (&vars, start, fsm[0].next1, NULL, 0);

  return (int)((ret >= start) ? ret - start : ret);
}

