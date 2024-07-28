
//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Reader/AkdYoloReader.hpp>
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


AkdYoloReader::AkdYoloReader() : m_simtime(0), m_processed(0), m_last_ms(0) {  // Fix 21.05.2013 defined value


	//Create description
	readerDescription = Description("Abstract Trade Event Reader", "Framework for concrete derived classes and UnitTest Mocks", "Trade Reader");


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


AkdYoloReader::~AkdYoloReader() 
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
void AkdYoloReader::initialize(map<string, Property>& properties){
	setProperties(properties);
	setInitialized(true);
}


//Inherited from PropertyHolder
void AkdYoloReader::setProperties(map<string, Property> &properties){

	// Buffer Log
	bufferLog = updateStringProperty(properties["Buffer Log"]);
	// setSpikesLog(tmpLogName);  TODO Generic
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
	// setYarpLog(tmpLogName, .. );  TODO: Refactor as static member in Reader, with pointer reference
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



void AkdYoloReader::appendTupel(const unsigned long long time, const yolo_event_t& tupel)
{
	m_timestamp.push_back(time);
	m_buffer.push_back(tupel);
}



/** the actual values for the given simulation time
 * this method should be read only, so an explicit set of simulation time shout happen before 
 */
void AkdYoloReader::getTuple(yolo_event_t &te, unsigned long long simtime)
{ 
	//! Reset the Value \todo this is inefficient todo all the time
	boost::get<0>(te).assign(0);
	boost::get<1>(te).assign(.0);

	// search the first tupel 
	// TODO: precondition: obsolete values are removed from the queue (set simulation time
	// -> reader thread  decay???
	for(unsigned index=0; index < m_buffer.size(); index++)
	{
		if(m_timestamp[index] > simtime) // all values are in the future
			break;

		te = m_buffer[index];
		break;
	}
}

/*! if none found return false, else assign the tick value to tick_time 
 */
bool AkdYoloReader::lastTickTime(unsigned long long &tick_time)
{
	vector<unsigned long long>::reverse_iterator rit_timestamp;

	rit_timestamp=m_timestamp.rbegin(); 
	
	if(rit_timestamp != m_timestamp.rend()) {
		tick_time = *rit_timestamp;
		return true;
	}

	return false;
}
 

void AkdYoloReader::logBufferHeader(std::ostream* os) 
{
	// Header
	// 					buf2,
	//	 "     : 2013-03-21T01:43:06.549+01:00	01:43:06.527  11  0.98  0.45  0.23
	*os<<
	     "Simtime                        Idx Timestamp     Class Probability Direction Distance" << endl;
}

void AkdYoloReader::logBuffer(std::ostream* os, const char* simtime)
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
		yolo_event_t &event = m_buffer[index];

		*os << buf << endl;
	}

}

void AkdYoloReader::getBufferBorders(yolo_event_t* &begin, yolo_event_t* &end) 
{
	boost::mutex::scoped_lock lock(threadMutex); // controls the object mutex

	begin=NULL; 
	end=NULL;  
	if(m_buffer.size()>0) {
		begin= &m_buffer[0]; 
		end= &m_buffer[m_buffer.size()-1];
	}
}


/*! new version for use within the class: no mutex and use stream member instead of parameter */
void AkdYoloReader::logBufferHeader() 
{
	std::ostream* os = bufferLog_os;

	// Header
	// 					buf2,
	//	 "     : 2013-03-21T01:43:06.549+01:00	01:43:06.527 EURUSD@AlpariUK-MT5 1.29354 +0.00000 11 4.0
	*os<<
	     //"Simtime                        Idx Timestamp     Security             Bid     Change    Spread  Volume" << endl;
		 "Idx Timestamp     Class Propability  Direction  Distance" << endl;
}

/*! new version for use within the class: no mutex and use stream member instead of parameter */
void AkdYoloReader::logBuffer(const char* simtime)
{

	std::ostream* os = bufferLog_os;

	*os << endl << "EventBuffer@" << simtime << endl; // Buffer at simtime

	static const unsigned line = 250;
	static char buf[line];

	if(m_timestamp.empty())
		*os << " <EMPTY>" << endl;
	else {
		vector<unsigned long long>::iterator iter; // timestamp
		size_t index;	
		for(iter=m_timestamp.begin(); iter!=m_timestamp.end(); iter++)
		{
			index = iter - m_timestamp.begin();
			yolo_event_t &event = m_buffer[index];

			////	simtime, index, 
			sprintf_s(buf, line, "%2d: %02d:%02d:%02d.%03d %d %04.2f %04.2f %04.2f", 
				index, 
				(*iter/1000/60/60)%24, (*iter/1000/60)%60, (*iter/1000) %60, *iter%1000,
				get<0>(event)[0], get<1>(event)[0], get<1>(event)[1], get<1>(event)[2]);

			*os << buf << endl;
		}
	}
}

/*! new version for use within the class: no mutex and use stream member instead of parameter */
bool AkdYoloReader::checkBufferBordersChanged() 
{
	// No Mutex

	yolo_event_t *begin=NULL, *end=NULL; 
	if(m_buffer.size()>0) {
		begin= &m_buffer[0]; 
		end= &m_buffer[m_buffer.size()-1];
	}

	// Fix: Track run empty buffers as well !
	//if(begin != NULL && (begin != last_begin || end != last_end) ) {
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
	*
	* m_simtime is set at the end to provide the last simtime 
	*/
void AkdYoloReader::syncSim(unsigned long long simtime)
{
	// Functional Logging Interface
	if(bufferLog_os != NULL && checkBufferBordersChanged()) {
		yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst()); 
		logBuffer(isoTimestamp.convert(simtime/1000, simtime%1000));
	}

	vector<unsigned long long>::iterator iter;
	vector<unsigned int>::iterator sec_iter; 

	// get rid off obsolete ticks (normal case)
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


	//// search for each security the latest and store overtaken ones in an index vector (seldom)
	//vector<unsigned int> overtaken;
	//for(unsigned sec=0; sec<m_securities.size(); sec++)
	//{
	//	int latest = -1; 
	//	for(iter=m_timestamp.begin(), sec_iter=m_security.begin();
	//		iter!=m_timestamp.end() && *iter <= simtime;
	//		iter++, sec_iter++)
	//	{
	//		if(*sec_iter==sec)
	//		{				
	//			if(latest!=-1)
	//				overtaken.push_back(latest);
	//			else
	//				m_last_ms = *iter;
	//			latest = unsigned(sec_iter - m_security.begin());
	//			if(m_processed == 0L)
	//				m_processed++; // see below
	//		}
	//	}
	//}

	//if(overtaken.size()>0) {
	//	// sort using default comparison (operator <):
	//	sort(overtaken.begin(), overtaken.end()); 

	//	// delete backwards
	//	for(vector<unsigned>::reverse_iterator rit=overtaken.rbegin(); rit!=overtaken.rend(); ++rit)
	//	{
	//  		m_timestamp.erase(m_timestamp.begin()+*rit);
	//		m_security.erase(m_security.begin()+*rit);
	//		m_buffer.erase(m_buffer.begin()+*rit);
	//		m_processed++; // ISSUE: Not good
	//	}
	//}

	m_simtime = simtime;
}


void AkdYoloReader::reset()
{
	Reader::reset();

	m_simtime = 0L;

	m_timestamp.clear();
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
		
size_t AkdYoloReader::getProcessed()
{
	return m_processed;
}

unsigned long long AkdYoloReader::getLast_ms()
{
	return m_last_ms;
}