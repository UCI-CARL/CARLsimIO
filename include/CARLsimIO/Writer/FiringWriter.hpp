#ifndef FIRINGWRITER_HPP_
#define FIRINGWRITER_HPP_

#include <CARLsimIO/Writer/Writer.hpp>
#include <CARLsimIO/api.hpp>	


#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace carlsimio {

	
	class CARLSIMIO_API FiringWriter : public Writer{
		public:
			FiringWriter(): m_simtime(0), m_processed(0), m_last_ms(0){}
			virtual ~FiringWriter(){}

			virtual void setProperties(map<string, Property>& properties);

			virtual void syncSim(unsigned long long simtime)  {m_simtime = simtime;}



			virtual void initializeFiring(size_t size);
			virtual void setFiring(const std::vector<unsigned> & aer) = 0;

			virtual void initializeTract(const std::string tract) {this->tract = tract;};

			// New Diagnostic Interface
			virtual size_t getQueued()=0;  // Amount of Item in the buffer
			virtual unsigned long long getNext_ms()=0;

			virtual void reset();

			virtual size_t getProcessed();  
			virtual unsigned long long getLast_ms();


		protected:

			std::vector<unsigned> firing;		// AER, group relative IDs, sorted ascending

			std::string tract;

			unsigned long long m_simtime;

			// New diagnostig interface
			unsigned long long m_last_ms; 
			size_t m_processed; 


	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* FIRINGWRITER_HPP_ */
