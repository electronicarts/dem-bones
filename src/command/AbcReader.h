///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#pragma once

#include <vector>
#include <DemBones/DemBonesExt.h>

using namespace std;
using namespace Dem;

/** Read Alembic cache files and set: model.u, model.fStart, model.subjectID, model.nV, model.nS, model.nF, model.fTime
	@return true if success
*/
bool readABCs(const vector<string>& fileNames, DemBonesExt<double, float>& model);
