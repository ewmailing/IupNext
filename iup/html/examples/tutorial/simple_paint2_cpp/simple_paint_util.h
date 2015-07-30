
/* avoid multiple inclusions */
#ifndef SIMPLE_PAINT_UTIL_H
#define SIMPLE_PAINT_UTIL_H

#include <iup.h>
#include <im.h>
#include <im_image.h>


char* str_duplicate(const char* str);
const char* str_filetitle(const char *filename);
const char* str_fileext(const char *filename);
int str_compare(const char *l, const char *r, int casesensitive);

void show_error(const char* message, int is_error);
void show_file_error(int error);

void scroll_update(Ihandle* ih, int view_width, int view_height);
void scroll_calc_center(Ihandle* ih, float *x, float *y);
void scroll_center(Ihandle* ih, float old_center_x, float old_center_y);
void scroll_move(Ihandle* ih, int canvas_width, int canvas_height, int move_x, int move_y, int view_width, int view_height);

void image_flood_fill(const imImage* image, int start_x, int start_y, long replace_color, double tol_percent);

void view_fit_rect(int canvas_width, int canvas_height, int image_width, int image_height, int *view_width, int *view_height);
void view_zoom_offset(int view_x, int view_y, int image_width, int image_height, double zoom_factor, int *x, int *y);

#endif
