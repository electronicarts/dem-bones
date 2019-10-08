///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#pragma once

#include <string>
#include <fbxsdk.h>

using namespace std;

class FbxSceneShared {
public:
	FbxSceneShared(bool importAnim=true);
	~FbxSceneShared();
	bool open(const string& fileName);
	FbxMesh* firstMesh(FbxNode* pNode);
	FbxSkin* firstSkin(FbxMesh* pMesh);

protected:
	FbxManager* lSdkManager;
	FbxIOSettings* ios;
	FbxImporter* lImporter;
	FbxScene* lScene;
};
