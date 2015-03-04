cmake_minimum_required (VERSION 2.8.7)

# Without this, the MSVC variable isn't defined for MSVC builds ( http://www.cmake.org/pipermail/cmake/2011-November/047130.html )
enable_language(CXX C)

# We need C++11 features, Visual Studio has those from VS2012, but it needs a new platform toolset for those; VS2013 supports them natively:
# Adapted from http://binglongx.wordpress.com/2013/06/28/set-non-default-platform-toolset-in-cmake/
if(MSVC OR MSVC_IDE)
  if( MSVC_VERSION LESS 1700 )       # VC10- / VS2010-
    message(FATAL_ERROR "The project requires C++11 features. "
      "You need at least Visual Studio 11 (Microsoft Visual Studio 2012), "
      "with Microsoft Visual C++ Compiler Nov 2012 CTP (v120_CTP_Nov2012).")
  elseif( MSVC_VERSION EQUAL 1700 )  # VC11 / VS2012
    message( "VC11: using Microsoft Visual Studio 2012 "
      "with Microsoft Visual C++ Compiler Nov 2012 CTP (v120_CTP_Nov2012)" )
    set(CMAKE_GENERATOR_TOOLSET "v120_CTP_Nov2012" CACHE STRING "Platform Toolset" FORCE)
  else() # VC12+, assuming C++11 supported.
  endif()
endif()





project (EsetBotWarz)





# Under Windows, we need Lua as DLL; on *nix we need it linked statically:
if (WIN32)
	add_definitions(-DLUA_BUILD_AS_DLL)
endif()

# Set options for JsonCPP, disable their tests:
set(JSONCPP_WITH_TESTS               NO CACHE BOOL "Enable JsonCPP tests"      FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST NO CACHE BOOL "Enable JsonCPP unit tests" FORCE)

# Set options for LibEvent, disable all their tests and benchmarks:
set(EVENT__DISABLE_OPENSSL   YES CACHE BOOL "Disable OpenSSL in LibEvent"       FORCE)
set(EVENT__DISABLE_BENCHMARK YES CACHE BOOL "Disable LibEvent benchmarks"       FORCE)
set(EVENT__DISABLE_TESTS     YES CACHE BOOL "Disable LibEvent tests"            FORCE)
set(EVENT__DISABLE_REGRESS   YES CACHE BOOL "Disable LibEvent regression tests" FORCE)
set(EVENT__DISABLE_SAMPLES   YES CACHE BOOL "Disable LibEvent samples"          FORCE)

# Check that the libraries are present:
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/lib/jsoncpp/CMakeLists.txt)
	message(FATAL_ERROR "JsonCPP is missing in folder lib/jsoncpp. Have you initialized the submodules / downloaded the extra libraries?")
endif()
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/lib/libevent/CMakeLists.txt)
	message(FATAL_ERROR "LibEvent is missing in folder lib/libevent. Have you initialized and updated the submodules / downloaded the extra libraries?")
endif()

# Add proper includes for LibEvent's event-config.h header:
include_directories(SYSTEM ${LIBEVENT_INCLUDE_DIRS})

# Include all the libraries:
add_subdirectory(lib/jsoncpp/)
add_subdirectory(lib/libevent/)
add_subdirectory(lib/lua/)
if (WIN32)
	add_subdirectory(lib/luaproxy/)
endif()
add_subdirectory(lib/Network/)



# Finally include the main app sources:
add_subdirectory (src)




