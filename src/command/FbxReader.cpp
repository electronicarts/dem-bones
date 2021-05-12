///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#include "FbxReader.h"
#include "LogMsg.h"
#include "FbxShared.h"
#include <Eigen/Dense>
#include <map>
#include <DemBones/MatBlocks.h>

using namespace std;
using namespace Eigen;

#define err(msgStr) {msg(1, msgStr); return false;}

class FbxSceneImporter: public FbxSceneShared {
public:
	MatrixXd v;
	vector<vector<int>> fv;

	vector<string> jointName;
	map<string, string> parent;
	map<string, VectorXd, less<string>, aligned_allocator<pair<const string, VectorXd>>> wT;
	map<string, Matrix4d, less<string>, aligned_allocator<pair<const string, Matrix4d>>> bind, preMulInv;
	map<string, Vector3i, less<string>, aligned_allocator<pair<const string, Vector3i>>> rotOrder;
	map<string, Vector3d, less<string>, aligned_allocator<pair<const string, Vector3d>>> orient;
	map<string, MatrixXd, less<string>, aligned_allocator<pair<const string, MatrixXd>>> m;
	map<string, int> lockM;
	VectorXd lockW;
	bool hasKeyFrame;

	//http://help.autodesk.com/view/FBX/2019/ENU/?guid=FBX_Developer_Help_getting_started_your_first_fbx_sdk_program_html
	bool load(const VectorXd& fTime) {
		//Get mesh
		FbxMesh* pMesh=firstMesh(lScene->GetRootNode());

		//Scence mush have at least one mesh
		if (pMesh==NULL) err("Scene has no mesh.\n");
	
		int nV=(int)pMesh->GetControlPointsCount();
		FbxVector4* cp=pMesh->GetControlPoints();
		v.resize(3, nV);
		for (int i=0; i<nV; i++) v.col(i)<<cp[i][0], cp[i][1], cp[i][2];

		FbxAMatrix gMat=pMesh->GetNode()->EvaluateGlobalTransform();

		int nFV=pMesh->GetPolygonCount();
		int* idx=pMesh->GetPolygonVertices();

		fv.resize(nFV);
		for (int i=0; i<nFV; i++) {
			int* begin=idx+pMesh->GetPolygonVertexIndex(i);
			int* end=begin+pMesh->GetPolygonSize(i);
			fv[i].assign(begin, end);
		}

		//http://docs.autodesk.com/FBX/2014/ENU/FBX-SDK-Documentation/index.html?url=cpp_ref/class_fbx_geometry_base.html,topicNumber=cpp_ref_class_fbx_geometry_base_html77b8b88d-2e98-42bc-8505-6c20a2debe66
		lockW=VectorXd::Zero(nV);
		if (pMesh->GetElementVertexColorCount()>0) {
			FbxGeometryElementVertexColor* leVtxc=pMesh->GetElementVertexColor(0);
			if (leVtxc->GetMappingMode()==FbxGeometryElement::eByPolygonVertex) {
				VectorXi slw=VectorXi::Zero(nV);
				
				switch (leVtxc->GetReferenceMode()) {
					case FbxGeometryElement::eDirect: {
						int count=0;
						for (int i=0; i<nFV; i++) 
							for (int k=0; k!=fv[i].size(); k++) {
								int vid=fv[i][k];
								lockW(vid)+=grayScale(leVtxc->GetDirectArray().GetAt(count++));
								slw(vid)++;
							}
					} break;
					case FbxGeometryElement::eIndexToDirect: {
						int count=0;
						for (int i=0; i<nFV; i++)
							for (int k=0; k!=fv[i].size(); k++) {
								int vid=fv[i][k];
								int id=leVtxc->GetIndexArray().GetAt(count++);
								lockW(vid)+=grayScale(leVtxc->GetDirectArray().GetAt(id));
								slw(vid)++;
							}
					} break;
				}

				for (int i=0; i<nV; i++) 
					if (slw(i)>0) lockW(i)/=(double)slw(i);
			}
		}

		jointName.clear();
		parent.clear();
		wT.clear();
		bind.clear();
		preMulInv.clear();
		m.clear();
		lockM.clear();
		hasKeyFrame=false;

		//Get skinCluster
		FbxSkin* pSkin=firstSkin(pMesh);
		int nB=0;

		//Get skinning weights (if skinCluster exists)
		if (pSkin!=NULL) {
			nB=(int)pSkin->GetClusterCount();

			//Indexing by the order in the skinCluster
			jointName.resize(nB);
			for (int j=0; j<nB; j++) jointName[j]=pSkin->GetCluster(j)->GetLink()->GetName();
	
			for (int j=0; j<nB; j++) {
				FbxCluster* pCluster=pSkin->GetCluster(j);
				
				double* val=pCluster->GetControlPointWeights();
				int* idx=pCluster->GetControlPointIndices();
				
				int nj=pCluster->GetControlPointIndicesCount();
				wT[jointName[j]]=VectorXd::Zero(nV);
				for (int k=0; k<nj; k++) {
					if (idx[k]>nV) return false;
					wT[jointName[j]](idx[k])=val[k];
				}
			}

			for (int j=1; j<nB; j++) {
				FbxAMatrix mat1, mat2;
				pSkin->GetCluster(j-1)->GetTransformMatrix(mat1);
				pSkin->GetCluster(j)->GetTransformMatrix(mat2);
				if ((Map<Matrix4d>((double*)(mat1))-Map<Matrix4d>((double*)(mat2))).squaredNorm()>1e-10) err("Multiple bind poses.\n");
			}
			pSkin->GetCluster(0)->GetTransformMatrix(gMat);			
		}

		Matrix4d gm=Map<Matrix4d>((double*)(gMat));
		v=(gm*v.colwise().homogeneous()).topRows<3>();
	
		// Load skeleton (if exists)
		vector<JointNode> jn(0);
		travel(lScene->GetRootNode(), NULL, jn);

		// No joint
		if ((nB==0)&&(jn.size()==0)) return true;

		if ((nB!=0)&&(jn.size()!=nB)) err("Scene has more joints than skinCluster has: "<<jn.size()<<"/"<<nB<<".\n");

		// No skinCluster, indexing by the DFS travel order in scene
		if (nB==0) {
			nB=(int)jn.size();
			jointName.resize(nB);
			for (int j=0; j<nB; j++) jointName[j]=jn[j].pNode->GetName();
		}

		for (int j=0; j<nB; j++) {
			string name=jn[j].pNode->GetName();
			parent[name]=(jn[j].pParentJoint==NULL)?"":jn[j].pParentJoint->GetName();
			bind[name]=Map<Matrix4d>((double*)(jn[j].pNode->EvaluateGlobalTransform()));

			EFbxRotationOrder lRotationOrder;
			jn[j].pNode->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrder);
			switch (lRotationOrder) {
				case eEulerXYZ: rotOrder[name]=Vector3i(0, 1, 2); break;
				case eEulerXZY: rotOrder[name]=Vector3i(0, 2, 1); break;
				case eEulerYZX: rotOrder[name]=Vector3i(1, 2, 0); break;
				case eEulerYXZ: rotOrder[name]=Vector3i(1, 0, 2); break;
				case eEulerZXY: rotOrder[name]=Vector3i(2, 0, 1); break;
				case eEulerZYX: rotOrder[name]=Vector3i(2, 1, 0); break;
			}

			FbxDouble3 oj=jn[j].pNode->PreRotation.Get();
			orient[name]<<oj[0], oj[1], oj[2];

			if (jn[j].pNode->GetParent()!=jn[j].pParentJoint) {
				Matrix4d gp=Map<Matrix4d>((double*)(jn[j].pNode->GetParent()->EvaluateGlobalTransform()));
				if (jn[j].pParentJoint==NULL) preMulInv[name]=gp.inverse(); else {
					Matrix4d gjp=Map<Matrix4d>((double*)(jn[j].pParentJoint->EvaluateGlobalTransform()));
					preMulInv[name]=gp.inverse()*gjp;
				}
			} else preMulInv[name]=Matrix4d::Identity();

			FbxProperty att=jn[j].pNode->FindProperty("demLock", false);
			if (att.IsValid()&&(att.GetPropertyDataType()==FbxBoolDT)&&att.Get<bool>()) lockM[name]=1; else lockM[name]=0;
		}

		int nFr=(int)fTime.size();
		for (int j=0; j<nB; j++) {
			if ((jn[j].pNode->LclRotation.GetCurveNode()!=NULL)||(jn[j].pNode->LclTranslation.GetCurveNode()!=NULL)) hasKeyFrame=true;
			string name=jn[j].pNode->GetName();
			m[name].resize(4*nFr, 4);
			for (int k=0; k<nFr; k++) {
				FbxTime tk;
				tk.SetSecondDouble(fTime(k));
				m[name].blk4(k, 0)=Map<Matrix4d>((double*)(jn[j].pNode->EvaluateGlobalTransform(tk)))*bind[name].inverse();
			}
		}

		return true;
	}

private:
	struct JointNode {
		FbxNode* pNode;
		FbxNode* pParentJoint;
		JointNode(FbxNode* pn=NULL, FbxNode* pp=NULL):pNode(pn), pParentJoint(pp) {}
	};

	void travel(FbxNode* pNode, FbxNode* pParentJoint, vector<JointNode>& jn) {
		for (int i=0; i<pNode->GetNodeAttributeCount(); i++)
			if (pNode->GetNodeAttributeByIndex(i)->GetAttributeType()==FbxNodeAttribute::eSkeleton) {
				jn.push_back(JointNode(pNode, pParentJoint));
				pParentJoint=pNode;
				break;
			}

		for (int j=0; j<pNode->GetChildCount(); j++)
			travel(pNode->GetChild(j), pParentJoint, jn);
	}

	double grayScale(const FbxColor& c) {
		return 0.2989*c.mRed+0.5870*c.mGreen+0.1140*c.mBlue;
	}
};

bool readFBXs(const vector<string>& fileNames, DemBonesExt<double, float>& model) {
	if ((int)fileNames.size()!=model.nS) err("Wrong number of FBX files or ABC files have not been loaded.\n");

	msg(1, "Reading FBXs:\n");

	FbxSceneImporter importer;

	MatrixXd wd(0, 0);
	bool hasKeyFrame=false;

	for (int s=0; s<model.nS; s++) {
		msg(1, "    \""<<fileNames[s]<<"\"... ");
		if (!importer.open(fileNames[s])) err("Error on opening file.\n");
		int nFr=model.fStart(s+1)-model.fStart(s);
		if (!importer.load(model.fTime.segment(model.fStart(s), nFr))) return false;

		if (s==0) {
			//Init
			if (importer.v.cols()!=model.nV) err("Inconsistent geometry.\n");
			model.u.resize(model.nS*3, model.nV);
			model.u.block(0, 0, 3, model.nV)=importer.v;
			model.fv=importer.fv;

			model.nB=(int)importer.jointName.size();
			model.boneName=importer.jointName;

			model.parent.resize(model.nB);
			model.bind.resize(model.nS*4, model.nB*4);
			model.preMulInv.resize(model.nS*4, model.nB*4);
			model.rotOrder.resize(model.nS*3, model.nB);
			model.orient.resize(model.nS*3, model.nB);
			model.lockM.resize(model.nB);

			for (int j=0; j<model.nB; j++) {
				string nj=model.boneName[j];
				
				model.parent(j)=-1;
				for (int k=0; k<model.nB; k++)
					if (model.boneName[k]==importer.parent[nj]) model.parent(j)=k;
			
				model.bind.blk4(s, j)=importer.bind[nj];
				model.preMulInv.blk4(s, j)=importer.preMulInv[nj];
				model.rotOrder.vec3(s, j)=importer.rotOrder[nj];
				model.orient.vec3(s, j)=importer.orient[nj];
				model.lockM(j)=importer.lockM[nj];
			}

			if (importer.wT.size()!=0) {
				wd=MatrixXd::Zero(model.nB, model.nV);
				for (int j=0; j<model.nB; j++) wd.row(j)=importer.wT[model.boneName[j]].transpose();
			}

			model.lockW=importer.lockW;

			model.m.resize(model.nF*4, model.nB*4);
		} else {
			//Merge
			if (importer.v.cols()!=model.nV) err("Inconsistent geometry.\n");
			model.u.block(s*3, 0, 3, model.nV)=importer.v;
			if (model.fv!=importer.fv) err("Inconsistent geometry.\n");

			if (model.nB!=importer.jointName.size()) err("Inconsistent joints set.\n");

			for (int j=0; j<model.nB; j++) {
				string nj=model.boneName[j];

				if (importer.parent.find(nj)==importer.parent.end()) err("Inconsistent joints set.\n");
				string pName=(model.parent(j)==-1)?"":model.boneName[model.parent(j)];
				if (importer.parent[nj]!=pName) err("Inconsistent skeleton hierarchy.\n");

				if (importer.bind.find(nj)==importer.bind.end()) err("Inconsistent joints set.\n");
				model.bind.blk4(s, j)=importer.bind[nj];
				if (importer.preMulInv.find(nj)==importer.preMulInv.end()) err("Inconsistent joints set.\n");
				model.preMulInv.blk4(s, j)=importer.preMulInv[nj];
				if (importer.rotOrder.find(nj)==importer.rotOrder.end()) err("Inconsistent joints set.\n");
				model.rotOrder.vec3(s, j)=importer.rotOrder[nj];
				if (importer.orient.find(nj)==importer.orient.end()) err("Inconsistent joints set.\n");
				model.orient.vec3(s, j)=importer.orient[nj];
				if (model.lockM(j)!=importer.lockM[nj]) err("Inconsistent joint lock set.\n");
			}

			if (wd.rows()!=importer.wT.size()) err("Inconsistent skinningWeights.\n");
			if (wd.rows()!=0) for (int j=0; j<model.nB; j++) wd.row(j)+=importer.wT[model.boneName[j]].transpose();
			model.lockW+=importer.lockW;
		}

		for (int j=0; j<model.nB; j++) model.m.block(s*4, j*4, nFr*4, 4)=importer.m[model.boneName[j]];
		hasKeyFrame|=importer.hasKeyFrame;
		
		msg(1, "Done!\n");
	}

	model.w=(wd/model.nS).sparseView(1, 1e-20);
	model.lockW/=(double)model.nS;
	if (!hasKeyFrame) model.m.resize(0, 0);

	msg(1, "    "<<model.nV<<" vertices");
	if (model.nB!=0) msg(1, ", "<<model.nB<<" joints found");
	if (hasKeyFrame) msg(1, ", key frames found");
	if (model.w.size()!=0) msg(1, ", skinning weights found");
	msg(1, "\n");

	return true;
}

#undef err
