#define CSM_IMPLEMENTATION
#define CSM_MODULE_MATH
#define CSM_MODULE_MESH
#include "csm_core/core.h"

#include "blender_export.c"

char fname_buf[4096];
char file_buf[1280000];
char blend_cmd[] = "blender --background --python tmp.py -- %s.blend %s.obj %s.bmp";

// make obj.h and bmp.h in core, getting into a happy helpers habit.

i32 main(i32 argc, char** argv) {
    // =====================================================
    // Process .blend into .obj and .bmp files
    // =====================================================
    FILE* py_file = fopen("tmp.py", "w");
    assert(py_file != NULL);
    fwrite(code_model_blender_export_py, code_model_blender_export_py_len, 1, py_file);
    fclose(py_file);

    char* asset_name = argv[1];
    char cmd_buf[4096];
    sprintf(cmd_buf, blend_cmd, asset_name, asset_name, asset_name);
    system(cmd_buf);

    // =====================================================
    // Process .obj into mesh file
    // =====================================================

    sprintf(fname_buf, "%s.obj", asset_name);
    FILE* obj_file = fopen(fname_buf, "r");
    assert(obj_file != NULL);
    char c;
    i32 i = 0;
    while((c = fgetc(obj_file)) != EOF) {
        file_buf[i] = c;
        i++;
    }
    file_buf[i] = '\0';
    fclose(obj_file);

    MeshData mesh_data = mesh_from_obj(file_buf);

    sprintf(fname_buf, "%s.mesh", asset_name);
    FILE* mesh_file = fopen(fname_buf, "w");
    assert(mesh_file != NULL);
    fwrite(&mesh_data, sizeof(u32) + sizeof(MeshVertexData) * mesh_data.vertices_len, 1, mesh_file);
    fclose(mesh_file);

    // =====================================================
    // Process .bmp into texture file
    // =====================================================
    
    return 0;
}
