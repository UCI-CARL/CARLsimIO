//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Channel/InboundChannel/FiringInboundChannel.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Reader/FiringReader.hpp>
#include <CARLsimIO/Log/Log.hpp>

#include <boost/math/distributions/normal.hpp>

#include <carl/boost_ext.hpp>

#include <yarp/carl/all.h>
#include <yarp/carl/StlUtils.h> 


using namespace carlsimio;

#define SPIKES_LOG_NAME			"Spikes Log"

#define SPARSE_CODING_NAME		"Sparse Coding"

#define NEURON_HEIGHT_NAME "Neuron Height"
#define NEURON_WIDTH_NAME "Neuron Width"

#define AXONAL_DELAY_NAME "Axonal Delay"

#define AXONAL_TRACT_NAME "Axonal Tract"

/** Default constructor, initialises the default channel properties */
FiringInboundChannel::FiringInboundChannel(): dopamineLog_os(NULL), dopamineLog_ofs(NULL) {

	
	addProperty(Property(Property::Integer, 1, SPARSE_CODING_NAME, "Translation of the firing ", true));


	addProperty(Property(Property::Integer, 4, NEURON_HEIGHT_NAME, "Height of the receiving neuron group.", true));  // Precision
	addProperty(Property(Property::Integer, 3, NEURON_WIDTH_NAME, "Width of the receiving neuron group.", true));  // Precision
	

	addProperty(Property(Property::Integer, 2, AXONAL_DELAY_NAME, "Axonal delay in ms for the spikes.", true));  

	addProperty(Property("left2right", AXONAL_TRACT_NAME, "Axonal Tract from original region to destination.", false));

	addProperty(Property("null", SPIKES_LOG_NAME, "Log file for Spikes", false));


	//Create the description
	channelDescription = Description("Firing Inbound Channel", "This is a firing inbound channel", "Firing Reader");

	//Initialize variables
	m_reader = NULL;


	m_last_ms = 0L;
	//m_last_type = 0;

	m_last_simtime = 0L;

}


/*! Destructor */
FiringInboundChannel::~FiringInboundChannel(){

	if(m_reader != NULL)
		delete m_reader;

}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

//Inherited from InputChannel
void FiringInboundChannel::initialize(Reader* reader, map<string, carlsimio::Property>& properties){

	//This class requires an tick reader, so cast it and check
	this->m_reader = dynamic_cast<FiringReader*>(reader);
	if(this->m_reader == NULL)
		throw CARLsimIOException("Cannot initialize FiringInboundChannel with a null reader.");

	updateProperties(properties);

	m_reader->initializeTract(axonalTract);

	//Start the reader thread running
	m_reader->start();			

	setInitialized(true);

}


//Inherited from Channel. This will be done immediately if we are not stepping or deferred until the end of the step */
void FiringInboundChannel::setProperties(map<string,carlsimio::Property>& properties){
	updateProperties(properties);
}

// Global
static yarp::carl::IsoTimestamp m_isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst());

//Inherited from InputChannel
void FiringInboundChannel::step(){

	//Check reader for errors
	if(m_reader->isError()){
		LOG(LOG_CRITICAL)<<"FiringReader Error: "<<m_reader->getErrorMessage();
		throw CARLsimIOException("Error in FiringReader");
	}


	m_reader->syncSim(m_simtime);		

	vector<unsigned int> aer;
	m_reader->getTuple(aer, m_simtime);

	firing = aer; 

		
	double min, max;
	double value;
	size_t w_index;
	int n_index;


}


/*--------------------------------------------------------------------*/
/*---------              PROTECTED METHODS                     -------*/
/*--------------------------------------------------------------------*/

/**  Updates the properties.
	If UpdateReadOnly is set to true only the read only properties will be updated */
void FiringInboundChannel::updateProperties(map<string, carlsimio::Property>& properties){

	if(propertyMap.size() != properties.size())
		throw CARLsimIOException("FiringInputChannel: Current properties do not match new properties.");

	updatePropertyCount = 0;
	for(map<string, Property>::iterator iter = properties.begin(); iter != properties.end(); ++iter)  {
		//In updateReadOnly mode, only update properties that are not read only
		if( ( isInitialized() && !propertyMap[iter->first].isReadOnly() ) || !isInitialized()) {
			string paramName = iter->second.getName();
			switch (iter->second.getType()) {
				case Property::Integer: {
					if (paramName == NEURON_HEIGHT_NAME)
						setHeight(updateIntegerProperty(iter->second));
					else
					if (paramName == NEURON_WIDTH_NAME)
						setWidth(updateIntegerProperty(iter->second));
					else
					if (paramName == SPARSE_CODING_NAME)
						setSparseCoding(updateIntegerProperty(iter->second));
					// depending on the sparse coding type, the internal buffer is handled (AER, Bitmap, ..)
					else 
					if (paramName == AXONAL_DELAY_NAME)
						setAxonalDelay(updateIntegerProperty(iter->second));
				}
				break;
				case Property::Double:  {
				}
				break;
				case Property::Combo: break;
				case Property::String:
					if(paramName == SPIKES_LOG_NAME) {
						m_spikesLogName = updateStringProperty(iter->second);
						//setSpikesLog(m_spikesLogName);
						//if(isInitialized() && spikesLog_os != NULL)
						//	neuronSim.logFiringsHeader(*spikesLog_os, 0,  0, getHeight(), ncboost::TradeEventInts+ncboost::TradeEventDoubles);  
					} else
					if (paramName == AXONAL_TRACT_NAME) {
						setAxonalTract(updateStringProperty(iter->second));
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
size_t FiringInboundChannel::getBufferQueued() { return m_reader->getQueued(); }  // Delegate to specific Buffer
unsigned long long FiringInboundChannel::getBufferNext() { return m_reader->getNext_ms(); }  // Delegate to specific Buffer

void FiringInboundChannel::reset() { m_reader->reset(); }

size_t FiringInboundChannel::getReaderProcessed() { return m_reader->getProcessed(); }  // Delegate to specific Buffer
unsigned long long FiringInboundChannel::getReaderLast() { return m_reader->getLast_ms(); }  // Delegate to specific Buffer
