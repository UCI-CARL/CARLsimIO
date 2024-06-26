CMAKE_MINIMUM_REQUIRED (VERSION 3.19.1)

#LN20201126  https://cmake.org/cmake/help/latest/guide/tutorial/index.html#id20
set(CMAKE_DEBUG_POSTFIX d)


SET(Boost_DEBUG OFF)


FIND_PACKAGE (Boost ${CARLSIMIO_BOOST_VERSION} EXACT REQUIRED COMPONENTS unit_test_framework date_time )

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

IF (${CARLSIMIO_SUPPORT_YARP})
  FIND_PACKAGE(ACE)
  FIND_PACKAGE(YARP)
	add_definitions(-DFINSPIKES_SUPPORT_YARP) 
	add_definitions(-DYARP_HAS_ACE)
	find_package(YARP 3.4 REQUIRED COMPONENTS os dev)
  #	INCLUDE_DIRECTORIES("C:/Test/yarp-3.4.1/include")
  #	INCLUDE_DIRECTORIES("C:/Test/ACE-6.5.12/ACE_wrappers")

  Message("$ENV{ACE_ROOT}")
  INCLUDE_DIRECTORIES($ENV{ACE_ROOT})  

  Message("${CMAKE_INSTALL_PREFIX}/include/yarp")
  INCLUDE_DIRECTORIES("${CMAKE_INSTALL_PREFIX}/include/yarp")

ENDIF (${CARLSIM_SUPPORT_YARP})


# var header generated in CMAKE_SROUCE_DIR with svn/git ignore
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/include/CARLsimIO")  


LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
SET(Boost_USE_MULTITHREADED ON)

FILE (GLOB_RECURSE output_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)

Message("Boost_INCLUDE_DIRS " ${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

IF(WIN32)
    SET (output_LIBS ${Boost_LIBRARIES} ${Log4Cxx_LIBRARIES} ${UnitTestPlusPlus_LIBRARIES} ${UnitTestPlusPlus_LIBRARIES} ${PROJECT_NAME} "ws2_32")
ELSE()
    SET (output_LIBS ${Boost_LIBRARIES} ${Log4Cxx_LIBRARIES} ${UnitTestPlusPlus_LIBRARIES} ${UnitTestPlusPlus_LIBRARIES} ${PROJECT_NAME})
ENDIF()


SET (output_BIN carlsimio-cli)

IF (NOT CMAKE_CROSSCOMPILING)
    LINK_DIRECTORIES(${MAINFOLDER}/lib/${ConfigurationName})
    ADD_EXECUTABLE(${output_BIN} ${output_SRCS})
    TARGET_LINK_LIBRARIES(${output_BIN} CARLsimIO)
   SET_TARGET_PROPERTIES(${output_BIN}
		PROPERTIES 
		 COMPILE_DEFINITIONS "CARLSIMIO_MSVC_LIB;_SCL_SECURE_NO_WARNINGS"
		 COMPILE_FLAGS "/MP")

ENDIF (NOT CMAKE_CROSSCOMPILING)

IF (${CARLSIMIO_BUILD_SHARED_LIB})
	INSTALL(TARGETS ${output_BIN} DESTINATION $<$<CONFIG:Debug>:debug/>bin)
ENDIF(${CARLSIMIO_BUILD_SHARED_LIB})



