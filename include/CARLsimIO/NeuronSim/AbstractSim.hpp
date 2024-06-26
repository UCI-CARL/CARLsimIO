#ifndef ABSTRACTSIM_HPP_
#define ABSTRACTSIM_HPP_

#include <vector>

namespace carlsimio {

	/** Neuron simulator using the Izhikevich model of neurons */
	class AbstractSim {

		public:
			AbstractSim(size_t numNeurons);
			virtual ~AbstractSim();

			const std::vector<unsigned>& getSpikes() { return spikeVector; }
			const std::vector<double>& getCurrentVector() { return currentVector; }

			virtual void setInputCurrent(unsigned index, double current) = 0;
			virtual void step() = 0;

		protected:
			/** Number of neurons in simulator */
			size_t numNeurons;

			/** Holds the current used in current time step */
			std::vector<double> currentVector;

			/** Holds the spikes generated by the neurons during the current time step */
			std::vector<unsigned> spikeVector;
	};

}


#endif /* ABSTRACTSIM_HPP_ */
