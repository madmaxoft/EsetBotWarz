cmake_minimum_required (VERSION 2.8.7)
project (EsetBotWarz)


include_directories (SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../lib/jsoncpp/include")
include_directories (SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../lib/libevent/include")
include_directories (SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/..")

SET (SRCS
	BotWarzApp.cpp
	Comm.cpp
	Globals.cpp
	Main.cpp
	sha1.cpp
)

SET (HDRS
	BotWarzApp.h
	Comm.h
	Globals.h
	sha1.h
)


list(APPEND SOURCE "${SRCS}")
list(APPEND SOURCE "${HDRS}")

if (MSVC)
	# MSVC-specific handling:
	# Precompiled headers (1st part)
	SET_SOURCE_FILES_PROPERTIES(
		Globals.cpp PROPERTIES COMPILE_FLAGS "/Yc\"Globals.h\""
	)

	# Make MSVC generate the PDB files even for the release build:
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   /Zi")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE}    /DEBUG")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")
	set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /DEBUG")
elseif (CMAKE_COMPILER_IS_GNUCXX)
	add_definitions("-std=c++11")
endif()

set(EXECUTABLE EsetBotWarz)
add_executable(${EXECUTABLE} ${SOURCE})




# Precompiled headers (2nd part)
if (MSVC)
	SET_TARGET_PROPERTIES(
		${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/Yu\"Globals.h\""
		OBJECT_DEPENDS "$(IntDir)/$(TargetName.pch)"
	)
endif ()


# Link the required libraries:
if (WIN32)
	target_link_libraries(${EXECUTABLE} ws2_32.lib Psapi.lib)
endif()
target_link_libraries(${EXECUTABLE} jsoncpp_lib_static Network event_core event_extra)

