#ifndef IMAGE_H
#define IMAGE_H

#include <QuickLook/QuickLook.h>

typedef CGImageRef (*image_func)(const char *, unsigned int, unsigned int);

CGImageRef ql_image_for_file(const char *filename, unsigned int width, unsigned int height);

#endif /* IMAGE_H */
