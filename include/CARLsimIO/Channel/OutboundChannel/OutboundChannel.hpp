#ifndef OUTBOUNDCHANNEL_H_
#define OUTBOUNDCHANNEL_H_

#include <CARLsimIO/Channel/Channel.hpp>
#include <CARLsimIO/Description.hpp>
#include <CARLsimIO/Writer/Writer.hpp>

#include <CARLsimIO/api.hpp>

#include <iostream>
#include <fstream>

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** Represents a Channel that can be written to e.g. it is possible
		to send spikes to this channel, which are decoded into something else */
	class CARLSIMIO_API OutboundChannel : public Channel {
		public:

			OutboundChannel(): spikesLog_os(NULL), spikesLog_ofs(NULL),
				codingLog_os(NULL), codingLog_ofs(NULL) {}

			virtual ~OutboundChannel() {
				if(spikesLog_ofs != NULL) 
					{spikesLog_ofs->close();
					delete spikesLog_ofs;}				
				 spikesLog_ofs = NULL;
				 if(codingLog_ofs != NULL) 
					{codingLog_ofs->close();
					delete codingLog_ofs;}				
				 codingLog_ofs = NULL;
			}
			Description getChannelDescription() { return channelDescription; }

			/*! Initialize the channel with a reader and set of properties
			 *
			 * The Channel assumes ownership of the Writer object.
			 * */
			virtual void initialize(Writer* writer, map<string, Property>& properties) = 0;

			/*! Not supposed to be reimplemented */
			void setSpikesLog(const string logName) {
				// Spikes - Output
				spikesLog_os = NULL;
				if(spikesLog_ofs != NULL) {
					spikesLog_ofs->close();
					delete spikesLog_ofs;
				}
				spikesLog_ofs = NULL;
				if(logName=="cout")
					spikesLog_os = &cout;
				else if(logName!="null") {
					spikesLog_ofs = new ofstream(logName);
					spikesLog_os = spikesLog_ofs;
				}
			}

			/*! Not supposed to be reimplemented */
			void setCodingLog(const string logName) {
				// En/De-Coding - Output
				codingLog_os = NULL;
				if(codingLog_ofs != NULL) {
					codingLog_ofs->close();
					delete codingLog_ofs;
				}
				codingLog_ofs = NULL;
				if(logName=="cout")
					codingLog_os = &cout;
				else if(logName!="null") {
					codingLog_ofs = new ofstream(logName);
					codingLog_os = codingLog_ofs;
				}
			}


			/**  Sets the current spike pattern  */
			virtual void setFiring(const std::vector<unsigned>& buffer) = 0;

		protected:

			/** Description of the channel */
			Description channelDescription;

			// Generic Implementation of Spike Logging
			ostream *spikesLog_os;
			ofstream *spikesLog_ofs;

			// Generic Implementation of En/De-Coding Logging
			ostream *codingLog_os;
			ofstream *codingLog_ofs;

	};
}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* OUTBOUNDCHANNEL_H_ */
