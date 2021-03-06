
cmake_minimum_required (VERSION 2.8.2)
project (lua)

file(GLOB SOURCE
	"src/*.c"
)

list(REMOVE_ITEM SOURCE "${PROJECT_SOURCE_DIR}/src/lua.c" "${PROJECT_SOURCE_DIR}/src/luac.c")

# add headers to MSVC project files:
if (MSVC)
	file(GLOB HEADERS "src/*.h")
	list(REMOVE_ITEM SOURCE "${PROJECT_SOURCE_DIR}/src/lua.h" "${PROJECT_SOURCE_DIR}/src/luac.h")
	set(SOURCE ${SOURCE} ${HEADERS})
	source_group("Sources" FILES ${SOURCE})
endif()


# Lua needs to be linked dynamically on Windows and statically on *nix, so that LuaRocks work
if (WIN32)
	add_library(lua SHARED ${SOURCE})

	set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out)

	# Output the executable into the $/out folder, so that the executable can find it when debugging:
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out)
	SET_TARGET_PROPERTIES(lua PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_SOURCE_DIR}/out
		ARCHIVE_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_SOURCE_DIR}/out
		ARCHIVE_OUTPUT_DIRECTORY_DEBUGPROFILE   ${CMAKE_SOURCE_DIR}/out
		ARCHIVE_OUTPUT_DIRECTORY_RELEASEPROFILE ${CMAKE_SOURCE_DIR}/out
		LIBRARY_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_SOURCE_DIR}/out
		LIBRARY_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_SOURCE_DIR}/out
		LIBRARY_OUTPUT_DIRECTORY_DEBUGPROFILE   ${CMAKE_SOURCE_DIR}/out
		LIBRARY_OUTPUT_DIRECTORY_RELEASEPROFILE ${CMAKE_SOURCE_DIR}/out
		RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_SOURCE_DIR}/out
		RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_SOURCE_DIR}/out
		RUNTIME_OUTPUT_DIRECTORY_DEBUGPROFILE   ${CMAKE_SOURCE_DIR}/out
		RUNTIME_OUTPUT_DIRECTORY_RELEASEPROFILE ${CMAKE_SOURCE_DIR}/out
	)
	if (MSVC)
		# Remove SCL warnings, we expect this library to have been tested safe
		SET_TARGET_PROPERTIES(
			lua PROPERTIES COMPILE_FLAGS "-D_CRT_SECURE_NO_WARNINGS"
		)
	endif()

	set_target_properties(lua PROPERTIES OUTPUT_NAME "lua51" PREFIX "")

	# NOTE: The DLL for each configuration is stored at the same place, thus overwriting each other.
	# This is known, however such behavior is needed for LuaRocks - they always load "lua5.1.dll" or "lua51.dll"
	# We make it work by compiling to "lua51.dll" and providing a proxy-DLL "lua5.1.dll"
	# See http://lua-users.org/wiki/LuaProxyDllFour for details
else()
	add_library(lua ${SOURCE})
endif()


# Tell Lua what dynamic loader to use (for LuaRocks):
if (UNIX)
	add_definitions(-DLUA_USE_DLOPEN)
endif()

if (UNIX)
	target_link_libraries(lua m ${DYNAMIC_LOADER})
endif()

