//CARLsimIO includes
#include <CARLsimIO/Reader/ReaderFactory.hpp>
#include <CARLsimIO/Reader/FiringReader.hpp>
#include <CARLsimIO/Reader/WbSensorReader.hpp>
#include <CARLsimIO/Reader/AkdYoloReader.hpp>

#ifdef CARLSIMIO_SUPPORT_YARP
#include <CARLsimIO/Reader/YarpFiringReader.hpp>
#include <CARLsimIO/Reader/YarpWbSensorReader.hpp>
#include <CARLsimIO/Reader/YarpAkdYoloReader.hpp>
#endif 

#include "CARLsimIO/CARLsimIOException.hpp"
#include "CARLsimIO/Log/Log.hpp"
using namespace carlsimio;

//Other includes
#include <iostream>
using namespace std;

/** Default constructor
 * Initialises the list of readers, if you've made a new reader, add it here! */
ReaderFactory::ReaderFactory(/*const int barhist*/){
LOG(LOG_DEBUG)<< __FUNCTION__;
#ifdef CARLSIMIO_SUPPORT_YARP
	this->readerList.push_back(YarpFiringReader().getReaderDescription());  
	this->readerList.push_back(YarpWbSensorReader().getReaderDescription());
	this->readerList.push_back(YarpAkdYoloReader().getReaderDescription());
#endif 
	printReaders();
}


/** Constructor that creates YARP readers as well */
ReaderFactory::ReaderFactory(string ip, unsigned port){
LOG(LOG_DEBUG)<< __FUNCTION__;
#ifdef CARLSIMIO_SUPPORT_YARP
	this->readerList.push_back(YarpFiringReader().getReaderDescription());  
	this->readerList.push_back(YarpWbSensorReader().getReaderDescription());
	this->readerList.push_back(YarpAkdYoloReader().getReaderDescription());
#endif 
	printReaders();

}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

/** Returns readers of a particular type */
vector<Description> ReaderFactory::getReadersOfType(string readerType){
	vector<Description> result;
	for(unsigned i = 0; i < readerList.size(); i++) {
		if(readerList[i].getType() == readerType)
			result.push_back(readerList[i]);
	}
	return result;
}


/*! Returns the default properties of a particular reader */
map<string, Property> ReaderFactory::getDefaultProperties(Description& desc){
#ifdef CARLSIMIO_SUPPORT_YARP
	if(desc.getName() == "Yarp Firing Reader") {
		return YarpFiringReader().getProperties();
	} else
	if (desc.getName() == "Yarp WbSensor Reader") {
		return YarpWbSensorReader().getProperties();
	} else
	if (desc.getName() == "Yarp Yolo Reader") {
		return YarpAkdYoloReader().getProperties();
	}
#endif 
	throw CARLsimIOException("Invalid reader");
}


/** Creates and initialises a particular reader */
Reader* ReaderFactory::create(Description& desc, map<string, Property>& readerProperties) {
	Reader* result = NULL;

#ifdef CARLSIMIO_SUPPORT_YARP
	if (desc.getName() == "Yarp Firing Reader") {
		result = new YarpFiringReader();
	} else
	if (desc.getName() == "Yarp WbSensor Reader") {
		result = new YarpWbSensorReader();
	} else
	if (desc.getName() == "Yarp Yolo Reader") {
		result = new YarpAkdYoloReader();
	}
#endif 
	else {
		throw CARLsimIOException("Invalid reader type");
	}
	result->initialize(readerProperties);
	return result;
}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

/** Prints out the available readers */
void ReaderFactory::printReaders(){
	for(size_t i=0; i<readerList.size(); ++i)
		LOG(LOG_DEBUG)<<"Reader: "<<readerList[i].getName()<<", "<<readerList[i].getDescription();
}




Reader* ReaderFactory::readFrom(const string& path, const string& xmlModel, std::map<std::string, Property> &channelProperties) {
		return NULL;
}
