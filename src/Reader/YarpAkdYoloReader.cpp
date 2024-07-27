
//CARLsimIO includes
#include <CARLsimIO/Reader/YarpAkdYoloReader.hpp>
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
//#include <yarp/carl/BottleDecoder.h> // TODO: add to all.h
#include <yarp/carl/AceUtils.h>
#include <yarp/os/ConstString.h>
#include <yarp/os/PortReader.h>
#include <yarp/os/ConnectionReader.h>
#include <yarp/os/Network.h>
#include <yarp/os/RpcClient.h>

//#include <yarp/os/TypedReaderCallback.h>

using namespace yarp::carl;
using namespace yarp::os;
using namespace yarp::os::impl;


// abstract
class YarpAkdYoloReader::YarpDataProcessor {
public:
	YarpDataProcessor(YarpAkdYoloReader* reader): m_reader(reader)
		{m_isoTimestamp = new yarp::carl::IsoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst());} // 03.05.2013
	virtual ~YarpDataProcessor() {m_reader = NULL; delete m_isoTimestamp;}
	void decode(Bottle& bottle) 
	{
		LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;
		const char* simtime = m_isoTimestamp->convert(m_reader->m_simtime /1000, m_reader->m_simtime %1000);
		LOG(LOG_DEBUG) << "decode bottle @ " << simtime;

		LOG(LOG_DEBUG) << "Receiving yarp::Bottle " << bottle.toString().c_str();

		LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "Processing AKD YOLO event ";

		yolo_event_t yolo_event;
		yolo_event.get<0>().at(0) = bottle.get(0).asInt(); 
		yolo_event.get<1>().at(0) = bottle.get(1).asDouble();
		yolo_event.get<1>().at(1) = bottle.get(2).asDouble();
		yolo_event.get<1>().at(2) = bottle.get(3).asDouble();

		LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "yolo_event(class,probability,direction,distance): (" << yolo_event.get<0>().at(0) << "," 
			<< yolo_event.get<1>().at(0) << "," << yolo_event.get<1>().at(1) << "," << yolo_event.get<1>().at(2) << ")";
	

		//unsigned long long timestamp = t;
		unsigned long long timestamp = m_reader->m_yolo_event_time;
		LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "timestamp(ms): " << timestamp;


		// There is exactly one Yarp thread to be synchronized with the Qt thread
		// Critical Section Start
		{
			boost::mutex::scoped_lock lock(m_reader->threadMutex); // this is a reference to the object's mutex

			unsigned long long lastTickTime;
			size_t index;
			if(m_reader->lastTickTime(lastTickTime)) {
				if (m_reader->m_simtime > 0 && timestamp < m_reader->m_simtime) // 21.04.2013 fix for uninitialized member
				{
					LOGS(LOG_INFO, m_reader->m_yarp_os) << "Skipping obsolete tick " << timestamp << "ms at SimTime: " << m_reader->getSimTime() << "ms";
					return;
				} else
				if(timestamp < lastTickTime) 
				{					
					LOGS(LOG_NOTICE, m_reader->m_yarp_os) << "insert yolo event " << timestamp << "ms before m_timestamp[" << index << "]:" << lastTickTime << "ms"; 
					m_reader->m_timestamp.insert(m_reader->m_timestamp.begin(), timestamp);
					m_reader->m_buffer.insert(m_reader->m_buffer.begin()+index, yolo_event);
					if(m_reader->bufferLog_os != NULL && m_reader->checkBufferBordersChanged())
						m_reader->logBuffer(m_isoTimestamp->convert(m_reader->m_simtime/1000, m_reader->m_simtime%1000));
					LOGS(LOG_NOTICE, m_reader->m_yarp_os) << "done.";
					return; // Mutex is handled automatically by leaving the scope
				}
			}

			LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "Begin append trade to the buffer";
			m_reader->m_timestamp.push_back(timestamp); // simtime is managend central and set in Channel
			m_reader->m_buffer.push_back(yolo_event);	
			LOGS(LOG_INFO, m_reader->m_yarp_os)  << "Append event " << " yolo @ " << timestamp << "ms";	
			if(m_reader->bufferLog_os != NULL && m_reader->checkBufferBordersChanged())
				m_reader->logBuffer(m_isoTimestamp->convert(m_reader->m_simtime/1000, m_reader->m_simtime%1000));
			LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "done.";
		}
		
	}

protected:
	YarpAkdYoloReader* m_reader;  // reserved
	IsoTimestamp* m_isoTimestamp;
};


// unbuffered
class YarpAkdYoloReader::UnbufferedDataProcessor : public PortReader, public YarpAkdYoloReader::YarpDataProcessor {
public:
	UnbufferedDataProcessor(YarpAkdYoloReader* reader): YarpAkdYoloReader::YarpDataProcessor(reader), PortReader() {}
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

YarpAkdYoloReader::YarpAkdYoloReader() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	// Define the properties of this reader
	addProperty(Property("/cns/inbound/yolo", "Yarp Reading Port", "Inbound Port for ticks", true));
	// Production Default
	addProperty(Property("/akd/outbound/yolo", "Yarp Writing Port", "Port of the sending YARP device (e.g. AKD server)", true));

	//addProperty(Property("/nc/fgdb/rpc/server", "Yarp RPC Server Port", "Port for sending RPC requests to the YARP Device FinGeneDB", true));
	//addProperty(Property("/finspikes/client/rpc", "Yarp RPC Client Port", "Port for receiving RPC response from the YARP Device FinGeneDBs", true));
	addProperty(Property(Property::Integer, 0, "Buffered", "0 unbuffered, 1 buffered", true));
	addProperty(Property(Property::Integer, 0, "Timezone", "MysqlTimestamp Timezone: 0 MT5 (default), 1 TickDB (TimezoneDst 1)", true)); // LiveData vs QS (2023)

	// Refactor cmd finspikes logging for SpikeStream FinSpikes-Plugin
	addProperty(Property("null", "Buffer Log", "Log file of the Buffer", false));
	addProperty(Property("null", "Yarp Log", "Log file for Yarp", false));


	//Create description
	readerDescription = Description("Yarp Yolo Reader", "This reads yolo events from a yarp port", "Yolo Reader");

	m_yolo_event_time = 0ULL;

	m_network = NULL;
	m_port = NULL;
	m_processor = NULL;

	m_bottle = NULL;

	m_yarp_os = NULL;

	m_timezone = 0;

}
 


YarpAkdYoloReader::~YarpAkdYoloReader(){

	if(m_port != NULL) {	
		m_port->close();	//! Terminates the thread. Th Yarp server closes all releated connections.
		delete m_port;		//! and free the memory
	}

	if(m_processor != NULL) 
		delete m_processor;

	if(m_network != NULL) 
		delete m_network;  

}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/


void YarpAkdYoloReader::workerFunction() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	// ISSUE/TODO Critial Section ???, how to connect with data processor ???
	// the whole think already inherits from boost::Thread ?!
}


void YarpAkdYoloReader::convertBottle() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	// Checking yarp_nc ISO-Timestamp 
		unsigned long long seconds = 10*3600+31*60+25;
		int ms = 123;
        IsoTimestamp iso1(3600);	// Berlin, Bern 
		const char *converted = iso1.convert(seconds,ms);
		if(string(converted) != string("1970-01-01T10:31:25.123+01:00")) // +01:00 Berlin,Bern,.. in Winter without no DLZ
			m_yarp_state = YarpError;

		// Encoder/Decoder

		// Tick
		double price = 1.32001;
		double change = 0.00005;
		double spread = 0.00012;
		double ask = price + spread;
		double volume = 1.50;
 
		m_bottle = new Bottle();
		//Mt5TickEncoder encoder (*m_bottle, 3600);
		//encoder.msg_head("EURUSD", "Alpari", 0,
		//			seconds, ms,
		//			price, change, ask, spread, volume);
		//encoder.msg_indicators_head(0);
		//encoder.msg_indicators_tail();
		//encoder.msg_tail();

		//BottleDecoder decoder(*m_bottle, 3600);
		//TickAccessor accessor(decoder);

		//ConstString &symbol = accessor.symbol();
		//LOG(LOG_DEBUG) << "symbol: " << symbol;
		////string s(symbol.c_str());
		//double b = accessor.bid(); 
		//ConstString &timestamp = accessor.time();

		//if(timestamp != ConstString("1970-01-01T10:31:25.123+01:00"))
		//	m_yarp_state = YarpError;

		//LOG(LOG_DEBUG) << "timestamp: " << timestamp; 

		delete m_bottle;
		m_bottle = NULL;

}

//Inherited from PropertyHolder
void YarpAkdYoloReader::setProperties(map<string, Property>& properties){
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	AkdYoloReader::setProperties(properties); // generic

	if(yarpLog_os != NULL) {
		setYarpLogger(yarpLog_os);
		*yarpLog_os << "TEST" << endl;
	}

	m_reading_port_name = updateStringProperty(properties["Yarp Reading Port"]);
	m_writing_port_name = updateStringProperty(properties["Yarp Writing Port"]);
	//m_rpc_server_port_name = updateStringProperty(properties["Yarp RPC Server Port"]);
	//m_rpc_client_port_name = updateStringProperty(properties["Yarp RPC Client Port"]);
	m_buffered = updateIntegerProperty(properties["Buffered"]);
	m_timezone = updateIntegerProperty(properties["Timezone"]);
}


//Inherited from PropertyHolder
void YarpAkdYoloReader::start() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	LOGS(LOG_NOTICE, m_yarp_os) << " YarpAkdYoloReader: Receiving events from: " << m_writing_port_name << " on: " << m_reading_port_name;

	// Yarp Ports
	// http://eris.liralab.it/yarpdoc/port_expert.html

	// TODO Query Ranges => second Port, RPC,
	// Getting replies
	//Common::tick_t tick;

	//tick[0] = 1.28000; // absolute bid value, mapping to releativ is done in channel, reader does only buffer and transport values
	//tick[1] = -0.0020; // change against last tick
	//tick[2] = 5; // spread in points
	//tick[3] = 1.0; // volume in lots
	//m_min.resize(securities(), tick);
	//tick[0] = 1.33000; // absolute bid value, mapping to releativ is done in channel, reader does only buffer and transport values
	//tick[1] = 0.00020; // change against last tick
	//tick[2] = 15; // spread in points
	//tick[3] = 30.0; // volume in lots
	//m_max.resize(securities(), tick);

	Network yarp;
	//! NO RPC; since Ranges come by param !!!

//	// Instanciate RPC-Client
			
//	RpcClient port;
//	port.open(m_rpc_client_port_name.c_str());
//
////! \todo TradeEvent no not use GeneDb to determine min/max values, but params 
//
//	// TODO: Refactor to Yarp Service Class -> as friend of this- see above Processor
//	int rpcRetries = 4;
//	int rpcWait_ms = 100;
//	while (!isStopRequested() && port.getOutputCount()==0 && rpcRetries>0) { // TODO -> break
//			LOGS(LOG_INFO, m_yarp_os) << "Trying to connect to " << m_rpc_server_port_name.c_str();
//			yarp.connect(m_rpc_client_port_name.c_str(), m_rpc_server_port_name.c_str());
//			// TODO: if(port.getOutputCount()>0)
//			//	break;
//			boost::this_thread::sleep(boost::posix_time::milliseconds(rpcWait_ms));		
//			rpcRetries--;
//			rpcWait_ms *= 2; // TCP/IP like escalation
//	}	
//	if(port.getOutputCount()==0)
//	{
//		LOGS(LOG_ERROR, m_yarp_os) << "Connection with FinGeneDB failed (Timeout).";
//		return;
//	}
//	
//	// Fixture
//	ncboost::securities_t secs;
//	vector<Common::security_t>::iterator iter;
//	for(iter=m_securities.begin(); iter!=m_securities.end(); iter++) {
//		secs.push_back(ncboost::security_t(iter->first.c_str(), iter->second.c_str()));
//	}
//
//	// Encoding RPC Request
//	Bottle rpcRequest;
//	yarp::nc::fgdb::GetRangesRequestEncoder encoder(rpcRequest);
//	encoder.msg(secs);
//
//	Bottle rpcResponse;
//	LOGS(LOG_INFO, m_yarp_os) << "Sending RPC request... " << rpcRequest.toString().c_str();
//	port.write(rpcRequest, rpcResponse);
//	LOGS(LOG_INFO, m_yarp_os) << "Got response: " << rpcResponse.toString().c_str();
//
//	// Decode Bottle, check results
//	BottleDecoder decoder(rpcResponse, 0, 0);
//	ENUM_BOTTLE_MSG_TYPE msg_type;
//	msg_type = decoder.msg_type();
//	LOGS(LOG_DEBUG, m_yarp_os) << "RPC Response Message-Type:" << msg_type; 
//
//	yarp::nc::fgdb::GetRangesResponseAccessor accessor(decoder);
//	LOGS(LOG_DEBUG, m_yarp_os) << "nRanges:" << accessor.nRanges();
//	assert(accessor.nRanges() == securities());
//	
//	ncboost::security_range_t range_decoded;
//	m_min.resize(securities());
//	m_max.resize(securities());
//	for(int sec_i=0;sec_i<securities();sec_i++) {
//		accessor.range(sec_i, range_decoded);
//		for(int i=0; i<Common::TupleSize; i++) {
//			//! \todo implement for TradeEvent
//			//m_min[sec_i][i] = range_decoded[ncboost::SECURITY_RANGE_MIN][i];
//			//m_max[sec_i][i] = range_decoded[ncboost::SECURITY_RANGE_MAX][i];
//		}
//	}
//
//	port.close();
//

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

		if (!yarp.isConnected(m_writing_port_name.c_str(), m_reading_port_name.c_str()))
			if (!yarp.connect(m_writing_port_name.c_str(), m_reading_port_name.c_str()))
				LOGS(LOG_WARNING, m_yarp_os) << "Reading port could not be connected with writing port.";
			else
				LOGS(LOG_INFO, m_yarp_os) << "Reading port connected to writing port.";
		else
			LOGS(LOG_INFO, m_yarp_os) << "Connection between reading and writing port exists.";

		int portRetries = 10;	// Properties 
		int portWait_ms = 1000;
		while (!isStopRequested() && !yarp.connect(m_writing_port_name.c_str(), m_reading_port_name.c_str()) && portRetries>0) {
				// TODO: Refact in generic Function
				// if(yarp.isConnected(m_writing_port_name.c_str(), m_reading_port_name.c_str())
				//	break;
				LOGS(LOG_INFO, m_yarp_os) << "Trying to connect wport " << m_writing_port_name.c_str() << " with rport " << m_reading_port_name.c_str();
				//yarp.connect(m_rpc_client_port_name.c_str(), m_rpc_server_port_name.c_str());
				boost::this_thread::sleep(boost::posix_time::milliseconds(portWait_ms));
				//portWait_ms *= 2; // TCP/IP like escalation
				portRetries--;
		}
		if(!yarp.isConnected(m_writing_port_name.c_str(), m_reading_port_name.c_str()))  //   better than portRetries == 0
		{
			LOGS(LOG_WARNING, m_yarp_os) << "Connection of Read to Write Port failed. (Timeout)";
			return;
		}

		// end critical section
	} else 
	if (m_buffered == 1) {
	}

}


void YarpAkdYoloReader::syncSim(unsigned long long simtime)
{
	// begin critical section
	boost::mutex::scoped_lock lock(threadMutex); // controls the object mutex
	
	// --> parent
	AkdYoloReader::syncSim(simtime);

	m_yolo_event_time = simtime;

	// end critical section
}
