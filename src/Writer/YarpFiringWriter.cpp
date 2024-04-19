
// CARLsimIO includes 
#include <CARLsimIO/Writer/YarpFiringWriter.hpp>
#include <CARLsimIO/Log/Log.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
using namespace carlsimio;

// STL includes
#include <sstream>
#include <iostream>
using namespace std;

// Yarp includes
#include <yarp/os/Bottle.h>
#include <yarp/carl/all.h>
//#include <yarp/carl/BottleDecoder.h> 
#include <yarp/carl/StlUtils.h> 
#include <yarp/carl/AceUtils.h>
#include <yarp/os/Network.h>
#include <yarp/os/RpcClient.h>



// is appended by group name 
#define YARP_PORTPREFIX_NAME "Yarp Portprefix"
//#define NEURON_GROUP_NAME "Neuron Group"  // replaced by tract


/** The default constructor, only initialises the default parameters and the description */
YarpFiringWriter::YarpFiringWriter() {


	// Note: append neuron group 
	addProperty(Property("/cns/outbound/firing", YARP_PORTPREFIX_NAME, "Prefix for outbound port firing", true));


	// Refactor logging for CARLsimIO-Plugin
	addProperty(Property("null", "Buffer Log", "Log file of the Buffer", false));
	addProperty(Property("null", "Yarp Log", "Log file for Yarp", false));

	//Create the description
	writerDescription = Description("Yarp Firing Writer", "This is a Yarp order writer", "Firing Writer");

	m_port = NULL;
	m_yarp_os = NULL;

}



/** Destructor */
YarpFiringWriter::~YarpFiringWriter(){

	if(isRunning()){
	}

	if (m_port != NULL) {
		m_port->close();
		delete m_port;
		m_port = NULL;
	}

}




/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

// Inherited from Writer
void YarpFiringWriter::initialize(map<string, Property>& properties){
	//updateProperties(properties);
	setProperties(properties); // LN 20.09.2013
	setInitialized(true);
}


void YarpFiringWriter::setFiring(const std::vector<unsigned>& aer) {
	//!Ensure thread save access to the instance data member m_order and m_orderChanged.
	boost::lock_guard<boost::mutex> guard(m_resource_mutex); 

	//!Copy the value from the parameter.
	this->firing = aer; // thread save copy

	//!log a warning if the last changed order could not have been processed by the prcessing thread (yarp).
	bool skipped = m_firing_changed;
	if(m_firing_changed) 
		LOGS(LOG_WARNING, m_yarp_os) << "The last changed order was skipped due Yarp is still processing"; 

	//!Set the changed variable, so that the waiting processing thread can determine if the order was changed.
	m_firing_changed = true;
	m_firing_time = m_simtime;

	//!Instantaneously inform the waiting processing thread that a new value is pending (sema like behavior).
	//! 
	m_cv.notify_all();
	//!The mutex is automatically unlocked when the scope of the function is left.

	// Log 1 Element Buffer = Pending Order
	logBuffer(skipped);
}



//! performant but not thread save due it is called in critical secions only
void YarpFiringWriter::logBuffer(bool skipped) {

	if(bufferLog_os == NULL)
		return;

	// derived from Functional Logging Interface of the Reader
	yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezoneDst()); // ISSUE2: -> see YarpTickReader
	const char* timestamp = isoTimestamp.convert(m_firing_time/1000, m_firing_time%1000);

}




//Inherited from PropertyHolder
void YarpFiringWriter::setProperties(map<string, Property> &properties){
	updateProperties(properties);

	FiringWriter::setProperties(properties); // generic, inializes Log

	if(yarpLog_os != NULL) {
		setYarpLogger(yarpLog_os); // -> m_yarp_os
	}
}


// Inherited from iSpikeThread
void YarpFiringWriter::start(){
	if(!isInitialized())
		throw CARLsimIOException("Cannot start YarpFiringWriter thread - it has not been initialized.");
	if(isRunning())
		throw CARLsimIOException("Cannot start YarpFiringWriter thread - it is already running.");


	yarp::os::Network yarp;

	string port = yarpPortPrefix;
	m_port = new yarp::os::Port;
	if (!m_port->open(port)) {
		throw CARLsimIOException(std::string("Cannot open port: ") + port);
	}

	this->setThreadPointer(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&YarpFiringWriter::workerFunction, this))));
}




/*--------------------------------------------------------------------*/
/*---------                 PRIVATE METHODS                    -------*/
/*--------------------------------------------------------------------*/

/** Updates the properties with values from the map */
void YarpFiringWriter::updateProperties(map<string, Property>& properties){
	yarpPortPrefix = updateStringProperty(properties[YARP_PORTPREFIX_NAME]);
}


//Inherited 
void YarpFiringWriter::workerFunction(){
	setRunning(true);
	clearError();

	try{

		boost::unique_lock<boost::mutex> guard(m_process_mutex);
		boost::chrono::milliseconds abort_check(sleepDuration_ms);
		unsigned long long time;

		while(!isStopRequested()){

			LOGS(LOG_DEBUG, m_yarp_os)<<"YarpFiringWriter: Checking if terminated.";
			if(isStopRequested())
				break;


			if(m_firing_changed && !firing.empty()) {  // determines if the value was changed or just the timeout happened
				LOGS(LOG_DEBUG, m_yarp_os)<<"YarpFiringWriter: Enter critical section\t{";


				yarp::os::Bottle msg;
				
				msg.addString(tract);

				yarp::carl::IsoTimestamp ts(yarp::carl::MysqlTimestamp::LocalTimezoneDst());  
				msg.addString(ts.convert(m_simtime / 1000, m_simtime % 1000));
				msg.addString("AER");

				// content
				yarp::os::Bottle aer; 
				for (auto iter = firing.begin(); iter < firing.end(); iter++) {
					aer.addInt(*iter);
				}					
				msg.addList() = aer; 


				bool ok = m_port->write(msg);

				m_firing_changed = false;
				m_processed++;
				LOGS(LOG_DEBUG, m_yarp_os)<<"YarpFiringWriter: Leaving critical section.\t}";
			}
				
		}
	}
	catch(CARLsimIOException& ex){
		setError(ex.msg());
	}
	catch(exception& ex){
		LOGS(LOG_CRITICAL, m_yarp_os)<<"YarpFiringWriter exception: "<<ex.what();
		setError("An exception occurred in the YarpFiringWriter.");
	}
	catch(...){
		setError("An unknown exception occurred in the YarpFiringWriter");
	}

	setRunning(false);
}



// New Diagnostic Interface
size_t YarpFiringWriter::getQueued() { return m_firing_changed ? 1 : 0; }

unsigned long long YarpFiringWriter::getNext_ms() { return m_firing_changed ? m_firing_time : 0L; }  
