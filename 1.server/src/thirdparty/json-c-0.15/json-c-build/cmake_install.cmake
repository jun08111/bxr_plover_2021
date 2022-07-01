# Install script for directory: /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS)
  set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS "OWNER_READ;OWNER_WRITE;OWNER_EXECUTE;GROUP_READ;GROUP_EXECUTE;WORLD_READ;WORLD_EXECUTE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE STATIC_LIBRARY FILES "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/libjson-c.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c/json-c-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c/json-c-targets.cmake"
         "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/CMakeFiles/Export/lib64/cmake/json-c/json-c-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c/json-c-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c/json-c-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c" TYPE FILE FILES "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/CMakeFiles/Export/lib64/cmake/json-c/json-c-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c" TYPE FILE FILES "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/CMakeFiles/Export/lib64/cmake/json-c/json-c-targets-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/cmake/json-c" TYPE FILE FILES "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-config.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/pkgconfig" TYPE FILE FILES "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_config.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/arraylist.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/debug.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_c_version.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_inttypes.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_object.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_object_iterator.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_tokener.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_types.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_util.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_visit.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/linkhash.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/printbuf.h;/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c/json_pointer.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json-c-0.15/include/json-c" TYPE FILE FILES
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json_config.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/json.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/arraylist.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/debug.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_c_version.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_inttypes.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_object.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_object_iterator.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_tokener.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_types.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_util.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_visit.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/linkhash.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/printbuf.h"
    "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json_pointer.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests/cmake_install.cmake")
  include("/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/apps/cmake_install.cmake")
  include("/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/doc/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
