
#include <CARLsimIO/Writer/WbActuatorWriter.hpp>


using namespace carlsimio;

//Inherited from PropertyHolder
void WbActuatorWriter::setProperties(map<string, Property>& properties) {

	// Buffer Log
	bufferLog = updateStringProperty(properties["Buffer Log"]);
	Common::openLog(bufferLog, &bufferLog_os, &bufferLog_ofs);
	if (bufferLog_os != NULL) {
		logBufferHeader();
	}

	// Yarp Log 
	yarpLog = updateStringProperty(properties["Yarp Log"]);
	Common::openLog(yarpLog, &yarpLog_os, &yarpLog_ofs);

}

void WbActuatorWriter::reset()
{
	Writer::reset();

	m_simtime = 0L;
	m_processed = 0;
	m_last_ms = 0L;
}

size_t WbActuatorWriter::getProcessed()
{
	return m_processed;
}

unsigned long long WbActuatorWriter::getLast_ms()
{
	return m_last_ms;
}