//CARLsimIO includes
#include "CARLsimIO/Writer/WriterFactory.hpp"
#include <CARLsimIO/CARLsimIOException.hpp>
#include <CARLsimIO/Writer/FiringWriter.hpp>
#include <CARLsimIO/Writer/YarpFiringWriter.hpp>
#include <CARLsimIO/Writer/YarpWbActuatorWriter.hpp>
#include "CARLsimIO/Log/Log.hpp"
using namespace carlsimio;

/** Default constructor
 * Initialises the list of writers, if you've made a new writer, add it here! */
WriterFactory::WriterFactory(){
	//Store list of available writers
	writerList.push_back(YarpFiringWriter().getWriterDescription());
	writerList.push_back(YarpWbActuatorWriter().getWriterDescription());
	printWriters();
}


/*--------------------------------------------------------------------*/
/*---------                 PUBLIC METHODS                     -------*/
/*--------------------------------------------------------------------*/

/** Returns all writers of a particular type */
vector<Description> WriterFactory::getWritersOfType(string writerType){
	vector<Description> result;
	for(unsigned i = 0; i < writerList.size(); i++){
		if(writerList[i].getType() == writerType)
			result.push_back(writerList[i]);
	}
	return result;
}


/*! Returns the default properties of a particular writer */
map<string, Property> WriterFactory::getDefaultProperties(Description& desc){
	if(desc.getName() == "Yarp Firing Writer") {
		return YarpFiringWriter().getProperties();
	} else
	if (desc.getName() == "Yarp WbActuator Writer") {
		return YarpWbActuatorWriter().getProperties();
	}
	throw CARLsimIOException("Invalid writer");
}


/** Creates a particular writer   */
Writer* WriterFactory::create(Description& desc, map<string, Property>& writerProperties){
	Writer* result = NULL;
	if(desc.getName() == "Yarp Firing Writer") {
		result = new YarpFiringWriter();
	} else
	if (desc.getName() == "Yarp WbActuator Writer") {
		result = new YarpWbActuatorWriter();
	}
	else {
		throw CARLsimIOException("Invalid writer");
	}
	result->initialize(writerProperties);
	return result;
}


/*--------------------------------------------------------------------*/
/*---------                 PRIVATE METHODS                    -------*/
/*--------------------------------------------------------------------*/

/** Prints out the available writers */
void WriterFactory::printWriters(){
	for(size_t i=0; i<writerList.size(); ++i)
		LOG(LOG_DEBUG)<<"Writer: "<<writerList[i].getName()<<", "<<writerList[i].getDescription();
}
