#include <stdio.h>
#include <iup.h>
#include <iupim.h>

Ihandle* interfaceImage(const char* filename)
{
	Ihandle* img = IupLoadImage(filename);
	if (!img)
	{               
	    printf("error loading: %s\n", filename);
	    return NULL;
	}
	Ihandle* lbl = IupLabel("");
	IupSetAttributeHandle(lbl, "IMAGE", img);
	return IupFrame(lbl);
}

static char* imgList[] = 
{
	"images/p1p2p3.data",
	"images/p1p2p3_delta.data",
	"images/bottom_view.data",
	"images/iso_view.data",
	"images/left_view.data",
	"images/right_view.data",
	"images/top_view.data",
	"images/local_sis.data"
};

static const int Nimages = sizeof(imgList) / sizeof(char*);

int main(void)
{
	IupOpen();
	
	Ihandle* img[Nimages];
	
	Ihandle* vbox = IupVbox(NULL);
	
	int i;
	for(i = 0; i < Nimages; i++)
	{
		printf("Loading image: %s\n", imgList[i]);
		img[i] = interfaceImage(imgList[i]);
		IupAppend(vbox, img[i]);
	}
	
	Ihandle* dlg = IupDialog(vbox);
	
	IupPopup(dlg, IUP_CENTER, IUP_CENTER);
	
	IupClose();
}
