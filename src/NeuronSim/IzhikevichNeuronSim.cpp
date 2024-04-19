#include <CARLsimIO/NeuronSim/IzhikevichNeuronSim.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>

#include <CARLsimIO/NeuronSim/AbstractSim.hpp>   
#include <CARLsimIO/NeuronSim/EmbeddedCpuSim.hpp>
#include <CARLsimIO/NeuronSim/CurrentRelaySim.hpp>
#include <CARLsimIO/NeuronSim/CarlsimSim.hpp>

#include <cstdio>


using namespace carlsimio;

class IzhikevichNeuronSim::Impl {
public:

	/** Empty constructors - parameters have to be set separately */
	Impl(IzhikevichNeuronSim* sim) :
		sim_(sim),
		numNeurons(0),
		aParam(0.1),
		bParam(0.2),
		cParam(-65.0),
		dParam(2.0),
		backendParam(1) // downward compatibility 
	{
		backend_ = nullptr;
	}


	/** Constructor with parameters */
	Impl(IzhikevichNeuronSim* sim,
		size_t numNeurons,
		double a, double b, double c, double d) :
		sim_(sim),
		numNeurons(numNeurons),
		aParam(a),
		bParam(b),
		cParam(c),
		dParam(d),
		backendParam(1) // downward compatibility 
	{
		backend_ = nullptr;
	}

	~Impl() {
		if(backend_)
			delete backend_;
	}

	/*--------------------------------------------------------------------*/
	/*---------                 PUBLIC METHODS                     -------*/
	/*--------------------------------------------------------------------*/


	const std::vector<unsigned>& getSpikes() { return backend_->getSpikes(); }

	const std::vector<double>& getCurrentVector() { return backend_->getCurrentVector(); }

	// collect params 
	void setParameterA(double a) { this->aParam = a; }
	void setParameterB(double b) { this->bParam = b; }
	void setParameterC(double c) { this->cParam = c; }
	void setParameterD(double d) { this->dParam = d; }
	void setParameterBackend(int backend) { this->backendParam = backend; }
	

	/** Re-initializes the simulator, creating arrays and setting parameters */
	void initialize(size_t numNeurons) {
		this->numNeurons = numNeurons;
		switch (backendParam) {
		case 0:
			backend_ = new CurrentRelaySim(numNeurons);
			break;
		case 1:
			backend_ = new EmbeddedCpuSim(numNeurons, aParam, bParam, cParam, dParam);
			break;
		case 2:
			backend_ = new CarlsimSim(numNeurons, aParam, bParam, cParam, dParam);
			break;
		default:
			throw CARLsimIOException("IzhikevichNeuronSim: Index is out of range.");
		}
	}


	/** Sets the input current to a particular neuron */
	void setInputCurrent(unsigned index, double current) {
		if (index >= numNeurons) {
			throw CARLsimIOException("IzhikevichNeuronSim: Index is out of range.");
		}
		backend_->setInputCurrent(index, current);	// better virual than switch here, its OOP still !!!
	}


	/** Advances simulator by one time step */
	void step() {
		backend_->step(); 
	}



	void logFiringsHeader(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
	{
		// "     0: 22:34:00.000 "	
		if (simtime>-1) s << "              Neuron ";
		for (int i = 1; i <= numNeurons; i++)
		{
			if (group>-1 && i%group == 1) s << '|'; // Tupel Group
			if (i>1 && cluster>-1 && i % (group*cluster) == 1) s << ' ' << '|';
			s << i / 100;
			//if(i%10==0) s << '|'; // Tupel Group
			//if(i%40==0) s << ' '; // Security Group
		}
		if (group>-1) s << '|';
		s << endl;

		// "     0: 22:34:00.000 "	
		if (simtime>-1) 	s << "                     ";
		for (int i = 1; i <= numNeurons; i++)
		{
			if (group>-1 && i%group == 1) s << '|'; // Tupel Group
			if (i>1 && cluster>-1 && i % (group*cluster) == 1) s << ' ' << '|';
			s << (i % 100) / 10;
			//if(i%10==0) s << '|'; // Tupel Group
			//if(i%40==0) s << ' '; // Security Group
		}
		if (group>-1) s << '|';
		s << endl;
		// "     0: 22:34:00.000 "	
		if (simtime>-1) 	s << "  Step  Time         ";
		for (int i = 1; i <= numNeurons; i++)
		{
			if (group>-1 && i%group == 1) s << '|'; // Tupel Group
			if (i>1 && cluster>-1 && i % (group*cluster) == 1) s << ' ' << '|';
			s << i % 10;
			//if(i%10==0) s << '|'; // Tupel Group
			//if(i%40==1) s << ' '; // Security Group
		}
		if (group>-1) s << '|';
		s << endl;
	}

	void logFirings(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
	{
		if (backendParam < 1)  // no Spike generating Backend
			return;

		static const unsigned line = 100;
		static char buf[line];

		//int offset = 0;
		if (tick_time>-1)
		{
			sprintf_s(buf, line, "%6llu: %02d:%02d:%02d.%03d ",
				simtime,
				(tick_time / 1000 / 60 / 60) % 24, (tick_time / 1000 / 60) % 60, (tick_time / 1000) % 60, tick_time % 1000);
			//offset = 8 + 12
		}
		else
			if (simtime>-1)
			{
				sprintf_s(buf, line, "%6llu: ", simtime);
				//offset = 8;
			}
		s << buf;

		auto &spkVec = sim_->getSpikes();
		//vector<unsigned>::iterator iter = spikeVector.begin();
		auto iter = spkVec.begin();
		for (int i = 0; i<numNeurons; i++)
		{
			if (group>-1 && i%group == 0) s << '|'; // Tupel Group
			if (i>1 && cluster>-1 && i % (group*cluster) == 0) s << ' ' << '|';
			//if (iter != spikeVector.end() && *iter == i)
			if (iter != spkVec.end() && *iter == i)
				{
				s << '-';
				iter++;
			}
			else
				s << ' ';
			//if(i%10==9) s << '|'; // Tupel Group
			//if(i%40==9) s << ' '; // Security Group
		}

		if (group>-1) s << '|';
		s << endl;
	}

	



	void logCurrentHeader(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
	{
		static const unsigned line = 100;
		static char buf[line];

		if (simtime>-1) 	s << "  Step  Time  Neuron ";
		for (int i = 1; i <= numNeurons; i++)
		{
			if (group>-1 && i%group == 1) s << ' '; // Tupel Group
			if (i>1 && cluster>-1 && i % (group*cluster) == 1) s << ' ' << ' ';
			sprintf_s(buf, line, "%5d ", i);
			s << buf;
		}
		if (group>-1) s << '|';
		s << endl;
	}

	void logCurrent(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
	{
		static const unsigned line = 100;
		static char buf[line];

		if (backendParam > 0)  // no current Based Backend
			return;

		//int offset = 0;
		if (tick_time>-1)
		{
			sprintf_s(buf, line, "%6llu: %02d:%02d:%02d.%03d ",
				simtime,
				(tick_time / 1000 / 60 / 60) % 24, (tick_time / 1000 / 60) % 60, (tick_time / 1000) % 60, tick_time % 1000);
			//offset = 8 + 12
		}
		else
			if (simtime>-1)
			{
				sprintf_s(buf, line, "%6llu: ", simtime);
				//offset = 8;
			}
		s << buf;

		auto &curVec = sim_->getCurrentVector();  // apply the back ref / public method
		for (int i = 0; i<numNeurons; i++)
		{
			if (group>-1 && i%group == 0) s << ' '; // Tupel Group
			if (i>1 && cluster>-1 && i % (group*cluster) == 0) s << ' ' << ' ';
			sprintf_s(buf, line, "%5.1f ", curVec[i]);  // csv
			s << buf;
		}

		if (group>-1) s << ' ';
		s << endl;
	}




	// +++++ PRIVATE PROPERTIES +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

	IzhikevichNeuronSim* sim_;

	AbstractSim* backend_; 

	/** Number of neurons in simulator */
	size_t numNeurons;

	//Izhikevich parameters
	double aParam;
	double bParam;
	double cParam;
	double dParam;
	int backendParam;  // 0 embeddedCPU (Legacy), 1 Current, 2 CARLSim...

	/** Records whether simulator has been initialized */
	bool initialized;

};




// ****************************************************************************************************************** //
// CARLSIM API IMPLEMENTATION
// ****************************************************************************************************************** //


// constructor / destructor

IzhikevichNeuronSim::IzhikevichNeuronSim() :
	_impl(new Impl(this)) {}

IzhikevichNeuronSim::IzhikevichNeuronSim(size_t numNeurons, double a, double b, double c, double d) :
	_impl(new Impl(this, numNeurons, a, b, c, d)) {}

IzhikevichNeuronSim::~IzhikevichNeuronSim() { delete _impl; }


const std::vector<unsigned>& IzhikevichNeuronSim::getSpikes() { return _impl->getSpikes(); };
const std::vector<double>& IzhikevichNeuronSim::getCurrentVector() { return _impl->getCurrentVector(); };


void IzhikevichNeuronSim::step() {
	_impl->step();
}


void IzhikevichNeuronSim::setInputCurrent(unsigned index, double current) {
	_impl->setInputCurrent(index, current);
}

void IzhikevichNeuronSim::setParameterA(double a) {
	_impl->setParameterA(a);
}
void IzhikevichNeuronSim::setParameterB(double b) {
	_impl->setParameterB(b);
}
void IzhikevichNeuronSim::setParameterC(double c) {
	_impl->setParameterC(c);
}
void IzhikevichNeuronSim::setParameterD(double d) {
	_impl->setParameterD(d);
}

void IzhikevichNeuronSim::setParameterBackend(int i) {
	_impl->setParameterBackend(i); // translation to bool
}

void IzhikevichNeuronSim::initialize(size_t numNeurons) {
	_impl->initialize(numNeurons);
}



void IzhikevichNeuronSim::logFiringsHeader(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
{
	_impl->logFiringsHeader(s, simtime, tick_time, group, cluster);
}

void IzhikevichNeuronSim::logFirings(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
{
	_impl->logFirings(s, simtime, tick_time, group, cluster);
}

void IzhikevichNeuronSim::logCurrentHeader(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
{
	_impl->logCurrentHeader(s, simtime, tick_time, group, cluster);
}

void IzhikevichNeuronSim::logCurrent(std::ostream &s, long long simtime, long long tick_time, int group, int cluster)
{
	_impl->logCurrent(s, simtime, tick_time, group, cluster);
}



