#ifndef IZHIKEVICHNEURONSIM_HPP_
#define IZHIKEVICHNEURONSIM_HPP_

#include <vector>
#include <stdlib.h>
#include <ostream>

#include <CARLsimIO/api.hpp>


#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** Neuron simulator using the Izhikevich model of neurons */
	class CARLSIMIO_API IzhikevichNeuronSim {

	public:
		
		IzhikevichNeuronSim();
		IzhikevichNeuronSim(size_t numNeurons, double a, double b, double c, double d);   

		~IzhikevichNeuronSim();

		const std::vector<unsigned>& getSpikes();
		const std::vector<double>& getCurrentVector(); 
		void step();
		void setInputCurrent(unsigned index, double current);
		void setParameterA(double a);
		void setParameterB(double b);
		void setParameterC(double c);
		void setParameterD(double d);
		void setParameterBackend(int i);
		 
		void initialize(size_t numNeurons);

		// LN
		void logFirings(std::ostream &s, long long simtime = -1, long long ticktime = -1, int group = -1, int cluster = -1);
		void logFiringsHeader(std::ostream &s, long long simtime = -1, long long ticktime = -1, int group = -1, int cluster = -1);

		void logCurrent(std::ostream &s, long long simtime = -1, long long ticktime = -1, int group = -1, int cluster = -1);
		void logCurrentHeader(std::ostream &s, long long simtime = -1, long long ticktime = -1, int group = -1, int cluster = -1);

	private:
		class Impl;
		Impl* _impl;
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif


#endif /* IZHIKEVICHNEURONSIM_HPP_ */
