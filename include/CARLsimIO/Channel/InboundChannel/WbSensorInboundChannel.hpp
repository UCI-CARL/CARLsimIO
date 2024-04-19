#ifndef WBSENSORINPUTCHANNEL_HPP_
#define WBSENSORINPUTCHANNEL_HPP_

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



namespace carlsimio {

	class WbSensorReader; // forward

	enum WbSensorType {Distance, Light};  // the actual Wb Sensors 

	/** Converts the values of a array of Wb Sensor array into a spike pattern train  
	* CAUTION: the size of the array (and potentially value) is defined at runtime by a parameter 
	*/

	class CARLSIMIO_API WbSensorInboundChannel : public InboundChannel {
	public:
		WbSensorInboundChannel();
		virtual ~WbSensorInboundChannel();
		const vector<unsigned>& getFiring() { return neuronSim.getSpikes(); }
		const vector<double>& getCurrentVector() { return neuronSim.getCurrentVector(); }
		void initialize(Reader* reader, map<string, Property>& properties);
		void setProperties(map<string, Property>& properties);
		void step();

		IzhikevichNeuronSim neuronSim;

		WbSensorReader* getReader() { return m_reader; }

		// New Diagnostic Interface
		virtual size_t getBufferQueued();  // Delegate to specific Buffer
		virtual unsigned long long getBufferNext();  // Delegate to specific Buffer
		virtual void reset();
		virtual size_t getReaderProcessed();
		virtual unsigned long long getReaderLast();

	private:
		//==========================  VARIABLES  ==========================

		string m_sensorPrefix; 



		/** each sensor vector values e.g. 8 DistanceSensors ps0..ps7 */
		unsigned int values;		// most likely constant, set in the constructor

		/*! Reads tick values from data source (YARP, File) */
		WbSensorReader* m_reader;

		/*! Min/Max Boundaries of the Sensor array. Loaded by the reader during initialization. */

		int m_minValue;
		int m_maxValue;


		/*! Lifespan of a sensor input in ms. This corresponds to the time step set for the sensor,
		 * e.g. 64 ms for E-Puck2.  It might be .. model time speed, if sim step > 1 ms.
		 * e.g. 16 ms => the m_lifespan becomes eff. 4 ms  (SNN simulator time)
		 */
		int m_lifespan;


		/** Standard deviation */
		double standardDeviation;

		/** Values at centre of each neuron's perceptual field 
		 * each sensor is scaled by the reader into integer to have a defined resolution
		 * e.g. min E-puck2  velocity has a min and max  
		 * => a 1024 are sufficient to map this value range , alternatively a interval 
		 * e.g. RGB leds   4 values map to 4 well defined colors
		 */
		vector<double> neuronValues;
		//ContainerT neuronValues;

		/** Normal distribution */
		vector<boost::math::normal_distribution<double>> normalDistribution;


		/** Factor by which input current to neurons is multiplied */
		vector<double> currentFactor;

		/** Constant current injected into neurons */
		double constantCurrent;

		/** Peak current - sets maximum current into neurons. */
		double peakCurrent;

		//==============================  METHODS  =========================
		void updateProperties(map<string, Property>& properties);

	};

}



#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* WBSENSORINPUTCHANNEL_HPP_ */
