#include <CARLsimIO/NeuronSim/CarlsimSim.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>

#include <carlsim_conf.h>
#include <carlsim.h>

#include <stdio.h>

using namespace carlsimio;

/** Constructor with parameters */
CarlsimSim::CarlsimSim(size_t numNeurons,
	double a, double b, double c, double d) :
	AbstractSim(numNeurons),
	aParam(a),
	bParam(b),
	cParam(c),
	dParam(d),
	iArray(numNeurons, 0.0),
	firing(numNeurons+1, false) // gIn + gDummy
{
	LOG(LOG_DEBUG) << "Initializing CARLsim with " << numNeurons << " neurons.";
	spikeVector.reserve(numNeurons);
	spikeVector.clear();


	SimMode mode = GPU_MODE;
	ComputingBackend backend = GPU_CORES; 
	int partitition = 0; // TCC
	//int partitition = 2; // WDDM
	LoggerMode logger = SILENT;

	sim = new CARLsim("finspikes2", mode, logger);

	gIn = sim->createGroup("in", numNeurons, EXCITATORY_NEURON, partitition, backend);  

	sim->setNeuronParameters(gIn, float(a), float(b), float(c), float(d)); // Izhikevic RS

/*
*********************************************************************************
********************      Welcome to CARLsim 4.0      ***************************
*********************************************************************************

***************************** Configuring Network ********************************
Starting CARLsim simulation "finspikes2" in DEVELOPER mode

*/

	int gDummy = sim->createGroup("in", 1, EXCITATORY_NEURON, partitition, backend); 
	sim->setNeuronParameters(gDummy, float(a), float(b), float(c), float(d)); // Izhikevic RS
	sim->connect(gIn, gDummy, "full", RangeWeight(0.0), 0.0);

	// disable COBA mode
	sim->setConductances(false);

	sim->setupNetwork();
}


CarlsimSim::~CarlsimSim() {
	if (sim)
		delete sim;
}

/** Sets the input current to a particular neuron */
void CarlsimSim::setInputCurrent(unsigned index, double current) {
	if (index >= numNeurons) {
		throw CARLsimIOException("IzhikevichNeuronSim: Index is out of range.");
	}
	iArray[index] = float(current);
}


/** Advances simulator by one time step */
void CarlsimSim::step() {
	sim->setExternalCurrent(gIn, iArray);
	sim->runNetwork(0, 1);
	sim->getFiring(firing);
	spikeVector.clear();
	for (int i = 0; i < numNeurons; i++) {
		if (firing[i])
			spikeVector.push_back(i);
		iArray[i] = .0f;
	}
}

