#!/bin/bash

if [ "$OSTYPE" == "linux-gnu".* ]; then
    EXECUTION_FILE="../bin/Linux/DemBones"
elif [ "$OSTYPE" == "darwin17" ]; then 
    EXECUTION_FILE="../bin/MacOS/DemBones"
fi

# Skinning decompsition to generate bone transformations and skinning weights from input meshes sequence
$EXECUTION_FILE -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=5 -o="Decomposition_05.fbx"
$EXECUTION_FILE -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=10 -o="Decomposition_10.fbx"
$EXECUTION_FILE -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=20 -o="Decomposition_20.fbx"

# Solve skinning weights from input meshes sequence and input bone transformations
$EXECUTION_FILE -i="Bone_Trans.fbx" -a="Bone_Anim.abc" -n=5 --nTransIters=0 -o="SolvedWeights.fbx"

#Solve bone transformations from input meshes sequence and input skinning weights
$EXECUTION_FILE -i="Bone_Skin.fbx" -a="Bone_Anim.abc" -n=5 --nWeightsIters=0 -o="SolvedTransformations.fbx"

# Optimize given bone transformations and skinning weights from input meshes sequence
$EXECUTION_FILE -i="Bone_All.fbx" -a="Bone_Anim.abc" --bindUpdate=1 -o="Optimized.fbx"

# Only solve helper bones using demLock attribute of the joints
$EXECUTION_FILE -i="Bone_Helpers.fbx" -a="Bone_Anim.abc" --bindUpdate=1 -o="SolvedHelpers.fbx"

# Partially solve skinning weights using per-vertex color attribute of the mesh
$EXECUTION_FILE -i="Bone_PartiallySkinned.fbx" -a="Bone_Anim.abc" --nTransIters=0 -o="SolvedPartiallyWeights.fbx"
