#ifndef YARPWBACTUATORWRITER_HPP_
#define YARPWBACTUATORWRITER_HPP_

//iSpike includes
#include <CARLsimIO/Writer/WbActuatorWriter.hpp>


//Other includes
#include <string>
#include <map>

#include <CARLsimIO/api.hpp>	

//Boost includes
#include <boost/thread.hpp>

// 
#include <carl/boost_ext.hpp>


#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace yarp {
	namespace os {
		class Network;
		class RpcClient;
		class Bottle;
		class Port;
	}
}

namespace carlsimio {

	class CARLSIMIO_API YarpWbActuatorWriter : public WbActuatorWriter {
	public:
		YarpWbActuatorWriter();
		virtual ~YarpWbActuatorWriter();
		void initialize(map<string, Property>& properties);

		virtual void setActuatorValues(const vector<double>& values);


		void setProperties(map<string, Property>& properties);
		void start();


		void logBuffer(bool skipped);

		// New Diagnostic Interface
		virtual size_t getQueued();
		virtual unsigned long long getNext_ms();

		virtual const unsigned long long getActuatorTime() { return m_actuator_time; }

		void setYarpLogger(std::ostream* os) { m_yarp_os = os; }  // for use with LOGS


	private:
		////===========================  VARIABLES  =========================
		///** Class handling connection to YARP */
		//YarpConnection* yarpConnection;

		/*! Controls whether the order should be output */
		bool m_actuator_changed;
		unsigned long long m_actuator_time;


		/** The port to use */
		string portName;

		/** Amount to sleep in between writing commands */
		unsigned sleepDuration_ms;


		//===========================  METHODS  ===========================
		void updateProperties(map<string, Property>& properties);
		void workerFunction();

	protected:
		string m_writing_port_name;
		string m_reading_port_name;
		string m_rpc_client_port_name;
		string m_rpc_server_port_name;

		/*! ensure save access to set the order and processing it*/
		boost::mutex m_resource_mutex;   // ensure threadsave access to m_order and m_changed
		boost::mutex m_process_mutex;	// coordinate background processing
		boost::condition_variable m_cv;

		yarp::os::Network* m_network;
		yarp::os::Bottle* m_bottle;
		yarp::os::Port* m_port;
	
  	yarp::os::RpcClient* m_rpc_client;

		std::ostream* m_yarp_os;

	};

}
#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* YARPWBACTUATORWRITER_HPP_ */
