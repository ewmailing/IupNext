/** \file
/** \file
* \brief Tree Control
*
* See Copyright Notice in "iup.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_assert.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_drvinfo.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_image.h"
#include "iup_array.h"
#include "iup_drvdraw.h"
#include "iup_draw.h"
#include "iup_register.h"
#include "iup_flatscrollbar.h"
#include "iup_childtree.h"

/* Kinds of node */
#define IFLATTREE_BRANCH        0
#define IFLATTREE_LEAF          1

#define IFLATTREE_UP        0
#define IFLATTREE_DOWN      1

#define IFLATTREE_MARK_SINGLE 0
#define IFLATTREE_MARK_MULTIPLE 1

#define IFLATTREE_TOGGLE_MARGIN 2
#define IFLATTREE_TOGGLE_BORDER 1
#define IFLATTREE_TOGGLE_SPACE  2

/* Orientation */
enum { IFLATTREE_EXPANDED, IFLATTREE_COLLAPSED };

typedef struct _iFlatTreeNode {
	char* title;
	char* image;
	char* image_expanded;
	char *fgColor;
	char *bgColor;
	char *font;
	int selected;
	int kind;
	int expand;
	int toggle_visible;
	int toggle_value;
	void* userdata;

	int posy, height, width;

	struct _iFlatTreeNode *parent;
	struct _iFlatTreeNode *firstChild;
	struct _iFlatTreeNode *brother;
} iFlatTreeNode;

struct _IcontrolData
{
	iupCanvas canvas;  /* from IupCanvas (must reserve it) */

	iFlatTreeNode *node;

	/* aux */
	int dragover_pos, dragged_pos;
	int has_focus, focus_id;
	int show_rename;

	/* attributes */
	int add_expanded;
	int indentation;
	int expand_all;
	int horiz_padding, vert_padding;  /* button margin */
	int spacing, icon_spacing, img_position;        /* used when both text and image are displayed */
	int horiz_alignment, vert_alignment;
	int border_width;
	int mark_mode, mark_start;
	int show_dragdrop;
	int lastAddNode;
	int show_toggle;
	int toggle;
	char* image_leaf;
	char* image_branch_expanded;
	char* image_branch_collapsed;
	int last_clock, min_clock;
	int level_gap;
	int toggle_size;
	int image_plusminus_height;

	Iarray *node_cache;
};

static iFlatTreeNode *iFlatTreeGetNode(Ihandle *ih, int id);
static Ihandle* load_image_plus(void)
{
	unsigned char imgdata[] = {
	  186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188,
	  145, 145, 145, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 145, 145, 145,
	  145, 145, 145, 252, 252, 252, 252, 252, 252, 252, 252, 252, 41, 66, 114, 252, 252, 252, 252, 252, 252, 252, 252, 252, 145, 145, 145,
	  145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 41, 66, 114, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
	  145, 145, 145, 250, 251, 251, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 250, 251, 251, 145, 145, 145,
	  145, 145, 145, 237, 237, 236, 237, 237, 236, 237, 237, 236, 41, 66, 114, 237, 237, 236, 237, 237, 236, 237, 237, 236, 145, 145, 145,
	  145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 41, 66, 114, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
	  145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
	  186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188 };

	Ihandle* image = IupImageRGB(9, 9, imgdata);
	return image;
}

static Ihandle* load_image_minus(void)
{
	unsigned char imgdata[] = {
	  186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188,
	  145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
	  145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
	  145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
	  145, 145, 145, 237, 237, 236, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 237, 237, 236, 145, 145, 145,
	  145, 145, 145, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 145, 145, 145,
	  145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
	  145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
	  186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188 };

	Ihandle* image = IupImageRGB(9, 9, imgdata);
	return image;
}

static void load_all_images_plusminus(void)
{
	IupSetHandle("IMGPLUS", load_image_plus());
	IupSetHandle("IMGMINUS", load_image_minus());
}
static void iFlatTreeInitializeImages(void)
{
	Ihandle *image_leaf, *image_blank, *image_paper;
	Ihandle *image_collapsed, *image_expanded, *image_empty;

#define IFLATTREE_IMG_WIDTH   16
#define IFLATTREE_IMG_HEIGHT  16

	unsigned char img_leaf[IFLATTREE_IMG_WIDTH*IFLATTREE_IMG_HEIGHT] =
	{
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 4, 4, 5, 5, 5, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 4, 5, 5, 1, 6, 1, 5, 0, 0, 0, 0,
	  0, 0, 0, 0, 3, 4, 4, 5, 5, 1, 6, 1, 5, 0, 0, 0,
	  0, 0, 0, 0, 3, 4, 4, 4, 5, 5, 1, 1, 5, 0, 0, 0,
	  0, 0, 0, 0, 2, 3, 4, 4, 4, 5, 5, 5, 4, 0, 0, 0,
	  0, 0, 0, 0, 2, 3, 3, 4, 4, 4, 5, 4, 4, 0, 0, 0,
	  0, 0, 0, 0, 0, 2, 3, 3, 4, 4, 4, 4, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 2, 2, 3, 3, 3, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	unsigned char img_collapsed[IFLATTREE_IMG_WIDTH*IFLATTREE_IMG_HEIGHT] =
	{
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 2, 6, 5, 5, 7, 2, 3, 0, 0, 0, 0, 0, 0,
	  0, 0, 2, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 0,
	  0, 0, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 4, 3,
	  0, 0, 2, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 4, 3,
	  0, 0, 2, 5, 7, 7, 7, 7, 7, 7, 1, 7, 1, 7, 4, 3,
	  0, 0, 2, 5, 7, 7, 7, 7, 7, 7, 7, 1, 7, 1, 4, 3,
	  0, 0, 2, 5, 7, 7, 7, 7, 1, 7, 1, 7, 1, 7, 4, 3,
	  0, 0, 2, 5, 7, 7, 7, 7, 7, 1, 7, 1, 7, 1, 4, 3,
	  0, 0, 2, 5, 7, 7, 7, 7, 1, 7, 1, 7, 1, 1, 4, 3,
	  0, 0, 2, 5, 1, 7, 1, 1, 7, 1, 7, 1, 1, 1, 4, 3,
	  0, 0, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
	  0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	unsigned char img_expanded[IFLATTREE_IMG_WIDTH*IFLATTREE_IMG_HEIGHT] =
	{
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 2, 1, 3, 3, 3, 3, 1, 2, 2, 2, 2, 2, 2, 0,
	  0, 0, 2, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 6, 4,
	  0, 0, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 6, 4,
	  0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 3, 6, 4,
	  0, 2, 1, 3, 3, 3, 3, 3, 5, 3, 5, 6, 4, 6, 6, 4,
	  0, 2, 1, 3, 3, 3, 3, 3, 3, 5, 3, 6, 4, 6, 6, 4,
	  0, 0, 2, 0, 3, 3, 3, 3, 5, 3, 5, 5, 2, 4, 2, 4,
	  0, 0, 2, 0, 3, 3, 5, 5, 3, 5, 5, 5, 6, 4, 2, 4,
	  0, 0, 0, 2, 0, 5, 3, 3, 5, 5, 5, 5, 6, 2, 4, 4,
	  0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4,
	  0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	unsigned char img_blank[IFLATTREE_IMG_WIDTH*IFLATTREE_IMG_HEIGHT] =
	{
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 0, 0, 0, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 5, 4, 0, 0, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 4, 0, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0,
	  0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
	};

	unsigned char img_paper[IFLATTREE_IMG_WIDTH*IFLATTREE_IMG_HEIGHT] =
	{
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 0, 0, 0, 0,
	  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 5, 4, 0, 0, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 4, 0, 0,
	  0, 0, 3, 1, 4, 3, 4, 3, 4, 3, 4, 2, 2, 2, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 3, 4, 3, 4, 3, 4, 3, 4, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 4, 3, 4, 3, 4, 3, 4, 3, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 3, 4, 3, 4, 3, 4, 3, 4, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 1, 4, 3, 4, 3, 4, 3, 4, 3, 1, 5, 2, 0,
	  0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
	  0, 0, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0,
	  0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
	};

	image_leaf = IupImage(IFLATTREE_IMG_WIDTH, IFLATTREE_IMG_HEIGHT, img_leaf);
	image_collapsed = IupImage(IFLATTREE_IMG_WIDTH, IFLATTREE_IMG_HEIGHT, img_collapsed);
	image_expanded = IupImage(IFLATTREE_IMG_WIDTH, IFLATTREE_IMG_HEIGHT, img_expanded);
	image_blank = IupImage(IFLATTREE_IMG_WIDTH, IFLATTREE_IMG_HEIGHT, img_blank);
	image_paper = IupImage(IFLATTREE_IMG_WIDTH, IFLATTREE_IMG_HEIGHT, img_paper);
	image_empty = IupImage(IFLATTREE_IMG_WIDTH, IFLATTREE_IMG_HEIGHT, NULL);

	IupSetAttribute(image_leaf, "0", "BGCOLOR");
	IupSetAttribute(image_leaf, "1", "192 192 192");
	IupSetAttribute(image_leaf, "2", "56 56 56");
	IupSetAttribute(image_leaf, "3", "99 99 99");
	IupSetAttribute(image_leaf, "4", "128 128 128");
	IupSetAttribute(image_leaf, "5", "161 161 161");
	IupSetAttribute(image_leaf, "6", "222 222 222");

	IupSetAttribute(image_collapsed, "0", "BGCOLOR");
	IupSetAttribute(image_collapsed, "1", "255 206 156");
	IupSetAttribute(image_collapsed, "2", "156 156 0");
	IupSetAttribute(image_collapsed, "3", "0 0 0");
	IupSetAttribute(image_collapsed, "4", "206 206 99");
	IupSetAttribute(image_collapsed, "5", "255 255 206");
	IupSetAttribute(image_collapsed, "6", "247 247 247");
	IupSetAttribute(image_collapsed, "7", "255 255 156");

	IupSetAttribute(image_expanded, "0", "BGCOLOR");
	IupSetAttribute(image_expanded, "1", "255 255 255");
	IupSetAttribute(image_expanded, "2", "156 156 0");
	IupSetAttribute(image_expanded, "3", "255 255 156");
	IupSetAttribute(image_expanded, "4", "0 0 0");
	IupSetAttribute(image_expanded, "5", "255 206 156");
	IupSetAttribute(image_expanded, "6", "206 206 99");

	IupSetAttribute(image_blank, "0", "BGCOLOR");
	IupSetAttribute(image_blank, "1", "255 255 255");
	IupSetAttribute(image_blank, "2", "000 000 000");
	IupSetAttribute(image_blank, "3", "119 119 119");
	IupSetAttribute(image_blank, "4", "136 136 136");
	IupSetAttribute(image_blank, "5", "187 187 187");

	IupSetAttribute(image_paper, "0", "BGCOLOR");
	IupSetAttribute(image_paper, "1", "255 255 255");
	IupSetAttribute(image_paper, "2", "000 000 000");
	IupSetAttribute(image_paper, "3", "119 119 119");
	IupSetAttribute(image_paper, "4", "136 136 136");
	IupSetAttribute(image_paper, "5", "187 187 187");

	IupSetAttribute(image_empty, "0", "BGCOLOR");
	IupSetAttribute(image_empty, "FLAT_ALPHA", "Yes"); /* necessary for Windows */

	IupSetHandle("IMGLEAF", image_leaf);
	IupSetHandle("IMGCOLLAPSED", image_collapsed);
	IupSetHandle("IMGEXPANDED", image_expanded);
	IupSetHandle("IMGBLANK", image_blank);
	IupSetHandle("IMGPAPER", image_paper);
	IupSetHandle("IMGEMPTY", image_empty);

#undef IFLATTREE_IMG_WIDTH
#undef IFLATTREE_IMG_HEIGHT
}

static void iFlatTreeSetNodeFont(Ihandle* ih, const char* font)
{
	if (font)
		iupAttribSetStr(ih, "DRAWFONT", font);
	else
	{
		font = IupGetAttribute(ih, "FONT");
		iupAttribSetStr(ih, "DRAWFONT", font);
	}
}

static char *iFlatTreeGetNodeImage(iFlatTreeNode *node)
{
	char* image;

	if (node->kind == IFLATTREE_LEAF)
	{
		if (node->image)
			image = node->image;
		else
			image = "IMGLEAF";
	}
	else
	{
		if (node->expand == IFLATTREE_COLLAPSED)
		{
			if (node->image)
				image = node->image;
			else
				image = "IMGCOLLAPSED";
		}
		else
			image = "IMGEXPANDED";
	}

	return image;
}

static int iFlatTreeRenameNode(Ihandle* ih)
{
	if (ih->data->show_rename && ih->data->has_focus)
	{
		iFlatTreeNode *hItemFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
		int pos, img_w, img_h, txt_x, txt_y, width, height;
		Ihandle* text = NULL;
		Ihandle* firstChild = ih->firstchild;
		char title[1024];
		char *image = iFlatTreeGetNodeImage(hItemFocus);

		if (!hItemFocus->title || *(hItemFocus->title) == 0)
			strcpy(title, "XXXXX");
		else
		{
			strcpy(title, hItemFocus->title);
			strcat(title, "X");

		}

		while (firstChild)
		{
			if (iupStrEqual(firstChild->iclass->name, "text"))
			{
				text = firstChild;
				break;
			}
			firstChild = firstChild->brother;
		}

		if (!text)
			return 1;

		iFlatTreeGetTitlePos(ih, ih->data->focus_id, image, &img_w, &img_h, &txt_x, &txt_y);
		iFlatTreeSetNodeFont(ih, hItemFocus->font);
		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			image, title, &width, &height, 0);

		if (image && ih->data->img_position == 0)
			txt_x += ih->data->icon_spacing + ih->data->horiz_padding;

		text->x = txt_x;
		text->y = txt_y;

		if (ih->data->show_toggle && hItemFocus->toggle_visible)
			text->x += ih->data->toggle_size;

		text->currentwidth = width - img_w + 1;
		text->currentheight = height;

		iupClassObjectLayoutUpdate(text);

		IupSetAttribute(text, "ALIGMENT", "ALEFT");
		IupSetStrAttribute(text, "PADDING", iupAttribGetStr(ih, "PADDING"));
		IupSetAttribute(text, "FONT", hItemFocus->font);
		IupSetAttribute(text, "VISIBLE", "YES");
		IupSetAttribute(text, "ACTIVE", "YES");
		IupSetAttribute(text, "VALUE", hItemFocus->title);
		IupSetFocus(text);

		pos = IupConvertXYToPos(text, txt_x, txt_y);
		IupSetInt(text, "CARETPOS", pos);
	}
	return 0;
}

static void iFllatTreeUpdateText(Ihandle *text, int x, int y, int w, int h)
{
	if (text->x == x && text->y == y && text->currentwidth > w && text->currentheight > h)
		return;

	text->x = x;
	text->y = y;

	text->currentwidth = w;
	text->currentheight = h;

	iupClassObjectLayoutUpdate(text);
}

static int iFlatTreeGetNodeLevel(Ihandle *ih, iFlatTreeNode *node)
{
	int level = -1;
	iFlatTreeNode *parent = node;

	while (parent)
	{
		level++;
		parent = parent->parent;
		if (!IupGetInt(ih, "ADDROOT") && parent == ih->data->node)
			break;
	}

	return level;
}

static iFlatTreeGetTitlePos(Ihandle *ih, int id, char *image, int *img_w, int *img_h, int *txt_x, int *txt_y)
{
	int pos = iFlatTreeConvertIdToPos(ih, id);
	int total_h = iFlatTreeConvertIdToY(ih, id, NULL);
	int posx = IupGetInt(ih, "POSX");
	int posy = IupGetInt(ih, "POSY");
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int level = iFlatTreeGetNodeLevel(ih, node);
	int border_width = ih->data->border_width;

	*img_h = 0;
	*img_w = 0;

	if (image)
		iupImageGetInfo(image, img_w, img_h, NULL);

	*txt_x = -posx + border_width;
	*txt_y = -posy + border_width;

	*txt_x += level * ih->data->level_gap + *img_h;
	*txt_y += total_h;
}

static int iFlatTreeConvertIdToPos(Ihandle *ih, int id)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i, pos = 0;

	if (id > count)
		return -1;

	for (i = 0; i < count; i++)
	{
		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;
		if (iFlatTreeFindNodeId(ih->data->node_cache, nodes[i]) == id)
			break;
		pos++;
	}

	return pos;
}

static int iFlatTreeConvertPosToId(Ihandle *ih, int pos)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i, cont = 0;

	if (pos > count)
		return -1;

	for (i = 0; i < count; i++)
	{
		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;
		if (cont == pos)
			break;
		cont++;
	}

	return i;
}

static int iFlatTreeFocusPageUp(Ihandle *ih)
{
	int item_height;
	int new_focus_id = ih->data->focus_id;
	int total_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, &item_height);

	int posy = IupGetInt(ih, "POSY");
	int dy = IupGetInt(ih, "DY");

	if ((total_y + item_height) > posy && (total_y + item_height) < (posy + dy))
		new_focus_id = iFlatTreePageLastItemId(ih, posy);

	if (new_focus_id == ih->data->focus_id)
		new_focus_id = iFlatTreePageUpFromFocus(ih);

	int endPagePos = total_y + posy - dy;

	ih->data->focus_id = new_focus_id;

	return new_focus_id;
}

static int iFlatTreeFocusPageDown(Ihandle *ih)
{
	int item_height;
	int new_focus_id = ih->data->focus_id;
	int total_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, &item_height);

	int posy = IupGetInt(ih, "POSY");
	int dy = IupGetInt(ih, "DY");

	if ((total_y + item_height) > posy && (total_y + item_height) < (posy + dy))
		new_focus_id = iFlatTreePageLastItemId(ih, posy);

	if (new_focus_id == ih->data->focus_id)
		new_focus_id = iFlatTreePageDownFromFocus(ih);

	int endPagePos = total_y + posy - dy;

	ih->data->focus_id = new_focus_id;

	return new_focus_id;
}

static void iFlatTreeInvertSelection(Ihandle* ih)
{
	int i;
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	for (i = 0; i < iupArrayCount(ih->data->node_cache); i++)
		nodes[i]->selected = !(nodes[i]->selected);
}

static void iFlatTreeSelectAll(Ihandle* ih)
{
	int i;
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	for (i = 0; i < iupArrayCount(ih->data->node_cache); i++)
		nodes[i]->selected = 1;
}

static void iFlatTreeClearAllSelectionExcept(Ihandle* ih, iFlatTreeNode *hItemExcept)
{
	int i;
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	for (i = 0; i < iupArrayCount(ih->data->node_cache); i++)
	{
		if (nodes[i] != hItemExcept)
			nodes[i]->selected = 0;
	}
}

static int iFlatTreeFindNodeId(Iarray *node_cache, iFlatTreeNode* node)
{
	iFlatTreeNode **nodes = iupArrayGetData(node_cache);
	int count = iupArrayCount(node_cache);
	int i;

	for (i = 0; i < count; i++)
	{
		if (nodes[i] == node)
			return i;
	}
	return -1;
}

static void iFlatTreeSelectRange(Ihandle* ih, iFlatTreeNode *hItem1, iFlatTreeNode *hItem2, int clear)
{
	int i;
	int id1 = iFlatTreeFindNodeId(ih->data->node_cache, hItem1);
	int id2 = iFlatTreeFindNodeId(ih->data->node_cache, hItem2);

	if (id1 > id2)
	{
		int tmp = id1;
		id1 = id2;
		id2 = tmp;
	}

	for (i = 0; i < iupArrayCount(ih->data->node_cache); i++)
	{
		iFlatTreeNode **data = iupArrayGetData(ih->data->node_cache);
		if (i < id1 || i > id2)
		{
			if (clear)
				data[i]->selected = 0;
		}
		else
			data[i]->selected = 1;
	}
}

static int iFlatTreeUpdateNodeCache(Ihandle *ih, iFlatTreeNode *node, int *next_posy, int level)
{
	iFlatTreeNode *brother = node;

	while (brother)
	{
		iFlatTreeNode **data = iupArrayInc(ih->data->node_cache);
		char *image = iFlatTreeGetNodeImage(node);
		int w, h, img_w, img_h;

		int count = iupArrayCount(ih->data->node_cache);
		data[count - 1] = brother;

		iFlatTreeSetNodeFont(ih, brother->font);
		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			image, brother->title, &w, &h, 0);

		brother->height = h;
		brother->width = w + ((level + 1) * ih->data->level_gap);

		if (iFlatTreeNodeIsVisible(ih, brother))
		{
			brother->posy = *next_posy;
			*next_posy += (h + ih->data->spacing);
		}
		else
			brother->posy = -1;


		if (brother->kind == IFLATTREE_BRANCH && brother->firstChild)
			iFlatTreeUpdateNodeCache(ih, brother->firstChild, next_posy, level + 1);

		brother = brother->brother;
	}
}

static void iFlatTreeRebuildCache(Ihandle *ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	int next_posy = 0;

	iupArrayRemove(ih->data->node_cache, 0, count);

	iFlatTreeUpdateNodeCache(ih, ih->data->node, &next_posy, 0);
}

static iFlatTreeNode *iFlatTreeGetNode(Ihandle *ih, int id)
{
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int count = iupArrayCount(ih->data->node_cache);

	if (id >= 0 && id < count)
		return nodes[id];
	else if (id == IUP_INVALID_ID && count != 0)
		return nodes[ih->data->focus_id];
	else
		return NULL;
}

static int iFlatTreeNodeIsVisible(Ihandle *ih, iFlatTreeNode *node)
{
	iFlatTreeNode *parent = node->parent;

	if (!parent)
		return 1;

	while (parent)
	{
		if (parent->expand == IFLATTREE_COLLAPSED)
			return 0;
		parent = parent->parent;
	}

	return 1;
}

static int iFlatTreeGetNextVisibleNodeId(Ihandle *ih, int id)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i, lastVisibleNode = id;

	for (i = id + 1; i < count; i++)
	{
		iFlatTreeNode *node = nodes[i];
		if (iFlatTreeNodeIsVisible(ih, node))
		{
			lastVisibleNode = i;
			break;
		}
	}

	return lastVisibleNode;
}

static int iFlatTreeGetPreviousVisibleNodeId(Ihandle *ih, int id)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i, lastVisibleNode = id;

	for (i = id - 1; i >= 0; i--)
	{
		iFlatTreeNode *node = nodes[i];
		if (iFlatTreeNodeIsVisible(ih, node))
		{
			lastVisibleNode = i;
			break;
		}
	}

	return lastVisibleNode;
}

static int iFlatTreeGetNodeDepth(Ihandle *ih, iFlatTreeNode *node)
{
	int depth = 0;
	iFlatTreeNode *parent = node->parent;

	if (iupAttribGetInt(ih, "ADDROOT"))
		depth++;

	while (parent)
	{
		depth++;
		parent = parent->parent;
	}

	return depth;
}

static void iFlatTreeGetMaxNodeDepth(iFlatTreeNode *node, int *depth);

static int iFlatTreeGetMaxDepth(Ihandle *ih)
{
	int maxDepth = -1;

	if (iupAttribGetInt(ih, "ADDROOT"))
		maxDepth++;

	iFlatTreeGetMaxNodeDepth(ih->data->node, &maxDepth);

	return maxDepth;
}

static void iFlatTreeGetMaxNodeDepth(iFlatTreeNode *node, int *depth)
{
	iFlatTreeNode *brother = node->brother;
	while (brother)
	{
		if (brother->kind = IFLATTREE_BRANCH && brother->firstChild)
		{
			*depth++;
			iFlatTreeGetMaxDepth(brother->firstChild, depth);
		}
		brother = brother->brother;
	}
}

static int iFlatTreeGetVisibleNodes(iFlatTreeNode *node)
{
	int count = 0;
	iFlatTreeNode *brother;

	brother = node;

	while (brother)
	{
		count++;
		if (brother->kind == IFLATTREE_BRANCH && brother->firstChild && brother->expand == IFLATTREE_EXPANDED)
			count += iFlatTreeGetVisibleNodes(brother->firstChild);
		brother = brother->brother;
	}

	return count;
}

static int iFlatTreeGetVisibleNodesCount(Ihandle *ih)
{
	int count;
	iFlatTreeNode *node = ih->data->node;

	if (!iupAttribGetInt(ih, "ADDROOT"))
		node = node->brother;

	count = iFlatTreeGetVisibleNodes(node);

	return count;
}

static int iFlatTreeGetTotalChildCount(iFlatTreeNode *node)
{
	int count = 0;
	iFlatTreeNode *child = node->firstChild;

	while (child)
	{
		if (child->firstChild)
			count += iFlatTreeGetTotalChildCount(child->firstChild);
		count++;
		child = child->brother;
	}

	return count;
}

static iFlatTreeNode *iFlatTreeGetNodeFromString(Ihandle* ih, const char* name_id)
{
	int id = IUP_INVALID_ID;
	iupStrToInt(name_id, &id);
	return iFlatTreeGetNode(ih, id);
}

static iFlatTreeNode *iFlatTreeCloneNode(iFlatTreeNode *node, int copyUserdata)
{
	iFlatTreeNode *newNode = (iFlatTreeNode*)malloc(sizeof(iFlatTreeNode));

	memset(newNode, 0, sizeof(iFlatTreeNode));

	newNode->title = strdup(node->title);
	newNode->image = strdup(node->image);
	newNode->image_expanded = strdup(node->image_expanded);
	newNode->bgColor = strdup(node->bgColor);
	newNode->fgColor = strdup(node->fgColor);
	newNode->font = strdup(node->font);
	newNode->kind = node->kind;
	newNode->expand = node->expand;
	if (copyUserdata)
		newNode->userdata = node->userdata;
	newNode->selected = 0;

	if (node->firstChild)
	{
		iFlatTreeNode *brother = node->firstChild;
		iFlatTreeNode *lastNode = NULL;

		while (brother)
		{
			iFlatTreeNode *newNewNode = iFlatTreeCloneNode(brother, copyUserdata);
			if (!lastNode)
				newNode->firstChild = newNewNode;
			else
				lastNode->brother = newNewNode;
			newNewNode->parent = newNode;
			lastNode = newNewNode;
			brother = brother->brother;
		}
	}

	return newNode;
}

static iFlatTreeNode *iFlatTreeCopyNode(Ihandle *ih, int srcId, int dstId)
{
	iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
	iFlatTreeNode *newNode = iFlatTreeCloneNode(srcNode, 0);
	iFlatTreeNode *dstNode = iFlatTreeGetNode(ih, dstId);

	if (!dstNode)
		return NULL;

	if (dstNode->kind == IFLATTREE_BRANCH && dstNode->expand == IFLATTREE_EXPANDED)
	{
		/* copy as first child of expanded branch */
		newNode->parent = dstNode;
		newNode->brother = dstNode->firstChild;
		dstNode->firstChild = newNode;
	}
	else
	{
		newNode->parent = dstNode->parent;
		newNode->brother = dstNode->brother;
		dstNode->brother = newNode;
	}

	iFlatTreeRebuildCache(ih);

	return newNode;
}

static int iFlatTreeUnlinkNode(Ihandle *ih, iFlatTreeNode* node, int onlyChildren)
{
	iFlatTreeNode *children = node->firstChild;
	iFlatTreeNode *parent = node->parent;

	if (node->kind == IFLATTREE_LEAF || (node->kind == IFLATTREE_BRANCH && !onlyChildren))
	{
		if (node->parent && node == node->parent->firstChild)
			parent->firstChild = node->brother;
		else
		{
			iFlatTreeNode *brother = NULL;
			if (node->parent)
				brother = node->parent->firstChild;
			else
				brother = ih->data->node;
			while (brother->brother && brother->brother != node)
				brother = brother->brother;
			brother->brother = node->brother;
		}
	}
	else
		node->firstChild = NULL;
}

static void iFlatTreeDelNode(iFlatTreeNode *node, int onlyChildren)
{
	iFlatTreeNode *brother = node->firstChild;

	while (brother)
	{
		iFlatTreeNode *nextNode = brother->brother;
		iFlatTreeDelNode(brother, 0);
		brother = nextNode;
	}

	if (onlyChildren)
		return;

	if (node->title)
		free(node->title);

	if (node->image)
		free(node->image);

	if (node->fgColor)
		free(node->fgColor);

	if (node->bgColor)
		free(node->bgColor);

	if (node->font)
		free(node->font);

	free(node);

	node = NULL;
}

static int iFlatTreeRemoveNode(Ihandle *ih, iFlatTreeNode* node, int onlyChildren)
{
	iFlatTreeUnlinkNode(ih, node, onlyChildren);

	iFlatTreeRebuildCache(ih);

	if (node == ih->data->node)
		ih->data->node = NULL;

	iFlatTreeDelNode(node, onlyChildren);
}

static iFlatTreeNode *iFlatTreeMoveNode(Ihandle *ih, int srcId, int dstId)
{
	iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
	iFlatTreeNode *dstNode = iFlatTreeGetNode(ih, dstId);

	if (!dstNode)
		return NULL;

	iFlatTreeUnlinkNode(ih, srcNode, 0);

	if (dstNode->kind == IFLATTREE_BRANCH && dstNode->expand == IFLATTREE_EXPANDED)
	{
		/* copy as first child of expanded branch */
		srcNode->parent = dstNode;
		srcNode->brother = dstNode->firstChild;
		dstNode->firstChild = srcNode;
	}
	else
	{
		srcNode->parent = dstNode->parent;
		srcNode->brother = dstNode->brother;
		dstNode->brother = srcNode;
	}

	return srcNode;
}

static void iFlatTreeAddNode(Ihandle* ih, int id, int kind, const char* title)
{
	iFlatTreeNode *hPrevItem = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	char str[1024];

	iFlatTreeNode *hItemNew;

	if (!hPrevItem && id != -1)
		return;

	if (!title || *title == 0)
		sprintf(str, "TITLE%d", id);
	else
		strcpy(str, title);

	if (id == -1)
		hPrevItem = iFlatTreeGetNode(ih, 0);

	hItemNew = (iFlatTreeNode *)malloc(sizeof(iFlatTreeNode));

	memset(hItemNew, 0, sizeof(iFlatTreeNode));

	hItemNew->title = strdup(str);
	hItemNew->kind = kind;
	hItemNew->toggle_visible = 1;

	if (hPrevItem->kind == IFLATTREE_LEAF)
	{
		iFlatTreeNode *tmp = hPrevItem->brother;
		hPrevItem->brother = hItemNew;
		hItemNew->brother = tmp;
		hItemNew->firstChild = NULL;
		hItemNew->parent = hPrevItem->parent;
		if (hItemNew->kind == IFLATTREE_BRANCH)
			hItemNew->expand = (iupAttribGetBoolean(ih, "ADDEXPANDED")) ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;
	}
	else
	{
		iFlatTreeNode *tmp = hPrevItem->firstChild;
		hItemNew->parent = hPrevItem;
		hItemNew->brother = tmp;
		hItemNew->firstChild = NULL;
		hPrevItem->firstChild = hItemNew;
		if (hItemNew->kind == IFLATTREE_BRANCH)
			hItemNew->expand = (iupAttribGetBoolean(ih, "ADDEXPANDED")) ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;
	}
}

static void iFlatTreeInsertNode(Ihandle* ih, int id, int kind, const char* title)
{
	iFlatTreeNode *hPrevItem = iFlatTreeGetNode(ih, id);
	iFlatTreeNode *hItemNew, *tmp, str[1024];

	if (!hPrevItem && id != -1)
		return;

	if (!hPrevItem)
		hPrevItem = iFlatTreeGetNode(ih, 0);

	tmp = hPrevItem->brother;

	if (!title || *title == 0)
		sprintf(str, "TITLE%d", id);
	else
		strcpy(str, title);

	hItemNew = (iFlatTreeNode *)malloc(sizeof(iFlatTreeNode));

	memset(hItemNew, 0, sizeof(iFlatTreeNode));

	hItemNew->title = strdup(str);
	hItemNew->kind = kind;
	hItemNew->toggle_visible = 1;

	hPrevItem->brother = hItemNew;
	hItemNew->brother = tmp;
	hItemNew->parent = hPrevItem->parent;
	if (hItemNew->kind == IFLATTREE_BRANCH)
		hItemNew->expand = (iupAttribGetBoolean(ih, "ADDEXPANDED")) ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;
}

static int iFlatTreeRemoveMarkedNodes(Ihandle *ih, iFlatTreeNode *curNode)
{
	while (curNode)
	{
		if (curNode->selected)
		{
			iFlatTreeNode *nextNode = curNode->brother;
			iFlatTreeRemoveNode(ih, curNode, 0);
			curNode = nextNode;
		}
		else if (curNode->kind == IFLATTREE_BRANCH)
		{
			iFlatTreeRemoveMarkedNodes(ih, curNode->firstChild);
			curNode = curNode->brother;
		}
		else
			curNode = curNode->brother;
	}
}

static void iFlatTreeMarkNodeBlock()
{

}

static int iFlatTreeGetScrollbar(Ihandle* ih)
{
	int flat = iupFlatScrollBarGet(ih);
	if (flat != IUP_SB_NONE)
		return flat;
	else
	{
		if (!ih->handle)
			ih->data->canvas.sb = iupBaseGetScrollbar(ih);

		return ih->data->canvas.sb;
	}
}

static int iFlatTreeGetScrollbarSize(Ihandle* ih)
{
	if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
	{
		if (iupAttribGetBoolean(ih, "SHOWFLOATING"))
			return 0;
		else
			return iupAttribGetInt(ih, "SCROLLBARSIZE");
	}
	else
		return iupdrvGetScrollbarSize();
}

static int iFlatTreeConvertXYToPos(Ihandle* ih, int x, int y)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int posy = IupGetInt(ih, "POSY");
	int pos = 0, py, dy = y + posy, i;

	if (dy < 0)
		return -1;

	for (i = 0; i < count; i++)
	{
		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		py = nodes[i]->posy;

		if (dy >= py && dy < (py + nodes[i]->height))
			return pos;

		py += nodes[i]->height;
		pos++;
	}

	(void)x;
	return -1;
}

static int iFlatTreeTotalHeight(Ihandle *ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i;
	int total_h = 0;

	for (i = 0; i < count; i++)
	{
		int w, h;

		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		total_h += nodes[i]->height + ih->data->spacing;
	}

	return total_h;
}

static int iFlatTreeMaxWidth(Ihandle *ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i;
	int max_w = 0;

	for (i = 0; i < count; i++)
	{
		int w, h;

		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		max_w = (nodes[i]->width > max_w) ? nodes[i]->width : max_w;;
	}

	return max_w;
}

static int iFlatTreeConvertIdToY(Ihandle *ih, int id, int *h)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i, pos = 0;
	int w, temp_h;
	int total_h = 0;

	if (id > count)
		return -1;

	for (i = 0; i < count; i++)
	{
		const char *image = iFlatTreeGetNodeImage(nodes[i]);
		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		iFlatTreeSetNodeFont(ih, nodes[i]->font);
		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			image, nodes[i]->title, &w, &temp_h, 0);

		if (h)
			*h = temp_h;

		if (i == id)
			break;

		total_h += temp_h + ih->data->spacing;

	}

	return total_h;
}

static int iFlatTreePageLastItemId(Ihandle *ih, int py)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int posy = IupGetInt(ih, "POSY");
	int dy = IupGetInt(ih, "DY");
	int i, id = -1, last_id = -1;
	int total_y = posy + dy;
	int total_h, h;

	for (i = 0; i < count; i++)
	{
		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		id = iFlatTreeFindNodeId(ih->data->node_cache, nodes[i]);

		total_h = iFlatTreeConvertIdToY(ih, id, &h);

		if (total_h + h > total_y)
			break;

		last_id = id;
	}

	return last_id;
}

static int iFlatTreePageDownFromFocus(Ihandle *ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
	int posy = IupGetInt(ih, "POSY");
	int dy = IupGetInt(ih, "DY");
	int i, last_id = -1;
	int total_h = 0, w, h;

	for (i = ih->data->focus_id; i < count; i++)
	{
		const char *image = NULL;

		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		image = iFlatTreeGetNodeImage(nodes[i]);
		iFlatTreeSetNodeFont(ih, nodes[i]->font);
		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			nodes[i]->image, nodes[i]->title, &w, &h, 0);

		total_h += (h + ih->data->spacing);

		if (total_h + h > focus_y + dy)
			break;

		last_id = i;
	}

	return last_id;
}

static int iFlatTreePageUpFromFocus(Ihandle *ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
	int posy = IupGetInt(ih, "POSY");
	int dy = IupGetInt(ih, "DY");
	int i, last_id = ih->data->focus_id;
	int total_h = focus_y, w, h;

	for (i = ih->data->focus_id - 1; i >= 0; i--)
	{
		const char *image = NULL;
		if (!iFlatTreeNodeIsVisible(ih, nodes[i]))
			continue;

		image = iFlatTreeGetNodeImage(nodes[i]);
		iFlatTreeSetNodeFont(ih, nodes[i]->font);
		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			image, nodes[i]->title, &w, &h, 0);

		total_h -= (h + ih->data->spacing);

		if (total_h - h < focus_y - dy)
			break;

		last_id = i;
	}

	return last_id;
}

static void iFlatTreeUpdateScrollBar(Ihandle *ih)
{
	int canvas_width = ih->currentwidth;
	int canvas_height = ih->currentheight;
	int sb, max_w = 0, max_h = 0, view_width, view_height;

	if (iupAttribGetBoolean(ih, "BORDER")) /* native border around scrollbars */
	{
		canvas_width -= 2;
		canvas_height -= 2;
	}

	canvas_width -= 2 * ih->data->border_width;
	canvas_height -= 2 * ih->data->border_width;

	view_width = iFlatTreeMaxWidth(ih);
	view_height = iFlatTreeTotalHeight(ih);

	if (ih->data->show_dragdrop || iupAttribGetBoolean(ih, "DRAGDROPTREE"))
		view_height += ih->data->level_gap / 2; /* additional space for drop area */

	sb = iFlatTreeGetScrollbar(ih);
	if (sb)
	{
		int sb_size = iFlatTreeGetScrollbarSize(ih);
		int noscroll_width = canvas_width;
		int noscroll_height = canvas_height;

		if (sb & IUP_SB_HORIZ)
		{
			IupSetInt(ih, "XMAX", view_width);

			if (view_height > noscroll_height)  /* affects horizontal size */
				canvas_width -= sb_size;
		}
		else
			IupSetAttribute(ih, "XMAX", "0");

		if (sb & IUP_SB_VERT)
		{
			IupSetInt(ih, "YMAX", view_height);

			if (view_width > noscroll_width)  /* affects vertical size */
				canvas_height -= sb_size;
		}
		else
			IupSetAttribute(ih, "YMAX", "0");

		/* check again, adding a scrollbar may affect the other scrollbar need if not done already */
		if (sb & IUP_SB_HORIZ && view_height <= noscroll_height && view_height > canvas_height)
			canvas_width -= sb_size;
		if (sb & IUP_SB_VERT && view_width <= noscroll_width && view_width > canvas_width)
			canvas_height -= sb_size;

		if (canvas_width < 0) canvas_width = 0;
		if (canvas_height < 0) canvas_height = 0;

		if (sb & IUP_SB_HORIZ)
			IupSetInt(ih, "DX", canvas_width);
		else
			IupSetAttribute(ih, "DX", "0");

		if (sb & IUP_SB_VERT)
			IupSetInt(ih, "DY", canvas_height);
		else
			IupSetAttribute(ih, "DY", "0");

		IupSetfAttribute(ih, "LINEY", "%d", ih->data->level_gap);
	}
	else
	{
		IupSetAttribute(ih, "XMAX", "0");
		IupSetAttribute(ih, "YMAX", "0");

		IupSetAttribute(ih, "DX", "0");
		IupSetAttribute(ih, "DY", "0");
	}
}


/*******************************************************************************************************/
static void iFlatTreeDrawToggle(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, int x, int y, int h)
{
	char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
	char* bgcolor = iupAttribGet(ih, "BGCOLOR");
	int active = IupGetInt(ih, "ACTIVE");  /* native implementation */

	if (ih->data->toggle_size)
	{
		int check_alig = iupFlatGetVerticalAlignment(iupAttribGetStr(ih, "CHECKALIGN"));
		int check_xmin = x + IFLATTREE_TOGGLE_MARGIN;
		int check_ymin = y + IFLATTREE_TOGGLE_MARGIN + ((h - ih->data->toggle_size) / 2);
		int check_size = ih->data->toggle_size - 2 * IFLATTREE_TOGGLE_MARGIN;

		/* check border */
		iupFlatDrawBorder(dc, check_xmin, check_xmin + check_size,
			check_ymin, check_ymin + check_size,
			IFLATTREE_TOGGLE_BORDER, fgcolor, bgcolor, active);

		/* check mark */
		if (node->toggle_value)
		{
			if (node->toggle_value == -1)
				iupFlatDrawBox(dc, check_xmin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_xmin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
					check_ymin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_ymin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
					fgcolor, bgcolor, active);
			else
				iupFlatDrawCheckMark(dc, check_xmin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_xmin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
					check_ymin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_ymin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
					fgcolor, bgcolor, active);
		}
	}

}

static int iFlatTreeDrawNodes(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, int x, int y, char *foreground_color, char *background_color, int make_inactive, int active,
	int text_flags, int focus_feedback, int width, int border_width, int level, int *pos)
{
	iFlatTreeNode *brother = node;
	int x_pos, max_height;
	int first_y, first_y1;

	x_pos = x + (level * ih->data->level_gap);

	while (brother)
	{
		char *fgcolor = (brother->fgColor) ? brother->fgColor : foreground_color;
		char *bgcolor = (brother->bgColor) ? brother->bgColor : background_color;
		int w, h, img_w, img_h, txt_x, txt_y, toggle_gap = 0;
		const char *image = iFlatTreeGetNodeImage(brother);

		iFlatTreeGetTitlePos(ih, ih->data->focus_id, image, &img_w, &img_h, &txt_x, &txt_y);
		iFlatTreeSetNodeFont(ih, brother->font);
		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			image, brother->title, &w, &h, 0);

		img_w += ih->data->icon_spacing + ih->data->horiz_padding;

		if (brother->parent && brother == brother->parent->firstChild)
			first_y = y;

		if (level != 0)
		{
			int px1, py1, px2, py2;
			px1 = (x_pos - (ih->data->level_gap / 2)) + 1;
			py1 = y + h / 2;
			px2 = x_pos;
			py2 = py1;
			iupdrvDrawLine(dc, px1, py1, px2, py2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);
			if (!brother->brother)
			{
				px1 = x_pos - (ih->data->level_gap / 2);
				py1 = first_y;
				px2 = px1;
				py2 = (brother->brother) ? y + ih->data->level_gap : y + ih->data->level_gap / 2;
				iupdrvDrawLine(dc, px1, py1, px2, py2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);
			}

		}

		if (ih->data->show_toggle && brother->toggle_visible)
		{
			iFlatTreeDrawToggle(ih, dc, brother, x_pos, y, h);
			toggle_gap = ih->data->toggle_size;
		}

		iupFlatDrawBox(dc, x_pos + img_w + toggle_gap, x_pos + toggle_gap + w - 1, y, y + h - 1, bgcolor, bgcolor, 1);

		iFlatTreeSetNodeFont(ih, brother->font);

		iupFlatDrawIcon(ih, dc, x_pos + toggle_gap, y, w, h,
			ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_alignment, ih->data->vert_alignment, ih->data->horiz_padding, ih->data->vert_padding,
			image, make_inactive, brother->title, text_flags, 0, fgcolor, bgcolor, active);

		if (brother->selected || ih->data->dragover_pos == *pos)
		{
			unsigned char red, green, blue;
			char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
			unsigned char a = (unsigned char)iupAttribGetInt(ih, "HLCOLORALPHA");
			long selcolor;

			if (ih->data->dragover_pos == *pos)
				a = (2 * a) / 3;

			iupStrToRGB(hlcolor, &red, &green, &blue);
			selcolor = iupDrawColor(red, green, blue, a);

			iupdrvDrawRectangle(dc, x_pos + img_w + toggle_gap, y, x_pos + toggle_gap + w - 1, y + h - 1, selcolor, IUP_DRAW_FILL, 1);
		}

		if (ih->data->has_focus && ih->data->focus_id == iFlatTreeFindNodeId(ih->data->node_cache, brother) && focus_feedback)
			iupdrvDrawFocusRect(dc, x_pos + img_w + toggle_gap, y, x_pos + toggle_gap + w - border_width - 1, y + h - 1);

		y += h + ih->data->spacing;

		(*pos)++;

		if (brother->kind == IFLATTREE_BRANCH && brother->expand == IFLATTREE_EXPANDED)
			y = iFlatTreeDrawNodes(ih, dc, brother->firstChild, x, y, foreground_color, background_color, make_inactive, active, text_flags, focus_feedback, width, border_width, level + 1, pos);

		brother = brother->brother;

	}

	return y;
}

static int iFlatTreeDrawPlusMinus(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, char *bgcolor, int x, int y, int level)
{
	iFlatTreeNode *brother = node;

	while (brother)
	{
		int w, h;
		const char *image = iFlatTreeGetNodeImage(brother);

		iFlatTreeSetNodeFont(ih, brother->font);

		iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
			image, brother->title, &w, &h, 0);

		if (brother->kind == IFLATTREE_BRANCH)
		{
			int px, py;
			char *exp = (brother->expand == IFLATTREE_EXPANDED) ? "IMGMINUS" : "IMGPLUS";

			py = y + ((h - ih->data->image_plusminus_height) / 2);
			px = x + ((level - 1) * ih->data->level_gap) + 1 + ((ih->data->level_gap - ih->data->image_plusminus_height) / 2);

			if (level > 0)
				iupdrvDrawImage(dc, exp, 0, bgcolor, px, py, ih->data->image_plusminus_height, ih->data->image_plusminus_height);
		}

		y += h + ih->data->spacing;

		if (brother->kind == IFLATTREE_BRANCH && brother->expand == IFLATTREE_EXPANDED)
			y = iFlatTreeDrawPlusMinus(ih, dc, brother->firstChild, bgcolor, x, y, level + 1);

		brother = brother->brother;

	}

	return y;
}

static int iFlatTreeRedraw_CB(Ihandle* ih)
{
	int text_flags = iupDrawGetTextFlags(ih, "TABSTEXTALIGNMENT", "TABSTEXTWRAP", "TABSTEXTELLIPSIS");
	char* foreground_color = iupAttribGetStr(ih, "FGCOLOR");
	char* background_color = iupAttribGetStr(ih, "BGCOLOR");
	int posx = IupGetInt(ih, "POSX");
	int posy = IupGetInt(ih, "POSY");
	char* back_image = iupAttribGet(ih, "BACKIMAGE");
	int i, x, y, make_inactive = 0;
	int border_width = ih->data->border_width;
	int active = IupGetInt(ih, "ACTIVE");  /* native implementation */
	int focus_feedback = iupAttribGetBoolean(ih, "FOCUSFEEDBACK");
	iFlatTreeNode *node;
	int width, height, pos = 0;

	IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);

	iupdrvDrawGetSize(dc, &width, &height);

	iupFlatDrawBox(dc, border_width, width - border_width - 1, border_width, height - border_width - 1, background_color, background_color, 1);

	if (back_image)
	{
		int backimage_zoom = iupAttribGetBoolean(ih, "BACKIMAGEZOOM");
		if (backimage_zoom)
			iupdrvDrawImage(dc, back_image, 0, background_color, border_width, border_width, width - border_width, height - border_width);
		else
			iupdrvDrawImage(dc, back_image, 0, background_color, border_width, border_width, -1, -1);
	}

	if (!active)
		make_inactive = 1;

	x = -posx + border_width;
	y = -posy + border_width;

	if (iupAttribGetInt(ih, "ADDROOT"))
		node = ih->data->node;
	else
		node = ih->data->node->firstChild;

	iFlatTreeDrawNodes(ih, dc, node, x, y, foreground_color, background_color, make_inactive, active, text_flags, focus_feedback, width, border_width, 0, &pos);

	iFlatTreeDrawPlusMinus(ih, dc, node, background_color, x, y, 0);

	if (border_width)
	{
		char* bordercolor = iupAttribGetStr(ih, "BORDERCOLOR");
		iupFlatDrawBorder(dc, 0, width - 1,
			0, height - 1,
			border_width, bordercolor, background_color, active);
	}

	iupdrvDrawFlush(dc);

	iupdrvDrawKillCanvas(dc);

	return IUP_DEFAULT;
}

static int iFlatTreeTextEditKILLFOCUS_CB(Ihandle* text)
{
	Ihandle* ih = text->parent;
	IupSetAttribute(text, "VISIBLE", "NO");
	IupSetAttribute(text, "ACTIVE", "NO");
	IupUpdate(ih);
	return IUP_DEFAULT;
}

static int iFlatTreeTextEditKANY_CB(Ihandle* text, int c)
{
	if (c == K_ESC || c == K_CR)
	{
		iFlatTreeTextEditKILLFOCUS_CB(text);
		return IUP_IGNORE;  /* always ignore to avoid the defaultenter/defaultesc behavior from here */
	}

	return IUP_CONTINUE;
}

static int iFlatTreeTextEditKCR_CB(Ihandle* text, int c)
{
	Ihandle* ih = text->parent;

	iFlatTreeNode *hItemFocus = iFlatTreeGetNode(ih, ih->data->focus_id);

	if (hItemFocus->title)
		free(hItemFocus->title);

	hItemFocus->title = strdup(IupGetAttribute(text, "VALUE"));

	iFlatTreeTextEditKILLFOCUS_CB(text);
	return IUP_DEFAULT;
}

static int iFlatTreeTextEditVALUECHANGED_CB(Ihandle* text)
{
	Ihandle* ih = text->parent;
	iFlatTreeNode *hItemFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
	int pos, img_w, img_h, txt_x, txt_y, width, height;
	char val[1024];
	const char *image = iFlatTreeGetNodeImage(hItemFocus);

	strcpy(val, IupGetAttribute(text, "VALUE"));

	iFlatTreeSetNodeFont(ih, hItemFocus->font);
	iFlatTreeGetTitlePos(ih, ih->data->focus_id, image, &img_w, &img_h, &txt_x, &txt_y);
	iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
		image, val, &width, &height, 0);

	if (text->currentwidth > width)
		return;

	iFllatTreeUpdateText(text, text->x, text->y, width, height);

	return IUP_DEFAULT;
}

static char* iFlatTreeGetIdValueAttrib(Ihandle* ih, int pos);

static void iFlatTreeCallSelectionCallback(Ihandle* ih, IFnii cb, int id, int state)
{
	char *text;

	if (id < 1 || !cb)
		return;

	text = iFlatTreeGetIdValueAttrib(ih, id);

	if (cb(ih, id, state) == IUP_CLOSE)
		IupExitLoop();
}

static void iFlatTreeSingleCallSelectionCb(Ihandle* ih, IFnsii cb, int id, int old_id)
{
	if (old_id != -1)
	{
		if (old_id != id)
		{
			iFlatTreeCallSelectionCallback(ih, cb, old_id, 0);
			iFlatTreeCallSelectionCallback(ih, cb, id, 1);
		}
	}
	else
		iFlatTreeCallSelectionCallback(ih, cb, id, 1);
}

static void iFlatTreeMultipleCallActionCb(Ihandle* ih, IFnsii cb, IFns multi_cb, char* str, int count)
{
	int i;
	int unchanged = 1;

	if (multi_cb)
	{
		if (multi_cb(ih, str) == IUP_CLOSE)
			IupExitLoop();
	}
	else
	{
		/* must simulate the click on each item */
		for (i = 0; i < count; i++)
		{
			if (str[i] != 'x')
			{
				if (str[i] == '+')
					iFlatTreeCallSelectionCallback(ih, cb, i, 1);
				else
					iFlatTreeCallSelectionCallback(ih, cb, i, 0);
				unchanged = 0;
			}
		}
	}
}

static void iFlatTreeSelectNode(Ihandle* ih, int id, int ctrlPressed, int shftPressed)
{
	iFlatTreeNode *node;
	IFns sel_cb = (IFns)IupGetCallback(ih, "SELECTION_CB");
	IFns multi_cb = (IFns)IupGetCallback(ih, "MULTISELECTION_CB");
	IFns multiun_cb = (IFns)IupGetCallback(ih, "MULTIUNSELECTION_CB");
	int count = iupArrayCount(ih->data->node_cache);

	ih->data->focus_id = id;

	if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
	{
		int i, start, end;
		char* str;
		char *val = iupAttribGet(ih, "_IUPFLATTREE_LASTSELECTED");
		int last_id = (val) ? atoi(val) : 0;
		if (id <= last_id)
		{
			start = id;
			end = last_id;
		}
		else
		{
			start = last_id;
			end = id;
		}

		str = malloc(count + 1);
		memset(str, 'x', count); /* mark all as unchanged */
		str[count] = 0;

		if (!ctrlPressed)
		{
			/* un-select all */
			for (i = 0; i < count; i++)
			{
				node = iFlatTreeGetNode(ih, i);
				if (node->selected)
				{
					str[i] = '-';
					node->selected = 0;
				}
			}
		}

		if (shftPressed)
		{
			/* select interval */
			for (i = start; i <= end; i++)
			{
				node = iFlatTreeGetNode(ih, i);
				if (!node->selected)
				{
					str[i] = '+';
					node->selected = 1;
				}
			}
		}
		else
		{
			node = iFlatTreeGetNode(ih, id);

			if (ctrlPressed)
			{
				/* toggle selection */
				if (node->selected)
				{
					str[id] = '-';
					node->selected = 0;
				}
				else
				{
					str[id] = '+';
					node->selected = 1;
				}
			}
			else
			{
				if (!node->selected)
				{
					str[id] = '+';
					node->selected = 1;
				}
			}
		}

		if (multi_cb || sel_cb)
			iFlatTreeMultipleCallActionCb(ih, sel_cb, multi_cb, str, count);

		free(str);
	}
	else
	{
		int i, old_id = -1;

		for (i = 0; i < count; i++)
		{
			node = iFlatTreeGetNode(ih, i);
			if (!node->selected)
				continue;
			node->selected = 0;
			old_id = i;
			break;
		}
		node = iFlatTreeGetNode(ih, id);

		node->selected = 1;

		if (sel_cb)
			iFlatTreeSingleCallSelectionCb(ih, sel_cb, id, old_id);
	}

	if (!shftPressed)
		iupAttribSetInt(ih, "_IUPFLATTREE_LASTSELECTED", id);
}

static int iFlatTreeCallDragDropCb(Ihandle* ih, int drag_id, int drop_id, int is_ctrl, int is_shift)
{
	IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");

	/* ignore a drop that will do nothing */
	if (is_ctrl == 0 && (drag_id - 1 == drop_id || drag_id == drop_id))
		return IUP_DEFAULT;
	if (is_ctrl != 0 && drag_id == drop_id)
		return IUP_DEFAULT;

	drag_id++;
	if (drop_id < 0)
		drop_id = -1;
	else
		drop_id++;

	if (cbDragDrop)
		return cbDragDrop(ih, drag_id, drop_id, is_shift, is_ctrl);  /* starts at 1 */

	return IUP_CONTINUE; /* allow to move/copy by default if callback not defined */
}

static int iFlatTreeHitToggle(Ihandle *ih, int x, int y, int id, int level)
{
	int h;
	int py = iFlatTreeConvertIdToY(ih, id, &h) + ((h - ih->data->toggle_size) / 2);
	int px = (level * ih->data->level_gap) + 1 + ((ih->data->level_gap - ih->data->toggle_size) / 2);
	int posx = IupGetInt(ih, "POSX");
	int posy = IupGetInt(ih, "POSY");

	x += posx - ih->data->border_width;
	y += posy - ih->data->border_width;

	if (x > px && x < px + ih->data->toggle_size && y > py && y < py + ih->data->toggle_size)
		return 1;

	return 0;
}

static int iFlatTreeHitPlusMinus(Ihandle *ih, int x, int y, int id, int level)
{
	int h;
	int py = iFlatTreeConvertIdToY(ih, id, &h) + ((h - ih->data->image_plusminus_height) / 2);
	int px = ((level - 1) * ih->data->level_gap) + 1 + ((ih->data->level_gap - ih->data->image_plusminus_height) / 2);
	int posx = IupGetInt(ih, "POSX");
	int posy = IupGetInt(ih, "POSY");

	x += posx - ih->data->border_width;
	y += posy - ih->data->border_width;

	if (x > px && x < px + ih->data->image_plusminus_height && y > py && y < py + ih->data->image_plusminus_height)
		return 1;

	return 0;
}

static int iFlatTreeButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
	iFlatTreeNode *node;
	IFniiiis button_cb = (IFniiiis)IupGetCallback(ih, "FLAT_BUTTON_CB");
	IFnii toggle_value_cb = (IFniiiis)IupGetCallback(ih, "TOGGLEVALUE_CB");
	char *image;
	int pos = iFlatTreeConvertXYToPos(ih, x, y);
	int id, level, width, height, xmin, xmax;
	int toggle_gap = (ih->data->show_toggle) ? ih->data->toggle_size : 0;

	if (button_cb)
	{
		if (button_cb(ih, button, pressed, x, y, status) == IUP_IGNORE)
			return IUP_DEFAULT;
	}

	if (button == IUP_BUTTON1 && !pressed && ih->data->dragged_pos > 0)
	{
		if (pos == -1)
		{
			if (y < 0)
				pos = 1;
			else
			{
				int count = iFlatTreeGetVisibleNodesCount(ih);
				pos = count;
			}
		}

		if (iFlatTreeCallDragDropCb(ih, ih->data->dragged_pos, pos, iup_iscontrol(status), iup_isshift(status)) == IUP_CONTINUE)
		{
			iFlatTreeNode *droppedNode = NULL;
			int srcId = iFlatTreeConvertPosToId(ih, ih->data->dragged_pos);
			iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
			int destId = iFlatTreeConvertPosToId(ih, pos);
			iFlatTreeNode *destNode = iFlatTreeGetNode(ih, destId);

			if (!iup_iscontrol(status))
				droppedNode = iFlatTreeMoveNode(ih, srcId, destId);
			else
				droppedNode = iFlatTreeCopyNode(ih, srcId, destId);

			if (!droppedNode)
				return IUP_DEFAULT;

			/* select the dropped item */
			iFlatTreeSelectNode(ih, iFlatTreeFindNodeId(ih->data->node_cache, droppedNode), 0, 0); /* force no ctrl and no shift for selection */

			iFlatTreeRebuildCache(ih);
		}

		ih->data->dragover_pos = -1;
		ih->data->dragged_pos = -1;

		iFlatTreeUpdateScrollBar(ih);
		IupUpdate(ih);

		return IUP_DEFAULT;
	}

	if (pos == -1)
		return IUP_DEFAULT;

	id = iFlatTreeConvertPosToId(ih, pos);
	node = iFlatTreeGetNode(ih, id);
	level = iFlatTreeGetNodeLevel(ih, node);
	image = iFlatTreeGetNodeImage(node);
	iFlatTreeSetNodeFont(ih, node->font);
	iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
		image, node->title, &width, &height, 0);
	xmin = (level * ih->data->level_gap) + toggle_gap;
	xmax = xmin + width;

	if (button == IUP_BUTTON3)
	{
		IFni cbRightClick = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");
		if (cbRightClick)
			cbRightClick(ih, id);
		return IUP_DEFAULT;
	}

	if (button == IUP_BUTTON1 && pressed)
	{
		if (iup_isdouble(status) && x > xmin && x < xmax)
		{
			if (node->kind == IFLATTREE_LEAF)
			{
				IFnis dc_cb = (IFnis)IupGetCallback(ih, "EXECUTELEAF_CB");
				if (dc_cb)
				{
					if (dc_cb(ih, id, node->title) == IUP_IGNORE)
						return IUP_DEFAULT;
				}
			}
			else
			{
				if (node->expand == IFLATTREE_EXPANDED)
					node->expand = IFLATTREE_COLLAPSED;
				else
					node->expand = IFLATTREE_EXPANDED;
			}
			iFlatTreeRebuildCache(ih);
		}
		else if (ih->data->show_rename  && x > xmin && x < xmax)
		{
			int current_clock = clock();

			if (id == ih->data->focus_id && (current_clock - ih->data->last_clock) > ih->data->min_clock)
			{
				IFni cb = (IFnnn)IupGetCallback(ih, "SHOWRENAME_CB");
				if (cb)
				{
					if (cb(ih, id) == IUP_IGNORE)
						return IUP_DEFAULT;
				}
				return iFlatTreeRenameNode(ih);
			}

			iFlatTreeSelectNode(ih, id, iup_iscontrol(status), iup_isshift(status));

			if (ih->data->show_dragdrop)
				ih->data->dragged_pos = pos;

			ih->data->last_clock = clock();
		}
		else if (iFlatTreeHitPlusMinus(ih, x, y, id, level))
		{
			if (node->expand == IFLATTREE_EXPANDED)
				node->expand = IFLATTREE_COLLAPSED;
			else
				node->expand = IFLATTREE_EXPANDED;
		}
		else if (iFlatTreeHitToggle(ih, x, y, id, level))
		{
			IFnis tv_cb = (IFnis)IupGetCallback(ih, "TOGGLEVALUE_CB");
			int markWhenToggle = IupGetInt(ih, "MARKWHENTOGGLE");

			if (node->toggle_value > 0)  /* was ON */
			{
				if (ih->data->show_toggle == 2)
					node->toggle_value = -1;
				else
					node->toggle_value = 0;
			}
			else if (node->toggle_value == -1)
				node->toggle_value = 0;
			else  /* was OFF */
				node->toggle_value = 1;

			if (markWhenToggle)
			{
				if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
					node->selected = (node->toggle_value > 0) ? 1 : 0;
				else
				{
					int i, count = iupArrayCount((ih->data->node_cache));

					for (i = 0; i < count; i++)
					{
						iFlatTreeNode *node = iFlatTreeGetNode(ih, i);
						if (!node->selected)
							continue;
						node->selected = 0;
						break;
					}
					node->selected = 1;
				}
			}

			if (tv_cb)
			{
				if (tv_cb(ih, id, node->toggle_value) == IUP_IGNORE)
					return IUP_DEFAULT;
			}
		}
		iFlatTreeRebuildCache(ih);
	}

	iFlatTreeUpdateScrollBar(ih);
	IupUpdate(ih);

	return IUP_DEFAULT;
}

static int iFlatTreeMotion_CB(Ihandle* ih, int x, int y, char* status)
{
	IFniis motion_cb = (IFniis)IupGetCallback(ih, "FLAT_MOTION_CB");
	int pos;

	iupFlatScrollBarMotionUpdate(ih, x, y);

	if (motion_cb)
	{
		if (motion_cb(ih, x, y, status) == IUP_IGNORE)
			return IUP_DEFAULT;
	}

	if (!iup_isbutton1(status) || ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE || !ih->data->show_dragdrop)
		return IUP_IGNORE;

	pos = iFlatTreeConvertXYToPos(ih, x, y);
	if (pos == -1)
		return IUP_DEFAULT;

	if (y < 0 || y > ih->currentheight)
	{
		/* scroll if dragging out of canvas */
		int h;
		int my = IupGetInt(ih, "YMAX");
		int dy = IupGetInt(ih, "DY");
		int id = iFlatTreeConvertPosToId(ih, pos);
		int py = iFlatTreeConvertIdToY(ih, id, &h);
		int posy = (y < 0) ? py : (py + h) - dy;
		IupSetInt(ih, "POSY", posy);
	}

	if (ih->data->dragged_pos >= 0)
		ih->data->dragover_pos = pos;

	IupUpdate(ih);

	return IUP_DEFAULT;
}

static int iFlatTreeFocus_CB(Ihandle* ih, int focus)
{
	IFni cb = (IFni)IupGetCallback(ih, "FLAT_FOCUS_CB");
	if (cb)
	{
		if (cb(ih, focus) == IUP_IGNORE)
			return IUP_DEFAULT;
	}

	ih->data->has_focus = focus;

	IupUpdate(ih);

	return IUP_DEFAULT;
}

static int iFlatTreeResize_CB(Ihandle* ih, int width, int height)
{
	(void)width;
	(void)height;

	iFlatTreeUpdateScrollBar(ih);

	return IUP_DEFAULT;
}

static void iFlatTreeScrollFocusVisible(Ihandle* ih, int direction)
{
	int focus_pos = iFlatTreeConvertIdToPos(ih, ih->data->focus_id);
	int focus_posy = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
	iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);
	int posy = IupGetInt(ih, "POSY");
	int dy = IupGetInt(ih, "DY");
	int ymin = IupGetInt(ih, "YMIN");
	int ymax = IupGetInt(ih, "YMAX");
	int line_height, w;

	if (dy >= (ymax - ymin))
		return;

	iFlatTreeSetNodeFont(ih, node->font);
	iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
		node->image, node->title, &w, &line_height, 0);

	if (focus_posy > posy && (focus_posy + line_height) < (posy + dy))
	{
		posy = focus_posy + line_height - dy;
		IupSetInt(ih, "POSY", posy);
	}
	else if (direction == IFLATTREE_DOWN)
	{
		posy += (focus_posy - posy - dy + line_height);
		IupSetInt(ih, "POSY", posy);
	}
	else
	{
		posy -= (posy - focus_posy);
		IupSetInt(ih, "POSY", posy);
	}
}

static int iFlatTreeKCr_CB(Ihandle* ih)
{
	if (ih->data->has_focus)
	{
		if (ih->data->focus_id >= 0)
		{
			iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);

			if (node->kind == IFLATTREE_BRANCH)
			{
				if (node->expand == IFLATTREE_EXPANDED)
					node->expand = IFLATTREE_COLLAPSED;
				else
					node->expand = IFLATTREE_EXPANDED;
			}
			else
			{
				IFnis dc_cb = (IFnis)IupGetCallback(ih, "EXECUTELEAF_CB");
				if (dc_cb)
				{
					if (dc_cb(ih, ih->data->focus_id, node->title) == IUP_IGNORE)
						return IUP_DEFAULT;
				}
			}



			iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
			iFlatTreeRebuildCache(ih);
			IupUpdate(ih);
		}
	}
	return IUP_DEFAULT;
}

static int iFlatTreeKUp_CB(Ihandle* ih)
{
	if (ih->data->has_focus)
	{
		if (ih->data->focus_id > 0)
		{
			int ctrltPressed = IupGetInt(NULL, "CONTROLKEY");
			int shftPressed = IupGetInt(NULL, "SHIFTKEY");
			int previousId = iFlatTreeGetPreviousVisibleNodeId(ih, ih->data->focus_id);

			if (ctrltPressed)
				ih->data->focus_id = previousId;
			else
				iFlatTreeSelectNode(ih, previousId, 0, shftPressed);

			iFlatTreeScrollFocusVisible(ih, IFLATTREE_UP);
			IupUpdate(ih);
		}
	}
	return IUP_DEFAULT;
}

static int iFlatTreeKDown_CB(Ihandle* ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	if (ih->data->has_focus)
	{
		if (ih->data->focus_id < count - 1)
		{
			int ctrltPressed = IupGetInt(NULL, "CONTROLKEY");
			int shftPressed = IupGetInt(NULL, "SHIFTKEY");
			int nextId = iFlatTreeGetNextVisibleNodeId(ih, ih->data->focus_id);

			if (ctrltPressed)
				ih->data->focus_id = nextId;
			else
				iFlatTreeSelectNode(ih, nextId, 0, shftPressed);

			iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
			IupUpdate(ih);
		}
	}
	return IUP_DEFAULT;
}

static int iFlatTreeKcSpace_CB(Ihandle* ih)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, ih->data->focus_id);
	if (!hItem)
		return IUP_IGNORE;

	if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
		iFlatTreeClearAllSelectionExcept(ih, hItem);

	hItem->selected = !hItem->selected;

	return IUP_DEFAULT;
}

static int iFlatTreeKF2_CB(Ihandle* ih)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, ih->data->focus_id);
	if (!hItem)
		return IUP_IGNORE;

	iFlatTreeRenameNode(ih);

	return IUP_DEFAULT;
}

static int iFlatTreeKHome_CB(Ihandle* ih)
{
	if (ih->data->has_focus)
	{
		int id = iFlatTreeConvertPosToId(ih, 0);

		iFlatTreeSelectNode(ih, id, 0, 0);

		iFlatTreeScrollFocusVisible(ih, IFLATTREE_UP);
		IupUpdate(ih);
	}
	return IUP_DEFAULT;
}

static int iFlatTreeKEnd_CB(Ihandle* ih)
{
	int count = iFlatTreeGetVisibleNodesCount(ih);
	if (ih->data->has_focus)
	{
		int id = iFlatTreeConvertPosToId(ih, count - 1);

		iFlatTreeSelectNode(ih, id, 0, 0);

		iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
		IupUpdate(ih);
	}
	return IUP_DEFAULT;
}

static int iFlatTreeKPgUp_CB(Ihandle* ih)
{
	if (ih->data->has_focus)
	{
		int id = iFlatTreeFocusPageUp(ih);

		iFlatTreeSelectNode(ih, id, 0, 0);

		iFlatTreeScrollFocusVisible(ih, IFLATTREE_UP);
		IupUpdate(ih);
	}
	return IUP_DEFAULT;
}

static int iFlatTreeKPgDn_CB(Ihandle* ih)
{
	int visibleCount = iFlatTreeGetVisibleNodesCount(ih);
	if (ih->data->has_focus)
	{
		int id = iFlatTreeFocusPageDown(ih);

		iFlatTreeSelectNode(ih, id, 0, 0);

		iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
		IupUpdate(ih);
	}
	return IUP_DEFAULT;
}

static int iFlatListFindNextItem(Ihandle* ih, char c)
{
	int focus_pos = iFlatTreeConvertIdToPos(ih, ih->data->focus_id);
	int count = iupArrayCount(ih->data->node_cache);
	int start = focus_pos + 1;
	int i, end = count;
	iFlatTreeNode **nodes = (iFlatTreeNode*)iupArrayGetData(ih->data->node_cache);

	if (start == end)
	{
		end = start - 1;
		start = 0;
	}

	c = iup_tolower(c);

	for (i = start; i < end; i++)
	{
		char* title = nodes[i]->title;

		if (title && iup_tolower(title[0]) == c)
			return i + 1;
	}

	return -1;
}

static int iFlatTreeScroll_CB(Ihandle *ih)
{
	IupUpdate(ih);

	return IUP_DEFAULT;
}

/******************************************************************************************/


static char* iFlatTreeGetAddExpandedAttrib(Ihandle* ih)
{
	return iupStrReturnBoolean(ih->data->add_expanded);
}

static int iFlatTreeSetAddExpandedAttrib(Ihandle* ih, const char* value)
{
	if (iupStrBoolean(value))
		ih->data->add_expanded = 1;
	else
		ih->data->add_expanded = 0;

	return 0;
}

static char* iFlatTreeGetIndentationAttrib(Ihandle* ih)
{
	return iupStrReturnInt(ih->data->indentation);
}

static int iFlatTreeSetIndentationAttrib(Ihandle* ih, const char* value)
{
	iupStrToInt(value, &ih->data->indentation);
	return 0;
}

static char* iFlatTreeGetShowToggleAttrib(Ihandle* ih)
{
	if (ih->data->show_toggle)
	{
		if (ih->data->show_toggle == 2)
			return "3STATE";
		else
			return "YES";
	}
	else
		return "NO";
}

static int iFlatTreeSetShowToggleAttrib(Ihandle* ih, const char* value)
{
	if (iupStrEqualNoCase(value, "3STATE"))
		ih->data->show_toggle = 2;
	else if (iupStrBoolean(value))
		ih->data->show_toggle = 1;
	else
		ih->data->show_toggle = 0;

	return 0;
}

static int iFlatTreeSetAlignmentAttrib(Ihandle* ih, const char* value)
{
	char value1[30], value2[30];

	iupStrToStrStr(value, value1, value2, ':');

	ih->data->horiz_alignment = iupFlatGetHorizontalAlignment(value1);
	ih->data->vert_alignment = iupFlatGetVerticalAlignment(value2);

	if (ih->handle)
		iupdrvRedrawNow(ih);

	return 1;
}

static char* iFlatTreeGetAlignmentAttrib(Ihandle *ih)
{
	char* horiz_align2str[3] = { "ALEFT", "ACENTER", "ARIGHT" };
	char* vert_align2str[3] = { "ATOP", "ACENTER", "ABOTTOM" };
	return iupStrReturnStrf("%s:%s", horiz_align2str[ih->data->horiz_alignment], vert_align2str[ih->data->vert_alignment]);
}

static char* iFlatTreeGetSpacingAttrib(Ihandle* ih)
{
	return iupStrReturnInt(ih->data->spacing);
}

static char* iFlatTreeGetHasFocusAttrib(Ihandle* ih)
{
	return iupStrReturnBoolean(ih->data->has_focus);
}

static char* iFlatTreeGetIdValueAttrib(Ihandle* ih, int id)
{
	int count = iFlatTreeGetVisibleNodesCount(ih);
	iFlatTreeNode *node;

	if (id < 0)
		return 0;

	node = iFlatTreeGetNode(ih, id);

	return node->title;
}

static int iFlatListSetIdValueAttrib(Ihandle* ih, int pos, const char* value)
{
	int count = iupArrayCount(ih->data->node_cache);

	if (pos < 1)
		return 0;

	if (!value)
		iFlatListRemoveItem(ih, 0, count - pos - 1);
	else if (pos <= count)
	{
		iFlatTreeNode **nodes = (iFlatTreeNode*)iupArrayGetData(ih->data->node_cache);

		if (nodes[pos - 1]->title)
			free(nodes[pos - 1]->title);
		nodes[pos - 1]->title = iupStrDup(value);
	}
	else
	{
		iFlatTreeNode **nodes = (iFlatTreeNode*)iupArrayInsert(ih->data->node_cache, count, pos - 1 - count + 1);
		nodes[pos - 1]->title = iupStrDup(value);
	}

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		IupUpdate(ih);
	}

	return 0;
}

static char* iFlatTreeGetStateAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	if (hItem->kind == IFLATTREE_LEAF)
		return NULL;

	if (hItem->expand == IFLATTREE_EXPANDED)
		return "EXPANDED";
	else
		return "COLLAPSED";
}

static int iFlatTreeSetStateAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return 0;

	if (hItem->kind == IFLATTREE_LEAF)
		return 0;

	if (iupStrEqualNoCase(value, "EXPANDED"))
		hItem->expand = IFLATTREE_EXPANDED;
	else /* "HORIZONTAL" */
		hItem->expand = IFLATTREE_COLLAPSED;

	if (ih->handle)
	{
		iFlatTreeRebuildCache(ih);
		iFlatTreeUpdateScrollBar(ih);
		IupUpdate(ih);
	}

	return 0;
}

static char* iFlatTreeGetKindAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);

	if (node->kind == IFLATTREE_BRANCH)
		return "BRANCH";
	else
		return "LEAF";
}

static char* iFlatTreeGetParentAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, hItem->parent));
}

static char* iFlatTreeGetNextAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, hItem->brother));
}

static char* iFlatTreeGetPreviousAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	iFlatTreeNode *brother;
	if (!hItem)
		return NULL;

	brother = hItem->parent->firstChild;

	if (brother == hItem)
		return NULL;

	while (brother->brother != hItem)
		brother = brother->brother;

	return iupStrReturnInt(iupTreeFindNodeId(ih, brother));
}

static char* iFlatTreeGetLastAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItemLast = NULL;
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	hItemLast = hItem;
	while (hItemLast->brother)
		hItemLast = hItemLast->brother;

	return iupStrReturnInt(iupTreeFindNodeId(ih, hItemLast));
}

static char* iFlatTreeGetFirstAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	return iupStrReturnInt(iupTreeFindNodeId(ih, hItem->parent->firstChild));
}

static char* iFlatTreeGetTitleAttrib(Ihandle* ih, int id)
{
	char title[255];
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;
	return iupStrReturnStr(hItem->title);
}

static int iFlatTreeSetTitleAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return 0;

	if (!value)
		value = "";

	hItem->title = iupStrDup(value);

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetTitleFontAttrib(Ihandle* ih, int pos)
{
	iFlatTreeNode *hItem;

	int count = iupArrayCount(ih->data->node_cache);
	if (pos < 1 || pos > count)
		return 0;

	hItem = iFlatTreeGetNode(ih, pos);

	return hItem->font;
}

static int iFlatTreeSetTitleFontAttrib(Ihandle* ih, int pos, const char* value)
{
	iFlatTreeNode *hItem;

	int count = iupArrayCount(ih->data->node_cache);
	int i = pos;

	if (pos < 1 || pos > count)
		return 0;

	hItem = iFlatTreeGetNode(ih, pos);

	if (hItem->font)
		free(hItem->font);
	hItem->font = iupStrDup(value);


	if (ih->handle)
	{
		IupUpdate(ih);
		iFlatTreeRebuildCache(ih);
		iFlatTreeUpdateScrollBar(ih);
	}

	return 0;
}

static int iTreeSetTitleFontStyleAttrib(Ihandle* ih, int id, const char* value)
{
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	font = IupGetAttributeId(ih, "TITLEFONT", id);
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "TITLEFONT", id, "%s, %s %d", typeface, value, size);

	return 0;
}

static char* iFlatTreeGetTitleFontStyleAttrib(Ihandle* ih, int id)
{
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];

	char* font = IupGetAttributeId(ih, "TITLEFONT", id);
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnStrf("%s%s%s%s", is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "");
}

static int iFlatTreeSetTitleFontSizeAttrib(Ihandle* ih, int id, const char* value)
{
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	font = IupGetAttributeId(ih, "TITLEFONT", id);
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "TITLEFONT", id, "%s, %s%s%s%s %s", typeface, is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "", value);

	return 0;
}

static char* iFlatTreeGetTitleFontSizeAttrib(Ihandle* ih, int id)
{
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];

	char* font = IupGetAttributeId(ih, "TITLEFONT", id);
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnInt(size);
}

static int iFlatTreeSetTitleFontStyleAttrib(Ihandle* ih, int pos, const char* value)
{
	iFlatTreeNode *node = (iFlatTreeNode *)iFlatTreeGetNode(ih, pos);
	int count = iupArrayCount(ih->data->node_cache);
	int i = pos - 1;
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	if (pos < 1 || pos > count)
		return 0;

	font = node->font;
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "TITLEFONT", pos, "%s, %s %d", typeface, value, size);

	return 0;
}

static char* iFlatTreeGetToggleValueAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem;

	if (!ih->data->show_toggle)
		return NULL;

	hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	if (!hItem->toggle_visible)
		return NULL;

	return iupStrReturnChecked(hItem->toggle_value);
}

static int iFlatTreeSetToggleValueAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItem;

	if (!ih->data->show_toggle)
		return 0;

	hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return 0;

	if (!hItem->toggle_visible)
		return 0;

	if (ih->data->show_toggle == 2 && iupStrEqualNoCase(value, "NOTDEF"))
		hItem->toggle_value = -1;  /* indeterminate, inconsistent */
	else if (iupStrEqualNoCase(value, "ON"))
		hItem->toggle_value = 1;
	else
		hItem->toggle_value = 0;

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetNodeTitleStyleAttrib(Ihandle* ih, int pos)
{
	iFlatTreeNode *items = (iFlatTreeNode *)iFlatTreeGetNode(ih, pos);
	int count = iupArrayCount(ih->data->node_cache);
	int i = pos - 1;
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (pos < 1 || pos > count)
		return 0;

	font = items[i].font;
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnStrf("%s%s%s%s", is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "");
}

static char* iFlatTreeGetToggleVisibleAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem;

	if (!ih->data->show_toggle)
		return NULL;

	hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	return iupStrReturnBoolean(hItem->toggle_visible);
}

static int iFlatTreeSetToggleVisibleAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItem;

	if (!ih->data->show_toggle)
		return 0;

	hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return 0;

	hItem->toggle_visible = iupStrBoolean(value);

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetUserDataAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *item = (iFlatTreeNode *)iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);

	if (id >= 0 && id < count)
		return item->userdata;
	else if (id == IUP_INVALID_ID && count != 0)
	{
		item = (iFlatTreeNode *)iFlatTreeGetNode(ih, ih->data->focus_id);
		if (id >= 0 && id < count)
			return item->userdata;
	}
	return NULL;
}

static int iFlatTreeSetRenameAttrib(Ihandle* ih, const char* value)
{
	iFlatTreeRenameNode(ih);
	(void)value;
	return 0;
}

static int iFlatTreeSetUserDataAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *item = (iFlatTreeNode *)iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);

	if (id >= 0 && id < count)
		item->userdata = (void*)value;
	else if (id == IUP_INVALID_ID && count != 0)
	{
		item = (iFlatTreeNode *)iFlatTreeGetNode(ih, ih->data->focus_id);
		if (id >= 0 && id < count)
			item->userdata = (void*)value;
	}
	return 0;
}

static int iFlatTreeSetAddLeafAttrib(Ihandle* ih, int pos, const char* value)
{
	int count = iupArrayCount(ih->data->node_cache);

	if (pos > count)
		return 0;

	if (value)
	{
		iFlatTreeAddNode(ih, pos, IFLATTREE_LEAF, value);

		iFlatTreeRebuildCache(ih);
	}

	ih->data->lastAddNode = pos + 1;

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetInsertLeafAttrib(Ihandle* ih, int pos, const char* value)
{
	int count = iupArrayCount(ih->data->node_cache);
	int i;

	if (pos > count)
		return 0;

	if (value)
	{
		iFlatTreeInsertNode(ih, pos, IFLATTREE_LEAF, value);

		iFlatTreeRebuildCache(ih);
	}

	ih->data->lastAddNode = pos + 1;

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetAddBranchAttrib(Ihandle* ih, int pos, const char* value)
{
	int count = iupArrayCount(ih->data->node_cache);

	if (pos > count)
		return 0;

	if (value)
	{
		iFlatTreeAddNode(ih, pos, IFLATTREE_BRANCH, value);

		iFlatTreeRebuildCache(ih);
	}

	ih->data->lastAddNode = pos + 1;

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetInsertBranchAttrib(Ihandle* ih, int pos, const char* value)
{
	int count = iupArrayCount(ih->data->node_cache);
	int i;

	if (pos > count)
		return 0;

	if (value)
	{
		iFlatTreeInsertNode(ih, pos, IFLATTREE_BRANCH, value);

		iFlatTreeRebuildCache(ih);
	}

	ih->data->lastAddNode = pos + 1;

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetDelNodeAttrib(Ihandle* ih, int pos, const char* value)
{
	int count, i;
	iFlatTreeNode *node = iFlatTreeGetNode(ih, pos);

	if (!ih->handle)  /* do not do the action before map */
		return 0;

	if (iupStrEqualNoCase(value, "ALL"))
	{
		iFlatTreeRemoveNode(ih, ih->data->node, 0);
		return 0;
	}

	if (iupStrEqualNoCase(value, "SELECTED")) /* selected here means the reference one */
	{
		iFlatTreeRemoveNode(ih, node, 0);
		return 0;
	}
	else if (iupStrEqualNoCase(value, "CHILDREN"))  /* children of the reference node */
	{
		iFlatTreeRemoveNode(ih, node, 1);
		return 0;
	}
	else if (iupStrEqualNoCase(value, "MARKED"))
	{
		iFlatTreeRemoveMarkedNodes(ih, ih->data->node);
		return 0;
	}

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
{
	int count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	int i, all = iupStrFalse(value);

	for (i = 0; i < count; i++)
	{
		if (nodes[i]->kind == IFLATTREE_LEAF)
			continue;

		nodes[i]->expand = (all) ? 1 : 0;
	}

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetMoveNodeAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItemDst, *hParent, *hItemSrc;
	int dstId = IUP_INVALID_ID;

	if (!ih->handle)  /* do not do the action before map */
		return 0;

	iupStrToInt(value, &dstId);

	/* If Drag item is an ancestor of Drop item then return */
	hParent = hItemDst;
	while (hParent)
	{
		hParent = hParent->parent;
		if (hParent == hItemSrc)
			return 0;
	}

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetCopyNodeAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItemDst, *hParent, *hItemSrc;
	int dstId = IUP_INVALID_ID;

	if (!ih->handle)  /* do not do the action before map */
		return 0;

	iupStrToInt(value, &dstId);

	/* If Drag item is an ancestor of Drop item then return */
	hParent = hItemDst;
	while (hParent)
	{
		hParent = hParent->parent;
		if (hParent == hItemSrc)
			return 0;
	}

	iFlatTreeCopyNode(ih, id, dstId);

	return 0;
}

static int iFlatTreeSetValueAttrib(Ihandle* ih, const char* value)
{
	iFlatTreeNode *node = NULL;
	iFlatTreeNode *nodeFocus;
	int old_focus_id = ih->data->focus_id;

	nodeFocus = iFlatTreeGetNode(ih, ih->data->focus_id);

	if (iupStrEqualNoCase(value, "ROOT") || iupStrEqualNoCase(value, "FIRST"))
	{
		if (iupAttribGetInt(ih, "ADDROOT"))
			ih->data->focus_id = iFlatTreeFindNodeId(ih->data->node_cache, ih->data->node);
		else
			ih->data->focus_id = iFlatTreeFindNodeId(ih->data->node_cache, ih->data->node->firstChild);
	}
	else if (iupStrEqualNoCase(value, "LAST"))
	{
		int pos = iFlatTreeGetVisibleNodesCount(ih) - 1;
		ih->data->focus_id = iFlatTreeConvertPosToId(ih, pos);
	}
	else if (iupStrEqualNoCase(value, "PGUP"))
		ih->data->focus_id = iFlatTreeFocusPageUp(ih);
	else if (iupStrEqualNoCase(value, "PGDN"))
		ih->data->focus_id = iFlatTreeFocusPageDown(ih);
	else if (iupStrEqualNoCase(value, "NEXT"))
		ih->data->focus_id = iFlatTreeGetNextVisibleNodeId(ih, ih->data->focus_id);
	else if (iupStrEqualNoCase(value, "PREVIOUS"))
		ih->data->focus_id = iFlatTreeGetPreviousVisibleNodeId(ih, ih->data->focus_id);
	else if (iupStrEqualNoCase(value, "CLEAR"))
		ih->data->focus_id = -1;
	else
	{
		int id = IUP_INVALID_ID;
		iupStrToInt(value, &id);
		ih->data->focus_id = id;
	}
	if (ih->data->focus_id != old_focus_id && ih->data->focus_id >= 0)
	{
		int direction = (old_focus_id < ih->data->focus_id) ? IFLATTREE_DOWN : IFLATTREE_UP;
		if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
			iFlatTreeSelectNode(ih, ih->data->focus_id, 0, 0);
		iFlatTreeScrollFocusVisible(ih, direction);
	}

	IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetValueAttrib(Ihandle* ih)
{
	if (ih->data->focus_id < 0)
		return "0";
	else if (iFlatTreeGetVisibleNodesCount(ih) == 0)
		return "-1";

	return iupStrReturnInt(ih->data->focus_id);
}

static int iFlatTreeSetMarkAttrib(Ihandle* ih, const char* value)
{
	if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
		return 0;

	if (iupStrEqualNoCase(value, "BLOCK"))
	{
		iFlatTreeNode *hItemFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
		iFlatTreeSelectRange(ih, (iFlatTreeNode*)iupAttribGet(ih, "_IUPTREE_MARKSTART_NODE"), hItemFocus, 0);
	}
	else if (iupStrEqualNoCase(value, "CLEARALL"))
		iFlatTreeClearAllSelectionExcept(ih, NULL);
	else if (iupStrEqualNoCase(value, "MARKALL"))
		iFlatTreeSelectAll(ih);
	else if (iupStrEqualNoCase(value, "INVERTALL")) /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
		iFlatTreeInvertSelection(ih);
	else if (iupStrEqualPartial(value, "INVERT")) /* iupStrEqualPartial allows the use of "INVERTid" form */
	{
		iFlatTreeNode *hItem = iFlatTreeGetNodeFromString(ih, &value[strlen("INVERT")]);
		if (!hItem)
			return 0;

		hItem->selected = !(hItem->selected); /* toggle */
	}
	else
	{
		iFlatTreeNode *hItem1, *hItem2;

		char str1[50], str2[50];
		if (iupStrToStrStr(value, str1, str2, '-') != 2)
			return 0;

		hItem1 = iFlatTreeGetNodeFromString(ih, str1);
		if (!hItem1)
			return 0;
		hItem2 = iFlatTreeGetNodeFromString(ih, str2);
		if (!hItem2)
			return 0;

		iFlatTreeSelectRange(ih, hItem1, hItem2, 0);
	}

	IupUpdate(ih);

	return 1;
}

static int iFlatTreeSetMarkStartAttrib(Ihandle* ih, const char* value)
{
	iFlatTreeNode *hItem = iFlatTreeGetNodeFromString(ih, value);
	if (!hItem)
		return 0;

	iupAttribSet(ih, "_IUPTREE_MARKSTART_NODE", (char*)hItem);

	return 1;
}

static char* iFlatTreeGetMarkedAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	if (!hItem)
		return NULL;

	return iupStrReturnBoolean(hItem->selected);
}

static int iFlatTreeSetMarkedAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	int set_focus = 0;
	if (!hItem)
		return 0;

	if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
	{
		iFlatTreeNode *hItemFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
		if (hItemFocus != hItem)
		{
			hItemFocus->selected = 0;
			set_focus = 1;
		}
	}

	hItem->selected = iupStrBoolean(value);

	if (set_focus)
		ih->data->focus_id = id;

	return 0;
}

static char* iFlatTreeGetMarkedNodesAttrib(Ihandle* ih)
{
	int count = iupArrayCount(ih->data->node_cache);
	char* str = iupStrGetMemory(count + 1);
	int i;

	for (i = 0; i < count; i++)
	{
		iFlatTreeNode *node = iFlatTreeGetNode(ih->data->node_cache, i);
		if (node->selected)
			str[i] = '+';
		else
			str[i] = '-';
	}

	str[count] = 0;
	return str;
}

static int iFlatTreeSetMarkedNodesAttrib(Ihandle* ih, const char* value)
{
	int count, i, nodeCount;

	if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE || !value)
		return 0;

	nodeCount = iupArrayCount(ih->data->node_cache);

	count = (int)strlen(value);
	if (count > nodeCount)
		count = nodeCount;

	for (i = 0; i < count; i++)
	{
		iFlatTreeNode *node = iFlatTreeGetNode(ih->data->node_cache, i);
		if (value[i] == '+')
			node->selected = 1;
		else
			node->selected = 1;
	}

	return 0;
}

static char* iFlatTreeGetMarkModeAttrib(Ihandle* ih)
{
	if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
		return "SINGLE";
	else
		return "MULTIPLE";
}

static int iFlatTreeSetMarkModeAttrib(Ihandle* ih, const char* value)
{
	if (iupStrEqualNoCase(value, "MULTIPLE"))
		ih->data->mark_mode = IFLATTREE_MARK_MULTIPLE;
	else
		ih->data->mark_mode = IFLATTREE_MARK_SINGLE;

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static int iFlatTreeSetImageAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *node;
	int count = iupArrayCount(ih->data->node_cache);

	if (id < 1 || id > count)
		return 0;

	node = iFlatTreeGetNode(ih, id);

	if (node->image)
		free(node->image);
	node->image = iupStrDup(value);

	iFlatTreeRebuildCache(ih);

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static char* iFlatTreeGetImageAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node;
	int count = iupArrayCount(ih->data->node_cache);

	if (id < 1 || id > count)
		return 0;

	node = iFlatTreeGetNode(ih, id);

	return node->image;
}

static int iFlatTreeSetImageExpandedAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *node;
	int count = iupArrayCount(ih->data->node_cache);

	if (id < 1 || id > count)
		return 0;

	node = iFlatTreeGetNode(ih, id);

	if (node->image_expanded)
		free(node->image_expanded);
	node->image_expanded = iupStrDup(value);

	if (ih->handle)
	{
		iFlatTreeUpdateScrollBar(ih);
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static char* iFlatTreeGetImageExpandedAttrib(Ihandle* ih, int pos)
{
	iFlatTreeNode *node;
	int count = iupArrayCount(ih->data->node_cache);

	if (pos < 1 || pos > count)
		return 0;

	node = iFlatTreeGetNode(ih, pos);

	return node->image_expanded;
}

static char* iFlatTreeGetImageNativeHandleAttribId(Ihandle* ih, int pos)
{
	iFlatTreeNode *node;
	int count = iupArrayCount(ih->data->node_cache);

	if (pos < 1 || pos > count)
		return 0;

	node = iFlatTreeGetNode(ih, pos);

	if (node->image)
		return iupImageGetImage(node->image, ih, 0, NULL);

	return NULL;
}

static int iFlatTreeSetImagePositionAttrib(Ihandle* ih, const char* value)
{
	ih->data->img_position = iupFlatGetImagePosition(value);

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetImagePositionAttrib(Ihandle *ih)
{
	char* img_pos2str[4] = { "LEFT", "RIGHT", "TOP", "BOTTOM" };
	return img_pos2str[ih->data->img_position];
}

static char* iFlatTreeGetShowDragDropAttrib(Ihandle* ih)
{
	return iupStrReturnBoolean(ih->data->show_dragdrop);
}

static int iFlatTreeSetShowDragDropAttrib(Ihandle* ih, const char* value)
{
	/* valid only before map */
	if (ih->handle)
		return 0;

	if (iupStrBoolean(value))
		ih->data->show_dragdrop = 1;
	else
		ih->data->show_dragdrop = 0;

	return 0;
}

void iFlatTreeDragDropCopyNode(Ihandle* ih_src, Ihandle *ih, iFlatTreeNode *nodeSrc, iFlatTreeNode *nodeDst, int isControl)
{
	iFlatTreeNode *nodeNew, *parent;
	int id_new, count, id_dst;

	int old_count = iupArrayCount(ih->data->node_cache);

	id_dst = iFlatTreeFindNodeId(ih->data->node_cache, nodeDst);

	if (isControl)
		nodeNew = iFlatTreeCloneNode(nodeSrc, 0);
	else
	{
		iFlatTreeUnlinkNode(ih_src, nodeSrc, 0);
		nodeNew = nodeSrc;
	}

	if (nodeDst->kind == IFLATTREE_BRANCH && nodeDst->expand == IFLATTREE_EXPANDED)
	{
		/* copy as first child of expanded branch */
		nodeNew->parent = nodeDst;
		nodeNew->brother = nodeDst->firstChild;
		nodeDst->firstChild = nodeNew;
	}
	else
	{
		nodeNew->parent = nodeDst->parent;
		nodeNew->brother = nodeDst->brother;
		nodeDst->brother = nodeNew;
	}
}

static int iFlatTreeDropData_CB(Ihandle *ih, char* type, void* data, int len, int x, int y)
{
	int pos = iFlatTreeConvertXYToPos(ih, x, y);
	int is_ctrl = 0;
	char key[5];

	/* Data is not the pointer, it contains the pointer */
	Ihandle* ih_source;
	memcpy((void*)&ih_source, data, len);

	/* A copy operation is enabled with the CTRL key pressed, or else a move operation will occur.
	   A move operation will be possible only if the attribute DRAGSOURCEMOVE is Yes.
	   When no key is pressed the default operation is copy when DRAGSOURCEMOVE=No and move when DRAGSOURCEMOVE=Yes. */
	iupdrvGetKeyState(key);
	if (key[1] == 'C')
		is_ctrl = 1;

	/* Here copy/move of multiple selection is not allowed,
	   only a single node and its children. */

	int srcPos = iupAttribGetInt(ih_source, "_IUP_FLAT_TREE_SOURCEPOS");
	int src_id = iFlatTreeConvertPosToId(ih_source, srcPos);
	iFlatTreeNode *itemDst, *itemSrc;

	itemSrc = iFlatTreeGetNode(ih_source, srcPos);
	if (!itemSrc)
		return IUP_DEFAULT;

	itemDst = iFlatTreeGetNode(ih, pos);
	if (!itemDst)
		return IUP_DEFAULT;

	/* Copy the node and its children to the new position */
	iFlatTreeDragDropCopyNode(ih_source, ih, itemSrc, itemDst, is_ctrl);

	if (ih->handle)
	{
		iFlatTreeRebuildCache(ih);
		iFlatTreeUpdateScrollBar(ih);
		IupUpdate(ih);
	}

	if (ih_source->handle)
	{
		iFlatTreeRebuildCache(ih_source);
		iFlatTreeUpdateScrollBar(ih_source);
		IupUpdate(ih_source);
	}

	(void)type;
	return IUP_DEFAULT;
}

static int iFlatTreeDragData_CB(Ihandle *ih, char* type, void *data, int len)
{
	int pos = iupAttribGetInt(ih, "_IUP_FLAT_TREE_SOURCEPOS");
	if (pos < 1)
		return IUP_DEFAULT;

	IupSetAttributeId(ih, "MARKED", pos, "YES");

	/* Copy source handle */
	memcpy(data, (void*)&ih, len);

	(void)type;
	return IUP_DEFAULT;
}


static int iFlatTreeDragDataSize_CB(Ihandle* ih, char* type)
{
	(void)ih;
	(void)type;
	return sizeof(Ihandle*);
}

static int iFlatTreeDragBegin_CB(Ihandle* ih, int x, int y)
{
	int pos = iFlatTreeConvertXYToPos(ih, x, y);

	if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
		return IUP_IGNORE;

	iupAttribSetInt(ih, "_IUP_FLAT_TREE_SOURCEPOS", pos);
	return IUP_DEFAULT;
}

static int iFlatTreeDragEnd_CB(Ihandle *ih, int del)
{
	iupAttribSetInt(ih, "_IUP_FLAT_TREE_SOURCEPOS", -1);
	(void)del;
	return IUP_DEFAULT;
}

static int iFlatTreeSetDragDropTreeAttrib(Ihandle* ih, const char* value)
{
	if (iupStrBoolean(value))
	{
		/* Register callbacks to enable drag and drop between lists */
		IupSetCallback(ih, "DRAGBEGIN_CB", (Icallback)iFlatTreeDragBegin_CB);
		IupSetCallback(ih, "DRAGDATASIZE_CB", (Icallback)iFlatTreeDragDataSize_CB);
		IupSetCallback(ih, "DRAGDATA_CB", (Icallback)iFlatTreeDragData_CB);
		IupSetCallback(ih, "DRAGEND_CB", (Icallback)iFlatTreeDragEnd_CB);
		IupSetCallback(ih, "DROPDATA_CB", (Icallback)iFlatTreeDropData_CB);
	}
	else
	{
		/* Unregister callbacks */
		IupSetCallback(ih, "DRAGBEGIN_CB", NULL);
		IupSetCallback(ih, "DRAGDATASIZE_CB", NULL);
		IupSetCallback(ih, "DRAGDATA_CB", NULL);
		IupSetCallback(ih, "DRAGEND_CB", NULL);
		IupSetCallback(ih, "DROPDATA_CB", NULL);
	}

	return 1;
}

static int iFlatTreeSetIconSpacingAttrib(Ihandle* ih, const char* value)
{
	iupStrToInt(value, &ih->data->icon_spacing);
	if (ih->handle)
		IupUpdate(ih);
	return 0;
}

static char* iFlatTreeGetIconSpacingAttrib(Ihandle *ih)
{
	return iupStrReturnInt(ih->data->icon_spacing);
}

static char* iFlatTreeGetCountAttrib(Ihandle* ih)
{
	return iupStrReturnInt(iupArrayCount(ih->data->node_cache));
}

static char* iFlatTreeGetChildCountAttrib(Ihandle* ih, int id)
{
	int count;
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	iFlatTreeNode *child = hItem->firstChild;
	if (!hItem)
		return NULL;

	count = 0;
	while (child != NULL)
	{
		count++;

		/* Go to next sibling item */
		child = child->brother;
	}

	return iupStrReturnInt(count);
}

static char* iFlatTreeGetTotalChildCountAttrib(Ihandle* ih, int id)
{
	int count;
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	iFlatTreeNode *child = hItem->firstChild;
	if (!hItem)
		return NULL;

	count = iFlatTreeGetTotalChildCount(hItem);

	return iupStrReturnInt(count);
}

static char* iFlatTreeGetRootCountAttrib(Ihandle* ih)
{
	int count;
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, 0);
	iFlatTreeNode *brother;

	if (!hItem)
		return "0";

	if (iupAttribGetInt(ih, "ADDROOT"))
		count = 1;

	brother = hItem->brother;

	count = 1;
	while (brother != NULL)
	{
		count++;

		/* Go to next sibling item */
		brother = brother->brother;
	}

	return iupStrReturnInt(count);
}

static char* iFlatTreeGetDepthAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *hItem = iFlatTreeGetNode(ih, id);
	int depth = -1;

	if (!hItem)
		return NULL;

	depth = iFlatTreeGetNodeDepth(ih, hItem);

	return iupStrReturnInt(depth);
}

static int iFlatTreeSetSpacingAttrib(Ihandle* ih, const char* value)
{
	iupStrToInt(value, &ih->data->spacing);
	if (ih->handle)
		IupUpdate(ih);
	return 0;
}

static char* iFlatTreeGetNodeFGColorAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	if (id < 0 || id >(count - 1))
		return 0;

	return node->fgColor;
}

static int iFlatTreeSetNodeFGColorAttrib(Ihandle* ih, int pos, const char* value)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, pos);
	int count = iupArrayCount(ih->data->node_cache);

	if (pos < 0 || pos >(count - 1))
		return 0;

	if (node->fgColor)
		free(node->fgColor);
	node->fgColor = iupStrDup(value);

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetNodeBGColorAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);

	if (id < 0 || id >(count - 1))
		return 0;

	return node->bgColor;
}

static int iFlatTreeSetNodeBGColorAttrib(Ihandle* ih, int pos, const char* value)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, pos);
	int count = iupArrayCount(ih->data->node_cache);

	if (pos < 0 || pos >(count - 1))
		return 0;

	if (node->bgColor)
		free(node->bgColor);
	node->bgColor = iupStrDup(value);

	if (ih->handle)
		IupUpdate(ih);

	return 0;
}

static char* iFlatTreeGetNodeFontAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	if (id < 0 || id >(count - 1))
		return 0;

	return node->font;
}

static int iFlatTreeSetNodeFontAttrib(Ihandle* ih, int pos, const char* value)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, pos);
	int count = iupArrayCount(ih->data->node_cache);
	int i = pos;

	if (pos < 0 || pos >(count - 1))
		return 0;

	if (node->font)
		free(node->font);
	node->font = iupStrDup(value);

	if (ih->handle)
	{
		iFlatTreeRebuildCache(ih);
		IupUpdate(ih);
	}

	return 0;
}

static int iFlatTreeSetNodeFontStyleAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	if (id < 0 || id >(count - 1))
		return 0;

	font = node->font;
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "NODEFONT", id, "%s, %s %d", typeface, value, size);

	return 0;
}

static char* iFlatTreeGetNodeFontStyleAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (id < 0 || id >(count - 1))
		return 0;

	font = node->font;
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnStrf("%s%s%s%s", is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "");
}

static int iFlatTreeSetNodeFontSizeAttrib(Ihandle* ih, int id, const char* value)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (!value)
		return 0;

	if (id < 0 || id >(count - 1))
		return 0;

	font = node->font;
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return 0;

	IupSetfAttributeId(ih, "NODEFONT", id, "%s, %s %d", typeface, is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "", value);

	return 0;
}

static char* iFlatTreeGetNodeFontSizeAttrib(Ihandle* ih, int id)
{
	iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
	int count = iupArrayCount(ih->data->node_cache);
	int size = 0;
	int is_bold = 0,
		is_italic = 0,
		is_underline = 0,
		is_strikeout = 0;
	char typeface[1024];
	char* font;

	if (id < 0 || id >(count - 1))
		return 0;

	font = node->font;
	if (!font)
		font = IupGetAttribute(ih, "FONT");

	if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
		return NULL;

	return iupStrReturnInt(size);
}

static int iFlatTreeSetPaddingAttrib(Ihandle* ih, const char* value)
{
	iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
	if (ih->handle)
		IupUpdate(ih);
	return 0;
}

static char* iFlatTreeGetPaddingAttrib(Ihandle* ih)
{
	return iupStrReturnIntInt(ih->data->horiz_padding, ih->data->vert_padding, 'x');
}

static int iFlatTreeSetTopNodeAttrib(Ihandle* ih, const char* value)
{
	int pos = 1;
	if (iupStrToInt(value, &pos))
	{
		int count = iupArrayCount(ih->data->node_cache);
		int id = iFlatTreeConvertPosToId(ih, pos);
		int posy, y;

		if (pos < 1 || pos > count)
			return 0;

		posy = iFlatTreeConvertIdToY(ih, id, NULL);
		IupSetInt(ih, "POSY", posy);

		IupUpdate(ih);
	}
	return 0;
}

static char* iFlatTreeGetShowRenameAttrib(Ihandle* ih)
{
	return iupStrReturnBoolean(ih->data->show_rename);
}

static int iFlatTreeSetShowRenameAttrib(Ihandle* ih, const char* value)
{
	if (iupStrBoolean(value))
		ih->data->show_rename = 1;
	else
		ih->data->show_rename = 0;

	return 0;
}

static int iFlatTreeWheel_CB(Ihandle* ih, float delta)
{
	if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
		iupFlatScrollBarWheelUpdate(ih, delta);
	return IUP_DEFAULT;
}

static int iFlatTreeSetFlatScrollbarAttrib(Ihandle* ih, const char* value)
{
	/* can only be set before map */
	if (ih->handle)
		return IUP_DEFAULT;

	if (value && !iupStrEqualNoCase(value, "NO"))
	{
		if (iupFlatScrollBarCreate(ih))
		{
			IupSetAttribute(ih, "SCROLLBAR", "NO");
			IupSetCallback(ih, "WHEEL_CB", (Icallback)iFlatTreeWheel_CB);
		}
		return 1;
	}
	else
		return 0;
}

static int iFlatTreeSetBorderWidthAttrib(Ihandle* ih, const char* value)
{
	iupStrToInt(value, &ih->data->border_width);
	if (ih->handle)
		iupdrvRedrawNow(ih);
	return 0;
}

static char* iFlatTreeGetBorderWidthAttrib(Ihandle *ih)
{
	return iupStrReturnInt(ih->data->border_width);
}

static int iFlatTreeSetAttribPostRedraw(Ihandle* ih, const char* value)
{
	(void)value;
	if (ih->handle)
		iupdrvPostRedraw(ih);
	return 1;
}

/*****************************************************************************************/

static void iFlatTreeSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
	if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
		iupFlatScrollBarSetChildrenCurrentSize(ih, shrink);
}

static void iFlatTreeSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
	if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
		iupFlatScrollBarSetChildrenPosition(ih);

	(void)x;
	(void)y;
}

static void iFlatTreeDestroyMethod(Ihandle* ih)
{
	int i, count = iupArrayCount(ih->data->node_cache);
	iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
	iFlatTreeNode *nextNode = ih->data->node;

	if (ih->data->node)
	{
		iFlatTreeDelNode(ih->data->node, 0);
		ih->data->node = NULL;
	}

	iupArrayDestroy(ih->data->node_cache);
}

static int iFlatTreeCreateMethod(Ihandle* ih, void** params)
{
	(void)params;
	iFlatTreeNode **nodes;
	Ihandle* text;

	/* free the data allocated by IupCanvas, and reallocate */
	free(ih->data);
	ih->data = iupALLOCCTRLDATA();

	text = IupText(NULL);

	text->currentwidth = 20;  /* just to avoid initial size 0x0 */
	text->currentheight = 10;
	text->flags |= IUP_INTERNAL;
	iupChildTreeAppend(ih, text);

	IupSetCallback(text, "VALUECHANGED_CB", (Icallback)iFlatTreeTextEditVALUECHANGED_CB);
	IupSetCallback(text, "KILLFOCUS_CB", (Icallback)iFlatTreeTextEditKILLFOCUS_CB);
	IupSetCallback(text, "K_ANY", (Icallback)iFlatTreeTextEditKANY_CB);
	IupSetCallback(text, "K_CR", (Icallback)iFlatTreeTextEditKCR_CB);
	IupSetAttribute(text, "FLOATING", "IGNORE");
	IupSetAttribute(text, "VISIBLE", "NO");
	IupSetAttribute(text, "ACTIVE", "NO");

	/* change the IupCanvas default values */
	ih->expand = IUP_EXPAND_NONE;

	/* non zero default values */
	ih->data->img_position = IUP_IMGPOS_LEFT;
	ih->data->horiz_alignment = IUP_ALIGN_ALEFT;
	ih->data->vert_alignment = IUP_ALIGN_ACENTER;
	ih->data->horiz_padding = 2;
	ih->data->vert_padding = 2;
	ih->data->icon_spacing = 2;
	ih->data->add_expanded = 1;
	ih->data->dragover_pos = -1;
	ih->data->show_toggle = 0;
	ih->data->level_gap = (iupRound(iupdrvGetScreenDpi()) > 120) ? 24 : 16;
	ih->data->toggle_size = (iupRound(iupdrvGetScreenDpi()) > 120) ? 24 : 16;
	ih->data->image_plusminus_height = 9;

	ih->data->node = (iFlatTreeNode*)malloc(sizeof(iFlatTreeNode));

	memset(ih->data->node, 0, sizeof(iFlatTreeNode));

	ih->data->node_cache = iupArrayCreate(10, sizeof(iFlatTreeNode*));

	nodes = iupArrayInc(ih->data->node_cache);

	nodes[0] = ih->data->node;
	nodes[0]->kind = IFLATTREE_BRANCH;
	nodes[0]->toggle_visible = 1;

	ih->data->last_clock = 0;
	ih->data->min_clock = 500;

	/* internal callbacks */
	IupSetCallback(ih, "ACTION", (Icallback)iFlatTreeRedraw_CB);
	IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatTreeButton_CB);
	IupSetCallback(ih, "MOTION_CB", (Icallback)iFlatTreeMotion_CB);
	IupSetCallback(ih, "RESIZE_CB", (Icallback)iFlatTreeResize_CB);
	IupSetCallback(ih, "FOCUS_CB", (Icallback)iFlatTreeFocus_CB);
	IupSetCallback(ih, "K_CR", (Icallback)iFlatTreeKCr_CB);
	IupSetCallback(ih, "K_UP", (Icallback)iFlatTreeKUp_CB);
	IupSetCallback(ih, "K_DOWN", (Icallback)iFlatTreeKDown_CB);
	IupSetCallback(ih, "K_HOME", (Icallback)iFlatTreeKHome_CB);
	IupSetCallback(ih, "K_END", (Icallback)iFlatTreeKEnd_CB);
	IupSetCallback(ih, "K_PGUP", (Icallback)iFlatTreeKPgUp_CB);
	IupSetCallback(ih, "K_PGDN", (Icallback)iFlatTreeKPgDn_CB);
	IupSetCallback(ih, "K_sUP", (Icallback)iFlatTreeKUp_CB);
	IupSetCallback(ih, "K_sDOWN", (Icallback)iFlatTreeKDown_CB);
	IupSetCallback(ih, "K_cUP", (Icallback)iFlatTreeKUp_CB);
	IupSetCallback(ih, "K_cDOWN", (Icallback)iFlatTreeKDown_CB);
	IupSetCallback(ih, "K_cSP", (Icallback)iFlatTreeKcSpace_CB);
	IupSetCallback(ih, "K_F2", (Icallback)iFlatTreeKF2_CB);
	IupSetCallback(ih, "SCROLL_CB", (Icallback)iFlatTreeScroll_CB);

	return IUP_NOERROR;
}


/******************************************************************************/


IUP_API Ihandle* IupFlatTree(void)
{
	return IupCreate("flattree");
}

Iclass* iupFlatTreeNewClass(void)
{
	Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

	ic->name = "flattree";
	ic->format = NULL;  /* no parameters */
	ic->nativetype = IUP_TYPECANVAS;
	ic->childtype = IUP_CHILDNONE;
	ic->is_interactive = 1;
	ic->has_attrib_id = 1;

	/* Class functions */
	ic->New = iupFlatTreeNewClass;
	ic->Create = iFlatTreeCreateMethod;
	ic->Destroy = iFlatTreeDestroyMethod;
	ic->SetChildrenCurrentSize = iFlatTreeSetChildrenCurrentSizeMethod;
	ic->SetChildrenPosition = iFlatTreeSetChildrenPositionMethod;

	/* Callbacks */
	iupClassRegisterCallback(ic, "TOGGLEVALUE_CB", "ii");
	iupClassRegisterCallback(ic, "SELECTION_CB", "ii");
	iupClassRegisterCallback(ic, "MULTISELECTION_CB", "Ii");
	iupClassRegisterCallback(ic, "MULTIUNSELECTION_CB", "Ii");
	iupClassRegisterCallback(ic, "BRANCHOPEN_CB", "i");
	iupClassRegisterCallback(ic, "BRANCHCLOSE_CB", "i");
	iupClassRegisterCallback(ic, "EXECUTELEAF_CB", "i");
	iupClassRegisterCallback(ic, "SHOWRENAME_CB", "i");
	iupClassRegisterCallback(ic, "RENAME_CB", "is");
	iupClassRegisterCallback(ic, "DRAGDROP_CB", "iiii");
	iupClassRegisterCallback(ic, "RIGHTCLICK_CB", "i");
	iupClassRegisterCallback(ic, "FLAT_BUTTON_CB", "iiiis");
	iupClassRegisterCallback(ic, "FLAT_MOTION_CB", "iis");
	iupClassRegisterCallback(ic, "FLAT_FOCUS_CB", "i");

	iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupFlatSetActiveAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_DEFAULT);

	/* General Attributes */

	iupClassRegisterAttribute(ic, "ADDEXPANDED", NULL, NULL, "YES", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "ADDROOT", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iFlatTreeSetAttribPostRedraw, IUP_FLAT_FORECOLOR, NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
	iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iFlatTreeSetAttribPostRedraw, IUP_FLAT_BACKCOLOR, NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
	iupClassRegisterAttribute(ic, "EMPTYAS3STATE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "TXTHLCOLOR", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "HLCOLORALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "128", IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "INDENTATION", iFlatTreeGetIndentationAttrib, iFlatTreeSetIndentationAttrib, NULL, NULL, IUPAF_DEFAULT);
	iupClassRegisterAttribute(ic, "SHOWTOGGLE", iFlatTreeGetShowToggleAttrib, iFlatTreeSetShowToggleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "SPACING", iFlatTreeGetSpacingAttrib, iFlatTreeSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttribute(ic, "TOPNODE", NULL, iFlatTreeSetTopNodeAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "SHOWRENAME", iFlatTreeGetShowRenameAttrib, iFlatTreeSetShowRenameAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	/* IupFlatTree Attributes - NODES */

	iupClassRegisterAttribute(ic, "COUNT", iFlatTreeGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "CHILDCOUNT", iFlatTreeGetChildCountAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TOTALCHILDCOUNT", iFlatTreeGetTotalChildCountAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "ROOTCOUNT", iFlatTreeGetRootCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "COLOR", iFlatTreeGetNodeFGColorAttrib, iFlatTreeSetNodeFGColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttributeId(ic, "DEPTH", iFlatTreeGetDepthAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "KIND", iFlatTreeGetKindAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "PARENT", iFlatTreeGetParentAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "NEXT", iFlatTreeGetNextAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "PREVIOUS", iFlatTreeGetPreviousAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "LAST", iFlatTreeGetLastAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "FIRST", iFlatTreeGetFirstAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "STATE", iFlatTreeGetStateAttrib, iFlatTreeSetStateAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TITLE", iFlatTreeGetTitleAttrib, iFlatTreeSetTitleAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttributeId(ic, "TITLEFONT", iFlatTreeGetTitleFontAttrib, iFlatTreeSetTitleFontAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttributeId(ic, "TITLEFONTSTYLE", iFlatTreeGetTitleFontStyleAttrib, iFlatTreeSetTitleFontStyleAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TITLEFONTSIZE", iFlatTreeGetTitleFontSizeAttrib, iFlatTreeSetTitleFontSizeAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TOGGLEVALUE", iFlatTreeGetToggleValueAttrib, iFlatTreeSetToggleValueAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "TOGGLEVISIBLE", iFlatTreeGetToggleVisibleAttrib, iFlatTreeSetToggleVisibleAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "USERDATA", iFlatTreeGetUserDataAttrib, iFlatTreeSetUserDataAttrib, IUPAF_NO_STRING | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "RENAME", NULL, iFlatTreeSetRenameAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

	/* IupFlatTree Attributes - IMAGES */

	iupClassRegisterAttributeId(ic, "IMAGE", iFlatTreeGetImageAttrib, iFlatTreeSetImageAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "IMAGEEXPANDED", iFlatTreeGetImageExpandedAttrib, iFlatTreeSetImageExpandedAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "IMAGELEAF", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);

	/* IupFlatTree Attributes - FOCUS NODE */

	iupClassRegisterAttribute(ic, "VALUE", iFlatTreeGetValueAttrib, iFlatTreeSetValueAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "HASFOCUS", NULL, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);

	/* IupFlatTree Attributes - MARKS */

	iupClassRegisterAttribute(ic, "MARK", NULL, iFlatTreeSetMarkAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "MARKED", iFlatTreeGetMarkedAttrib, iFlatTreeSetMarkedAttrib, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "MARKEDNODES", iFlatTreeGetMarkedNodesAttrib, iFlatTreeSetMarkedNodesAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "MARKMODE", iFlatTreeGetMarkModeAttrib, iFlatTreeSetMarkModeAttrib, IUPAF_SAMEASSYSTEM, "SINGLE", IUPAF_NOT_MAPPED);
	iupClassRegisterAttribute(ic, "MARKSTART", NULL, iFlatTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "MARKWHENTOGGLE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

	/* IupFlatTree Attributes - HIERARCHY */

	iupClassRegisterAttributeId(ic, "ADDLEAF", NULL, iFlatTreeSetAddLeafAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "ADDBRANCH", NULL, iFlatTreeSetAddBranchAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "COPYNODE", NULL, iFlatTreeSetCopyNodeAttrib, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "DELNODE", NULL, iFlatTreeSetDelNodeAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "EXPANDALL", NULL, iFlatTreeSetExpandAllAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "INSERTLEAF", NULL, iFlatTreeSetInsertLeafAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "INSERTBRANCH", NULL, iFlatTreeSetInsertBranchAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "MOVENODE", NULL, iFlatTreeSetMoveNodeAttrib, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, IUP_FLAT_BORDERCOLOR, IUPAF_DEFAULT);  /* inheritable */
	iupClassRegisterAttribute(ic, "BORDERWIDTH", iFlatTreeGetBorderWidthAttrib, iFlatTreeSetBorderWidthAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);  /* inheritable */
	iupClassRegisterAttributeId(ic, "NODEFGCOLOR", iFlatTreeGetNodeFGColorAttrib, iFlatTreeSetNodeFGColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttributeId(ic, "NODEBGCOLOR", iFlatTreeGetNodeBGColorAttrib, iFlatTreeSetNodeBGColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttributeId(ic, "NODEFONT", iFlatTreeGetNodeFontAttrib, iFlatTreeSetNodeFontAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
	iupClassRegisterAttributeId(ic, "NODEFONTSTYLE", iFlatTreeGetNodeFontStyleAttrib, iFlatTreeSetNodeFontStyleAttrib, IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttributeId(ic, "NODEFONTSIZE", iFlatTreeGetNodeFontSizeAttrib, iFlatTreeSetNodeFontSizeAttrib, IUPAF_NO_SAVE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "PADDING", iFlatTreeGetPaddingAttrib, iFlatTreeSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "2x2", IUPAF_NOT_MAPPED);
	iupClassRegisterAttribute(ic, "HASFOCUS", iFlatTreeGetHasFocusAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "ALIGNMENT", iFlatTreeGetAlignmentAttrib, iFlatTreeSetAlignmentAttrib, "ALEFT:ACENTER", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "FOCUSFEEDBACK", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	iupClassRegisterAttributeId(ic, "IMAGENATIVEHANDLE", iFlatTreeGetImageNativeHandleAttribId, NULL, IUPAF_NO_STRING | IUPAF_READONLY | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "IMAGEPOSITION", iFlatTreeGetImagePositionAttrib, iFlatTreeSetImagePositionAttrib, IUPAF_SAMEASSYSTEM, "LEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "ICONSPACING", iFlatTreeGetIconSpacingAttrib, iFlatTreeSetIconSpacingAttrib, IUPAF_SAMEASSYSTEM, "2", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TEXTALIGNMENT", NULL, NULL, IUPAF_SAMEASSYSTEM, "ALEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TEXTWRAP", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "TEXTELLIPSIS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "BACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "BACKIMAGEZOOM", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "FITTOBACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "SHOWDRAGDROP", iFlatTreeGetShowDragDropAttrib, iFlatTreeSetShowDragDropAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "DRAGDROPTREE", NULL, iFlatTreeSetDragDropTreeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
	iupClassRegisterAttribute(ic, "VISIBLELINES", NULL, NULL, "5", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	iupClassRegisterReplaceAttribDef(ic, "SCROLLBAR", "YES", NULL);  /* change the default to Yes */
	iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */
	iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */

	iFlatTreeInitializeImages();
	load_all_images_plusminus();

	/* Flat Scrollbar */
	iupFlatScrollBarRegister(ic);

	iupClassRegisterAttribute(ic, "FLATSCROLLBAR", NULL, iFlatTreeSetFlatScrollbarAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

	return ic;
}
