#ifndef ABI_H
#define ABI_H

#define FRAMEWORK_ABI_VERSION 1

typedef struct {
    const char *name;
    const char *description;
    const char *version;
    int abi_version;
} plugin_info_t;

#endif

