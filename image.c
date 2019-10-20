#include "image.h"

CGImageRef ql_image_for_file(const char *filename, unsigned int width, unsigned int height) {
    CGImageRef image = NULL;
    CFStringRef file_str = NULL;
    CFURLRef file_url = NULL;

    CFDictionaryRef options = NULL;
    CFStringRef options_names[1];
    CFTypeRef options_values[1];
    options_names[0] = kQLThumbnailOptionIconModeKey;
    options_values[0] = kCFBooleanTrue;
    options = CFDictionaryCreate(NULL, (const void **)options_names, (const void **)options_values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CGSize size = CGSizeMake(width, height);

    file_str = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    file_url = CFURLCreateWithFileSystemPath(NULL, file_str, kCFURLPOSIXPathStyle, 0);

    image = QLThumbnailImageCreate(kCFAllocatorDefault, file_url, size, options);

    if (options) CFRelease(options);
    if (file_str) CFRelease(file_str);
    if (file_url) CFRelease(file_url);

    return image;
}
