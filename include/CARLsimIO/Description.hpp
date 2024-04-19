#ifndef DESCRIPTION_HPP_
#define DESCRIPTION_HPP_

// generic utility class from iSpikes 

#include <CARLsimIO/api.hpp>

//Other includes
#include <string>
using namespace std;

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** This class describes a channel or a reader or writer */
	class CARLSIMIO_API Description {
		public:
			Description();
			Description(string name, string description, string type);
			Description(const Description& copy_from_me);
			~Description();
			Description & operator= (const Description& desc);

			/** Retrieves the Channel description */
			string getDescription() const { return description;	}

			/**  Retrives the Channel name	*/
			string getName() const { return name; }

			/** Retrieves the type of Reader this Channel accepts */
			string getType() const { return type; }

		private:
			/// Name
			string name;

			/// A description
			string description;

			/// The type
			string type;
	};

}


#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* DESCRIPTION_HPP_ */
