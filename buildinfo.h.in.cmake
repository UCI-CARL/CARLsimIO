#ifndef HEADER_SRC_BUILDINFO_H_INCLUDED
#define HEADER_SRC_BUILDINFO_H_INCLUDED

/*
 * AUTO-GENERATION WARNING:
 *     This file has been automatically generated from "appinfo.h.in.cmake".
 *     DO NOT edit this file, as any changes will be automatically discarded.
 */
 
#cmakedefine APPLICATION_BUILD               @APPLICATION_BUILD@

#ifndef APPLICATION_BUILD
#   error "APPLICATION_BUILD is not defined. Check SVN integration in CMakeLists.txt file."
#endif

#endif