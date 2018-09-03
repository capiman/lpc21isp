cmake_minimum_required( VERSION 2.8.5 )

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Relase)
endif()

project(lpc21isp LANGUAGES C)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "OpenBSD")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FREEBSD__")
elseif(APPLE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__APPLE__")
endif()

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-D_GNU_SOURCE)
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
  add_definitions(-D_CONSOLE -D_MBCS -D_CRT_SECURE_NO_WARNINGS)
endif()


add_executable(${PROJECT_NAME} 
  lpc21isp.c lpc21isp.h
  adprog.c adprog.h
  lpcprog.c lpcprog.h
  lpcterm.c lpcterm.h
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

# uninstall
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake/make_uninstall.cmake")
