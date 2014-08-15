/** \file
 * \brief Windows Driver keyboard mapping
 *
 * See Copyright Notice in iup.h
 *  */

#include <windows.h>
#include "iglobal.h"
#include "win.h"
#include "iupkey.h"

#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS       0xBB   /* '+' any country */
#define VK_OEM_COMMA      0xBC   /* ',' any country */
#define VK_OEM_MINUS      0xBD   /* '-' any country */
#define VK_OEM_PERIOD     0xBE   /* '.' any country */
#define VK_OEM_102        0xE2   /*  "<>" or "\|" on RT 102-key kbd. */
#endif

typedef struct _Iwin2iupkey
{
	int wincode;
	int iupcode;
	int s_iupcode;
	int c_iupcode;
	int m_iupcode;
} Iwin2iupkey;

static Iwin2iupkey winkey_map[] = {

{ VK_BACK,      K_BS,    K_BS,      0,        0        },
{ VK_TAB,       K_TAB,   K_sTAB,    K_cTAB,   K_mTAB   },
{ VK_RETURN,    K_CR,    K_sCR,     0,        0        },
{ VK_ESCAPE,    K_ESC,   K_ESC,     0,        0        },
{ VK_SPACE,     K_SP,    K_sSP,     K_cSP,    0        },
{ VK_CLEAR,     K_MIDDLE,0,         K_cMIDDLE,0        },
{ VK_PAUSE,     K_PAUSE, 0,         0,        0        },

{ VK_HOME,      K_HOME,  K_sHOME,   K_cHOME,  K_mHOME  },
{ VK_UP,        K_UP,    K_sUP,     K_cUP,    K_mUP    },
{ VK_PRIOR,     K_PGUP,  K_sPGUP,   K_cPGUP,  K_mPGUP  },
{ VK_LEFT,      K_LEFT,  K_sLEFT,   K_cLEFT,  K_mLEFT  },
{ VK_END,       K_END,   K_sEND,    K_cEND,   K_mEND   },
{ VK_RIGHT,     K_RIGHT, K_sRIGHT,  K_cRIGHT, K_mRIGHT },
{ VK_DOWN,      K_DOWN,  K_sDOWN,   K_cDOWN,  K_mDOWN  },
{ VK_NEXT,      K_PGDN,  K_sPGDN,   K_cPGDN,  K_mPGDN  },

{ VK_INSERT,    K_INS,   K_sINS,    K_cINS,   K_mINS   },
{ VK_DELETE,    K_DEL,   K_sDEL,    K_cDEL,   K_mDEL   },

{ '1',   K_1, K_exclam,      0, K_m1 },
{ '2',   K_2, K_at,          0, K_m2 },
{ '3',   K_3, K_numbersign,  0, K_m3 },
{ '4',   K_4, K_dollar,      0, K_m4 },
{ '5',   K_5, K_percent,     0, K_m5 },
{ '6',   K_6, K_circum,      0, K_m6 },
{ '7',   K_7, K_ampersand,   0, K_m7 },
{ '8',   K_8, K_asterisk,    0, K_m8 },
{ '9',   K_9, K_parentleft,  0, K_m9 },
{ '0',   K_0, K_parentright, 0, K_m0 },

{ 'A',   K_a, K_A, K_cA, K_mA},
{ 'B',   K_b, K_B, K_cB, K_mB},
{ 'C',   K_c, K_C, K_cC, K_mC},
{ 'D',   K_d, K_D, K_cD, K_mD},
{ 'E',   K_e, K_E, K_cE, K_mE},
{ 'F',   K_f, K_F, K_cF, K_mF},
{ 'G',   K_g, K_G, K_cG, K_mG},
{ 'H',   K_h, K_H, K_cH, K_mH},
{ 'I',   K_i, K_I, K_cI, K_mI},
{ 'J',   K_j, K_J, K_cJ, K_mJ},
{ 'K',   K_k, K_K, K_cK, K_mK},
{ 'L',   K_l, K_L, K_cL, K_mL},
{ 'M',   K_m, K_M, K_cM, K_mM},
{ 'N',   K_n, K_N, K_cN, K_mN},
{ 'O',   K_o, K_O, K_cO, K_mO},
{ 'P',   K_p, K_P, K_cP, K_mP},
{ 'Q',   K_q, K_Q, K_cQ, K_mQ},
{ 'R',   K_r, K_R, K_cR, K_mR},
{ 'S',   K_s, K_S, K_cS, K_mS},
{ 'T',   K_t, K_T, K_cT, K_mT},
{ 'U',   K_u, K_U, K_cU, K_mU},
{ 'V',   K_v, K_V, K_cV, K_mV},
{ 'W',   K_w, K_W, K_cW, K_mW},
{ 'X',   K_x, K_X, K_cX, K_mX},
{ 'Y',   K_y, K_Y, K_cY, K_mY},
{ 'Z',   K_z, K_Z, K_cZ, K_mZ},

{ VK_F1,  K_F1,  K_sF1,  K_cF1,  K_mF1  },
{ VK_F2,  K_F2,  K_sF2,  K_cF2,  K_mF2  },
{ VK_F3,  K_F3,  K_sF3,  K_cF3,  K_mF3  },
{ VK_F4,  K_F4,  K_sF4,  K_cF4,  K_mF4  },
{ VK_F5,  K_F5,  K_sF5,  K_cF5,  K_mF5  },
{ VK_F6,  K_F6,  K_sF6,  K_cF6,  K_mF6  },
{ VK_F7,  K_F7,  K_sF7,  K_cF7,  K_mF7  },
{ VK_F8,  K_F8,  K_sF8,  K_cF8,  K_mF8  },
{ VK_F9,  K_F9,  K_sF9,  K_cF9,  K_mF9  },
{ VK_F10, K_F10, K_sF10, K_cF10, K_mF10 },
{ VK_F11, K_F11, K_sF11, K_cF11, K_mF11 },
{ VK_F12, K_F12, K_sF12, K_cF12, K_mF12 },

{ VK_OEM_1,      K_semicolon,    K_colon,      0, 0 },
{ VK_OEM_PLUS,   K_equal,        K_plus,       0, 0 },
{ VK_OEM_COMMA,  K_comma,        K_less,       0, 0 },
{ VK_OEM_MINUS,  K_minus,        K_underscore, 0, 0 },
{ VK_OEM_PERIOD, K_period,       K_greater,    0, 0 },
{ VK_OEM_2,      K_slash,        K_question,   0, 0 },
{ VK_OEM_3,      K_grave,        K_tilde,      0, 0 },
{ VK_OEM_4,      K_bracketleft,  K_braceleft,  0, 0 },
{ VK_OEM_5,      K_backslash,    K_bar,        0, 0 },
{ VK_OEM_6,      K_bracketright, K_braceright, 0, 0 },
{ VK_OEM_7,      K_apostrophe,   K_quotedbl,   0, 0 },
{ VK_OEM_102,    K_backslash,    K_bar,        0, 0 },

{ VK_NUMPAD0,   K_0, K_0,  0, K_m0 },
{ VK_NUMPAD1,   K_1, K_1,  0, K_m1 },
{ VK_NUMPAD2,   K_2, K_2,  0, K_m2 },
{ VK_NUMPAD3,   K_3, K_3,  0, K_m3 },
{ VK_NUMPAD4,   K_4, K_4,  0, K_m4 },
{ VK_NUMPAD5,   K_5, K_5,  0, K_m5 },
{ VK_NUMPAD6,   K_6, K_6,  0, K_m6 },
{ VK_NUMPAD7,   K_7, K_7,  0, K_m7 },
{ VK_NUMPAD8,   K_8, K_8,  0, K_m8 },
{ VK_NUMPAD9,   K_9, K_9,  0, K_m9 },
{ VK_MULTIPLY,  K_asterisk, 0, 0, 0 },
{ VK_ADD,       K_plus,     0, 0, 0 },
{ VK_SUBTRACT,  K_minus,    0, 0, 0 },
{ VK_DECIMAL,   K_period,   0, 0, 0 },
{ VK_DIVIDE,    K_slash,    0, 0, 0 }
};

#ifndef IUP_NO_ABNT
static Iwin2iupkey keytable_abnt[] = {
{ '1',   K_1, K_exclam,      '¹', K_m1 },
{ '2',   K_2, K_at,          '²', K_m2 },
{ '3',   K_3, K_numbersign,  '³', K_m3 },
{ '4',   K_4, K_dollar,      '£', K_m4 },
{ '5',   K_5, K_percent,     '¢', K_m5 },
{ '6',   K_6, K_circum,      '¬', K_m6 },

{ VK_OEM_PLUS, K_equal,        K_plus,       '§', 0 },
{ VK_OEM_1,    'ç',            'Ç',          0,   0 },  
{ VK_OEM_2,    K_semicolon,    K_colon,      0,   0 },
{ VK_OEM_3,    K_apostrophe,   K_quotedbl,   0,   0 },
{ VK_OEM_4,    K_grave,        '´',          0,   0 },
{ VK_OEM_5,    K_bracketright, K_braceright, 'º', 0 },
{ VK_OEM_6,    K_bracketleft,  K_braceleft,  'ª', 0 },
{ VK_OEM_7,    K_circum,       K_tilde,      0,   0 },
{ VK_OEM_102,  K_backslash,    K_bar,        0,   0 },
{ 0xC1,        K_slash,        K_question,   '°', 0 }
};
#endif

static int winKeyMap2Iup(Iwin2iupkey* table, int i)
{
  if (GetKeyState(VK_CONTROL) & 0x8000)
    return table[i].c_iupcode;
  else if (GetKeyState(VK_MENU) & 0x8000) 
    return table[i].m_iupcode;
  else if (GetKeyState(VK_CAPITAL) & 0x01)
  {
    if ((GetKeyState(VK_SHIFT) & 0x8000) || !iupKeyCanCaps(table[i].iupcode))
      return table[i].iupcode;
    else
      return table[i].s_iupcode;
  }
  else if (GetKeyState(VK_SHIFT) & 0x8000)
    return table[i].s_iupcode;
  else
    return table[i].iupcode;
}

int iupwinKeyDecode(int winkey)
{
  int i;                        

#ifndef IUP_NO_ABNT
  HKL k = GetKeyboardLayout(0);    
  if((int)HIWORD(k) == 0x416) /* ABNT */
  {
    for ( i=0; i < sizeof(keytable_abnt)/sizeof(keytable_abnt[0]); i++)
    {
      if (keytable_abnt[i].wincode == winkey)
        return winKeyMap2Iup(keytable_abnt, i);
    }
  }
#endif

  for ( i=0; i < sizeof(winkey_map)/sizeof(winkey_map[0]); i++)
  {
    if (winkey_map[i].wincode == winkey)
      return winKeyMap2Iup(winkey_map, i);
  }

  return 0;
}

static int winKeyCallKAnyCb(Ihandle *p, int c)
{
  char *attr=iupKeyEncode(c);
  for (; p; p=parent(p))
  {
    IFni cb = NULL;
    if (attr)
      cb = (IFni)IupGetCallback(p,attr);
    if (!cb)
      cb=(IFni)IupGetCallback(p,IUP_K_ANY);

    if (cb)
    {
      int result = cb(p, c);

      if(result != IUP_CONTINUE)
        return result;
    }
  }
  return 0;
}

int iupwinKeyProcess( Ihandle* h, int winkey )
{
  int key = iupwinKeyDecode(winkey);

	if (key)
		return winKeyCallKAnyCb( h, key );
	else
		return 0;
}
