# Requirements:
# - Object to export must be named "mesh"
# - Baked ambient occlusion map to export must be named "ao"
import bpy, sys, pathlib
blend_file, obj_file, bmp_file = sys.argv[-3:]

bpy.ops.wm.open_mainfile(filepath=blend_file)
mesh = bpy.data.objects["mesh"]
mesh.select_set(True)
bpy.context.view_layer.objects.active = mesh

# TODO: see how it imports and modify if needed:
#   forward_axis: X, NEGATIVE_X, ...
#   up_axis
#   global_scale: 0.1, 1.0, 100.0, ...
bpy.ops.wm.obj_export(
    filepath = obj_file,
    apply_modifiers = True, # yes?
    apply_transform = False,
    export_selected_objects = True,
    export_uv = True,
    export_normals = True,
    export_materials = False,
    export_triangulated_mesh = True)

workdir = pathlib.Path().resolve()
img = bpy.data.images["ao"]
img.filepath_raw = str(workdir / bmp_file)
img.file_format = "BMP"
img.save()
