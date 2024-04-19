#ifndef PROPERTY_HPP_
#define PROPERTY_HPP_

// generic utility class from iSpikes  

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <CARLsimIO/api.hpp>	
using namespace std;

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	/** Describes a general Channel, Reader or Writer Property
		Information that is stored depends on the type. */
	class CARLSIMIO_API Property {
		public:
			enum ValueType {
				Integer,
				Double,
				String,
				Combo,
				Undefined
			};

		private:
			ValueType type;
			string name;
			string description;
			bool readOnly;

			int intVal;
			double doubleVal;
			string stringVal;
			vector<string> options;

		public:
			Property();
			Property(ValueType type, int value, string name, string description, bool readOnly);
			Property(ValueType type, double value, string name, string description, bool readOnly);
			Property(string value, string name, string description, bool readOnly);
			Property(string value, vector<string> options, string name, string description, bool readOnly);
			Property(const Property& prop);
			virtual ~Property();
			Property& operator=(const Property& rhs);
			virtual string toString() { return ""; }
			ValueType getType() { return this->type; }
			string getName() {	return this->name; }
			string getDescription() { return this->description; }
			bool isReadOnly() { return this->readOnly; }

			int getInt();
			void setInt(int newInt);
			double getDouble();
			void setDouble(double newDouble);
			string getString();
			void setString(string newString);
			vector<string> getOptions();
			void setOptions(vector<string> options);

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* PROPERTY_HPP_ */
