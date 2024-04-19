#ifndef CURRENTRELAYSIM_HPP_
#define CURRENTRELAYSIM_HPP_

#include <CARLsimIO/NeuronSim/AbstractSim.hpp>

namespace carlsimio {

	/** Neuron simulator using the Izhikevich model of neurons */
	class CurrentRelaySim: public AbstractSim {

		public:
			CurrentRelaySim(size_t numNeurons);

			void setInputCurrent(unsigned index, double current);

			void step();

		private:

			//Data structures for simulation
			std::vector<double> iArray;

	};

}


#endif /* IZHIKEVICHNEURONSIM_HPP_ */
