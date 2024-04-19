#ifndef OUTBOUNDCHANNELFACTORY_HPP_
#define OUTBOUNDCHANNELFACTORY_HPP_

//CARLsimIO includes
#include "CARLsimIO/Channel/OutboundChannel/OutboundChannel.hpp"
#include <CARLsimIO/Writer/Writer.hpp>

//Other includes
#include <string>
using namespace std;

#include <CARLsimIO/api.hpp>	

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** The Output Channel Factory can list all Output Channels available in the system and can produce a particular type of an Output Channel 	  */
	class CARLSIMIO_API OutboundChannelFactory {
		private:
			/** A list of available Output Channels */
			vector<Description> channelList;

			//============================  METHODS  =======================
			void printOutboundChannels();


		public:
			OutboundChannelFactory();
			vector<Description> getAllChannels();
			OutboundChannel* create(Description& desc, Writer* writer, map<string, Property>& channelProperties);
			map<string, Property> getDefaultProperties(Description& desc);

		};

}
#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* OUTBOUNDCHANNELFACTORY_HPP_ */
