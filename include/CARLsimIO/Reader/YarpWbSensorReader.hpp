#ifndef YARPWBSENSORREADER_HPP_
#define YARPWBSENSORREADER_HPP_

#include <CARLsimIO/Reader/WbSensorReader.hpp>
#include <string>
#include <iostream>
#include <map>

//using namespace std;

#include <CARLsimIO/api.hpp>	

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace yarp {
	namespace os {
		class Network;
		class Port;
		//class BufferedPort;
		template <typename T> class BufferedPort;  // LN 2018 Fix for Yarp 2.3.72.0
		class Bottle;
	}
}

namespace carlsimio {


	/** Retrieves a tick values from a given file and makes it available upon request
	  V2: list of ticks with timestamp and symbol, synchronize simulator time with data time */
	class CARLSIMIO_API YarpWbSensorReader : public WbSensorReader {

	private:
		class YarpDataProcessor;
		class UnbufferedDataProcessor;

	public:

		/** Create a new file tick reader which reads from a file named ticksIn.txt */
		//FileTickReader(vector<TickReader::security_t> securities);
		YarpWbSensorReader();

		virtual ~YarpWbSensorReader();

		//virtual void initialize(map<string, Property>& properties);
		virtual void setProperties(map<string, Property>& properties);

		virtual void start();

		virtual void syncSim(unsigned long long simtime);

		void convertBottle();  // See bufferd port --> read  include .. header from YarpDevice 

		typedef enum { YarpDefined, YarpConnected, YarpStarted, YarpError } yarp_state_t;
		yarp_state_t getYarpState() { return m_yarp_state; }

		virtual const unsigned long long getTickTime() { return m_tick_time; }

		void setYarpLogger(std::ostream* os) { m_yarp_os = os; }  // for use with LOGS
//			ostream & logYarp(TLogLevel level) { if(m_yarp_os != NULL) return *m_yarp_os; else return Log().Get(level); }

	protected:
		string m_writing_port_name;
		string m_reading_port_name;
		string m_rpc_server_port_name;   // maybe for control and sync
		string m_rpc_client_port_name;
		int m_buffered;
		int m_timezone;

		yarp_state_t m_yarp_state;
		virtual void workerFunction();
		YarpDataProcessor* m_processor;
		friend YarpDataProcessor;
		friend UnbufferedDataProcessor;

		yarp::os::Network* m_network;
		yarp::os::Bottle* m_bottle;
		yarp::os::Port* m_port;
		//yarp::os::BufferedPort* m_buffered_port;
		yarp::os::BufferedPort<YarpWbSensorReader>* m_buffered_port;  // LN2018 Fix for Yarp 2.3.72  --> set E-Puck  WbSensorData
		yarp::os::Port* m_rpc_port;

		unsigned long long m_tick_time;

		std::ostream* m_yarp_os;
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* YARPWBSENSORREADER_HPP_ */
