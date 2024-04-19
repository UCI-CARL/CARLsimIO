#include <CARLsimIO/NeuronSim/EmbeddedCpuSim.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>

#include <algorithm>

using namespace carlsimio;

/** Constructor with parameters */
EmbeddedCpuSim::EmbeddedCpuSim(size_t numNeurons,
		double a, double b, double c, double d) :
	AbstractSim(numNeurons),
	aParam(a),
	bParam(b),
	cParam(c),
	dParam(d),
	vArray(numNeurons, c),
	uArray(numNeurons, b*c),
	iArray(numNeurons, 0.0),
	firedArray(numNeurons, false)
{
	LOG(LOG_DEBUG) << "Initializing EmbeddedCpuSim with " << numNeurons << " neurons.";
	spikeVector.reserve(numNeurons);
	spikeVector.clear();
}


/** Sets the input current to a particular neuron */
void EmbeddedCpuSim::setInputCurrent(unsigned index, double current){
	if(index >= numNeurons) {
		throw CARLsimIOException("IzhikevichNeuronSim: Index is out of range.");
	}
	iArray[index] = current;
}


/** Advances simulator by one time step */
void EmbeddedCpuSim::step(){

	//Clear spikes and fired array
	spikeVector.clear();
	std::fill(firedArray.begin(), firedArray.end(), false);

	//Calculate state of neurons
	for (unsigned n = 0; n < numNeurons; ++n) {
		for (unsigned t = 0; t<4; ++t) {
			if (!firedArray[n]) {
				vArray[n] += 0.25 * ((0.04* vArray[n] + 5.0) * vArray[n] + 140.0 - uArray[n] + iArray[n]);
				uArray[n] += 0.25 * (aParam * (bParam * vArray[n] - uArray[n]));
				firedArray[n] = vArray[n] >= 30.0;
			}
		}
		iArray[n] = 0.0;

		//Neuron has fired -add spike to buffer
		if (firedArray[n]) {
			vArray[n] = cParam;
			uArray[n] += dParam;
			spikeVector.push_back(n);
		}
	}
}
