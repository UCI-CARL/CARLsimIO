#ifndef YARPAKDYOLOREADER_HPP_
#define YARPAKDYOLOREADER_HPP_


#include <CARLsimIO/Reader/AkdYoloReader.hpp>

#include <string>
#include <iostream>
#include <map>


#include <CARLsimIO/api.hpp>

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
	class CARLSIMIO_API YarpAkdYoloReader : public AkdYoloReader {

		private: 
			class YarpDataProcessor;
			class UnbufferedDataProcessor;

		public:

			/** Create a new file tick reader which reads from a file named ticksIn.txt */
			//FileTickReader(vector<TickReader::security_t> securities);
			YarpAkdYoloReader();

			virtual ~YarpAkdYoloReader();

			//virtual void initialize(map<string, Property>& properties);
			virtual void setProperties(map<string, Property>& properties);

			virtual void start(); 

			virtual void syncSim(unsigned long long simtime);
			
			void convertBottle();

			typedef enum {YarpDefined, YarpConnected, YarpStarted, YarpError} yarp_state_t;
			yarp_state_t getYarpState() {return m_yarp_state;}

			virtual const unsigned long long getYoloEventTime() {return m_yolo_event_time;}

			void setYarpLogger(std::ostream* os) {m_yarp_os = os;}  // for use with LOGS
//			ostream & logYarp(TLogLevel level) { if(m_yarp_os != NULL) return *m_yarp_os; else return Log().Get(level); }

		protected:
			string m_writing_port_name;
			string m_reading_port_name;			
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
			//yarp::os::BufferedPort* m_buffered_port;
			yarp::os::BufferedPort<YarpAkdYoloReader>* m_buffered_port;
			yarp::os::Port* m_rpc_port;

			unsigned long long m_yolo_event_time;

			std::ostream* m_yarp_os;

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* YARPAKDYOLOREADER_HPP_ */
