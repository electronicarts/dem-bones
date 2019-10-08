///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/AbcGeom/All.h>

#include "AbcReader.h"
#include "LogMsg.h"

using namespace Alembic;

class AbcFile {
public:
	string fileName;
	int nSamples;

	AbcFile() {
		nSamples=0;
	}

	//http://jonmacey.blogspot.com/2011/12/getting-started-with-alembic.html
	//devkitBase/devkit/plug-ins/AbcImport/AlembicNode.cpp
	//devkitBase/devkit\plug-ins/gpuCache/CacheReaderAlembic.cpp
	Abc::IObject firstAnimatedMesh(Abc::IObject obj) {
		const Abc::MetaData& md2=obj.getMetaData();
		if (AbcGeom::IPolyMeshSchema::matches(md2)||AbcGeom::ISubDSchema::matches(md2)) return obj;

		int numChildren=(int)obj.getNumChildren();
		for (int i=0; i<numChildren; i++) {
			Abc::IObject child(obj, obj.getChildHeader(i).getName());
			Abc::IObject found=firstAnimatedMesh(child);
			if (found!=Abc::IObject()) return found;
		}

		return Abc::IObject();
	}

	int open(const string& _fileName, int& nV) {
		fileName=_fileName;
		msg(1, "    \""<<fileName<<"\": ");

		factory.setPolicy(Alembic::Abc::ErrorHandler::kQuietNoopPolicy);

		archive=factory.getArchive(fileName);
		if (!archive.valid()) {
			msg(1, "cannot read file.\n");
			return 0;
		}

		meshObj=firstAnimatedMesh(archive.getTop());
		if (meshObj==Abc::IObject()) {
			msg(1, "found no mesh.\n");
			return 0;
		}
		msg(1, meshObj.getFullName());

		mesh=AbcGeom::IPolyMesh(meshObj.getParent(), meshObj.getName());

		schema=mesh.getSchema();

		if (nV==0) nV=(int)schema.getPositionsProperty().getValue()->size(); else
			if ((int)schema.getPositionsProperty().getValue()->size()!=nV) {
				msg(1, "inconsistent number of vertices: "<<schema.getPositionsProperty().getValue()->size()<<"/"<<nV<<".\n");
				return 0;
			}

		nSamples=(int)schema.getNumSamples();
		msg(1, " ("<<nSamples<<" frames)\n");

		return nSamples;
	}

	void readData(DemBonesExt<double, float>& model, int start) {
		//Global transform
		AbcGeom::IXform x(meshObj.getParent(), AbcGeom::kWrapExisting);
		AbcGeom::XformSample xs;
		x.getSchema().get(xs);
		AbcGeom::M44d gm=xs.getMatrix();

		AbcCoreAbstract::TimeSamplingPtr ts=schema.getTimeSampling();
		int nSamples=(int)schema.getNumSamples();
		int nStep=max(1, model.nF/50);
		for (int k=0; k<nSamples; k++) {
			model.fTime(start+k)=double(ts->getSampleTime((AbcCoreAbstract::index_t)k));
			Abc::P3fArraySamplePtr p=schema.getPositionsProperty().getValue(Abc::ISampleSelector((AbcCoreAbstract::index_t)k));
			#pragma omp parallel for
			for (int i=0; i<model.nV; i++) {
				AbcGeom::V3f pm=(*p)[i]*gm;
				model.v.col(i).segment<3>((start+k)*3)<<pm.x, pm.y, pm.z;
			}

			if ((start+k)%nStep==nStep-1) msg(1, '.');
		}
	}

private:
	Alembic::Abc::IArchive archive;
	Alembic::AbcCoreFactory::IFactory factory;
	Abc::IObject meshObj;
	AbcGeom::IPolyMesh mesh;
	AbcGeom::IPolyMeshSchema schema;
};

bool readABCs(const vector<string>& fileNames, DemBonesExt<double, float>& model) {
	msg(1, "Checking ABCs:\n");

	model.nS=(int)fileNames.size();
	model.nV=0;

	vector<AbcFile> files(model.nS);
	model.nF=0;
	for (int s=0; s<model.nS; s++) {
		int nfs=files[s].open(fileNames[s], model.nV);
		if (nfs==0) return false;
		model.nF+=nfs;
	}

	msg(1, "Reading ABCs");

	model.v.resize(3*model.nF, model.nV);
	model.fTime.resize(model.nF);
	model.fStart.resize(model.nS+1);
	model.fStart(0)=0;

	for (int s=0; s<model.nS; s++) {
		files[s].readData(model, model.fStart(s));
		model.fStart(s+1)=model.fStart(s)+files[s].nSamples;
	}

	model.subjectID.resize(model.nF);
	for (int s=0; s<model.nS; s++)
		for (int k=model.fStart(s); k<model.fStart(s+1); k++) model.subjectID(k)=s;

	msg(1, " Done!\n");

	return true;
}
