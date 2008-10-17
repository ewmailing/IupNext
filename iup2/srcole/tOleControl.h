// tOleControl.h: interface for the tOleControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOLECONTROL_H__653E3861_8C5D_11D3_A908_0004AC253E99__INCLUDED_)
#define AFX_TOLECONTROL_H__653E3861_8C5D_11D3_A908_0004AC253E99__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "tIupCtl.h"
#include "tOleHandler.h"

class tOleControl : public tIupCtl  
{
public:
	void setattr(const char * attribute, const char * value);
	void setcurrentsize(int w, int h);
	void setnaturalsize(void);
	char * getattr(const char *attr);
	bool created;
  bool initialized;
	void map(Ihandle *parent);
	static Iclass * iupclass;
  tOleHandler * olehandler;
	static Ihandle *CreateFromArray(const char *ProgID);
	static void Initialize(void);
	tOleControl(char *progID);
	virtual ~tOleControl();

  CLSID m_clsid;

};

#endif // !defined(AFX_TOLECONTROL_H__653E3861_8C5D_11D3_A908_0004AC253E99__INCLUDED_)
