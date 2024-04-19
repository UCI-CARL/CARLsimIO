#include <CARLsimIO/version.h>
#include <CARLsimIO/libraryinfo.h>

EXTERNC int projectname_version_major()
{
// If APPLICATION_VERSION_MAJOR is set to 0 in CMakeList.txt, the symbol is undefined:
//	version.cpp(6): error C2065: 'APPLICATION_VERSION_MAJOR' : undeclared identifier
#ifdef APPLICATION_VERSION_MAJOR
    return APPLICATION_VERSION_MAJOR;
#else
    return 0;
#endif
}

EXTERNC int projectname_version_minor()
{
    return APPLICATION_VERSION_MINOR;
}

EXTERNC int projectname_version_patch()
{
    return APPLICATION_VERSION_PATCH;
}
