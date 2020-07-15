rem Skinning decompsition to generate bone transformations and skinning weights from input meshes sequence
call "../bin/Windows/DemBones.exe" -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=5 -o="Decomposition_05.fbx"
call "../bin/Windows/DemBones.exe" -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=10 -o="Decomposition_10.fbx"
call "../bin/Windows/DemBones.exe" -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=20 -o="Decomposition_20.fbx"

rem Joint grouping 
call "../bin/Windows/DemBones.exe" -i="Bone_Geom.fbx" -a="Bone_Anim.abc" -b=20 --bindUpdate=2 -o="Decomposition_20_grouped.fbx"

rem Solve skinning weights from input meshes sequence and input bone transformations
call "../bin/Windows/DemBones.exe" -i="Bone_Trans.fbx" -a="Bone_Anim.abc" --nTransIters=0 -o="SolvedWeights.fbx"

rem Solve bone transformations from input meshes sequence and input skinning weights
call "../bin/Windows/DemBones.exe" -i="Bone_Skin.fbx" -a="Bone_Anim.abc" --nWeightsIters=0 -o="SolvedTransformations.fbx"

rem Optimize given bone transformations and skinning weights from input meshes sequence
call "../bin/Windows/DemBones.exe" -i="Bone_All.fbx" -a="Bone_Anim.abc" --bindUpdate=1 -o="Optimized.fbx"

rem Only solve helper bones using demLock attribute of the joints
call "../bin/Windows/DemBones.exe" -i="Bone_Helpers.fbx" -a="Bone_Anim.abc" --bindUpdate=1 -o="SolvedHelpers.fbx"

rem Partially solve skinning weights using per-vertex color attribute of the mesh
call "../bin/Windows/DemBones.exe" -i="Bone_PartiallySkinned.fbx" -a="Bone_Anim.abc" --nTransIters=0 -o="SolvedPartialWeights.fbx"
