//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Channel/OutboundChannel/WbActuatorOutboundChannel.hpp>
#include <CARLsimIO/Log/Log.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
using namespace carlsimio;

//Property names  
#define ACTUATOR_PREFIX_NAME		"Actuator Prefix"		

//! RealVector / Security
#define MIN_NAME		"Min"			
#define MAX_NAME		"Max"

#define RATE_OF_DECAY_NAME		"Rate of Decay"
#define CURRENT_INCREMENT_NAME	"Current Increment"
#define NEURON_HEIGHT_NAME		"Neuron Height"
#define THRESHOLD_NAME			"Threshold"

#define SPIKES_LOG_NAME			"Spikes Log"	// Logs Spikes from the Nemo as TestStream	
#define CODING_LOG_NAME			"Coding Log"	// Values of the spikes during the decoding process (for each single step)
		// needs to log the current values for each neuron, their decay etc. in a tabular fashion

////Debug defines
//#define DEBUG_NEURON_IDS
//#define DEBUG_CURRENT_VALUES

///** Constructor */
WbActuatorOutboundChannel::WbActuatorOutboundChannel() {

	//! Default for YARP
	addProperty(Property("vel", ACTUATOR_PREFIX_NAME, "Prefix the Webots actuator array", true));

	//! Ranges (Min/Max) per Security 
	addProperty(Property(Property::Double, 7.5, MAX_NAME, "Maximal Velocity", true));
	addProperty(Property(Property::Double, -7.5, MIN_NAME, "Minimal Velocity", true));

	addProperty(Property(Property::Double, 10, CURRENT_INCREMENT_NAME, "The amount by which the input current to the neurons is incremented by each spike", false));

	addProperty(Property(Property::Double, 0.1, RATE_OF_DECAY_NAME, "The rate of decay (lamda) of the potential variables: e^(-lamda*t)", false));
	addProperty(Property(Property::Double, 0.03, THRESHOLD_NAME, "The minimal potential of a neuron column to considered", false));
	addProperty(Property(Property::Integer, 2, NEURON_HEIGHT_NAME, "Height of the neuron group and precision of decoding", true));  // 5 Actions => 5 dedicated Neurons

	// Refactor cmd finspikes logging for SpikeStream CARLsimIO-Plugin
	addProperty(Property("null", SPIKES_LOG_NAME, "Log file for Spikes", false));
	addProperty(Property("null", CODING_LOG_NAME, "Log file for Decoding", false));

	//Create the description
	channelDescription = Description("WbActuator Outbound Channel", "This channel converts a pattern of spikes into actuator command and writes it to a yarp port", "WbActuator Writer");

	//Initialize variables
	writer = NULL;
	width = 0; // set in initialize
}

//** Destructor */
WbActuatorOutboundChannel::~WbActuatorOutboundChannel() {
	if (writer != NULL)
		delete writer;
}


//*--------------------------------------------------------------------*/
//*---------                 PUBLIC METHODS                     -------*/
//*--------------------------------------------------------------------*/

//Inherited from OutputChannel
void WbActuatorOutboundChannel::setFiring(const std::vector<unsigned>& buffer) {
	//Work through the neuron ids in the buffer
#ifdef DEBUG_NEURON_IDS
	cout << "OutputChannel: Firing neuron IDs ";
#endif//DEBUG_NEURON_IDS

	if (spikesLog_os != NULL)
		logFirings(buffer, 0, m_simtime, getHeight(), (int)(m_doubles));  //  + m_doubles

	for (std::vector<unsigned>::const_iterator iter = buffer.begin(); iter != buffer.end(); ++iter) {
#ifdef DEBUG_NEURON_IDS
		cout << *iter << ", ";
#endif//DEBUG_NEURON_IDS
		currentVariables[*iter] += currentIncrement;
	}
#ifdef DEBUG_NEURON_IDS
	cout << endl;
#endif//DEBUG_NEURON_IDS

#ifdef DEBUG_CURRENT_VALUES
	bool hasCurrent = false;
	for (size_t i = 0; i < currentVariables.size(); i++) {
		if (currentVariables[i] > 0.1) {
			hasCurrent = true;
			printf("%d:%.1f ", i, currentVariables[i]);
		}
	}
	if (hasCurrent)
		printf("\n");
#endif

	


}


//Inherited from PropertyHolder
void WbActuatorOutboundChannel::setProperties(map<string, Property>& properties) {
	updateProperties(properties);
}


//Inherited from OutputChannel
void WbActuatorOutboundChannel::initialize(Writer* writer, map<string, Property>& properties) {
	//This class requires an angle writer, so cast and check
	this->writer = dynamic_cast<WbActuatorWriter*>(writer);
	if (this->writer == NULL)
		throw CARLsimIOException("Cannot initialize OrderOutputChannel with a null writer.");

	//Update properties in this and dependent classes
	updateProperties(properties);

	this->width = m_actuatorPrefix == "vel" ? 2 : m_actuatorPrefix == "led" || m_actuatorPrefix == "rgb" ? 4 :
		m_actuatorPrefix == "frnt" || m_actuatorPrefix == "body" ? 1 : 0;

	m_doubles = this->width;


	auto n = size(); 

	//Set up current variables
	for (unsigned i = 0; i < n; ++i) {
		currentVariables.push_back(0.0);
	}

	size_t index;


		// Double Vector
		for (size_t double_i = 0; double_i < m_doubles; double_i++) {
			index = double_i;
			double dmin = m_min;
			double dmax = m_max;

			double valueDist = (dmax - dmin) / double(height - 1);
			// populate the values
			for (unsigned j = 0; j < this->height; ++j) {
				double dval = dmin + j * valueDist;
				currentVariableValues.push_back(dval);
			}
		}


	//Start the writer thread running
	this->writer->setActuatorPrefix(m_actuatorPrefix);
	this->writer->start();

	setInitialized(true);

	/*! Header require the correct size of neurons. The Parameter was set in update but
	the channel is not initialized at this time. */
	if (/*isInitialized() &&*/ spikesLog_os != NULL)
		logFiringsHeader(0, 0, getHeight(), (int)(m_doubles));

}


//Inherited from Channel
void WbActuatorOutboundChannel::step() {
	//Check writer for errors
	if (writer->isError()) {
		LOG(LOG_CRITICAL) << "WbActuatorWriter Error: " << writer->getErrorMessage();
		throw CARLsimIOException("Error in WbActuatorWriter");
	}

	writer->syncSim(m_simtime);

	// Exponential decay of variables
	for (vector<double>::iterator iter = currentVariables.begin(); iter != currentVariables.end(); ++iter) {
		(*iter) *= rateOfDecay; 
	}





	int decoded = 0; //Amount of decoded values. Only send x if all obligatoric values are defined in the Neural Group

	// prepare logging
	std::streamsize precision = 0;
	std::ostringstream os; // ISSUE: perfomant ?
	bool blog = false;
	static const unsigned line = 100;
	static char buf[line];

	if (codingLog_os != NULL) {
		os.precision(1); 
		os.setf(std::ios::fixed, std::ios::floatfield);
		sprintf_s(buf, line, "%6llu: %02d:%02d:%02d.%03d",
			m_simtime, (m_simtime / 1000 / 60 / 60) % 24, (m_simtime / 1000 / 60) % 60, (m_simtime / 1000) % 60, m_simtime % 1000);
	}

	size_t offset;

	vector<double> actuatorValue(m_doubles, 0);

	vector<int> perm_vel = { 0, 2, 1, 3 };
	vector<int>* perm = nullptr;

	perm = &perm_vel;

		// Decode Double Vector
		for (size_t double_i = 0; double_i < m_doubles; double_i++) {

			offset = double_i* height;   //  m_ints 

			//decode the current values weighted sums 
			 double valueSum = .0;
			 double weightSum = .0;
			for (unsigned j = 0; j < this->height; ++j) {

				// patch topografic index
				auto oj = offset + j;
				if (perm)
					oj = (*perm)[oj];

				valueSum += currentVariableValues.at(offset + j) * currentVariables.at(oj);
				weightSum += currentVariables.at(oj);
				if (codingLog_os != NULL) // always log to the buffer, decide later
					os << setw(4) << currentVariables.at(oj) << " ";
			}
			if (codingLog_os != NULL)
				os << "= " << setw(4) << weightSum << " | ";

			//Calculate new value
			if (abs(weightSum) > m_threshold) {
				actuatorValue[double_i] = valueSum / weightSum;
				decoded++;
				blog = true;
			}

		}

	if (codingLog_os != NULL && blog)
		*codingLog_os << buf << "  | " << os.str() << std::endl;

	if (decoded == m_doubles ) {  // + m_ints

		if (Log::ReportingLevel() <= LOG_INFO || codingLog_os != NULL) {
			static const size_t n = 250;
			static char buf[n];
			if (m_actuatorPrefix == "vel") {
				sprintf_s(buf, n, "left: %f right: %f\n",
					actuatorValue[0],
					actuatorValue[1]
				); 
			}
			LOGS(LOG_INFO, codingLog_os) << "actuator " << m_actuatorPrefix  << buf;
		}

		writer->setActuatorValues(actuatorValue);

		currentVariables.assign(currentVariables.size(), .0);
	}

}


/*--------------------------------------------------------------------*/
/*---------              PROTECTED METHODS                     -------*/
/*--------------------------------------------------------------------*/

/** Updates the properties by first checking if any are read-only  */
void WbActuatorOutboundChannel::updateProperties(map<string, Property>& properties) {
	if (propertyMap.size() != properties.size())
		throw CARLsimIOException("WbActuatorOutboundChannel: Current properties do not match new properties.");

	updatePropertyCount = 0;
	for (map<string, Property>::iterator iter = properties.begin(); iter != properties.end(); ++iter) {
		//In updateReadOnly mode, only update properties that are not read only
		if ((isInitialized() && !propertyMap[iter->first].isReadOnly()) || !isInitialized()) {
			string paramName = iter->second.getName();
			switch (iter->second.getType()) {
			case Property::Integer: {
				if (paramName == NEURON_HEIGHT_NAME)
					setHeight(updateIntegerProperty(iter->second));
				}
				break;
			case Property::Double: {
				if (paramName == RATE_OF_DECAY_NAME)
					rateOfDecay = exp(-updateDoubleProperty(iter->second)); // N(t) = N_0 * e^(-lamda*t), stepvariable: = dt = 1ms
				else if (paramName == MIN_NAME)
					m_min = updateDoubleProperty(iter->second);
				else if (paramName == MAX_NAME)
					m_max = updateDoubleProperty(iter->second);
				else if (paramName == CURRENT_INCREMENT_NAME)
					currentIncrement = updateDoubleProperty(iter->second);
				else if (paramName == THRESHOLD_NAME)
					m_threshold = updateDoubleProperty(iter->second);
				}
				break;
			case Property::Combo:
				break;
			case Property::String: {
				// Var2: get more info out of the string and validate it as well
				if (paramName == ACTUATOR_PREFIX_NAME) {
					m_actuatorPrefix = updateStringProperty(iter->second);
					LOG(LOG_DEBUG) << "Prefix Property: " << m_actuatorPrefix;					
				} else
					if (paramName == SPIKES_LOG_NAME) {
						m_spikesLogName = updateStringProperty(iter->second);
						setSpikesLog(m_spikesLogName);
						if (isInitialized() && spikesLog_os != NULL)
							; // logFiringsHeader(0, 0, getHeight(), ncboost::OrderInts + ncboost::OrderDoubles);
					}
					else
						if (paramName == CODING_LOG_NAME) {
							m_codingLogName = updateStringProperty(iter->second);
							setCodingLog(m_codingLogName);
						}
				}
				break;
			default:
				throw CARLsimIOException("Property type not recognized.");
			}
		}
	}

	if (!(rateOfDecay > 0.0 && rateOfDecay < 1.0))
		throw CARLsimIOException("WbActuatorOutboundChannel: Rate of Decay must in greater then 0.0 and lower than 1.0.");

	//Check all properties were updated
	if (!isInitialized() && updatePropertyCount != propertyMap.size())
		throw CARLsimIOException("WbActuatorOutboundChannel: Some or all of the properties were not updated: ", updatePropertyCount);

}



// New Diagnostic Interface
size_t WbActuatorOutboundChannel::getBufferQueued() { return writer->getQueued(); }  // Delegate to specific Buffer
unsigned long long WbActuatorOutboundChannel::getBufferNext() { return writer->getNext_ms(); }  // Delegate to specific Buffer

void WbActuatorOutboundChannel::reset() {
	writer->reset();

	if (spikesLog_os != NULL) {
		setSpikesLog(m_spikesLogName);
		if (isInitialized())
			; // logFiringsHeader(0, 0, getHeight(), ncboost::OrderInts + ncboost::OrderDoubles);
	}

	if (codingLog_os != NULL)
		setCodingLog(m_codingLogName);

}


size_t WbActuatorOutboundChannel::getReaderProcessed() { return writer->getProcessed(); }  // Delegate to specific Buffer
unsigned long long WbActuatorOutboundChannel::getReaderLast() { return writer->getLast_ms(); }  // Delegate to specific Buffer





#include <cstdio>

void WbActuatorOutboundChannel::logFiringsHeader(long long simtime, long long tick_time, int group, int cluster)
{
	if (spikesLog_os == NULL) return;
	std::ostream& s = *spikesLog_os;
	int numNeurons = size();

	// "     0: 22:34:00.000 "	
	if (simtime > -1) s << "              Neuron ";
	for (int i = 1; i <= numNeurons; i++)
	{
		if (group > -1 && i % group == 1) s << '|'; // Tupel Group
		if (i > 1 && cluster > -1 && i % (group * cluster) == 1) s << ' ' << '|';
		s << i / 100;
	}
	if (group > -1) s << '|';
	s << endl;

	if (simtime > -1) 	s << "                     ";
	for (int i = 1; i <= numNeurons; i++)
	{
		if (group > -1 && i % group == 1) s << '|'; // Tupel Group
		if (i > 1 && cluster > -1 && i % (group * cluster) == 1) s << ' ' << '|';
		s << (i % 100) / 10;
	}
	if (group > -1) s << '|';
	s << endl;
	if (simtime > -1) 	s << "  Step  Time         ";
	for (int i = 1; i <= numNeurons; i++)
	{
		if (group > -1 && i % group == 1) s << '|'; // Tupel Group
		if (i > 1 && cluster > -1 && i % (group * cluster) == 1) s << ' ' << '|';
		s << i % 10;
	}
	if (group > -1) s << '|';
	s << endl;
}

void WbActuatorOutboundChannel::logFirings(const std::vector<unsigned>& buffer, long long simtime, long long tick_time, int group, int cluster)
{
	if (spikesLog_os == NULL) return;
	std::ostream& s = *spikesLog_os;


	static const unsigned line = 100;
	static char buf[line];

	if (tick_time > -1)
	{
		sprintf_s(buf, line, "%6llu: %02d:%02d:%02d.%03d ",
			simtime,
			(tick_time / 1000 / 60 / 60) % 24, (tick_time / 1000 / 60) % 60, (tick_time / 1000) % 60, tick_time % 1000);
	}
	else
		if (simtime > -1)
		{
			sprintf_s(buf, line, "%6llu: ", simtime);
		}
	s << buf;

	std::vector<unsigned>::const_iterator iter = buffer.begin();
	int numNeurons = size();
	for (int i = 1; i <= numNeurons; i++)
	{
		if (group > -1 && i % group == 1) s << '|'; // Tupel Group
		if (i > 1 && cluster > -1 && i % (group * cluster) == 1) s << ' ' << '|';
		if (iter != buffer.end() && *iter == (i - 1))
		{
			s << '-';
			iter++;
		}
		else
			s << ' ';
	}

	if (group > -1) s << '|';
	s << endl;
}