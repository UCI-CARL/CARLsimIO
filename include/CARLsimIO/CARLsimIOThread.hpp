#ifndef CARLSIMIOTHREAD_HPP
#define CARLSIMIOTHREAD_HPP

//CARLsimIO includes
#include <CARLsimIO/Log/Log.hpp>
#include <CARLsimIO/api.hpp>	

//STL includes
#include <string>
using namespace std;

//Boost includes 
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	/*! Methods common to all threads in iSpike along with error setting and retrieval */
	class CARLSIMIO_API CARLsimIOThread {

		public:
			/** Constructor */
			CARLsimIOThread(){
				errorFlag = false;
				running = false;
				stopRequested = false;
				errorMessage = "";
			}

			/** Virtual destructor */
			virtual ~CARLsimIOThread(){}

			/*! Clears the error in the thread */
			void clearError(){ errorFlag = false; errorMessage = ""; }

			/*! Returns the message associated with an error */
			string getErrorMessage() { return errorMessage; }

			/*! Returns true if an error has occurred */
			bool isError() { return errorFlag; }

			/*! Returns true if thread is running */
			bool isRunning() { return running; }

			/*! Returns true if thread has been requested to stop. */
			bool isStopRequested() { return stopRequested; }

			/*! Asks thread to stop running */
			void requestStop() { stopRequested = true; }

			/*! Sets an error in the thread */
			void setError(string errorMsg) { errorFlag = true; errorMessage = errorMsg; LOG(LOG_CRITICAL)<<"Thread error: "<< errorMsg; }

			/*! Sets running state to true */
			void setRunning(bool running) { this->running = running; stopRequested = !running; }

			/*!  Initialises the channel and starts the conversion thread  */
			virtual void start() = 0;


		protected:
			/** Thread run method */
			virtual void workerFunction() = 0;

			/** Returns pointer to thread */
			boost::shared_ptr<boost::thread> getThreadPointer() const { return threadPointer; }

			/** Sets thread pointer */
			void setThreadPointer(boost::shared_ptr<boost::thread> threadPointer) {	this->threadPointer = threadPointer; }

			/** Controls thread access to critical components  */
			boost::mutex threadMutex;


		private:
			/*! Records if an error has taken place in the thread.  */
			bool errorFlag;

			/*! Holds a message associated with an error in the thread */
			string errorMessage;

			/*! Flag to record whether thread is running */
			bool running;

			/*! Controls running of thread */
			bool stopRequested;

			/** Pointer to the running thread */
			boost::shared_ptr<boost::thread> threadPointer;

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif//CARLSIMIOTHREAD_HPP
