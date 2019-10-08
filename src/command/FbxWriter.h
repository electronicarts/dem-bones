///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#pragma once

#include <vector>
#include <DemBones/DemBonesExt.h>

using namespace std;
using namespace Dem;

/** Write FBX files from the model
	@param fileNames is the list of output files
	@param inputFileNames is the list of original input files, which is used to initilize the scene to get others than skinCluster-related info
	@return true if success
*/
bool writeFBXs(const vector<string>& fileNames, const vector<string>& inputFileNames, DemBonesExt<double, float>& model, bool embedMedia=true);
