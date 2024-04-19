#ifndef READERFACTORY_HPP_
#define READERFACTORY_HPP_

//CARLsimIO includes
#include "CARLsimIO/Reader/Reader.hpp"
#include <CARLsimIO/Description.hpp>

//Other includes
#include <string>
#include <vector>
using namespace std;

#include <CARLsimIO/api.hpp>	// CARLsimIO 2.1.2

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	/** Lists all Readers available in the system and can produce a particular type of a Reader 	 */
	class CARLSIMIO_API ReaderFactory {
		public:
			ReaderFactory(/*const int barhist=0*/);
			ReaderFactory(string ip, unsigned port);
			//1>AddChannelDialog.obj : error LNK2019: unresolved external symbol "__declspec(dllimport) public: __cdecl ispike::InputChannelFactory::~InputChannelFactory(void)" (__imp_??1InputChannelFactory@ispike@@QEAA@XZ) referenced in function "public: void * __cdecl ispike::InputChannelFactory::`scalar deleting destructor'(unsigned int)" (??_GInputChannelFactory@ispike@@QEAAPEAXI@Z)
			~ReaderFactory(void) {};
			//Reader* create(Description& desc, map<string, Property>& readerProperties, const int barhist);
			Reader* create(Description& desc, map<string, Property>& readerProperties);
			map<string, Property> getDefaultProperties(Description& desc);
			vector<Description> getReadersOfType(string readerType);

			Reader* readFrom(const string& path, const string& xmlModel, std::map<std::string, Property> &channelProperties);

		private:
			//==============================  VARIABLES  =============================
			/// A list of available readers
			vector<Description> readerList;
			//string ip;
			//unsigned port;

			//=============================  METHODS  ================================
			void printReaders();

	};

}
#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* READERFACTORY_HPP_ */
