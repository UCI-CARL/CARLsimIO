
//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Reader/WbSensorReader.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>
using namespace carlsimio;

//Other includes
//#include <boost/regex.hpp>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

#ifdef CARLSIMIO_SUPPORT_YARP
// Yarp Includes
#include <yarp/carl/StlUtils.h> 
#include <yarp/carl/AceUtils.h> 
#include <yarp/os/impl/PlatformTime.h>    
#endif

WbSensorReader::WbSensorReader() : m_simtime(0), m_processed(0), m_last_ms(0) { 

	//Create description
	readerDescription = Description("Abstract Tick Reader", "Framework for concrete derived classes and UnitTest Mocks", "Tick Reader");


	// Buffer Logging
	bufferLog_os = NULL;
	bufferLog_ofs = NULL;
	Common::initializeLog(&bufferLog_os, &bufferLog_ofs); 
	last_begin = NULL;
	last_end = NULL;

	// Yarp Logging
	yarpLog_os = NULL;
	yarpLog_ofs = NULL;
	Common::initializeLog(&yarpLog_os, &yarpLog_ofs); 

}


WbSensorReader::~WbSensorReader()
{
	if(bufferLog_ofs != NULL) {
		bufferLog_ofs->close();
		delete bufferLog_ofs;
		bufferLog_ofs = NULL;
	 }

	if(yarpLog_ofs != NULL) {
		yarpLog_ofs->close();
		delete yarpLog_ofs;	
		yarpLog_ofs = NULL;
	}

	Common::releaseLog(&bufferLog_os, &bufferLog_ofs);
	Common::releaseLog(&yarpLog_os, &yarpLog_ofs);

}


// Inherited from Reader
void WbSensorReader::initialize(map<string, Property>& properties) {
	setProperties(properties);
	setInitialized(true);
}


//Inherited from PropertyHolder
void WbSensorReader::setProperties(map<string, Property>& properties) {

	// Buffer Log
	bufferLog = updateStringProperty(properties["Buffer Log"]);
	bufferLog_os = NULL;
	if(bufferLog_ofs != NULL) {
		bufferLog_ofs->close();
		delete bufferLog_ofs;
	}
	bufferLog_ofs = NULL;
	if(bufferLog=="cout")
		bufferLog_os = &cout;
	else if(bufferLog!="null") {
		bufferLog_ofs = new ofstream(bufferLog);
		bufferLog_os = bufferLog_ofs;
	}
	Common::openLog(bufferLog, &bufferLog_os, &bufferLog_ofs);

	last_begin = NULL;
	last_end = NULL;
	if (bufferLog_os != NULL) {
		logBufferHeader();
		checkBufferBordersChanged();
	}

	// Yarp Log 
	yarpLog = updateStringProperty(properties["Yarp Log"]);
	// setYarpLog(tmpLogName, .. ); 
	yarpLog_os = NULL;
	if(yarpLog_ofs != NULL) {
		yarpLog_ofs->close();
		delete yarpLog_ofs;
	}
	yarpLog_ofs = NULL;
	if(yarpLog=="cout")
		yarpLog_os = &cout;
	else if(yarpLog!="null") {
		yarpLog_ofs = new ofstream(yarpLog);
		yarpLog_os = yarpLog_ofs;
	}

	Common::openLog(yarpLog, &yarpLog_os, &yarpLog_ofs);
	// Caution: set to m_yarp_os in Yarp specialized subclass

}


/*! Lookup the index of the security
* http://www.cplusplus.com/reference/algorithm/find/
*/
unsigned int WbSensorReader::lookup()
{
	throw CARLsimIOException("Securities are not applicable to WbSensorReader");
	return -1;
}


void WbSensorReader::appendTuple(const unsigned long long time, const std::vector<int>& values)
{
	m_timestamp.push_back(time);
	m_buffer.push_back(values);
}






/** the actual values for the given simulation time
 * this method should be read only, so an explicit set of simulation time shout happen before
 */
void WbSensorReader::getTuple(std::vector<int> & tuple, unsigned long long simtime)
{

	if (!m_buffer.empty()) {
		tuple = m_buffer.front(); 
		m_buffer.pop_front();
	}
}

/*! if none found return false, else assign the tick value to tick_time
 */
bool WbSensorReader::lastTickTime(unsigned long long& tick_time)
{
	if (m_timestamp.empty()) 
		return false;

	tick_time = m_timestamp.back();

	return true; 
}


void WbSensorReader::logBufferHeader(std::ostream* os)
{
	*os <<
		"Simtime                        Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
}

void WbSensorReader::logBuffer(std::ostream* os, const char* simtime)
{
	boost::mutex::scoped_lock lock(threadMutex); // controls the object mutex

	static const unsigned line = 250;
	static char buf[line];
	static char buf2[line];

	vector<unsigned long long>::iterator iter; // timestamp
	size_t index;
}

//void WbSensorReader::getBufferBorders(Common::tick_t*& begin, Common::tick_t*& end)

/*! new version for use within the class: no mutex and use stream member instead of parameter */
void WbSensorReader::logBufferHeader()
{
	std::ostream* os = bufferLog_os;

	// Header
	*os <<
		//"Simtime                        Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
		"Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
}

/*! new version for use within the class: no mutex and use stream member instead of parameter */
void WbSensorReader::logBuffer(const char* simtime)
{

	std::ostream* os = bufferLog_os;

	*os << endl << "TickBuffer@" << simtime << endl; // Buffer at simtime

	static const unsigned line = 250;
	static char buf[line];
	static char buf2[line];

	if (m_timestamp.empty())
		*os << " <EMPTY>" << endl;
	else {
		vector<unsigned long long>::iterator iter; // timestamp
		size_t index;
		for (iter = m_timestamp.begin(); iter != m_timestamp.end(); iter++)
		{
			index = iter - m_timestamp.begin();

			if(index < m_buffer.size()) {
				std::vector<int>& tick = m_buffer[index];

				//	simtime, index, 
				sprintf_s(buf, line, " %2zd: %02lld:%02lld:%02lld.%03lld %d ",  // %d %d %d
					index,
					(*iter / 1000 / 60 / 60) % 24, (*iter / 1000 / 60) % 60, (*iter / 1000) % 60, *iter % 1000,
					tick[0]);  // , tick[1], tick[2], tick[3]

				*os << buf << endl;
			}
		}
	}
}

/*! new version for use within the class: no mutex and use stream member instead of parameter */
bool WbSensorReader::checkBufferBordersChanged()
{
	// No Mutex

	std::vector<int>* begin = NULL, * end = NULL;
	if (m_buffer.size() > 0) {
		begin = &m_buffer[0];
		end = &m_buffer[m_buffer.size() - 1];
	}

	// Track run empty buffers as well
	if (begin != last_begin || end != last_end) {
		last_begin = begin;
		last_end = end;
		return true;
	}
	else
		return false;

	return false; 
}

/*! Sync with SNN simulator
	* eg. dispose obsolete values, that are ticks that are older than their life span
	* and checks for each security, if a newer value is now pending and that is still not ahead of the simtime
	*
	* m_simtime is set at the end to provide the last simtime
	*/
void WbSensorReader::syncSim(unsigned long long simtime)
{
	// Functional Logging Interface
	if (bufferLog_os != NULL && checkBufferBordersChanged()) {
		yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst());
		logBuffer(isoTimestamp.convert(simtime / 1000, simtime % 1000));
	}

	vector<unsigned long long>::iterator iter;
	vector<unsigned int>::iterator sec_iter;

	// get rid off obsolete ticks (normal case)
	int obsolete = 0;
	for (iter = m_timestamp.begin();
		iter != m_timestamp.end() && *iter + m_lifespan < simtime; 
		iter++)
	{
		obsolete++;
		m_last_ms = *iter; 
		m_processed++;
	}
	if (obsolete > 0)
	{
		// deque 
		m_timestamp.erase(m_timestamp.begin(), m_timestamp.begin() + obsolete);
		if(!m_buffer.size() > obsolete) 
			m_buffer.erase(m_buffer.begin(), m_buffer.begin() + obsolete);
	}

	m_simtime = simtime;
}


void WbSensorReader::reset()
{
	Reader::reset();

	m_simtime = 0L;

	m_timestamp.clear();
	m_buffer.clear();

	m_processed = 0;
	m_last_ms = 0L;

	if(bufferLog_ofs!=NULL) { 
		Common::openLog(bufferLog, &bufferLog_os, &bufferLog_ofs);
		logBufferHeader(); 
	}
	if(yarpLog_ofs!=NULL) {
		Common::openLog(yarpLog, &yarpLog_os, &yarpLog_ofs);
	}

}

size_t WbSensorReader::getProcessed()
{
	return m_processed;
}

unsigned long long WbSensorReader::getLast_ms()
{
	return m_last_ms;
}
