#ifndef PROPERTYHOLDER_H
#define PROPERTYHOLDER_H

// generic utility class from iSpikes 

#include <CARLsimIO/api.hpp>	
#include "CARLsimIO/Property.hpp"

#include <stdlib.h>

#include <map>
#include <string>
using namespace std;
 

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	/** Abstract class with functionality to get and set properties */
	class CARLSIMIO_API PropertyHolder {
		public:
			/** Returns a map containing the current properties */
			virtual map<string, Property> getProperties(){ return propertyMap; }

			/** Sets the properties */
			virtual void setProperties(map<string, Property>& properties) = 0;

		protected:
			//=======================  VARIABLES  ========================
			/** Properties */
		    map<string, Property> propertyMap;

			/** Allows subclasses to track the number of properties that have been updated */
			unsigned updatePropertyCount;

			//=======================  METHODS  ===========================
			void addProperty(Property property);
			void checkProperty(Property& property);
			void printProperties();
			int updateIntegerProperty(Property& property);
			double updateDoubleProperty(Property& property);
			string updateStringProperty(Property& property);
			string updateComboProperty(Property& property);

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif//PROPERTYHOLDER_H
