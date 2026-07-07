#define CSM_IMPLEMENTATION
#define CSM_SKIP_MATH
#include "csm_core/core.h"

char blend_cmd[] = "blender --background --python code/prebuild/blender_export.py -- assets/blend/%s.blend assets/obj/%s.obj assets/bmp/%s.bmp";

i32 main(i32 argc, char** argv) {
    printf("Hello!\n");
    return 0;
}
