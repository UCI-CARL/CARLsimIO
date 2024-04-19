#include "CARLsimIO/Channel/OutboundChannel/OutboundChannelFactory.hpp"
#include <CARLsimIO/Channel/OutboundChannel/FiringOutboundChannel.hpp>
#include <CARLsimIO/Channel/OutboundChannel/WbActuatorOutboundChannel.hpp>
#include <CARLsimIO/Writer/FiringWriter.hpp>
#include <CARLsimIO/Writer/Writer.hpp>
#include "CARLsimIO/CARLsimIOException.hpp"
#include "CARLsimIO/Log/Log.hpp"
using namespace carlsimio;


/** Default constructor
	 Initializes the list of Output Channels, if you've made a new Output Channel, add it here!  */
OutboundChannelFactory::OutboundChannelFactory()  {
	// Initially Empty
	this->channelList.push_back(FiringOutboundChannel().getChannelDescription());  
	this->channelList.push_back(WbActuatorOutboundChannel().getChannelDescription());
	printOutboundChannels();
}


/** Returns all Output Channels in the system */
vector<Description> OutboundChannelFactory::getAllChannels() {
	return channelList;
}


/** Creates a particular named Output Channel  */
OutboundChannel* OutboundChannelFactory::create(Description& desc, Writer* writer, map<string,Property>& channelProperties) {
	if(desc.getName() == "Firing Outbound Channel") {
		FiringOutboundChannel* channel = new FiringOutboundChannel();
		channel->initialize((FiringWriter*)writer, channelProperties);
		return channel;
	} else
	if (desc.getName() == "WbActuator Outbound Channel") {
		WbActuatorOutboundChannel* channel = new WbActuatorOutboundChannel();
		channel->initialize((WbActuatorWriter*)writer, channelProperties);
		return channel;
	}
	throw CARLsimIOException("Invalid channel type");
}


/*! Returns the default properties of a particular output channel */
map<string, Property> OutboundChannelFactory::getDefaultProperties(Description& desc){
	if(desc.getName() == "Firing Outbound Channel") {
		return FiringOutboundChannel().getProperties();
	} else
	if (desc.getName() == "WbActuator Outbound Channel") {
		return WbActuatorOutboundChannel().getProperties();
	}
	throw CARLsimIOException("Invalid outbound channel");
}



/** Prints out the available input channels */
void OutboundChannelFactory::printOutboundChannels(){
	for(size_t i=0; i<channelList.size(); ++i)
		LOG(LOG_DEBUG)<<"Outbound Channel: "<<channelList[i].getName()<<", "<<channelList[i].getDescription();
}


