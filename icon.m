#include <AppKit/AppKit.h>

#include "image.h"
#include "io.h"
#include "errors.h"
#include "ui.h"

CGImageRef iconForFile(const char *path, unsigned int width, unsigned int height) {
    NSURL *url = [NSURL fileURLWithPath:[NSString stringWithCString:path encoding:NSUTF8StringEncoding]];
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    NSImage *icon = [workspace iconForFile:url.path];
    if (!icon) {
        return NULL;
    }
    NSRect rect = NSMakeRect(0, 0, width, height);
    CGImageRef image = [icon CGImageForProposedRect:&rect context:nil hints:nil];
    if (!image) {
        return NULL;
    }
    return image;
}

int main(int argc, char **argv) {
    @autoreleasepool {
        struct options opts;
        int res = get_options(argc, argv, &opts);
        if (res != 0) {
            fatal("error getting options", res);
        }
        NSString *path = [NSString stringWithCString:opts.input encoding:NSUTF8StringEncoding];
        NSString *src = [path stringByStandardizingPath];
        const char *filename = [src UTF8String];

        res = save_icon(filename, ql_image_for_file, opts.output, opts.type, opts.width, opts.height, opts.quality, true);
        if (res != 0) {
            res = save_icon(filename, iconForFile, opts.output, opts.type, opts.width, opts.height, opts.quality, false);
            if (res != 0) {
                fatal("error saving image", res);
            }
        }

        free(opts.input);
        if (strlen(opts.output) > 0)
            free(opts.output);
        if (opts.type_specified)
            free(opts.type);
    }
    return EXIT_SUCCESS;
}


