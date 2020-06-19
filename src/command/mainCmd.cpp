///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#include <tclap/CmdLine.h>
#include <DemBones/DemBonesExt.h>
#include "FbxReader.h"
#include "AbcReader.h"
#include "FbxWriter.h"
#include "LogMsg.h"

using namespace std;
using namespace TCLAP;
using namespace Eigen;
using namespace Dem;

class MyDemBones: public DemBonesExt<double, float> {
public:
	void cbIterBegin() {
		msg(1, "    Iter #"<<iter<<": ");
	}

	void cbIterEnd() {
		msg(1, "RMSE = "<<rmse()<<"\n");
	}

	void cbInitSplitBegin() {
		msg(1, ">");

	}

	void cbInitSplitEnd() {
		msg(1, nB);
	}

	void cbWeightsBegin() {
		msg(1, "Updating weights");
	}

	void cbWeightsEnd() {
		msg(1, " Done! ");
	}

	void cbTranformationsBegin() {
		msg(1, "Updating trans");
	}

	void cbTransformationsEnd() {
		msg(1, " Done! ");
	}

	void cbTransformationsIterEnd() {
		msg(1, ".");
	}

	void cbWeightsIterEnd() {
		msg(1, ".");
	}
} model;

int main(int argc, char** argv) {
	//http://tclap.sourceforge.net/manual.html
	//http://tclap.sourceforge.net/html/annotated.html
	CmdLine cmd("Dem Bones - (c) Electronic Arts 2019\n"
"    1. This tool only handles clean input data, i.e. only one piece of geometry with one skinCluster and no excessive joint.\n\
     2. To hard-lock the transformation of a bone: in the input fbx files, create a bool attribute for the joint with name \"demLock\" and set its value to \"true\".\n\
     3. To soft-lock skinning weights of a vertex: in the input fbx files, paint the vertex color to grayscale.The closer the color to white, the more skinning weights of the vertex are preserved.", '=', "1.1");

	ValueArg<string> logFile("", "log", "log file name", false, "", "filename", cmd);
	ValueArg<int> dbg("", "dbg", "debug level", false, 1, "int", cmd);

	ValueArg<double> weightsSmoothStep("", "weightsSmoothStep", "step size for the weights smoothness", false, model.weightsSmoothStep, "double", cmd);
	ValueArg<double> weightsSmooth("", "weightsSmooth", "weights smoothness soft constraint", false, model.weightsSmooth, "double", cmd);
	ValueArg<int> nnz("z", "nnz", "number of non-zero weights per vertex", false, model.nnz, "int", cmd);
	ValueArg<int> nWeightsIters("", "nWeightsIters", "number of weights update iterations per global iteration", false, model.nWeightsIters, "int", cmd);

	ValueArg<double> transAffineNorm("", "transAffineNorm", "p-Norm for bone translations affinity", false, model.transAffineNorm, "double", cmd);
	ValueArg<double> transAffine("", "transAffine", "bone translations affinity soft constraint", false, model.transAffine, "double", cmd);
	ValueArg<int> bindUpdate("", "bindUpdate", "update bind pose (0=no update, 1=update joint positions)", false, model.bindUpdate, "int", cmd);
	ValueArg<int> nTransIters("", "nTransIters", "number of transformation update iterations per global iteration", false, model.nTransIters, "int", cmd);

	ValueArg<int> nIters("n", "nIters", "number of global iterations", false, model.nIters, "int", cmd);
	ValueArg<int> nInitIters("", "nInitIters", "number iterations per init cluster splitting", false, model.nInitIters, "int", cmd);

	ValueArg<int> nBones("b", "nBones", "number of bones", false, -1, "int", cmd);

	MultiArg<string> outFile("o", "out", "output (fbx files), each outut correspond to one abc file", true, "filename", cmd);
	MultiArg<string> initFile("i", "init", "rest pose/init skin clusters (fbx files), each file correspond to one abc file", true, "filename", cmd);
	MultiArg<string> dataFile("a", "abc", "animated mesh sequences (alembic geometry cache files)", true, "filename", cmd);

	try {
		cmd.parse(argc, argv);

		GLOBAL_DBG=dbg.getValue();

		if (logFile.getValue()!="") {
			msg(1, "Log file: \""<<logFile.getValue()<<"\"\n");
			GLOBAL_LOG_FILE_STREAM.open(logFile.getValue().c_str());
		}

		if ((initFile.getValue().size()!=0)&&(initFile.getValue().size()!=dataFile.getValue().size())) {
			msg(1, "Inconsistent init/mesh sequence files: #fbx/#abc = "<<initFile.getValue().size()<<"/"<<dataFile.getValue().size());
			if (GLOBAL_LOG_FILE_STREAM.is_open()) GLOBAL_LOG_FILE_STREAM.close();
			return 1;
		}

		if ((initFile.getValue().size()!=dataFile.getValue().size())||(dataFile.getValue().size()!=outFile.getValue().size())) {
			msg(1, "Inconsistent number of files: #abc/#fbx/#out = "<<dataFile.getValue().size()<<"/"<<initFile.getValue().size()<<"/"<<outFile.getValue().size());
			if (GLOBAL_LOG_FILE_STREAM.is_open()) GLOBAL_LOG_FILE_STREAM.close();
			return 1;
		}

		msg(1, dataFile.getValue().size()<<" animated mesh sequence(s):\n");
		for (int i=0; i!=dataFile.getValue().size(); i++)
			msg(1, "    \""<<dataFile.getValue()[i]<<"\" + \""<<initFile.getValue()[i]<<"\" --> \""<<outFile.getValue()[i]<<"\"\n");

		msg(1, "Parameters:\n");

		if (nBones.isSet()) {
		msg(1, "    nBones (target)    = "<<nBones.getValue()<<"\n");
		msg(1, "    nInitIters         = "<<nInitIters.getValue()<<(nInitIters.isSet()?"\n":" (default)\n"));
		}

		msg(1, "    nIters             = "<<nIters.getValue()<<(nIters.isSet()?"\n":" (default)\n"));
		msg(1, "    nTransIters        = "<<nTransIters.getValue()<<(nTransIters.isSet()?"\n":" (default)\n"));
		msg(1, "    nWeightsIters      = "<<nWeightsIters.getValue()<<(nWeightsIters.isSet()?"\n":" (default)\n"));
		msg(1, "    bindUpdate         = "<<bindUpdate.getValue());
		switch (bindUpdate.getValue()) {
			case 0: msg(1, " (no update)"); break;
			case 1: msg(1, " (update joint positions)"); break;
		};
		msg(1, (bindUpdate.isSet()?"\n":" (default)\n"));
		msg(1, "    transAffine        = "<<transAffine.getValue()<<(transAffine.isSet()?"\n":" (default)\n"));
		msg(1, "    transAffineNorm    = "<<transAffineNorm.getValue()<<(transAffineNorm.isSet()?"\n":" (default)\n"));
		msg(1, "    nnz                = "<<nnz.getValue()<<(nnz.isSet()?"\n":" (default)\n"));
		msg(1, "    weightsSmooth      = "<<weightsSmooth.getValue()<<(weightsSmooth.isSet()?"\n":" (default)\n"));
		msg(1, "    weightsSmoothStep  = "<<weightsSmoothStep.getValue()<<(weightsSmoothStep.isSet()?"\n":" (default)\n"));
	} catch (ArgException &e) {
		msg(1, "Error: "<<e.error()<<" for arg "<<e.argId()<<"\n");
	}

	if (!readABCs(dataFile.getValue(), model)) return 1;
	if (!readFBXs(initFile.getValue(), model)) return 1;

	model.nIters=nIters.getValue();
	model.nTransIters=nTransIters.getValue();
	model.nWeightsIters=nWeightsIters.getValue();
	model.bindUpdate=bindUpdate.getValue();
	model.transAffine=transAffine.getValue();
	model.transAffineNorm=transAffineNorm.getValue();

	model.nnz=nnz.getValue();
	model.weightsSmooth=weightsSmooth.getValue();
	model.weightsSmoothStep=weightsSmoothStep.getValue();

	if (model.nB==0) {
		if (!nBones.isSet()) {
			msg(1, "No joint found in. Need to set the number of bones (-b/--nBones)\n");
			return 1;
		}

		model.nB=nBones.getValue();
		model.nInitIters=nInitIters.getValue();
		msg(1, "Initializing bones: 1");
		model.init();
		msg(1, "\n");
	}

	msg(1, "Computing Skinning Decomposition:\n");
	model.compute();

	if (!writeFBXs(outFile.getValue(), initFile.getValue(), model)) return 1;

	return 0;
}
