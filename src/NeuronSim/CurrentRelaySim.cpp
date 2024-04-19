
#include <CARLsimIO/NeuronSim/CurrentRelaySim.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>

using namespace carlsimio;

/** Constructor with parameters */
CurrentRelaySim::CurrentRelaySim(size_t numNeurons) :
	AbstractSim(numNeurons),
	iArray(numNeurons, 0.0)	
{
	LOG(LOG_DEBUG) << "Initializing CurrentRelay with " << numNeurons << " neurons.";
	currentVector.resize(numNeurons, 0.0);
}


/** Sets the input current to a particular neuron */
void CurrentRelaySim::setInputCurrent(unsigned index, double current){
	if(index >= numNeurons) {
		throw CARLsimIOException("IzhikevichNeuronSim: Index is out of range.");
	}
	iArray[index] = current;
}


/** Advances simulator by one time step */
void CurrentRelaySim::step(){
	for (unsigned n = 0; n < numNeurons; ++n) {
		currentVector[n] = iArray[n];
		iArray[n] = 0.0;
	}
}
