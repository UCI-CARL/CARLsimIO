#ifndef EMBEDDEDCPUSIM_HPP_
#define EMBEDDEDCPUSIM_HPP_

#include <CARLsimIO/NeuronSim/AbstractSim.hpp>

namespace carlsimio {

	/** Neuron simulator using the Izhikevich model of neurons */
	class EmbeddedCpuSim : public AbstractSim {

		public:
			EmbeddedCpuSim(size_t numNeurons, double a, double b, double c, double d);

			void setInputCurrent(unsigned index, double current);

			void step();

		private:

			//Izhikevich parameters
			double aParam;
			double bParam;
			double cParam;
			double dParam;

			//Data structures for simulation
			std::vector<double> vArray;
			std::vector<double> uArray;
			std::vector<double> iArray;
			std::vector<bool> firedArray;

	};

}

#endif /* EMBEDDEDCPUSIM_HPP_ */
