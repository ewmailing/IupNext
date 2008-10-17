/** \file
 * \brief Keyboard Keys definitions.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPKEY_H 
#define __IUPKEY_H

/* from 32 to 126, all character sets are equal */

#define K_SP          ' '   /* 32 */
#define K_exclam      '!'   /* 33 */
#define K_quotedbl    '\"'  /* 34 */
#define K_numbersign  '#'   /* 35 */
#define K_dollar      '$'   /* 36 */
#define K_percent     '%'   /* 37 */
#define K_ampersand   '&'   /* 38 */
#define K_apostrophe  '\''  /* 39 */
#define K_parentleft  '('   /* 40 */
#define K_parentright ')'   /* 41 */
#define K_asterisk    '*'   /* 42 */
#define K_plus        '+'   /* 43 */
#define K_comma       ','   /* 44 */
#define K_minus       '-'   /* 45 */
#define K_period      '.'   /* 46 */
#define K_slash       '/'   /* 47 */
#define K_0           '0'   /* 48 */
#define K_1           '1'   /* 49 */
#define K_2           '2'   /* 50 */
#define K_3           '3'   /* 51 */
#define K_4           '4'   /* 52 */
#define K_5           '5'   /* 53 */
#define K_6           '6'   /* 54 */
#define K_7           '7'   /* 55 */
#define K_8           '8'   /* 56 */
#define K_9           '9'   /* 57 */
#define K_colon       ':'   /* 58 */
#define K_semicolon   ';'   /* 59 */
#define K_less        '<'   /* 60 */
#define K_equal       '='   /* 61 */
#define K_greater     '>'   /* 62 */
#define K_question    '?'   /* 63 */
#define K_at          '@'   /* 64 */
#define K_A           'A'   /* 65 */
#define K_B           'B'   /* 66 */
#define K_C           'C'   /* 67 */
#define K_D           'D'   /* 68 */
#define K_E           'E'   /* 69 */
#define K_F           'F'   /* 70 */
#define K_G           'G'   /* 71 */
#define K_H           'H'   /* 72 */
#define K_I           'I'   /* 73 */
#define K_J           'J'   /* 74 */
#define K_K           'K'   /* 75 */
#define K_L           'L'   /* 76 */
#define K_M           'M'   /* 77 */
#define K_N           'N'   /* 78 */
#define K_O           'O'   /* 79 */
#define K_P           'P'   /* 80 */
#define K_Q           'Q'   /* 81 */
#define K_R           'R'   /* 82 */
#define K_S           'S'   /* 83 */
#define K_T           'T'   /* 84 */
#define K_U           'U'   /* 85 */
#define K_V           'V'   /* 86 */
#define K_W           'W'   /* 87 */
#define K_X           'X'   /* 88 */
#define K_Y           'Y'   /* 89 */
#define K_Z           'Z'   /* 90 */
#define K_bracketleft '['   /* 91 */
#define K_backslash   '\\'  /* 92 */
#define K_bracketright ']'  /* 93 */
#define K_circum      '^'   /* 94 */
#define K_underscore  '_'   /* 95 */
#define K_grave       '`'   /* 96 */
#define K_a           'a'   /* 97 */
#define K_b           'b'   /* 98 */
#define K_c           'c'   /* 99 */
#define K_d           'd'   /* 100 */
#define K_e           'e'   /* 101 */
#define K_f           'f'   /* 102 */
#define K_g           'g'   /* 103 */
#define K_h           'h'   /* 104 */
#define K_i           'i'   /* 105 */
#define K_j           'j'   /* 106 */
#define K_k           'k'   /* 107 */
#define K_l           'l'   /* 108 */
#define K_m           'm'   /* 109 */
#define K_n           'n'   /* 110 */
#define K_o           'o'   /* 111 */
#define K_p           'p'   /* 112 */
#define K_q           'q'   /* 113 */
#define K_r           'r'   /* 114 */
#define K_s           's'   /* 115 */
#define K_t           't'   /* 116 */
#define K_u           'u'   /* 117 */
#define K_v           'v'   /* 118 */
#define K_w           'w'   /* 119 */
#define K_x           'x'   /* 120 */
#define K_y           'y'   /* 121 */
#define K_z           'z'   /* 122 */
#define K_braceleft   '{'   /* 123 */
#define K_bar         '|'   /* 124 */
#define K_braceright  '}'   /* 125 */
#define K_tilde       '~'   /* 126 */

#define iup_isprint(_c) ((_c) > 31 && (_c) < 127)

/* also define the escape sequences that have keys associated */

#define K_BS     '\b'       /* 8 */
#define K_TAB    '\t'       /* 9 */
#define K_LF     '\n'       /* 10 (0x0A) not a real key, is a combination of CR with a modifier, just to document */
#define K_CR     '\r'       /* 13 (0x0D) */

/* backward compatible definitions */
                            
#define  K_quoteleft   K_grave
#define  K_quoteright  K_apostrophe

/* IUP Extended Codes, range start at 128 */
/* Modifiers use 128 interval             */
/* these definitions are specific to IUP  */

#define isxkey(_c)     ((_c) > 128)
#define IUPxCODE(_c)   ((_c) + 128)   /* Normal */
#define IUPsxCODE(_c)  ((_c) + 256)   /* Shift  */
#define IUPcxCODE(_c)  ((_c) + 384)   /* Ctrl   */
#define IUPmxCODE(_c)  ((_c) + 512)   /* Alt    */
#define IUPyxCODE(_c)  ((_c) + 640)   /* Sys (Win or Apple) */

#define IUP_NUMMAXCODES 768
                                         
#define K_HOME     IUPxCODE(1)                
#define K_UP       IUPxCODE(2)
#define K_PGUP     IUPxCODE(3)
#define K_LEFT     IUPxCODE(4)
#define K_MIDDLE   IUPxCODE(5)
#define K_RIGHT    IUPxCODE(6)
#define K_END      IUPxCODE(7)
#define K_DOWN     IUPxCODE(8)
#define K_PGDN     IUPxCODE(9)
#define K_INS      IUPxCODE(10)
#define K_DEL      IUPxCODE(11)
#define K_acute    IUPxCODE(12) /* no Shift/Ctrl/Alt */
/* unused          IUPxCODE(13) */
/* unused          IUPxCODE(14) */
/* unused          IUPxCODE(15) */
#define K_PAUSE    IUPxCODE(16)
#define K_ESC      IUPxCODE(17)
#define K_ccedilla IUPxCODE(18)
#define K_F1       IUPxCODE(19)
#define K_F2       IUPxCODE(20)
#define K_F3       IUPxCODE(21)
#define K_F4       IUPxCODE(22)
#define K_F5       IUPxCODE(23)
#define K_F6       IUPxCODE(24)
#define K_F7       IUPxCODE(25)
#define K_F8       IUPxCODE(26)
#define K_F9       IUPxCODE(27)
#define K_F10      IUPxCODE(28)
#define K_F11      IUPxCODE(29)
#define K_F12      IUPxCODE(30)
#define K_Print    IUPxCODE(31)
#define K_Menu     IUPxCODE(32)

#define K_sHOME    IUPsxCODE(1)
#define K_sUP      IUPsxCODE(2)
#define K_sPGUP    IUPsxCODE(3)
#define K_sLEFT    IUPsxCODE(4)
#define K_sMIDDLE  IUPsxCODE(5)
#define K_sRIGHT   IUPsxCODE(6)
#define K_sEND     IUPsxCODE(7)
#define K_sDOWN    IUPsxCODE(8)
#define K_sPGDN    IUPsxCODE(9)
#define K_sINS     IUPsxCODE(10)
#define K_sDEL     IUPsxCODE(11)
#define K_sSP      IUPsxCODE(12)
#define K_sTAB     IUPsxCODE(13)
#define K_sCR      IUPsxCODE(14)
#define K_sBS      IUPsxCODE(15)
#define K_sPAUSE   IUPsxCODE(16)
#define K_sESC     IUPsxCODE(17)
#define K_Ccedilla IUPsxCODE(18)
#define K_sF1      IUPsxCODE(19)
#define K_sF2      IUPsxCODE(20)
#define K_sF3      IUPsxCODE(21)
#define K_sF4      IUPsxCODE(22)
#define K_sF5      IUPsxCODE(23)
#define K_sF6      IUPsxCODE(24)
#define K_sF7      IUPsxCODE(25)
#define K_sF8      IUPsxCODE(26)
#define K_sF9      IUPsxCODE(27)
#define K_sF10     IUPsxCODE(28)
#define K_sF11     IUPsxCODE(29)
#define K_sF12     IUPsxCODE(30)
#define K_sPrint   IUPsxCODE(31)
#define K_sMenu    IUPsxCODE(32)

#define K_cHOME    IUPcxCODE(1)
#define K_cUP      IUPcxCODE(2)
#define K_cPGUP    IUPcxCODE(3)
#define K_cLEFT    IUPcxCODE(4)
#define K_cMIDDLE  IUPcxCODE(5)
#define K_cRIGHT   IUPcxCODE(6)
#define K_cEND     IUPcxCODE(7)
#define K_cDOWN    IUPcxCODE(8)
#define K_cPGDN    IUPcxCODE(9)
#define K_cINS     IUPcxCODE(10)
#define K_cDEL     IUPcxCODE(11)
#define K_cSP      IUPcxCODE(12)
#define K_cTAB     IUPcxCODE(13)
#define K_cCR      IUPcxCODE(14)
#define K_cBS      IUPcxCODE(15)
#define K_cPAUSE   IUPcxCODE(16)
#define K_cESC     IUPcxCODE(17)
#define K_cCcedilla IUPcxCODE(18)
#define K_cF1      IUPcxCODE(19)
#define K_cF2      IUPcxCODE(20)
#define K_cF3      IUPcxCODE(21)
#define K_cF4      IUPcxCODE(22)
#define K_cF5      IUPcxCODE(23)
#define K_cF6      IUPcxCODE(24)
#define K_cF7      IUPcxCODE(25)
#define K_cF8      IUPcxCODE(26)
#define K_cF9      IUPcxCODE(27)
#define K_cF10     IUPcxCODE(28)
#define K_cF11     IUPcxCODE(29)
#define K_cF12     IUPcxCODE(30)
#define K_cPrint   IUPcxCODE(31)
#define K_cMenu    IUPcxCODE(32)

#define K_mHOME    IUPmxCODE(1)
#define K_mUP      IUPmxCODE(2)
#define K_mPGUP    IUPmxCODE(3)
#define K_mLEFT    IUPmxCODE(4)
#define K_mMIDDLE  IUPmxCODE(5)
#define K_mRIGHT   IUPmxCODE(6)
#define K_mEND     IUPmxCODE(7)
#define K_mDOWN    IUPmxCODE(8)
#define K_mPGDN    IUPmxCODE(9)
#define K_mINS     IUPmxCODE(10)
#define K_mDEL     IUPmxCODE(11)
#define K_mSP      IUPmxCODE(12)
#define K_mTAB     IUPmxCODE(13)
#define K_mCR      IUPmxCODE(14)
#define K_mBS      IUPmxCODE(15)
#define K_mPAUSE   IUPmxCODE(16)
#define K_mESC     IUPmxCODE(17)
#define K_mCcedilla IUPmxCODE(18)
#define K_mF1      IUPmxCODE(19)
#define K_mF2      IUPmxCODE(20)
#define K_mF3      IUPmxCODE(21)
#define K_mF4      IUPmxCODE(22)
#define K_mF5      IUPmxCODE(23)
#define K_mF6      IUPmxCODE(24)
#define K_mF7      IUPmxCODE(25)
#define K_mF8      IUPmxCODE(26)
#define K_mF9      IUPmxCODE(27)
#define K_mF10     IUPmxCODE(28)
#define K_mF11     IUPmxCODE(29)
#define K_mF12     IUPmxCODE(30)
#define K_mPrint   IUPmxCODE(31)
#define K_mMenu    IUPmxCODE(32)

#define K_yHOME    IUPyxCODE(1)
#define K_yUP      IUPyxCODE(2)
#define K_yPGUP    IUPyxCODE(3)
#define K_yLEFT    IUPyxCODE(4)
#define K_yMIDDLE  IUPyxCODE(5)
#define K_yRIGHT   IUPyxCODE(6)
#define K_yEND     IUPyxCODE(7)
#define K_yDOWN    IUPyxCODE(8)
#define K_yPGDN    IUPyxCODE(9)
#define K_yINS     IUPyxCODE(10)
#define K_yDEL     IUPyxCODE(11)
#define K_ySP      IUPyxCODE(12)
#define K_yTAB     IUPyxCODE(13)
#define K_yCR      IUPyxCODE(14)
#define K_yBS      IUPyxCODE(15)
#define K_yPAUSE   IUPyxCODE(16)
#define K_yESC     IUPyxCODE(17)
#define K_yCcedilla IUPyxCODE(18)
#define K_yF1      IUPyxCODE(19)
#define K_yF2      IUPyxCODE(20)
#define K_yF3      IUPyxCODE(21)
#define K_yF4      IUPyxCODE(22)
#define K_yF5      IUPyxCODE(23)
#define K_yF6      IUPyxCODE(24)
#define K_yF7      IUPyxCODE(25)
#define K_yF8      IUPyxCODE(26)
#define K_yF9      IUPyxCODE(27)
#define K_yF10     IUPyxCODE(28)
#define K_yF11     IUPyxCODE(29)
#define K_yF12     IUPyxCODE(30)
#define K_yPrint   IUPyxCODE(31)
#define K_yMenu    IUPyxCODE(32)

/* unused IUPxCODE  33-79 */
/* unused IUPsxCODE 33-68 */

#define K_sPlus         IUPsxCODE(69)   
#define K_sComma        IUPsxCODE(70)   
#define K_sMinus        IUPsxCODE(71)   
#define K_sPeriod       IUPsxCODE(72)   
#define K_sSlash        IUPsxCODE(73)   
#define K_sAsterisk     IUPsxCODE(79)
                        
#define K_cA     IUPcxCODE(33)
#define K_cB     IUPcxCODE(34)
#define K_cC     IUPcxCODE(35)
#define K_cD     IUPcxCODE(36)
#define K_cE     IUPcxCODE(37)
#define K_cF     IUPcxCODE(38)
#define K_cG     IUPcxCODE(39)
#define K_cH     IUPcxCODE(40)
#define K_cI     IUPcxCODE(41)
#define K_cJ     IUPcxCODE(42)
#define K_cK     IUPcxCODE(43)
#define K_cL     IUPcxCODE(44)
#define K_cM     IUPcxCODE(45)
#define K_cN     IUPcxCODE(46)
#define K_cO     IUPcxCODE(47)
#define K_cP     IUPcxCODE(48)
#define K_cQ     IUPcxCODE(49)
#define K_cR     IUPcxCODE(50)
#define K_cS     IUPcxCODE(51)
#define K_cT     IUPcxCODE(52)
#define K_cU     IUPcxCODE(53)
#define K_cV     IUPcxCODE(54)
#define K_cW     IUPcxCODE(55)
#define K_cX     IUPcxCODE(56)
#define K_cY     IUPcxCODE(57)
#define K_cZ     IUPcxCODE(58)
#define K_c1     IUPcxCODE(59)
#define K_c2     IUPcxCODE(60)
#define K_c3     IUPcxCODE(61)
#define K_c4     IUPcxCODE(62)
#define K_c5     IUPcxCODE(63)
#define K_c6     IUPcxCODE(64)
#define K_c7     IUPcxCODE(65)        
#define K_c8     IUPcxCODE(66)         
#define K_c9     IUPcxCODE(67)
#define K_c0     IUPcxCODE(68)
#define K_cPlus         IUPcxCODE(69)   
#define K_cComma        IUPcxCODE(70)   
#define K_cMinus        IUPcxCODE(71)   
#define K_cPeriod       IUPcxCODE(72)   
#define K_cSlash        IUPcxCODE(73)   
#define K_cSemicolon    IUPcxCODE(74) 
#define K_cEqual        IUPcxCODE(75)
#define K_cBracketleft  IUPcxCODE(76)
#define K_cBracketright IUPcxCODE(77)
#define K_cBackslash    IUPcxCODE(78)
#define K_cAsterisk     IUPcxCODE(79)

#define K_mA     IUPmxCODE(33)
#define K_mB     IUPmxCODE(34)
#define K_mC     IUPmxCODE(35)
#define K_mD     IUPmxCODE(36)
#define K_mE     IUPmxCODE(37)
#define K_mF     IUPmxCODE(38)
#define K_mG     IUPmxCODE(39)
#define K_mH     IUPmxCODE(40)
#define K_mI     IUPmxCODE(41)
#define K_mJ     IUPmxCODE(42)
#define K_mK     IUPmxCODE(43)
#define K_mL     IUPmxCODE(44)
#define K_mM     IUPmxCODE(45)
#define K_mN     IUPmxCODE(46)
#define K_mO     IUPmxCODE(47)
#define K_mP     IUPmxCODE(48)
#define K_mQ     IUPmxCODE(49)
#define K_mR     IUPmxCODE(50)
#define K_mS     IUPmxCODE(51)
#define K_mT     IUPmxCODE(52)
#define K_mU     IUPmxCODE(53)
#define K_mV     IUPmxCODE(54)
#define K_mW     IUPmxCODE(55)
#define K_mX     IUPmxCODE(56)
#define K_mY     IUPmxCODE(57)
#define K_mZ     IUPmxCODE(58)
#define K_m1     IUPmxCODE(59)
#define K_m2     IUPmxCODE(60)
#define K_m3     IUPmxCODE(61)
#define K_m4     IUPmxCODE(62)
#define K_m5     IUPmxCODE(63)
#define K_m6     IUPmxCODE(64)
#define K_m7     IUPmxCODE(65)        
#define K_m8     IUPmxCODE(66)         
#define K_m9     IUPmxCODE(67)
#define K_m0     IUPmxCODE(68)
#define K_mPlus         IUPmxCODE(69)   
#define K_mComma        IUPmxCODE(70)   
#define K_mMinus        IUPmxCODE(71)   
#define K_mPeriod       IUPmxCODE(72)   
#define K_mSlash        IUPmxCODE(73)   
#define K_mSemicolon    IUPmxCODE(74) 
#define K_mEqual        IUPmxCODE(75)
#define K_mBracketleft  IUPmxCODE(76)
#define K_mBracketright IUPmxCODE(77)
#define K_mBackslash    IUPmxCODE(78)
#define K_mAsterisk     IUPmxCODE(79)

#define K_yA     IUPyxCODE(33)
#define K_yB     IUPyxCODE(34)
#define K_yC     IUPyxCODE(35)
#define K_yD     IUPyxCODE(36)
#define K_yE     IUPyxCODE(37)
#define K_yF     IUPyxCODE(38)
#define K_yG     IUPyxCODE(39)
#define K_yH     IUPyxCODE(40)
#define K_yI     IUPyxCODE(41)
#define K_yJ     IUPyxCODE(42)
#define K_yK     IUPyxCODE(43)
#define K_yL     IUPyxCODE(44)
#define K_yM     IUPyxCODE(45)
#define K_yN     IUPyxCODE(46)
#define K_yO     IUPyxCODE(47)
#define K_yP     IUPyxCODE(48)
#define K_yQ     IUPyxCODE(49)
#define K_yR     IUPyxCODE(50)
#define K_yS     IUPyxCODE(51)
#define K_yT     IUPyxCODE(52)
#define K_yU     IUPyxCODE(53)
#define K_yV     IUPyxCODE(54)
#define K_yW     IUPyxCODE(55)
#define K_yX     IUPyxCODE(56)
#define K_yY     IUPyxCODE(57)
#define K_yZ     IUPyxCODE(58)
#define K_y1     IUPyxCODE(59)
#define K_y2     IUPyxCODE(60)
#define K_y3     IUPyxCODE(61)
#define K_y4     IUPyxCODE(62)
#define K_y5     IUPyxCODE(63)
#define K_y6     IUPyxCODE(64)
#define K_y7     IUPyxCODE(65)        
#define K_y8     IUPyxCODE(66)         
#define K_y9     IUPyxCODE(67)
#define K_y0     IUPyxCODE(68)
#define K_yPlus         IUPyxCODE(69)   
#define K_yComma        IUPyxCODE(70)   
#define K_yMinus        IUPyxCODE(71)   
#define K_yPeriod       IUPyxCODE(72)   
#define K_ySlash        IUPyxCODE(73)   
#define K_ySemicolon    IUPyxCODE(74) 
#define K_yEqual        IUPyxCODE(75)
#define K_yBracketleft  IUPyxCODE(76)
#define K_yBracketright IUPyxCODE(77)
#define K_yBackslash    IUPyxCODE(78)
#define K_yAsterisk     IUPyxCODE(79)

/* unused 80-128 */


#endif
