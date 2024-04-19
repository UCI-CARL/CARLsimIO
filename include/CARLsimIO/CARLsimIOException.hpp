#ifndef CARLSIMIOEXCEPTION_HPP_
#define CARLSIMIOEXCEPTION_HPP_

#include <string>
#include <sstream>
using namespace std;

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {


	class CARLsimIOException : public exception {
		private:
			string description;

		public:
			CARLsimIOException(string description) throw() {
				this->description = description;
			}
			CARLsimIOException(string description, int arg1) throw() {
				ostringstream oss;
				oss<<description<<arg1;
				this->description = oss.str();
			}
			~CARLsimIOException() throw() {}

			const string& msg() const {
				return description;
			}

			const char* what() const throw() {
				return description.c_str();
			}

		};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* CARLSIMIOEXCEPTION_HPP_ */
