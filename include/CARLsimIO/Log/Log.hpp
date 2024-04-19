#ifndef LOG_HPP_
#define LOG_HPP_

//Other includes
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <CARLsimIO/api.hpp>	

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


/**
 * For a description on how to use the log levels, see http://blog.pantheios.org/2010/10/choosing-severity-levels.html
 */
enum TLogLevel {
	LOG_EMERGENCY, LOG_ALERT, LOG_CRITICAL, LOG_ERROR,
	LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG
};

#define LOG(level) \
	if (level > Log::ReportingLevel()) ; \
	else Log().Get(level)

#define LOGS(level, _ostream) \
	if (_ostream == NULL && level > Log::ReportingLevel()) ; \
	else Log(_ostream).Get(level)


namespace carlsimio {

	/** A simple logging class based on this article: http://drdobbs.com/cpp/201804215?pgno=1  */
	class CARLSIMIO_API Log {
		public:
			Log(ostream* os=NULL);
			virtual ~Log();
			ostringstream& Get(TLogLevel level = LOG_DEBUG);
			static TLogLevel& ReportingLevel();
			static int currentId;

		protected:
			ostringstream os; // oss
			ostream* m_os;


		private:
			Log(const Log&);
			Log& operator =(const Log&);
			string logLevelToString(TLogLevel level);
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* LOG_HPP_ */
