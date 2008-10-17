/** \file
 * \brief manage hot keys encoding and decoding.
 *
 * See Copyright Notice in iup.h
 * $Id: ikey.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdio.h>      /* sprintf */

#include "iglobal.h"
#include "iupkey.h"

/* remember the conflicts: K_BS=K_cH, K_TAB=K_cI and K_CR=K_cM. */

static struct
{
   char *s;
   int c;
} ikey_map[]= 
  {
    {"K_exclam",         K_exclam},               
    {"K_grave",          K_grave},             
    {"K_quotedbl",       K_quotedbl},             
    {"K_numbersign",     K_numbersign},           
    {"K_dollar",         K_dollar},               
    {"K_percent",        K_percent},              
    {"K_ampersand",      K_ampersand},            
    {"K_apostrophe",     K_apostrophe},           
    {"K_parentleft",     K_parentleft},            
    {"K_parentright",    K_parentright},          
    {"K_asterisk",       K_asterisk},             
    {"K_plus",           K_plus},                 
    {"K_comma",          K_comma},                
    {"K_minus",          K_minus},                
    {"K_period",         K_period},               
    {"K_slash",          K_slash},                
    {"K_colon",          K_colon},                
    {"K_semicolon ",     K_semicolon},            
    {"K_less",           K_less},                 
    {"K_equal",          K_equal},                
    {"K_greater",        K_greater},              
    {"K_question",       K_question},             
    {"K_bracketleft",    K_bracketleft},          
    {"K_backslash",      K_backslash},            
    {"K_bracketright",   K_bracketright},         
    {"K_circum",         K_circum},               
    {"K_underscore",     K_underscore},           
    {"K_quoteleft",      K_quoteleft},            
    {"K_at",             K_at},                   
    {"K_braceleft",      K_braceleft},            
    {"K_bar",            K_bar},                  
    {"K_braceright",     K_braceright},           
    {"K_tilde",          K_tilde},                
    {"K_ESC"   ,K_ESC},
    {"K_BS"    ,K_BS},
    {"K_PAUSE" ,K_PAUSE},
    {"K_CR"    ,K_CR},     {"K_sCR"   ,K_sCR},
    {"K_SP"    ,K_SP},     {"K_sSP"   ,K_sSP},    {"K_cSP"   ,K_cSP},
    {"K_TAB"   ,K_TAB},    {"K_sTAB"  ,K_sTAB},   {"K_cTAB"  ,K_cTAB},    {"K_mTAB"  ,K_mTAB},
    {"K_HOME"  ,K_HOME},   {"K_sHOME" ,K_sHOME},  {"K_cHOME" ,K_cHOME},   {"K_mHOME" ,K_mHOME},  
    {"K_UP"    ,K_UP},     {"K_sUP"   ,K_sUP},    {"K_cUP"   ,K_cUP},     {"K_mUP"   ,K_mUP},    
    {"K_PGUP"  ,K_PGUP},   {"K_sPGUP" ,K_sPGUP},  {"K_cPGUP" ,K_cPGUP},   {"K_mPGUP" ,K_mPGUP},  
    {"K_LEFT"  ,K_LEFT},   {"K_sLEFT" ,K_sLEFT},  {"K_cLEFT" ,K_cLEFT},   {"K_mLEFT" ,K_mLEFT},  
    {"K_RIGHT" ,K_RIGHT},  {"K_sRIGHT",K_sRIGHT}, {"K_cRIGHT",K_cRIGHT},  {"K_mRIGHT",K_mRIGHT}, 
    {"K_END"   ,K_END},    {"K_sEND"  ,K_sEND},   {"K_cEND"  ,K_cEND},    {"K_mEND"  ,K_mEND},   
    {"K_DOWN"  ,K_DOWN},   {"K_sDOWN" ,K_sDOWN},  {"K_cDOWN" ,K_cDOWN},   {"K_mDOWN" ,K_mDOWN},  
    {"K_PGDN"  ,K_PGDN},   {"K_sPGDN" ,K_sPGDN},  {"K_cPGDN" ,K_cPGDN},   {"K_mPGDN" ,K_mPGDN},  
    {"K_INS"   ,K_INS},    {"K_sINS"   ,K_sINS},  {"K_cINS"  ,K_cINS},    {"K_mINS"  ,K_mINS},   
    {"K_DEL"   ,K_DEL},    {"K_sDEL"   ,K_sDEL},  {"K_cDEL"  ,K_cDEL},    {"K_mDEL"  ,K_mDEL},   
    {"K_MIDDLE",K_MIDDLE},                        {"K_cMIDDLE",K_cMIDDLE},
    {"K_0",  K_0},   {"K_m0", K_m0},                           
    {"K_1",  K_1},   {"K_m1", K_m1},                           
    {"K_2",  K_2},   {"K_m2", K_m2},                           
    {"K_3",  K_3},   {"K_m3", K_m3},                           
    {"K_4",  K_4},   {"K_m4", K_m4},                           
    {"K_5",  K_5},   {"K_m5", K_m5},                           
    {"K_6",  K_6},   {"K_m6", K_m6},                           
    {"K_7",  K_7},   {"K_m7", K_m7},                            
    {"K_8",  K_8},   {"K_m8", K_m8}, 
    {"K_9",  K_9},   {"K_m9", K_m9}, 
    {"K_A",  K_A},   {"K_a",  K_a},    {"K_cA"  ,K_cA},    {"K_mA"  ,K_mA},  
    {"K_B",  K_B},   {"K_b",  K_b},    {"K_cB"  ,K_cB},    {"K_mB"  ,K_mB},  
    {"K_C",  K_C},   {"K_c",  K_c},    {"K_cC"  ,K_cC},    {"K_mC"  ,K_mC},  
    {"K_D",  K_D},   {"K_d",  K_d},    {"K_cD"  ,K_cD},    {"K_mD"  ,K_mD},  
    {"K_E",  K_E},   {"K_e",  K_e},    {"K_cE"  ,K_cE},    {"K_mE"  ,K_mE},  
    {"K_F",  K_F},   {"K_f",  K_f},    {"K_cF"  ,K_cF},    {"K_mF"  ,K_mF},  
    {"K_G",  K_G},   {"K_g",  K_g},    {"K_cG"  ,K_cG},    {"K_mG"  ,K_mG},  
    {"K_H",  K_H},   {"K_h",  K_h},    {"K_cH"  ,K_cH},    {"K_mH"  ,K_mH},  
    {"K_I",  K_I},   {"K_i",  K_i},    {"K_cI"  ,K_cI},    {"K_mI"  ,K_mI},  
    {"K_J",  K_J},   {"K_j",  K_j},    {"K_cJ"  ,K_cJ},    {"K_mJ"  ,K_mJ},  
    {"K_K",  K_K},   {"K_k",  K_k},    {"K_cK"  ,K_cK},    {"K_mK"  ,K_mK},  
    {"K_L",  K_L},   {"K_l",  K_l},    {"K_cL"  ,K_cL},    {"K_mL"  ,K_mL},  
    {"K_M",  K_M},   {"K_m",  K_m},    {"K_cM"  ,K_cM},    {"K_mM"  ,K_mM},  
    {"K_N",  K_N},   {"K_n",  K_n},    {"K_cN"  ,K_cN},    {"K_mN"  ,K_mN},  
    {"K_O",  K_O},   {"K_o",  K_o},    {"K_cO"  ,K_cO},    {"K_mO"  ,K_mO},  
    {"K_P",  K_P},   {"K_p",  K_p},    {"K_cP"  ,K_cP},    {"K_mP"  ,K_mP},  
    {"K_Q",  K_Q},   {"K_q",  K_q},    {"K_cQ"  ,K_cQ},    {"K_mQ"  ,K_mQ},  
    {"K_R",  K_R},   {"K_r",  K_r},    {"K_cR"  ,K_cR},    {"K_mR"  ,K_mR},  
    {"K_S",  K_S},   {"K_s",  K_s},    {"K_cS"  ,K_cS},    {"K_mS"  ,K_mS},  
    {"K_T",  K_T},   {"K_t",  K_t},    {"K_cT"  ,K_cT},    {"K_mT"  ,K_mT},  
    {"K_U",  K_U},   {"K_u",  K_u},    {"K_cU"  ,K_cU},    {"K_mU"  ,K_mU},  
    {"K_V",  K_V},   {"K_v",  K_v},    {"K_cV"  ,K_cV},    {"K_mV"  ,K_mV},  
    {"K_W",  K_W},   {"K_w",  K_w},    {"K_cW"  ,K_cW},    {"K_mW"  ,K_mW},  
    {"K_X",  K_X},   {"K_x",  K_x},    {"K_cX"  ,K_cX},    {"K_mX"  ,K_mX},  
    {"K_Y",  K_Y},   {"K_y",  K_y},    {"K_cY"  ,K_cY},    {"K_mY"  ,K_mY},  
    {"K_Z",  K_Z},   {"K_z",  K_z},    {"K_cZ"  ,K_cZ},    {"K_mZ"  ,K_mZ},  
    {"K_F1"    ,K_F1},     {"K_sF1"   ,K_sF1},    {"K_cF1"   ,K_cF1},     {"K_mF1"   ,K_mF1},  
    {"K_F2"    ,K_F2},     {"K_sF2"   ,K_sF2},    {"K_cF2"   ,K_cF2},     {"K_mF2"   ,K_mF2},  
    {"K_F3"    ,K_F3},     {"K_sF3"   ,K_sF3},    {"K_cF3"   ,K_cF3},     {"K_mF3"   ,K_mF3},  
    {"K_F4"    ,K_F4},     {"K_sF4"   ,K_sF4},    {"K_cF4"   ,K_cF4},     {"K_mF4"   ,K_mF4},  
    {"K_F5"    ,K_F5},     {"K_sF5"   ,K_sF5},    {"K_cF5"   ,K_cF5},     {"K_mF5"   ,K_mF5},  
    {"K_F6"    ,K_F6},     {"K_sF6"   ,K_sF6},    {"K_cF6"   ,K_cF6},     {"K_mF6"   ,K_mF6},  
    {"K_F7"    ,K_F7},     {"K_sF7"   ,K_sF7},    {"K_cF7"   ,K_cF7},     {"K_mF7"   ,K_mF7},  
    {"K_F8"    ,K_F8},     {"K_sF8"   ,K_sF8},    {"K_cF8"   ,K_cF8},     {"K_mF8"   ,K_mF8},  
    {"K_F9"    ,K_F9},     {"K_sF9"   ,K_sF9},    {"K_cF9"   ,K_cF9},     {"K_mF9"   ,K_mF9},  
    {"K_F10"   ,K_F10},    {"K_sF10"  ,K_sF10},   {"K_cF10"  ,K_cF10},    {"K_mF10"  ,K_mF10}, 
    {"K_F11"   ,K_F11},    {"K_sF11"  ,K_sF11},   {"K_cF11"  ,K_cF11},    {"K_mF11"  ,K_mF11}, 
    {"K_F12"   ,K_F12},    {"K_sF12"  ,K_sF12},   {"K_cF12"  ,K_cF12},    {"K_mF12"  ,K_mF12}, 
    {NULL      ,0}
  };

int iupKeyCanCaps(int iupcode)
{
  if (iupcode >= K_a && iupcode <= K_z)
    return 1;
#ifndef IUP_NO_ABNT
  if (iupcode == 'ç')
    return 1;
#endif
  return 0;
}

char *iupKeyEncode(int c)
{
  int i;
  for (i=0; ikey_map[i].s; ++i)
    if (c == ikey_map[i].c)
      return ikey_map[i].s;
      
  return NULL;
}

int iupKeyDecode(char *c)
{
  int i;
  for (i=0; ikey_map[i].s; ++i)
    if (iupStrEqual (c, ikey_map[i].s))
      return ikey_map[i].c;
      
  return -1;
}

void iupKeyForEach(void (*func)(char *name, int code, void* user_data), void* user_data)
{
  int i;
  for (i=0; ikey_map[i].s; ++i)
    func(ikey_map[i].s, ikey_map[i].c, user_data);
}
