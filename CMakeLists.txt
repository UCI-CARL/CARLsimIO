CMAKE_MINIMUM_REQUIRED (VERSION 3.22)

# LN2021 enforce parallel build as default
if(MSVC)
  get_property(_USER_CMAKE_CXX_FLAGS CACHE CMAKE_CXX_FLAGS PROPERTY VALUE)
  string(FIND ${_USER_CMAKE_CXX_FLAGS} "/MP" _MSVC_MP_IDX)
  if(_MSVC_MP_IDX EQUAL -1)
    message("enforce parallel build")
    set(CMAKE_CXX_FLAGS "${_USER_CMAKE_CXX_FLAGS} /MP" CACHE STRING "enforce parallel build" FORCE)    
  endif()
endif()


PROJECT (CARLsimIO)
SET (APPLICATION_NAME "CARLsimIO")
SET (APPLICATION_CODENAME "${PROJECT_NAME}")
SET (APPLICATION_COPYRIGHT_YEARS "2024")
SET (APPLICATION_VERSION_MAJOR 1)
SET (APPLICATION_VERSION_MINOR 0)
SET (APPLICATION_VERSION_PATCH 0)
SET (APPLICATION_VERSION_TYPE SNAPSHOT)
SET (APPLICATION_VERSION_STRING "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}-${APPLICATION_VERSION_TYPE}")
SET (APPLICATION_VENDOR_ID "UCI-CARL")
SET (APPLICATION_VENDOR_NAME "UCI-CARL")
SET (APPLICATION_VENDOR_URL "https://github.com/UCI-CARL")
SET (APPLICATION_ID "${APPLICATION_VENDOR_ID}.${PROJECT_NAME}")

SET (APPLICATION_BUILD 1)

# Project Search Paths
LIST (APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)


#Boost Version
set(CARLSIMIO_BOOST_VERSION "1.74" CACHE STRING "Exact Version of Boost")

#find package CARLsim  see nemo_cpu, if not found warning/status, default = OFF
OPTION(CARLSIMIO_USE_CARLSIM "Uses CARLsim as Backend" ON)

   
# speed up          
FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
	OPTION(CARLSIMIO_USE_OPENMP "Use OpenMP for parallelisation of the host code" OFF)
ELSE(OPENMP_FOUND)
	MESSAGE(WARNING "OpenMP not found. Host code will be compiled without")
ENDIF(OPENMP_FOUND)
IF(CARLSIMIO_USE_OPENMP AND OPENMP_FOUND)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF(CARLSIMIO_USE_OPENMP AND OPENMP_FOUND)		  
		  
# Configure information
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/libraryinfo.h.in.cmake ${CMAKE_BINARY_DIR}/include/CARLsimIO/libraryinfo.h)

# Add Meta info - CAUTION: this is done at config time 
# BINARY_DIR instead SOURCE_DIR is best practice due cMake book and is added at root level in the search path
# , alternative would be a sub dir like generated
# the down side is that the binary include need to be added as include dir 
# second alternativ, generate in std. include but add a svn/git ignore flag
# In to automate an build time, attach it to the target, see book
# Var1: use svn from sub dir info for sub dir, e.g. rev of lib, rev of app rev. of test
# Var 2: use the rev from source root, (remark: following this argumentation, cMake.cache would be needed as well, ...
# svn/git ignore
# is 
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/buildinfo.h.in.cmake ${CMAKE_BINARY_DIR}/include/CARLsimIO/buildinfo.h)

# Add Build Targets
ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(docs)

# Add Install Targets
INSTALL(FILES ${CMAKE_BINARY_DIR}/include/CARLsimIO/libraryinfo.h DESTINATION include)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include PATTERN ".svn" EXCLUDE )


# Client enabled/disabled
OPTION(CARLSIMIO_CLIENT_ENABLED "Build Client" ON)
IF(CARLSIMIO_CLIENT_ENABLED)
	ADD_SUBDIRECTORY(client)
ENDIF(CARLSIMIO_CLIENT_ENABLED)

# Packaging
#IF(NOT APPLE)
#	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
#	SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
#ENDIF(NOT APPLE)
SET(CPACK_PACKAGE_VERSION_MAJOR ${APPLICATION_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${APPLICATION_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${APPLICATION_VERSION_PATCH})

IF(WIN32)
	SET(CPACK_GENERATOR "NSIS")
ENDIF(WIN32)

IF(APPLE)
	SET(CPACK_GENERATOR "PackageMaker")
ENDIF(APPLE)

SET(CPACK_SOURCE_GENERATOR "TGZ")

# By default cpack grabs everything in the source tree. However, the local
# source tree may contain user-specific files which should not be included.
# A list of such files (or patterns) can be specified in the file
# CMake/cpack.ignore
IF(EXISTS ${CMAKE_SOURCE_DIR}/CMake/cpack.ignore)
	FILE(READ CMake/cpack.ignore USER_CPACK_SOURCE_IGNORE_FILES)
	SET(CPACK_SOURCE_IGNORE_FILES ${USER_CPACK_SOURCE_IGNORE_FILES} ${CPACK_SOURCE_IGNORE_FILES})
ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/CMake/cpack.ignore)

SET(CPACK_SOURCE_IGNORE_FILES
		build
		docs/website
		".svn"
		".doc"
		".docx"
	${CPACK_SOURCE_IGNORE_FILES})
MESSAGE(${CPACK_SOURCE_IGNORE_FILES})

INCLUDE(CPack)
