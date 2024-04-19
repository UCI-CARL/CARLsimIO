#ifndef WRITERFACTORY_HPP_
#define WRITERFACTORY_HPP_

//CARLsimIO includes
#include "CARLsimIO/Writer/Writer.hpp"
#include <CARLsimIO/Description.hpp>

//Other includes
#include <vector>
#include <string>
using namespace std;

#include <CARLsimIO/api.hpp>	// CARLsimIO 2.1.2

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** List all Writers available in the system and creates a particular type of a Writer */
	class CARLSIMIO_API WriterFactory {
		private:
			/** A list of available writers */
			vector<Description> writerList;
			//string ip;
			//unsigned port;

			//=========================  METHODS  ===========================
			void printWriters();

		public:
			WriterFactory();
			//WriterFactory(std::string ip, unsigned port);
			Writer* create(Description& desc, map<string, Property>& writerProperties );
			map<string, Property> getDefaultProperties(Description& desc);
			vector<Description> getWritersOfType(string writerType);

	};
}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* WRITERFACTORY_HPP_ */
