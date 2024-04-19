#ifndef WBACTUATOROUTBOUNDCHANNEL_HPP_
#define WBACTUATOROUTBOUNDCHANNEL_HPP_

#include <CARLsimIO/Channel/OutboundChannel/OutboundChannel.hpp>
#include <CARLsimIO/Writer/WbActuatorWriter.hpp>
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
	class CARLSIMIO_API WbActuatorOutboundChannel : public OutboundChannel {
	public:
		WbActuatorOutboundChannel();
		virtual ~WbActuatorOutboundChannel();
		void initialize(Writer* writer, map<string, Property>& properties);
		void setFiring(const std::vector<unsigned>& buffer);
		void setProperties(map<string, Property>& properties);
		void step();


		// New Diagnostic Interface
		virtual size_t getBufferQueued(); // Amount of Item in the buffer/pending
		virtual unsigned long long getBufferNext();  // model ms of next pending Item in the buffer 
		virtual void reset();
		virtual size_t getReaderProcessed();  // Amount of Item that have been processed by the channel
		virtual unsigned long long getReaderLast();  // model ms of last item processed by the channel



	protected:
		void updateProperties(map<string, Property>& properties);

		void logFiringsHeader(long long simtime, long long tick_time, int group, int cluster);
		void logFirings(const std::vector<unsigned>& buffer, long long simtime, long long tick_time, int group, int cluster);


	private:
		/** Sends Orders to YARP, file, etc. */
		WbActuatorWriter* writer;

	
		string m_actuatorPrefix;

		size_t  m_doubles;  // m_ints,    doubles for velo,  int for led on color leds,  rgb: float for index into value map of rgb entries
		

		double m_min, m_max;


		/** The rate at which the current variables decay */
		double rateOfDecay;

		double m_threshold;

		/** Vector holding the current variables */
		std::vector<double> currentVariables;

		std::vector<double> currentVariableValues;

		/** The amount by which a current variable is incremented with each spike */
		double currentIncrement;

		// to be able to reset logs
		string m_spikesLogName;
		string m_codingLogName;
	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* WBACTUATOROUTBOUNDCHANNEL_HPP_ */
