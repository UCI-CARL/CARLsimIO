#ifndef AKDYOLOREADER_HPP_
#define AKDYOLOREADER_HPP_

//CARLsimIO includes
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Property.hpp>

#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Common.hpp>

#include <CARLsimIO/api.hpp>

#include <vector>
#include <deque>
#include <array>
#include <queue>

//namespace std {
//	class ostream;
//}

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

#include <CARLsimIO/AkdYoloEvent.hpp>

namespace carlsimio {


	// Abstract
	class CARLSIMIO_API AkdYoloReader : public Reader {
		public:

			AkdYoloReader();
			virtual ~AkdYoloReader();

			/** supposed to be abstract */
			virtual void initialize(map<string, Property>& properties); 
			virtual void setProperties(map<string, Property>& properties);
			virtual void start() {}


			/** 
			 */
			void appendTupel(const unsigned long long time, const yolo_event_t& tupel); 


			/** 
			 exc: not found -> ignore tick
			*/
			//unsigned int lookup();


			void getMinMax(vector<yolo_event_t> &min, vector<yolo_event_t> &max) {min = m_min; max = m_max; }

			/** the actual values
			*/
			void getTuple(yolo_event_t &event, unsigned long long timestamp);


			/*! Sync with SNN simulator
			 * eg. dispose obsolete values, that are ticks that are older than their life span
			 * and checks for each security, if a newer value is now pending and that is still not ahead of the simtime
			 */
			virtual void syncSim(unsigned long long simtime);
		              
			const vector<yolo_event_t> &getBuffer() { return m_buffer; }  //deque		

			const vector<unsigned long long>  &getTimestamp() { return m_timestamp; }

			virtual const unsigned long long getTickTime() {return 0ULL;}  // obs

			const unsigned long long getSimTime() {return m_simtime; }
			void setSimTime(const unsigned long long simtime) {m_simtime = simtime;}
			void setTickLifespan(const int lifespan) {m_lifespan = lifespan;}

			bool lastTickTime(unsigned long long &tick_time);	

			/*! Old Interface, for use from outside the class */
			void logBufferHeader(std::ostream* os);
			void logBuffer(std::ostream* os, const char* simtime);
			void getBufferBorders(yolo_event_t* &begin, yolo_event_t* &end);

			/*! New Functional Log Interface */
			virtual void logBufferHeader();
			virtual void logBuffer(const char* simtime);
			bool checkBufferBordersChanged();


			// New Diagnostic Interface
			virtual size_t getQueued() { return m_timestamp.size(); }  // Amount of Item in the buffer
			virtual unsigned long long getNext_ms() { return m_timestamp.size() > 0 ? m_timestamp.front() : 0L; } 

			virtual void reset();
		
			virtual size_t getProcessed();  
			virtual unsigned long long getLast_ms();

		protected:

			/** Minimum of the n-th security, loaded at startup (YARP or File) */
			vector<yolo_event_t> m_min;

			/** Maximum of the n-th security */
			vector<yolo_event_t> m_max;

			vector<yolo_event_t> m_buffer;   // queue lacks of access methods by index!

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
			//string bufferLog;
			//ostream *bufferLog_os;
			//ofstream *bufferLog_ofs;

			yolo_event_t* last_begin;
			yolo_event_t* last_end;


		private:
			void workerFunction() {}

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* AKDYOLOREADER_HPP_ */
