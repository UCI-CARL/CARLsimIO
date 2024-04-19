#ifndef WBSENSORREADER_HPP_
#define WBSENSORREADER_HPP_

//CARLsimIO includes
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Property.hpp>

#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Common.hpp>

#include <CARLsimIO/api.hpp>

#include <vector>
#include <deque>


#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {
	
	// Abstract
	class CARLSIMIO_API WbSensorReader : public Reader {
	public:

		WbSensorReader();
		virtual ~WbSensorReader();

		/** supposed to be abstract */
		virtual void initialize(map<string, Property>& properties);
		virtual void setProperties(map<string, Property>& properties);
		virtual void start() {}


		virtual void setSensorPrefix(string prefix) {m_sensor_prefix = prefix;}

		virtual void setTupleSize(int size) { m_tuple_size = size; }


			/** Sets the value tupel for the tick of the security just received.
			 The index is looked up before, e.g. by an efficient hash
			 */
		void appendTuple(const unsigned long long time, const std::vector<int> &values);   // sensor values, integer by type but size is configurable at runtime


		unsigned int lookup();


		void getMinMax(int &min, int &max) { min = m_min; max = m_max; }

		/** the actual values
		*/
		void getTuple(std::vector<int>& values, unsigned long long timestamp);

		/*! Sync with SNN simulator
		 * eg. dispose obsolete values, that are ticks that are older than their life span
		 * and checks for each security, if a newer value is now pending and that is still not ahead of the simtime
		 */
		virtual void syncSim(unsigned long long simtime);

		const deque<vector<int>>& getBuffer() { return m_buffer; }

		const vector<unsigned long long>& getTimestamp() { return m_timestamp; }


		virtual const unsigned long long getTickTime() { return 0ULL; }  // obs

		const unsigned long long getSimTime() { return m_simtime; }
		void setSimTime(const unsigned long long simtime) { m_simtime = simtime; }
		void setTickLifespan(const int lifespan) { m_lifespan = lifespan; }

		bool lastTickTime(unsigned long long& tick_time);

		/*! Old Interface, for use from outside the class */
		void logBufferHeader(std::ostream* os);
		void logBuffer(std::ostream* os, const char* simtime);

		/*! New Functional Log Interface */
		virtual void logBufferHeader();
		virtual void logBuffer(const char* simtime);  // ISSUE: simtime as parameter or member, which induces a dependency on syncTime?
		bool checkBufferBordersChanged();


		// New Diagnostic Interface
		virtual size_t getQueued() { return m_timestamp.size(); }  // Amount of Item in the buffer
		virtual unsigned long long getNext_ms() { return m_timestamp.size() > 0 ? m_timestamp.front() : 0L; }

		virtual void reset();

		virtual size_t getProcessed();
		virtual unsigned long long getLast_ms();


	protected:

		// Sensor Array is homogenous 
		string m_sensor_prefix;

		// Amount of elements in a sensor array input
		int m_tuple_size; 

		int m_min;

		int m_max;

		// tupel of variable size, alt. -> template 
		deque<std::vector<int>> m_buffer;   // deque allows the access by index
		// performance optimization allocate a value pool to avoid dynamic memory allocation

		/** Timestamps for the ticks (-> securities/tupels in the buffer) in ms.
			They are ordered ascending and might occur multiple, since ticks of several securities occur at the same instant
			The conversion from Timestamps takes place in the Yarp reader. */
		vector<unsigned long long> m_timestamp;

		/*! ms how long a tick is valid
		 * might be specific by security
		 */
		unsigned long long m_lifespan;

		unsigned long long m_simtime; // set by the channel 

		// New diagnostig interface
		unsigned long long m_last_ms;
		size_t m_processed;

		// Buffer Logging
		std::vector<int>* last_begin;
		std::vector<int>* last_end;


	private:
		void workerFunction() {}

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif
#endif /* WBSENSORREADER_HPP_ */
