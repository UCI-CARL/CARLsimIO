#ifndef FIRINGREADER_HPP_
#define FIRINGREADER_HPP_

//CARLsimIO includes
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Property.hpp>

// -> .cpp
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Common.hpp>

#include <CARLsimIO/api.hpp>

#include <vector>
#include <array>
#include <queue>

#include <carl/boost_ext.hpp>

//namespace std {
//	class ostream;
//}

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {


	// Abstract
	class CARLSIMIO_API FiringReader : public Reader {
		public:

			FiringReader();
			virtual ~FiringReader();

			/** supposed to be abstract */
			virtual void initialize(map<string, Property>& properties); 
			virtual void setProperties(map<string, Property>& properties);
			virtual void start() {}



			/** 
			 */
			//void appendTupel(const unsigned security, const unsigned long long time, const ncboost::trade_event_t& tupel); 
			void appendTupel(const unsigned long long time, const vector<unsigned int>& aer);


			/** the actual values
			*/
			//const vector<tick_t> getTupel(const unsigned long long from, const unsigned long long to);
			//void getTuple(ncboost::trade_event_t &tick, unsigned security, unsigned long long timestamp);
			void getTuple(vector<unsigned int>& aer, unsigned long long timestamp);


			/*! Sync with SNN simulator 
			 * eg. dispose obsolete values, that are ticks that are older than their life span
			 * and checks for each security, if a newer value is now pending and that is still not ahead of the simtime
			 */
			virtual void syncSim(unsigned long long simtime); 
	
			//const vector<ncboost::trade_event_t> &getBuffer() { return m_buffer; }
			const vector<vector<unsigned int>>& getBuffer() { return m_buffer; }
			const vector<unsigned long long>  &getTimestamp() { return m_timestamp; }
			//const vector<unsigned int> &getSecurity() { return m_security; }

			virtual const unsigned long long getTickTime() {return 0ULL;}  // obs

			const unsigned long long getSimTime() {return m_simtime; }
			void setSimTime(const unsigned long long simtime) {m_simtime = simtime;}
			void setTickLifespan(const int lifespan) {m_lifespan = lifespan;}

			//bool lastTickTime(unsigned security, unsigned long long &tick_time, size_t &index);	
			bool lastTickTime(unsigned long long& tick_time, size_t& index);

			/*! Old Interface, for use from outside the class */
			void logBufferHeader(std::ostream* os);
			void logBuffer(std::ostream* os, const char* simtime);

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

			//! ugly, since scope is merely a Yarp Concept...
			virtual bool hasScopePort() { return false; }  
			virtual void publishDopamine(double delta, double level) {};

		protected:


			vector<vector<unsigned int>> m_buffer;   // queue lacks of access methods by index!

			vector<unsigned long long> m_timestamp; 

			/*! ms how long a tick is valid 
			 * might be specific by security  
			 */
			unsigned long long m_lifespan; 

			unsigned long long m_simtime; // set by the channel 

			// New diagnostig interface
			unsigned long long m_last_ms; 
			size_t m_processed; 

			vector<unsigned int>* last_begin;
			vector<unsigned int>* last_end;


		private:
			void workerFunction() {}

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif
#endif /* FIRINGREADER_HPP_ */
