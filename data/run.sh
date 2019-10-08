#!/bin/bash

# Skinning decompsition to generate bone transformations and skinning weights from input meshes sequence
../bin/DemBones -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=5 -o="Decomposition_05.fbx"
../bin/DemBones -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=10 -o="Decomposition_10.fbx"
../bin/DemBones -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=20 -o="Decomposition_20.fbx"

# Solve skinning weights from input meshes sequence and input bone transformations
../bin/DemBones -i="Bone_Trans.fbx" -a="Bone_Anim.abc" -n=5 --nTransIters=0 -o="SolvedWeights.fbx"

#Solve bone transformations from input meshes sequence and input skinning weights
../bin/DemBones -i="Bone_Skin.fbx" -a="Bone_Anim.abc" -n=5 --nWeightsIters=0 -o="SolvedTransformations.fbx"

# Optimize given bone transformations and skinning weights from input meshes sequence
../bin/DemBones -i="Bone_All.fbx" -a="Bone_Anim.abc" --bindUpdate=1 -o="Optimized.fbx"
