#include <CoreFoundation/CoreFoundation.h>
#include <QuickLook/QuickLook.h>
#include "errors.h"
#include "image.h"
#include "io.h"

bool is_type_supported(const char *type) {
    if (!type)
        return false;
    CFArrayRef supported_types = CGImageDestinationCopyTypeIdentifiers();
    CFStringRef type_str = CFStringCreateWithCString(NULL, type, kCFStringEncodingUTF8);
    bool is_supported = CFArrayContainsValue(supported_types, CFRangeMake(0, CFArrayGetCount(supported_types)), type_str);

    CFRelease(type_str);
    CFRelease(supported_types);
    return is_supported;
}

int save_icon(const char *filename, image_func fn, const char *icon_name, const char *type, unsigned int width, unsigned int height, float compression_quality, bool release_image_immediately) {
    int res = 0;
    CFStringRef type_str = NULL;
    CFStringRef icon_str = NULL;
    CFURLRef icon_url = NULL;
    CGImageRef image = NULL;
    CGImageDestinationRef icon = NULL;
    CFDictionaryRef properties  = NULL;
    CFStringRef property_names[1];
    CFTypeRef property_values[1];

    property_names[0] = kCGImageDestinationLossyCompressionQuality;
    property_values[0] = CFNumberCreate(NULL, kCFNumberFloatType, &compression_quality);
    properties = CFDictionaryCreate(NULL, (const void **)property_names, (const void **)property_values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    type_str = CFStringCreateWithCString(NULL, type, kCFStringEncodingUTF8);

    icon_str = CFStringCreateWithCString(NULL, icon_name, kCFStringEncodingUTF8);
    icon_url = CFURLCreateWithFileSystemPath(NULL, icon_str, kCFURLPOSIXPathStyle, 0);

    image = fn(filename, width, height);
    if (!image) {
        res = ERROR_GENERATING_IMAGE;
        goto cleanup;
    }

    CFMutableDataRef data = NULL;

    if (strlen(icon_name) == 0) {
        data = CFDataCreateMutable(kCFAllocatorDefault, 0);
        icon = CGImageDestinationCreateWithData(data, type_str, 1, NULL);
    } else {
        icon = CGImageDestinationCreateWithURL(icon_url, type_str, 1, NULL);
    }

    if (!icon) {
        res = ERROR_CREATING_IMAGE_DESTINATION;
        goto cleanup;
    }

    CGImageDestinationAddImage(icon, image, properties);
    if (!CGImageDestinationFinalize(icon)) {
        res = ERROR_FINALIZING_IMAGE;
        goto cleanup;
    }

    if (strlen(icon_name) == 0) {
        CFIndex len = CFDataGetLength(data);
        long written = fwrite(CFDataGetBytePtr(data), 1, len, stdout);
        if (written < len) {
            res = ERROR_WRITING_OUTPUT;
            goto cleanup;
        }
        if (data) CFRelease(data);
    }

cleanup:
    if (property_values[0]) CFRelease(property_values[0]);
    if (properties) CFRelease(properties);
    if (type_str) CFRelease(type_str);
    if (icon_str) CFRelease(icon_str);
    if (icon_url) CFRelease(icon_url);
    if (release_image_immediately && image) CFRelease(image);
    if (icon) CFRelease(icon);
    return res;
}
