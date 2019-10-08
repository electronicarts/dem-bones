///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#include "FbxShared.h"

FbxSceneShared::FbxSceneShared(bool importAnim) {
	// Initialize the SDK manager. This object handles memory management.
	lSdkManager=FbxManager::Create();
	// Create the IO settings object.
	ios=FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);
	// Import animation?
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, importAnim);
	// Create an importer using the SDK manager.
	lImporter=FbxImporter::Create(lSdkManager, "");
	// Create a new scene so that it can be populated by the imported file.
	lScene=FbxScene::Create(lSdkManager, "myScene");
}

FbxSceneShared::~FbxSceneShared() {
	lScene->Destroy();
	lImporter->Destroy();
	ios->Destroy();
	lSdkManager->Destroy();
}


bool FbxSceneShared::open(const string& fileName) {
	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(fileName.c_str(), -1, lSdkManager->GetIOSettings())) return false;

	// Clear the scene
	lScene->Clear();

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	return true;
}

FbxMesh* FbxSceneShared::firstMesh(FbxNode* pNode) {
	for (int i=0; i<pNode->GetNodeAttributeCount(); i++)
		if ((pNode->GetNodeAttributeByIndex(i)->GetAttributeType()==FbxNodeAttribute::eMesh)) return (FbxMesh*)pNode->GetNodeAttributeByIndex(i);

	for (int j=0; j<pNode->GetChildCount(); j++) {
		FbxMesh* pMesh=firstMesh(pNode->GetChild(j));
		if (pMesh!=NULL) return pMesh;
	}

	return NULL;
}

FbxSkin* FbxSceneShared::firstSkin(FbxMesh* pMesh) {
	for (int i=0; i<pMesh->GetDeformerCount(); i++) {
		FbxDeformer* pDeformer=pMesh->GetDeformer(i);
		if ((pDeformer->GetDeformerType()==FbxDeformer::eSkin)&&
			(((FbxSkin*)pDeformer)->GetClusterCount()>0)) return (FbxSkin*)pDeformer;
	}

	return NULL;
}