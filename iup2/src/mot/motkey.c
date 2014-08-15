/** \file
 * \brief Motif Driver keyboard mapping
 *
 * See Copyright Notice in iup.h
 *  */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <stdio.h>      
#include <Xm/Xm.h>
#include <X11/keysym.h>

#include "iglobal.h"
#include "iupkey.h"
#include "motif.h"

static struct {
  KeySym motcode;
	int iupcode;
	int s_iupcode;
	int c_iupcode;
	int m_iupcode;
} motkey_map[]= {

{ XK_BackSpace, K_BS,    K_BS,      0,        0        },
{ XK_Tab,       K_TAB,   K_sTAB,    K_cTAB,   K_mTAB   },
{ XK_Return,    K_CR,    K_sCR,     0,        0        },
{ XK_Escape,    K_ESC,   K_ESC,     0,        0        },
{ XK_space,     K_SP,    K_sSP,     K_cSP,    0        },
{ XK_Begin,     K_MIDDLE,0,         0,        0        },
{ XK_Pause,     K_PAUSE, 0,         0,        0        },

{ XK_Home,      K_HOME,  K_sHOME,   K_cHOME,  K_mHOME  },
{ XK_Up,        K_UP,    K_sUP,     K_cUP,    K_mUP    },
{ XK_Prior,     K_PGUP,  K_sPGUP,   K_cPGUP,  K_mPGUP  },
{ XK_Left,      K_LEFT,  K_sLEFT,   K_cLEFT,  K_mLEFT  },
{ XK_End,       K_END,   K_sEND,    K_cEND,   K_mEND   },
{ XK_Right,     K_RIGHT, K_sRIGHT,  K_cRIGHT, K_mRIGHT },
{ XK_Down,      K_DOWN,  K_sDOWN,   K_cDOWN,  K_mDOWN  },
{ XK_Next,      K_PGDN,  K_sPGDN,   K_cPGDN,  K_mPGDN  },

{ XK_Insert,    K_INS,   K_sINS,    K_cINS,   K_mINS   },
{ XK_Delete,    K_DEL,   K_sDEL,    K_cDEL,   K_mDEL   },

{ XK_1,   K_1, K_exclam,      0, K_m1 },
{ XK_2,   K_2, K_at,          0, K_m2 },
{ XK_3,   K_3, K_numbersign,  0, K_m3 },
{ XK_4,   K_4, K_dollar,      0, K_m4 },
{ XK_5,   K_5, K_percent,     0, K_m5 },
{ XK_6,   K_6, K_circum,      0, K_m6 },
{ XK_7,   K_7, K_ampersand,   0, K_m7 },
{ XK_8,   K_8, K_asterisk,    0, K_m8 },
{ XK_9,   K_9, K_parentleft,  0, K_m9 },
{ XK_0,   K_0, K_parentright, 0, K_m0 },

{ XK_a,   K_a, K_A, K_cA, K_mA },
{ XK_b,   K_b, K_B, K_cB, K_mB },
{ XK_c,   K_c, K_C, K_cC, K_mC },
{ XK_d,   K_d, K_D, K_cD, K_mD },
{ XK_e,   K_e, K_E, K_cE, K_mE },
{ XK_f,   K_f, K_F, K_cF, K_mF },
{ XK_g,   K_g, K_G, K_cG, K_mG },
{ XK_h,   K_h, K_H, K_cH, K_mH },
{ XK_i,   K_i, K_I, K_cI, K_mI },
{ XK_j,   K_j, K_J, K_cJ, K_mJ },
{ XK_k,   K_k, K_K, K_cK, K_mK },
{ XK_l,   K_l, K_L, K_cL, K_mL },
{ XK_m,   K_m, K_M, K_cM, K_mM },
{ XK_n,   K_n, K_N, K_cN, K_mN },
{ XK_o,   K_o, K_O, K_cO, K_mO },
{ XK_p,   K_p, K_P, K_cP, K_mP },
{ XK_q,   K_q, K_Q, K_cQ, K_mQ },
{ XK_r,   K_r, K_R, K_cR, K_mR },
{ XK_s,   K_s, K_S, K_cS, K_mS },
{ XK_t,   K_t, K_T, K_cT, K_mT },
{ XK_u,   K_u, K_U, K_cU, K_mU },
{ XK_v,   K_v, K_V, K_cV, K_mV },
{ XK_w,   K_w, K_W, K_cW, K_mW },
{ XK_x,   K_x, K_X, K_cX, K_mX },
{ XK_y,   K_y, K_Y, K_cY, K_mY },
{ XK_z,   K_z, K_Z, K_cZ, K_mZ },

{ XK_F1,  K_F1,  K_sF1,  K_cF1,  K_mF1 },
{ XK_F2,  K_F2,  K_sF2,  K_cF2,  K_mF2 },
{ XK_F3,  K_F3,  K_sF3,  K_cF3,  K_mF3 },
{ XK_F4,  K_F4,  K_sF4,  K_cF4,  K_mF4 },
{ XK_F5,  K_F5,  K_sF5,  K_cF5,  K_mF5 },
{ XK_F6,  K_F6,  K_sF6,  K_cF6,  K_mF6 },
{ XK_F7,  K_F7,  K_sF7,  K_cF7,  K_mF7 },
{ XK_F8,  K_F8,  K_sF8,  K_cF8,  K_mF8 },
{ XK_F9,  K_F9,  K_sF9,  K_cF9,  K_mF9 },
{ XK_F10, K_F10, K_sF10, K_cF10, K_mF10 },
{ XK_F11, K_F11, K_sF11, K_cF11, K_mF11 },
{ XK_F12, K_F12, K_sF12, K_cF12, K_mF12 },

{ XK_semicolon,   K_semicolon,    K_colon,      0, 0 },
{ XK_equal,       K_equal,        K_plus,       0, 0 },
{ XK_comma,       K_comma,        K_less,       0, 0 },
{ XK_minus,       K_minus,        K_underscore, 0, 0 },
{ XK_period,      K_period,       K_greater,    0, 0 },
{ XK_slash,       K_slash,        K_question,   0, 0 },
{ XK_grave,       K_grave,        K_tilde,      0, 0 },
{ XK_bracketleft, K_bracketleft,  K_braceleft,  0, 0 },
{ XK_backslash,   K_backslash,    K_bar,        0, 0 },
{ XK_bracketright,K_bracketright, K_braceright, 0, 0 },
{ XK_apostrophe,  K_apostrophe,   K_quotedbl,   0, 0 },

{ XK_KP_0,   K_0, K_0,  0, K_m0 },
{ XK_KP_1,   K_1, K_1,  0, K_m1 },
{ XK_KP_2,   K_2, K_2,  0, K_m2 },
{ XK_KP_3,   K_3, K_3,  0, K_m3 },
{ XK_KP_4,   K_4, K_4,  0, K_m4 },
{ XK_KP_5,   K_5, K_5,  0, K_m5 },
{ XK_KP_6,   K_6, K_6,  0, K_m6 },
{ XK_KP_7,   K_7, K_7,  0, K_m7 },
{ XK_KP_8,   K_8, K_8,  0, K_m8 },
{ XK_KP_9,   K_9, K_9,  0, K_m9 },
{ XK_KP_Multiply,  K_asterisk, 0, 0, 0 },
{ XK_KP_Add,       K_plus,     0, 0, 0 },
{ XK_KP_Subtract,  K_minus,    0, 0, 0 },
{ XK_KP_Decimal,   K_period,   0, 0, 0 },
{ XK_KP_Divide,    K_slash,    0, 0, 0 },

{ XK_ccedilla,     'ç',       'Ç',     0, 0 },
{ XK_dead_tilde,   K_circum,  K_tilde, 0, 0 },
{ XK_dead_acute,   K_grave,   '´',     0, 0 },
{ XK_dead_grave,   K_grave,   K_tilde, 0, 0 },

{ XK_KP_Enter,     K_CR,    K_sCR,     0,        0        },
{ XK_KP_Home,      K_HOME,  K_sHOME,   K_cHOME,  K_mHOME  },
{ XK_KP_Left,      K_LEFT,  K_sLEFT,   K_cLEFT,  K_mLEFT  },
{ XK_KP_Up,        K_UP,    K_sUP,     K_cUP,    K_mUP    },
{ XK_KP_Page_Up,   K_PGUP,  K_sPGUP,   K_cPGUP,  K_mPGUP  },
{ XK_KP_End,       K_END,   K_sEND,    K_cEND,   K_mEND   },
{ XK_KP_Right,     K_RIGHT, K_sRIGHT,  K_cRIGHT, K_mRIGHT },
{ XK_KP_Down,      K_DOWN,  K_sDOWN,   K_cDOWN,  K_mDOWN  },
{ XK_KP_Page_Down, K_PGDN,  K_sPGDN,   K_cPGDN,  K_mPGDN  },
{ XK_KP_Insert,    K_INS,   K_sINS,    K_cINS,   K_mINS   },
{ XK_KP_Delete,    K_DEL,   K_sDEL,    K_cDEL,   K_mDEL   },
{ XK_KP_Begin,     K_MIDDLE,0,         K_cMIDDLE,0        },

};

int iupmotKeyGetCode(XKeyEvent *ev)
{
  int i;
  KeySym motcode = XKeycodeToKeysym(iupmot_display, ev->keycode, 0);
  for ( i=0; i < sizeof(motkey_map)/sizeof(motkey_map[0]); i++)
  {
    if (motkey_map[i].motcode == motcode)
    {
      if (ev->state & ControlMask)
        return motkey_map[i].c_iupcode;
      else if (ev->state & Mod1Mask)
        return motkey_map[i].m_iupcode;
      else if (ev->state & LockMask)
      {
        if ((ev->state & ShiftMask) || !iupKeyCanCaps(motkey_map[i].iupcode))
          return motkey_map[i].iupcode;
        else
          return motkey_map[i].s_iupcode;
      }
      else if (ev->state & ShiftMask)
        return motkey_map[i].s_iupcode;
      else
        return motkey_map[i].iupcode;
    }
  }

  return 0;
}

int iupmotKeyGetMenuKey (char *c)
{
   int code = iupKeyDecode(c);
   if (code != -1 && code > 50 && code < 86) return code;
   return 0;
}
