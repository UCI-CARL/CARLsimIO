#include <CARLsimIO/Writer/FiringWriter.hpp>


using namespace carlsimio;

//Inherited from PropertyHolder
void FiringWriter::setProperties(map<string, Property> &properties){

	// Buffer Log
	bufferLog = updateStringProperty(properties["Buffer Log"]);
	Common::openLog(bufferLog, &bufferLog_os, &bufferLog_ofs);

	if(bufferLog_os != NULL) {
		logBufferHeader();
	}

	// Yarp Log 
	yarpLog = updateStringProperty(properties["Yarp Log"]);
	Common::openLog(yarpLog, &yarpLog_os, &yarpLog_ofs);

}


void FiringWriter::initializeFiring(size_t size) {
	firing.reserve(size);  // AER
}


void FiringWriter::reset()
{
	Writer::reset();

	m_simtime = 0L;

	m_processed = 0;
	m_last_ms = 0L;

}
		
size_t FiringWriter::getProcessed()
{
	return m_processed;
}

unsigned long long FiringWriter::getLast_ms()
{
	return m_last_ms;
}