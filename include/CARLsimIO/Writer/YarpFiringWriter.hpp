#ifndef YARPFIRINGWRITER_HPP_
#define YARPFIRINGWRITER_HPP_

//iSpike includes
#include <CARLsimIO/Writer/FiringWriter.hpp>


//Other includes
#include <string>
#include <map>

#include <CARLsimIO/api.hpp>	

//Boost includes
#include <boost/thread.hpp>


#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace yarp {
	namespace os {
		//class Network;
		class RpcClient;
		//class Bottle;
		class Port;
	}
}

namespace carlsimio {

	/** Writes angles to YARP */
	class CARLSIMIO_API YarpFiringWriter : public FiringWriter {
		public:
			YarpFiringWriter();
			virtual ~YarpFiringWriter();
			void initialize(map<string,Property>& properties);

			virtual void setFiring(const std::vector<unsigned>& aer);


			void setProperties(map<string,Property>& properties);
			void start();

			void logBuffer(bool skipped);

			// New Diagnostic Interface
			virtual size_t getQueued();
			virtual unsigned long long getNext_ms();


			void setYarpLogger(std::ostream* os) {m_yarp_os = os;}  // for use with LOGS


		private:

			bool m_firing_changed;			
			unsigned long long m_firing_time; 


			/** The port to use */
			string portName;

			/** Amount to sleep in between writing commands */
			unsigned sleepDuration_ms;


			//===========================  METHODS  ===========================
			void updateProperties(map<string, Property>& properties);
			void workerFunction();

		protected: 
		
			string yarpPortPrefix; 
			//string yarpNeuronGroup; 

			yarp::os::Port* m_port;


			/*! ensure save access to set the order and processing it	
			*/
			boost::mutex m_resource_mutex;   // ensure threadsave access to m_order and m_changed
			boost::mutex m_process_mutex;	// coordinate background processing
			boost::condition_variable m_cv;		

			std::ostream* m_yarp_os;

	};

}
#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* YARPFIRINGWRITER_HPP_ */
