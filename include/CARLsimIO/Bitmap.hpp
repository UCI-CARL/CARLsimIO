#ifndef BITMAP_HPP_
#define BITMAP_HPP_

#include <CARLsimIO/api.hpp>	

//Other includes
#include <string>
using namespace std;

namespace carlsimio {

	/** iSpike: A bitmap representation of an image, which has a height, width and depth.
		For example, with depth 3, each unsigned char could hold a red, green or blue value,
		so the contents array would be arranged as RGBRGBRGBRGB. This could be a 2x2 or a 4x1
		image, depending on the width and height settings. */
	class CARLSIMIO_API Bitmap {

		public:

			Bitmap();
			Bitmap(unsigned width, unsigned height, unsigned depth);
			Bitmap(unsigned width, unsigned height, unsigned depth, unsigned char initVal);
			Bitmap(const Bitmap& copy_from_me);
			~Bitmap();
			Bitmap& operator=(const Bitmap& rhs);
			unsigned char* getContents() const  { return contents;  }
			unsigned char getPixel(unsigned x, unsigned y, unsigned d) ;
			unsigned getDepth() const { return depth; }
			unsigned getHeight() const { return height; }
			unsigned getWidth() const { return width; }
			bool isEmpty() const { return size() == 0; }
			void reset(unsigned width, unsigned height, unsigned depth);

			/*! \return number of bytes used by this Bitmap */
			unsigned size() const;

		private:

			unsigned width;
			unsigned height;
			unsigned depth;
			unsigned char* contents;
	};

}

#endif /* BITMAP_HPP_ */
