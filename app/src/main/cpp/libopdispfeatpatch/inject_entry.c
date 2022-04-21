#ifndef DISPLAY_TUNER_VERSION
#error Please define macro DISPLAY_TUNER_VERSION in CMakeLists.txt
#endif

#include <stddef.h>
#include <errno.h>

#include "../libbasehalpatch/ipc/inject_io_init.h"

__attribute__((used, section("DISPLAY_TUNER_VERSION"), visibility("default")))
const char g_display_tuner_version[] = DISPLAY_TUNER_VERSION;

// called by daemon with ptrace
__attribute__((noinline, visibility("default")))
void *op_disp_feature_patch_inject_init(int fd) {
    (void) g_display_tuner_version;
    if (fd < 0) {
        return (void *) -EINVAL;
    }
    if (initElfHeaderInfo("libopdispfeatpatch.so", &op_disp_feature_patch_inject_init) == 0) {
        return (void *) -EBADE;
    }
    return (void *) (size_t) BaseHalPatchInitSocket(fd);
}
