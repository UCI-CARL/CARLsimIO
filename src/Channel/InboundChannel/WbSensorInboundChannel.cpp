
#ifdef CARLSIMIO_USE_OPENMP
#include <omp.h>
#endif

//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Channel/InboundChannel/WbSensorInboundChannel.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Reader/WbSensorReader.hpp>
#include <CARLsimIO/Log/Log.hpp>

using namespace carlsimio;

//Names of properties
#define SENSOR_PREFIX_NAME "Sensor Prefix"  // replaces "security"   ps for ps0,ps1,..,ps7  ls for ls0,..,ls7,  tof for tof, gyro for gyro, wheel for wheel0,wheel1 (left,right)
#define SENSOR_LIFESPAN_NAME "Sensor Lifespan" 
#define STANDARD_DEVIATION_NAME "Standard Deviation"  

#define NEURON_HEIGHT_NAME "Neuron Height"
#define PARAM_A_NAME "Parameter A"
#define PARAM_B_NAME "Parameter B"
#define PARAM_C_NAME "Parameter C"
#define PARAM_D_NAME "Parameter D"
#define PEAK_CURRENT_NAME "Peak Current"
#define CONSTANT_CURRENT_NAME "Constant Current"
#define SPIKES_LOG_NAME "Spikes Log"    // Rename to Log -> Spikes or Current
#define BACKEND_NAME "Backend"


/** Default constructor, initialises the default channel properties */
//  < Distance, int, 8, array<int, 8>>
WbSensorInboundChannel::WbSensorInboundChannel() {

	addProperty(Property("tof", SENSOR_PREFIX_NAME, "Name of the sensor array: accl, gyro, ls, ps, tof, whls", true));  // translates into yarp::os::Vocab of the yarp reader

 	addProperty(Property(Property::Integer, 1, NEURON_HEIGHT_NAME, "Height of the neuron network, Precision for the scalar values", true));  // Precision  Fixed to 1 
	addProperty(Property(Property::Double, 0.5, STANDARD_DEVIATION_NAME, "The standard deviation as a percentage of neurons covered", true));

	addProperty(Property(Property::Double, 0.1, PARAM_A_NAME, "Parameter A of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, 0.2, PARAM_B_NAME, "Parameter B of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, -65.0, PARAM_C_NAME, "Parameter C of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, 2.0, PARAM_D_NAME, "Parameter D of the Izhikevich Neuron Model", false));
	addProperty(Property(Property::Double, 80.0, PEAK_CURRENT_NAME, "Maximum current that will be injected into neuron", true));  // default for tof
	addProperty(Property(Property::Double, 0.0, CONSTANT_CURRENT_NAME, "This value is added to the incoming current", false));
	addProperty(Property(Property::Integer, 1, BACKEND_NAME, "0: CurrentRelay, 1: Spikes by embedded CPU, 2: Spikes by CARLsim", false));  // featCurrentVector

	// tbd  1ms or 64/16 = 4ms or 16 or 64ms 
	addProperty(Property(Property::Integer, 4, SENSOR_LIFESPAN_NAME, "How long the tick induces its spike pattern before it vanishes into oblivion", true));  // 4 x 16 = 64

	// Refactor cmd finspikes logging for SpikeStream FinSpikes-Plugin
	addProperty(Property("null", SPIKES_LOG_NAME, "Log file for Spikes", false));

	//Create the description
	channelDescription = Description("WbSensor Inbound Channel", "This is a WbSensor array inbound channel", "WbSensor Reader");
	
	//Initialize variables
	m_reader = NULL;
}


/*! Destructor */
WbSensorInboundChannel::~WbSensorInboundChannel() {
	if (m_reader != NULL)
		delete m_reader;
}




/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

//Inherited from InputChannel
//<Distance, int, 8, array<int,8>>
void WbSensorInboundChannel::initialize(Reader* reader, map<string, Property>& properties) {

	//This class requires an tick reader, so cast it and check
	this->m_reader = dynamic_cast<WbSensorReader*>(reader);			
	if (this->m_reader == NULL)
		throw CARLsimIOException("Cannot initialize WbSensor InboundChannel with a null reader.");

	// Transformation of the String Property to Securities and handover to Reader
	const string& sensorPrefix = properties.at(SENSOR_PREFIX_NAME).getString();
	if (sensorPrefix == "ps" || sensorPrefix == "ls")
		this->values = 8;
	else
	if (sensorPrefix == "gyro" || sensorPrefix == "accl")
		this->values = 3;
	else
	if (sensorPrefix == "whls")
		this->values = 2;
	else
	if (sensorPrefix == "tof")
		this->values = 1;
	else
		throw CARLsimIOException("Cannot initialize WbSensor InboundChannel with unsupported prefix.");
	m_reader->setSensorPrefix(sensorPrefix);

	m_reader->setTupleSize(this->values);  

	this->width = this->values; 


	//Start the reader thread running
	m_reader->start();			
	m_reader->getMinMax(m_minValue, m_maxValue);
	if (!(m_minValue >= 0 && m_minValue < m_maxValue) ) 
		throw CARLsimIOException("Reader has empty value range.");

	//Update and store properties
	updateProperties(properties);

	m_reader->setTickLifespan(m_lifespan);

	LOG(LOG_DEBUG) << "SensorValue Lifespan set on Reader: " << m_lifespan;

	if (height == 1) {	// for simple coding
	

		for (int iValue = 0; iValue < this->values; iValue++) {

			// linear mapping
			currentFactor.push_back(peakCurrent - constantCurrent);
			
		}

	} else {
		//throw CARLsimIOException("Channel must use one neurons");

		double valueDist = (m_maxValue - m_minValue) / double(height - 1);
		double sdValue = standardDeviation * valueDist;
		double mean = (m_minValue + m_maxValue) / 2.0;

		for (int iValue = 0; iValue < this->values; iValue++) {

			normalDistribution.push_back(boost::math::normal_distribution<double>(0.0, sdValue));  

			currentFactor.push_back((peakCurrent - constantCurrent) / pdf(normalDistribution[iValue], 0.0));

			for (unsigned j = 0; j < this->height; ++j) {
				neuronValues.push_back(m_minValue + j * valueDist);
			}
		}

	}


	//Set up Izhikevich simulation
	neuronSim.initialize(size());

	setInitialized(true);

	/*! Header require the correct size of neurons. The Parameter was set in update but
		the channel is not initialized at this time. */
	if (/*isInitialized() &&*/ spikesLog_os != NULL)
		neuronSim.logFiringsHeader(*spikesLog_os, 0, 0, 8, -1);   // Common::TupleSize  --> Array size == values  getHeight() --> 1

}



//Inherited from Channel. This will be done immediately if we are not stepping or deferred until the end of the step */
//<Distance, int, 8, array<int, 8>>
void WbSensorInboundChannel::setProperties(map<string, Property>& properties) {
	updateProperties(properties);
}



//Inherited from InputChannel
//<Distance, int, 8, array<int, 8>>
void WbSensorInboundChannel::step() {

	//Check reader for errors
	if (m_reader->isError()) {
		LOG(LOG_CRITICAL) << "WbSensorReader Error: " << m_reader->getErrorMessage();
		throw CARLsimIOException("Error in WbSensorReader");
	}

	// ISSUE: as in main before .. ?
	if (spikesLog_os != NULL)
		neuronSim.logFirings(*spikesLog_os, 0, m_simtime, 8, -1);

	m_reader->syncSim(m_simtime);

	vector<int> tick(values, 0);  // should be cached as instance variable, set in ::initialize
	m_reader->getTuple(tick, m_simtime);
	bool isEmptyTuple = true; 


	vector<int> perm_dst = { 4, 6, 7, 5, 3, 1, 0, 2 };
	vector<int> *perm = nullptr; 
	//if (sensorPrefix == "ps" || sensorPrefix == "ls")
	if (this->values == 8)
		perm = &perm_dst;
		

	for (int iValue = 0; iValue < values; iValue++)
	{

		// musst be inside for OpenMP
		double value = tick[iValue];
		double min = m_minValue;
		double max = m_maxValue;

		// linear index reagarding to the width dimension of the neuron matrix
		int w_index = iValue;
		if (perm)
			w_index = (*perm)[iValue];

		if (std::abs(value) < 0.000005) {
			// reset current in CARLsim
			continue; // ignore empty tuple element
		}
		else
			if (value > max) {
				value = max;
			}
			else
				if (value < min) {
					value = min;
				}
		isEmptyTuple = false;

		if (height == 1) {


			

			double current = constantCurrent + currentFactor[w_index] * (value - min)/(max - min);
			if (current > 0)
				neuronSim.setInputCurrent(w_index, current);

		}
		else {

			for (int h_index = 0; h_index < this->height; ++h_index) {
				int n_index = w_index * this->height + h_index;
				neuronSim.setInputCurrent(n_index,
					constantCurrent + currentFactor[w_index] * pdf(normalDistribution[w_index], neuronValues[n_index] - value));
			}

		}
	}

	// reset firing activity (buffer empty)
	if (isEmptyTuple) {
		for (int iValue = 0; iValue < values; iValue++)
		{
			int w_index = iValue;
			for (int h_index = 0; h_index < this->height; ++h_index) {
				int n_index = w_index * this->height + h_index;
				neuronSim.setInputCurrent(n_index, 0);
			}
		}
	}
	
	//Step the simulator
	neuronSim.step();
}




/*--------------------------------------------------------------------*/
/*---------              PROTECTED METHODS                     -------*/
/*--------------------------------------------------------------------*/

/**  Updates the properties.
	If UpdateReadOnly is set to true only the read only properties will be updated */
//<Distance, int, 8, array<int, 8>>
void WbSensorInboundChannel::updateProperties(map<string, Property>& properties) {

	if (propertyMap.size() != properties.size())
		throw CARLsimIOException("WbSensorInputChannel: Current properties do not match new properties.");

	updatePropertyCount = 0;
	for (map<string, Property>::iterator iter = properties.begin(); iter != properties.end(); ++iter) {
		//In updateReadOnly mode, only update properties that are not read only
		if ((isInitialized() && !propertyMap[iter->first].isReadOnly()) || !isInitialized()) {
			string paramName = iter->second.getName();
			switch (iter->second.getType()) {
			case Property::Integer: {
				if (paramName == NEURON_HEIGHT_NAME)
					setHeight(updateIntegerProperty(iter->second));
				else
					if (paramName == SENSOR_LIFESPAN_NAME)
						m_lifespan = updateIntegerProperty(iter->second);
					else if (paramName == BACKEND_NAME)
						neuronSim.setParameterBackend(updateIntegerProperty(iter->second));
			}
								  break;
			case Property::Double: {
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
				else if (paramName == STANDARD_DEVIATION_NAME)
					standardDeviation = updateDoubleProperty(iter->second);
			}
								 break;
			case Property::Combo: break;
			case Property::String:
				if (paramName == SENSOR_PREFIX_NAME) {
					m_sensorPrefix = updateStringProperty(iter->second);
					LOG(LOG_DEBUG) << "Sensor Prefix Property: " << m_sensorPrefix;
				} else
				if (paramName == SPIKES_LOG_NAME) {
						string& tmpLogName = updateStringProperty(iter->second);
						setSpikesLog(tmpLogName);
						/*! Enable actication of spike logs during the simulation
							Since the logging the header require the correct size of neurons, the function must be
							called in initialized as well. */
						if (isInitialized() && spikesLog_os != NULL)
							neuronSim.logFiringsHeader(*spikesLog_os, 0, 0, 8, -1);
					}
				break;
			default:
				throw CARLsimIOException("Property type not recognized.");
			}
		}
	}

	//Check all properties were updated 
	if (!isInitialized() && updatePropertyCount != propertyMap.size())
		throw CARLsimIOException("Some or all of the properties were not updated: ", updatePropertyCount);
}




// New Diagnostic Interface
size_t WbSensorInboundChannel::getBufferQueued() { return m_reader->getQueued(); }  // Delegate to specific Buffer
unsigned long long WbSensorInboundChannel::getBufferNext() { return m_reader->getNext_ms(); }  // Delegate to specific Buffer

void WbSensorInboundChannel::reset() { m_reader->reset(); }

size_t WbSensorInboundChannel::getReaderProcessed() { return m_reader->getProcessed(); }  // Delegate to specific Buffer
unsigned long long WbSensorInboundChannel::getReaderLast() { return m_reader->getLast_ms(); }  // Delegate to specific Buffer


