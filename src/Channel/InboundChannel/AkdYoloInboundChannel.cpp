
//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Channel/InboundChannel/AkdYoloInboundChannel.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Reader/AkdYoloReader.hpp>
#include <CARLsimIO/Log/Log.hpp>


#include <boost/math/distributions/normal.hpp>

#include <carl/boost_ext.hpp>

#include <yarp/carl/all.h>
#include <yarp/carl/StlUtils.h> 


using namespace carlsimio;

//Local debugging information
//#define DEBUG
//#define RECORD_TIMING

//Names of properties
//#define SECURITIES_NAME		"Securities"
#define PROB_MIN_NAME		"Probability Min"
#define PROB_MAX_NAME		"Probability Max"
#define DIR_MIN_NAME	"Direction Min"
#define DIR_MAX_NAME	"Direction Max"
#define DIST_MIN_NAME		"Distance Min"
#define DIST_MAX_NAME		"Distance Max"
#define CLASS_MIN_NAME		"Class Min"
#define CLASS_MAX_NAME		"Class Max"
#define YOLO_LIFESPAN_NAME   "Yolo Lifespan" 

#define STANDARD_DEVIATION_NAME "Standard Deviation"
#define SPIKES_LOG_NAME			"Spikes Log"

#define NEURON_HEIGHT_NAME "Neuron Height"
#define PARAM_A_NAME "Parameter A"
#define PARAM_B_NAME "Parameter B"
#define PARAM_C_NAME "Parameter C"
#define PARAM_D_NAME "Parameter D"
#define PEAK_CURRENT_NAME "Peak Current"
#define CONSTANT_CURRENT_NAME "Constant Current"
#define BACKEND_NAME "Backend"

/** Default constructor, initialises the default channel properties */
AkdYoloInboundChannel::AkdYoloInboundChannel() {

	//! Default for YARP
	//addProperty(Property("EURUSD@Alpari-MT5", SECURITIES_NAME, "List of securities to be transformed into spike patterns", true));
	// Default for File
	//addProperty(Property("EURUSD@Alpari, USDCHF@Alpari, DE30@XTB", "Securities", "List of securities to be transformed into spike patterns", true));	

	//! Neu 17.09.2020  STATE World, Portfolio, TE
	// Existiert eine Position (LONG/SHORT) oder ein pending ORDER?
	// => Entscheidungsmatrix:  IDLE, FILLED_LONG, FILLED_SHORT, PENDING_LONG, PENDING_SHORT  => Wieder 5
	// State hat Dauer-Feuer, siehe Filter-Matrix, Minsky Unterdrücker/Zensoren

	//! Default Maxima per Security - For TradeEvents those values are not loaded from the GeneDb
	addProperty(Property("0.85;", PROB_MIN_NAME, "Lower Bound of Probability", true));
	addProperty(Property("1.00;", PROB_MAX_NAME, "Upper Bound of Probability", true));
	addProperty(Property("0.0;", DIR_MIN_NAME, "Lower Bound of Direction", true));
	addProperty(Property("1.0;", DIR_MAX_NAME, "Upper Bound of Direction", true));
	addProperty(Property("0.18;", DIST_MIN_NAME, "Lower Bound of Distance", true));
	addProperty(Property("0.50;", DIST_MAX_NAME, "Upper Bound of Distance", true));

	//! Ranges for Types (valid for all Securities) Values see ITradeEventHandler.h
	//! Using the symbolic names would be nicer but integer are more generic
	addProperty(Property(Property::Integer, 6, CLASS_MIN_NAME, "First Event Class", true));
	addProperty(Property(Property::Integer, 6, CLASS_MAX_NAME, "Last Event Class", true));

	addProperty(Property(Property::Integer, 2, NEURON_HEIGHT_NAME, "Height of the neuron network, Precision for the scalar values", true));  // Precision
	addProperty(Property(Property::Double, 0.25, STANDARD_DEVIATION_NAME, "The standard deviation as a percentage of neurons covered", true));
		
	addProperty(Property(Property::Double, 0.1, PARAM_A_NAME, "Parameter A of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, 0.2, PARAM_B_NAME, "Parameter B of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, -65.0, PARAM_C_NAME, "Parameter C of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, 2.0, PARAM_D_NAME, "Parameter D of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, 40.0, PEAK_CURRENT_NAME, "Maximum current that will be injected into neuron", true));
	addProperty(Property(Property::Double, 0.0, CONSTANT_CURRENT_NAME, "This value is added to the incoming current", false));
	addProperty(Property(Property::Integer, 1, BACKEND_NAME, "0: CurrentRelay, 1: Spikes by embedded CPU, 2: Spikes by CARLsim, 3: Spikes by NeMo, 4: Spikes by embedded GPU", false));  // featCurrentVector

	/*! \todo: Property, respectivly get from FinGeneDB:  avg(ticks)/sec, what is Q(85) -> fractal trading time
	 * e.g. avg EURUSD = 150ms, but during hot markets (x-Quantile) every 25ms a new tick => m_lifespan(EURUSD)=25ms
	 * fast = 2 ticks per seconds  ==> lifespan ~500ms
	 * min = 50ms
	 * 150ms : face recognition
	 * 200ms good to calculate and debug
     */
	addProperty(Property(Property::Integer, 200, YOLO_LIFESPAN_NAME, "How long the tick induces its spike pattern before it vanishes into oblivion", true));

	// Refactor cmd finspikes logging for SpikeStream FinSpikes-Plugin
	addProperty(Property("null", SPIKES_LOG_NAME, "Log file for Spikes", false));

	//Create the description
	channelDescription = Description("Yolo Inbound Channel", "This is a Yolo Event inbound channel", "Yolo Reader");

	//Initialize variables
	m_reader = NULL;

	height = 0; // set in initialize
	width = 0; // set in initialize

	m_last_ms = 0L;
	m_last_type = 0;

	m_last_simtime = 0L;

}


/*! Destructor */
AkdYoloInboundChannel::~AkdYoloInboundChannel(){

	if(m_reader != NULL)
		delete m_reader;


}




/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

//Inherited from InputChannel
void AkdYoloInboundChannel::initialize(Reader* reader, map<string, Property>& properties){

	//This class requires an tick reader, so cast it and check
	this->m_reader = dynamic_cast<AkdYoloReader*>(reader);			
	if(this->m_reader == NULL)
		throw CARLsimIOException("Cannot initialize AkdYoloInboundChannel with a null reader.");


	m_securities = 1; // reserved
	m_ints = AkdYoloEventInts;
	m_doubles = AkdYoloEventDoubles;

	this->width = m_ints + m_doubles; 

	//Start the reader thread running
	m_reader->start();			// X ISSUE: why initialize and not channel.start() ???
								// |==> No Issue: The Framework is build so, same is true for Yarp Reader
								// (+) Min/Max Array is needed below
		// TODO:  Here must be waited for the reader thread until it is ready (e.g. yarp connections are set up ...)!!!
		// ISSUE: What for ?, thread terminate if no polling !!!

	const vector<double> &min_prob = Common::doublesFromString(properties.at(PROB_MIN_NAME).getString());
	const vector<double> &max_prob = Common::doublesFromString(properties.at(PROB_MAX_NAME).getString());
	const vector<double> &min_dir = Common::doublesFromString(properties.at(DIR_MIN_NAME).getString());
	const vector<double> &max_dir = Common::doublesFromString(properties.at(DIR_MAX_NAME).getString());
	const vector<double> &min_dist = Common::doublesFromString(properties.at(DIST_MIN_NAME).getString());
	const vector<double> &max_dist = Common::doublesFromString(properties.at(DIST_MAX_NAME).getString());

	m_minValues.resize(m_securities);
	m_maxValues.resize(m_securities);

	for(int sec_i=0;sec_i<m_securities;sec_i++) 
	{
		//! Doubles
		m_minValues[sec_i].get<1>().at(0) = min_prob.at(sec_i);
		m_minValues[sec_i].get<1>().at(1) = min_dir.at(sec_i);
		m_minValues[sec_i].get<1>().at(2) = min_dist.at(sec_i);

		m_maxValues[sec_i].get<1>().at(0) = max_prob.at(sec_i);
		m_maxValues[sec_i].get<1>().at(1) = max_dir.at(sec_i);
		m_maxValues[sec_i].get<1>().at(2) = max_dist.at(sec_i);		

		//! Integers
		m_minValues[sec_i].get<0>().at(0) = properties.at(CLASS_MIN_NAME).getInt();
		m_maxValues[sec_i].get<0>().at(0) = properties.at(CLASS_MAX_NAME).getInt();

	}

	//! \todo ISSUE 15.09.2013 \bug This should rather take place _before_ ranges initizied from \sa OrderOutboundChannel.cpp
	//Update and store properties
	updateProperties(properties);
	
	// 20.03.2013 moved from Reader ot Channel as parameter. The actual value is still used by the TickReader.
	// Future implementation might require values retrieved by the Reader (ranges) and applied to a formular parameter in the param
	//m_lifespan = properties.at(TICK_LIFESPAN_NAME).getInt();  // utilize updateProperties instead
	m_reader->setTickLifespan(m_lifespan);
	LOG(LOG_DEBUG) << "Lifespan set on Reader: " << m_lifespan;

	if(height < 2)   // see TickInboundChannel for details
		throw CARLsimIOException("InputReader must use two or more neurons");


	double min, max;
	double mean;
	double valueDist; 
	double sdValue;
	size_t index;
	for(size_t iSecurity=0; iSecurity<m_securities; iSecurity++) {
		for(size_t double_i=0; double_i < m_doubles; double_i++) {
			// linear index reagarding to the width dimension of the neuron matrix
			index = iSecurity*(m_doubles+m_ints) + double_i;
			min = m_minValues.at(iSecurity).get<1>().at(double_i);
			max = m_maxValues.at(iSecurity).get<1>().at(double_i);

			//  Value covered by each neuron 			
			valueDist = (max - min) / double(height - 1);  // see TickInboundChannel for details

			 // Standard deviation expressed in value  ISSUE: SD normally expressed for normalized (1.0) distribution => must be scaled
			sdValue = standardDeviation * valueDist;
			mean = (min+max)/2.0; 

			// Create normal distribution and calculate current factor
			// ISSUE: mean is only 0.0 if min = -max !!! => Error in the implementation, proof by measuring the error in the range 0..100
			// CAUTION Order is essential !, always securities first, than their values...
			normalDistribution.push_back(boost::math::normal_distribution<double>(0.0, sdValue));  // ISSUE: 0.0 or mean, Assumption: 
// No, this seems not to work at all ...  LN 13.05.2013
//			normalDistribution.push_back(boost::math::normal_distribution<double>(mean, sdValue));  
			currentFactor.push_back(peakCurrent / pdf(normalDistribution[index], 0.0)); 

			// populate the angles
			for(unsigned j=0; j<this->height; ++j) {
				neuronValues.push_back(min + j * valueDist);		// this is wrong !!!,   [19]=65.1107 anstatt max=69.1047  => nicht peek sondern falscher wert
//				neuronValues.push_back(min + (j+1) * valueDist);		// this is wrong !!!,   
			}
		}

		for(size_t int_i=0; int_i < m_ints; int_i++) 
		{
			index = iSecurity*(m_doubles+m_ints) + m_doubles + int_i;
			min = m_minValues.at(iSecurity).get<0>().at(int_i);
			max = m_maxValues.at(iSecurity).get<0>().at(int_i);


			switch(int_i) {
				case 0:
					// min,max defined by yarp.. enum 0, size .. 
					break;					
			}
			// populate the enum values / classification
//! \bug 15.09.2013 This produces a sparse array which was not wanted! 
//!		=> transformation min/max must take place somewhere else, probabliy at the value mapping
//! Alternative: always to up to height, min value = .. if exceeds max put 0
			for(unsigned j=(unsigned)min; j<this->height && j<(unsigned)max; ++j) {
				neuronValues.push_back(j);
			}
		}

	}

	//Set up Izhikevich simulation
	neuronSim.initialize(size()); 



	setInitialized(true);

	/*! Header require the correct size of neurons. The Parameter was set in update but 
		the channel is not initialized at this time. */ 
	if(spikesLog_os != NULL) 
		neuronSim.logFiringsHeader(*spikesLog_os, 0,  0, getHeight(), (int) (m_ints+m_doubles));  // TickInd

}


//Inherited from Channel. This will be done immediately if we are not stepping or deferred until the end of the step */
void AkdYoloInboundChannel::setProperties(map<string,Property>& properties){
	updateProperties(properties);
}

// Global
static yarp::carl::IsoTimestamp m_isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst());

//Inherited from InputChannel
void AkdYoloInboundChannel::step(){

	//Check reader for errors
	if(m_reader->isError()){
		LOG(LOG_CRITICAL)<<"YoloReader Error: "<<m_reader->getErrorMessage();
		throw CARLsimIOException("Error in YoloReader");
	}


	// ISSUE: as in main before .. ?
	if(spikesLog_os != NULL) 
		neuronSim.logFirings(*spikesLog_os, 0, m_simtime, getHeight(), (int) (m_ints + m_doubles) ); 


//! \todo implement for TradeEvent
//

	m_reader->syncSim(m_simtime);		//! \todo challenge pending due to lag



		
	double min, max;
	double value;
	size_t w_index;
	int n_index;
	yolo_event_t trade_event;

	for(int iSecurity=0; iSecurity<m_securities; iSecurity++)
	{

		m_reader->getTuple(trade_event, m_simtime);

		// Double Scalar Vector 
		for(size_t double_i=0; double_i < m_doubles; double_i++) {

			// linear index reagarding to the width dimension of the neuron matrix
			w_index = iSecurity*(m_doubles+m_ints) + double_i;
			//min = m_minValues.at(iSecurity).get<1>().at(double_i);
			//max = m_maxValues.at(iSecurity).get<1>().at(double_i);
			min = m_minValues.at(0).get<1>().at(double_i);
			max = m_maxValues.at(0).get<1>().at(double_i);
			value = trade_event.get<1>().at(double_i);

			if( std::abs(value) < 0.00001 )
				continue; // ignore only the specific tuple element, not the whole !!!
			else
			if(value > max)
				value = max;
			else 
			if(value < min)
				value = min;

			//Set input currents to neurons
			for(unsigned h_index = 0; h_index < this->height; ++h_index)	{
// Linear like it should be, ..
				n_index = (int) w_index * this->height + h_index; 
// ISSUE: 13.05.2013: adoption for spikestream, iterator, position -->  z is incremented first, than x
				neuronSim.setInputCurrent(n_index, 
					constantCurrent + currentFactor[w_index] * pdf(normalDistribution[w_index], neuronValues[n_index]-value) );
			}
		}

		for(size_t int_i=0; int_i < m_ints; int_i++)  
		{
			w_index = iSecurity*(m_doubles+m_ints) + m_doubles + int_i;
			switch(int_i) {
				case 0:
					min = m_minValues.at(iSecurity).get<0>().at(int_i);
					max = m_maxValues.at(iSecurity).get<0>().at(int_i);
					value = trade_event.get<0>().at(int_i);
					if (value == 0)
						continue;  // 0 is an invalid value, classes start with 1 
					if (value < min || value > max)
						value = 0;
					break;
			}
			//Set input currents to neurons
			for(unsigned h_index = 0; h_index < this->height; ++h_index) {
				n_index = (int) w_index * this->height + h_index; 
				bool on = value == 0 && h_index == 0 || h_index == value - min + 1; 
				neuronSim.setInputCurrent(n_index, 
					constantCurrent + peakCurrent * (on ? 1.0 : 0.0 ));
			}
		}

		m_last_ms = m_reader->getLast_ms();


	}


	//Step the simulator
	neuronSim.step();
}


/*--------------------------------------------------------------------*/
/*---------              PROTECTED METHODS                     -------*/
/*--------------------------------------------------------------------*/

/**  Updates the properties.
	If UpdateReadOnly is set to true only the read only properties will be updated */
void AkdYoloInboundChannel::updateProperties(map<string, Property>& properties) {

	if(propertyMap.size() != properties.size())
		throw CARLsimIOException("AkdYoloInputChannel: Current properties do not match new properties.");

	updatePropertyCount = 0;

	for(map<string, Property>::iterator iter = properties.begin(); iter != properties.end(); ++iter)  {
		//In updateReadOnly mode, only update properties that are not read only
		if( ( isInitialized() && !propertyMap[iter->first].isReadOnly() ) || !isInitialized()) {
			string paramName = iter->second.getName();
			switch (iter->second.getType()) {
				case Property::Integer: {
					if (paramName == CLASS_MIN_NAME)
						updateIntegerProperty(iter->second);
					else if (paramName == CLASS_MAX_NAME)
						updateIntegerProperty(iter->second);
					else if (paramName == NEURON_HEIGHT_NAME)
						setHeight(updateIntegerProperty(iter->second));
					else if (paramName == YOLO_LIFESPAN_NAME)
						m_lifespan = updateIntegerProperty(iter->second);
					else if (paramName == BACKEND_NAME)
						neuronSim.setParameterBackend(updateIntegerProperty(iter->second));
				}
				break;
				case Property::Double:  {
					if (paramName == PARAM_A_NAME)
						neuronSim.setParameterA(updateDoubleProperty(iter->second));
					else if (paramName == PARAM_B_NAME)
						neuronSim.setParameterB(updateDoubleProperty(iter->second));
					else if (paramName == PARAM_C_NAME)
						neuronSim.setParameterC(updateDoubleProperty(iter->second));
					else if (paramName == PARAM_D_NAME)
						neuronSim.setParameterD(updateDoubleProperty(iter->second));
					else if (paramName == PEAK_CURRENT_NAME)
						peakCurrent = updateDoubleProperty(iter->second);
					else if (paramName == CONSTANT_CURRENT_NAME)
						constantCurrent = updateDoubleProperty(iter->second);
					else if(paramName == STANDARD_DEVIATION_NAME)
						standardDeviation = updateDoubleProperty(iter->second);
				}
				break;
				case Property::Combo: break;
				case Property::String:
					//if(paramName == SECURITIES_NAME) {
					//	string &tmpSecurities = updateStringProperty(iter->second); // satisfy the invariant and store 
					//	LOG(LOG_DEBUG) << "Security Property: " << tmpSecurities;   // see ::initialize how the value is transfered to the Reader																					// in the Channel only the amount of securities is relevant
					//} else 
					if(paramName == PROB_MIN_NAME) {
						string tmp = updateStringProperty(iter->second); 
						LOG(LOG_DEBUG) << "Profit Min Property: " << tmp;
					} else
					if(paramName == PROB_MAX_NAME) {
						string tmp = updateStringProperty(iter->second); 
						LOG(LOG_DEBUG) << "Profit Max Property: " << tmp;
					} else
					if(paramName == DIR_MIN_NAME) {
						string tmp = updateStringProperty(iter->second); 
						LOG(LOG_DEBUG) << "Percent Min Property: " << tmp;
					} else
					if(paramName == DIR_MAX_NAME) {
						string tmp = updateStringProperty(iter->second); 
						LOG(LOG_DEBUG) << "Percent Max Property: " << tmp;
					} else
					if(paramName == DIST_MIN_NAME) {
						string tmp = updateStringProperty(iter->second); 
						LOG(LOG_DEBUG) << "Points Min Property: " << tmp;
					} else
					if(paramName == DIST_MAX_NAME) {
						string tmp = updateStringProperty(iter->second); 
						LOG(LOG_DEBUG) << "Points Max Property: " << tmp;
					} else
					if(paramName == SPIKES_LOG_NAME) {
						m_spikesLogName = updateStringProperty(iter->second);
						setSpikesLog(m_spikesLogName);
						if(isInitialized() && spikesLog_os != NULL)
							neuronSim.logFiringsHeader(*spikesLog_os, 0,  0, getHeight(), AkdYoloEventInts+AkdYoloEventDoubles);  
					//} else
					//if(paramName == DOPAMINE_LOG_NAME) {
					//	m_dopamineLogName = updateStringProperty(iter->second);
					//	setDopamineLog(m_dopamineLogName);
					}
					break;
				default:
					throw CARLsimIOException("Property type not recognized.");
			}
		}
	}

	//Check all properties were updated 
	if(!isInitialized() && updatePropertyCount != propertyMap.size())
		throw CARLsimIOException("Some or all of the properties were not updated: ", updatePropertyCount);
}


// New Diagnostic Interface
size_t AkdYoloInboundChannel::getBufferQueued() { return m_reader->getQueued(); }  // Delegate to specific Buffer
unsigned long long AkdYoloInboundChannel::getBufferNext() { return m_reader->getNext_ms(); }  // Delegate to specific Buffer

void AkdYoloInboundChannel::reset() { m_reader->reset(); }

size_t AkdYoloInboundChannel::getReaderProcessed() { return m_reader->getProcessed(); }  // Delegate to specific Buffer
unsigned long long AkdYoloInboundChannel::getReaderLast() { return m_reader->getLast_ms(); }  // Delegate to specific Buffer
