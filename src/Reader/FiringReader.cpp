//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Reader/FiringReader.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>
using namespace carlsimio;

//Other includes
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

// Yarp Includes
#include <yarp/carl/StlUtils.h> 
#include <yarp/carl/AceUtils.h>
#include <yarp/os/impl/PlatformTime.h>


FiringReader::FiringReader() : m_simtime(0), m_processed(0), m_last_ms(0) {  // Fix 21.05.2013 defined value

	//Create description
	readerDescription = Description("Abstract Firing Reader", "Framework for concrete derived classes and UnitTest Mocks", "Firing Reader");


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

	m_lifespan = 1;  // patch
}


FiringReader::~FiringReader() 
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
void FiringReader::initialize(map<string, Property>& properties){
	setProperties(properties);
	setInitialized(true);
}


//Inherited from PropertyHolder
void FiringReader::setProperties(map<string, Property> &properties){

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
	if(bufferLog_os != NULL) {
		logBufferHeader();
		checkBufferBordersChanged();
	}

	// Yarp Log 
	yarpLog = updateStringProperty(properties["Yarp Log"]);
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

}



void FiringReader::appendTupel(const unsigned long long time, const vector<unsigned int>& aer)
{
	m_timestamp.push_back(time);
	m_buffer.push_back(aer);
}



/** the actual values for the given simulation time
 * this method should be read only, so an explicit set of simulation time shout happen before 
 */
void FiringReader::getTuple(vector<unsigned int> &aer, unsigned long long simtime)
{ 
	for(unsigned index=0; index < m_buffer.size(); index++)
	{
		if(m_timestamp[index] > simtime) // all values are in the future
			break;

			aer = m_buffer[index];
			break;
	}
}



/*! if none found return false, else assign the tick value to tick_time
 */
bool FiringReader::lastTickTime(unsigned long long& tick_time, size_t& index)
{
	if (!m_timestamp.empty()) {
		index = m_timestamp.size();
		tick_time = m_timestamp[index-1];
		return true; 
	}

	return false;
}


void FiringReader::logBufferHeader(std::ostream* os) 
{
	// Header
	*os<<
	     "Simtime                        Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
}

void FiringReader::logBuffer(std::ostream* os, const char* simtime)
{
	boost::mutex::scoped_lock lock(threadMutex); // controls the object mutex

	static const unsigned line = 250;
	static char buf[line];
	static char buf2[line];

	vector<unsigned long long>::iterator iter; // timestamp
	size_t index;
	for(iter=m_timestamp.begin(); iter!=m_timestamp.end(); iter++)
	{
		index = iter - m_timestamp.begin();
		vector<unsigned int>& aer = m_buffer[index];

		sprintf_s(buf2, line, "%s@%s", "", "");
		sprintf_s(buf, line, "%-30s  %2d: %02d:%02d:%02d.%03d %-20s %d",
			simtime, index, 
			(*iter/1000/60/60)%24, (*iter/1000/60)%60, (*iter/1000) %60, *iter%1000,
			buf2,
			aer.size() );

		*os << buf << endl;
	}

}


/*! new version for use within the class: no mutex and use stream member instead of parameter */
void FiringReader::logBufferHeader() 
{
	std::ostream* os = bufferLog_os;

	// Header
	*os<<
	     //"Simtime                        Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
		 "Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
}

/*! new version for use within the class: no mutex and use stream member instead of parameter */
void FiringReader::logBuffer(const char* simtime)
{

	std::ostream* os = bufferLog_os;

	*os << endl << "FiringBuffer@" << simtime << endl; // Buffer at simtime

	static const unsigned line = 250;
	static char buf[line];
	static char buf2[line];

	if(m_timestamp.empty())
		*os << " <EMPTY>" << endl;
	else {
		vector<unsigned long long>::iterator iter; // timestamp
		size_t index;	
		for(iter=m_timestamp.begin(); iter!=m_timestamp.end(); iter++)
		{
			index = iter - m_timestamp.begin();
			vector<unsigned int>& aer = m_buffer[index];

			sprintf_s(buf, line, " %2d: %02d:%02d:%02d.%03d %-20s %d:", 
				index, 
				(*iter/1000/60/60)%24, (*iter/1000/60)%60, (*iter/1000) %60, *iter%1000,
				"",
				aer.size() );

			*os << buf; 

			for (auto iter = aer.begin(); iter < aer.end(); iter++)
				*os << " " << *iter; 
			
			*os << endl;
		}
	}
}

/*! new version for use within the class: no mutex and use stream member instead of parameter */
bool FiringReader::checkBufferBordersChanged() 
{
	//// No Mutex

	vector<unsigned int>* begin = NULL, * end = NULL;
	if(m_buffer.size()>0) {
		begin= &m_buffer[0]; 
		end= &m_buffer[m_buffer.size()-1];
	}

	if(begin != last_begin || end != last_end) {
		last_begin = begin;
		last_end = end;
		return true;
	} else
		return false;

}

/*! Sync with SNN simulator 
	* eg. dispose obsolete values, that are ticks that are older than their life span
	* and checks for each security, if a newer value is now pending and that is still not ahead of the simtime
	*/
void FiringReader::syncSim(unsigned long long simtime)
{
	// Functional Logging Interface
	if(bufferLog_os != NULL && checkBufferBordersChanged()) {
		yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst()); // ISSUE2: -> see YarpTickReader
		logBuffer(isoTimestamp.convert(simtime/1000, simtime%1000));
	}

	vector<unsigned long long>::iterator iter;
	vector<unsigned int>::iterator sec_iter; 

	int obsolete = 0;
	for(iter=m_timestamp.begin(); 
		iter!=m_timestamp.end() && *iter + m_lifespan < simtime; // caution when calculating with unsigned ints
		iter++) 
	{
			obsolete++;
			m_last_ms = *iter; // ISSUE
			m_processed++;
	}
	if(obsolete>0)
	{
		m_timestamp.erase(m_timestamp.begin(),m_timestamp.begin()+obsolete);
		m_buffer.erase(m_buffer.begin(),m_buffer.begin()+obsolete);
	}


	vector<unsigned int> overtaken;

	if(overtaken.size()>0) {
		// sort using default comparison (operator <):
		sort(overtaken.begin(), overtaken.end()); 

		// delete backwards
		for(vector<unsigned>::reverse_iterator rit=overtaken.rbegin(); rit!=overtaken.rend(); ++rit)
		{
	  		m_timestamp.erase(m_timestamp.begin()+*rit);
			m_buffer.erase(m_buffer.begin()+*rit);
			m_processed++; 
		}
	}

	m_simtime = simtime;
}


void FiringReader::reset()
{
	Reader::reset();

	m_simtime = 0L;

	m_timestamp.clear();
	//m_security.clear();
	m_buffer.clear();

	m_processed = 0;
	m_last_ms = 0L;

	//! New Logging interface
	if(bufferLog_ofs!=NULL) { 
		Common::openLog(bufferLog, &bufferLog_os, &bufferLog_ofs);
		logBufferHeader(); 
	}
	if(yarpLog_ofs!=NULL) {
		Common::openLog(yarpLog, &yarpLog_os, &yarpLog_ofs);
	}

}
		
size_t FiringReader::getProcessed()
{
	return m_processed;
}

unsigned long long FiringReader::getLast_ms()
{
	return m_last_ms;
}