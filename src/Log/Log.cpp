//CARLsimIO includes
#include <CARLsimIO/Log/Log.hpp>
using namespace carlsimio;

//Other includes
#include <boost/date_time.hpp>
#include <stdio.h>

int Log::currentId = 0;

/** Constructor */
//Log::Log(){
//	os.precision(4);
//}
Log::Log(ostream* _os) : m_os(_os) {
	os.precision(1+5);  
}

/** Destructor */
Log::~Log(){
  os << endl;
  if(m_os == NULL) { 
	// default
	fprintf(stderr, "%s", os.str().c_str());
	fflush(stderr);
  } else {  
	  
  }
}


TLogLevel& Log::ReportingLevel(){
  static TLogLevel reportingLevel(LOG_DEBUG);
  return reportingLevel;
}


string Log::logLevelToString(TLogLevel level){
  string result;
  switch(level){
    case LOG_EMERGENCY:
      result =  "Emergency";
      break;
    case LOG_ALERT:
      result = "Alert";
      break;
    case LOG_CRITICAL:
      result = "Critical";
      break;
    case LOG_ERROR:
      result = "Error";
      break;
    case LOG_WARNING:
      result = "Warning";
      break;
    case LOG_NOTICE:
      result = "Notice";
      break;
    case LOG_INFO:
      result = "Info";
      break;
    case LOG_DEBUG:
      result = "Debug";
      break;
  }
  return result;
}

ostringstream& Log::Get(TLogLevel level){
   boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
   os << now;
   os << " " << this->logLevelToString(level) << ": " << currentId << ": ";
   size_t size = 9 - this->logLevelToString(level).size(); 
   os << string(size, ' ');
   currentId++;
   return os;
}

