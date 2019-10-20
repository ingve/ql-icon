#ifndef UI_H
#define UI_H

struct options {
    int width;
    int height;
    char *input;
    char *output;
    char *type;
    bool type_specified;
    float quality;
    bool verbose;
};

void usage(const char *binary_name);
void fatal(const char *message, int retval);
void list_image_types();
int get_options(int argc, char **argv, struct options *options);

#endif /* UI_H */
