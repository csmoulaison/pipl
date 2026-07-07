#define CSM_IMPLEMENTATION
#define CSM_SKIP_MATH
#include "csm_core/core.h"

#include "blender_export.c"

char blend_cmd[] = "blender --background --python tmp.py -- %s.blend %s.obj %s.bmp";

i32 main(i32 argc, char** argv) {
    FILE* py = fopen("tmp.py", "w");
    fwrite(code_model_blender_export_py, code_model_blender_export_py_len, 1, py);
    fclose(py);

    char* asset_name = argv[1];
    char cmd_buf[4096];
    sprintf(cmd_buf, blend_cmd, asset_name, asset_name, asset_name);
    system(cmd_buf);
    return 0;
}
