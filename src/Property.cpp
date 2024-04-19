#include "CARLsimIO/Property.hpp"
#include "CARLsimIO/CARLsimIOException.hpp"
using namespace carlsimio;

//Other includes
#include <iostream>
using namespace std;

/** Empty constructor */
Property::Property(){
	type = Property::Undefined;
	name = "Unnamed";
	description = "Undescribed";
	readOnly = false;

	intVal = 0;
	doubleVal = 0.0;
	stringVal = "Undefined";
}


/** Integer property constructor - avoids confusion between double and integer. */
Property::Property(ValueType type, int value, string name, string description, bool readOnly){
	if(type == Property::Integer){
		this->type = Property::Integer;
		this->intVal = value;
		this->doubleVal = 0;
	}
	else if(type == Property::Double){
		this->type = Property::Double;
		this->doubleVal = value;
		this->intVal = 0;
	}
	this->name = name;
	this->description = description;
	this->readOnly = readOnly;

	stringVal = "Undefined";
}


/** Double property constructor - avoids confusion between double and integer.  */
Property::Property(ValueType type, double value, string name, string description, bool readOnly){
	if(type == Property::Integer){
		this->type = Property::Integer;
		this->intVal = (int)value;
		this->doubleVal = 0;
	}
	else if(type == Property::Double){
		this->type = Property::Double;
		this->doubleVal = value;
		this->intVal = 0;
	}
	this->name = name;
	this->description = description;
	this->readOnly = readOnly;

	this->stringVal = "Undefined";
}


/** String property constructor */
Property::Property(string value, string name, string description, bool readOnly){
	this->type = Property::String;
	this->stringVal = value;
	this->name = name;
	this->description = description;
	this->readOnly = readOnly;

	this->intVal = 0;
	this->doubleVal = 0.0;
}


/** Combo property constructor */
Property::Property(string value, vector<string> options, string name, string description, bool readOnly){
	this->type = Property::Combo;
	this->options = options;
	this->stringVal = value;
	this->name = name;
	this->description = description;
	this->readOnly = readOnly;

	this->intVal = 0;
	this->doubleVal = 0.0;
}


/** Property copy constructor */
Property::Property(const Property& prop){
	this->type = prop.type;
	this->name = prop.name;
	this->description = prop.description;
	this->readOnly = prop.readOnly;
	this->intVal = prop.intVal;
	this->doubleVal = prop.doubleVal;
	this->stringVal = prop.stringVal;
	this->options = prop.options;
}


/** Property destructor */
Property::~Property(){
}


/*----------------------------------------------------------*/
/*------                PUBLIC METHODS                ------*/
/*----------------------------------------------------------*/

/** Property assignment operator */
Property& Property::operator=(const Property& rhs){
	//Check for self assignment
	if(this == &rhs)
		return *this;

	this->type = rhs.type;
	this->name = rhs.name;
	this->description = rhs.description;
	this->readOnly = rhs.readOnly;
	this->intVal = rhs.intVal;
	this->doubleVal = rhs.doubleVal;
	this->stringVal = rhs.stringVal;
	this->options = rhs.options;

	return *this;
}


int Property::getInt(){
	if(this->type == Property::Integer)
		return intVal;
	throw CARLsimIOException("getInt() should not be called on a non-integer property: " + getName());
}

void Property::setInt(int newInt){
	if(this->type != Property::Integer)
		throw CARLsimIOException("setInt() should not be called on a non-integer property: " + getName());
	this->intVal = newInt;
}

double Property::getDouble(){
	if(this->type == Property::Double)
		return doubleVal;
	throw CARLsimIOException("getDouble() should not be called on a non-double property: " + getName());
}

void Property::setDouble(double newDouble){
	if(this->type != Property::Double)
		throw CARLsimIOException("setDouble() should not be called on a non-double property: " + getName());
	this->doubleVal = newDouble;
}

string Property::getString(){
	if(this->type == Property::String || this->type == Property::Combo)
		return stringVal;
	throw CARLsimIOException("getString() should not be called on a non-string or non-combo property: " + getName());
}

void Property::setString(string newString){
	if(this->type != Property::String && this->type != Property::Combo)
		throw CARLsimIOException("setString() should not be called on a non-string or non-combo property: " + getName());
	this->stringVal = newString;
}

vector<string> Property::getOptions(){
	if(this->type == Property::Combo)
		return options;
	throw CARLsimIOException("getOptions() should not be called on a non-combo property: " + getName());
}

void Property::setOptions(vector<string> options){
	if(this->type != Property::Combo)
		throw CARLsimIOException("setOptions() should not be called on a non-combo property: " + getName());
	this->options = options;
}


