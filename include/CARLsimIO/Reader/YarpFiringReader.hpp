#ifndef YARPFIRINGREADER_HPP_
#define YARPFIRINGREADER_HPP_

#include <CARLsimIO/Reader/FiringReader.hpp>
#include <string>
#include <iostream>
#include <map>

//using namespace std;

#include <CARLsimIO/api.hpp>	// iSpike 2.1.2

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

#define SCOPE_PORT_BUFFERED

namespace yarp {
	namespace os {
		class Network;
		class Port;
		class Bottle;
#ifdef SCOPE_PORT_BUFFERED
		template <typename T> class BufferedPort;
#endif
	}
}

namespace carlsimio {


	/** Retrieves a tick values from a given file and makes it available upon request
	  V2: list of ticks with timestamp and symbol, synchronize simulator time with data time */
	class CARLSIMIO_API YarpFiringReader : public FiringReader {

		private: 
			class YarpDataProcessor;
			class UnbufferedDataProcessor;

		public:

			/** Create a new file tick reader which reads from a file named ticksIn.txt */
			//FileTickReader(vector<TickReader::security_t> securities);
			YarpFiringReader();

			virtual ~YarpFiringReader();

			//virtual void initialize(map<string, Property>& properties);
			virtual void setProperties(map<string, Property>& properties);

			virtual void start(); 

			virtual void syncSim(unsigned long long simtime);

			
			void convertBottle();

			typedef enum {YarpDefined, YarpConnected, YarpStarted, YarpError} yarp_state_t;
			yarp_state_t getYarpState() {return m_yarp_state;}

			virtual const unsigned long long getTradeEventTime() {return m_trade_event_time;}

			void setYarpLogger(std::ostream* os) {m_yarp_os = os;}  // for use with LOGS
//			ostream & logYarp(TLogLevel level) { if(m_yarp_os != NULL) return *m_yarp_os; else return Log().Get(level); }

			//virtual bool hasScopePort() { return m_scope_port != NULL;  }
			//virtual void publishDopamine(double delta, double level);

		protected:
			string m_writing_port_name;
			string m_reading_port_name;
			
			string m_scope_port_name;   // LN2019 yarpscope alternative: dopamine_port  - scope_port drücckt aus, dass der Port für yarpscope ist siehe rot/grün balken

			//string m_rpc_server_port_name;
			//string m_rpc_client_port_name;
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
//			yarp::os::BufferedPort* m_buffered_port;
			//yarp::os::Port* m_rpc_port;


			unsigned long long m_trade_event_time;

			std::ostream* m_yarp_os;

			/*! experimental
				yarp bottle of type TradeEvent are buffered in a queue
				if sim-time reaches or passes the timestamp of the trade event
				the tradeevent is getting active. 
				and the (flanken gesteuerterte)  .. injection of dopamin
					reward interpreted and translated into a dopamine injection 
						(or rejection, if a penalty) 

			*/
			double m_dopamine; // current dopamin level
			double m_dopamine_injection; // last dopamin injection 
			// registered handler, -> Interface/Callback ? -->  direct injection
#ifndef SCOPE_PORT_BUFFERED
			yarp::os::Port* m_scope_port;  //! to connect a YarpScope 
#else
			yarp::os::BufferedPort<yarp::os::Bottle>* m_scope_port;  //! to connect a YarpScope 
#endif
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* YARPFIRINGREADER_HPP_ */
