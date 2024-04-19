#ifndef FIRINGOUTBOUNDCHANNEL_HPP_
#define FIRINGOUTBOUNDCHANNEL_HPP_

#include <CARLsimIO/Channel/OutboundChannel/OutboundChannel.hpp>
#include <CARLsimIO/Writer/FiringWriter.hpp>
#include <CARLsimIO/Log/Log.hpp>
#include <CARLsimIO/Common.hpp>	

#include <string>
#include <vector>

#include <CARLsimIO/api.hpp>	


#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** Encodes joint angles into spikes */
	class CARLSIMIO_API FiringOutboundChannel : public OutboundChannel {
		public:
			FiringOutboundChannel();
			virtual ~FiringOutboundChannel();
			void initialize(Writer* writer, map<string,Property>& properties);
			void setFiring(const std::vector<unsigned>& buffer);
			void setProperties(map<string, Property>& properties);
			void step();


			// New Diagnostic Interface
			virtual size_t getBufferQueued(); // Amount of Item in the buffer/pending
			virtual unsigned long long getBufferNext();  // model ms of next pending Item in the buffer 
			virtual void reset();
			virtual size_t getReaderProcessed();  // Amount of Item that have been processed by the channel
			virtual unsigned long long getReaderLast();  // model ms of last item processed by the channel

			/** Retrieves the height of the Channel  */
			int getSparseCoding() { return this->sparseCoding; }

			/** Sets the height of the Channel */
			void setSparseCoding(int sparseCoding) { this->sparseCoding = sparseCoding; }

			/** Sets the axonal tract of the Channel */
			void setAxonalTract(const std::string axonalTract) { this->axonalTract = axonalTract; }

		protected:
			void updateProperties(map<string, Property>& properties);

			void logFiringsHeader(long long simtime, long long tick_time, int group, int cluster);
			void logFirings(const std::vector<unsigned>& buffer, long long simtime, long long tick_time, int group, int cluster);


		private:
			/** Sends firing to YARP, file, etc. */
			FiringWriter* writer;

			std::vector<unsigned> firing;		// AER, group relative IDs, sorted ascending
			std::vector<bool> fired;		// Bitmap, neuron Id, width x height

			int sparseCoding;
			string axonalTract;

			// to be able to reset logs
			string m_spikesLogName; 
			string m_codingLogName;
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* FIRINGOUTBOUNDCHANNEL_HPP_ */
