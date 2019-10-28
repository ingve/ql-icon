#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <QuickLook/QuickLook.h>
#include "errors.h"
#include "io.h"
#include "ui.h"

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
            fprintf(stderr, "%s\n", buffer);
        }
        free(buffer);
    }
    if(supported_types) CFRelease(supported_types);
}

int get_options(int argc, char **argv, struct options *options) {
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
    options->width = 512;
    options->height = 512;
    options->input = NULL;
    options->output = NULL;
    options->type = "public.png";
    options->type_specified = false;
    options->quality = 1.0f;
    options->verbose = false;

    int ch;
    char *binary_name;
    while ((ch = getopt_long_only(argc, argv, "w:h:o:i:q:t:v:", longopts, NULL)) != -1)
        switch (ch) {
        case 'w':
            errno = 0;
            options->width = strtol(optarg, NULL, 10);
            if (errno)
                fatal("invalid width", ERROR_INVALID_PARAMETER);
            break;
        case 'h':
            errno = 0;
            options->height = strtol(optarg, NULL, 10);
            if (errno)
                fatal("invalid height", ERROR_INVALID_PARAMETER);
            break;
        case 'o':
            options->output = strdup(optarg);
            break;
        case 'i':
            options->input = strdup(optarg);
            break;
        case 'q':
            errno = 0;
            options->quality = strtof(optarg, NULL);
            if (errno)
                fatal("invalid quality", ERROR_INVALID_PARAMETER);
            break;
        case 't':
            options->type = strdup(optarg);
            options->type_specified = true;
            if (!is_type_supported(options->type))
                fatal("image type not supported", ERROR_IMAGE_TYPE_NOT_SUPPORTED);
            break;
        case 'l':
            list_image_types();
            return ERROR_NOTHING_TO_GENERATE;
        case 'v':
            options->verbose = true;
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

    if (!options->input)
        fatal("input file name must be specified", ERROR_MISSING_REQUIRED_PARAMETER);
    if (!options->output)
        options->output = "";
    if (options->verbose)
        fprintf(stderr, "width: %d, height: %d, input: %s, output: %s, type: %s, quality: %.2f\n", options->width, options->height, options->input, options->output, options->type, options->quality);

    return 0;
}
