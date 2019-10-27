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
    # use_triangles = triangulate. Bullet expects triangle meshes
    bpy.ops.export_scene.obj(filepath=outputFilename, check_existing=False, use_mesh_modifiers=True, use_triangles=True)

def exportHordeCollada():
    outputFilename = makeRelativeFilename("dae")
    bpy.ops.wm.collada_export(filepath=outputFilename, check_existing=False, apply_modifiers=True, triangulate=True)

exportBulletObj()
exportHordeCollada()
