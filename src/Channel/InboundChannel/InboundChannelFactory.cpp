
#include <CARLsimIO/Channel/InboundChannel/InboundChannelFactory.hpp>
#include <CARLsimIO/Channel/InboundChannel/FiringInboundChannel.hpp>
#include <CARLsimIO/Channel/InboundChannel/WbSensorInboundChannel.hpp>
#include <CARLsimIO/Reader/Reader.hpp>
#include <CARLsimIO/Reader/ReaderFactory.hpp>
#include <CARLsimIO/Reader/FiringReader.hpp>
#include "CARLsimIO/CARLsimIOException.hpp"
#include "CARLsimIO/Log/Log.hpp"

using namespace carlsimio;


/** Default constructor
	Initialises the list of Input Channels, if you've made a new Input Channel, add it here!  */
InboundChannelFactory::InboundChannelFactory(){
	channelList.push_back(FiringInboundChannel().getChannelDescription());
	channelList.push_back(WbSensorInboundChannel().getChannelDescription());
	printInboundChannels();
}


/** Returns all Input Channels in the system */
vector<Description> InboundChannelFactory::getAllChannels() {
	return channelList;
}


/** Creates a particular Input Channel */
InboundChannel* InboundChannelFactory::create(Description& desc, Reader* reader, map<string, Property>& channelProperties) {
	if(desc.getName() == "Firing Inbound Channel"){
		FiringInboundChannel* channel = new FiringInboundChannel();
		channel->initialize(reader, channelProperties);  // (FiringReader*)
		return channel;
	} else
	if (desc.getName() == "WbSensor Inbound Channel") {
		WbSensorInboundChannel* channel = new WbSensorInboundChannel();
		channel->initialize(reader, channelProperties);
		return channel;
	}

	throw CARLsimIOException("Invalid channel name");
}



/*! Returns the default properties of a particular input channel */
map<string, Property> InboundChannelFactory::getDefaultProperties(Description& desc){
	if(desc.getName() == "Firing Inbound Channel") {
		return FiringInboundChannel().getProperties();
	} else
	if (desc.getName() == "WbSensor Inbound Channel") {
		return WbSensorInboundChannel().getProperties();;
	}
	throw CARLsimIOException("Invalid inbound channel");
}


/** Prints out the available input channels */
void InboundChannelFactory::printInboundChannels(){
	for(size_t i=0; i<channelList.size(); ++i)
		LOG(LOG_DEBUG)<<"Inbound Channel: "<<channelList[i].getName()<<", "<<channelList[i].getDescription();
}



InboundChannel* InboundChannelFactory::readFrom(const string& path , const string& xmlModel) {

		ReaderFactory factory;

		return NULL;

}