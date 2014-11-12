/** \file
 * \brief Plot component for Iup.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_PLOT_H 
#define __IUP_PLOT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize IupPlot widget class */
void IupPlotOpen(void);

/* Create an IupPlot widget instance */
Ihandle* IupPlot(void);

/***********************************************/
/*           Additional API                    */
void IupPlotBegin(Ihandle *ih, int strXdata);
void IupPlotAdd(Ihandle *ih, double x, double y);
void IupPlotAddStr(Ihandle *ih, const char* x, double y);
int  IupPlotEnd(Ihandle *ih);

void IupPlotInsertStr(Ihandle *ih, int index, int sample_index, const char* x, double y);
void IupPlotInsert(Ihandle *ih, int index, int sample_index, double x, double y);

void IupPlotInsertStrPoints(Ihandle* ih, int index, int sample_index, const char** x, double* y, int count);
void IupPlotInsertPoints(Ihandle* ih, int index, int sample_index, double *x, double *y, int count);

void IupPlotAddPoints(Ihandle* ih, int index, double *x, double *y, int count);
void IupPlotAddStrPoints(Ihandle* ih, int index, const char** x, double* y, int count);

void IupPlotGetSample(Ihandle* ih, int index, int sample_index, double *x, double *y);
void IupPlotGetSampleStr(Ihandle* ih, int index, int sample_index, const char* *x, double *y);

void IupPlotTransform(Ihandle* ih, double x, double y, double *cnv_x, double *cnv_y);
void IupPlotTransformTo(Ihandle* ih, double cnv_x, int cnv_y, double *x, double *y);

typedef struct _cdCanvas cdCanvas;

void IupPlotPaintTo(Ihandle *ih, cdCanvas* cnv);
/***********************************************/


#ifdef __cplusplus
}
#endif

#endif
