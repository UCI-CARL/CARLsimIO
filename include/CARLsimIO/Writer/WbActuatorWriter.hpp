#ifndef WBACTUATORWRITER_HPP_
#define WBACTUATORWRITER_HPP_

#include <CARLsimIO/Writer/Writer.hpp>
#include <CARLsimIO/api.hpp>	

#include <carl/boost_ext.hpp>

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	/*! WbActuatorWriter, receives an angle as input and writes it to the designated output */
	class CARLSIMIO_API WbActuatorWriter : public Writer {
	public:
		WbActuatorWriter() : m_simtime(0), m_processed(0), m_last_ms(0) {}
		virtual ~WbActuatorWriter() {}

		virtual void setProperties(map<string, Property>& properties);

		//  ActuatorPrefix, Index to Neuron Group 
		virtual void setActuatorPrefix(const string prefix) { m_actuatorPrefix = prefix; }


		virtual void syncSim(unsigned long long simtime) { m_simtime = simtime; }


		virtual void setActuatorValues(const vector<double> &values) = 0;

		// New Diagnostic Interface
		virtual size_t getQueued() = 0;  // Amount of Item in the buffer
		virtual unsigned long long getNext_ms() = 0;

		virtual void reset();

		virtual size_t getProcessed();
		virtual unsigned long long getLast_ms();


	protected:


		//ncboost::order_t m_order;
		vector<double> m_actuatorValues;  // _actuator

		//vector<Common::security_t> m_securities; // Copy
		string m_actuatorPrefix; // copy

		// string::prefix;

		unsigned long long m_simtime;

		// New diagnostig interface
		unsigned long long m_last_ms;
		size_t m_processed;


	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* WBACTUATORWRITER_HPP_ */
