#ifndef IO_H
#define IO_H

#include "image.h"

bool is_type_supported(const char *type);
int save_icon(const char *filename, image_func fn, const char *icon_name, const char *type, unsigned int width, unsigned int height, float compression_quality, bool release_image_immediately);

#endif /* IO_H */
