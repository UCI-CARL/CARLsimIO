#ifndef CHANNEL_H_
#define CHANNEL_H_

//iSpike includes
#include <CARLsimIO/PropertyHolder.hpp>
#include <CARLsimIO/Log/Log.hpp>

//Other includes
#include <string>
using namespace std;

#include <CARLsimIO/api.hpp>	

#ifdef BOOST_MSVC
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif


namespace carlsimio {

	/** Abstract class holding properties common to all input and output channels */
	class CARLSIMIO_API Channel : public PropertyHolder {
		public:
			Channel(){ initialized = false; }
			virtual ~Channel(){}

			/** Retrieves the Channel Identifier  */
			int getId()	{ return this->id; }

			/** Sets the identifier of the Channel */
			void setId(int id){ this->id = id; }

			/** Retrieves the width of the Channel */
			int getWidth(){ return this->width;	}

			/** Sets the width of the Channel */
			void setWidth(int width){ this->width = width; }

			/** Retrieves the height of the Channel  */
			int getHeight(){ return this->height; }

			/** Sets the height of the Channel */
			void setHeight(int height){	this->height = height;	}

			/** Returns the size of the channel */
			unsigned size() { return width*height; }

			/** Retrieves the description of a Channel  */
			string getDescription(){ return this->description;	}

			/** Sets the description of a Channel */
			void setDescription(std::string description){ this->description = description; }

			/** Advances the channel by one time step */
			virtual void step() = 0;

			/*! Simulation Time in ms
			 * Q&D, should be set by Spikestream, respectively and Experiment
			 */
			unsigned long long m_simtime; // private,  kept public for compatibility reasons
			virtual void setSimtime(unsigned long long simtime) { m_simtime = simtime; }

			// New Diagnostic Interface
			virtual size_t getBufferQueued() { return 0; }  // Amount of Item in the buffer
			virtual unsigned long long getBufferNext() { return 0L; }  // model ms of next pending Item in the buffer 
			virtual void reset() { }
			virtual size_t getReaderProcessed() { return 0; }  // Amount of Item that have been processed by the channel
			virtual unsigned long long getReaderLast() { return 0L; }  // model ms of last item processed by the channel

		protected:
			//============================  VARIABLES  =========================
			/** Channel Identifier */
			int id;

			/** Neuron Width */
			unsigned width;

			/** Neuron Height */
			unsigned height;

			/** Channel Description */
			string description;


			//=============================  METHODS  ===========================
			bool isInitialized() {return initialized; }
			void setInitialized(bool initialized) { this->initialized = initialized; }


		private:
			/** Flag recording whether channel is initialized or not */
			bool initialized;

	};

}

#ifdef BOOST_MSVC
#  pragma warning( pop )
#endif

#endif /* CHANNEL_H_ */