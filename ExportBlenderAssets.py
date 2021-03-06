# This script was last tested working with Blender 2.8

import bpy

blendPath = bpy.context.blend_data.filepath

def makeRelativeFilename(newExtension):
    return "{}.{}".format(blendPath[:blendPath.rfind('.')], newExtension)

def exportBulletObj():
    # Output in same directory as blend, with .obj extension
    outputFilename = makeRelativeFilename("obj")
    # print(outputFilename)

    # use_mesh_modifiers = apply modifiers
    # use_materials = (false) Don't export materials. We only use the .objs for collision
    # use_triangles = triangulate. Bullet expects triangle meshes
    # https://docs.blender.org/api/current/bpy.ops.export_scene.html?highlight=export_scene#bpy.ops.export_scene.obj
    bpy.ops.export_scene.obj(filepath=outputFilename, check_existing=False,
                             use_mesh_modifiers=True, use_materials=False, use_triangles=True)

def exportHordeCollada():
    outputFilename = makeRelativeFilename("dae")
    # https://docs.blender.org/api/current/bpy.ops.wm.html?highlight=collada#bpy.ops.wm.collada_export
    bpy.ops.wm.collada_export(filepath=outputFilename, check_existing=False, apply_modifiers=True, triangulate=True)

exportBulletObj()
exportHordeCollada()
