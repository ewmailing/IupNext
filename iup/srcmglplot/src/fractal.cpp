/***************************************************************************
 * pixel.cpp is part of Math Graphic Library
 * Copyright (C) 2007-2016 Alexey Balakin <mathgl.abalakin@gmail.ru>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "mgl2/other.h"
#include "mgl2/data.h"
MGL_NO_EXPORT char *mgl_read_gz(gzFile fp);
//-----------------------------------------------------------------------------
//
//	IFS series
//
//-----------------------------------------------------------------------------
void MGL_NO_EXPORT mgl_ifs_2d_point(HCDT A, mreal& x, mreal& y, mreal amax)
{
	long i, n=A->GetNy();
	mreal r = amax*mgl_rnd(), sum_prob = 0, x1;
	for(i=0;i<n;i++)
	{
		sum_prob += A->v(6,i);
		if(r<sum_prob)	break;
	}
	x1= A->v(0,i)*x + A->v(1,i)*y + A->v(4,i);
	y = A->v(2,i)*x + A->v(3,i)*y + A->v(5,i);	x = x1;
}
HMDT MGL_EXPORT mgl_data_ifs_2d(HCDT A, long n, long skip)
{
	if(!A || A->GetNx()<7 || n<1)	return 0;	// incompatible dimensions
	mreal amax=0;
	for(long i=0; i<A->GetNy(); i++)	amax += A->v(6,i);
	if(amax<=0)	return 0;
	
	mglData *f = new mglData(2,n);
	mreal x = 0, y = 0;
	for(long i=0; i<skip; i++)	mgl_ifs_2d_point(A, x, y,amax);
	for(long i=0; i<n; i++)
	{
		mgl_ifs_2d_point(A, x, y, amax);
		f->a[2*i] = x;	f->a[2*i+1] = y;
	}
	return f;
}
uintptr_t MGL_EXPORT mgl_data_ifs_2d_(uintptr_t *d, long *n, long *skip)
{	return uintptr_t(mgl_data_ifs_2d(_DT_,*n,*skip));	}
//-----------------------------------------------------------------------------
void MGL_NO_EXPORT mgl_ifs_3d_point(HCDT A, mreal& x, mreal& y, mreal& z, mreal amax)
{
	int i, n=A->GetNy();
	mreal r = amax*mgl_rnd(), sum_prob = 0, x1, y1;
	for (i=0; i<n; i++)
	{
		sum_prob += A->v(12,i);
		if(r < sum_prob)  break;
	}
	x1= A->v(0,i)*x + A->v(1,i)*y + A->v(2,i)*z + A->v(9,i);
	y1= A->v(3,i)*x + A->v(4,i)*y + A->v(5,i)*z + A->v(10,i);
	z = A->v(6,i)*x + A->v(7,i)*y + A->v(8,i)*z + A->v(11,i);
	x = x1;	y = y1;
}
HMDT MGL_EXPORT mgl_data_ifs_3d(HCDT A, long n, long skip)
{
	if(!A || A->GetNx()<13 || n<1)	return 0;   // incompatible dimensions
	mreal amax = 0;
	for(int i=0; i<A->GetNy(); i++)	amax += A->v(12,i);
	if(amax <= 0) return 0;
	
	mglData *f = new mglData(3,n);
	mreal x = 0, y = 0, z = 0;
	for(long i=0; i<skip; i++)	mgl_ifs_3d_point(A, x, y, z, amax);
	for(long i=0; i<n; i++)
	{
		mgl_ifs_3d_point(A, x, y, z, amax);
		f->a[3*i] = x;	f->a[3*i+1] = y;	f->a[3*i+2] = z;
	}
	return f;
}
uintptr_t MGL_EXPORT mgl_data_ifs_3d_(uintptr_t *d, long *n, long *skip)
{   return uintptr_t(mgl_data_ifs_3d(_DT_,*n,*skip));   }
//-----------------------------------------------------------------------------
HMDT MGL_EXPORT mgl_data_ifs_file(const char *fname, const char *name, long n, long skip)
{
	gzFile fp = gzopen(fname,"r");
	if(!fp)	return 0;		// Couldn't open file file
	char *buf = mgl_read_gz(fp);	gzclose(fp);
	char *s = (char*)strstr(buf,name);
	if(!s)	return 0;		// No data for fractal 'name' in the file

	char *p = (char*)strchr(s,'{'), *e;
	if(!p)	return 0;		// Wrong data format for fractal 'name' in the file
	bool ext3d = false;
	e = (char*)strstr(s,"(3D)");	if(e && e<p)	ext3d = true;
	e = (char*)strstr(s,"(3d)");	if(e && e<p)	ext3d = true;
	e = (char*)strchr(p,'}');
	
	std::vector<mreal> nums;
	for(size_t i=0;p[i] && p+i<e;i++)
	{
		while(p[i]<=' ')	i++;
		if(p[i]==';' || p[i]=='#')	while(p[i] && p[i]!='\n')	i++;
		if(strchr("0123456789.+-",p[i]))	// this is number
		{
			nums.push_back(atof(p+i));
			while(p[i]>' ')	i++;
		}
	}
	HMDT dat = new mglData, res;
	if(ext3d)
	{
		dat->Set(&(nums[0]), 13, nums.size()/13, 1);
		res = mgl_data_ifs_3d(dat, n, skip);
	}
	else
	{
		dat->Set(&(nums[0]), 7, nums.size()/7, 1);
		res = mgl_data_ifs_2d(dat, n, skip);
	}
	delete dat;	free(buf);	return res;
}
uintptr_t mgl_data_ifs_file_(const char *fname, const char *name, long *n, long *skip,int l,int m)
{	char *s=new char[l+1];		memcpy(s,fname,l);	s[l]=0;
	char *t=new char[m+1];		memcpy(t,name,m);	t[m]=0;
	uintptr_t r = uintptr_t(mgl_data_ifs_file(s,t,*n,*skip));
	delete []s;	delete []t;		return r;	}
//-----------------------------------------------------------------------------
