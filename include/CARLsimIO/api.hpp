#ifndef HEADER_CARLSIMIO_API_H_INCLUDED
#define HEADER_CARLSIMIO_API_H_INCLUDED
  
#ifdef _MSC_VER // MSVC toolchain 
#	ifdef CARLSIMIO_MSVC_LIB // Static
#		define CARLSIMIO_API 
#	else  
#		if defined(CARLSIMIO_MSVC_DLL)
#			define CARLSIMIO_API __declspec(dllexport)
#		else
#			define CARLSIMIO_API __declspec(dllimport)
#	endif
#	include <boost/config.hpp>	// -> *
#	endif
#else
# define CARLSIMIO_API 
#endif // _MSC_VER

#ifndef _MSC_VER // MSVC toolchain 
      // https://stackoverflow.com/questions/59948723/how-to-use-sprintf-s-for-linux-c
#     define sprintf_s(buf, n, ...) snprintf((buf), (n), __VA_ARGS__)
#endif

#endif // HEADER_CARLSIMIO_API_H_INCLUDED
