#include <CoreFoundation/CoreFoundation.h>
#include <QuickLook/QuickLook.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "errors.h"
#include "io.h"
#include "ui.h"

int main(int argc, char** argv) {
    struct options opts;
    int res = get_options(argc, argv, &opts);
    if (res != 0) {
        return res;
    }

    res = save_icon(opts.input, ql_image_for_file, opts.output, opts.type, opts.width, opts.height, opts.quality, true);
    if (res != 0) {
        fatal("failed to generate icon", res);
    }

    free(opts.input);
    if (strlen(opts.output) > 0)
        free(opts.output);
    if (opts.type_specified)
        free(opts.type);

    return EXIT_SUCCESS;
}
