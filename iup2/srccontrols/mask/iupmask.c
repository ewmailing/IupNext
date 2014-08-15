/** \file
 * \brief iupmask imk_match_functions - pattern matching
 *
 * See Copyright Notice in iup.h
 *  */

/*
Arquivo que implementa as funcoes que permitem o uso das funcoes de pattern
matching sobre um texto do IUP.

A funcao de pattern matching usada foi desenvolvida por Pedro Miller,
Milton Johnatan, Pedro Willensems e Vinicius Almendra, sendo modificadas
para a presente biblioteca por Carlos Augusto Teixeira Mendes e Pedro Miller.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <iup.h>
#include <iupmatrix.h>
#include <iupcbs.h>
#include <iupkey.h>

#include "iupmask.h"
#include "istrutil.h"

#include "imask.h"

typedef struct _iupMask
{
  const char* mask;
  iMask* fsm;
  int autofill,
      casei,
      min,
      max,
      lin, 
      col,
      is_matrix;
  char type;
  float fmin, 
        fmax;
} iupMask;

/* Atributos usados internamente */

#define ATTR_OLD_ACTION  "_IUP_MASK_OLD_ACTION"


/*
%F Funcao chamada pela callback de texto para validar a tecla pressionada.
Ela submete a string futura ao algoritmo de pattern matching para validar
a tecla.

h        - Handle do texto
c        - tecla pressionada
newstr   - String futura informada pelo IUP
lin, col - Linha e coluna da celua, no caso de h se referir
           a uma matriz
mode     - Se mode = 1, leva em conta o valor atual do campo na construcao da
           string futura, se for igual a 0 finge que o valor atual e' "".
newstr   - Buffer do usuario que vai ser usado para compor a string futura.
           Se o modo autofill estiver selecionado (atraves do atributo IUP
           correspondente), e a funcao de pattern match retornar uma string
           para ser adicionada ao valor futuro, esta string sera concatenada
           ao valor da string calculada e colocada neste buffer.
fill     - Tem o seu valor setada para um valor > 0 se a funcao de pattern match
           retornou uma string para ser adicionada ao texto futuro.  Retorna 0
           caso contrario.  O valor > 0 e' a posicao e (zero based) em que o
           caret deve ser colocado.
match    - Tem o seu valor setado para 1 se a string completa o pattern, 0 caso
           ele ainda nao complete o pattern (IMK_PARTIALMATCH)

Retorna IUP_IGNORE se a string futura nao e' compativel com a mascara, IUP_DEFAULT
caso contrario.
*/
static int DoMatch(Ihandle *h, iupMask* iup_mask, int c, char *newstr, int *fill, int *match, int mode)
{
  char newchars[200];
  int  ret, pos, oldlen, newlen, autofill;

  *fill  = 0;
  *match = 0;

  /* Se mode == 0, string futura e' gerada apenas a partir da tecla.
     Usado na edicao inicial da matriz...  */
  if(!mode)
  {
    pos    = 0;
    oldlen = 0;
  }
  else
  {
    char* oldstr = IupGetAttribute(h,IUP_VALUE);
    pos = 0;
    oldlen = strlen(oldstr);
  }
  newlen = (int)strlen(newstr);

  autofill = iup_mask->autofill;

  /* Se vou apagar a ultima letra do texto, desliga o modo autofill. Isto
  e' feito pois se a ultima letra for um literal colocado por um autofill,
  anterior, entao ao apagarmos esta letra, ela volta como resultado do
  autofill, e como consequencia nunca conseguimos apaga-la.
  */
  if((c == K_BS && pos == oldlen) || ((c == K_DEL || c == K_sDEL) && pos == oldlen-1))
    autofill = 0;

  /* Submete a string futura ao pattern match */
  ret = iMaskMatch(newstr,iup_mask->fsm,0,NULL,NULL,autofill?newchars:NULL,iup_mask->casei);

  /* Aceitamos a string se o pattern match retornou IMK_PARTIALMATCH ou
  se ele aceitou a string completa (ret == strlen(newstr))
  */
  if (ret == IMK_PARTIALMATCH || ret == newlen)
  {
    if(ret != IMK_PARTIALMATCH)
      *match = 1;

    if(autofill && newchars[0])
    {
      strcat(newstr,newchars);
      *fill = strlen(newstr);
    }
    return IUP_DEFAULT;
  }

  /* Bom, vamos tentar outro approach:
  Se pos nao esta no final da string, entao pode ser que
  a letra (ou letras) colocada no meio, geraria um autofill, que
  faria com que ela fosse aceita...
  Testa isto.  Para fazer este teste temos que descobrir qual
  parte da string futura e' consequencia da nova tecla e qual
  e' antiga.  Isto pode ser descoberto da seguinte forma:
  Se Ha um trecho marcado no texto, a parte nova entra em
  cima dele.  Caso contrario, podemos achar a regiao atraves
  da diferenca de tamanho entre a string futura e a antiga e
  a posicao do caret.
  */
  if(autofill && pos != oldlen)
  {
    char  resto[200];
    char* oldstr;
    int   start=0, stop=0;
    char *s = IupGetAttribute(h,IUP_SELECTION);

    if (s)
      iupStrToIntInt(s,&start,&stop, ':');
    else 
      start = stop = 0; /* Nao ha area marcada */

    oldstr = IupGetAttribute(h,IUP_VALUE);

    if(start != stop) /* Ha area marcada */
      pos = stop-1;
    strcpy(resto,oldstr+pos);
    newstr[newlen-(oldlen-pos)]=0;

    /* Submete a string futura ao pattern match */
    ret = iMaskMatch(newstr,iup_mask->fsm,0,NULL,NULL,newchars,iup_mask->casei);

    if ((ret != IMK_PARTIALMATCH && ret != (int)strlen(newstr)) || !*newchars)
      return IUP_IGNORE;

    /* Agora apenda new chars e o resto e ve se ainda bate */
    strcat(newstr,newchars);
    *fill = strlen(newstr);
    strcat(newstr,resto);

    ret = iMaskMatch(newstr,iup_mask->fsm,0,NULL,NULL,newchars,iup_mask->casei);

    if (ret == IMK_PARTIALMATCH || ret == (int)strlen(newstr))
    {
      if(ret != IMK_PARTIALMATCH)
        *match = 1;
      if(*newchars)
      {
        strcat(newstr,newchars);
        *fill = strlen(newstr);
      }
      return IUP_DEFAULT;
    }
    return IUP_IGNORE;
  }

  return IUP_IGNORE;
}


/******************************************************************************
*******************************************************************************

Callbacks

*******************************************************************************
******************************************************************************/

static int TextCb(Ihandle *h, iupMask* iup_mask, int c, char* newstr, int* match, int mode)
{
  int ret, fill;

  ret = DoMatch(h,iup_mask,c,newstr,&fill,match,mode);

  if (ret == IUP_DEFAULT && fill)
  {
    IupSetAttribute(h,IUP_VALUE,newstr);
    IupSetfAttribute(h,IUP_CARET,"%d",fill+1);
  }

  return ret;
}

static int IntCb(Ihandle *h, iupMask* iup_mask, int c, char* newstr, int* match, int mode)
{
  int ret, fill;

  ret = DoMatch(h,iup_mask,c,newstr,&fill,match,mode);

  if (ret == IUP_DEFAULT)
  {
    int val = 0;
    sscanf(newstr,"%d",&val);
    if(val < iup_mask->min || val > iup_mask->max)
      return IUP_IGNORE;
  }
  return ret;
}

static int FloatCb(Ihandle *h, iupMask* iup_mask, int c, char* newstr, int* match, int mode)
{
  int ret, fill;

  ret = DoMatch(h,iup_mask,c,newstr,&fill,match,mode);

  if(ret == IUP_DEFAULT)
  {
    float val = 0;
    sscanf(newstr,"%f",&val);
    if(val < iup_mask->fmin || val > iup_mask->fmax)
      return IUP_IGNORE;
  }
  return ret;
}

static int CallOld(Ihandle *h, iupMask* iup_mask, int c, char* after, int mode)
{
  Icallback oldf = (Icallback)IupGetCallback(h,ATTR_OLD_ACTION);

  if(oldf)
  {
    if(iup_mask->is_matrix)
    {
      IFniiiis cb_mat = (IFniiiis)IupGetCallback(h,ATTR_OLD_ACTION);
      return cb_mat(h,c,iup_mask->lin,iup_mask->col,mode,after);
    }
    else
    {
      IFnis cb_text = (IFnis)IupGetCallback(h,ATTR_OLD_ACTION);
      return cb_text(h,c,after);
    }
  }

  return IUP_DEFAULT;
}

static int ActionCb(Ihandle *h, iupMask* iup_mask, int c, int mode, char* after)
{
  static char newstr[200];
  int match;
  int ret = IUP_DEFAULT;

  strcpy(newstr, after);

  switch(iup_mask->type)
  {
  case 'I': ret = IntCb  (h,iup_mask,c,newstr,&match,mode); break;
  case 'F': ret = FloatCb(h,iup_mask,c,newstr,&match,mode); break;
  default : ret = TextCb (h,iup_mask,c,newstr,&match,mode); break;
  }

  if (match)
  {
    Icallback cb = IupGetCallback(h, "MATCH_CB");
    if (cb) cb(h);
  }

  if (ret == IUP_DEFAULT)
    ret = CallOld(h,iup_mask,c,newstr,mode);

  return ret;
}

static int MatActionCb(Ihandle *h, int c, int lin, int col, int mode, char* after)
{
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);
  if(!after || !iup_mask)
  {
    IFniiiis cb_mat = (IFniiiis)IupGetCallback(h,ATTR_OLD_ACTION);
    if (cb_mat)
      return cb_mat(h,c,lin,col,mode,after);
    return IUP_DEFAULT;
  }
  return ActionCb(h,iup_mask,c,mode,after);
}

static int TextActionCb(Ihandle *h, int c, char* after)
{
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");
  if(!after || !iup_mask)
  {
    IFnis cb_text = (IFnis)IupGetCallback(h,ATTR_OLD_ACTION);
    if (cb_text)
      return cb_text(h,c,after);
    return IUP_DEFAULT;
  }
  return ActionCb(h,iup_mask,c,1,after);
}


/******************************************************************************
*******************************************************************************

Check, Set and Remove

*******************************************************************************
******************************************************************************/

static int maskCheck(iupMask* iup_mask, char *val)
{
  if (!val || !(*val) || !iup_mask) 
    return 1;

  if (iMaskMatch(val,iup_mask->fsm,0,NULL,NULL,NULL,iup_mask->casei) != (int)strlen(val))
    return 0;

  switch(iup_mask->type)
  {
  case 'I':
    {
      int ival = 0;
      sscanf(val,"%d",&ival);
      if(ival < iup_mask->min || ival > iup_mask->max)
        return 0;
      break;
    }
  case 'F':
    {
      float fval = 0;
      sscanf(val,"%f",&fval);
      if(fval < iup_mask->fmin || fval > iup_mask->fmax)
        return 0;
      break;
    }
  }

  return 1;
}

static int maskSet(Ihandle* h, const char* mask, int autofill, int casei, int lin, int col)
{
  Icallback oldf = NULL;
  iMask* fsm;
  iupMask* iup_mask;

  /* Faz o parse da mascara para ver se ela e' valida */
  if(iMaskParse(mask,&fsm) != IMK_PARSE_OK)
    return 0;

  iup_mask = (iupMask*)malloc(sizeof(iupMask));

  iup_mask->mask = mask;
  iup_mask->autofill = autofill;
  iup_mask->casei = casei;
  iup_mask->lin = lin;
  iup_mask->col = col;
  iup_mask->col = col;
  iup_mask->fsm = fsm;

  iup_mask->min = 0;
  iup_mask->max = 0;
  iup_mask->fmin = 0;
  iup_mask->fmax = 0;
  iup_mask->type = 0;

  iup_mask->is_matrix = iupStrEqual(IupGetClassName(h),"matrix");

  /* Vou me pendurar na callback de ACTION.
  Verifica se ja tinha alguma funcao pendurada nela.  Se tinha, guarda
  o ponteiro para poder chamar a funcao antiga da nossa callback...
  */
  if(iup_mask->is_matrix)
  {
    iupMask* old_iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA",lin,col);
    if (old_iup_mask) {free(old_iup_mask->fsm); free(old_iup_mask); }

    IupMatSetAttribute(h,"_IUPMASK_DATA",lin,col,(char*)iup_mask);

    /* Usamos o atributo interno da matriz "ACTION_CB".  */
    oldf = IupGetCallback(h, "ACTION_CB");
    if(oldf && oldf != (Icallback)MatActionCb)
      IupSetCallback(h,ATTR_OLD_ACTION,oldf);
    IupSetCallback(h,"ACTION_CB",(Icallback)MatActionCb);
  }
  else
  {
    char* action_attrib;

    iupMask* old_iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");
    if (old_iup_mask) {free(old_iup_mask->fsm); free(old_iup_mask); }

    IupSetAttribute(h,"_IUPMASK_DATA",(char*)iup_mask);

    action_attrib = "ACTION";
    if (iupStrEqual(IupGetClassName(h), "list"))
      action_attrib = "EDIT_CB";

    oldf = IupGetCallback(h, action_attrib);
    if(oldf && oldf != (Icallback)TextActionCb)
      IupSetCallback(h,ATTR_OLD_ACTION,oldf);
    IupSetCallback(h, action_attrib,(Icallback)TextActionCb);
  }

  return 1;
}

static int maskSetInt(Ihandle *h, int autofill, int min, int max, int lin, int col)
{
  int ret;

  if (min < 0)
    ret = maskSet(h,IUPMASK_INT,autofill,0,lin,col);
  else
    ret = maskSet(h,IUPMASK_UINT,autofill,0,lin,col);

  if (ret)
  {
    iupMask* iup_mask;
    
    if (lin==0 && col==0)
      iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");
    else
      iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA",lin,col);

    iup_mask->min = min;
    iup_mask->max = max;
    iup_mask->type = 'I';
  }

  return ret;
}

static int maskSetFloat(Ihandle *h, int autofill, float min, float max, int lin, int col)
{
  int ret;

  if (min < 0.0)
    ret = maskSet(h,IUPMASK_FLOAT,autofill,0,lin,col);
  else
    ret = maskSet(h,IUPMASK_UFLOAT,autofill,0,lin,col);

  if (ret)
  {
    iupMask* iup_mask;
    
    if (lin==0 && col==0)
      iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");
    else
      iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA",lin,col);

    iup_mask->fmin = min;
    iup_mask->fmax = max;
    iup_mask->type = 'F';
  }

  return ret;
}

static void maskRemove(Ihandle *h, iupMask* iup_mask)
{
  if(iup_mask->is_matrix)
  {
    IupSetCallback(h,"ACTION_CB",IupGetCallback(h, ATTR_OLD_ACTION));
  }
  else
  {
    char* action_attrib = "ACTION";
    if (iupStrEqual(IupGetClassName(h), "list"))
      action_attrib = "EDIT_CB";

    IupSetCallback(h, action_attrib, IupGetCallback(h, ATTR_OLD_ACTION));
  }

  IupSetCallback(h,ATTR_OLD_ACTION,NULL);

  free(iup_mask->fsm); 
  free(iup_mask); 
}

/****************************************************************************
                                 iupmask API
 ****************************************************************************/

void iupmaskRemove(Ihandle *h)
{
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");
  if (iup_mask)
    maskRemove(h, iup_mask);
}

void iupmaskMatRemove(Ihandle *h, int lin, int col)
{
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);
  if (iup_mask)
    maskRemove(h, iup_mask);
}

int iupmaskSetInt(Ihandle *h, int autofill, int min, int max)
{
  return maskSetInt(h,autofill,min,max,0,0);
}

int iupmaskMatSetInt(Ihandle *h, int autofill, int min, int max, int lin, int col)
{
  return maskSetInt(h,autofill,min,max,lin,col);
}

int iupmaskSetFloat(Ihandle* h, int autofill, float min, float max)
{
  return maskSetFloat(h,autofill,min,max,0,0);
}

int iupmaskMatSetFloat(Ihandle* h, int autofill, float min, float max, int lin, int col)
{
  return maskSetFloat(h,autofill,min,max,lin,col);
}

int iupmaskSet(Ihandle* h, const char* mask, int autofill, int casei)
{
  return maskSet(h,mask,autofill,casei,0,0);
}

int iupmaskMatSet(Ihandle* h, const char* mask, int autofill, int casei, int lin, int col)
{
  return maskSet(h,mask,autofill,casei,lin,col);
}

int iupmaskCheck(Ihandle* h)
{
  char *val = IupGetAttribute(h,IUP_VALUE);
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");
  return maskCheck(iup_mask,val);
}

int iupmaskMatCheck(Ihandle *h, int lin, int col)
{
  char *val = IupMatGetAttribute(h,"",lin,col);
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);
  return maskCheck(iup_mask,val);
}

int iupmaskGet(Ihandle *h, char **sval)
{
  char *val = IupGetAttribute(h,IUP_VALUE);
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");

  if (maskCheck(iup_mask,val))
  {
    *sval = val;
    return 1;
  }
  else
    return 0;
}

int iupmaskMatGet(Ihandle *h, char **sval, int lin, int col)
{
  char *val = IupMatGetAttribute(h,"",lin,col);
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);

  if (maskCheck(iup_mask,val))
  {
    *sval = val;
    return 1;
  }
  else
    return 0;
}

int iupmaskGetDouble(Ihandle *h, double *dval)
{
  char *val = IupGetAttribute(h,IUP_VALUE);
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");

  if(maskCheck(iup_mask,val))
  {
    *dval = 0.0;
    sscanf(val,"%lf",dval);
    return 1;
  }
  else
    return 0;
}

int iupmaskMatGetDouble(Ihandle *h, double *dval, int lin, int col)
{
  char *val = IupMatGetAttribute(h,"",lin,col);
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);

  if(maskCheck(iup_mask,val))
  {
    *dval = 0.0;
    sscanf(val,"%lf",dval);
    return 1;
  }
  else
    return 0;
}

int iupmaskGetFloat(Ihandle *h, float *fval)
{
  char *val = IupGetAttribute(h,IUP_VALUE);
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");

  if(maskCheck(iup_mask,val))
  {
    *fval = 0.0F;
    sscanf(val,"%f",fval);
    return 1;
  }
  else
    return 0;
}

int iupmaskMatGetFloat(Ihandle *h, float *fval, int lin, int col)
{
  char *val = IupMatGetAttribute(h,"",lin,col);
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);

  if (maskCheck(iup_mask,val))
  {
    *fval = 0.0F;
    sscanf(val,"%f",fval);
    return 1;
  }
  else
    return 0;
}

int iupmaskGetInt(Ihandle *h, int *ival)
{
  char *val = IupGetAttribute(h,IUP_VALUE);
  iupMask* iup_mask = (iupMask*)IupGetAttribute(h,"_IUPMASK_DATA");

  if (maskCheck(iup_mask,val))
  {
    *ival = 0;
    sscanf(val,"%d",ival);
    return 1;
  }
  else
    return 0;
}

int iupmaskMatGetInt(Ihandle *h, int *ival, int lin, int col)
{
  char *val = IupMatGetAttribute(h,"",lin,col);
  iupMask* iup_mask = (iupMask*)IupMatGetAttribute(h,"_IUPMASK_DATA", lin, col);

  if(maskCheck(iup_mask,val))
  {
    *ival = 0;
    sscanf(val,"%d",ival);
    return 1;
  }
  else
    return 0;
}
