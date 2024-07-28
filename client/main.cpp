/**
 * test client, allows creating a single input or output channel and then runs it indefinitely
 */

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <iostream>
#include <fstream>


//#include <boost/lexical_cast.hpp>

#include <CARLsimIO/Reader/ReaderFactory.hpp>
#include <CARLsimIO/Reader/YarpWbSensorReader.hpp>
#include <CARLsimIO/Reader/YarpAkdYoloReader.hpp>

//#include <CARLsimIO/Reader/ReaderDescription.hpp>
#include <CARLsimIO/Property.hpp>
#include <CARLsimIO/Channel/Channel.hpp>
#include <CARLsimIO/Channel/InboundChannel/WbSensorInboundChannel.hpp>
#include <CARLsimIO/Channel/InboundChannel/AkdYoloInboundChannel.hpp>
#include <CARLsimIO/Channel/InboundChannel/InboundChannelFactory.hpp>
#include <CARLsimIO/Channel/OutboundChannel/OutboundChannelFactory.hpp>
#include <CARLsimIO/Channel/OutboundChannel/OutboundChannel.hpp>
#include <CARLsimIO/Description.hpp>
#include <CARLsimIO/Writer/WriterFactory.hpp>
#include <CARLsimIO/Writer/Writer.hpp>
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Log/Log.hpp>  
#include <CARLsimIO/CARLsimIOException.hpp>

#include <yarp/carl/StlUtils.h> 
#include <yarp/carl/AceUtils.h>
//#include <yarp/os/impl/PlatformTime.h>

#include <ctime>


using namespace carlsimio;

#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
using namespace boost::chrono;


unsigned verbose = 0;


boost::program_options::options_description
programOptions()
{
	namespace po = boost::program_options;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "print this message")
		("verbose,v", po::value<unsigned>()->default_value(6), "Set verbosity from 0 (silent) to 7 (debug)")
		//("interactive,i", "Interaktive mode for defaults and set options")
		//("spikes", po::value<std::string>()->default_value("null"), "Outputs spikes to a file <arg> or standard out")
		//("buffer", po::value<std::string>()->default_value("null"), "Outputs buffer movements to a file <arg>or standard out")
		//("yarp", po::value<std::string>()->default_value("null"), "Outputs yarp movements to a file <arg>or standard out") 

		("spikes", po::value<std::string>()->default_value("cli_spikes.log"), "Outputs spikes to a file <arg> or standard out")
		("buffer", po::value<std::string>()->default_value("cli_buffer.log"), "Outputs buffer movements to a file <arg>or standard out")
		("yarp", po::value<std::string>()->default_value("cli_yarp.log"), "Outputs yarp movements to a file <arg>or standard out")


		("duration,d", po::value<double>()->default_value(180.0), "Duration of the Simulation in Seconds")
		("speed", po::value<double>()->default_value(0.5), "Speedfactor of the Simulation Time")
		("simtime", po::value<string>()->default_value(""), "Simulation time in ISO-Format (1970-01-01T00:00:00.000+01:00, 2013-03-20T14:16:50.000+01:00) or current local time (default)")
		("monitor", po::value<unsigned>()->default_value(1000), "ms for monitoring output")
		("autosync", po::value<unsigned>()->default_value(0), "1: waits until the buffer is not empty and use the first timestamp as simtime. 0: no synchronisation")
			// 1: wait for the first element, but keep simtime if defined
			// 2: like 1 but overwrite simtime
			// 3: like 2 but waits for buffer during simulation
		("lag", po::value<unsigned>()->default_value(0), "ms simulator behind model time")
		("backend", po::value<unsigned>()->default_value(1), "Simulation Backend: 0: CurrentRelay, 1: Spikes by embedded CPU, 2: Spikes by CARLsim")

		//// WbDistanceSensor Defaults    Use the Devices, it is irrelevant which Robot, see Webots.Devices
		//("channel", po::value<std::string>()->default_value("WbDistanceSensor"), "WbDistanceSensor, WbLightSensor, WbTofSensor, WbPositionSensor, WbAccelerometerSensor, WbGyroSensor")
		//("channel.lifespan", po::value<unsigned>()->default_value(4), "Lifespan of the bottle to induce current (and therefore ticks), default 64ms")
		//("channel.neuron_height", po::value<unsigned>()->default_value(1), "Neuron Height")
		//("channel.sd", po::value<double>()->default_value(0.5), "Standard Deviation")
		//("reader", po::value<std::string>()->default_value("YarpWbDistance"), "FileTick, YarpTick. Default: YarpWbDistance ")
		//("reader.reading_port", po::value<std::string>()->default_value("/cns/inbound/wb/e-puck2/ps"), "Inbound Port of Wb E-Puck sensor readings")
		//("reader.writing_port", po::value<std::string>()->default_value("/wb/e-puck2/sensors"), "Port of the sending WbRobot YARP device.")
		//("reader.rpc_server_port", po::value<std::string>()->default_value("/wb/e-puck2/supervisor"), "Port for sending RPC requests to the WbRobot YARP Device")
		//("reader.rpc_client_port", po::value<std::string>()->default_value("/cns/client/wb/epuck2"), "Port for receiving RPC response from the WbRobot YARP Device")
		//("reader.buffered", po::value<unsigned>()->default_value(0), "0 unbuffered, 1 buffered")

		// AkdYolo Defaults    Use the Devices, it is irrelevant which Robot, see Webots.Devices
		("channel", po::value<std::string>()->default_value("AkdYolo"), "AkdYolo, WbDistanceSensor, WbLightSensor, WbTofSensor, WbPositionSensor, WbAccelerometerSensor, WbGyroSensor")
		("channel.lifespan", po::value<unsigned>()->default_value(10), "Lifespan of the bottle to induce current (and therefore ticks), default 64ms")
		("channel.neuron_height", po::value<unsigned>()->default_value(2), "Neuron Height")
		("channel.sd", po::value<double>()->default_value(0.5), "Standard Deviation")
		("reader", po::value<std::string>()->default_value("YarpAkdYolo"), "FileTick, YarpTick. Default: YarpWbDistance, YarpAkdYolo ")
		("reader.reading_port", po::value<std::string>()->default_value("/cns/yolo:i"), "Inbound Port of AKD Yolo readings")
		("reader.writing_port", po::value<std::string>()->default_value("/akd/yolo:o"), "Port of the sending YOLO server")
		//("reader.rpc_server_port", po::value<std::string>()->default_value("/wb/e-puck2/supervisor"), "Port for sending RPC requests to the WbRobot YARP Device")
		//("reader.rpc_client_port", po::value<std::string>()->default_value("/cns/client/wb/epuck2"), "Port for receiving RPC response from the WbRobot YARP Device")
		("reader.buffered", po::value<unsigned>()->default_value(0), "0 unbuffered, 1 buffered")

		
		;

	return desc;
}


boost::program_options::variables_map
processOptions(int argc, char* argv[],
		const boost::program_options::options_description& desc)
{
	namespace po = boost::program_options;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS]\n\n";
		std::cout << desc << std::endl;
		exit(1);
	}

	return vm;
}

#include <CARLsimIO/buildinfo.h>
#include <CARLsimIO/libraryinfo.h>

int main(int argc, char* argv[])
{
	namespace po = boost::program_options; // namespace shortcut


	int build = APPLICATION_BUILD;
	int version_major = APPLICATION_VERSION_MAJOR;
	int version_minor = APPLICATION_VERSION_MINOR;
	int version_patch = APPLICATION_VERSION_PATCH; 
	printf("CARLsimIO - Neural Interface for CARLsim, Copyright (C) 2024 UCI CARL. All rights reserved.\n", version_major, version_patch, version_patch, build);


	try {
		po::options_description desc = programOptions();
		po::variables_map vm = processOptions(argc, argv, desc);
		int verbose = vm["verbose"].as<unsigned>();
		int monitor = vm["monitor"].as<unsigned>();
		Log::ReportingLevel() = (TLogLevel) verbose; 


		const string &vm_channel = vm["channel"].as<string>();
		LOG(LOG_INFO) << vm_channel << "Channel";


		// // ----------------------------------------
		if (vm_channel == "AkdYolo")
		{
			boost::scoped_ptr<AkdYoloInboundChannel> inboundChannel(new AkdYoloInboundChannel());
			std::map<std::string, Property> inboundChannelProperties = inboundChannel->getProperties();

			int neuronCount = vm["channel.neuron_height"].as<unsigned>();
			LOG(LOG_DEBUG) << "Neuron Height: " << neuronCount;
			inboundChannelProperties.at("Neuron Height").setInt(neuronCount);

			LOG(LOG_DEBUG) << "Standard Deviation: " << vm["channel.sd"].as<double>();
			inboundChannelProperties.at("Standard Deviation").setDouble(vm["channel.sd"].as<double>());

			inboundChannelProperties.at("Yolo Lifespan").setInt(vm["channel.lifespan"].as<unsigned>());

			LOG(LOG_DEBUG) << "Backend: " << vm["backend"].as<unsigned>();
			inboundChannelProperties.at("Backend").setInt(vm["backend"].as<unsigned>());

			inboundChannelProperties.at("Spikes Log").setString(vm["spikes"].as<string>());

			yarp::carl::MysqlTimestamp mysqlTimestamp;
			string vm_simtime = vm["simtime"].as<string>();
			unsigned long long msec_simtime;
			if (vm_simtime == "") {
				std::time_t time_now = std::time(NULL);
				msec_simtime = (time_now + yarp::carl::MysqlTimestamp::LocalTimezone()) * 1000;
			}
			else {
				mysqlTimestamp.convert(vm_simtime.c_str());
				msec_simtime = mysqlTimestamp.ace_time_value().get_msec();
			}

			const string& vm_reader = vm["reader"].as<string>();
			if (vm_reader == "YarpAkdYolo") {

				const string vm_yarp = vm["yarp"].as<string>();
				// Yarp - Output
				ostream* yarp_os = NULL;
				ofstream* yarp_ofs = NULL;
				if (vm_yarp == "cout")
					yarp_os = &cout;
				else if (vm_yarp != "null") {
					yarp_ofs = new ofstream(vm_yarp);
					yarp_os = yarp_ofs;
				}

				YarpAkdYoloReader* reader = new YarpAkdYoloReader();
				LOG(LOG_DEBUG) << "YarpAkdYoloReader: 0x" << reader;

				if (yarp_os != NULL)
					reader->setYarpLogger(yarp_os);

				map<string, Property> readerProps = reader->getProperties();
				LOG(LOG_DEBUG) << "Yarp Reading Port: " << vm["reader.reading_port"].as<string>();
				readerProps.at("Yarp Reading Port").setString(vm["reader.reading_port"].as<string>());
				LOG(LOG_DEBUG) << "Yarp Writing Port: " << vm["reader.writing_port"].as<string>();
				readerProps.at("Yarp Writing Port").setString(vm["reader.writing_port"].as<string>());
				// control the server, shutdown, stop, restart
				//LOG(LOG_DEBUG) << "Yarp RPC Server Port: " << vm["reader.rpc_server_port"].as<string>();
				//readerProps.at("Yarp RPC Server Port").setString(vm["reader.rpc_server_port"].as<string>());
				//LOG(LOG_DEBUG) << "Yarp RPC Client Port: " << vm["reader.rpc_client_port"].as<string>();
				//readerProps.at("Yarp RPC Client Port").setString(vm["reader.rpc_client_port"].as<string>());
				LOG(LOG_DEBUG) << "Buffered: " << vm["reader.buffered"].as<unsigned>();
				readerProps.at("Buffered").setInt(vm["reader.buffered"].as<unsigned>());

				readerProps.at("Buffer Log").setString(vm["buffer"].as<string>());
				readerProps.at("Yarp Log").setString(vm["yarp"].as<string>());

				reader->initialize(readerProps);


				LOG(LOG_DEBUG) << "initialize inbound channel..";
				inboundChannel->initialize(reader, inboundChannelProperties);

				// Diagnosting Output
				//const string vm_spikes = vm["spikes"].as<string>();
				//const string vm_buffer = vm["buffer"].as<string>();

				//// Spikes - Output
				//ostream* spikes_os = NULL;
				//ofstream* spikes_ofs = NULL;
				//if (vm_spikes == "cout")
				//	spikes_os = &cout;
				//else if (vm_spikes != "null") {
				//	spikes_ofs = new ofstream(vm_spikes);
				//	spikes_os = spikes_ofs;
				//}
				//if (spikes_os != NULL)
				//	inboundChannel->neuronSim.logFiringsHeader(*spikes_os, 0, 0, 8, -1);  // TOOD elements Common::TupleSize);

				//// Buffer - Output (TimeStamps, Securities, Ticks)   
				//ostream* buffer_os = NULL;
				//ofstream* buffer_ofs = NULL;
				//if (vm_buffer == "cout")
				//	buffer_os = &cout;
				//else if (vm_buffer != "null") {
				//	buffer_ofs = new ofstream(vm_buffer);
				//	buffer_os = buffer_ofs;
				//}
				//if (buffer_os != NULL)
				//	reader->logBufferHeader(buffer_os);


				// Check for logging
				double duration = vm["duration"].as<double>();
				double speed = vm["speed"].as<double>();
				unsigned autosync = vm["autosync"].as<unsigned>();
				unsigned lag = vm["lag"].as<unsigned>();

				if (lag > 0)
					msec_simtime -= lag;
				yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezone());
				const char* simstart = isoTimestamp.convert(msec_simtime / 1000, msec_simtime % 1000);
				LOG(LOG_INFO) << "Run neural spike generation for " << duration << "s" << " starting at: " << (autosync > 1 ? "<autosync>" : simstart);
				LOGS(LOG_DEBUG, yarp_os) << "SimStart: " << simstart;

				milliseconds timeout_ms(180 * 1000);  // Testing: 3 minutes 
				milliseconds buffer_ms(100);
				if (autosync > 0 && reader->getBuffer().empty()) {
					LOG(LOG_INFO) << "Waiting " << timeout_ms << " for the buffer to receive the first element...";
					while (reader->getBuffer().empty() && timeout_ms > buffer_ms) {
						boost::this_thread::sleep_for(buffer_ms);
						timeout_ms -= buffer_ms;
					}
					if (timeout_ms <= buffer_ms) {
						LOG(LOG_CRITICAL) << "No data feed (timeout)";
						exit(1);
					}
					if (autosync >= 2) {
						// overwrite simtime
						msec_simtime = reader->getTimestamp().front() - lag;
						simstart = isoTimestamp.convert(msec_simtime / 1000, msec_simtime % 1000);
						LOG(LOG_INFO) << "Received first element with timestamp " << simstart;
					}
				}

				time_point<process_real_cpu_clock, nanoseconds> cpu_start = process_real_cpu_clock::now();
				time_point<process_real_cpu_clock, nanoseconds> cpu_current;
				nanoseconds ns_ist, ns_sleep, ns_total_sleep, ns_last_sleep;

				inboundChannel->m_simtime = msec_simtime; // accessor, eliminate Property, eliminate reader.m_simtime
				unsigned long long fired = 0;
				for (int delay = 0; delay < 1000.0 * duration; delay++)
				{

					if (autosync >= 3 && reader->getBuffer().empty()) {
						milliseconds timeout_ms(300 * 1000);  // Development: 5 minutes
						milliseconds buffer_ms(5);
						LOG(LOG_INFO) << "Buffer is empty...";
						while (reader->getBuffer().empty() && timeout_ms > buffer_ms) {
							boost::this_thread::sleep_for(buffer_ms);
							timeout_ms -= buffer_ms;
						}
						if (timeout_ms <= buffer_ms) {
							LOG(LOG_ERROR) << "No more input (timeout)";
							break;
						}
						msec_simtime = max(msec_simtime, reader->getTimestamp().front() - lag);
					}

					cpu_current = process_real_cpu_clock::now();
					nanoseconds ns_soll((long long)(delay * 1.0e6 / speed));
					ns_ist = cpu_current - cpu_start;
					ns_sleep = ns_soll - ns_ist;
					ns_last_sleep = ns_sleep;
					if (ns_sleep < nanoseconds(0))
						ns_sleep = nanoseconds(0);
					else
						ns_total_sleep += ns_sleep;
					boost::this_thread::sleep_for(ns_sleep);


					const vector<unsigned>& firings = inboundChannel->getFiring();

					//if (spikes_os != NULL)
					//	inboundChannel->neuronSim.logFirings(*spikes_os, delay, inboundChannel->m_simtime, 8, -1);

					//if (buffer_os != NULL)
					//{
					//	AkdYoloReader* reader = inboundChannel->getReader();
					//}

					if (!firings.empty())
					{
						fired += firings.size();
					}

					inboundChannel->step();

					if (delay % monitor == monitor - 1) {
						const char* simtime = isoTimestamp.convert(inboundChannel->m_simtime / 1000, inboundChannel->m_simtime % 1000);
						LOG(LOG_INFO) << simtime << ": " << fired << " spikes fired after " << (delay + 1) / 1000 << "s. Buffer contains " << reader->getBuffer().size() << " ticks.";
						LOG(LOG_DEBUG) << ns_total_sleep << " thread sleep, " << ns_last_sleep << " last sleep";
					}

					inboundChannel->m_simtime++;

				}

				// clean-up
				//if (spikes_ofs != NULL) {
				//	spikes_ofs->close();
				//	delete spikes_ofs;
				//}
				//if (buffer_ofs != NULL) {
				//	buffer_ofs->close();
				//	delete buffer_ofs;
				//}
				//if (yarp_ofs != NULL) {
				//	yarp_ofs->close();
				//	delete yarp_ofs;
				//}
				reader = NULL; // Memory ownership is handed over to channel. Reader is freed in the destructor.

			}
			else
				if (vm_reader == "File") {


				}

		}


		// ----------------------------------------
		if(vm_channel == "WbDistanceSensor")
		{
			boost::scoped_ptr<WbSensorInboundChannel> inboundChannel(new WbSensorInboundChannel());
			std::map<std::string, Property> inboundChannelProperties = inboundChannel->getProperties();

			int neuronCount = vm["channel.neuron_height"].as<unsigned>();
			LOG(LOG_DEBUG) << "Neuron Height: " << neuronCount;
			inboundChannelProperties.at("Neuron Height").setInt(neuronCount);

			LOG(LOG_DEBUG) << "Standard Deviation: " << vm["channel.sd"].as<double>();
			inboundChannelProperties.at("Standard Deviation").setDouble(vm["channel.sd"].as<double>());

			inboundChannelProperties.at("Sensor Lifespan").setInt(vm["channel.lifespan"].as<unsigned>());

			LOG(LOG_DEBUG) << "Backend: " << vm["backend"].as<unsigned>();
			inboundChannelProperties.at("Backend").setInt(vm["backend"].as<unsigned>());  

			yarp::carl::MysqlTimestamp mysqlTimestamp;
			string vm_simtime = vm["simtime"].as<string>();
			unsigned long long msec_simtime;
			if(vm_simtime=="") {
				std::time_t time_now = std::time(NULL);
				msec_simtime = (time_now + yarp::carl::MysqlTimestamp::LocalTimezone()) * 1000; 
			} else {
				mysqlTimestamp.convert(vm_simtime.c_str());
				msec_simtime = mysqlTimestamp.ace_time_value().get_msec();
			}

			const string &vm_reader = vm["reader"].as<string>();
			if(vm_reader == "YarpWbDistance") {

				const string vm_yarp = vm["yarp"].as<string>();
				// Yarp - Output
				ostream* yarp_os = NULL;
				ofstream* yarp_ofs = NULL;
				if(vm_yarp=="cout")
					yarp_os = &cout;
				else if(vm_yarp!="null") {
					yarp_ofs = new ofstream(vm_yarp);
					yarp_os = yarp_ofs;
				}

				YarpWbSensorReader *reader = new YarpWbSensorReader();
				LOG(LOG_DEBUG) << "YarpWbSensorReader: 0x" << reader;

				if(yarp_os != NULL)
					reader->setYarpLogger(yarp_os);

				map<string, Property> readerProps = reader->getProperties();
				LOG(LOG_DEBUG) << "Yarp Reading Port: " << vm["reader.reading_port"].as<string>();
				readerProps.at("Yarp Reading Port").setString(vm["reader.reading_port"].as<string>()); 
				LOG(LOG_DEBUG) << "Yarp Writing Port: " << vm["reader.writing_port"].as<string>();
				readerProps.at("Yarp Writing Port").setString(vm["reader.writing_port"].as<string>()); 
				LOG(LOG_DEBUG) << "Yarp RPC Server Port: " << vm["reader.rpc_server_port"].as<string>();
				readerProps.at("Yarp RPC Server Port").setString(vm["reader.rpc_server_port"].as<string>()); 
				LOG(LOG_DEBUG) << "Yarp RPC Client Port: " << vm["reader.rpc_client_port"].as<string>();
				readerProps.at("Yarp RPC Client Port").setString(vm["reader.rpc_client_port"].as<string>()); 
				LOG(LOG_DEBUG) << "Buffered: " << vm["reader.buffered"].as<unsigned>();
				readerProps.at("Buffered").setInt(vm["reader.buffered"].as<unsigned>()); 
				reader->initialize(readerProps);

				LOG(LOG_DEBUG) << "initialize inbound channel..";
				inboundChannel->initialize(reader, inboundChannelProperties); 

				// Diagnosting Output
				const string vm_spikes = vm["spikes"].as<string>();
				const string vm_buffer = vm["buffer"].as<string>();

				// Spikes - Output
				ostream* spikes_os = NULL;
				ofstream* spikes_ofs = NULL;
				if(vm_spikes=="cout")
					spikes_os = &cout;
				else if(vm_spikes!="null") {
					spikes_ofs = new ofstream(vm_spikes);
					spikes_os = spikes_ofs;
				}
				if (spikes_os != NULL)
					inboundChannel->neuronSim.logFiringsHeader(*spikes_os, 0, 0, 8, -1);  // TOOD elements Common::TupleSize);

				// Buffer - Output (TimeStamps, Securities, Ticks)   
				ostream* buffer_os = NULL;
				ofstream* buffer_ofs = NULL;
				if(vm_buffer=="cout")
					buffer_os = &cout;
				else if(vm_buffer!="null") {
					buffer_ofs = new ofstream(vm_buffer);
					buffer_os = buffer_ofs;
				}
				if(buffer_os != NULL)
					reader->logBufferHeader(buffer_os);


				// Check for logging
				double duration = vm["duration"].as<double>();
				double speed = vm["speed"].as<double>();
				unsigned autosync = vm["autosync"].as<unsigned>();
				unsigned lag = vm["lag"].as<unsigned>();

				if(lag>0)
					msec_simtime -= lag;
				yarp::carl::IsoTimestamp isoTimestamp(yarp::carl::MysqlTimestamp::LocalTimezone());
				const char* simstart = isoTimestamp.convert(msec_simtime/1000, msec_simtime%1000);
				LOG(LOG_INFO) << "Run neural spike generation for " << duration << "s" << " starting at: " << (autosync>1?"<autosync>":simstart); 
				LOGS(LOG_DEBUG, yarp_os) << "SimStart: " << simstart;

				milliseconds timeout_ms(180 * 1000);  // Testing: 3 minutes 
				milliseconds buffer_ms(100);
				if(autosync>0 && reader->getBuffer().empty()) {
					LOG(LOG_INFO) << "Waiting " << timeout_ms << " for the buffer to receive the first element..."; 				
					while(reader->getBuffer().empty() && timeout_ms > buffer_ms) {
						boost::this_thread::sleep_for(buffer_ms);
						timeout_ms -= buffer_ms;
					}
					if(timeout_ms <= buffer_ms) {
						LOG(LOG_CRITICAL) << "No data feed (timeout)";
						exit(1);
					}
					if(autosync>=2) {
						// overwrite simtime
						msec_simtime = reader->getTimestamp().front() - lag;
						simstart = isoTimestamp.convert(msec_simtime/1000, msec_simtime%1000);
						LOG(LOG_INFO) << "Received first element with timestamp " << simstart;
					}
				}

				time_point<process_real_cpu_clock,nanoseconds> cpu_start = process_real_cpu_clock::now(); 
				time_point<process_real_cpu_clock,nanoseconds> cpu_current;
				nanoseconds ns_ist, ns_sleep, ns_total_sleep, ns_last_sleep;

				inboundChannel->m_simtime = msec_simtime; // accessor, eliminate Property, eliminate reader.m_simtime
				unsigned long long fired = 0; 
				for (int delay = 0; delay < 1000.0*duration; delay++)
				{ 

					if(autosync>=3 && reader->getBuffer().empty()){					
						milliseconds timeout_ms(300 * 1000);  // Development: 5 minutes
						milliseconds buffer_ms(5);
						LOG(LOG_INFO) << "Buffer is empty..."; 				
						while(reader->getBuffer().empty() && timeout_ms > buffer_ms) {
							boost::this_thread::sleep_for(buffer_ms);
							timeout_ms -= buffer_ms;
						}
						if(timeout_ms <= buffer_ms) {
							LOG(LOG_ERROR) << "No more input (timeout)";
							break;
						}
						msec_simtime = max(msec_simtime, reader->getTimestamp().front() - lag);
					}

					cpu_current = process_real_cpu_clock::now();
					nanoseconds ns_soll ((long long) (delay * 1.0e6 / speed)); 			
					ns_ist = cpu_current - cpu_start;
					ns_sleep = ns_soll - ns_ist;
					ns_last_sleep = ns_sleep;
					if(ns_sleep < nanoseconds(0))
						ns_sleep = nanoseconds(0);
					else
						ns_total_sleep += ns_sleep;
					boost::this_thread::sleep_for(ns_sleep);

	
					const vector<unsigned>& firings = inboundChannel->getFiring(); 

					if(spikes_os != NULL)
						inboundChannel->neuronSim.logFirings(*spikes_os, delay, inboundChannel->m_simtime, 8, -1);

					if(buffer_os != NULL)
					{
						WbSensorReader* reader = inboundChannel->getReader();
					}

					if (!firings.empty()) 
					{
						fired += firings.size();						
					}

					inboundChannel->step();

					if (delay % monitor == monitor-1) {
						const char* simtime = isoTimestamp.convert(inboundChannel->m_simtime /1000, inboundChannel->m_simtime %1000);
						LOG(LOG_INFO) << simtime << ": " << fired << " spikes fired after " << (delay+1)/1000 << "s. Buffer contains " << reader->getBuffer().size() << " ticks.";
						LOG(LOG_DEBUG) << ns_total_sleep << " thread sleep, " << ns_last_sleep << " last sleep";
					}

					inboundChannel->m_simtime++;   

				}

				// clean-up
				if(spikes_ofs!=NULL) {
					spikes_ofs->close();
					delete spikes_ofs;
				}
				if(buffer_ofs!=NULL) {
					buffer_ofs->close();
					delete buffer_ofs;
				}
				if(yarp_ofs!=NULL) {
					yarp_ofs->close();
					delete yarp_ofs;
				}
				reader = NULL; // Memory ownership is handed over to channel. Reader is freed in the destructor.

			} else 
			if(vm_reader == "File") {


			}

		}



	} catch(CARLsimIOException &ex) {
		std::cerr << ex.what() << std::endl;
		return -1;
	} catch(std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return -1;
	} catch(...) {
		std::cerr << "random: An unknown error occurred\n";
		return -1;
	}

	//

    return 0;
}
