#ifndef INBOUNDCHANNELFACTORY_HPP_
#define INBOUNDCHANNELFACTORY_HPP_

//CARLsimIO includes
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Channel/InboundChannel/InboundChannel.hpp>

//Other includes
#include <string>
using namespace std;

#include <CARLsimIO/api.hpp>	

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/**
	 * @class InputChannelFactory
	 * @brief A Factory of Input Channels
	 *
	 * The Input Channel Factory can list all Input Channels available in the system and can produce a particular type of an Input Channel  */
	class CARLSIMIO_API InboundChannelFactory {
		private:
			/** A list of available Input Channels */
			vector<Description> channelList;

			//============================  METHODS  =======================
			void printInboundChannels();


		public:
			InboundChannelFactory();
			~InboundChannelFactory() {}; 

			vector<Description> getAllChannels();
			InboundChannel* create(Description& desc, Reader* reader, map<string, Property>& channelProperties);
			map<string, Property> getDefaultProperties(Description& desc);

			InboundChannel* readFrom(const string& path, const string& xmlModel);



	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* INBOUNDCHANNELFACTORY_HPP_ */
