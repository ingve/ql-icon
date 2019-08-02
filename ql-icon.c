#include <CoreFoundation/CoreFoundation.h>
#include <QuickLook/QuickLook.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define ERROR_GENERATING_ICON 1
#define ERROR_IMAGE_TYPE_NOT_SUPPORTED 2
#define ERROR_INVALID_PARAMETER 3
#define ERROR_MISSING_REQUIRED_PARAMETER 4
#define ERROR_GENERATING_IMAGE 5
#define ERROR_CREATING_IMAGE_DESTINATION 6
#define ERROR_FINALIZING_IMAGE 7
#define ERROR_WRITING_OUTPUT 8
#define ERROR_INTERNAL_ERROR 127

void usage(const char *binary_name) {
    fprintf(stderr, "%s [options]\n", binary_name);
}

void fatal(const char *message, int retval) {
    fprintf(stderr, "error: %s\n", message);
    exit(retval);
}

void list_image_types() {
    CFArrayRef supported_types = CGImageDestinationCopyTypeIdentifiers();
    for (int i = 0; i < CFArrayGetCount(supported_types); i++) {
        CFStringRef type = CFArrayGetValueAtIndex(supported_types, i);
        CFIndex len = CFStringGetLength(type);
        CFIndex c_str_len = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8) + 1;
        char *buffer = (char *)malloc(c_str_len);
        if (CFStringGetCString(type, buffer, c_str_len, kCFStringEncodingUTF8)) {
            printf("%s\n", buffer);
        }
        free(buffer);
    }
    CFRelease(supported_types);
}

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

CGImageRef image_for_file(const char *filename, unsigned int width, unsigned int height) {
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

int save_icon(const char *filename, const char *icon_name, const char *type, unsigned int width, unsigned int height, float compression_quality) {
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
    
    image = image_for_file(filename, width, height);
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
    if (image) CFRelease(image);
    if (icon) CFRelease(icon);
    return res;
}

int main(int argc, char** argv) {
    
    static struct option longopts[] = {
        { "width", required_argument, NULL, 'w' },
        { "height", required_argument, NULL, 'h' },
        { "output", required_argument, NULL, 'o' },
        { "input", required_argument, NULL, 'i' },
        { "quality", required_argument, NULL, 'q' },
        { "type", required_argument, NULL, 't' },
        { "listtypes", no_argument, NULL, 'l' },
        { "verbose", no_argument, NULL, 'v' },
        { NULL, 0, NULL, 0 }
    };

    /* defaults */
    int width = 512;
    int height = 512;
    char *input = NULL;
    char *output = NULL;
    char *type = "public.png";
    bool type_specified = false;
    float quality = 1.0f;
    bool verbose = false;

    int ch;
    char *binary_name;
    while ((ch = getopt_long_only(argc, argv, "w:h:o:i:q:t:v:", longopts, NULL)) != -1)
        switch (ch) {
        case 'w':
            errno = 0;
            width = strtol(optarg, NULL, 10);
            if (errno) 
                fatal("invalid width", ERROR_INVALID_PARAMETER);
            break;
        case 'h':
            errno = 0;
            height = strtol(optarg, NULL, 10);
            if (errno) 
                fatal("invalid height", ERROR_INVALID_PARAMETER);
            break;
        case 'o':
            output = strdup(optarg);
            break;
        case 'i':
            input = strdup(optarg);
            break;
        case 'q':
            errno = 0;
            quality = strtof(optarg, NULL);
            if (errno) 
                fatal("invalid quality", ERROR_INVALID_PARAMETER);
            break;
        case 't':
            type = strdup(optarg);
            type_specified = true;
            if (!is_type_supported(type))
                fatal("image type not supported", ERROR_IMAGE_TYPE_NOT_SUPPORTED);
            break;
        case 'l':
            list_image_types();
            return EXIT_SUCCESS;
        case 'v':
            verbose = true;
            break;
        default:
            binary_name = strdup(argv[0]);
            if (!binary_name) 
                fatal("out of memory", ERROR_INTERNAL_ERROR);
            usage(basename(binary_name));
            free(binary_name);
            return EXIT_FAILURE;
    }
    argc -= optind;
    argv += optind;

    if (!input)
        fatal("input file name must be specified", ERROR_MISSING_REQUIRED_PARAMETER);
    if (!output)
        output = "";
    if (verbose)
        fprintf(stderr, "width: %d, height: %d, input: %s, output: %s, type: %s, quality: %.2f\n", width, height, input, output, type, quality);

    int res = save_icon(input, output, type, width, height, quality);
    if (res < 0) {
        fatal("failed to generate icon", ERROR_GENERATING_ICON);
    }

    free(input);
    if (strlen(output) > 0)
        free(output);
    if (type_specified)
        free(type);
    
    return EXIT_SUCCESS;
}
