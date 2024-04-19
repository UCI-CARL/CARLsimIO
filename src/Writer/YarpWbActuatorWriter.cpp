
#include <CARLsimIO/Writer/YarpWbActuatorWriter.hpp>
#include <CARLsimIO/Log/Log.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
using namespace carlsimio;

// STL includes
#include <sstream>
#include <iostream>
using namespace std;

// Yarp includes
#include <yarp/os/Bottle.h>
#include <yarp/os/Port.h>
#include <yarp/carl/StlUtils.h> 
#include <yarp/carl/AceUtils.h>
#include <yarp/os/ConstString.h>
#include <yarp/os/PortReader.h>
#include <yarp/os/Vocab.h>
#include <yarp/os/ConnectionReader.h>
#include <yarp/os/Network.h>
#include <yarp/os/RpcClient.h>

using namespace yarp::os;


//Names of properties used
#define PORT_NAME_PROP "Port Name"
#define SLEEP_DURATION_PROP "Sleep Duration ms"
#define YARP_RPC_CLIENT_PORT_NAME "Yarp RPC Client Port"
#define YARP_RPC_SERVER_PORT_NAME "Yarp RPC Server Port"
#define YARP_WRITING_PORT_NAME "Yarp Writing Port"
#define YARP_READING_PORT_NAME "Yarp Reading Port"


/** The default constructor, only initialises the default parameters and the description */
YarpWbActuatorWriter::YarpWbActuatorWriter() {

	addProperty(Property("/cns/outbound/wb/e-puck2", YARP_WRITING_PORT_NAME, "Outbound Port for writing the WbActuator commands", true));
	addProperty(Property("/wb/e-puck2/actuators", YARP_READING_PORT_NAME, "Port for receiving WbActuator commands", true));

	addProperty(Property("/wb/e-puck2/supervisor", YARP_RPC_SERVER_PORT_NAME, "Port for sending RPC requests to the WbRobot YARP Device (wrapper)", true));
	addProperty(Property("/cns/client/wb/e-puck2", YARP_RPC_CLIENT_PORT_NAME, "Port for receiving RPC response from the WbRobot YARP Device (wrapper)", true));

	addProperty(Property(Property::Integer, 5000, SLEEP_DURATION_PROP, "Amount to sleep in milliseconds in between sending command.", false));

	addProperty(Property("null", "Buffer Log", "Log file of the Buffer", false));
	addProperty(Property("null", "Yarp Log", "Log file for Yarp", false));

	//Create the description
	writerDescription = Description("Yarp WbActuator Writer", "This is a Yarp WbActuator writer", "WbActuator Writer");


	m_rpc_client = NULL; // created in ::start()


	m_yarp_os = NULL;

}



/** Destructor */
YarpWbActuatorWriter::~YarpWbActuatorWriter() {

	if (isRunning()) {
		requestStop();
		m_cv.notify_all(); 
		getThreadPointer()->join();
	}

	if (m_rpc_client != NULL) {
		m_rpc_client->close();
		delete m_rpc_client;
		m_rpc_client = NULL;
	}
}




/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

// Inherited from Writer
void YarpWbActuatorWriter::initialize(map<string, carlsimio::Property>& properties) {
	setProperties(properties); 
	setInitialized(true);
}


void YarpWbActuatorWriter::setActuatorValues(const vector<double>& values) {

	//!Ensure thread save access to the instance data member m_order and m_orderChanged.
	boost::lock_guard<boost::mutex> guard(m_resource_mutex);

	//!Copy the value from the parameter.
	m_actuatorValues = values;

	//!log a warning if the last changed order could not have been processed by the prcessing thread (yarp).
	bool skipped = m_actuator_changed;
	
	if (m_actuator_changed)
		LOGS(LOG_WARNING, m_yarp_os) << "The last changed order was skipped due Yarp is still processing";

	//!Set the changed variable, so that the waiting processing thread can determine if the order was changed.
	m_actuator_changed = true;
	m_actuator_time = m_simtime;
	//!Instantaneously inform the waiting processing thread that a new value is pending (sema like behavior).
	m_cv.notify_all();
	//!The mutex is automatically unlocked when the scope of the function is left.

	// Log 1 Element Buffer = Pending Order
	logBuffer(skipped);

}

//! performant but not thread save due it is called in critical secions only
void YarpWbActuatorWriter::logBuffer(bool skipped) {

	if (bufferLog_os == NULL)
		return;

	// derived from Functional Logging Interface of the Reader
	yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst()); // ISSUE2: -> see YarpTickReader
	const char* timestamp = isoTimestamp.convert(m_actuator_time / 1000, m_actuator_time % 1000);

	static const size_t n = 250;
	static char buf[n];

	if(m_actuatorPrefix == "vel")
		sprintf_s(buf, n, "%s @ %s : %f %f", m_actuatorPrefix.c_str(), timestamp, m_actuatorValues[0], m_actuatorValues[1]);
	else
		sprintf_s(buf, n, "actuator undefined %s", m_actuatorPrefix.c_str());

	*bufferLog_os << buf << endl;
}



//Inherited from PropertyHolder
void YarpWbActuatorWriter::setProperties(map<string, carlsimio::Property>& properties) {
	updateProperties(properties);

	WbActuatorWriter::setProperties(properties); // generic, inializes Log

	if (yarpLog_os != NULL) {
		setYarpLogger(yarpLog_os); // -> m_yarp_os
	}
}


// Inherited from iSpikeThread
void YarpWbActuatorWriter::start() {
	if (!isInitialized())
		throw CARLsimIOException("Cannot start YarpWbActuatorWriter thread - it has not been initialized.");
	if (isRunning())
		throw CARLsimIOException("Cannot start YarpWbActuatorWriter thread - it is already running.");


	// Instanciate RPC-Client
	yarp::os::Network yarp;
	m_rpc_client = new yarp::os::RpcClient(); 
	if (!m_rpc_client->open(m_rpc_client_port_name.c_str()))
		throw CARLsimIOException("Cannot open RPC Client");
	LOGS(LOG_INFO, m_yarp_os) << "rpc client opend " << m_rpc_server_port_name.c_str();

	// Ensure Connection 
	int rpcRetries = 3;
	int rpcWait_ms = 1000;
	while (!isStopRequested() && m_rpc_client->getOutputCount() == 0 && rpcRetries > 0) {
		LOGS(LOG_INFO, m_yarp_os) << "Trying to connect to " << m_rpc_server_port_name.c_str();
		yarp.connect(m_rpc_client_port_name.c_str(), m_rpc_server_port_name.c_str());  // LN20210708 see above
		rpcRetries--;
		boost::this_thread::sleep(boost::posix_time::milliseconds(rpcWait_ms));
	}
	if (m_rpc_client->getOutputCount() == 0)
	{
		LOGS(LOG_ERROR, m_yarp_os) << "Connection with TS failed (Timeout).";
		return;
	}
	else {
		LOGS(LOG_INFO, m_yarp_os) << "Connected to " << m_rpc_server_port_name.c_str();
	}


	m_network = new Network();
	m_port = new Port();
		
	m_port->open(m_writing_port_name.c_str());

	/*! The actual connection between the peers is delegated to the yarpserver
		by persistant connections by applying commands like the following:
			yarp connect --persist /mt5w/1.1 /finspikes/inbound/barinds
		A retry mechanismus fails when multiple channels shall be applied
		(they are initialized sequencially). Following the Yarp design philosophy
		the low-level protocoll shall be handled by yarp (sever). */
	if (!yarp.isConnected(m_writing_port_name.c_str(), m_reading_port_name.c_str()))
		if (!yarp.connect(m_writing_port_name.c_str(), m_reading_port_name.c_str()))
			LOGS(LOG_WARNING, m_yarp_os) << "Reading port could not be connected with writing port.";
		else
			LOGS(LOG_INFO, m_yarp_os) << "Reading port connected to writing port.";
	else
		LOGS(LOG_INFO, m_yarp_os) << "Connection between reading and writing port already exists.";


	this->setThreadPointer(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&YarpWbActuatorWriter::workerFunction, this))));
}




/*--------------------------------------------------------------------*/
/*---------                 PRIVATE METHODS                    -------*/
/*--------------------------------------------------------------------*/

/** Updates the properties with values from the map */
void YarpWbActuatorWriter::updateProperties(map<string, carlsimio::Property>& properties) {

	WbActuatorWriter::setProperties(properties); // generic

	if (yarpLog_os != NULL) {
		setYarpLogger(yarpLog_os); // -> m_yarp_os
	}

	sleepDuration_ms = updateIntegerProperty(properties[SLEEP_DURATION_PROP]);

	m_rpc_client_port_name = updateStringProperty(properties[YARP_RPC_CLIENT_PORT_NAME]);
	m_rpc_server_port_name = updateStringProperty(properties[YARP_RPC_SERVER_PORT_NAME]);

	m_reading_port_name = updateStringProperty(properties[YARP_READING_PORT_NAME]);
	m_writing_port_name = updateStringProperty(properties[YARP_WRITING_PORT_NAME]);
}


void YarpWbActuatorWriter::workerFunction() {
	setRunning(true);
	clearError();

	try {

		boost::unique_lock<boost::mutex> guard(m_process_mutex);

		boost::chrono::milliseconds abort_check(sleepDuration_ms);

		unsigned long long time;
		
		auto prefixVocab = Vocab::encode("vel");
		
		auto velVocab = Vocab::encode("vel");
		
		vector<double> actuatorValues(prefixVocab == velVocab ? 2 : -1, .0);  

		while (!isStopRequested()) {

			m_cv.wait_for(guard, abort_check);  // supposed to perform a unblocking wait 


			LOGS(LOG_DEBUG, m_yarp_os) << "YarpWbActuatorWriter: Checking if terminated.";
			if (isStopRequested())
				break;


			if (m_actuator_changed) {  // determines if the value was changed or just the timeout happened

				LOGS(LOG_DEBUG, m_yarp_os) << "YarpWbActuatorWriter: Enter critical section\t{";

				{
					boost::lock_guard<boost::mutex> guard(m_resource_mutex);
					actuatorValues = m_actuatorValues; // local copy
					time = m_actuator_time;
				}

				if (actuatorValues.empty()) {
					LOGS(LOG_WARNING, m_yarp_os) << "YarpWbActuatorWriter: Warning - actuator was changed but is empty.\t{";
					continue;
				}

				Bottle actuator; 

				if(prefixVocab == velVocab) {
					actuator.addVocab(prefixVocab);
					actuator.addDouble(actuatorValues[0]); 
					actuator.addDouble(actuatorValues[1]);
				}
				else {
					throw CARLsimIOException("Unsupported prefix");
				}

				m_port->write(actuator); 

				m_actuator_changed = false;
				m_processed++;
				m_last_ms = time;
				LOGS(LOG_DEBUG, m_yarp_os) << "YarpWbActuatorWriter: Leaving critical section.\t}";
			}

		}
	}
	catch (CARLsimIOException& ex) {
		setError(ex.msg());
	}
	catch (exception& ex) {
		LOGS(LOG_CRITICAL, m_yarp_os) << "YarpWbActuatorWriter exception: " << ex.what();
		setError("An exception occurred in the YarpWbActuatorWriter.");
	}
	catch (...) {
		setError("An unknown exception occurred in the YarpWbActuatorWriter");
	}

	setRunning(false);
}



// New Diagnostic Interface
size_t YarpWbActuatorWriter::getQueued() { return m_actuator_changed ? 1 : 0; }

unsigned long long YarpWbActuatorWriter::getNext_ms() { return m_actuator_changed ? m_actuator_time : 0L; }
