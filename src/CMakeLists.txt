CMAKE_MINIMUM_REQUIRED (VERSION 3.20)

OPTION(CARLSIMIO_BUILD_STATIC_LIB "Create a static library version of CARLsimIO" TRUE)
OPTION(CARLSIMIO_BUILD_SHARED_LIB "Create a shared library version of CARLsimIO" TRUE)
OPTION(CARLSIMIO_SUPPORT_YARP "Support YARP Middleware" ON)
IF (${CARLSIMIO_SUPPORT_YARP})
	add_definitions(-DCARLSIMIO_SUPPORT_YARP) 
	add_definitions(-DYARP_HAS_ACE)
	find_package(YARP 3.4 REQUIRED COMPONENTS os dev carl)
ENDIF (${CARLSIMIO_SUPPORT_YARP})

# Global Options affecting libraries, executables, and tests


set(CMAKE_DEBUG_POSTFIX d)	

SET(Boost_DEBUG ON)

find_package(Boost ${CARLSIMIO_BOOST_VERSION} EXACT REQUIRED COMPONENTS system regex thread)

# now see if boost was actually found
MESSAGE("Searching for Boost ${BOOST_COMPONENTS}")
if (Boost_FOUND)
  message("Boost WAS found!")
endif (Boost_FOUND)

# just print out the variables to see their values
message("BOOST_ROOT: " $ENV{BOOST_ROOT})
message("Boost_DIR : " ${Boost_DIR})
message("Boost_INCLUDE_DIR : " ${Boost_INCLUDE_DIRS})


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})


IF (${CARLSIMIO_SUPPORT_YARP})
  #find_package(ACE REQUIRED)		
  find_package(YARP REQUIRED)
	#INCLUDE_DIRECTORIES("${CMAKE_INSTALL_PREFIX}/include")
	#INCLUDE_DIRECTORIES("C:/Test/ACE-6.5.12/ACE_wrappers")  

  #Message("$ENV{ACE_ROOT}")
  INCLUDE_DIRECTORIES($ENV{ACE_ROOT})  

  #Message("${CMAKE_INSTALL_PREFIX}/include/yarp")
  INCLUDE_DIRECTORIES("${CMAKE_INSTALL_PREFIX}/include/yarp")
  INCLUDE_DIRECTORIES("${CMAKE_INSTALL_PREFIX}/include")

# ISSUE Linux Pi5  does not find ACE, but it works for YARP and YarpDevCarlEpuck 
#  find_package(ACE 6.5)  

ENDIF (${CARLSIMIO_SUPPORT_YARP})

IF (${CARLSIMIO_USE_CARLSIM})
	add_definitions(-DCARLSIMIO_USE_CARLSIM)
  INCLUDE_DIRECTORIES("${CMAKE_INSTALL_PREFIX}/include") 
	SET(carlsim_HRDS "../include/CARLsimIO/NeuronSim/CarlsimSim.hpp")
	SET(carlsim_SRC "NeuronSim/CarlsimSim.cpp")
ENDIF (${CARLSIMIO_USE_CARLSIM})




LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
SET(Boost_USE_MULTITHREADED ON)

IF(WIN32)
    SET (project_LIBS ${Boost_LIBRARIES} ${Log4Cxx_LIBRARIES} "ws2_32")
ELSE()
    SET (project_LIBS ${Boost_LIBRARIES} ${Log4Cxx_LIBRARIES})
ENDIF()

IF (NOT DEFINED project_build_shared)
   IF (CMAKE_CROSSCOMPILING)
       SET(project_build_shared 0)
   ELSE (CMAKE_CROSSCOMPILING)
       SET(project_build_shared 1)
   ENDIF (CMAKE_CROSSCOMPILING)
ENDIF (NOT DEFINED project_build_shared)


SET(project_HRDRS
  ../include/CARLsimIO/api.hpp
  ../include/CARLsimIO/AkdYoloEvent.hpp
  ../include/CARLsimIO/Bitmap.hpp
  ../include/CARLsimIO/Channel/Channel.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/OutboundChannel.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/OutboundChannelFactory.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/FiringOutboundChannel.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/WbActuatorOutboundChannel.hpp  # generic super class
  ../include/CARLsimIO/Channel/InboundChannel/InboundChannel.hpp
  ../include/CARLsimIO/Channel/InboundChannel/InboundChannelFactory.hpp
  ../include/CARLsimIO/Channel/InboundChannel/FiringInboundChannel.hpp
  ../include/CARLsimIO/Channel/InboundChannel/WbSensorInboundChannel.hpp   # Webots SensorArray -> Vector of Integer values like Distance-/Light Sensor (8x), ToF Sensor (1x), Wheels (2x)
  ../include/CARLsimIO/Channel/InboundChannel/AkdYoloInboundChannel.hpp   
  ../include/CARLsimIO/Common.hpp
  ../include/CARLsimIO/Description.hpp
  ../include/CARLsimIO/CARLsimIOException.hpp
  ../include/CARLsimIO/CARLsimIOThread.hpp  
  ../include/CARLsimIO/Log/Log.hpp
  ../include/CARLsimIO/NeuronSim/IzhikevichNeuronSim.hpp
  ../include/CARLsimIO/NeuronSim/AbstractSim.hpp
  ../include/CARLsimIO/NeuronSim/CurrentRelaySim.hpp
  ../include/CARLsimIO/NeuronSim/EmbeddedCpuSim.hpp
  ../include/CARLsimIO/Property.hpp
  ../include/CARLsimIO/PropertyHolder.hpp
  ../include/CARLsimIO/Reader/FiringReader.hpp
  ../include/CARLsimIO/Reader/Reader.hpp
  ../include/CARLsimIO/Reader/ReaderBuffer.hpp
  ../include/CARLsimIO/Reader/ReaderFactory.hpp
  ../include/CARLsimIO/Reader/FileWbSensorReader.hpp
  ../include/CARLsimIO/Reader/WbSensorReader.hpp
  ../include/CARLsimIO/Reader/AkdYoloReader.hpp
  ../include/CARLsimIO/version.h
  ../include/CARLsimIO/Writer/Writer.hpp
  ../include/CARLsimIO/Writer/WriterFactory.hpp
  ../include/CARLsimIO/Writer/FiringWriter.hpp  
  ../include/CARLsimIO/Writer/WbActuatorWriter.hpp  
)

IF (${CARLSIMIO_SUPPORT_YARP})
list(APPEND project_HRDRS
	../include/CARLsimIO/Reader/YarpFiringReader.hpp
	../include/CARLsimIO/Reader/YarpWbSensorReader.hpp
	../include/CARLsimIO/Reader/YarpAkdYoloReader.hpp
	../include/CARLsimIO/Writer/YarpFiringWriter.hpp 
	../include/CARLsimIO/Writer/YarpWbActuatorWriter.hpp 
)
ENDIF (${CARLSIMIO_SUPPORT_YARP})


SOURCE_GROUP("Header Files\\Channel" FILES  
  ../include/CARLsimIO/Channel/Channel.hpp
)

SOURCE_GROUP("Header Files\\Channel\\InboundChannel" FILES  
  ../include/CARLsimIO/Channel/InboundChannel/InboundChannel.hpp
  ../include/CARLsimIO/Channel/InboundChannel/InboundChannelFactory.hpp
  ../include/CARLsimIO/Channel/InboundChannel/FiringInboundChannel.hpp
  ../include/CARLsimIO/Channel/InboundChannel/WbSensorInboundChannel.hpp
  ../include/CARLsimIO/Channel/InboundChannel/AkdYoloInboundChannel.hpp
)

SOURCE_GROUP("Header Files\\Channel\\OutboundChannel" FILES  
  ../include/CARLsimIO/Channel/OutboundChannel/OutboundChannel.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/OutboundChannelFactory.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/FiringOutboundChannel.hpp
  ../include/CARLsimIO/Channel/OutboundChannel/WbActuatorOutboundChannel.hpp
)

SOURCE_GROUP("Header Files\\Log"  FILES  
  "../include/CARLsimIO/Log/Log.hpp"
)
 

SOURCE_GROUP("Header Files\\NeuronSim" FILES  
  ../include/CARLsimIO/NeuronSim/IzhikevichNeuronSim.hpp
  ../include/CARLsimIO/NeuronSim/AbstractSim.hpp
  ../include/CARLsimIO/NeuronSim/CurrentRelaySim.hpp
  ../include/CARLsimIO/NeuronSim/EmbeddedCpuSim.hpp
    ${carlsim_HRDS}
)

SOURCE_GROUP("Header Files\\Reader" FILES  
  ../include/CARLsimIO/Reader/Reader.hpp
  ../include/CARLsimIO/Reader/ReaderBuffer.hpp
  ../include/CARLsimIO/Reader/ReaderFactory.hpp
  ../include/CARLsimIO/Reader/FiringReader.hpp
  ../include/CARLsimIO/Reader/FileWbSensorReader.hpp    # consistency with Yarp Prefix
  ../include/CARLsimIO/Reader/WbSensorReader.hpp
  ../include/CARLsimIO/Reader/AkdYoloReader.hpp
)

SOURCE_GROUP("Header Files\\Writer" FILES  
  ../include/CARLsimIO/Writer/Writer.hpp
  ../include/CARLsimIO/Writer/WriterFactory.hpp
  ../include/CARLsimIO/Writer/FiringWriter.hpp  
  ../include/CARLsimIO/Writer/WbActuatorWriter.hpp  
)

IF (${CARLSIMIO_SUPPORT_YARP})
SOURCE_GROUP("Header Files\\YARP" FILES  
  ../include/CARLsimIO/Reader/YarpFiringReader.hpp
  ../include/CARLsimIO/Reader/YarpWbSensorReader.hpp
  ../include/CARLsimIO/Reader/YarpAkdYoloReader.hpp
  ../include/CARLsimIO/Writer/YarpFiringWriter.hpp  
  ../include/CARLsimIO/Writer/YarpWbActuatorWriter.hpp  
)
ENDIF (${CARLSIMIO_SUPPORT_YARP})


SET(project_SRCS
    Bitmap.cpp 
	Channel/InboundChannel/InboundChannelFactory.cpp
	Channel/InboundChannel/FiringInboundChannel.cpp
	Channel/InboundChannel/WbSensorInboundChannel.cpp
	Channel/InboundChannel/AkdYoloInboundChannel.cpp
    Channel/OutboundChannel/OutboundChannelFactory.cpp
    Channel/OutboundChannel/FiringOutboundChannel.cpp
    Channel/OutboundChannel/WbActuatorOutboundChannel.cpp
    Common.cpp
    Description.cpp
    Log/Log.cpp
	NeuronSim/IzhikevichNeuronSim.cpp
    NeuronSim/AbstractSim.cpp
    NeuronSim/CurrentRelaySim.cpp
    NeuronSim/EmbeddedCpuSim.cpp
	${carlsim_SRC}
    Property.cpp
    PropertyHolder.cpp
    Reader/ReaderBuffer.cpp
    Reader/ReaderFactory.cpp
    Reader/FiringReader.cpp
    Reader/FileWbSensorReader.cpp
    Reader/WbSensorReader.cpp
    Reader/AkdYoloReader.cpp
    Writer/WriterFactory.cpp
    Writer/FiringWriter.cpp
    Writer/WbActuatorWriter.cpp
    version.cpp
	)

IF (${CARLSIMIO_SUPPORT_YARP})
	list(APPEND project_SRCS
		Reader/YarpFiringReader.cpp
		Reader/YarpWbSensorReader.cpp
		Reader/YarpAkdYoloReader.cpp
		Writer/YarpFiringWriter.cpp
		Writer/YarpWbActuatorWriter.cpp
    )
ENDIF (${CARLSIMIO_SUPPORT_YARP})


MESSAGE("Debug Lib Suffix: ${CMAKE_DEBUG_POSTFIX}")

IF (${CARLSIMIO_BUILD_STATIC_LIB})

ENDIF(${CARLSIMIO_BUILD_STATIC_LIB})

IF (${CARLSIMIO_BUILD_SHARED_LIB})
	ADD_LIBRARY(CARLsimIO SHARED ${project_HRDRS} ${project_SRCS})
	TARGET_LINK_LIBRARIES(CARLsimIO ${project_LIBS})
	#if (${CARLSIMIO_SUPPORT_YARP})
	#	target_compile_definitions(CARLsimIO PRIVATE YARP_HAS_ACE)
	#endif() 
	 
	IF (${CARLSIMIO_SUPPORT_YARP})	
		TARGET_LINK_LIBRARIES(CARLsimIO YARP::YARP_init)
		TARGET_LINK_LIBRARIES(CARLsimIO YARP::YARP_OS)
		TARGET_LINK_LIBRARIES(CARLsimIO YARP::YARP_carl)
		TARGET_LINK_LIBRARIES(CARLsimIO YARP::YARP_sig)
		TARGET_LINK_LIBRARIES(CARLsimIO YARP::YARP_dev)
		# DEBUG ACE
		
		#TARGET_LINK_LIBRARIES(CARLsimIO "$ENV{ACE_ROOT}/lib/ACE$<$<CONFIG:Debug>:d>.lib")  
		#TARGET_LINK_LIBRARIES(CARLsimIO "$ENV{ACE_ROOT}/lib/ACE.lib")  	
		#TARGET_LINK_LIBRARIES(CARLsimIO "$ENV{ACE_ROOT}/lib/ACE.lib")  	
		#target_link_libraries(CARLsimIO ACE::ACE)
		# Linux Pi5 /usr/lib/aarch64-linux-gnu
		TARGET_LINK_LIBRARIES(CARLsimIO "$ENV{ACE_ROOT}/libACE.so") 	

	ENDIF (${CARLSIMIO_SUPPORT_YARP})

	if(${CARLSIMIO_USE_CARLSIM})	
		target_link_libraries(CARLsimIO 
			# ${CMAKE_INSTALL_PREFIX}/lib/carlsim$<$<CONFIG:Debug>:d>.lib
			#/home/ln/carlsimpp-t01/build/install/lib/libcarlsim.so.6.1.0  # Linux Pi5
			${CMAKE_INSTALL_PREFIX}/lib/libcarlsim$<$<CONFIG:Debug>:d>.so

			)    
	endif()

	SET_TARGET_PROPERTIES(CARLsimIO
		PROPERTIES 
      		VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}"
      		COMPILE_DEFINITIONS "CARLSIMIO_MSVC_DLL"	  		 
#		COMPILE_FLAGS "/MP"	#PATCH Linux Pi5
		DEFINE_SYMBOL BOOST_THREAD_USE_DLL
		CLEAN_DIRECT_OUTPUT 1)

	INSTALL(TARGETS CARLsimIO DESTINATION bin)
ENDIF(${CARLSIMIO_BUILD_SHARED_LIB})
