#ifndef AKDYOLOINBOUNDCHANNEL_HPP_
#define AKDYOLOINBOUNDCHANNEL_HPP_


//CARLsimIO includes
#include <CARLsimIO/Channel/InboundChannel/InboundChannel.hpp>
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/NeuronSim/IzhikevichNeuronSim.hpp>

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

#include <CARLsimIO/AkdYoloEvent.hpp>


namespace carlsimio {

	class AkdYoloReader; // forward

	/** Converts a joint angle input into a spike pattern */
	class CARLSIMIO_API AkdYoloInboundChannel : public InboundChannel {
		public:
			AkdYoloInboundChannel();
			virtual ~AkdYoloInboundChannel();
			const vector<unsigned>& getFiring() { return neuronSim.getSpikes(); }
			const vector<double>& getCurrentVector() { return neuronSim.getCurrentVector(); }
			void initialize(Reader* reader, map<string, Property>& properties);
			void setProperties(map<string, Property>& properties);
			void step();

			IzhikevichNeuronSim neuronSim;

			AkdYoloReader* getReader() {return m_reader;}

			// New Diagnostic Interface
			virtual size_t getBufferQueued();  // Delegate to specific Buffer
			virtual unsigned long long getBufferNext();  // Delegate to specific Buffer
			virtual void reset();
			virtual size_t getReaderProcessed(); 
			virtual unsigned long long getReaderLast();  

		private:
			//==========================  VARIABLES  ==========================
			
			/** each event has a certain amount of values probability, direction, distance.. */
			unsigned int values;		// most likely constant, set in the constructor

			/*! reserved for symbolic class names */
			size_t m_securities;		

			/*! size of classification vector of the yolo event */
			size_t m_ints; 

			/*! size of scalar value vector of the yolo event */
			size_t m_doubles;

			/*! Reads event values from data source (YARP, File) */
			AkdYoloReader* m_reader;

			/*! Min/Max Boundaries of the ticks. Loaded by the reader during initialization. */
			vector<yolo_event_t> m_minValues;    
			vector<yolo_event_t> m_maxValues;

			/*! Lifespan of a tick in ms. For now a parameter like sd but might be a formular 
			    with respect to statistical values retrieved by the Reader, like avg/min/max Lifespan
			 */
			int m_lifespan; 

			// TODO: ENUM_Enconding -> default: Poplation, ..

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

			// figure out, if changed
			unsigned long long m_last_ms; 
			unsigned long long m_last_simtime; // dopamine decay
			int m_last_type;

			// to be able to reset logs
			string m_spikesLogName; 

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* AKDYOLOINBOUNDCHANNEL_HPP_ */
