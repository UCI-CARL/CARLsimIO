#ifndef READER_HPP_
#define READER_HPP_

//iSpike includes
#include "CARLsimIO/CARLsimIOThread.hpp"
#include "CARLsimIO/PropertyHolder.hpp"
#include <CARLsimIO/Description.hpp>
#include <CARLsimIO/Common.hpp> 

//Other includes
#include <map>
#include <string>
using namespace std;

#include <CARLsimIO/api.hpp>	// CARLsimIO 2.1.2

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** This class represents a Reader, capable of retrieving analogue information
	  * from a predefined source and serving it upon request */
	class CARLSIMIO_API Reader : public PropertyHolder, public CARLsimIOThread {
		public:
			Reader() : initialized(false) { 
				Common::initializeLog(&bufferLog_os, &bufferLog_ofs); 
				Common::initializeLog(&yarpLog_os, &yarpLog_ofs); 
			}
			virtual ~Reader() {
				Common::releaseLog(&bufferLog_os, &bufferLog_ofs);
				Common::releaseLog(&yarpLog_os, &yarpLog_ofs);
			}
			virtual Description getReaderDescription() const { return readerDescription; }
			virtual void initialize(map<string, Property>& properties) = 0;
			virtual void initializeTract(const std::string tract) { this->tract = tract; };
			virtual bool isInitialized() { return initialized; }
			virtual void setInitialized(bool initialized) { this->initialized = initialized; }

			// New Diagnostic Interface
			virtual size_t getQueued() { return 0; }  // Amount of Item in the buffer
			virtual unsigned long long getNext_ms() { return 0L; }  // model ms of next pending Item in the buffer 

			// New Functional Log Interface
			virtual void logBufferHeader() {}
			virtual void logBuffer(const char* ) {}


			/*! New Support Interface 29.06.2013.
				Resets the Reader in the same state as it is when started the first time.
				Update 18.07.2013: Extended by generic Framework for Functional Logging */
			virtual void reset() {
				//! New Logging interface
				if(bufferLog_ofs!=NULL) { 
					Common::openLog(bufferLog, &bufferLog_os, &bufferLog_ofs);
					logBufferHeader(); 
				}
				if(yarpLog_ofs!=NULL) {
					Common::openLog(yarpLog, &yarpLog_os, &yarpLog_ofs);
				}
			}

			virtual size_t getProcessed() { return 0; }  // Amount of Item that have been Transferred from the Buffer for Processing 
			virtual unsigned long long getLast_ms() { return 0L; }  // ms modeltime of last processed message


		protected:
			//=======================  VARIABLES  =========================
			/** Description of the reader */
			Description readerDescription;

			string tract;

			/*! Functional Logging Interface - Common Datamembers for all Readers.*/

			// Buffer Logging
			string bufferLog;
			ostream *bufferLog_os;
			ofstream *bufferLog_ofs;

			// Yarp Logging
			string yarpLog;
			ostream *yarpLog_os;	// set to m_yarp_os !
			ofstream *yarpLog_ofs;


		private:
			/** Records whether reader has been initialized */
			bool initialized;

	};

}
#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* READER_HPP_ */
