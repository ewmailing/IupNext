
/* avoid multiple inclusions */
#ifndef SIMPLE_PAINT_FILE_H
#define SIMPLE_PAINT_FILE_H

#include <stdlib.h>
#include <im.h>
#include <im_image.h>


class SimplePaintFile
{
  char* filename;
  imImage* image;

public:
  bool dirty;  /* leave it public */

  SimplePaintFile()
    :filename(NULL), image(NULL), dirty(false)
  {
  }

  void SetFilename(const char* new_filename);
  const char* GetFilename() const { return filename; }

  void SetImage(imImage* new_image, bool release = true);
  const imImage* GetImage() const { return image; }

  imImage* Read(const char* new_filename) const;
  bool Write(const char* new_filename) const;
  void SetFormat(const char* new_filename);

  bool New(int width, int height);
  void New(imImage* new_image);
  bool Open(const char* new_filename);
  bool SaveAsFile(const char* new_filename);
  bool SaveCheck();
  void SaveFile();
  void Close();
};


#endif
