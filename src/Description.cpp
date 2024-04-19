//CARLsimIO includes
#include "CARLsimIO/Description.hpp"
using namespace carlsimio;

/** Empty constructor */
Description::Description(){
	name = "Unnamed";
	description = "Undescribed";
	type = "Unknown";
}


/**  Default constructor */
Description::Description(string name, string description, string type){
	this->name = name;
	this->description = description;
	this->type = type;
}


/** Copy constructor */
Description::Description(const Description& desc){
	this->name = desc.name;
	this->description = desc.description;
	this->type = desc.type;
}


/** Destructor */
Description::~Description() {
}


/** Assignment operator */
Description & Description::operator= (const Description & rhs) {
	if (this != &rhs) {// protect against invalid self-assignment
		this->name = rhs.name;
		this->description = rhs.description;
		this->type = rhs.type;
	}

	// by convention, always return *this
	return *this;
}

