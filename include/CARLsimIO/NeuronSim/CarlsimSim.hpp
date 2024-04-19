#ifndef CARLSIMSIM_HPP_
#define CARLSIMSIM_HPP_

#include <CARLsimIO/NeuronSim/AbstractSim.hpp>

class CARLsim; 

namespace carlsimio {

	/** Neuron simulator using the Izhikevich model of neurons */
	class CarlsimSim : public AbstractSim {

	public:
		CarlsimSim(size_t numNeurons, double a, double b, double c, double d);
		virtual ~CarlsimSim();

		void setInputCurrent(unsigned index, double current);

		void step();

	private:
		//Izhikevich parameters
		double aParam;
		double bParam;
		double cParam;
		double dParam;
		//Current Collector
		std::vector<float> iArray;
		//Pimpl
		CARLsim* sim;
		int gIn;
		std::vector<bool> firing;
	};

}

#endif //CARLSIMSIM_HPP_
