#ifndef COMMON_HPP_
#define COMMON_HPP_


//CARLsimIO includes
#include <CARLsimIO/api.hpp>
#include <CARLsimIO/Bitmap.hpp>

//Other includes  // 
#include <array>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

namespace carlsimio {

	/** Static common functionality */
	class CARLSIMIO_API Common {
		public:

			static void openLog(string& log, ostream **p_log_os, ofstream **p_log_ofs)
			{
				(*p_log_os) = NULL;
				if((*p_log_ofs) != NULL) {
					(*p_log_ofs)->close();
					delete (*p_log_ofs);
				}

				(*p_log_ofs) = NULL;
				if(log=="cout")
					(*p_log_os) = &cout;
				else if(log!="null") {
					(*p_log_ofs) = new ofstream(log);
					(*p_log_os) = *p_log_ofs;
				}
			}

			static void initializeLog(ostream **p_log_os, ofstream **p_log_ofs) {
				*p_log_os = NULL;
				*p_log_ofs = NULL;
			}

			static void releaseLog(ostream **p_log_os, ofstream **p_log_ofs) {
				*p_log_os = NULL; // dereference
				if(*p_log_ofs != NULL) {
					(*p_log_ofs)->close();
					delete (*p_log_ofs);	
					(*p_log_ofs) = NULL;
				}
			}

			/*! iSpike Legacy */
			static void savePPMImage(const char* filename, Bitmap* image);
			static void writePatternToFile(const char* fileName, vector<int> pattern, int numOfNeurons);

			//========================  VARIABLES  =======================
			static const int redVsGreen = 0;
			static const int greenVsRed = 1;
			static const int blueVsYellow = 2;
			static const int greyVsGrey = 3;
			static const int motionSensitive = 4;
			static const int motionSensitiveLog = 5;
	};

}

#endif /* COMMON_HPP_ */
