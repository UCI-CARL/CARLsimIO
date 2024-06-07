
//CARLsimIO includes
#include <CARLsimIO/Reader/YarpWbSensorReader.hpp>
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
#include <yarp/carl/all.h>
#include <yarp/carl/StlUtils.h> 
//#include <yarp/carl/BottleDecoder.h>
#include <yarp/carl/AceUtils.h>
#include <yarp/os/ConstString.h>
#include <yarp/os/PortReader.h>
#include <yarp/os/ConnectionReader.h>
#include <yarp/os/Network.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Vocab.h>


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



// abstract
class YarpWbSensorReader::YarpDataProcessor {
public:
	YarpDataProcessor(YarpWbSensorReader* reader) : m_reader(reader)
	{
		m_isoTimestamp = new yarp::carl::IsoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst());
	} 
	virtual ~YarpDataProcessor() { m_reader = NULL; delete m_isoTimestamp; }
	void decode(Bottle& bottle)
	{
		LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;
		const char* simtime = m_isoTimestamp->convert(m_reader->m_simtime / 1000, m_reader->m_simtime % 1000);
		LOG(LOG_DEBUG) << "decode bottle @ " << simtime;

		LOG(LOG_DEBUG) << "Receiving yarp::Bottle " << bottle.toString().c_str();

		
		static const int N = 8;
		int unsigned long long t;
		int tof;
		int ps[N];
		int ls[N];
		int whls[2];

		int accl[3];  // x  y z  m/s^2 
		int gyro[3];  // gX GY GZ   // Neigung => Cost 

		bool b_t, b_tof, b_ps, b_ls, b_whls, b_accl, b_gyro;
		b_t = b_tof = b_ps = b_ls = b_whls = b_accl = b_gyro = false;

		int idx = 0;
		int n = bottle.size(); 

		// read next 
		auto nextVocabIs = [&](const char* sym) {
			if (!(idx < n))
				return false;
			auto vocab = bottle.get(idx).asVocab();
			if (vocab == Vocab::encode(sym)) {
				idx++;
				return true; 
			}
			else 
				return false; 
		};

		if (nextVocabIs("t")) {
			t = bottle.get(idx++).asInt64();
		} 

		if(nextVocabIs("tof")) {
			tof = bottle.get(idx++).asInt();
			b_tof = true; 
		} 

		if (nextVocabIs("ps")) {
			for(int i=0; i<N; i++)
				ps[i] = bottle.get(idx++).asInt();
			b_ps = true; 
		} 

		if (nextVocabIs("ls")) {
			for (int i = 0; i < N; i++)
				ls[i] = bottle.get(idx++).asInt();
			b_ls = true;
		} 

		if (nextVocabIs("whls")) {
			for (int i = 0; i < 2; i++)
				whls[i] = bottle.get(idx++).asInt();
			b_whls = true;
		} 

		if (nextVocabIs("gyro")) {
			for (int i = 0; i < 3; i++)
				gyro[i] = bottle.get(idx++).asInt();
			b_gyro = true;
		}

		if (nextVocabIs("accl")) {
			for (int i = 0; i < 3; i++)
				accl[i] = bottle.get(idx++).asInt();
			b_accl = true;
		}

		assert(idx == n); // all read, bottle formatted correctly

		std::vector<int> tuple(m_reader->m_tuple_size, 0.0);

		if (b_ps && m_reader->m_sensor_prefix == "ps")
			for (int i = 0; i < m_reader->m_tuple_size; i++)
				tuple[i] = ps[i];
		else
			if (b_ls && m_reader->m_sensor_prefix == "ls")
				for (int i = 0; i < m_reader->m_tuple_size; i++)
					tuple[i] = ls[i];
			else
				if (b_gyro && m_reader->m_sensor_prefix == "gyro")
					for (int i = 0; i < m_reader->m_tuple_size; i++)
						tuple[i] = gyro[i];
				else
					if (b_accl && m_reader->m_sensor_prefix == "accl")
						for (int i = 0; i < m_reader->m_tuple_size; i++)
							tuple[i] = accl[i];
					else
						if (b_whls && m_reader->m_sensor_prefix == "whls")
							for (int i = 0; i < m_reader->m_tuple_size; i++)
								tuple[i] = whls[i];
						else
							if (b_tof && m_reader->m_sensor_prefix == "tof")
								tuple[0] = tof;
							else {
								LOGS(LOG_INFO, m_reader->m_yarp_os) << "No sensory data for prefix " << m_reader->m_sensor_prefix;
								return;
							}
		

		//unsigned long long timestamp = t;
		unsigned long long timestamp = m_reader->m_tick_time;  
		LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "timestamp(ms): " << timestamp;


		// There is exactly one Yarp thread to be synchronized with the Qt thread
		// Critical Section Start
		{
			boost::mutex::scoped_lock lock(m_reader->threadMutex); // this is a reference to the object's mutex

			unsigned long long lastTickTime;
			if (m_reader->lastTickTime(lastTickTime)) {
				if (m_reader->m_simtime > 0 && timestamp < m_reader->m_simtime) // 21.04.2013 fix for uninitialized member
				{
					LOGS(LOG_INFO, m_reader->m_yarp_os) << "Skipping obsolete tick " << timestamp << "ms at SimTime: " << m_reader->getSimTime() << "ms";
					return;
				}
				else
					if (timestamp < lastTickTime)
					{
						LOGS(LOG_NOTICE, m_reader->m_yarp_os) << "insert tick " << timestamp << "ms before m_timestamp:" << lastTickTime << "ms";
						m_reader->m_timestamp.insert(m_reader->m_timestamp.begin(), timestamp);
						m_reader->m_buffer.insert(m_reader->m_buffer.begin(), tuple);

						if (m_reader->bufferLog_os != NULL && m_reader->checkBufferBordersChanged())
							m_reader->logBuffer(m_isoTimestamp->convert(m_reader->m_simtime / 1000, m_reader->m_simtime % 1000));
						LOGS(LOG_NOTICE, m_reader->m_yarp_os) << "done.";
						return; // Mutex is handled automatically by leaving the scope
					}
			}

			LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "Begin append tick to the buffer";
			m_reader->m_timestamp.push_back(timestamp); // simtime is managend central and set in Channel
			m_reader->m_buffer.push_back(tuple);
			LOGS(LOG_INFO, m_reader->m_yarp_os) << "Append WbSensor " << " ps  @" << timestamp << "ms";
			if (m_reader->bufferLog_os != NULL && m_reader->checkBufferBordersChanged())
				m_reader->logBuffer(m_isoTimestamp->convert(m_reader->m_simtime / 1000, m_reader->m_simtime % 1000));
			LOGS(LOG_DEBUG, m_reader->m_yarp_os) << "done.";

		}
	}

protected:
	YarpWbSensorReader* m_reader;  // reserved
	IsoTimestamp* m_isoTimestamp;
};


// unbuffered
class YarpWbSensorReader::UnbufferedDataProcessor : public PortReader, public YarpWbSensorReader::YarpDataProcessor {
public:
	UnbufferedDataProcessor(YarpWbSensorReader* reader) : YarpWbSensorReader::YarpDataProcessor(reader), PortReader() {}
	virtual ~UnbufferedDataProcessor() {}
	bool read(ConnectionReader& connection) {
		LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;
		Bottle bottle;
		if (bottle.read(connection)) {
			decode(bottle);
			return true;
		}
		else
			return false;
	}
};


YarpWbSensorReader::YarpWbSensorReader() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	// Define the properties of this reader
	addProperty(Property("/cns/inbound/wb/e-puck2/ps", "Yarp Reading Port", "Inbound Port of CNS for Sensor readings", true));  // reads complete message, filter specific event e.g. ps or ls
	addProperty(Property("/wb/e-puck2/sensors", "Yarp Writing Port", "Port of the sending WbRobot YARP device (wrapper)", true));
	addProperty(Property("/wb/e-puck2/supervisor", "Yarp RPC Server Port", "Port for sending RPC requests to the WbRobot YARP Device (wrapper)", true));
	addProperty(Property("/cns/client/wb/epuck2", "Yarp RPC Client Port", "Port for receiving RPC response from the WbRobot YARP Device (wrapper)", true));
	addProperty(Property(Property::Integer, 0, "Buffered", "0 unbuffered, 1 buffered", true));
	addProperty(Property(Property::Integer, 0, "Timezone", "MysqlTimestamp Timezone: 0 MT5 (default), 1 TickDB (TimezoneDst 1)", true)); // LiveData vs QS (2023)

	// Refactor cmd finspikes logging for SpikeStream FinSpikes-Plugin
	addProperty(Property("null", "Buffer Log", "Log file of the Buffer", false));
	addProperty(Property("null", "Yarp Log", "Log file for Yarp", false));

	//Create description
	readerDescription = Description("Yarp WbSensor Reader", "This reads sensor data from a yarp WbRobot sensor port", "WbSensor Reader");

	m_tick_time = 0ULL;

	m_network = NULL;
	m_port = NULL;
	m_processor = NULL;

	m_bottle = NULL; 

	m_yarp_os = NULL;

	m_timezone = 0; 

}



YarpWbSensorReader::~YarpWbSensorReader() {
	
	if (m_port != NULL) {
		m_port->close();	//!< Terminates the thread. Th Yarp server closes all releated connections.
		delete m_port;		//!< and free the memory
	}
	if (m_processor != NULL) delete m_processor;
	if (m_network != NULL) delete m_network;

}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/


void YarpWbSensorReader::workerFunction() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

}


void YarpWbSensorReader::convertBottle()
{
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	m_bottle = NULL;

}

//Inherited from PropertyHolder
void YarpWbSensorReader::setProperties(map<string, carlsimio::Property>& properties) {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	WbSensorReader::setProperties(properties); // generic

	if (yarpLog_os != NULL) {
		setYarpLogger(yarpLog_os); // -> m_yarp_os
		*yarpLog_os << "TEST" << endl;
	}

	m_reading_port_name = updateStringProperty(properties["Yarp Reading Port"]);
	m_writing_port_name = updateStringProperty(properties["Yarp Writing Port"]);
	m_rpc_server_port_name = updateStringProperty(properties["Yarp RPC Server Port"]);
	m_rpc_client_port_name = updateStringProperty(properties["Yarp RPC Client Port"]);
	m_buffered = updateIntegerProperty(properties["Buffered"]);
	m_timezone = updateIntegerProperty(properties["Timezone"]);
}


//Inherited from PropertyHolder
void YarpWbSensorReader::start() {
	LOG(LOG_DEBUG) << "tid:" << GetCurrentThreadId() << " " << __FUNCTION__;

	LOGS(LOG_NOTICE, m_yarp_os) << " YarpWbSensorReader: Receiving ticks from: " << m_writing_port_name << " on: " << m_reading_port_name;

	// Instanciate RPC-Client
	Network yarp;
	RpcClient port;
	port.open(m_rpc_client_port_name.c_str());

	int rpcRetries = 10;
	int rpcWait_ms = 100;
	while (!isStopRequested() && port.getOutputCount() == 0 && rpcRetries > 0) { 
		LOGS(LOG_INFO, m_yarp_os) << "Trying to connect to " << m_rpc_server_port_name.c_str();
		yarp.connect(m_rpc_client_port_name.c_str(), m_rpc_server_port_name.c_str());
		boost::this_thread::sleep(boost::posix_time::milliseconds(rpcWait_ms));
		rpcRetries--;
		rpcWait_ms *= 2; // TCP/IP like escalation
	}
	if (port.getOutputCount() == 0)
	{
		LOGS(LOG_ERROR, m_yarp_os) << "Connection with FinGeneDB failed (Timeout).";
		return;
	}

	// Encoding RPC Request
	Bottle rpcRequest;
	rpcRequest.addString("range");  // check if sensor (array) was initialized and is ready

	rpcRequest.addString(m_sensor_prefix);  // check if sensor (array) was initialized and is ready

	Bottle rpcResponse;
	LOGS(LOG_INFO, m_yarp_os) << "Sending RPC request... " << rpcRequest.toString().c_str();
	port.write(rpcRequest, rpcResponse);
	LOGS(LOG_INFO, m_yarp_os) << "Got response: " << rpcResponse.toString().c_str();

	m_max = rpcResponse.pop().asInt();
	m_min = rpcResponse.pop().asInt();

	port.close();


	// if unbuffered  
	if (m_buffered == 0) {
		m_network = new Network();
		m_port = new Port();
		UnbufferedDataProcessor* unbuffered_processor = new UnbufferedDataProcessor(this);
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

		// end critical section
	}
	else
		if (m_buffered == 1) {
		}

}


void YarpWbSensorReader::syncSim(unsigned long long simtime)
{
	// begin critical section
	boost::mutex::scoped_lock lock(threadMutex); // controls the object mutex


	// --> parent
	WbSensorReader::syncSim(simtime);

	m_tick_time = simtime; 

	// end critical section
}

