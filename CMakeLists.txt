project(lua-proxy)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# uses the FindGear2D.cmake to search for Gear2D
find_package(Gear2D REQUIRED)
find_package(Lua51 REQUIRED)

set(CMAKE_INSTALL_PREFIX ${Gear2D_COMPONENT_PREFIX})

# add the Gear2D include directories and link directories.
# they are calculated by FindGear2D.cmake
include_directories(${Gear2D_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
link_directories(${Gear2D_LINK_DIR})

# Use git revision as version

include(GetGitRevisionDescription)
git_describe(Gear2D_COMPONENT_VERSION --tags)

if(Gear2D_COMPONENT_VERSION AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.in")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/VERSION.in" "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" @ONLY)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
  file(STRINGS VERSION Gear2D_COMPONENT_VERSION)
else()
  set(Gear2D_COMPONENT_VERSION "unknown")
endif()
set(CMAKE_INSTALL_PREFIX ${Gear2D_COMPONENT_PREFIX})

file(GLOB compile_sources "src/*.cc")

add_library(basic SHARED ${compile_sources})

install(TARGETS basic
				LIBRARY DESTINATION proxy-lua)

target_link_libraries(basic ${Gear2D_LIBRARY} ${LUA_LIBRARIES})

if (WIN32)
    get_filename_component(LUA_LIBRARY_DIR ${LUA_LIBRARIES} PATH)
    set(LUA_DLL "${LUA_LIBRARY_DIR}/lua51.dll")
    message(STATUS "Packaging Lua51 DLLs: ${LUA_DLL}")
    install(FILES ${LUA_DLL}
        DESTINATION bin
    )
endif (WIN32)

# CPack stuff
if (WIN32)
    set(CPACK_SOURCE_GENERATOR ZIP)
    set(CPACK_GENERATOR NSIS;ZIP)
    set(CPACK_PACKAGE_NAME "Gear2D-Lua-Proxy")
elseif(UNIX)
    set(CPACK_PACKAGE_NAME "gear2d-lua-proxy")
    set(CPACK_SOURCE_GENERATOR TGZ)
    set(CPACK_GENERATOR STGZ;TGZ)
endif()

set(CPACK_PACKAGE_VENDOR "Gear2D Labs")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "gear2d")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gear2D - Lua Proxy Component")
set(CPACK_PACKAGE_VERSION "${Gear2D_COMPONENT_VERSION}")

if(WIN32)
    set(CPACK_SOURCE_IGNORE_FILES "VERSION.in;\\\\.git.*;${CMAKE_BINARY_DIR}/.*;.*~;\\\\.kdev4/.*;.*\\\\.kdev4;.*\\\\.swp")
else()
    set(CPACK_SOURCE_IGNORE_FILES "VERSION.in;\\\\.git.*;${CMAKE_BINARY_DIR}/.*;.*~;\\\\.kdev4/.*;.*\\\\.kdev4;.*\\\\.swp;.*thirdparty/win32/.*")
endif(WIN32)

set(PACK_PACKAGE_EXECUTABLES gear2d;Gear2D Game Engine)

if (MSVC)
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-MSVC-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
else()
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
endif()

if(NOT WIN32)
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
endif(NOT WIN32)

# we dont want top-level directories for components
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

include(CPack)