//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Channel/OutboundChannel/FiringOutboundChannel.hpp>
#include <CARLsimIO/Log/Log.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
using namespace carlsimio;

#define SPARSE_CODING_NAME		"Sparse Coding"

#define NEURON_HEIGHT_NAME		"Neuron Height"
#define NEURON_WIDTH_NAME		"Neuron Width"

#define AXONAL_TRACT_NAME		"Axonal Tract"

#define SPIKES_LOG_NAME			"Spikes Log"	// Logs Spikes from the Nemo as TestStream	
#define CODING_LOG_NAME			"Coding Log"	// Values of the spikes during the decoding process (for each single step)
		

//
///** Constructor */
FiringOutboundChannel::FiringOutboundChannel() {


	addProperty(Property(Property::Integer, 1, SPARSE_CODING_NAME, "Translation of the firing ", true));

	addProperty(Property(Property::Integer, 4, NEURON_HEIGHT_NAME, "Height of the neuron group and precision of decoding", true));
	addProperty(Property(Property::Integer, 3, NEURON_WIDTH_NAME, "Width of the neuron group and precision of decoding", true));

	addProperty(Property("left2right", AXONAL_TRACT_NAME, "Axonal Tract from original region to destination.", false));

	// Refactor cmd finspikes logging for SpikeStream CARLsimIO-Plugin
	addProperty(Property("null", SPIKES_LOG_NAME, "Log file for Spikes", false));
	addProperty(Property("null", CODING_LOG_NAME, "Log file for Decoding", false));

	//Create the description
	channelDescription = Description("Firing Outbound Channel", "This channel encodes spikes of an output layer for external processing.", "Firing Writer");

	//Initialize variables
	writer = NULL;
	width = 0; // set in initialize
}

//** Destructor */
FiringOutboundChannel::~FiringOutboundChannel(){
	if(writer != NULL)
		delete writer;
}


//*--------------------------------------------------------------------*/
//*---------                 PUBLIC METHODS                     -------*/
//*--------------------------------------------------------------------*/

//Inherited from OutputChannel
void FiringOutboundChannel::setFiring(const std::vector<unsigned>& buffer){
	//Work through the neuron ids in the buffer
	#ifdef DEBUG_NEURON_IDS
		cout<<"OutputChannel: Firing neuron IDs ";
	#endif//DEBUG_NEURON_IDS

	if(spikesLog_os != NULL) 
		//logFirings(buffer, 0, m_simtime, getHeight()* getWidth(), 1); 
		logFirings(buffer, 0, m_simtime, getWidth(), getHeight());

	firing.clear();

	//fired.resize(fired.size(), false);
	for (int id = 0; id < size(); id++) 
		fired[id] = false;

	for (std::vector<unsigned>::const_iterator iter = buffer.begin(); iter != buffer.end(); ++iter) {
		fired[*iter] = true;
	}
	
	for (int id = 0; id < size(); id++) {
		if (fired[id])
			firing.push_back(id);
	}
	

	#ifdef DEBUG_CURRENT_VALUES
		bool hasCurrent = false;
		for(size_t i=0; i<currentVariables.size(); i++) {
			if(currentVariables[i]>0.1) {
				hasCurrent = true;
				printf("%d:%.1f ", i, currentVariables[i]);
			}
		}
		if(hasCurrent)
			printf("\n");
	#endif



}


//Inherited from PropertyHolder
void FiringOutboundChannel::setProperties(map<string, Property>& properties){
	updateProperties(properties);
}


//Inherited from OutputChannel
void FiringOutboundChannel::initialize(Writer* writer, map<string, Property>& properties){
	//This class requires an angle writer, so cast and check
	this->writer = dynamic_cast<FiringWriter*>(writer);
	if(this->writer == NULL)
		throw CARLsimIOException("Cannot initialize FiringOutputChannel with a null writer.");

	//Update properties in this and dependent classes
	updateProperties(properties);

	firing.reserve(this->size());  // AER

	fired.resize(this->size(), false);  // bit map 

	this->writer->initializeFiring(this->size());
	this->writer->initializeTract(this->axonalTract);

	this->writer->start();

	setInitialized(true);

	/*! Header require the correct size of neurons. The Parameter was set in update but 
	the channel is not initialized at this time. */ 
	if(/*isInitialized() &&*/ spikesLog_os != NULL)
		logFiringsHeader(0, 0, getWidth(), getHeight());

}


//Inherited from Channel
void FiringOutboundChannel::step(){
	//Check writer for errors
	if(writer->isError()){
		LOG(LOG_CRITICAL)<<"FiringWriter Error: "<<writer->getErrorMessage();
		throw CARLsimIOException("Error in FiringWriter");
	}

	writer->syncSim(m_simtime);

	writer->setFiring(firing);

}


/*--------------------------------------------------------------------*/
/*---------              PROTECTED METHODS                     -------*/
/*--------------------------------------------------------------------*/

/** Updates the properties by first checking if any are read-only  */
void FiringOutboundChannel::updateProperties(map<string, Property>& properties) {
	if(propertyMap.size() != properties.size())
		throw CARLsimIOException("FiringOutputChannel: Current properties do not match new properties.");

	updatePropertyCount = 0;
	for(map<string,Property>::iterator iter = properties.begin(); iter != properties.end(); ++iter){
		//In updateReadOnly mode, only update properties that are not read only
		if((isInitialized() && !propertyMap[iter->first].isReadOnly()) || !isInitialized()) {
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
				}
				break;
				case Property::Double:{

				}
				break;
				case Property::Combo: 
					break;
				case Property::String: {
						if(paramName == SPIKES_LOG_NAME) {
							m_spikesLogName = updateStringProperty(iter->second);
							setSpikesLog(m_spikesLogName);
							/*! Enable actication of spike logs during the simulation 
								Since the logging the header require the correct size of neurons, the function must be 
								called in initialized as well. */ 
							if(isInitialized() && spikesLog_os != NULL)
								logFiringsHeader(0, 0, getHeight(), getWidth());
						} else
						if(paramName == CODING_LOG_NAME) {
							m_codingLogName = updateStringProperty(iter->second);
							setCodingLog(m_codingLogName);
						} else 
						if (paramName == AXONAL_TRACT_NAME) {
							setAxonalTract(updateStringProperty(iter->second));
						}
					} 
					break;
				default:
					throw CARLsimIOException("Property type not recognized.");
			}
		}
	}

	//Check all properties were updated
	if(!isInitialized() && updatePropertyCount != propertyMap.size())
		throw CARLsimIOException("OrderOutputChannel: Some or all of the properties were not updated: ", updatePropertyCount);

}



// New Diagnostic Interface
size_t FiringOutboundChannel::getBufferQueued() { return writer->getQueued(); }  // Delegate to specific Buffer
unsigned long long FiringOutboundChannel::getBufferNext() { return writer->getNext_ms(); }  // Delegate to specific Buffer

void FiringOutboundChannel::reset() { 
	writer->reset();

	if(spikesLog_os != NULL) {
		setSpikesLog(m_spikesLogName);
		if(isInitialized())
			//logFiringsHeader(0,  0, getHeight() * getWidth(), 1);
			logFiringsHeader(0, 0, getWidth(), getHeight());
	}

	if(codingLog_os != NULL)
		setCodingLog(m_codingLogName);

}


size_t FiringOutboundChannel::getReaderProcessed() { return writer->getProcessed(); }  // Delegate to specific Buffer
unsigned long long FiringOutboundChannel::getReaderLast() { return writer->getLast_ms(); }  // Delegate to specific Buffer



#include <cstdio>
  
void FiringOutboundChannel::logFiringsHeader(long long simtime, long long tick_time, int group, int cluster)
{
	if(spikesLog_os == NULL) return;
	std::ostream &s = *spikesLog_os; 
	int numNeurons = size();

	if(simtime>-1) s << "              Neuron ";
	for(int i=1; i<=numNeurons;i++) 
	{
		if(group>-1 && i%group==1) s << '|'; // Tupel Group
		if(i>1 && cluster>-1 && i%(group*cluster)==1) s << ' ' << '|';
		s << i / 100;
	}
	if(group>-1) s << '|';
	s << endl;

	if(simtime>-1) 	s << "                     ";
	for(int i=1; i<=numNeurons;i++) 
	{
		if(group>-1 && i%group==1) s << '|'; // Tupel Group
		if(i>1 && cluster>-1 && i%(group*cluster)==1) s << ' ' << '|';
		s << (i % 100) / 10;
	}
	if(group>-1) s << '|';
	s << endl;
					  // "     0: 22:34:00.000 "	
	if(simtime>-1) 	s << "  Step  Time         ";
	for(int i=1; i<=numNeurons;i++) 
	{
		if(group>-1 && i%group==1) s << '|'; // Tupel Group
		if(i>1 && cluster>-1 && i%(group*cluster)==1) s << ' ' << '|';
		s << i % 10;
	}
	if(group>-1) s << '|';
	s << endl;
}

void FiringOutboundChannel::logFirings(const std::vector<unsigned>& buffer, long long simtime, long long tick_time, int group, int cluster)
{
	if(spikesLog_os == NULL) return;
	std::ostream &s = *spikesLog_os;


	static const unsigned line = 100;
	static char buf[line];

	if(tick_time>-1)
	{
		sprintf_s(buf, line, "%6llu: %02d:%02d:%02d.%03d ", 
				simtime, 
				(tick_time/1000/60/60)%24, (tick_time/1000/60)%60, (tick_time /1000) %60,  tick_time %1000);
	} else
	if(simtime>-1)
	{
		sprintf_s(buf, line, "%6llu: ", simtime);
		//offset = 8;
	}
	s << buf;

	std::vector<unsigned>::const_iterator iter = firing.begin();
	int numNeurons = size();

	for(int i=1; i<=numNeurons; i++)
	{
		if(group>-1 && i%group==1) s << '|'; // Tupel Group
		if(i>1 && cluster>-1 && i%(group*cluster)==1) s << ' ' << '|';
		if(iter!=firing.end() && *iter==(i-1) )
		//if(fired[i-1])
		{
			s << '-';
			iter++;
		} else
			s << ' ';	
	}


	if(group>-1) s << '|';
	s << endl;
}