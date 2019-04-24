#include <stdlib.h>
#include <stdio.h>
#include <iup.h>
#include <iupkey.h>

static Ihandle* load_image_FileSave(void)
{
  unsigned char imgdata[] = {
    255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 183, 182, 245, 255, 183, 182, 245, 255, 179, 178, 243, 255, 174, 173, 241, 255, 168, 167, 238, 255, 162, 161, 234, 255, 155, 154, 231, 255, 148, 147, 228, 255, 143, 142, 224, 255, 136, 135, 221, 255, 129, 128, 218, 255, 123, 122, 214, 255, 117, 116, 211, 255, 112, 111, 209, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 179, 178, 243, 255, 190, 189, 255, 255, 147, 146, 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 246, 248, 251, 255, 75, 88, 190, 255, 89, 88, 176, 255, 89, 88, 176, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 173, 172, 240, 255, 190, 189, 255, 255, 138, 137, 239, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 246, 248, 251, 255, 234, 238, 246, 255, 63, 82, 184, 255, 51, 51, 103, 255, 86, 85, 170, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 167, 166, 237, 255, 190, 189, 255, 255, 129, 128, 230, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 246, 248, 251, 255, 234, 238, 246, 255, 222, 229, 240, 255, 52, 77, 179, 255, 122, 121, 223, 255, 83, 82, 164, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 159, 158, 233, 255, 190, 189, 255, 255, 119, 118, 220, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 246, 248, 251, 255, 234, 238, 246, 255, 222, 229, 240, 255, 210, 219, 234, 255, 40, 71, 173, 255, 114, 113, 215, 255, 80, 79, 159, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 152, 151, 229, 255, 190, 189, 255, 255, 110, 109, 211, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 246, 248, 251, 255, 234, 238, 246, 255, 222, 229, 240, 255, 210, 219, 234, 255, 198, 209, 229, 255, 28, 65, 167, 255, 103, 103, 204, 255, 77, 77, 154, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 146, 145, 226, 255, 190, 189, 255, 255, 103, 102, 204, 255, 255, 255, 255, 255, 255, 255, 255, 255, 246, 248, 251, 255, 234, 238, 246, 255, 222, 229, 240, 255, 210, 219, 234, 255, 198, 209, 229, 255, 189, 202, 225, 255, 17, 59, 161, 255, 92, 93, 194, 255, 74, 74, 148, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 139, 138, 223, 255, 188, 187, 255, 255, 183, 182, 255, 255, 96, 99, 201, 255, 86, 94, 196, 255, 75, 88, 190, 255, 63, 82, 184, 255, 52, 77, 179, 255, 40, 71, 173, 255, 28, 65, 167, 255, 17, 59, 161, 255, 92, 93, 193, 255, 84, 86, 186, 255, 71, 71, 143, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 132, 131, 219, 255, 180, 179, 255, 255, 174, 173, 255, 255, 164, 163, 252, 255, 143, 142, 244, 255, 135, 134, 236, 255, 129, 128, 230, 255, 122, 121, 223, 255, 114, 113, 215, 255, 108, 107, 209, 255, 92, 93, 193, 255, 84, 86, 186, 255, 76, 80, 178, 255, 68, 68, 137, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 124, 123, 215, 255, 170, 169, 255, 255, 160, 159, 251, 255, 148, 147, 245, 255, 75, 91, 113, 255, 75, 91, 113, 255, 75, 91, 113, 255, 75, 91, 113, 255, 82, 98, 118, 255, 91, 106, 125, 255, 84, 86, 186, 255, 76, 79, 178, 255, 68, 73, 170, 255, 65, 65, 131, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 118, 117, 212, 255, 160, 159, 255, 255, 145, 144, 245, 255, 135, 134, 236, 255, 75, 91, 113, 255, 0, 0, 0, 255, 52, 60, 71, 255, 206, 217, 233, 255, 212, 221, 236, 255, 103, 116, 133, 255, 67, 75, 174, 255, 68, 73, 170, 255, 60, 66, 163, 255, 62, 62, 125, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 112, 111, 209, 255, 154, 153, 255, 255, 135, 134, 236, 255, 129, 128, 230, 255, 75, 91, 113, 255, 52, 60, 71, 255, 104, 120, 141, 255, 216, 224, 237, 255, 224, 231, 241, 255, 115, 127, 143, 255, 53, 65, 163, 255, 60, 66, 162, 255, 53, 61, 156, 255, 60, 59, 120, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 108, 107, 207, 255, 143, 142, 243, 255, 129, 128, 230, 255, 36, 68, 170, 255, 33, 50, 71, 255, 171, 180, 195, 255, 179, 187, 198, 255, 188, 193, 202, 255, 196, 200, 206, 255, 72, 77, 86, 255, 51, 62, 158, 255, 54, 61, 156, 255, 49, 57, 152, 255, 57, 57, 114, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 108, 107, 207, 84, 101, 100, 195, 255, 86, 85, 170, 255, 83, 82, 164, 255, 80, 79, 159, 255, 77, 77, 154, 255, 74, 74, 148, 255, 71, 71, 143, 255, 68, 68, 137, 255, 65, 65, 131, 255, 60, 59, 120, 255, 60, 59, 120, 255, 57, 57, 114, 255, 55, 54, 110, 255, 255, 0, 255, 255,
    255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255 };

  Ihandle* image = IupImageRGBA(16, 16, imgdata);
  return image;
}

#define TEST_IMAGE_SIZE 20

static unsigned char image_data_32[TEST_IMAGE_SIZE*TEST_IMAGE_SIZE * 4] =
{
  000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 000, 000, 255, 255, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 255, 255, 255, 192, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 255,
  000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255, 000, 000, 000, 255,
};

static int button_cb(Ihandle *ih, int but, int pressed, int x, int y, char* status)
{
  printf("BUTTON_CB(but=%c (%d), x=%d, y=%d [%s]) - [pos=%d]\n",(char)but,pressed,x,y, status, IupConvertXYToPos(ih, x, y));
  return IUP_DEFAULT;
}

static int motion_cb(Ihandle *ih,int x,int y,char* status)
{
  printf("MOTION_CB(x=%d, y=%d [%s]) - [pos=%d]\n",x,y, status,IupConvertXYToPos(ih, x, y));
  return IUP_DEFAULT;
}

static int multilist_cb (Ihandle *ih, char *s)
{
  printf("MULTISELECT_CB(%s)\n", s);
  return IUP_DEFAULT;
}

static int list_cb (Ihandle *ih, char *t, int i, int v)
{
  printf("ACTION_CB[%p](%s - %d %d)\n", ih, t, i, v);
  return IUP_DEFAULT;
}

static int dblclick_cb(Ihandle *ih, int pos, char *text)
{
  printf("DBLCLICK_CB(%d - %s)\n", pos, text);
  return IUP_DEFAULT;
}

static int btok_cb(Ihandle *ih)
{
  printf("Default Enter\n");
  return IUP_DEFAULT;
}

static int btcancel_cb(Ihandle *ih)
{
  printf("Default Esc\n");
  return IUP_DEFAULT;
}

static int getvalue_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(text, "VALUE", IupGetAttribute(list, "VALUE"));
//  IupSetAttribute(list, "ACTIVE", "No");
  return IUP_DEFAULT;
}

static int setvalue_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(list, "VALUE", IupGetAttribute(text, "VALUE"));
//  IupSetAttribute(list, "ACTIVE", "Yes");
  return IUP_DEFAULT;
}

static int getcount_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(text, "VALUE", IupGetAttribute(list, "COUNT"));
  return IUP_DEFAULT;
}

static int insertitem_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(list, "INSERTITEM3", IupGetAttribute(text, "VALUE"));
  return IUP_DEFAULT;
}

static int setitem_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(list, "3", IupGetAttribute(text, "VALUE"));
  return IUP_DEFAULT;
}

static int appenditem_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(list, "APPENDITEM", IupGetAttribute(text, "VALUE"));
  return IUP_DEFAULT;
}

static int removeitem_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(list, "REMOVEITEM", IupGetAttribute(text, "VALUE"));
  return IUP_DEFAULT;
}
                   
static int topitem_cb(Ihandle *ih)
{
  Ihandle *list = (Ihandle*)IupGetAttribute(IupGetDialog(ih), "_ACTIVE_LIST");
  Ihandle *text = IupGetDialogChild(ih, "text");
  IupSetAttribute(list, "TOPITEM", IupGetAttribute(text, "VALUE"));
  return IUP_DEFAULT;
}

static void setactivelist(Ihandle* ih)
{
  Ihandle* dialog = IupGetDialog(ih);
  Ihandle* label = (Ihandle*)IupGetAttribute(dialog, "_LABEL");
  IupSetAttribute(dialog, "_ACTIVE_LIST", (char*)ih);
  IupSetAttribute(label, "TITLE", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"));
}

static int getfocus_cb(Ihandle *ih)
{
  setactivelist(ih);
  printf("GETFOCUS_CB(%s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"));
  return IUP_DEFAULT;
}

static int killfocus_cb(Ihandle *ih)
{
  printf("KILLFOCUS_CB(%s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"));
  return IUP_DEFAULT;
}

static int leavewindow_cb(Ihandle *ih)
{
  printf("LEAVEWINDOW_CB(%s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"));
  return IUP_DEFAULT;
}

static int enterwindow_cb(Ihandle *ih)
{
  setactivelist(ih);
  printf("ENTERWINDOW_CB(%s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"));
  return IUP_DEFAULT;
}

char *iupKeyCodeToName(int code);

static int k_any(Ihandle *ih, int c)
{
  if (iup_isprint(c))
    printf("K_ANY(%s, %d = %s \'%c\')\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"), c, iupKeyCodeToName(c), (char)c);
  else
    printf("K_ANY(%s, %d = %s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"), c, iupKeyCodeToName(c));
  return IUP_CONTINUE;
}

static int help_cb(Ihandle* ih)
{
  printf("HELP_CB(%s)\n", IupGetAttribute(IupGetParent(IupGetParent(ih)), "TITLE"));
  return IUP_DEFAULT;
}

static int valuechanged_cb(Ihandle *ih)
{
  printf("VALUECHANGED_CB(%p)=%s\n", ih, IupGetAttribute(ih, "VALUE"));
  return IUP_DEFAULT;
}

static void set_callbacks(Ihandle* list)
{
  IupSetCallback(list, "ACTION_CB", (Icallback)list_cb);
  IupSetCallback(list, "VALUECHANGED_CB", (Icallback)valuechanged_cb);
  IupSetCallback(list, "DBLCLICK_CB", (Icallback)dblclick_cb);
//  IupSetCallback(list, "FLAT_BUTTON_CB",    (Icallback)button_cb);
//  IupSetCallback(list, "FLAT_MOTION_CB",    (Icallback)motion_cb);

  IupSetCallback(list, "GETFOCUS_CB", getfocus_cb);
  IupSetCallback(list, "KILLFOCUS_CB", killfocus_cb);

  IupSetCallback(list, "ENTERWINDOW_CB", (Icallback)enterwindow_cb);
  IupSetCallback(list, "LEAVEWINDOW_CB", (Icallback)leavewindow_cb);

  IupSetCallback(list, "K_ANY", (Icallback)k_any);
  IupSetCallback(list, "HELP_CB", (Icallback)help_cb);
}

//#define IupFrame IupRadio

void FlatListTest(void)
{
  Ihandle *dlg, *list1, *list2, *text, *btok, *btcancel,
    *box, *lists, *buttons1, *buttons2, *label, *image1, *image2;

  image1 = IupImageRGBA(TEST_IMAGE_SIZE, TEST_IMAGE_SIZE, image_data_32);
  image2 = load_image_FileSave();

  list1 = IupFlatList();
  list2 = IupFlatList();

  IupSetAttributes(list1, "1=\"Number 1\", 2=\"Number 2\", 3=\"Number 3 Very Long\", 4=\"Number 4\", 5=\"Number 5\", 6=\"Number 6\", 7=\"Number 7\", 8=\"Number 8\", 9=\"Number 9\","
                          "NAME=list1, TIP=FlatList, VALUE=+--++--");
  IupSetAttributes(list2, "1=\"Number 1\", 2=\"Number 2\", 3=\"Number 3\", 4=\"Number 4\", 5=\"Number 5\", 6=\"Number 6 Long Long Long\", 7=\"Number 7\", 9=\"Number 9\","
                          "MULTIPLE=YES, NAME=list2, TIP=FlatList, VALUE=+--++--, XXX_CANFOCUS=NO");

  IupSetAttributeHandle(list1, "IMAGE3", image1);
  IupSetAttributeHandle(list1, "IMAGE4", image2);
  IupSetAttributeHandle(list1, "IMAGE6", image1);
  IupSetAttributeHandle(list2, "IMAGE5", image2);
  IupSetAttributeHandle(list2, "IMAGE1", image2);
  IupSetAttributeHandle(list2, "IMAGE2", image2);

  //IupSetAttributes(list1, "5=\"US$ 1000\", 6=\"US$ 2000\", 7=\"US$ 300.000.000\", 8=\"US$ 4000\"");
  //IupSetAttributes(list1, "9=\"US$ 1000\", 10=\"US$ 2000\", 11=\"US$ 300.000.000\", 12=\"US$ 4000\"");
  //IupSetAttributes(list1, "13=\"US$ 1000\", 14=\"US$ 2000\", 15=\"US$ 300.000.000\", 16=\"US$ 4000\"");
  //IupSetAttributes(list1, "17=\"US$ 1000\", 18=\"US$ 2000\", 19=\"US$ 300.000.000\", 20=\"US$ 4000\"");
  //IupSetAttributes(list1, "21=\"US$ 1000\", 22=\"US$ 2000\", 23=\"US$ 300.000.000\", 24=\"US$ 4000\"");

  set_callbacks(list1);
  set_callbacks(list2);

  //IupSetAttributes(list2, "6=\"R$ 1000\", 7=\"R$ 2000\", 8=\"R$ 123456789\", 9=\"R$ 4000\", 10=\"R$ 5000\",");
  //IupSetAttributes(list2, "11=\"R$ 1000\", 12=\"R$ 2000\", 13=\"R$ 123456789\", 14=\"R$ 4000\", 15=\"R$ 5000\",");
  //IupSetAttributes(list2, "16=\"R$ 1000\", 17=\"R$ 2000\", 18=\"R$ 123456789\", 19=\"R$ 4000\", 20=\"R$ 5000\",");
  
  IupSetCallback(list2, "MULTISELECT_CB", (Icallback)multilist_cb);

  IupSetAttribute(list1, "SHOWDRAGDROP", "Yes");

//  IupSetAttribute(list1, "DRAGDROPLIST", "Yes");
//  IupSetAttribute(list2, "DRAGDROPLIST", "Yes");
//  IupSetAttribute(list1, "DRAGSOURCE", "YES");
  //IupSetAttribute(list1, "DRAGSOURCEMOVE", "YES");
//  IupSetAttribute(list1, "DRAGTYPES", "ITEMLIST");
//  IupSetAttribute(list2, "DROPTARGET", "YES");
//  IupSetAttribute(list2, "DROPTYPES", "ITEMLIST");

//  IupSetAttribute(list1, "PADDING", "10x10");
  IupSetAttribute(list2, "PADDING", "10x10");

//  IupSetAttribute(list1, "SPACING", "10");
  IupSetAttribute(list2, "SPACING", "0");
//  IupSetAttribute(list1, "ICONSPACING", "10");

//  IupSetAttribute(list, "VISIBLECOLUMNS", "7");
//  IupSetAttribute(list, "VISIBLELINES", "4");
//  IupSetAttribute(list1, "VISIBLECOLUMNS", "3");
//  IupSetAttribute(list2, "VISIBLECOLUMNS", "3");
//  IupSetAttribute(list1, "VISIBLELINES", "0");
//  IupSetAttribute(list2, "VISIBLELINES", "0");

//  IupSetAttribute(list2, "BGCOLOR", "0 255 0");
  IupSetAttribute(list2, "ITEMBGCOLOR3", "0 255 0");
  IupSetAttribute(list2, "ITEMBGCOLOR4", "0 255 0");
  IupSetAttribute(list2, "ITEMBGCOLOR5", "0 255 0");

  IupSetAttribute(list2, "ITEMFGCOLOR4", "255 0 0");
  IupSetAttribute(list2, "ITEMFGCOLOR5", "255 0 0");
  IupSetAttribute(list2, "ITEMFGCOLOR6", "255 0 0");

  IupSetAttribute(list1, "ITEMFONTSTYLE3", "Bold");
//  IupSetAttribute(list2, "ITEMFONT4", "Times, 24");

  printf("COUNT(list1)=%s\n", IupGetAttribute(list1, "COUNT"));
  printf("COUNT(list2)=%s\n", IupGetAttribute(list2, "COUNT"));

  lists = IupHbox(
    IupSetAttributes(IupFrame(IupVbox(list1, NULL)), "TITLE=Single"),
    IupSetAttributes(IupFrame(IupVbox(list2, NULL)), "TITLE=Multiple"),
    IupVbox(
      btok = IupSetCallbacks(IupButton("Default Enter", NULL), "ACTION", btok_cb, NULL),
      btcancel = IupSetCallbacks(IupButton("Default Esc", NULL), "ACTION", btcancel_cb, NULL),
      NULL),
    NULL);

  text = IupText(NULL);
  IupSetAttribute(text, "EXPAND", "HORIZONTAL");
  IupSetAttribute(text, "NAME", "text");

  buttons1 = IupHbox(
    IupSetCallbacks(IupButton("Get(VALUE)", NULL), "ACTION", getvalue_cb, NULL),
    IupSetCallbacks(IupButton("Set(VALUE)", NULL), "ACTION", setvalue_cb, NULL),
    IupSetCallbacks(IupButton("Get(COUNT)", NULL), "ACTION", getcount_cb, NULL),
    IupSetCallbacks(IupButton("TOPITEM", NULL), "ACTION", topitem_cb, NULL),
    NULL);
  buttons2 = IupHbox(
    IupSetCallbacks(IupButton("Set Item 3", NULL), "ACTION", setitem_cb, NULL),
    IupSetCallbacks(IupButton("INSERTITEM3", NULL), "ACTION", insertitem_cb, NULL),
    IupSetCallbacks(IupButton("APPENDITEM", NULL), "ACTION", appenditem_cb, NULL),
    IupSetCallbacks(IupButton("REMOVEITEM", NULL), "ACTION", removeitem_cb, NULL),
    NULL);

  box = IupVbox(lists, IupHbox(IupLabel("Attrib. Value:  "), text, NULL), buttons1, buttons2, IupHbox(IupLabel("Current List:  "), label = IupLabel(NULL), NULL), NULL);
  IupSetAttributes(buttons1,"MARGIN=10x10, GAP=10");
  IupSetAttributes(buttons2,"MARGIN=10x10, GAP=10");
  IupSetAttributes(lists,"MARGIN=10x10, GAP=10");
  IupSetAttribute(label,"EXPAND", "HORIZONTAL");

  dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupFlatList Test");
  IupSetAttributeHandle(dlg, "DEFAULTENTER", btok);
  IupSetAttributeHandle(dlg, "DEFAULTESC", btcancel);
  IupSetAttribute(dlg, "_LABEL", (char*)label);

//  IupSetAttribute(box, "BGCOLOR", "92 92 255");
//  IupSetAttribute(dlg, "BGCOLOR", "92 92 255");
//  IupSetAttribute(dlg, "BACKGROUND", "200 10 80");
//  IupSetAttribute(dlg, "BGCOLOR", "173 177 194");  // Motif BGCOLOR for documentation

//  IupSetAttribute(dlg, "FONT", "Helvetica, 24");
//  IupSetAttribute(box, "FGCOLOR", "255 0 0");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  ListTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
