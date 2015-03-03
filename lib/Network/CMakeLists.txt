
cmake_minimum_required (VERSION 2.6)
project (Network)

# Add the flags for c++11 support:
if (CMAKE_COMPILER_IS_GNUCXX)
	add_definitions("-std=c++11")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_definitions("-std=c++11")
endif()

SET (SRCS
	CriticalSection.cpp
	Event.cpp
	Globals.cpp
	HostnameLookup.cpp
	IPLookup.cpp
	NetworkSingleton.cpp
	ServerHandleImpl.cpp
	StringUtils.cpp
	TCPLinkImpl.cpp
	UDPEndpointImpl.cpp
)

SET (HDRS
	CriticalSection.h
	Event.h
	Globals.h
	HostnameLookup.h
	IPLookup.h
	Network.h
	NetworkSingleton.h
	ServerHandleImpl.h
	StringUtils.h
	TCPLinkImpl.h
	UDPEndpointImpl.h
)

include_directories (SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../libevent/include")

add_library(Network ${SRCS} ${HDRS})

# Precompiled headers (MSVC-only):
if (MSVC)
	SET_SOURCE_FILES_PROPERTIES(
		Globals.cpp PROPERTIES COMPILE_FLAGS "/Yc\"Globals.h\""
	)
	SET_TARGET_PROPERTIES(
		${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/Yu\"Globals.h\""
		OBJECT_DEPENDS "$(IntDir)/$(TargetName.pch)"
	)
endif()



if(UNIX)
	if(NOT APPLE)
		target_link_libraries(Network rt)
	endif()

	target_link_libraries(Network pthread event_core event_extra)
endif()
