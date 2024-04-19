#ifndef FIRINGINBOUNDCHANNEL_HPP_
#define FIRINGINBOUNDCHANNEL_HPP_

//CARLsimIO includes
#include <CARLsimIO/Channel/InboundChannel/InboundChannel.hpp>
#include <CARLsimIO/Common.hpp>  

//Other includes
#include <boost/math/distributions/normal.hpp>
#include <string>
#include <vector>
#include <map>
using namespace std;

#include <CARLsimIO/api.hpp>	

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	class FiringReader; // forward

	class CARLSIMIO_API FiringInboundChannel : public InboundChannel {
		public:
			FiringInboundChannel();
			virtual ~FiringInboundChannel();
			const vector<unsigned>& getFiring() { return firing;  }
			const vector<double>& getCurrentVector() { return currentVector; }
			void initialize(Reader* reader, map<string, Property>& properties);
			
			/*! Not supposed to be reimplemented */
			void setDopamineLog(const string logName);

			/** Sets */
			void setSparseCoding(int sparseCoding) { this->sparseCoding = sparseCoding; }

			/** Sets */
			void setAxonalDelay(int axonalDelay) { this->axonalDelay = axonalDelay; }

			/** Sets the axonal tract of the Channel */
			void setAxonalTract(const std::string axonalTract) { this->axonalTract = axonalTract; }

			void setProperties(map<string, Property>& properties);
			void step();


			FiringReader* getReader() {return m_reader;}

			// New Diagnostic Interface
			virtual size_t getBufferQueued();  // Delegate to specific Buffer
			virtual unsigned long long getBufferNext();  // Delegate to specific Buffer
			virtual void reset();
			virtual size_t getReaderProcessed(); 
			virtual unsigned long long getReaderLast();  


		private:
			//==========================  VARIABLES  ==========================
			
			/** each tick has a certain amount of values aks, bid, spread, volumne, ..*/
			unsigned int values;		// most likely constant, set in the constructor

			/*! In this context, only the amount of securities is releveant. The meaning is already covered by the reader,
			 * that is initialized before the channel. However the definition takes place as a property due to design of spikestream
			 */
			size_t m_securities;		// depends on the configuration (e.g. Yarp, Device, Config, or .. -> CARLsimIO is a YARP Device?
			size_t m_doubles;	// size of scalar value vector of a trade event
			size_t m_ints;		// size of classification vector of a trade event
								// => hight >= m_ints

			/*! Reads tick values from data source (YARP, File) */
			FiringReader* m_reader;

			///*! Min/Max Boundaries of the ticks. Loaded by the reader during initialization. */
			//vector<ncboost::trade_event_t> m_minValues;    
			//vector<ncboost::trade_event_t> m_maxValues;

			/*! Lifespan of a tick in ms. For now a parameter like sd but might be a formular 
			    with respect to statistical values retrieved by the Reader, like avg/min/max Lifespan
			 */
			int m_lifespan; 

			// ENUM_Enconding -> default: Poplation, ..

			/** Standard deviation */
			double standardDeviation; 

			/** Values at centre of each neuron's perceptual field */
			vector<double> neuronValues; 

			/** Normal distribution */
			vector<boost::math::normal_distribution<double>> normalDistribution;

			/*! Izhikevich neuron simulator */
//			IzhikevichNeuronSim neuronSim;

			/** Factor by which input current to neurons is multiplied */
			vector<double> currentFactor;

			/** Constant current injected into neurons */
			double constantCurrent;

			/** Peak current - sets maximum current into neurons. */
			double peakCurrent;

			//==============================  METHODS  =========================
			void updateProperties(map<string, Property>& properties);


			//! Parameter for Neurotransmitter modulated STDP Learning
			double m_dopamine_decay;
			double m_synaptic_decay;
			double m_penalty;
			double m_reward;
			double m_encouragement;
			double m_disproval;
			double m_init_da_level;
			double m_max_da_level;
			int m_rewarded_group;

			//! Actual Dopamine Level 
			double m_dopamine;

			// figure out, if changed
			unsigned long long m_last_ms; 
			unsigned long long m_last_simtime; // dopamine decay
			int m_last_type;

			// Generic Implementation of En/De-Coding Logging
			ostream *dopamineLog_os;
			ofstream *dopamineLog_ofs;

			// to be able to reset logs
			string m_spikesLogName; 
			string m_dopamineLogName;

			// patch
			vector<unsigned> firing;
			vector<double> currentVector;


			int sparseCoding;
			int axonalDelay;
			string axonalTract;
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* FIRINGINBOUNDCHANNEL_HPP_ */
