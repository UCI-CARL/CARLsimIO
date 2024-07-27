//CARLsimIO includes
#include <CARLsimIO/Common.hpp>
#include <CARLsimIO/Bitmap.hpp>
#include <CARLsimIO/CARLsimIOException.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
using namespace carlsimio;

//Other includes
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <ios>
#include <vector>
#include <algorithm>


/*! x */
vector<double> Common::doublesFromString(const string stringValue)
{
	vector<double> doubles;
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep1(";");  // Level 1  // ISSUE: Use ; instead? -> see barsecsFormString
	tokenizer tokens1(stringValue, sep1);
	for (tokenizer::iterator iter1 = tokens1.begin();
		iter1 != tokens1.end(); ++iter1)
	{
		doubles.push_back(boost::lexical_cast<double>(*iter1));
	}
	return doubles;
}



/** Writes the supplied image to file */
void Common::savePPMImage(const char* filename, Bitmap* image){
  std::ofstream file_handle(filename, std::ios::binary);
  if (file_handle) {
    file_handle << "P6" << std::endl << image->getWidth() << ' ' << image->getHeight() << std::endl << 255 << std::endl;
	//Write colour image
	if(image->getDepth() ==3)
		file_handle.write((char *)image->getContents(), image->getWidth() * image->getHeight() * image->getDepth());

	//Write black and white image
	else if(image->getDepth() == 1){
		int imageSize = image->size();
		char* imageContents = (char*)image->getContents();
		for(int i=0; i<imageSize; ++i){
			//Write each pixel three times
			file_handle.write(&imageContents[i], 1);
			file_handle.write(&imageContents[i], 1);
			file_handle.write(&imageContents[i], 1);
		}
	}

	//Unknown image
	else
		throw CARLsimIOException("Common: Image that is not depth 1 or 3 cannot be written.");
    file_handle.close();
  }
}


/** Writes a spike pattern to file */
void Common::writePatternToFile(const char* fileName, std::vector<int> pattern, int numOfNeurons){
 std::ofstream fileStream;

 fileStream.open(fileName, std::fstream::out | std::fstream::app);

 if (!fileStream) {
   std::ostringstream messageStream;
   messageStream << "Can't write angles: " << fileName;
   std::string message(messageStream.str());
   throw CARLsimIOException(message);
 }

 //fileStream << boost::lexical_cast<std::string>(angle) << std::endl;
 for( int i = 0; i < numOfNeurons; i++ ) {
   if(std::find(pattern.begin(), pattern.end(), i) != pattern.end()) {
     fileStream << "1,";
   }
   else {
     fileStream << "0,";
   }
 }
 fileStream << std::endl;

 if (fileStream.fail()) {
   std::ostringstream messageStream;
   messageStream << "Can't write angles: " << fileName;
   std::string message(messageStream.str());
   throw CARLsimIOException(message);
 }

 fileStream.close();
}
