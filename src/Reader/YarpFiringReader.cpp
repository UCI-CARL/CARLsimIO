
//CARLsimIO includes
#include <CARLsimIO/Reader/YarpFiringReader.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Log/Log.hpp>
using namespace carlsimio;

//Other includes
#include <boost/regex.hpp>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

// Yarp
#include <yarp/os/Bottle.h>
#include <yarp/os/Port.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/carl/all.h>
#include <yarp/carl/StlUtils.h> 
#include <yarp/carl/BottleDecoder.h>
#include <yarp/carl/AceUtils.h>
#include <yarp/os/ConstString.h>
#include <yarp/os/PortReader.h>
#include <yarp/os/ConnectionReader.h>
#include <yarp/os/Network.h>
#include <yarp/os/RpcClient.h>

using namespace yarp::carl;
using namespace yarp::os;
using namespace yarp::os::impl;

#ifdef _MSC_VER // MSVC toolchain 
#	include <windows.h>
#else
#   include <boost/thread/thread.hpp>
#	define GetCurrentThreadId() boost::this_thread::get_id()
#endif
#define _CARLSIMIO_GET_TID GetCurrentThreadId()


// is appended by group name 
#define YARP_PORTPREFIX_NAME "Yarp Portprefix"


// abstract
class YarpFiringReader::YarpDataProcessor {
public:
	YarpDataProcessor(YarpFiringReader* reader): m_reader(reader)
		{m_isoTimestamp = new yarp::carl::IsoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst());}
	virtual ~YarpDataProcessor() {m_reader = NULL; delete m_isoTimestamp;}
	void decode(Bottle& bottle) 
	{
		LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;
		const char* simtime = m_isoTimestamp->convert(m_reader->m_simtime /1000, m_reader->m_simtime %1000);
		LOG(LOG_DEBUG) << "decode bottle @ " << simtime;

		// Axonal Trackct
		Value& msg_tract = bottle.get(0);
		LOG(LOG_DEBUG) << "Axonal tract" << msg_tract.asString().c_str();

		// AER

		yarp::carl::BottleDecoder decoder(bottle, yarp::carl::MysqlTimestamp::LocalTimezoneDst());

		if (msg_tract.asString() != m_reader->tract)
		{
			LOG(LOG_DEBUG) << "Ignoring yarp::Bottle " << bottle.get(0).asString().c_str();
			return;
		}

		//ConstString &iso_timestamp = trade.time();
		Value& msg_time = bottle.get(1);
//		ConstString& iso_timestamp = msg_time.asString();
//		/home/ln/carlsimpp-t01/src/CARLsimIO/src/Reader/YarpFiringReader.cpp:74:63: error: cannot bind non-const lvalue reference of type ‘yarp::os::ConstString&’ {aka ‘std::__cxx11::basic_string<char>&’} to an rvalue of type ‘std::string’ {aka ‘std::__cxx11::basic_string<char>’}
		ConstString iso_timestamp = msg_time.asString();

		LOGS(LOG_INFO, m_reader->m_yarp_os) << "isoTimestamp: " << iso_timestamp; 

		MysqlTimestamp mysql_timestamp = MysqlTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst(), 1);

		mysql_timestamp.convert(iso_timestamp.c_str()); 
		const ACE_Time_Value& ace_time_value = mysql_timestamp.ace_time_value();
		unsigned long long timestamp = ace_time_value.get_msec();
		
		timestamp += 2; // delay

		LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "timestamp(ms): " << timestamp;	

		// convert   AER (1 3 4 6 7)
		Value& msg_format = bottle.get(2);
		assert(msg_format.asString() == "AER");

		vector<unsigned int> aer; //  = { 1,3,4 };
		Value& msg_aer = bottle.get(3);
		assert(msg_aer.isList());
		Bottle *list = msg_aer.asList();
		for (int i = 0; i < list->size(); i++) {
			aer.push_back(list->get(i).asInt());
		}

		// There is exactly one Yarp thread to be synchronized with the Qt thread
		// Critical Section Start
		{
			boost::mutex::scoped_lock lock(m_reader->threadMutex); // this is a reference to the object's mutex
		
			unsigned long long lastTickTime;
			size_t index;

			if(m_reader->lastTickTime(lastTickTime, index)) {
				if(m_reader->m_simtime > 0 && timestamp < m_reader->m_simtime - 250)
				{
					LOGS(LOG_INFO, m_reader->m_yarp_os) << "Skipping obsolete trade " << timestamp << "ms (" << iso_timestamp << ") at SimTime: " << m_reader->getSimTime() << "ms"; 
					return;
				} else
				if(timestamp < lastTickTime) 
				{					
					LOGS(LOG_NOTICE, m_reader->m_yarp_os) << "insert trade " << timestamp << "ms (" << iso_timestamp << ") before m_timestamp[" << index << "]:" << lastTickTime << "ms"; 
					m_reader->m_timestamp.insert(m_reader->m_timestamp.begin()+index, timestamp);
					m_reader->m_buffer.insert(m_reader->m_buffer.begin() + index, aer);
					if(m_reader->bufferLog_os != NULL && m_reader->checkBufferBordersChanged())
						m_reader->logBuffer(m_isoTimestamp->convert(m_reader->m_simtime/1000, m_reader->m_simtime%1000));
					LOGS(LOG_NOTICE, m_reader->m_yarp_os) << "done.";
					return; // Mutex is handled automatically by leaving the scope
				}
			}

			LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "Begin append trade to the buffer";
			m_reader->m_timestamp.push_back(timestamp); // simtime is managend central and set in Channel
			m_reader->m_buffer.push_back(aer);
			if(m_reader->bufferLog_os != NULL && m_reader->checkBufferBordersChanged())
				m_reader->logBuffer(m_isoTimestamp->convert(m_reader->m_simtime/1000, m_reader->m_simtime%1000));
			LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "done.";
		}
	}

protected:
	YarpFiringReader* m_reader;  // reserved
	IsoTimestamp* m_isoTimestamp;
};


// unbuffered
class YarpFiringReader::UnbufferedDataProcessor : public PortReader, public YarpFiringReader::YarpDataProcessor {
public:
	UnbufferedDataProcessor(YarpFiringReader* reader): YarpFiringReader::YarpDataProcessor(reader), PortReader() {}
	virtual ~UnbufferedDataProcessor() {}
    bool read(ConnectionReader& connection) {		
		LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;
		Bottle bottle;
        if(bottle.read(connection))	{	
			decode(bottle);
			return true;
		} else
			return false;
    }
};


YarpFiringReader::YarpFiringReader() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;


	// Define the properties of this reader
	addProperty(Property("/cns/inbound/firing/", YARP_PORTPREFIX_NAME, "Inbound Port for ticks", true));
	addProperty(Property(Property::Integer, 0, "Buffered", "0 unbuffered, 1 buffered", true));
	addProperty(Property(Property::Integer, 0, "Timezone", "MysqlTimestamp Timezone: 0 MT5 (default), 1 TickDB (TimezoneDst 1)", true)); // LiveData vs QS (2023)

	// Refactor cmd finspikes logging for SpikeStream CARLsimIO-Plugin
	addProperty(Property("null", "Buffer Log", "Log file of the Buffer", false));
	addProperty(Property("null", "Yarp Log", "Log file for Yarp", false));

	//Create description
	readerDescription = Description("Yarp Firing Reader", "This reads firing from a yarp port", "Firing Reader");

	m_trade_event_time = 0ULL;

	m_network = NULL;
	m_port = NULL;
	m_processor = NULL;

	m_bottle = NULL;

	// Experimental
	m_dopamine = .0;
	m_dopamine_injection = .0;
	
	m_yarp_os = NULL;

	m_scope_port = NULL;

	m_timezone = 0; // ::setProperties

}
 


YarpFiringReader::~YarpFiringReader(){

	if(m_port != NULL) {
		
		m_port->close();	//! Terminates the thread. Th Yarp server closes all releated connections.
		delete m_port;		//! and free the memory

	}

	if(m_processor != NULL) 
		delete m_processor;

	if (m_scope_port != NULL) {  
		m_scope_port->close();
		delete m_scope_port;
	}

	if(m_network != NULL) 
		delete m_network;  

}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

void YarpFiringReader::workerFunction() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

}


void YarpFiringReader::convertBottle() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	m_bottle = NULL;

}

//Inherited from PropertyHolder
void YarpFiringReader::setProperties(map<string, carlsimio::Property>& properties){
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	FiringReader::setProperties(properties); // generic

	if(yarpLog_os != NULL) {
		setYarpLogger(yarpLog_os); 
	}

	m_reading_port_name = updateStringProperty(properties["Yarp Portprefix"]);
	m_buffered = updateIntegerProperty(properties["Buffered"]);
	m_timezone = updateIntegerProperty(properties["Timezone"]);
}


//Inherited from PropertyHolder
void YarpFiringReader::start() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	LOGS(LOG_NOTICE, m_yarp_os) << " YarpTickReader: Receiving ticks from: " << m_writing_port_name << " on: " << m_reading_port_name;


	Network yarp;

	// if unbuffered  
	if (m_buffered == 0) {
		m_network = new Network();
		m_port = new Port();
		UnbufferedDataProcessor *unbuffered_processor = new UnbufferedDataProcessor(this);
		m_processor = unbuffered_processor; // generalization

		// begin critical section
		boost::mutex::scoped_lock lock(threadMutex);
		m_port->setReader(*unbuffered_processor);
		m_port->open(m_reading_port_name.c_str());
		// end critical section
	} else 
	if (m_buffered == 1) {
	}

}


void YarpFiringReader::syncSim(unsigned long long simtime)
{
	// begin critical section
	boost::mutex::scoped_lock lock(threadMutex); // controls the object mutex

	FiringReader::syncSim(simtime);

	// end critical section
}

