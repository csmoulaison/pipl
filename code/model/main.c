#define CSM_IMPLEMENTATION
#define CSM_MODULE_MATH
#define CSM_MODULE_MESH
#define CSM_MODULE_TEXTURE
#include "csm_core/core.h"

#include "blender_export.c"

char fname_buf[4096];
char file_buf[64000000];
char blend_cmd[] = "blender --background --python tmp.py -- %s.blend %s.obj %s.bmp";
TextureData tex_data;

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

    StringWriter fname_writer = string_writer_from_memory(fname_buf, sizeof(fname_buf));
    string_write_cstring(&fname_writer, asset_name);
    string_write_cstring(&fname_writer, ".bmp");
    File file = file_open(&fname_writer.string, FILE_OPEN_READ);
    StringWriter file_writer = string_writer_from_memory(file_buf, sizeof(file_buf));
    string_write_file(&file, &file_writer);
    file_close(&file);

    texture_from_bmp(&file_writer.string, &tex_data);

    string_writer_clear(&fname_writer);
    string_write_cstring(&fname_writer, asset_name);
    string_write_cstring(&fname_writer, ".texture");

    char buf[4096];
    string_to_cstring(&fname_writer.string, buf, 4096);
    printf("filename %s\n", buf);
    file = file_open(&fname_writer.string, FILE_OPEN_WRITE);
    file_write(&file, 
        &tex_data, sizeof(u64) * 2 
        + sizeof(PixelData) * tex_data.height * tex_data.width);
    file_close(&file);
    
    return 0;
}
