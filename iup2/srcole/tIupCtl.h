// tIupCtl.h: interface for the tIupCtl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIUPCTL_H__653E3860_8C5D_11D3_A908_0004AC253E99__INCLUDED_)
#define AFX_TIUPCTL_H__653E3860_8C5D_11D3_A908_0004AC253E99__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "iup.h"
extern "C"
{
#include "iupcpi.h"
}

typedef Ihandle *(*tCreateFunction)(Iclass *, void **);

class tIupCtl  
{
public:
	virtual void setcurrentsize(int w, int h);
	virtual void setnaturalsize(void);
	virtual char * getattr(const char *attribute);
	virtual void map (Ihandle* parent);
	Ihandle * get_ihandle(void);
	static Iclass *RegisterIupClass(
    char *classname, 
    char *led_name, 
    char *args,
    tCreateFunction creation_function);
	virtual void setattr(const char *attribute, const char *value);
	static tIupCtl * GetObjFromIhandle(Ihandle *handle);
	tIupCtl();
	virtual ~tIupCtl();

protected:
	void initIhandle(void);
	Ihandle * handle;
};

#endif // !defined(AFX_TIUPCTL_H__653E3860_8C5D_11D3_A908_0004AC253E99__INCLUDED_)
